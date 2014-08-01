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

#include "ntxm/demokit.h"

#define timers2ms(tlow,thigh)(tlow | (thigh<<16)) >> 5

int ticksSpeed;
unsigned int lastTime;
unsigned int timeCounted;
int clockStopped;

void demoInit(void)
{
	reStartRealTicks();
	reStartTicks();
}

#ifndef ARM7
#define TIMER_ENABLE (1<<7)
#define TIMER_DIV_1024 3
#define TIMER_CASCADE (1<<2)
#endif

void reStartRealTicks(void)
{
#ifdef ARM7
	TIMER2_DATA=0;
	TIMER3_DATA=0;
	TIMER2_CR=TIMER_DIV_1024 | TIMER_ENABLE;
	TIMER3_CR=TIMER_CASCADE | TIMER_ENABLE;
#else
	DSTimerWrite(2, (TIMER_DIV_1024 | TIMER_ENABLE) << 16);
	DSTimerWrite(3, (TIMER_CASCADE | TIMER_ENABLE) << 16);
#endif
}

unsigned int getRealTicks(void)
{
#ifdef ARM7
	return timers2ms(TIMER2_DATA, TIMER3_DATA);
#else
	return timers2ms(DSTimerTick(2), DSTimerTick(3));
#endif
}

void reStartTicks(void)
{
	ticksSpeed = 100;
	clockStopped = 0;
	setTicksTo(0);
}

void startTicks(void)
{
	if (clockStopped) {
		clockStopped = 0;
		lastTime = getRealTicks();
	}
}

void stopTicks(void)
{
	if (!clockStopped) {
		clockStopped = 1;
		getTicks();
	}
}

void setTicksTo(unsigned int time)
{
	timeCounted = time;
	lastTime = getRealTicks();
}

unsigned int getTicks(void)
{
	unsigned int t = ((getRealTicks() - lastTime)*ticksSpeed)/100;
	if ((t > 0) || (-t < timeCounted)) {
		timeCounted += t;
	} else {
		timeCounted = 0;
	}
	lastTime = getRealTicks();
	return timeCounted;
}

void setTicksSpeed(int percentage)
{
	getTicks();
	ticksSpeed = percentage;
}

int getTicksSpeed(void)
{
	return ticksSpeed;
}

void delay(unsigned int d) {
	unsigned int start = getTicks();
	while (getTicks() <= start+d);
}


int my_rand(void)
{
	static int seed = 2701;
	return seed = ((seed * 1103515245) + 12345) & 0x7fffffff;
}

