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

#ifndef _NTXM7_H_
#define _NTXM7_H_

#include "player.h"

class NTXM7
{
	public:
		NTXM7(void (*_playTimerHandler)(void));
		~NTXM7(void);
		
		// override new and delete to avoid linking cruft. (by WinterMute)
		static void* operator new (size_t size);
		static void operator delete (void *p);
		
		// Exchange commands with ARM9, call this every vblank
		void updateCommands(void);
		
		// call this from the timer0 irq handler
		// the timer is set up for you.
		void timerHandler(void);
		
		void setSong(Song *song);
		void play(bool repeat, u8 potpos=0, u16 row=0);
		void stop(void);
		
		// Play a single note using an instrument from the loaded song
		// instidx: index of the instrument
		//    note: 48 corresponds to c-4
		//  volume: 0-255
		// channel: 0-15, 255=auto
		void playNote(u8 instidx, u8 note=48, u8 volume=255, u8 channel=255);
		
		// Play the given sample (and send a notification when done)
		void playSample(Sample *sample, u8 note, u8 volume, u8 channel);
		
		// Stop playback on a channel
		void stopChannel(u8 channel);
		
		// Set a pattern to looping
		void setPatternLoop(bool loopstate);
		
	private:
		Player *player;
};

#endif
