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

#include <stdlib.h>

#include "ntxm/ntxm7.h"
#include "ntxm/fifocommand.h"

void* NTXM7::operator new (size_t size) {

	return malloc(size);

} // default ctor implicitly called here

void NTXM7::operator delete (void *p) {

	if ( NULL != p )
		free(p);

} // default dtor implicitly called here

NTXM7::NTXM7(void (*_playTimerHandler)(void))
	:player(0)
{
    CommandInit();
	player = new Player(_playTimerHandler);
}

NTXM7::~NTXM7(void)
{
	CommandDeinit();
	delete player;
}

void NTXM7::timerHandler(void)
{
	player->playTimerHandler();
}

void NTXM7::setSong(Song *song)
{
	player->setSong(song);
}

void NTXM7::play(bool repeat, u8 potpos, u16 row)
{
	player->play(potpos, row, repeat);
}

void NTXM7::stop(void)
{
	player->stop();
}

void NTXM7::playNote(u8 instidx, u8 note, u8 volume, u8 channel)
{
	player->playNote(note, volume, channel, instidx);
}

void NTXM7::playSample(Sample *sample, u8 note, u8 volume, u8 channel)
{
	player->playSample(sample, note, volume, channel);
}

void NTXM7::stopChannel(u8 channel)
{
	player->stopChannel(channel);
}

void NTXM7::setPatternLoop(bool loopstate)
{
	player->setPatternLoop(loopstate);
}
