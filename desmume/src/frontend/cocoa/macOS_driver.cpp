/*
	Copyright (C) 2018 DeSmuME team
 
	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.
 
	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
 
	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>

#include "macOS_driver.h"
#include "ClientAVCaptureObject.h"
#include "ClientExecutionControl.h"
#include "ClientVideoOutput.h"
#include "NDSSystem.h"
#include "SPU.h"
#ifdef HAVE_LUA
#include "lua-engine.h"
#include "MacLuaScriptConsole.h"
#endif


pthread_mutex_t* macOS_driver::GetCoreThreadMutexLock()
{
	return this->__mutexThreadExecute;
}

void macOS_driver::SetCoreThreadMutexLock(pthread_mutex_t *theMutex)
{
	this->__mutexThreadExecute = theMutex;
}

pthread_rwlock_t* macOS_driver::GetCoreExecuteRWLock()
{
	return this->__rwlockCoreExecute;
}

void macOS_driver::SetCoreExecuteRWLock(pthread_rwlock_t *theRwLock)
{
	this->__rwlockCoreExecute = theRwLock;
}

void macOS_driver::SetExecutionControl(ClientExecutionControl *execControl)
{
	this->__execControl = execControl;
}

void macOS_driver::AVI_SoundUpdate(void *soundData, int soundLen)
{
	ClientAVCaptureObject *avCaptureObject = this->__execControl->GetClientAVCaptureObjectApplied();
	
	if ((avCaptureObject == NULL) || !avCaptureObject->IsCapturingAudio())
	{
		return;
	}
	
	avCaptureObject->CaptureAudioFrames(soundData, soundLen);
}

bool macOS_driver::AVI_IsRecording()
{
	ClientAVCaptureObject *avCaptureObject = this->__execControl->GetClientAVCaptureObjectApplied();
	return ((avCaptureObject != NULL) && avCaptureObject->IsCapturingVideo());
}

bool macOS_driver::WAV_IsRecording()
{
	ClientAVCaptureObject *avCaptureObject = this->__execControl->GetClientAVCaptureObjectApplied();
	return ((avCaptureObject != NULL) && avCaptureObject->IsCapturingAudio());
}

void macOS_driver::EMU_DebugIdleEnter()
{
	this->__execControl->SetIsInDebugTrap(true);
	pthread_rwlock_unlock(this->__rwlockCoreExecute);
	pthread_mutex_unlock(this->__mutexThreadExecute);
}

void macOS_driver::EMU_DebugIdleUpdate()
{
	usleep(50);
}

void macOS_driver::EMU_DebugIdleWakeUp()
{
	pthread_mutex_lock(this->__mutexThreadExecute);
	pthread_rwlock_wrlock(this->__rwlockCoreExecute);
	this->__execControl->SetIsInDebugTrap(false);
}

void macOS_driver::SetDisplayOutputManager(ClientDisplayViewOutputManager *displayOutputManager)
{
	this->__displayOutputManager = displayOutputManager;
}

extern "C" void macOS_PumpEvents();

BaseDriver::eStepMainLoopResult macOS_driver::EMU_StepMainLoop(bool allowSleep, bool allowPause, int frameSkip, bool disableUser, bool disableCore)
{
	if (this->__execControl == NULL)
	{
		return ESTEP_DONE;
	}

	if (this->__execControl->GetExecutionBehavior() != ExecutionBehavior_Pause)
	{
		this->__execControl->SetExecutionBehavior(ExecutionBehavior_Pause);
	}

	pthread_rwlock_wrlock(this->__rwlockCoreExecute);

	if (!disableCore)
	{
#ifdef HAVE_LUA
		lua_script_clear_graphics_buffer();
		NDS_beginProcessingInput();
		CallRegisteredLuaFunctions(LUACALL_BEFOREEMULATION);
		NDS_endProcessingInput();
#endif

		NDS_exec<false>();
		SPU_Emulate_user();
		this->__execControl->FetchOutputPostNDSExec();

#ifdef HAVE_LUA
		CallRegisteredLuaFunctions(LUACALL_AFTEREMULATION);
#endif
	}

	pthread_rwlock_unlock(this->__rwlockCoreExecute);

	if (!disableUser && this->__displayOutputManager != NULL)
	{
		this->__displayOutputManager->SetNDSFrameInfoToAll(this->__execControl->GetNDSFrameInfo());
	}

	macOS_PumpEvents();

	return ESTEP_DONE;
}
