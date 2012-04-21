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

#include "ntxm/ntxm9.h"
#include "ntxm/demokit.h"
#include "ntxm/fifocommand.h"

NTXM9::NTXM9()
	:xm_transport(0), song(0)
{
	xm_transport = new XMTransport();
	CommandInit();
}

NTXM9::~NTXM9()
{
	delete xm_transport;
	
	if(song != 0)
		delete song;

	CommandDeinit();
}

u16 NTXM9::load(const char *filename)
{
	u16 err = xm_transport->load(filename, &song);
	CommandSetSong(song);
	return err;
}

const char *NTXM9::getError(u16 error_id)
{
	return xm_transport->getError(error_id);
}

void NTXM9::play(bool repeat)
{
	if(song == 0)
		return;
	
	CommandStartPlay(0, 0, repeat);
}

void NTXM9::stop(void)
{
	if(song == 0)
		return;
	
	CommandStopPlay();
}
