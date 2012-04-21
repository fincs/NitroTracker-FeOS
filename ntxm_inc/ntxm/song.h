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

#ifndef SONG_H
#define SONG_H

#include <nds.h>

#include "instrument.h"

#define MAX_INSTRUMENTS			128
#define MAX_INSTRUMENT_SAMPLES	16
#define MAX_PATTERNS			256
#define MAX_POT_LENGTH			256
#define MAX_CHANNELS			16 // XM has 32, but 16 is DS hardware limited unless software mixing were implemented. Yeah, perhaps.
#define MAX_PATTERN_LENGTH		256
#define DEFAULT_PATTERN_LENGTH	64
#define DEFAULT_BPM				125
#define DEFAULT_SPEED			6
#define DEFAULT_CHANNELS		4

#define EMPTY_NOTE				255
#define STOP_NOTE				254

#define NO_INSTRUMENT			255
#define MAX_SONG_NAME_LENGTH	20

#define NO_VOLUME				255
#define MAX_VOLUME				127

#define NO_EFFECT				255
#define NO_EFFECT_PARAM			0

#define EFFECT_ARPEGGIO			0x0
#define EFFECT_PORTA_UP			0x1
#define EFFECT_VOLUME_SLIDE		0xA
#define EFFECT_SET_VOLUME		0xC
#define EFFECT_PATTERN_BREAK	0xD
#define EFFECT_E				0xE

#define EFFECT_E_FINE_PORTA_UP			0x1
#define EFFECT_E_FINE_PORTA_DOWN		0x2
#define EFFECT_E_SET_GLISS_CONTROL		0x3
#define EFFECT_E_SET_VIBRATO_CONTROL	0x4
#define EFFECT_E_SET_FINETUNE			0x5
#define EFFECT_E_SET_LOOP				0x6

#define EFFECT_E_NOTE_CUT				0x0C

#define EFFECT_SET_SPEED_TEMPO			0x0F
/*
  0      Appregio
  1  (*) Porta up
  2  (*) Porta down
  3  (*) Tone porta
  4  (*) Vibrato
  5  (*) Tone porta+Volume slide
  6  (*) Vibrato+Volume slide
  7  (*) Tremolo
  8      Set panning
  9      Sample offset
  A  (*) Volume slide
  B      Position jump
  C      Set volume
  D      Pattern break
  E1 (*) Fine porta up
  E2 (*) Fine porta down
  E3     Set gliss control
  E4     Set vibrato control
  E5     Set finetune
  E6     Set loop begin/loop
  E7     Set tremolo control
  E9     Retrig note
  EA (*) Fine volume slide up
  EB (*) Fine volume slide down
  EC     Note cut
  ED     Note delay
  EE     Pattern delay
  F      Set tempo/BPM
  G      (010h) Set global volume
  H  (*) (011h) Global volume slide
  I      (012h) Unused
  J      (013h) Unused
  K      (014h) Unused
  L      (015h) Set envelope position
  M      (016h) Unused
  N      (017h) Unused
  O      (018h) Unused
  P  (*) (019h) Panning slide
  Q      (01ah) Unused
  R  (*) (01bh) Multi retrig note
  S      (01ch) Unused
  T      (01dh) Tremor
  U      (01eh) Unused
  V      (01fh) Unused
  W      (020h) Unused
  X1 (*) (021h) Extra fine porta up
  X2 (*) (021h) Extra fine porta down
*/

// NitroTracker allows for two effects (effect and effect2) simultaneously.
// This is possible in XM through volume effects to some extent.

typedef struct {
	u8 note;
	u8 instrument;
	u8 volume;
	u8 effect;
	u8 effect_param;
	u8 effect2;
	u8 effect2_param;
} Cell;

/*
This class represents a song. The format is kept open. The current feature set
is a subset of XM, but export and import for mod, it, s3m could come. To edit a
pattern, get its pointer with getPattern().
*/

class Song {
	friend class Player;
	
	public:
		
		Song(u8 _speed=DEFAULT_SPEED, u8 _bpm=DEFAULT_BPM, u8 _channels=DEFAULT_CHANNELS);
		
		~Song();
		
		void setExternalTimerHandler(void (*_externalTimerHandler)(void));
		
		Cell **getPattern(u8 idx);
		u8 getChannels(void);
		u16 getPatternLength(u8 idx);
		
		// Get milliseconds per row. Returns a 16.16 fixed point value
		u32 getMsPerRow(void);
		
		// Get milliseconds per tick. Returns a 16.16 fixed point value
		u32 getMsPerTick(void);
		
		Instrument *getInstrument(u8 instidx);
		u8 getInstruments(void);
		
		void setInstrument(u8 idx, Instrument *instrument);
		
		//
		// Playback control
		//
		
		// POT functions
		void potAdd(u8 ptn=0);
		void potDel(u8 element);
		void potIns(u8 idx, u8 pattern);
		u8 getPotLength(void);
		u8 getPotEntry(u8 idx);
		void setPotEntry(u8 idx, u8 value);
		
		void addPattern(u16 length=DEFAULT_PATTERN_LENGTH);
		
		// More/less channels
		void channelAdd(void);
		void channelDel(void);
		
		u8 getNumPatterns(void);
		
		void resizePattern(u8 ptn, u16 newlength);
		
		// The most important functions
		void setName(const char *_name);
		const char *getName(void);
		
		void setRestartPosition(u8 _restart_position);
		u8 getRestartPosition(void);
		
		u8 getTempo(void);
		u8 getBPM(void);
		
		void setTempo(u8 _tempo);
		void setBpm(u8 _bpm);
		
		// Zapping
		void zapPatterns(void);
		void zapInstruments(void);
		
		void clearCell(Cell *cell);
		
		// Muting
		void setChannelMute(u8 chn, bool muted);
		bool channelMuted(u8 chn);
		
	private:
		
		void killPatterns(void);
		void killInstruments(void);
		
		u8 speed;
		u8 bpm;
		u8 n_channels;
		u8 restart_position;
		
		u16 *patternlengths;
		u16 *internal_patternlengths; // "real length of the pattern":
		// patternlengths stores the lengths of the patterns that user actually sees.
		// If the user shortens a pattern, then only the value in patternlengths is
		// decreased, but the data is not deleted. This helps prevent accidental data
		// loss. internal_patternlengths can therefore only grow.
		
		u8 *pattern_order_table;
		Instrument **instruments;
		
		char *name;
		
		u16 n_patterns;
		u16 potsize;
		
		Cell ***patterns;
		
		bool channels_muted[MAX_CHANNELS];
};

#endif
