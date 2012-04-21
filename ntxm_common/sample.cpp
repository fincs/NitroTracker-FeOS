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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>

#include "ntxm/sample.h"
#include "ntxm/fifocommand.h"

#ifdef ARM9
#include "ntxm/ntxmtools.h"
#endif

#define MAX(x,y)						((x)>(y)?(x):(y))
#define LOOKUP_FREQ(note,finetune)		(linear_freq_table_lookup(MAX(0,N_FINETUNE_STEPS*(note)+(finetune))))

// This is defined in audio.h for arm7 but not for arm9
#if !defined(SOUND_FORMAT_ADPCM)
#define SOUND_FORMAT_ADPCM	(2<<29)
#define SOUND_16BIT 		(1<<29)
#define SOUND_8BIT 		(0)
#endif



/* ===================== PUBLIC ===================== */

inline u32 linear_freq_table_lookup(u32 note)
{
	/*
	// readable version
	if(note<=LINEAR_FREQ_TABLE_MAX_NOTE*N_FINETUNE_STEPS) {
		if(note>=LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS) {
			return linear_freq_table[note-LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS];
		} else {
			u32 octaveoffset = ((LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS-1)-note) / (12*N_FINETUNE_STEPS) + 1;
			u32 relnote = note % (12*N_FINETUNE_STEPS);
			#ifdef ARM7
			//CommandDbgOut("minoct:%u noteoct:%u\n",
			//	      (LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS-1)/(12*N_FINETUNE_STEPS),
			//	      note/(12*N_FINETUNE_STEPS)
			//	     );
			#endif
			#ifdef ARM9
			iprintf("%u %u\n",octaveoffset,relnote);
			#endif
			return linear_freq_table[relnote] >> octaveoffset;
		}
	}
	return 0;
	*/

	// fast version
	if(note<=LINEAR_FREQ_TABLE_MAX_NOTE*N_FINETUNE_STEPS)
	{
		if(note>=LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS) {
			return linear_freq_table[note-LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS];
		} else {
			return linear_freq_table[note % (12*N_FINETUNE_STEPS)] >>
				(((LINEAR_FREQ_TABLE_MIN_NOTE*N_FINETUNE_STEPS-1)-note) / (12*N_FINETUNE_STEPS)  + 1);
		}
	}
	return 0;
}

#ifdef ARM9

Sample::Sample(void *_sound_data, u32 _n_samples, u16 _sampling_frequency, bool _is_16_bit,
	u8 _loop, u8 _volume)
	:original_data(0), pingpong_data(0), n_samples(_n_samples), is_16_bit(_is_16_bit), loop(_loop),
	loop_start(0), loop_length(0), volume(_volume), panning(128)
{
	sound_data = _sound_data;

	memset(name, 0, SAMPLE_NAME_LENGTH);

	calcSize();
	setFormat();
	calcRelnoteAndFinetune(_sampling_frequency);

	setLoopStartAndLength(0, _n_samples);
}

Sample::Sample(const char *filename, u8 _loop, bool *_success)
	:original_data(0), pingpong_data(0), loop(_loop), loop_start(0), loop_length(0), volume(255), panning(128)
{
	sound_data = (void**)calloc(20*sizeof(void*), 1);

	if(!wav.load(filename))
	{
		printf("WAV loading failed\n");
		*_success = false;
		return;
	}

	char *smpname = strrchr(filename, '/') + 1;
	strncpy(name, smpname, SAMPLE_NAME_LENGTH);

	sound_data = wav.getAudioData();

	calcRelnoteAndFinetune( wav.getSamplingRate() );

	u8 bit_per_sample = wav.getBitPerSample();

	if(bit_per_sample == 16)
		is_16_bit = true;
	else
		is_16_bit = false;

	if(wav.getCompression() == CMP_ADPCM)
		sound_format = SOUND_FORMAT_ADPCM;
	else
		setFormat();

	n_samples = wav.getNSamples();

	/*
	if(sound_format == SOUND_FORMAT_ADPCM) {
		n_samples = wav.getNSamples() * 4; // ADPCM compresses 4 samples in 1
	} else {
		n_samples = wav.getNSamples();
	}*/

	calcSize();

	if(wav.isStereo() == true)
	{
		if(!convertStereoToMono())
		{
			printf("Stereo 2 Mono conversion failed\n");
			*_success = false;
			return;
		}
	}

	setLoopStart(0);
	setLoopLength(n_samples);

	*_success = true;
}

Sample::~Sample()
{
	if(pingpong_data != 0)
		removePingPongLoop();

	free(sound_data);
}

void Sample::saveAsWav(char *filename)
{
	wav.setCompression(0);
	wav.setNChannels(1);
	wav.setSamplingRate(LOOKUP_FREQ(rel_note+96,finetune));
	wav.setBitPerSample(is_16_bit?16:8);
	wav.setNSamples(n_samples);
	wav.setAudioData((u8*)getData());
	wav.save(filename);
}

#endif

#if defined(ARM7)

// volume_ ranges from 0-127. The value 255 means "no volume", i.e. the sample's own volume shall be used.
void Sample::play(u8 note, u8 volume_ , u8 channel)
{
	if(channel>15) return; // DS has only 16 channels!

	/*
	if(note+rel_note > N_LINEAR_FREQ_TABLE_NOTES) {
		CommandDbgOut("Freq out of range!\n");
		return;
	}
	*/

	u32 loop_bit;
	if( ( ( loop == FORWARD_LOOP ) || (loop == PING_PONG_LOOP) ) && (loop_length > 0) )
		loop_bit = SOUND_REPEAT;
	else
		loop_bit = SOUND_ONE_SHOT;

	// Add 48 to the note, because otherwise absolute_note can get negative.
	// (The minimum value of relative note is -48)
	u8 absolute_note = note + 48;

	// Choose the subsampled version. The first 12 octaves will be fine,
	// if the note is higher, choose a subsampled version.
	// Octave 12 is more of a good guess, so there could be better, more
	// reasonable values.
	u8 realnote;

	realnote = absolute_note+rel_note;

	// If a volume is given, it overrides the sample's own volume
	u8 smpvolume;
	if(volume_ == NO_VOLUME)
		smpvolume = volume / 2; // Smp volume is 0..255
	else
		smpvolume = volume_; // Channel volume is 0..127

	SCHANNEL_CR(channel) = 0;
	SCHANNEL_TIMER(channel) = SOUND_FREQ((int)LOOKUP_FREQ(realnote,finetune));
	SCHANNEL_SOURCE(channel) = (uint32)sound_data;

	if( loop == NO_LOOP )
	{
		SCHANNEL_REPEAT_POINT(channel) = 0;
		SCHANNEL_LENGTH(channel) = size >> 2;
	}
	else if( loop == FORWARD_LOOP )
	{
		SCHANNEL_REPEAT_POINT(channel) = loop_start >> 2;
		SCHANNEL_LENGTH(channel) = loop_length >> 2;
	}
	else if( loop == PING_PONG_LOOP )
	{
		SCHANNEL_REPEAT_POINT(channel) = loop_start >> 2;
		SCHANNEL_LENGTH(channel) = loop_length >> 1;
	}

	SCHANNEL_CR(channel) =
		SCHANNEL_ENABLE |
		loop_bit |
		sound_format |
		SOUND_PAN(panning/2) |
		SOUND_VOL(smpvolume);
}

void Sample::bendNote(u8 note, u8 basenote, u8 finetune, u8 channel)
{
	// Add 48 to the note, because otherwise absolute_note can get negative.
	// (The minimum value of relative note is -48)
	u8 absolute_note = note + 48;
	u8 realnote = (absolute_note+rel_note);

	SCHANNEL_TIMER(channel) = SOUND_FREQ((int)LOOKUP_FREQ(realnote,finetune));
}

#endif

u32 Sample::calcPlayLength(u8 note)
{
	u32 samples_per_second = LOOKUP_FREQ(48+note+rel_note,finetune);
	return n_samples * 1000 / samples_per_second;
}

#ifdef ARM9

void Sample::setRelNote(s8 _rel_note) {
	rel_note = _rel_note;
}

void Sample::setFinetune(s8 _finetune) {
	finetune = _finetune;
}

#endif

u8 Sample::getRelNote(void) {
	return rel_note;
}

s8 Sample::getFinetune(void) {
	return finetune;
}

u32 Sample::getSize(void)
{
	if(loop == PING_PONG_LOOP)
	{
		if(is_16_bit) {
			return original_n_samples * 2;
		} else {
			return original_n_samples;
		}
	} else {
		return size;
	}
}

u32 Sample::getNSamples(void)
{
	if(loop == PING_PONG_LOOP)
		return original_n_samples;
	else
		return n_samples;
}

void *Sample::getData(void)
{
	// sound_data is modified for the loop, but original_data points to the unmodified sound data
	if(loop == PING_PONG_LOOP)
		return original_data;
	else
		return sound_data;
}

u8 Sample::getLoop(void) {
	return loop;
}

#ifdef ARM9

bool Sample::setLoop(u8 loop_) // Set loop type. Can fail due to memory constraints
{
	if(loop_ == loop)
		return true;

	if(loop == PING_PONG_LOOP) // Switching from ping-pong to sth else
	{
		removePingPongLoop();
	}

	loop = loop_;

	if(loop_ == NO_LOOP)
	{
		setLoopStart(0);
		setLoopLength(n_samples);
	}

	if(loop_ == PING_PONG_LOOP)
	{
		// Check if enough memory is available
		u8 *testmem = (u8*)malloc(2*loop_length + size);
		if(testmem == 0)
			return false;
		free(testmem);

		setupPingPongLoop();
	}

	return true;
}

#endif

bool Sample::is16bit(void) {
	return is_16_bit;
}

u32 Sample::getLoopStart(void)
{
	if(is_16_bit)
		return loop_start / 2;
	else
		return loop_start;
}

#ifdef ARM9

void Sample::setLoopStart(u32 _loop_start)
{
	u32 loop_length_in_samples;
	if(is_16_bit)
		loop_length_in_samples = loop_length / 2;
	else
		loop_length_in_samples = loop_length;

	_loop_start = my_clamp(_loop_start, 0, n_samples-1);
	loop_length_in_samples = my_clamp(loop_length_in_samples, 0, n_samples-1 - _loop_start);

	setLoopStartAndLength(_loop_start, loop_length_in_samples);
}

void Sample::setLoopLength(u32 _loop_length)
{
	u32 loop_start_in_samples;
	if(is_16_bit)
		loop_start_in_samples = loop_start / 2;
	else
		loop_start_in_samples = loop_start;

	loop_start_in_samples = my_clamp(loop_start_in_samples, 0, n_samples-1);
	u32 ll = my_clamp(_loop_length, 0, n_samples - loop_start_in_samples);

	setLoopStartAndLength(loop_start_in_samples, ll);
}

void Sample::setLoopStartAndLength(u32 _loop_start, u32 _loop_length)
{
	// NDS fix: If loop length is 0, it won't play the beginning of the sample until the loop
	if(_loop_length == 0)
		_loop_length = 2;

	if(is_16_bit)
	{
		loop_length = _loop_length * 2;
		loop_start = _loop_start * 2;
	}
	else
	{
		loop_length = _loop_length;
		loop_start = _loop_start;
	}

	if( loop == PING_PONG_LOOP )
		updatePingPongLoop();
}

#endif

u32 Sample::getLoopLength(void)
{
	if(is_16_bit)
		return loop_length / 2;
	else
		return loop_length;
}

void Sample::setVolume(u8 vol) {
	volume = vol;
}

u8 Sample::getVolume(void) {
	return volume;
}

void Sample::setPanning(u8 pan)
{
	panning = pan;
}

u8 Sample::getPanning(void)
{
	return panning;
}

void Sample::setName(const char *name_)
{
	strncpy(name, name_, SAMPLE_NAME_LENGTH-1);
}

const char *Sample::getName(void)
{
	return name;
}

#ifdef ARM9

// Deletes the part between start sample and end sample
void Sample::delPart(u32 startsample, u32 endsample)
{
	if(endsample >= n_samples)
		endsample = n_samples-1;

	bool restore_ping_pong = false;
	if(loop == PING_PONG_LOOP)
	{
		setLoop(NO_LOOP);
		restore_ping_pong = true;
	}

	// Special case: everything is deleted
	if((startsample==0)&&(endsample==n_samples))
	{
		free(sound_data);
		n_samples = 0;
		calcSize();
		loop_start = loop_length = 0;
		return;
	}

	u32 new_n_samples = n_samples - (endsample - startsample + 1);

	u8 bps;
	if(is_16_bit) bps=2; else bps=1;

	// Copy the data after the deleted part
	if(endsample < n_samples - 1)
	{
		memmove((u8*)sound_data + startsample * bps, (u8*)sound_data + (endsample + 1) * bps, ((n_samples - 1) - endsample) * bps);
	}
	sound_data = realloc(sound_data, new_n_samples * bps);

	n_samples = new_n_samples;

	// Now everything's clear and we set the variables right
	calcSize();

	// Update Loop
	u32 loop_end = loop_start + loop_length;
	u32 start = startsample * bps;
	u32 end = endsample * bps;
	u32 del = end - start + 1;

	if(loop != NO_LOOP)
	{
		if(start < loop_end)
		{
			if(start > loop_start)
			{
				if(end < loop_end)
				{
					loop_length -= del;
				}
				else
				{
					loop_length = start - loop_start;
				}
			}
			else
			{
				if(end > loop_end)
				{
					loop_start = 0;
					loop_length = getSize();
				}
				else if(end > loop_start)
				{
					loop_length -= end - loop_start;
					loop_start = start;
				}
				else
				{
					loop_start -= del;
				}
			}
		}
	}

	u32 size = getSize();
	if(loop_start > size)
	{
		loop_start = size;
	}
	if(loop_start + loop_length > size)
	{
		 loop_length = size - loop_start;
	}
	if(loop_start == loop_length)
	{
		loop_start = 0;
		loop_length = size;
	}

	if(restore_ping_pong)
		setLoop(PING_PONG_LOOP);
}

void Sample::fadeIn(u32 startsample, u32 endsample)
{
	fade(startsample, endsample, true);

	if(loop == PING_PONG_LOOP)
		updatePingPongLoop();
}

void Sample::fadeOut(u32 startsample, u32 endsample)
{
	fade(startsample, endsample, false);

	if( loop == PING_PONG_LOOP )
		updatePingPongLoop();
}

void Sample::reverse(u32 startsample, u32 endsample)
{
	void *data = getData();
	u32 nsamples = getNSamples();

	if(endsample >= nsamples)
		endsample = nsamples-1;

	s32 offset = startsample;
	s32 length = endsample - startsample;

	// Make sure there is enough RAM for this
	void *testmem = malloc( (is_16_bit?2:1) * length);
	if(testmem == NULL)
	{
		iprintf("Not enough memory for reversing\n");
		return;
	}
	else
		free(testmem);

	// Do it!
	if(is_16_bit == true)
	{
		s16 *new_sounddata = (s16*)malloc(2 * length);
		s16 *sounddata = (s16*)(data);

		// First reverse the selected region
		for(s32 i=0;i<length;++i) {
			new_sounddata[i] = sounddata[offset+length-1-i];
		}

		// Then copy it into the sample
		for(s32 i=0;i<length;++i) {
			sounddata[offset+i] = new_sounddata[i];
		}

		free(new_sounddata);

	} else {

		s8 *new_sounddata = (s8*)malloc(length);
		s8 *sounddata = (s8*)(data);

		// First reverse the selected region
		for(s32 i=0;i<length;++i) {
			new_sounddata[i] = sounddata[offset+length-1-i];
		}

		// Then copy it into the sample
		for(s32 i=0;i<length;++i) {
			sounddata[offset+i] = new_sounddata[i];
		}

		free(new_sounddata);
	}

	// Now everything's clear and we set the variables right
	calcSize();

	if( loop == PING_PONG_LOOP )
		updatePingPongLoop();
}


void Sample::normalize(u16 percent, u32 startsample, u32 endsample)
{
	void *data = getData();

	if(is_16_bit == true)
	{
		s16 *sounddata = (s16*)(data);
		s32 smp;

		for(u32 i=startsample;i<endsample;++i) {
			smp = (s32)percent * (s32)sounddata[i] / 100;

			smp = my_clamp(smp, -32768, 32767);

			sounddata[i] = smp;
		}

	} else {

		s8 *sounddata = (s8*)(data);
		s16 smp;

		for(u32 i=startsample;i<endsample;++i) {
			smp = (s32)percent * (s32)sounddata[i] / 100;

			smp = my_clamp(smp, -128, 127);

			sounddata[i] = smp;
		}
	}

	if( loop == PING_PONG_LOOP )
		updatePingPongLoop();
}

void Sample::drawLine(int x1, int y1, int x2, int y2)
{
	x1 = my_clamp(x1, 0, n_samples-1);
	x2 = my_clamp(x2, 0, n_samples-1);
	int minval = is_16_bit?-32768:-128;
	int maxval = is_16_bit?32767:127;
	y1 = my_clamp(y1, minval, maxval);
	y2 = my_clamp(y2, minval, maxval);

	void *data = getData();
	s16 *sounddata16 = (s16*)(data);
	s8 *sounddata8 = (s8*)(data);

	// Guarantees that all lines go from left to right
	if ( x2 < x1 ) {
		int tmp = x2; x2 = x1; x1 = tmp;
		tmp = y2; y2 = y1; y1 = tmp;
	}
	s32 dy = y2 - y1, dx = x2 - x1;
	// If the gradient is greater than one we have to flip the axes
	if ( abs(dy) < dx )	{
		s32 add = 1;
		int xp = x1, yp = y1;
		if(dy < 0) {
			dy = -dy;
			add =- 1;
		}
		s32 d = 2*dy - dx;
		for(; xp<=x2; xp++)	{
			if(d > 0) {
				yp += add;
				d -= 2 * dx;
			}
			if(is_16_bit) sounddata16[xp] = yp; else sounddata8[xp] = yp;
			d += 2 * dy;
		}
	} else {
		int tmp = x1; x1 = y1; y1 = tmp;
		tmp = x2; x2 = y2; y2 = tmp;
		if ( x2 < x1 ) {
			tmp = x2; x2 = x1; x1 = tmp;
			tmp = y2; y2 = y1; y1 = tmp;
		}
		dy = y2 - y1; dx = x2 - x1;
		s32 add = 1;
		if(dy < 0) {
			dy = -dy;
			add=-1;
		}
		int xp = x1, yp = y1;
		s32 d = 2 * dy - dx;
		for(xp=x1; xp<=x2; xp++) {
			if(d > 0) {
				yp += add;
				d -= 2 * dx;
			}
			if(is_16_bit) sounddata16[yp] = xp; else sounddata8[yp] = xp;
			d += 2 * dy;
		}
	}

	if( loop == PING_PONG_LOOP )
			updatePingPongLoop();
}

#endif

/* ===================== PRIVATE ===================== */

void Sample::calcSize(void)
{
	if(is_16_bit) {
		size = n_samples*2;
	} else {
		size = n_samples;
	}
}

#ifdef ARM9

void Sample::setFormat(void) {

	// TODO ADPCM and stuff
	if(is_16_bit) {
		sound_format = SOUND_16BIT;
	} else {
		sound_format = SOUND_8BIT;
	}
}

int fncompare (const void *elem1, const void *elem2 )
{
	if ( *(u16*)elem1 < *(u16*)elem2) return -1;
	else if (*(u16*)elem1 == *(u16*)elem2) return 0;
	else return 1;
}

// Takes the sampling rate in hz and searches for FT2-compatible values for
// finetune and rel_note in the freq_table
void Sample::calcRelnoteAndFinetune(u32 freq)
{
	u16 freqpos = findClosestFreq(freq);

	finetune = freqpos%N_FINETUNE_STEPS;
	rel_note = freqpos/N_FINETUNE_STEPS - BASE_NOTE;
}

// finds the freq in the freq table that is closest to freq ^^
u16 Sample::findClosestFreq(u32 freq)
{
	// Binary search!
	bool found = false;

	u16 left = 0, right = LINEAR_FREQ_TABLE_SIZE-1, middle = (right-left)/2 + left;
	if ( (linear_freq_table_lookup(middle) <= freq) && (linear_freq_table_lookup(middle+1) >= freq) ) {
		found = true;
	} else

		while(!found) {

			if(linear_freq_table_lookup(middle) < freq) {
				left = middle+1;
			} else {
				right = middle-1;
			}

			middle = (right-left)/2 + left;

			if ( (linear_freq_table_lookup(middle) <= freq) && (linear_freq_table_lookup(middle+1) > freq) ) {
				found = true;
			}
		}

	return middle;
}

bool Sample::convertStereoToMono(void)
{
	// Check if there is enough RAM for this
	if(my_get_free_mem() < size)
	{
		printf("not enough ram for stereo 2 mono conversion\n");
		return false;
	}

	if(is_16_bit == true)
	{
		// Make a buffer for the converted sample
		s16 *tmpbuf = (s16*)malloc(size);
		s16 *src = (s16*)sound_data;

		// Convert the sample down
		s32 smp;
		for(u32 i=0; i<size/2; ++i) {
			smp = src[2*i] + src[2*i+1];
			tmpbuf[i] = smp / 2;
		}

		// Overwrite the original with the converted sample
		memcpy(sound_data, tmpbuf, size);

		// Delete the temporary buffer
		free(tmpbuf);
	}
	else
	{
		// Make a buffer for the converted sample
		s8 *tmpbuf = (s8*)malloc(size);
		s8 *src = (s8*)sound_data;

		// Convert the sample down
		s32 smp;
		for(u32 i=0; i<size/2; ++i) {
			smp = src[2*i] + src[2*i+1];
			tmpbuf[i] = smp / 2;
		}

		// Overwrite the original with the converted sample
		memcpy(sound_data, tmpbuf, size);

		// Delete the temporary buffer
		free(tmpbuf);
	}
	return true;
}

void Sample::fade(u32 startsample, u32 endsample, bool in)
{
	void *data = getData();
	u32 nsamples = getNSamples();

	if(endsample >= nsamples)
		endsample = nsamples-1;

	s32 offset = startsample;
	s32 length = endsample - startsample + 1;

	if(is_16_bit == true)
	{
		s16 *sounddata = (s16*)(data);

		if(in==true) {
			for(s32 i=0;i<length;++i) {
				sounddata[offset+i] = (((i * 1024) / length) * sounddata[offset+i]) / 1024;
			}
		} else {
			for(s32 i=0;i<length;++i) {
				sounddata[offset+i] = ((((length-i) * 1024) / length) * sounddata[offset+i]) / 1024;
			}
		}
	}
	else
	{
		s8 *sounddata = (s8*)(data);

		if(in==true) {
			for(s32 i=0;i<length;++i) {
				sounddata[offset+i] = i * sounddata[offset+i] / length;
			}
		} else {
			for(s32 i=0;i<length;++i) {
				sounddata[offset+i] = (length-i) * sounddata[offset+i] / length;
			}
		}
	}

	// Now everything's clear and we set the variables right
	calcSize();

	if( loop == PING_PONG_LOOP )
		updatePingPongLoop();
}

void Sample::setupPingPongLoop(void)
{
	original_data = sound_data; // "Backup"
	original_n_samples = n_samples;

	u32 original_size = size;

	pingpong_data = realloc(pingpong_data, original_size + loop_length);

	// Copy sound data until loop end
	memcpy(pingpong_data, original_data, loop_start + loop_length);

	// Copy reverse loop
	if(is_16_bit)
	{
		s16 *orig = (s16*)original_data;
		s16 *pp = (s16*)pingpong_data;
		u32 pos = (loop_start + loop_length) / 2;

		for(u32 i=0; i<loop_length/2; ++i)
			pp[pos+i] = orig[pos-i];
	}
	else
	{
		s8 *orig = (s8*)original_data;
		s8 *pp = (s8*)pingpong_data;
		u32 pos = loop_start + loop_length;

		for(u32 i=0; i<loop_length; ++i)
			pp[pos+i] = orig[pos-i];
	}

	// Copy rest
	u32 pos = loop_start + loop_length;

	memcpy((u8*)pingpong_data+ pos + loop_length, (u8*)original_data + pos, original_size - pos);

	// Set as new sound data
	sound_data = pingpong_data;

	if(is_16_bit)
		n_samples = (original_size + loop_length) / 2;
	else
		n_samples = original_size + loop_length;

	calcSize();

	DC_FlushAll();
}

void Sample::removePingPongLoop(void)
{
	free(pingpong_data);
	pingpong_data = 0;

	sound_data = original_data;
	n_samples = original_n_samples;

	if(is_16_bit)
		size = n_samples*2;
	else
		size = n_samples;

	calcSize();

	DC_FlushAll();
}

void Sample::updatePingPongLoop(void)
{
	if(pingpong_data != 0)
		removePingPongLoop();
	setupPingPongLoop();
}

#endif

