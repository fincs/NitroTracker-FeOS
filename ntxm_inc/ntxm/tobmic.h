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

// Microphone functions that record at higher quality than the libnds
// functions. TODO: submit a libnds patch for this :)

#ifndef _TOBMIC_H_
#define _TOBMIC_H_

#define PM_GAIN_OFFSET	3

void tob_PM_SetAmp(u8 control);
void tob_PM_SetGain(u8 gain);
u16 tob_MIC_ReadData112(void);
void tob_StartRecording(u16* buffer, int length);
int tob_StopRecording(void);
void tob_ProcessMicrophoneTimerIRQ(void);
void tob_MIC_On(void);
void tob_MIC_Off(void);

#endif
