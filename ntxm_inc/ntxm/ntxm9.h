/*
 * libNTXM - XM Player Library for the Nintendo DS
 *
 *    Copyright (C) 2005-2008 Tobias Weyand (0xtob)
 *                         me@nitrotracker.tobw.net
 *
 */

/***** BEGIN LICENSE BLOCK *****
 * 
 * Version: Noncommercial zLib License / GPL 3.0
 * 
 * The contents of this file are subject to the Noncommercial zLib License 
 * (the "License"); you may not use this file except in compliance with
 * the License. You should have recieved a copy of the license with this package.
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 3 or later (the "GPL"),
 * in which case the provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only under the terms of
 * either the GPL, and not to allow others to use your version of this file under
 * the terms of the Noncommercial zLib License, indicate your decision by
 * deleting the provisions above and replace them with the notice and other
 * provisions required by the GPL. If you do not delete the provisions above,
 * a recipient may use your version of this file under the terms of any one of
 * the GPL or the Noncommercial zLib License.
 * 
 ***** END LICENSE BLOCK *****/

#ifndef _NTXM9_H_
#define _NTXM9_H_

#include "song.h"
#include "xm_transport.h"

class NTXM9
{
	public:
		NTXM9();
		~NTXM9();
		
		// Load the specified xm file from the file system using libfat
		// Returns 0 on success, else an error code
		u16 load(const char *filename);
		
		// Returns a pointer to a string describing the error corresponding
		// to the given error code.
		const char *getError(u16 error_id);
		
		// Start playing
		void play(bool repeat);
		
		// Stop playing
		void stop(void);
		
	private:
		XMTransport* xm_transport;
		Song *song;
};

extern "C"
{
	bool ntxmInstallARM7();
	void ntxmUninstallARM7();
}

#endif
