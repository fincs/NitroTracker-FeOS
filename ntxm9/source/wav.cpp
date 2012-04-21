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
#include <stdio.h>
#include <string.h>
#include <nds.h>

/*
#ifdef ARM9
#include <fat.h>
#endif
*/

#include "ntxm/wav.h"

#ifdef ARM9
#include "ntxm/ntxmtools.h"
#endif

/* ===================== PUBLIC ===================== */

Wav::Wav()
	:compression_(CMP_PCM), n_channels_(1), sampling_rate_(22050), bit_per_sample_(8),
	n_samples_(0), audio_data_(0)
{

}

Wav::~Wav() {

}

bool Wav::load(const char *filename)
{
#if defined(ARM9)
	// Init

	FILE *fileh;

	fileh = fopen(filename, "r");

	if((s32)fileh == -1)
		return false;

	// Check if the file is not too big
	fseek(fileh, 0, SEEK_END);
	u32 filesize = ftell(fileh);

	u32 free_ram = my_get_free_mem();

	if( filesize > free_ram )
	{
		fclose(fileh);
#ifdef DEBUG
		iprintf("file too big for ram\n");
#endif
		return false;
	}

	if(filesize == 0)
	{
		fclose(fileh);
#ifdef DEBUG
		iprintf("0-byte file!\n");
#endif
		return false;
	}

	fseek(fileh, 0, SEEK_SET);

	char *buf = (char*)malloc(5);
	memset(buf,0,5);

	// RIFF header
	fread(buf, 1, 4, fileh);

	if(strcmp(buf,"RIFF")!=0) {
		fclose(fileh);
		return false;
	}

	u32 riff_size;
	fread(&riff_size, 4, 1, fileh);

	// WAVE header
	fread(buf, 1, 4, fileh);
	if(strcmp(buf,"WAVE")!=0) {
		fclose(fileh);
		return false;
	}

	// fmt chunk
	fread(buf, 1, 4, fileh);
	if(strcmp(buf,"fmt ")!=0) {
		fclose(fileh);
		return false;
	}

	u32 fmt_chunk_size;
	fread(&fmt_chunk_size, 4, 1, fileh);

	u16 compression_code;
	fread(&compression_code, 2, 1, fileh);

	// 1 : pcm, 17 : ima adpcm
	if(compression_code == 1) {
		compression_ = CMP_PCM;
	} /*else if(compression_code == 17) {
		compression_ = CMP_ADPCM;
	}*/ else {
		fclose(fileh);
		return false;
	}

	u16 n_channels; // They really thought they were cool when making this 16 bit.
	fread(&n_channels, 2, 1, fileh);

	if(n_channels > 2) {
		fclose(fileh);
		return false;
	} else {
		n_channels_ = n_channels;
	}

	u32 sampling_rate;
	fread(&sampling_rate, 4, 1, fileh);

	sampling_rate_ = sampling_rate;

	u32 avg_bytes_per_sec; // We don't need this
	fread(&avg_bytes_per_sec, 4, 1, fileh);

	u16 block_align; // We don't need this
	fread(&block_align, 2, 1, fileh);

	u16 bit_per_sample;
	fread(&bit_per_sample, 2, 1, fileh);
	if((bit_per_sample==8)||(bit_per_sample==16)) {
		bit_per_sample_ = bit_per_sample;
	} else {
        fclose(fileh);
        return false;
    }

	// Skip extra bytes
	fseek(fileh, fmt_chunk_size - 16, SEEK_CUR);

	fread(buf, 1, 4, fileh);

	// Skip forward to the data chunk
	while((!feof(fileh))&&(strcmp(buf,"data")!=0)) {
		u32 chunk_size;
		fread(&chunk_size, 4, 1, fileh);
		fseek(fileh, chunk_size, 1);
		fread(buf, 1, 4, fileh);
	}

	if(feof(fileh)) {
		fclose(fileh);
		return false;
	}

	// data chunk
	u32 data_chunk_size;
	fread(&data_chunk_size, 4, 1, fileh);

	u8 sample_size = bit_per_sample_/8;
	if(compression_ == CMP_PCM) {
		n_samples_ = data_chunk_size / sample_size;
	} else {
		n_samples_ = data_chunk_size * 2 * sample_size;
	}

	audio_data_ = (u8*)malloc(data_chunk_size);
	//memset(audio_data_, 0, data_chunk_size);

	if(audio_data_ == 0) {
#ifdef DEBUG
		iprintf("Could not alloc mem for wav.\n");
#endif
		free(buf);
		fclose(fileh);
		return false;
	}

	// Read the data
	fread(audio_data_, data_chunk_size, 1, fileh);

	// Convert 8 bit samples from unsigned to signed
	if(bit_per_sample == 8) {
		s8* signed_data = (s8*)audio_data_;
		for(u32 i=0; i<data_chunk_size; ++i) {
			signed_data[i] = audio_data_[i] - 128;
		}
	}

	// Finish up
	free(buf);

	fclose(fileh);

#endif
	return true;
}

bool Wav::save(const char *filename)
{
	FILE *fileh = fopen(filename, "w");
	if(fileh == NULL)
		return false;

	// RIFF header
	fwrite("RIFF", 1, 4, fileh);

	u32 data_chunk_size = bit_per_sample_ / 8 * n_channels_ * n_samples_;

	u32 riff_size = data_chunk_size + 32;
	fwrite(&riff_size, 4, 1, fileh);

	// WAVE header
	fwrite("WAVE", 1, 4, fileh);

	// fmt chunk
	fwrite("fmt ", 1, 4, fileh);

	u32 fmt_chunk_size = 16;
	fwrite(&fmt_chunk_size, 4, 1, fileh);

	u16 compression_code = 1; // Always PCM
	fwrite(&compression_code, 2, 1, fileh);

	u16 nch = n_channels_;
	fwrite(&nch, 2, 1, fileh);

	u32 sampling_rate = sampling_rate_;
	fwrite(&sampling_rate, 4, 1, fileh);

	u32 avg_bytes_per_sec = sampling_rate_ * bit_per_sample_ / 8 * n_channels_;
	fwrite(&avg_bytes_per_sec, 4, 1, fileh);

	u16 block_align = bit_per_sample_ / 8 * n_channels_;
	fwrite(&block_align, 2, 1, fileh);

	u16 bit_per_sample = bit_per_sample_;
	fwrite(&bit_per_sample, 2, 1, fileh);

	// data chunk
	fwrite("data", 1, 4, fileh);

	fwrite(&data_chunk_size, 4, 1, fileh);

#ifdef DEBUG
	printf("rate: %u\ndata: %u\n", sampling_rate_, data_chunk_size);
#endif

	if(bit_per_sample == 8)
	{
		// Convert from unsigned to signed
		s8 smp = 0;
		for(u32 i=0; i<data_chunk_size; ++i)
		{
			smp = (s8)((s16)audio_data_[i] - 128);
			fwrite( &smp, 1, 1, fileh );
		}
	}
	else if(bit_per_sample == 16)
	{
		u16 *audio = (u16*)audio_data_;
		fwrite(audio, data_chunk_size, 1, fileh);
	}

	fclose(fileh);

	return true; // Hehe
}

/* ===================== PRIVATE ===================== */
