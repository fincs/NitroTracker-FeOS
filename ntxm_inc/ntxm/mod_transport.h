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

#ifndef MOD_TRANSPORT
#define MOD_TRANSPORT

#include "format_transport.h"

#define MOD_TRANSPORT_ERROR_INITFAIL				1
#define MOD_TRANSPORT_ERROR_FOPENFAIL				2
#define MOD_TRANSPORT_ERROR_MAGICNUMBERINVALID			3
#define MOD_TRANSPORT_ERROR_MEMFULL				4
#define MOD_TRANSPORT_ERROR_PATTERN_READ				5
#define MOD_TRANSPORT_FILE_TOO_BIG_FOR_RAM			6
#define MOD_TRANSPORT_NULL_INSTRUMENT				7 // Deprecated
#define MOD_TRANSPORT_PATTERN_TOO_LONG				8
#define MOD_TRANSPORT_FILE_ZERO_BYTE				9

typedef struct SampleInfo
{
	char name[22];
	u16 length;
	u8 finetune;
	u8 volume;
	u16 repeat_offset;
	u16 repeat_length;
};

class ModTransport: public FormatTransport {
	public:
		
		// Loads a song from a file and puts it in the song argument
		// returns 0 on success, an error code else
		u16 load(const char *filename, Song **_song);
		
		// Saves a song to a file
		u16 save(const char *filename, Song *song);
		
		const char *getError(u16 error_id);
		
	private:
};

#endif
