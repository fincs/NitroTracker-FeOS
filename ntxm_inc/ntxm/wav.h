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

#ifndef WAV_H
#define WAV_H

#define CMP_PCM		0
#define CMP_ADPCM	1

#include <nds.h>

/*

A class that imeplements wav reading
and writing exactly as far as we need
it and not one single bit more.

It supports
- 8/16 Bit
- arbitrary sampling rate
- raw PCM/IMA ADPCM

*/

class Wav {
	public:
		Wav();
		~Wav();
		bool load(const char *filename);
		bool save(const char *filename);
		
		u8 *getAudioData(void)    { return audio_data_; }
		u32 getNSamples(void)     { return (n_channels_==2)?n_samples_/2:n_samples_; }
		u16 getSamplingRate(void) { return sampling_rate_; }
		bool isStereo(void)       { return n_channels_ == 2; }
		u8 getBitPerSample(void)  { return bit_per_sample_; }
		u8 getCompression(void)   { return compression_; }

		void setCompression(u8 compression)     { compression_ = compression; }
		void setNChannels(u8 n_channels)        { n_channels_ = n_channels; }
		void setSamplingRate(u16 sampling_rate) { sampling_rate_ = sampling_rate; }
		void setBitPerSample(u8 bit_per_sample) { bit_per_sample_ = bit_per_sample; }
		void setNSamples(u32 n_samples)         { n_samples_ = n_samples; }
		void setAudioData(u8 *audio_data)       { audio_data_ = audio_data; }
		
	private:
		u8 compression_;
		u8 n_channels_;
		u16 sampling_rate_;
		u8 bit_per_sample_;
		u32 n_samples_;
		u8 *audio_data_;
};

#endif
