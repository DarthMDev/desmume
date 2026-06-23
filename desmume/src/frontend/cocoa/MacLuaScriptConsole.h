/*
	Copyright (C) 2026 DeSmuME Team

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

#ifndef __MACLUASCRIPTCONSOLE_H__
#define __MACLUASCRIPTCONSOLE_H__

#ifdef HAVE_LUA

#import <Cocoa/Cocoa.h>

#ifdef __cplusplus
extern "C" {
#endif

void lua_script_open_console(void);
void lua_script_close_all(void);

#ifdef __cplusplus
}
#endif

#endif /* HAVE_LUA */

#endif /* __MACLUASCRIPTCONSOLE_H__ */
