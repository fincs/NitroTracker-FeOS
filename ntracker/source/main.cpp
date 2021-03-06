/*
 * NitroTracker - An FT2-style tracker for the Nintendo DS
 *
 *                                by Tobias Weyand (0xtob)
 *
 * http://nitrotracker.tobw.net
 * http://code.google.com/p/nitrotracker
 */

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#define DEBUG
//#define WIFIDEBUG
//#define WIFI

#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <tobkit/tobkit.h>

// Special tracker widgets
#include "tobkit/nitrotracker_logo.h"
#include "tobkit/recordbox.h"
#include "tobkit/numbersliderrelnote.h"
#include "tobkit/envelope_editor.h"
#include "tobkit/sampledisplay.h"
#include "tobkit/patternview.h"
#include "tobkit/normalizebox.h"

#include <ntxm/ntxm9.h>
#include <ntxm/fifocommand.h>
#include <ntxm/song.h>
#include <ntxm/xm_transport.h>
#include <ntxm/wav.h>
#include <ntxm/instrument.h>
#include <ntxm/sample.h>
#include <ntxm/ntxmtools.h>
#include "state.h"
#include "settings.h"
#include "tools.h"

#include "icon_disk_bin.h"
#include "icon_song_bin.h"
#include "icon_sample_bin.h"
#include "icon_wrench_bin.h"
#include "icon_trumpet_bin.h"

#include "icon_flp_bin.h"
#include "icon_pause_bin.h"
#include "icon_play_bin.h"
#include "icon_stop_bin.h"

#include "sampleedit_control_icon_bin.h"
#include "sampleedit_wave_icon_bin.h"
#include "sampleedit_loop_icon_bin.h"
#include "sampleedit_fadein_bin.h"
#include "sampleedit_fadeout_bin.h"
#include "sampleedit_all_bin.h"
#include "sampleedit_none_bin.h"
#include "sampleedit_del_bin.h"
#include "sampleedit_reverse_bin.h"
#include "sampleedit_record_bin.h"
#include "sampleedit_normalize_bin.h"
#include "sampleedit_draw_bin.h"
#include "sampleedit_draw_small_bin.h"

// libnds copypasta BEGIN!

/*! \brief A macro which returns a u16* pointer to background map ram (Main Engine) */
#define BG_MAP_RAM(base)		((u16*)(((base)*0x800) + 0x06000000))
/*! \brief A macro which returns a u16* pointer to background tile ram (Main Engine) */
#define BG_TILE_RAM(base)		((u16*)(((base)*0x4000) + 0x06000000))
/*! \brief A macro which returns a u16* pointer to background graphics memory ram (Main Engine) */
#define BG_BMP_RAM(base)		((u16*)(((base)*0x4000) + 0x06000000))

/* A macro which returns a u16* pointer to background tile ram (Main Engine)
use BG_TILE_RAM unless you really can't */
#define CHAR_BASE_BLOCK(n)			(((n)*0x4000)+ 0x06000000)
/* A macro which returns a u16* pointer to background Map ram (Main Engine)
use BG_MAP_RAM unless you really can't*/
#define SCREEN_BASE_BLOCK(n)		(((n)*0x800) + 0x06000000)

/*! \brief A macro which returns a u16* pointer to background map ram (Sub Engine) */
#define BG_MAP_RAM_SUB(base)	((u16*)(((base)*0x800) + 0x06200000))
/*! \brief A macro which returns a u16* pointer to background tile ram (Sub Engine) */
#define BG_TILE_RAM_SUB(base)	((u16*)(((base)*0x4000) + 0x06200000))
/*! \brief A macro which returns a u16* pointer to background graphics ram (Sub Engine) */
#define BG_BMP_RAM_SUB(base)	((u16*)(((base)*0x4000) + 0x06200000))

/* A macro which returns a u16* pointer to background Map ram (Sub Engine)
use BG_MAP_RAM_SUB unless you really can't */
#define SCREEN_BASE_BLOCK_SUB(n)	(((n)*0x800) + 0x06200000)
/* A macro which returns a u16* pointer to background tile ram (Sub Engine)
use BG_TILE_RAM_SUB unless you really can't */
#define CHAR_BASE_BLOCK_SUB(n)		(((n)*0x4000)+ 0x06200000)

#define REG_BLDCNT     (*(vu16*)0x04000050)
#define REG_BLDY	   (*(vu16*)0x04000054)
#define REG_BLDALPHA   (*(vu16*)0x04000052)

#define REG_BLDCNT_SUB     (*(vu16*)0x04001050)
#define REG_BLDALPHA_SUB   (*(vu16*)0x04001052)
#define REG_BLDY_SUB	   (*(vu16*)0x04001054)

#define DISPLAY_BG0_ACTIVE    (1 << 8)
#define DISPLAY_BG1_ACTIVE    (1 << 9)
#define DISPLAY_BG2_ACTIVE    (1 << 10)
#define DISPLAY_BG3_ACTIVE    (1 << 11)

#define BLEND_NONE         (0<<6)
#define BLEND_ALPHA        (1<<6)
#define BLEND_FADE_WHITE   (2<<6)
#define BLEND_FADE_BLACK   (3<<6)

#define BLEND_SRC_BG0      (1<<0)
#define BLEND_SRC_BG1      (1<<1)
#define BLEND_SRC_BG2      (1<<2)
#define BLEND_SRC_BG3      (1<<3)
#define BLEND_SRC_SPRITE   (1<<4)
#define BLEND_SRC_BACKDROP (1<<5)

#define BLEND_DST_BG0      (1<<8)
#define BLEND_DST_BG1      (1<<9)
#define BLEND_DST_BG2      (1<<10)
#define BLEND_DST_BG3      (1<<11)
#define BLEND_DST_SPRITE   (1<<12)
#define BLEND_DST_BACKDROP (1<<13)

// ---

#define REPEAT_FREQ	10

#define FRONT_BUFFER	0
#define BACK_BUFFER	1

#define FILETYPE_SONG	0
#define FILETYPE_SAMPLE	1
#define FILETYPE_INST	2

touchPosition touch;
u8 frame = 0;

u8 active_buffer = FRONT_BUFFER;

u16 *main_vram_front, *main_vram_back, *sub_vram;

bool typewriter_active = false;
volatile bool need_to_redraw = false;

u16 keys_that_are_repeated = KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT;

// Make the key botmasks variable for switching handedness
u16 mykey_LEFT = KEY_LEFT, mykey_UP = KEY_UP, mykey_RIGHT = KEY_RIGHT, mykey_DOWN = KEY_DOWN,
	mykey_A = KEY_A, mykey_B = KEY_B, mykey_X = KEY_X, mykey_Y = KEY_Y, mykey_L = KEY_L, mykey_R = KEY_R;

GUI *gui;

// <Misc GUI>
	Button *buttonrenameinst, *buttonrenamesample, *buttontest, *buttonstopnote, *buttonemptynote, *buttondelnote, *buttoninsnote2,
		*buttondelnote2, *buttoninsnote;
	BitButton *buttonswitchsub, *buttonplay, *buttonstop, *buttonpause;
	CheckBox *cbloop;
	ToggleButton *tbrecord, *tbmultisample;
	Label *labeladd, *labeloct;
	NumberBox *numberboxadd, *numberboxoctave;
	Piano *kb;
	ListBox *lbinstruments, *lbsamples;
	TabBox *tabbox;
	Pixmap *pixmaplogo;
// </Misc GUI>

// <Disk op gui>
	Label *labelitem, *labelFilename, *labelramusage_disk;
	RadioButton *rbsong, *rbsample, *rbinst;
	RadioButton::RadioButtonGroup *rbgdiskop;
	Button *buttonsave, *buttonload, *buttonchangefilename, *buttonnew;
	FileSelector *fileselector;
	MemoryIndicator *memoryiindicator_disk;
	CheckBox *cbsamplepreview;
// </Disk op gui>

// <Song Gui>
	Label *labeltempo, *labelbpm, *labelptns, *labelptnlen,
		*labelchannels, *labelsongname, *labelrestartpos, *labelramusage;
	ListBox *lbpot;
	Button *buttonpotup, *buttonpotdown, *buttoncloneptn,
		*buttonmorechannels, *buttonlesschannels, *buttonzap, *buttonrenamesong;
	NumberBox *nbtempo;
	NumberSlider *nsptnlen, *nsbpm, *nsrestartpos;
	MemoryIndicator *memoryiindicator;
// </Song Gui>

// <Sample Gui>
	RecordBox *recordbox;
	NormalizeBox *normalizeBox;
	SampleDisplay *sampledisplay;
	HTabBox *sampletabbox;

	Label *labelsamplevolume, *labelrelnote, *labelfinetune, *labelpanning;
	NumberSlider *nssamplevolume, *nsfinetune, *nspanning;
	NumberSliderRelNote *nsrelnote;

	Label *labelsampleedit_select, *labelsampleedit_edit, *labelsampleedit_record;
	BitButton *buttonsmpfadein, *buttonsmpfadeout, *buttonsmpselall, *buttonsmpselnone, *buttonsmpseldel,
		*buttonsmpreverse, *buttonrecord, *buttonsmpnormalize;

	GroupBox *gbsampleloop;
	RadioButton::RadioButtonGroup *rbg_sampleloop;
	RadioButton *rbloop_none, *rbloop_forward, *rbloop_pingpong;
	CheckBox *cbsnapto0xing;

	ToggleButton *buttonsmpdraw;
// </Sample Gui>

// <Instrument Gui>
	EnvelopeEditor *volenvedit;
	Button *btnaddenvpoint, *btndelenvpoint, *btnenvzoomin, *btnenvzoomout, *btnenvdrawmode;
	ToggleButton *tbmapsamples;
	CheckBox *cbvolenvenabled;
// </Instrument Gui>

// <Settings Gui>
	RadioButton::RadioButtonGroup *rbghandedness;
	RadioButton *rblefthanded, *rbrighthanded;
	GroupBox *gbhandedness, *gbdsmw;
	CheckBox *cbdsmwsend, *cbdsmwrecv;
	Button *btndsmwtoggleconnect;
// </Settings Gui>

// <Main Screen>
	Button *buttonins, *buttondel, *buttonstopnote2, *buttoncolselect, *buttonemptynote2, *buttonunmuteall;
	BitButton *buttonswitchmain;
	Button *buttoncut, *buttoncopy, *buttonpaste, *buttonsetnotevol;
	PatternView *pv;
	NumberSlider *nsnotevolume;
	Label *labelmute, *labelnotevol;
// </Main Screen>

// <Things that suddenly pop up>
	Typewriter *tw;
	MessageBox *mb;
// </Things that suddenly pop up>

u16 *b1n, *b1d;
int lastx, lasty;
Song *song;
State *state;
Settings *settings;
XMTransport xm_transport;

Cell **clipboard = 0;
u16 clipboard_width = 0, clipboard_height = 0;

u8 dsmw_lastnotes[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
u8 dsmw_lastchannels[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool fastscroll = false;

uint16* map;

char **arm7debugstrs;

// TODO: Make own class for tracker control and remove forward declarations
void handleButtons(u16 buttons, u16 buttonsheld);
void HandleTick(void);
void handlePotPosChangeFromSong(u16 newpotpos);
void drawMainScreen(void);
void redrawSubScreen(void);
void showMessage(const char *msg);
void deleteMessageBox(void);
void stopPlay(void);

#ifdef DEBUG
void saveScreenshot(void);
void dumpSample(void);
#endif

void clearMainScreen(void)
{
	u16 col = settings->getTheme()->col_dark_bg;
	u32 colcol = col | col << 16;
	dmaFillHalfWords(colcol, main_vram_front, 192*256*2);
	dmaFillHalfWords(colcol, main_vram_back, 192*256*2);
}

void clearSubScreen(void)
{
	u16 col = settings->getTheme()->col_dark_bg;
	u32 colcol = col | col << 16;
	// Fill the bg with the bg color except for the place where the keyboard is
	dmaFillHalfWords(colcol, sub_vram, 153*256*2);
	for(int y=154;y<192;++y)
	{
		for(int x=0;x<224;++x)
			sub_vram[256*y+x] = 0;
		for(int x=224;x<256;++x)
			sub_vram[256*y+x] = colcol;
	}
}

void drawSampleNumbers(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == NULL)
	{
		for(u8 key=0; key<24; ++key)
		{
			kb->setKeyLabel(key, '0');
		}
		return;
	}

	char label;
	u8 note, sample_id;
	for(u8 key=0; key<24; ++key)
	{
		note = state->basenote + key;
		sample_id = inst->getNoteSample(note);
		sprintf(&label, "%x", sample_id);

		kb->setKeyLabel(key, label);
	}
}

void updateKeyLabels(void)
{
	kb->hideKeyLabels();
	if(lbsamples->is_visible() == true)
	{
		drawSampleNumbers();
		kb->showKeyLabels();
	}
}

void handleNoteStroke(u8 note)
{
	// If we are recording
	if(state->recording == true)
	{
		// Check if this was an empty- or stopnote
		if((note==EMPTY_NOTE)||(note==STOP_NOTE)) {
			// Because then we don't use the offset since they have fixed indices
			song->clearCell(&(song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->row]));
			if(note==STOP_NOTE)
				song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->row].note = note;
		} else {
			song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->row].note = state->basenote + note;
			song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->row].instrument = state->instrument;
		}

		// Check if we are not at the bottom and only scroll down as far as possible
		if((state->playing == false)||(state->pause==true))
		{
			state->row += state->add;
			state->row %= song->getPatternLength(song->getPotEntry(state->potpos));
		}

		// Redraw
		drawMainScreen();
		//////////////////
		DC_FlushAll();
		//////////////////
	}

	// If we are in sample mapping mode, map the pressed key to the selected sample for the current instrument
	if((state->map_samples == true)&&(note!=EMPTY_NOTE)&&(note!=STOP_NOTE))
	{
		Instrument *inst = song->getInstrument(state->instrument);
		if(inst != NULL)
		{
			inst->setNoteSample(state->basenote + note, state->sample);
			DC_FlushAll();
		}

		char label;
		sprintf(&label, "%x", state->sample);
		kb->setKeyLabel(note, label);
	}

	// Play the note
	if((note!=EMPTY_NOTE)&&(note!=STOP_NOTE))
	{
		// Send "play inst" command
		CommandPlayInst(state->instrument, state->basenote + note, 255, 255); // channel==255 -> search for free channel

#ifdef WIFI
		u8 midichannel = state->instrument % 16;
		if( (state->dsmi_connected) && (state->dsmi_send) )
			dsmi_write(NOTE_ON | midichannel, state->basenote + note, 127);
#endif
	}
}

void handleNoteRelease(u8 note)
{
	CommandStopInst(255);

#ifdef WIFI
	u8 midichannel = state->instrument % 16;
	if( (state->dsmi_connected) && (state->dsmi_send) )
		dsmi_write(NOTE_OFF | midichannel, state->basenote + note, 127);
#endif
}

void updateSampleList(Instrument *inst)
{
	if(inst == NULL)
	{
		for(u8 i=0; i<MAX_INSTRUMENT_SAMPLES; ++i)
		{
			lbsamples->set(i, "");
		}
	}
	else
	{
		Sample *sample;
		char *str=(char*)malloc(255);
		for(u8 i=0; i<MAX_INSTRUMENT_SAMPLES; ++i)
		{
			memset(str, 0, 255);
			sample = inst->getSample(i);
			if(sample != NULL)
			{
				strcpy(str, sample->getName());
				lbsamples->set(i, str);
			} else {
				lbsamples->set(i, "");
			}
		}
		free(str);
	}
}

void sampleChange(Sample *smp)
{
	if(smp == NULL)
	{
		sampledisplay->setSample(NULL);
		nssamplevolume->setValue(0);
		nspanning->setValue(64);
		nsrelnote->setValue(0);
		nsfinetune->setValue(0);
		rbg_sampleloop->setActive(0);

		return;
	}

	sampledisplay->setSample(smp);
	sampledisplay->hideLoopPoints();
	nssamplevolume->setValue( (smp->getVolume()+1)/4 );
	nspanning->setValue(smp->getPanning()/2);
	nsrelnote->setValue(smp->getRelNote());
	nsfinetune->setValue(smp->getFinetune());

	if( (smp->getLoop() >= 0) && (smp->getLoop() <= 2) )
		rbg_sampleloop->setActive(smp->getLoop());
	else
		rbg_sampleloop->setActive(0);
}

void volEnvSetInst(Instrument *inst)
{
	if(inst == 0)
	{
		volenvedit->setZoomAndPos(0, 0);
		volenvedit->setPoints(0, 0, 0);
		volenvedit->pleaseDraw();
	}
	else
	{
		u16 *xs, *ys;
		u16 n = inst->getVolumeEnvelope(&xs, &ys);
		volenvedit->setZoomAndPos(2, 0);
		volenvedit->setPoints(xs, ys, n);
		volenvedit->pleaseDraw();
	}
}

void handleSampleChange(u16 newsample)
{
	state->sample = newsample;

	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	if(inst == 0)
		return;

	Sample *smp = inst->getSample(newsample);
	sampleChange(smp);
}
void handleInstChange(u16 newinst)
{

	state->instrument = newinst;

	lbsamples->select(0);

	Instrument *inst = song->getInstrument(newinst);
	updateSampleList(inst);
	volEnvSetInst(inst);
	updateKeyLabels();
	if(inst != NULL)
	{
		cbvolenvenabled->setChecked(inst->getVolEnvEnabled());
		handleSampleChange(0);
	}
	else
	{
		sampleChange(NULL);
		return;
	}
}

void updateLabelChannels(void)
{
	char *labelstr = (char*)malloc(8);
	sprintf(labelstr, "chn: %2d", song->getChannels());
	labelchannels->setCaption(labelstr);
	free(labelstr);
}

void updateTempoAndBpm(void)
{
	nsbpm->setValue(song->getBPM());
	nbtempo->setValue(song->getTempo());
}

void setSong(Song *newsong)
{
	song = newsong;

	CommandSetSong(song);

	state->resetSong();

	pv->setSong(song);

	// Clear sample display
	sampledisplay->setSample(0);

	// Update POT
	lbpot->clear();
	u8 potentry;
	char *str=(char*)malloc(3);
	str[2]=0;
	for(u8 i=0;i<song->getPotLength();++i) {
		potentry = song->getPotEntry(i);
		sprintf(str, "%2x", potentry);
		lbpot->add(str);
	}
	free(str);

	// Update instrument list
	Instrument *inst;
	str=(char*)malloc(255);
	for(u8 i=0;i<MAX_INSTRUMENTS;++i)
	{
		memset(str, 0, 255);
		inst = song->getInstrument(i);
		if(inst!=NULL) {
			strcpy(str, inst->getName());
			lbinstruments->set(i, str);
		} else {
			lbinstruments->set(i, "");
		}
	}

	lbinstruments->select(0);

	updateSampleList(song->getInstrument(0));

	inst = song->getInstrument(0);
	if(inst != 0)
		sampleChange(inst->getSample(0));

	volEnvSetInst(song->getInstrument(0));

	inst = song->getInstrument(0);
	if(inst != 0)
		cbvolenvenabled->setChecked(inst->getVolEnvEnabled());

	updateLabelChannels();
	updateTempoAndBpm();
	nsptnlen->setValue(song->getPatternLength(song->getPotEntry(state->potpos)));
	nsrestartpos->setValue(song->getRestartPosition());
	cbloop->setChecked(false);

	numberboxadd->setValue(state->add);
	numberboxoctave->setValue(state->basenote/12);

	tbrecord->setState(false);

	inst = song->getInstrument(state->instrument);
	if(inst != NULL) {
		sampledisplay->setSample(inst->getSample(state->sample));
	}

	lbsamples->select(0);

	//memset(str, 0, 255);
	strcpy(str, song->getName());
	labelsongname->setCaption(str);

	free(str);

	//gui->draw();
	drawMainScreen();
}

bool loadSample(const char *filename_with_path)
{
	const char *filename = strrchr(filename_with_path, '/') + 1;
#ifdef DEBUG
	iprintf("file: %s %s\n",filename_with_path, filename);
#endif

	bool load_success;
	Sample *newsmp = new Sample(filename_with_path, false, &load_success);
	if(load_success == false)
	{
		delete newsmp;
		return false;
	}

	u8 instidx = lbinstruments->getidx();
	u8 smpidx = lbsamples->getidx();

	//
	// Create the instrument if it doesn't exist
	//
	Instrument *inst = song->getInstrument(instidx);
	if(inst == 0)
	{
		char *instname = (char*)malloc(MAX_INST_NAME_LENGTH+1);
		strncpy(instname, filename, MAX_INST_NAME_LENGTH);

		inst = new Instrument(instname);
		song->setInstrument(instidx, inst);

		free(instname);

		lbinstruments->set(state->instrument, song->getInstrument(state->instrument)->getName());
	}

	//
	// Insert new sample (if there's already one, it's deleted)
	//
	inst->setSample(smpidx, newsmp);

	lbsamples->set(lbsamples->getidx(), newsmp->getName());

	// Rename the instrument if we are in "single sample mode"
	if(!lbsamples->is_visible())
	{
		inst->setName(newsmp->getName());
		lbinstruments->set(state->instrument, song->getInstrument(state->instrument)->getName());
	}

	sampleChange(newsmp);

	DC_FlushAll();

	return true;
}

void loadSong(const char* n)
{
	stopPlay();

	delete song; // For christs sake do some checks before deleting the song!!
	pv->unmuteAll();

	mb = new MessageBox(&sub_vram, "one moment", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	Song *newsong = NULL;

	u16 err;

	if (my_file_exists(n))
		err = xm_transport.load(n, &newsong);
	else
		err = 0xFFFF;

	deleteMessageBox();

	if(err) {
		// Emergency: set up an empty song
		if (newsong) delete newsong;
		newsong = new Song(10, 125);
	}

	setSong(newsong);

	DC_FlushAll();

	if(err) {
		showMessage(err != 0xFFFF ? xm_transport.getError(err) : "file doesn't exist!");
	}
}

void handleLoad(void)
{
	// Debug function for fun and profit. Or is it?
	File *file = fileselector->getSelectedFile();
	if((file==0)||(file->is_dir == true)) {
#ifdef DEBUG
		printf("File fail #1\n");
#endif
		return;
	}

	// Make the extension lowercase
	char ext[4] = {0};
	//file->name.substr(file->name.length()-3,3).copyto(ext, 3);
	//lowercase(ext);
	const char* n = file->name.c_str();
	int l = strlen(n);
	ext[0] = n[l-3];
	ext[1] = n[l-2];
	ext[2] = n[l-1];
	ext[3] = n[l];
#ifdef DEBUG
	printf("ext: %s\n", ext);
#endif

	if(strcmp(ext, ".xm")==0)
	{
		loadSong(file->name_with_path.c_str());
	}
	else if(strcmp(ext, "wav")==0)
	{
		bool success = loadSample(file->name_with_path.c_str());

		if(success == false) {
			showMessage("wav loading failed");
		}
	}

	memoryiindicator_disk->pleaseDraw();
}

// Reads filename and path from fileselector and saves the file
void saveFile(void)
{
	stopPlay();

	char *filename = labelFilename->getCaption();
	std::string dir = fileselector->getDir();
	const char *path = dir.c_str();
	char *pathfilename = (char*)malloc( strlen(path)+strlen(filename)+1 );
	//memset(pathfilename, 0, strlen(path)+strlen(filename)+1);
	strcpy(pathfilename, path);
	strcpy(pathfilename+strlen(path), filename);

#ifdef DEBUG
	printf("path %s ...\n", path);
	printf("pathfilename %s ...\n", pathfilename);
	iprintf("saving %s ...\n", filename);
#endif

	mb = new MessageBox(&sub_vram, "one moment", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	int err = 0;
	if(rbsong->getActive() == true) // Save the song
	{
		if(song != 0) {
			err = xm_transport.save(pathfilename, song);
		}
	}
	else if(rbsample->getActive() == true) // Save the sample
	{
		song->getInstrument(state->instrument)->getSample(state->sample)->saveAsWav(pathfilename);
	}

	deleteMessageBox();

#ifdef DEBUG
	iprintf("done\n");
#endif

	free(pathfilename);

	if(err > 0)
	{
		showMessage(xm_transport.getError(err));
	}
}

void mbOverwrite(void) {

	deleteMessageBox();
	saveFile();
}

void handleSave(void)
{
	// sporadic filename sanity check
	char *filename = labelFilename->getCaption();
	if(strlen(filename)==0) {
		showMessage("No filename, stupid!");
		return;
	}

	if(rbsample->getActive() == true) // Sample sanity checks
	{
		Instrument *inst = song->getInstrument(state->instrument);
		if(inst == NULL)
		{
			showMessage("Empty instrument!");
			return;
		}
		Sample *smp = inst->getSample((state->sample));
		if(smp == NULL)
		{
			showMessage("Empty sample!");
			return;
		}
	}

	// construct filename with path
	std::string dir = fileselector->getDir();
	const char *path = dir.c_str();

	char *pathfilename = (char*)malloc(strlen(path)+strlen(filename)+1);
	//memset(pathfilename, 0, strlen(path)+strlen(filename)+1);
	strcpy(pathfilename, path);
	strcpy(pathfilename+strlen(path), filename);

	// Check if file already exists
	if(my_file_exists(pathfilename))
	{
		mb = new MessageBox(&sub_vram, "overwrite file", 2, "yes", mbOverwrite, "no", deleteMessageBox);
		gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
		mb->reveal();
		mb->pleaseDraw();
	} else {
		saveFile();
	}

	free(pathfilename);

}

void handleNew()
{
	stopPlay();

	delete song;

	Song* newsong = new Song(10, 125);
	setSong(newsong);
	DC_FlushAll();

	labelFilename->setCaption("");
	memoryiindicator_disk->pleaseDraw();
}


void handleDiskOPChangeFileType(u8 newidx)
{
	if(newidx==FILETYPE_SONG)
	{
		fileselector->setDir(settings->getSongPath());

		fileselector->selectFilter("song");
		cbsamplepreview->hide();

		labelFilename->setCaption(state->song_filename);
	}
	else if(newidx==FILETYPE_SAMPLE)
	{
		fileselector->setDir(settings->getSamplePath());

		fileselector->selectFilter("sample");
		cbsamplepreview->show();
		tabbox->pleaseDraw();

		labelFilename->setCaption(state->sample_filename);
	}
	else if(newidx==FILETYPE_INST)
	{
		fileselector->selectFilter("instrument");
	}
}


void deleteTypewriter(void)
{
	gui->unregisterOverlayWidget();
	typewriter_active = false;
	delete tw;
	redrawSubScreen();
}


void handleTypewriterFilenameOk(void)
{

	char *text = tw->getText();
	char *name = 0;
#ifdef DEBUG
	iprintf("%s\n", text);
#endif
	if(strcmp(text,"") != 0)
	{
		if( (rbsong->getActive() == true) && (strcmp(text+strlen(text)-3, ".xm") != 0) )
		{
			// Append extension
			name = (char*)malloc(strlen(text)+3+1);
			strcpy(name,text);
			strcpy(name+strlen(name),".xm");
		}
		else if( (rbsample->getActive() == true) && (strcmp(text+strlen(text)-4, ".wav") != 0) )
		{
			// Append extension
			name = (char*)malloc(strlen(text)+4+1);
			strcpy(name,text);
			strcpy(name+strlen(name),".wav");
		}
		else
		{
			// Leave as is
			name = (char*)malloc(strlen(text)+1);
			strcpy(name,text);
		}
		labelFilename->setCaption(name);

		// Remember the name
		if(rbsong->getActive() == true)
		{
			strcpy(state->song_filename, name);
		}
		else if(rbsample->getActive() == true)
		{
			strcpy(state->sample_filename, name);
		}
	}
	deleteTypewriter();
	free(name);
}


void emptyNoteStroke(void) {
	handleNoteStroke(EMPTY_NOTE);
	drawMainScreen();
}


void stopNoteStroke(void) {
	handleNoteStroke(STOP_NOTE);
	drawMainScreen();
}


void delNote(void) // Delete a cell and move the cells below it up
{
	//if(!state->recording) return;
	Cell **ptn = song->getPattern(song->getPotEntry(state->potpos));
	for(u16 row=state->row; row < song->getPatternLength(song->getPotEntry(state->potpos))-1; ++row) {
		ptn[state->channel][row] = ptn[state->channel][row+1];
	}

	Cell *lastcell = &ptn[state->channel][song->getPatternLength(song->getPotEntry(state->potpos))-1];
	song->clearCell(lastcell);

	DC_FlushAll();

	drawMainScreen();
}


void insNote(void)
{
	Cell **ptn = song->getPattern(song->getPotEntry(state->potpos));
	u16 ptnlen = song->getPatternLength(song->getPotEntry(state->potpos));
	for(s32 row=ptnlen-2; row >= state->row; --row) {
		ptn[state->channel][row+1] = ptn[state->channel][row];
	}

	Cell *firstcell = &ptn[state->channel][state->row];
	song->clearCell(firstcell);
	DC_FlushAll();

	drawMainScreen();
}


void changeAdd(u8 newadd) {
	state->add = newadd;
}


void changeOctave(u8 newoctave)
{
	state->basenote = 12*newoctave;

	if(lbsamples->is_visible() == true)
		drawSampleNumbers();
}


void drawMainScreen(void)
{
	// Draw widgets (to back buffer)
	gui->drawMainScreen();

	// Flip buffers
	active_buffer = !active_buffer;

	if(active_buffer == FRONT_BUFFER) {
	    bgSetMapBase(2, 2);
		main_vram_front = (uint16*)BG_BMP_RAM(2);
		main_vram_back = (uint16*)BG_BMP_RAM(8);
	} else {
	    bgSetMapBase(2, 8);
		main_vram_front = (uint16*)BG_BMP_RAM(8);
		main_vram_back = (uint16*)BG_BMP_RAM(2);
	}
}


void redrawSubScreen(void)
{
	// Fill screen
	for(u16 i=0;i<256*153;++i) {
		sub_vram[i] = RGB15(4,6,15)|BIT(15);
	}

	// Redraw GUI
	gui->drawSubScreen();
}


// Called on every tick when the song is playing
void HandleTick(void)
{
}


void startPlay(void)
{
	// Send play command
	if(state->pause == false)
		CommandStartPlay(state->potpos, 0, true);
	else
		CommandStartPlay(state->potpos, state->row, true);

	state->playing = true;
	state->pause = false;

	buttonplay->hide();
	buttonpause->show();
}


void stop(void)
{
	// Send stop command
	CommandStopPlay();
	state->playing = false;

	// The arm7 will get the command with a slight delay and may continue playing for
	// some ticks. But for saving battery, we only draw the screen continuously
	// if state->playing == true. So, by setting it to false here we might miss ticks
	// resultsing in the pattern view being out of sync with the song. So we wait two
	// frames to make sure the arm7 has really stopped and redraw the pattern.
	//swiWaitForVBlank(); swiWaitForVBlank();
	drawMainScreen();

#ifdef WIFI
	if( (state->dsmi_connected) && (state->dsmi_send) )
	{
		for(u8 chn=0; chn<16; ++chn) {
			dsmi_write(MIDI_CC | chn, 120, 0);
			dsmi_write(NOTE_OFF | chn, dsmw_lastnotes[chn], 0);
		}
	}
#endif
}

void stopPlay(void)
{
	stop();

	state->pause = false;

	buttonpause->hide();
	buttonplay->show();
}

void pausePlay(void)
{
	state->pause = true;

	// Send stop command
	CommandStopPlay();

	buttonpause->hide();
	buttonplay->show();
}

void potGoto(u8 pos)
{
	state->potpos = pos;
	state->row = 0;

	if((state->playing == true) && (state->pause == false)) // Play from new pattern
		CommandStartPlay(state->potpos, state->row, true);
}


void setRecordMode(bool is_on)
{
	state->recording = is_on;
	drawMainScreen(); // <- must redraw because of orange lines

	// Draw border
	u16 col;

	if(is_on)
		col = RGB15(31, 0, 0) | BIT(15); // red
	else
		col = RGB15(4,6,15) | BIT(15); // bg color

	for(u16 i=0; i<256; ++i)
		sub_vram[i] = col;

	for(u16 i=0; i<256; ++i)
		sub_vram[256*191+i] = col;

	for(u8 i=0; i<192; ++i)
		sub_vram[256*i] = col;

	for(u8 i=0; i<192; ++i)
		sub_vram[256*i+255] = col;
}


// Updates several GUI elements that display pattern
// related info to the new pattern
void updateGuiToNewPattern(u8 newpattern)
{
	// Update pattern length slider
	nsptnlen->setValue(song->getPatternLength(newpattern));
}


// Callback called from song when the pot element changes during playback
void handlePotPosChangeFromSong(u16 newpotpos)
{
	state->potpos = newpotpos;
	state->row = 0;

	// Update lbpot
	lbpot->select(newpotpos);

	// Update other GUI Elements
	updateGuiToNewPattern(song->getPotEntry(newpotpos));
}

#ifdef WIFI

void handleDSMWRecv(void)
{
	u8 message, data1, data2;

	while(dsmi_read(&message, &data1, &data2))
	{
		if(state->dsmi_recv) {

#ifdef DEBUG
			iprintf("got sth\n");
#endif

			u8 type = message & 0xF0;
			switch(type)
			{
				case NOTE_ON: {
					u8 inst = message & 0x0F;
					u8 note = data1;
					u8 volume = data2 * 2;
					u8 channel = 255;
#ifdef DEBUG
					iprintf("on %d %d\n", inst, note);
#endif
					CommandPlayInst(inst, note, volume, channel);
					break;
				}

				case NOTE_OFF: {
					u8 channel = message & 0x0F;
					CommandStopInst(channel);
					break;
				}
			}
		}
	}
}

#endif

// Callback called from lbpot when the user changes the pot element
void handlePotPosChangeFromUser(u16 newpotpos)
{
	// Update potpos in song
	if(newpotpos>=song->getPotLength()) {
		potGoto(song->getPotLength() - 1);
	} else {
		potGoto(newpotpos);
	}

	// Update other GUI Elements
	updateGuiToNewPattern(song->getPotEntry(newpotpos));

	drawMainScreen();
}

void handlePotDec(void) {

	u8 pattern = song->getPotEntry(state->potpos);
	if(pattern>0) {
		pattern--;
		song->setPotEntry(state->potpos, pattern);
		// If the current pos was changed, switch the pattern
		DC_FlushAll();

		drawMainScreen();

		// Update pattern length slider
		nsptnlen->setValue(song->getPatternLength(song->getPotEntry(state->potpos)));
	}
	char *str = (char*)malloc(3);
	sprintf(str, "%2x", pattern);
	lbpot->set(state->potpos, str);
	free(str);
}


void handlePotInc(void)
{
	u8 pattern = song->getPotEntry(state->potpos);
	if(pattern<MAX_PATTERNS-1) {
		pattern++;

		// Add new pattern if patterncount exceeded
		if(pattern > song->getNumPatterns()-1) {
			song->addPattern(nsptnlen->getValue());
		}

		song->setPotEntry(state->potpos, pattern);
		DC_FlushAll();

		drawMainScreen();

		// Update pattern length slider
		nsptnlen->setValue(song->getPatternLength(song->getPotEntry(state->potpos)));
	}
	char *str = (char*)malloc(3);
	sprintf(str, "%2x", pattern);
	lbpot->set(state->potpos, str);
	free(str);
}


// Inserts a pattern into the pot (copies the current pattern)
void handlePotIns(void)
{
	song->potIns(state->potpos, song->getPotEntry(state->potpos));
	DC_FlushAll();
	lbpot->ins(lbpot->getidx(), lbpot->get(lbpot->getidx()));
}


void handlePotDel(void)
{
	if(song->getPotLength()>1) {
		lbpot->del();
	}

	song->potDel(state->potpos);
	DC_FlushAll();

	if(state->potpos>=song->getPotLength()) {
		state->potpos = song->getPotLength() - 1;
	}

	if(song->getRestartPosition() >= song->getPotLength()) {
		song->setRestartPosition( song->getPotLength() - 1 );
		nsrestartpos->setValue( song->getRestartPosition() );
		DC_FlushAll();
	}
}

void handlePtnClone(void)
{
	u16 newidx = song->getNumPatterns();
	if(newidx == MAX_PATTERNS)
		return;

	u16 ptnlength = song->getPatternLength(song->getPotEntry(state->potpos));
	song->addPattern(ptnlength);
	song->potIns(state->potpos+1, newidx);

	Cell **srcpattern = song->getPattern(song->getPotEntry(state->potpos));
	Cell **destpattern = song->getPattern(newidx);

	for(u16 chn=0; chn<song->getChannels(); ++chn) {
		for(u16 row=0; row<ptnlength; ++row) {
			destpattern[chn][row] = srcpattern[chn][row];
		}
	}

	DC_FlushAll();
	char numberstr[3] = {0};
	sprintf(numberstr, "%2x", newidx);
	lbpot->ins(lbpot->getidx()+1, numberstr);
}

void handleChannelAdd(void)
{
	song->channelAdd();
	drawMainScreen();
	updateLabelChannels();
}


void handleChannelDel(void)
{
	song->channelDel();

	// Move back cursor if necessary
	if(state->channel > song->getChannels()-1) {
		state->channel = song->getChannels()-1;
	}

	// Unmute channel if it is muted
	if(pv->isMuted(song->getChannels()))
	{
		pv->unmute(song->getChannels());
	}

	// Unsolo channel is it is solo
	if(pv->soloChannel() == song->getChannels())
	{
		pv->unmuteAll();
	}

	drawMainScreen();
	updateLabelChannels();
}


void handlePtnLengthChange(s32 newlength)
{
	if(newlength != song->getPatternLength(song->getPotEntry(state->potpos)))
	{
		song->resizePattern(song->getPotEntry(state->potpos), newlength);
		DC_FlushAll();
		// Scroll back if necessary
		if(state->row >= newlength) {
			state->row = newlength-1;
		}
		drawMainScreen();
	}
}


void handleTempoChange(u8 tempo) {

	song->setTempo(tempo);
	DC_FlushAll();
}

void handleBpmChange(s32 bpm) {

	song->setBpm(bpm);
	DC_FlushAll();
}

void handleRestartPosChange(s32 restartpos)
{
	if(restartpos > song->getPotLength()-1) {
		nsrestartpos->setValue(song->getPotLength()-1);
		restartpos = song->getPotLength()-1;
	}
	song->setRestartPosition(restartpos);
	DC_FlushAll();
}

void zapPatterns(void)
{
	song->zapPatterns();
	DC_FlushAll();
	deleteMessageBox();

	// Update POT
	lbpot->clear();
	lbpot->add(" 0");

	updateLabelChannels();
	updateGuiToNewPattern(0);

	state->potpos = 0;
	state->row = 0;
	state->channel = 0;

	drawMainScreen();

	CommandSetSong(song);
	memoryiindicator->pleaseDraw();
}

void zapInstruments(void)
{
	song->zapInstruments();
	DC_FlushAll();
	deleteMessageBox();

	// Update instrument list
	for(u8 i=0;i<MAX_INSTRUMENTS;++i) {
		lbinstruments->set(i, "");
	}

	// Update sample list
	for(u8 i=0;i<MAX_INSTRUMENT_SAMPLES;++i) {
		lbsamples->set(i, "");
	}

	// Clear sample display
	sampledisplay->setSample(0);

	CommandSetSong(song);

	memoryiindicator->pleaseDraw();
}

void zapSong(void) {

	deleteMessageBox();
	delete song;
	setSong(new Song());
	memoryiindicator->pleaseDraw();
}

void handleZap(void)
{
	stopPlay(); // Safety first

	mb = new MessageBox(&sub_vram, "what to zap", 4, "patterns", zapPatterns,
		"instruments", zapInstruments, "song", zapSong, "cancel",
		deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void handleNewRow(u16 row)
{
	state->row = row;
	need_to_redraw = true; // Trigger a redraw

	if(!state->playing)
		return;

#ifdef WIFI
	if( (state->dsmi_connected) && (state->dsmi_send) )
	{
		Cell ** pattern = song->getPattern( song->getPotEntry( state->potpos ) );

		Cell *curr_cell;

		for(u8 chn=0; chn < song->getChannels(); ++chn)
		{
			if(song->channelMuted(chn))
				continue;

			curr_cell = &(pattern[chn][state->row]);

			if(curr_cell->note == 254) // Note off
			{
				dsmi_write(NOTE_OFF | dsmw_lastchannels[chn], dsmw_lastnotes[chn], 0);
				dsmw_lastnotes[chn] = curr_cell->note;
			}
			else if(curr_cell->note < 254) // Note on
			{
				// Turn the last note off
				if(dsmw_lastnotes[chn] < 254) {
					dsmi_write(NOTE_OFF | dsmw_lastchannels[chn], dsmw_lastnotes[chn], 0);
				}
				u8 midichannel = curr_cell->instrument % 16;
				dsmi_write(NOTE_ON | midichannel, curr_cell->note, curr_cell->volume / 2);

				dsmw_lastchannels[chn] = midichannel;
				dsmw_lastnotes[chn] = curr_cell->note;
			}
		}
	}
#endif
}

void handleStop(void)
{
	state->playing = false;
}

void handleSamplePreviewToggled(bool on)
{
	settings->setSamplePreview(on);
}

void handleFileChange(File file)
{
	if(!file.is_dir)
	{
		char *str = (char*)malloc(256);
		strncpy(str, file.name.c_str(), 256);
		lowercase(str);
		labelFilename->setCaption(str);

		if(rbsong->getActive() == true)
		{
			strcpy(state->song_filename, str);
		}
		else if(rbsample->getActive() == true)
		{
			strcpy(state->sample_filename, str);
		}

		// Preview wav files
		if( (strcmp(&str[strlen(str)-3], "wav") == 0) && (settings->getSamplePreview() == true) )
		{
			// Stop playing sample if necessary
			CommandStopInst(0);

			// Check if it's not too big
			u32 smpsize = my_getFileSize(file.name_with_path.c_str());

			u8* testptr = (u8*)malloc(smpsize); // Try to malloc it
			if(testptr == 0)
			{
#ifdef DEBUG
				iprintf("not enough ram for preview\n");
#endif
			}
			else
			{
#ifdef DEBUG
				iprintf("previewing\n");
#endif
				free(testptr);

				// Load sample
				bool success;
				Sample *smp = new Sample(file.name_with_path.c_str(), false, &success);
				if(!success)
				{
					delete smp;
				}
				else
				{
					// Stop and delete previously playing preview sample
					if(state->preview_sample)
					{
						CommandStopSample(0);
						while(state->preview_sample)
						{
							swiWaitForVBlank();
						}
					}

					// Play it
					state->preview_sample = smp;
					DC_FlushAll();
					CommandPlaySample(smp, 4*12, 255, 0);

					memoryiindicator_disk->pleaseDraw();

					// When the sample has finished playing, the arm7 sends a signal,
					// so the arm9 can delete the sampleb
				}
			}
		}

		free(str);
	}
}

void handleDirChange(const char *newdir)
{
	if(rbsong->getActive() == true)
	{
		settings->setSongPath(newdir);
	}
	else if(rbsample->getActive() == true)
	{
		settings->setSamplePath(newdir);
	}
}

void handlePreviewSampleFinished(void)
{
#ifdef DEBUG
	iprintf("Sample finished\n");
#endif
	delete state->preview_sample;
	state->preview_sample = 0;

	memoryiindicator_disk->pleaseDraw();
}

void setNoteVol(u16 vol)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	if(pv->getSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2) == true)
	{
		Cell **ptn = song->getPattern(song->getPotEntry(state->potpos));
		for(u16 col=sel_x1; col<=sel_x2; ++col) {
			for(u16 row=sel_y1; row<=sel_y2; ++row) {
				if( (ptn[col][row].note != EMPTY_NOTE) && (ptn[col][row].note != STOP_NOTE) )
					ptn[col][row].volume = vol;
			}
		}

	}
	else
	{
		Cell *cell = &(song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->row]);
		if( (cell->note != EMPTY_NOTE) && (cell->note != STOP_NOTE) )
			cell->volume = vol;

	}
	DC_FlushAll();
}

void handleNoteVolumeChanged(int vol)
{
	setNoteVol(vol);
}

void handleSetNoteVol(void)
{
	setNoteVol(nsnotevolume->getValue());
	drawMainScreen();
}

void showTypewriter(const char *prompt, const char *str, void (*okCallback)(void), void (*cancelCallback)(void))
{
    // TODO: Migrate to new TobKit to eliminate such ugliness
#define SUB_BG1_X0 (*(vuint16*)0x04001014)
#define SUB_BG1_Y0 (*(vuint16*)0x04001016)

	tw = new Typewriter(prompt, (uint16*)CHAR_BASE_BLOCK_SUB(1),
		(uint16*)SCREEN_BASE_BLOCK_SUB(12), 3, &sub_vram, &SUB_BG1_X0, &SUB_BG1_Y0);

	tw->setText(str);
	gui->registerOverlayWidget(tw, mykey_LEFT|mykey_RIGHT, SUB_SCREEN);
	if(okCallback!=0) {
		tw->registerOkCallback(okCallback);
	}
	if(cancelCallback != 0) {
		tw->registerCancelCallback(cancelCallback);
	}
	typewriter_active = true;
	tw->reveal();
}


void showTypewriterForFilename(void) {
	showTypewriter("filename", labelFilename->getCaption(), handleTypewriterFilenameOk, deleteTypewriter);
}


void handleTypewriterInstnameOk(void)
{
	song->getInstrument(lbinstruments->getidx())->setName(tw->getText());
	lbinstruments->set( lbinstruments->getidx(), tw->getText() );

	deleteTypewriter();
}


void showTypewriterForInstRename(void)
{
	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	if(inst==NULL) {
		return;
	}

	showTypewriter("inst name", lbinstruments->get(lbinstruments->getidx()), handleTypewriterInstnameOk, deleteTypewriter);
}

void handleTypewriterSongnameOk(void)
{
	song->setName(tw->getText());
	labelsongname->setCaption(song->getName());
	deleteTypewriter();
}

void showTypewriterForSongRename(void)
{
	if(!state->playing) {
		showTypewriter("song name", song->getName(), handleTypewriterSongnameOk, deleteTypewriter);
	}
}

void handleTypewriterSampleOk(void)
{
	song->getInstrument(lbinstruments->getidx())->getSample(lbsamples->getidx())->setName(tw->getText());
	lbsamples->set( lbsamples->getidx(), tw->getText() );

	deleteTypewriter();
}

void handleToggleMultiSample(bool on)
{
	if(on)
	{
		drawSampleNumbers();
		kb->showKeyLabels();
		tbmultisample->setCaption("-");
		lbinstruments->resize(114, 67);
		buttonrenamesample->show();
		lbsamples->show();
		tbmapsamples->show();
	}
	else
	{
		kb->hideKeyLabels();
		tbmultisample->setCaption("+");
		buttonrenamesample->hide();
		lbsamples->hide();
		lbinstruments->resize(114, 89);
		tbmapsamples->hide();
	}
}

void showTypewriterForSampleRename(void)
{
	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	if(inst == 0)
		return;

	Sample *sample = inst->getSample(lbsamples->getidx());
	if(sample == 0)
		return;

	showTypewriter("sample name", lbsamples->get(lbsamples->getidx()), handleTypewriterSampleOk, deleteTypewriter);
}

void handleRecordSampleOK(void)
{
	Sample *smp = recordbox->getSample();

	// Kill record box
	gui->unregisterOverlayWidget();
	delete recordbox;

	// Turn off the mic
	CommandMicOff();

	// Add instrument if necessary
	Instrument *inst = song->getInstrument(state->instrument);

	if(inst == 0)
	{
		inst = new Instrument("rec");
		song->setInstrument(state->instrument, inst);

		lbinstruments->set(state->instrument, inst->getName());
	}

	// Insert the sample into the instrument
	inst->setSample(state->sample, smp);

	lbsamples->set(state->sample, smp->getName());

	volEnvSetInst(inst);

	cbvolenvenabled->setChecked(inst->getVolEnvEnabled());

	sampleChange(smp);
	updateKeyLabels();
	redrawSubScreen();
}

void handleRecordSampleCancel(void)
{
	// Kill record box
	gui->unregisterOverlayWidget();
	delete recordbox;

	// Turn off the mic
	CommandMicOff();

	redrawSubScreen();
}

// OMG FUCKING BEST FEATURE111
void handleRecordSample(void)
{
	// Check RAM first!
	void *testbuf = malloc(RECORDBOX_SOUNDDATA_SIZE * 2);
	if(testbuf == 0)
	{
		showMessage("not enough ram free!");
		return;
	}

	free(testbuf);

	// Turn on the mic
	CommandMicOn();

	// Get sample
	Sample *smp = 0;
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != 0)
		smp = inst->getSample(state->sample);

	// Show record box

	recordbox = new RecordBox(&sub_vram, handleRecordSampleOK, handleRecordSampleCancel, smp, inst, state->sample);

	gui->registerOverlayWidget(recordbox, KEY_A | KEY_B, SUB_SCREEN);

	recordbox->reveal();

}


void handleNormalizeOK(void)
{
	u16 percent = normalizeBox->getValue();

	Sample *sample = song->getInstrument(state->instrument)->getSample(state->sample);

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(!sel_exists)
	{
		startsample = 0;
		endsample = sample->getNSamples() - 1;
	}

	sample->normalize(percent, startsample, endsample);

	gui->unregisterOverlayWidget();
	delete normalizeBox;
	redrawSubScreen();
}

void handleNormalizeCancel(void)
{
	gui->unregisterOverlayWidget();
	delete normalizeBox;
	redrawSubScreen();
}

void sample_show_normalize_window(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(!inst) return;
	Sample *smp = inst->getSample(state->sample);
	if(!smp) return;

	normalizeBox = new NormalizeBox(&sub_vram, handleNormalizeOK, handleNormalizeCancel);
	gui->registerOverlayWidget(normalizeBox, 0, SUB_SCREEN);
	normalizeBox->reveal();
}

void swapControls(Handedness handedness)
{
	if(handedness == LEFT_HANDED)
	{
		mykey_UP = KEY_X;
		mykey_DOWN = KEY_B;
		mykey_LEFT = KEY_Y;
		mykey_RIGHT = KEY_A;
		mykey_L = KEY_R;
		mykey_R = KEY_L;
		mykey_A = KEY_RIGHT;
		mykey_B = KEY_DOWN;
		mykey_X = KEY_UP;
		mykey_Y = KEY_LEFT;
		keys_that_are_repeated = KEY_A | KEY_B | KEY_X | KEY_Y;
	}
	else
	{
		mykey_UP = KEY_UP;
		mykey_DOWN = KEY_DOWN;
		mykey_LEFT = KEY_LEFT;
		mykey_RIGHT = KEY_RIGHT;
		mykey_L = KEY_L;
		mykey_R = KEY_R;
		mykey_A = KEY_A;
		mykey_B = KEY_B;
		mykey_X = KEY_X;
		mykey_Y = KEY_Y;
		keys_that_are_repeated = KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT;
	}
}

void swapPatternButtons(Handedness handedness)
{
	if(handedness == LEFT_HANDED)
	{
		buttonswitchmain->setPos(8, 1);
		labelmute->setPos(1, 23);
		buttonunmuteall->setPos(0, 32);
		labelnotevol->setPos(5, 44);
		nsnotevolume->setPos(0, 54);
		buttonsetnotevol->setPos(0, 70);
		buttoncut->setPos(0,  86);
		buttoncopy->setPos(0, 99);
		buttonpaste->setPos(0, 112);
		buttoncolselect->setPos(0, 125);
		buttoninsnote->setPos(0, 140);
		buttondelnote->setPos(0, 153);
		buttonemptynote2->setPos(0, 166);
		buttonstopnote2->setPos(0, 179);

		pv->setPos(30, 0);
	}
	else
	{
		buttonswitchmain->setPos(233, 1);
		labelmute->setPos(226, 23);
		buttonunmuteall->setPos(225, 32);
		labelnotevol->setPos(230, 44);
		nsnotevolume->setPos(225, 54);
		buttonsetnotevol->setPos(225, 70);
		buttoncut->setPos(225,  86);
		buttoncopy->setPos(225, 99);
		buttonpaste->setPos(225, 112);
		buttoncolselect->setPos(225, 125);
		buttoninsnote->setPos(225, 140);
		buttondelnote->setPos(225, 153);
		buttonemptynote2->setPos(225, 166);
		buttonstopnote2->setPos(225, 179);

		pv->setPos(0, 0);
	}

	drawMainScreen();
}

void handleHandednessChange(u8 handedness)
{
	clearMainScreen();

	if(handedness == 0)
	{
		settings->setHandedness(LEFT_HANDED);
		swapControls(LEFT_HANDED);
		swapPatternButtons(LEFT_HANDED);
	}
	else
	{
		settings->setHandedness(RIGHT_HANDED);
		swapControls(RIGHT_HANDED);
		swapPatternButtons(RIGHT_HANDED);
	}
}


void switchScreens(void)
{
	lcdSwap();
	gui->switchScreens();
	pv->clearSelection();
	drawMainScreen();
}


// Create the song and do other init stuff yet to be determined.
void setupSong(void) {
	song = new Song(6, 125);
	DC_FlushAll();
}


void deleteMessageBox(void)
{
	gui->unregisterOverlayWidget();

	delete mb;

	mb = 0;
	redrawSubScreen();
}


void showMessage(const char *msg)
{
	mb = new MessageBox(&sub_vram, msg, 1, "doh!", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

bool didExit = false;

void exitApp()
{
	deleteMessageBox();
	didExit = true;
}

void showAboutBox(void)
{
	mb = new MessageBox(&sub_vram, "nitrotracker.tobw.net", 2, "track on!", deleteMessageBox, "exit app", exitApp);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void clipboard_alloc(u16 width, u16 height)
{
	if(clipboard != 0) {
		for(u16 i=0; i<clipboard_width; ++i) {
			free(clipboard[i]);
		}
		free(clipboard);
	}

	clipboard = (Cell**)malloc(sizeof(Cell*)*width);
	for(u16 i=0; i<width; ++i) {
		clipboard[i] = (Cell*)malloc(sizeof(Cell)*height);
	}
	clipboard_width = width;
	clipboard_height = height;
}

void clipboard_free()
{
	if(clipboard != 0) {
		for(u16 i=0; i<clipboard_width; ++i) {
			free(clipboard[i]);
		}
		free(clipboard);
	}
}

void ptnCopy(bool cut)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	if(pv->getSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2) == false) return;
	u16 n_cols = sel_x2 - sel_x1 + 1;
	u16 n_rows = sel_y2 - sel_y1 + 1;

	clipboard_alloc(n_cols, n_rows);

	Cell **ptn = song->getPattern(song->getPotEntry(state->potpos));
	for(u16 col=sel_x1; col<=sel_x2; ++col) {
		for(u16 row=sel_y1; row<=sel_y2; ++row) {
			clipboard[col-sel_x1][row-sel_y1] = ptn[col][row];
		}
	}

	if(cut == true) {
		for(u16 col=sel_x1; col<=sel_x2; ++col) {
			for(u16 row=sel_y1; row<=sel_y2; ++row) {
				song->clearCell(&(ptn[col][row]));
			}
		}
	}
}

void handleCut(void)
{
	ptnCopy(true);
	drawMainScreen();
	DC_FlushAll();
}

void handleCopy(void)
{
	ptnCopy(false);
	drawMainScreen();
}

void handlePaste(void)
{
	if(clipboard != 0) {
		u16 startcol = state->channel;
		u16 endcol;
		if(startcol + clipboard_width - 1 >= song->getChannels()) {
			endcol = song->getChannels() - 1;
		} else {
			endcol = startcol + clipboard_width - 1;
		}
		u16 startrow = state->row;
		u16 endrow;
		u16 ptnlen = song->getPatternLength(song->getPotEntry(state->potpos));
		if(startrow + clipboard_height -1 >= ptnlen) {
			endrow = ptnlen-1;
		} else {
			endrow = startrow + clipboard_height - 1;
		}

		Cell **ptn = song->getPattern(song->getPotEntry(state->potpos));
		for(u16 col=startcol; col<=endcol; ++col) {
			for(u16 row=startrow; row<=endrow; ++row) {
				ptn[col][row] = clipboard[col-startcol][row-startrow];
			}
		}

		DC_FlushAll();
	}

	drawMainScreen();
}

void handleButtonColumnSelect(void)
{
	// Is there a selection?
	u16 x1, y1, x2, y2;
	if(pv->getSelection(&x1, &y1, &x2, &y2) == true) {
		// Yes: Expand the selection to use the complete rows
		y1 = 0;
		y2 = song->getPatternLength(song->getPotEntry(state->potpos)) - 1;
		pv->setSelection(x1, y1, x2, y2);
	} else {
		// No: Select row at cursor
		x1 = x2 = state->channel;
		y1 = 0;
		y2 = song->getPatternLength(song->getPotEntry(state->potpos)) - 1;
		pv->setSelection(x1, y1, x2, y2);
	}
	drawMainScreen();
}

void handleSampleVolumeChange(s32 newvol)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	u8 vol;
	if(newvol>=64) {
		vol = 255;
	} else {
		vol = newvol*4;
	}

	smp->setVolume(vol);
	DC_FlushAll();
}

void handleSamplePanningChange(s32 newpanning)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	u8 pan = newpanning * 2;

	smp->setPanning(pan);
	DC_FlushAll();
}

void handleSampleRelNoteChange(s32 newnote)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	DC_FlushAll();

	smp->setRelNote(newnote);
}

void handleSampleFineTuneChange(s32 newfinetune)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	DC_FlushAll();

	smp->setFinetune(newfinetune);
}

void handleMuteAll(void)
{
	pv->muteAll();
	drawMainScreen();
}

void handleUnmuteAll(void)
{
	pv->unmuteAll();
	drawMainScreen();
}

void sample_select_all(void)
{
	sampledisplay->select_all();
}

void sample_clear_selection(void)
{
	sampledisplay->clear_selection();
}

void sample_del_selection(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(sel_exists==false) return;

	smp->delPart(startsample, endsample);

	DC_FlushAll();

	sampledisplay->setSample(smp);
}

void sample_fade_in(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(sel_exists==false) return;

	smp->fadeIn(startsample, endsample);

	DC_FlushAll();

	sampledisplay->setSample(smp);
}

void sample_fade_out(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(sel_exists==false) return;

	smp->fadeOut(startsample, endsample);

	DC_FlushAll();

	sampledisplay->setSample(smp);
}

void sample_reverse(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(sel_exists==false) {
		startsample = 0;
		endsample = smp->getNSamples();
	}

	smp->reverse(startsample, endsample);

	DC_FlushAll();

	sampledisplay->setSample(smp);
}

void handleLoopToggle(bool state)
{
	CommandSetPatternLoop(state);
}

void sampleTabBoxChage(u8 tab)
{
	if( (tab==0) or (tab==1) )
		sampledisplay->setActive();
	else
		sampledisplay->setInactive();

	if(tab != 1) {
		sampledisplay->setDrawMode(false);
		buttonsmpdraw->setState(false);
	}

	if(tab==2)
	{
		Instrument *inst = song->getInstrument(state->instrument);
		if(inst == NULL) {
			sampledisplay->hideLoopPoints();
			return;
		}
		Sample *sample = inst->getSample(state->sample);
		if(sample == NULL) {
			sampledisplay->hideLoopPoints();
			return;
		}
		if(sample->getLoop() == 0)
			sampledisplay->hideLoopPoints();
		else
			sampledisplay->showLoopPoints();
	}
}

#ifdef WIFI

void dsmiConnect(void)
{
	mb = new MessageBox(&sub_vram, "connecting ...", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	int res = dsmi_connect();
	deleteMessageBox();

	if(res == 0) {
		showMessage("Sorry, couldn't connect.");
		state->dsmi_connected = false;
	} else {
#ifdef DEBUG
		iprintf("YAY, connected!\n");
#endif
		btndsmwtoggleconnect->setCaption("disconnect");
        btndsmwtoggleconnect->pleaseDraw();
        state->dsmi_connected = true;
	}
}

// h4x!
extern int sock, sockin;

void dsmiDisconnect(void)
{
	close(sock);
	close(sockin);

	Wifi_DisconnectAP();
	Wifi_DisableWifi();

	btndsmwtoggleconnect->setCaption("connect");
	btndsmwtoggleconnect->pleaseDraw();

	state->dsmi_connected = false;
}

void dsmiToggleConnect(void)
{
	if(!state->dsmi_connected)
		dsmiConnect();
	else
		dsmiDisconnect();
}

void handleDsmiSendToggled(bool is_active)
{
	state->dsmi_send = is_active;
}

void handleDsmiRecvToggled(bool is_active)
{
	state->dsmi_recv = is_active;
}
#endif

void toggleMapSamples(bool is_active)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == NULL)
		return;

	if(is_active)
	{
		if(tbmultisample->getState() == false)
			tbmultisample->setState(true);
	}

	state->map_samples = is_active;
}

void addEnvPoint(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != NULL)
		volenvedit->addPoint();
}

void delEnvPoint(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != NULL)
		volenvedit->delPoint();
}

void toggleVolEnvEnabled(bool is_enabled)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != NULL)
		inst->setVolEnvEnabled(is_enabled);
}

void handleMuteChannelsChanged(bool *muted_channels)
{
	for(u8 chn=0; chn < song->getChannels(); ++chn)
	{
		song->setChannelMute(chn, muted_channels[chn]);
	}

	DC_FlushAll();
}

void handleSampleLoopChanged(u8 val)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == 0)
		return;

	Sample *smp = inst->getSample(state->sample);
	if(smp == 0)
		return;

	smp->setLoop(val);

	if(val == NO_LOOP)
		sampledisplay->hideLoopPoints();
	else
		sampledisplay->showLoopPoints();

	DC_FlushAll();
}

void handleSnapTo0XingToggled(bool on)
{
	sampledisplay->setSnapToZeroCrossing(on);
}

void envZoomIn(void)
{
	volenvedit->zoomIn();
}

void envZoomOut(void)
{
	volenvedit->zoomOut();
}

void volEnvPointsChanged(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == 0)
		return;

	u16 *xs, *ys;
	u8 n_points = volenvedit->getPoints(&xs, &ys);

	inst->setVolumeEnvelopePoints(xs, ys, n_points);

	toggleVolEnvEnabled(n_points != 0 && inst->getVolEnvEnabled());
	volenvedit->pleaseDraw();

	DC_FlushAll();
}

void volEnvDrawFinish(void)
{
	cbvolenvenabled->setChecked(true);
	toggleVolEnvEnabled(true);
	DC_FlushAll();
}

void envStartDrawMode(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == 0)
		return;

	volenvedit->startDrawMode();
}

void sampleDrawToggle(bool on)
{
	sampledisplay->setDrawMode(on);
}

void setupGUI(void)
{
	gui = new GUI();
	gui->setTheme(settings->getTheme(), settings->getTheme()->col_dark_bg);

	kb = new Piano(0, 152, 224, 40, (uint16*)CHAR_BASE_BLOCK_SUB(0), (uint16*)SCREEN_BASE_BLOCK_SUB(1/*8*/), &sub_vram);
	kb->registerNoteCallback(handleNoteStroke);
	kb->registerReleaseCallback(handleNoteRelease);

	pixmaplogo = new Pixmap(101, 1, 76, 17, nitrotracker_logo, &sub_vram);
	pixmaplogo->registerPushCallback(showAboutBox);

	tabbox = new TabBox(1, 1, 139, 151, &sub_vram);
	tabbox->setTheme(settings->getTheme(), settings->getTheme()->col_dark_bg);
	tabbox->addTab(icon_song_bin);
	tabbox->addTab(icon_disk_bin);
	tabbox->addTab(icon_sample_bin);
	tabbox->addTab(icon_trumpet_bin);
	tabbox->addTab(icon_wrench_bin);

	// <Disk OP GUI>
		fileselector = new FileSelector(38, 21, 100, 111, &sub_vram);

		std::vector<std::string> samplefilter;
		samplefilter.push_back("wav");

		fileselector->addFilter("sample", samplefilter);

		std::vector<std::string> songfilter;
		songfilter.push_back("xm");
		fileselector->addFilter("song", songfilter);
		std::vector<std::string> instfilter;
		instfilter.push_back("xi");
		fileselector->addFilter("instrument", instfilter);
		fileselector->selectFilter("song");
		fileselector->registerFileSelectCallback(handleFileChange);
		fileselector->registerDirChangeCallback(handleDirChange);

		rbgdiskop = new RadioButton::RadioButtonGroup();

		rbsong   = new RadioButton(2, 21, 36, 14, &sub_vram, rbgdiskop);

		rbsong->setCaption("sng");

		rbsample = new RadioButton(2, 36, 36, 14, &sub_vram, rbgdiskop);

		rbsample->setCaption("smp");

		rbgdiskop->setActive(0);

		rbgdiskop->registerChangeCallback(handleDiskOPChangeFileType);

		memoryiindicator_disk = new MemoryIndicator(3, 52, 34, 8, &sub_vram, true);

		labelramusage_disk = new Label(8, 60, 34, 10, &sub_vram, false);
		labelramusage_disk->setCaption("ram");

		cbsamplepreview = new CheckBox(4, 71, 34, 14, &sub_vram, false, true);
		cbsamplepreview->setCaption("pre");
		cbsamplepreview->registerToggleCallback(handleSamplePreviewToggled);

		buttonload = new Button(3 , 102, 34, 14, &sub_vram);
		buttonload->setCaption("load");
		buttonload->registerPushCallback(handleLoad);

		buttonsave = new Button(3 , 118, 34, 14, &sub_vram);
		buttonsave->setCaption("save");
		buttonsave->registerPushCallback(handleSave);

		buttonnew = new Button(3 , 86, 34, 14, &sub_vram);
		buttonnew->setCaption("new");
		buttonnew->registerPushCallback(handleNew);

		labelFilename = new Label(3, 134, 110, 14, &sub_vram);
		labelFilename->setCaption("");
		labelFilename->registerPushCallback(showTypewriterForFilename);

		buttonchangefilename = new Button(115, 134, 23, 14, &sub_vram);
		buttonchangefilename->setCaption("...");
		buttonchangefilename->registerPushCallback(showTypewriterForFilename);

		tabbox->registerWidget(fileselector, 0, 1);
		tabbox->registerWidget(rbsong, 0, 1);
		tabbox->registerWidget(rbsample, 0, 1);
		tabbox->registerWidget(memoryiindicator_disk, 0, 1);
		tabbox->registerWidget(labelramusage_disk, 0, 1);
		tabbox->registerWidget(cbsamplepreview, 0, 1);
		tabbox->registerWidget(buttonsave, 0, 1);
		tabbox->registerWidget(buttonload, 0, 1);
		tabbox->registerWidget(buttonnew, 0, 1);
		tabbox->registerWidget(labelFilename, 0, 1);
		tabbox->registerWidget(buttonchangefilename, 0, 1);
	// </Disk OP GUI>

	// <Song gui>
		lbpot = new ListBox(4, 21, 50, 78, &sub_vram, 1, true);
		lbpot->set(0," 0");
		lbpot->registerChangeCallback(handlePotPosChangeFromUser);
		buttonpotup = new Button(70, 47, 12, 12, &sub_vram);
		buttonpotup->setCaption(">");
		buttonpotup->registerPushCallback(handlePotInc);
		buttonpotdown = new Button(56, 47, 12, 12, &sub_vram);
		buttonpotdown->setCaption("<");
		buttonpotdown->registerPushCallback(handlePotDec);
		buttonins = new Button(56, 21, 26, 12, &sub_vram);
		buttonins->setCaption("ins");
		buttonins->registerPushCallback(handlePotIns);
		buttondel = new Button(56, 60, 26, 12, &sub_vram);
		buttondel->setCaption("del");
		buttondel->registerPushCallback(handlePotDel);
		buttoncloneptn = new Button(56, 34, 26, 12, &sub_vram);
		buttoncloneptn->setCaption("cln");
		buttoncloneptn->registerPushCallback(handlePtnClone);

		labelptnlen = new Label(56, 73, 45, 12, &sub_vram, false);
		labelptnlen->setCaption("ptn len:");
		nsptnlen = new NumberSlider(56, 83, 32, 17, &sub_vram, DEFAULT_PATTERN_LENGTH, 1, 256, true);
		nsptnlen->registerChangeCallback(handlePtnLengthChange);

		labelchannels = new Label(88, 22, 48, 12, &sub_vram, false);
		labelchannels->setCaption("chn:  4");
		buttonlesschannels = new Button(86, 34, 23, 12, &sub_vram);
		buttonlesschannels->setCaption("sub");
		buttonlesschannels->registerPushCallback(handleChannelDel);
		buttonmorechannels = new Button(111, 34, 23, 12, &sub_vram);
		buttonmorechannels->setCaption("add");
		buttonmorechannels->registerPushCallback(handleChannelAdd);

		labeltempo = new Label(4, 101, 32, 12, &sub_vram, false);
		labeltempo->setCaption("tmp");
		labelbpm = new Label(38, 101, 32, 12, &sub_vram, false);
		labelbpm->setCaption("bpm");
		labelrestartpos = new Label(72, 101, 46, 12, &sub_vram, false);
		labelrestartpos->setCaption("restart");
		nbtempo = new NumberBox(4, 113, 32, 17, &sub_vram, 1, 1, 31);
		nbtempo->registerChangeCallback(handleTempoChange);
#ifdef DEBUG
		nsbpm = new NumberSlider(38, 113, 32, 17, &sub_vram, 120, 1, 255);
#else
		nsbpm = new NumberSlider(38, 113, 32, 17, &sub_vram, 120, 32, 255);
#endif
		nsbpm->registerChangeCallback(handleBpmChange);
		nsrestartpos = new NumberSlider(72, 113, 32, 17, &sub_vram, 0, 0, 255, true);
		nsrestartpos->registerChangeCallback(handleRestartPosChange);

		labelsongname = new Label(4, 132, 113, 14, &sub_vram, true);
		labelsongname->setCaption("unnamed");
		labelsongname->registerPushCallback(showTypewriterForSongRename);

		buttonrenamesong = new Button(118, 132, 20, 14, &sub_vram);
		buttonrenamesong->setCaption("...");
		buttonrenamesong->registerPushCallback(showTypewriterForSongRename);

		buttonzap = new Button(107, 114, 30, 14, &sub_vram);
		buttonzap->setCaption("zap!");
		buttonzap->registerPushCallback(handleZap);

		memoryiindicator = new MemoryIndicator(86, 52, 48, 8, &sub_vram);

		labelramusage = new Label(87, 59, 52, 12, &sub_vram, false);
		labelramusage->setCaption("ram use");

		tabbox->registerWidget(lbpot, 0, 0);
		tabbox->registerWidget(buttonpotup, 0, 0);
		tabbox->registerWidget(buttonpotdown, 0, 0);
		tabbox->registerWidget(buttonins, 0, 0);
		tabbox->registerWidget(buttondel, 0, 0);
		tabbox->registerWidget(buttoncloneptn, 0, 0);
		tabbox->registerWidget(nsptnlen, 0, 0);
		tabbox->registerWidget(labelptnlen, 0, 0);
		tabbox->registerWidget(labelchannels, 0, 0);
		tabbox->registerWidget(buttonmorechannels, 0, 0);
		tabbox->registerWidget(buttonlesschannels, 0, 0);
		tabbox->registerWidget(labeltempo, 0, 0);
		tabbox->registerWidget(labelbpm, 0, 0);
		tabbox->registerWidget(labelrestartpos, 0, 0);
		tabbox->registerWidget(nbtempo, 0, 0);
		tabbox->registerWidget(nsbpm, 0, 0);
		tabbox->registerWidget(nsrestartpos, 0, 0);
		tabbox->registerWidget(labelsongname, 0, 0);
		tabbox->registerWidget(buttonrenamesong, 0, 0);
		tabbox->registerWidget(buttonzap, 0, 0);
		tabbox->registerWidget(memoryiindicator, 0, 0);
		tabbox->registerWidget(labelramusage, 0, 0);
	// </Song gui>

	// <Sample Gui>

	sampledisplay = new SampleDisplay(4, 23, 131, 70, &sub_vram);
	sampledisplay->setActive();

	sampletabbox = new HTabBox(3, 94, 132, 55, &sub_vram);
	sampletabbox->setTheme(settings->getTheme(), settings->getTheme()->col_dark_bg);
	sampletabbox->addTab(sampleedit_wave_icon_bin);
	sampletabbox->addTab(sampleedit_draw_small_bin);
	sampletabbox->addTab(sampleedit_control_icon_bin);
	sampletabbox->addTab(sampleedit_loop_icon_bin);

	sampletabbox->registerTabChangeCallback(sampleTabBoxChage);

	// <Sample editing>
		labelsampleedit_record = new Label(18, 99, 21, 30, &sub_vram, true);
		labelsampleedit_record->setCaption("rec");

		buttonrecord = new BitButton(20, 110, 17, 17, &sub_vram, sampleedit_record_bin);
		buttonrecord->registerPushCallback(handleRecordSample);

		labelsampleedit_select = new Label(38, 99, 39, 30, &sub_vram, true);
		labelsampleedit_select->setCaption("select");

		buttonsmpselall = new BitButton(40, 110, 17, 17, &sub_vram, sampleedit_all_bin);
		buttonsmpselall->registerPushCallback(sample_select_all);

		buttonsmpselnone = new BitButton(58, 110, 17, 17, &sub_vram, sampleedit_none_bin);
		buttonsmpselnone->registerPushCallback(sample_clear_selection);

		labelsampleedit_edit = new Label(76, 99, 57, 48, &sub_vram, true);
		labelsampleedit_edit->setCaption("edit");

		buttonsmpfadein = new BitButton(78, 110, 17, 17, &sub_vram, sampleedit_fadein_bin);
		buttonsmpfadein->registerPushCallback(sample_fade_in);

		buttonsmpfadeout = new BitButton(96, 110, 17, 17, &sub_vram, sampleedit_fadeout_bin);
		buttonsmpfadeout->registerPushCallback(sample_fade_out);

		buttonsmpreverse = new BitButton(78, 128, 17, 17, &sub_vram, sampleedit_reverse_bin);
		buttonsmpreverse->registerPushCallback(sample_reverse);

		buttonsmpseldel = new BitButton(96, 128, 17, 17, &sub_vram, sampleedit_del_bin);
		buttonsmpseldel->registerPushCallback(sample_del_selection);

		buttonsmpnormalize = new BitButton(114, 110, 17, 17, &sub_vram, sampleedit_normalize_bin);
		buttonsmpnormalize->registerPushCallback(sample_show_normalize_window);

		sampletabbox->registerWidget(buttonrecord, 0, 0);
		sampletabbox->registerWidget(buttonsmpselall, 0, 0);
		sampletabbox->registerWidget(buttonsmpselnone, 0, 0);
		sampletabbox->registerWidget(buttonsmpseldel, 0, 0);
		sampletabbox->registerWidget(buttonsmpfadein, 0, 0);
		sampletabbox->registerWidget(buttonsmpfadeout, 0, 0);
		sampletabbox->registerWidget(buttonsmpreverse, 0, 0);
		sampletabbox->registerWidget(buttonsmpnormalize, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_edit, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_select, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_record, 0, 0);
	// </Sample editing>

	// <Drawing and Generating>
		buttonsmpdraw = new ToggleButton(18, 96, 17, 17, &sub_vram);
		buttonsmpdraw->setBitmap(sampleedit_draw_bin);
		buttonsmpdraw->registerToggleCallback(sampleDrawToggle);

		sampletabbox->registerWidget(buttonsmpdraw, 0, 1);
	// </Drawing and Generating>

	// <Sample settings>
		labelsamplevolume = new Label(21, 108, 25, 10, &sub_vram, false);
		labelsamplevolume->setCaption("vol");

		labelpanning = new Label(18, 127, 25, 10, &sub_vram, false);
		labelpanning->setCaption("pan");

		labelrelnote = new Label(84, 108, 25, 10, &sub_vram, false);
		labelrelnote->setCaption("rel");

		labelfinetune = new Label(73, 127, 30, 10, &sub_vram, false);
		labelfinetune->setCaption("tune");

		nssamplevolume = new NumberSlider(39, 103, 32, 17, &sub_vram, 64, 0, 64);
		nssamplevolume->registerChangeCallback(handleSampleVolumeChange);

		nspanning = new NumberSlider(39, 122, 32, 17, &sub_vram, 64, 0, 127, false);
		nspanning->registerChangeCallback(handleSamplePanningChange);

		nsrelnote = new NumberSliderRelNote(99, 103, 34, 17, &sub_vram, 0);
		nsrelnote->registerChangeCallback(handleSampleRelNoteChange);

		nsfinetune = new NumberSlider(99, 122, 34, 17, &sub_vram, 0, -128, 127);
		nsfinetune->registerChangeCallback(handleSampleFineTuneChange);

		sampletabbox->registerWidget(nssamplevolume, 0, 2);
		sampletabbox->registerWidget(nspanning, 0, 2);
		sampletabbox->registerWidget(nsrelnote, 0, 2);
		sampletabbox->registerWidget(nsfinetune, 0, 2);
		sampletabbox->registerWidget(labelfinetune, 0, 2);
		sampletabbox->registerWidget(labelrelnote, 0, 2);
		sampletabbox->registerWidget(labelsamplevolume, 0, 2);
		sampletabbox->registerWidget(labelpanning, 0, 2);
	// </Sample settings>

	// <Looping>
		gbsampleloop = new GroupBox(19, 99, 110, 32, &sub_vram);
		gbsampleloop->setText("loop type");

		rbg_sampleloop = new RadioButton::RadioButtonGroup();

		rbloop_none     = new RadioButton(21, 110, 40, 10, &sub_vram, rbg_sampleloop);
		rbloop_forward  = new RadioButton(68, 110, 40, 10, &sub_vram, rbg_sampleloop);
		rbloop_pingpong = new RadioButton(21, 120, 40, 10, &sub_vram, rbg_sampleloop);

		rbloop_none->setCaption("none");
		rbloop_forward->setCaption("forward");
		rbloop_pingpong->setCaption("ping-pong");

		rbloop_none->setActive(true);

		rbg_sampleloop->registerChangeCallback(handleSampleLoopChanged);

		cbsnapto0xing = new CheckBox(50, 134, 77, 10, &sub_vram, true, true);
		cbsnapto0xing->setCaption("snap");
		cbsnapto0xing->registerToggleCallback(handleSnapTo0XingToggled);

		sampletabbox->registerWidget(rbloop_none, 0, 3);
		sampletabbox->registerWidget(rbloop_forward, 0, 3);
		sampletabbox->registerWidget(rbloop_pingpong, 0, 3);
		sampletabbox->registerWidget(gbsampleloop, 0, 3);
		sampletabbox->registerWidget(cbsnapto0xing, 0, 3);
	// </Looping>

	tabbox->registerWidget(sampledisplay, 0, 2);
	tabbox->registerWidget(sampletabbox, 0, 2);
	// </Sample Gui>

	// <Instruments Gui>
		volenvedit = new EnvelopeEditor(5, 24, 130, 72, &sub_vram, MAX_ENV_X, MAX_ENV_Y, MAX_ENV_POINTS);
		volenvedit->registerPointsChangeCallback(volEnvPointsChanged);
		volenvedit->registerDrawFinishCallback(volEnvDrawFinish);

		cbvolenvenabled = new CheckBox(6, 98, 60, 10, &sub_vram, true, false);
		cbvolenvenabled->setCaption("env on");
		cbvolenvenabled->registerToggleCallback(toggleVolEnvEnabled);

		btnaddenvpoint = new Button(72, 100, 30, 10, &sub_vram);
		btnaddenvpoint->setCaption("add");
		btnaddenvpoint->registerPushCallback(addEnvPoint);

		btndelenvpoint = new Button(104, 100, 30, 10, &sub_vram);
		btndelenvpoint->setCaption("del");
		btndelenvpoint->registerPushCallback(delEnvPoint);

		btnenvzoomin = new Button(72, 112, 30, 10, &sub_vram);
		btnenvzoomin->setCaption("+");
		btnenvzoomin->registerPushCallback(envZoomIn);

		btnenvzoomout = new Button(104, 112, 30, 10, &sub_vram);
		btnenvzoomout->setCaption("-");
		btnenvzoomout->registerPushCallback(envZoomOut);

		btnenvdrawmode = new Button(6, 112, 60, 10, &sub_vram);
		btnenvdrawmode->setCaption("draw env");
		btnenvdrawmode->registerPushCallback(envStartDrawMode);

		tbmapsamples = new ToggleButton(5, 125, 80, 12, &sub_vram, false);
		tbmapsamples->setCaption("map samples");
		tbmapsamples->registerToggleCallback(toggleMapSamples);

		tabbox->registerWidget(btnaddenvpoint, 0, 3);
		tabbox->registerWidget(btndelenvpoint, 0, 3);
		tabbox->registerWidget(btnenvzoomin, 0, 3);
		tabbox->registerWidget(btnenvzoomout, 0, 3);
		tabbox->registerWidget(btnenvdrawmode, 0, 3);
		tabbox->registerWidget(cbvolenvenabled, 0, 3);
		tabbox->registerWidget(volenvedit, 0, 3);
		tabbox->registerWidget(tbmapsamples, 0, 3);
	// </Instruments Gui>

	// <Settings Gui>
		gbhandedness = new GroupBox(5, 23, 84, 25, &sub_vram);
		gbhandedness->setText("handedness");

		rbghandedness = new RadioButton::RadioButtonGroup();
		rblefthanded  = new RadioButton(7 , 35, 40, 14, &sub_vram, rbghandedness);
		rblefthanded->setCaption("left");
		rbrighthanded = new RadioButton(47, 35, 40, 14, &sub_vram, rbghandedness);
		rbrighthanded->setCaption("right");
		rbghandedness->setActive(1);
		rbghandedness->registerChangeCallback(handleHandednessChange);

		gbdsmw = new GroupBox(5, 55, 84, 54, &sub_vram);
		gbdsmw->setText("dsmidiwifi");

		btndsmwtoggleconnect = new Button(12, 67, 71, 14, &sub_vram);
		btndsmwtoggleconnect->setCaption("connect");

		cbdsmwsend = new CheckBox(7, 83, 40, 14, &sub_vram, true, true);
		cbdsmwsend->setCaption("send");

		cbdsmwrecv = new CheckBox(7, 97, 40, 14, &sub_vram, true, true);
		cbdsmwrecv->setCaption("receive");

#ifdef WIFI
		btndsmwtoggleconnect->registerPushCallback(dsmiToggleConnect);
		cbdsmwsend->registerToggleCallback(handleDsmiSendToggled);
		cbdsmwrecv->registerToggleCallback(handleDsmiRecvToggled);
#endif
		tabbox->registerWidget(rblefthanded, 0, 4);
		tabbox->registerWidget(rbrighthanded, 0, 4);
		tabbox->registerWidget(cbdsmwsend, 0, 4);
		tabbox->registerWidget(cbdsmwrecv, 0, 4);
		tabbox->registerWidget(btndsmwtoggleconnect, 0, 4);
		tabbox->registerWidget(gbhandedness, 0, 4);
		tabbox->registerWidget(gbdsmw, 0, 4);
	// </Settings Gui>

	lbinstruments = new ListBox(141, 33, 114, 89, &sub_vram, MAX_INSTRUMENTS, true, true, false);

	lbsamples = new ListBox(141, 101, 114, 23, &sub_vram, MAX_INSTRUMENT_SAMPLES, true, false, true);

	buttonswitchsub    = new BitButton(233, 1  , 21, 21, &sub_vram, icon_flp_bin, 18, 18);
	buttonplay         = new BitButton(180, 4  , 23, 15, &sub_vram, icon_play_bin, 12, 12, 5, 0, true);
	buttonpause        = new BitButton(180, 4  , 23, 15, &sub_vram, icon_pause_bin, 12, 12, 5, 0, false);
	buttonstop         = new BitButton(204, 4  , 23, 15, &sub_vram, icon_stop_bin, 12, 12, 5, 0);

	buttoninsnote2     = new Button(225, 140, 30, 12, &sub_vram);
	buttondelnote2     = new Button(225, 153, 30, 12, &sub_vram);
	buttonemptynote    = new Button(225, 166, 30, 12, &sub_vram);
	buttonstopnote     = new Button(225, 179, 30, 12, &sub_vram);
	buttonrenamesample = new Button(141, 125 , 23, 12, &sub_vram, false);
	buttonrenameinst   = new Button(141, 20 , 23, 12, &sub_vram);

	tbmultisample      = new ToggleButton(165, 21, 10, 10, &sub_vram);

	cbloop = new CheckBox(178, 19, 30, 12, &sub_vram, true, false, true);

	cbloop->setCaption("loop ptn");
	cbloop->registerToggleCallback(handleLoopToggle);

	tbrecord = new ToggleButton(140, 138, 44, 14, &sub_vram);
	tbrecord->setCaption("record!");
	tbrecord->registerToggleCallback(setRecordMode);

	labeladd = new Label(182, 126, 22, 12, &sub_vram, false, true);
	labeladd->setCaption("add");
	labeloct = new Label(206, 126, 25, 12, &sub_vram, false, true);
	labeloct->setCaption("oct");
	numberboxadd    = new NumberBox(185, 135, 18, 17, &sub_vram, state->add, 0, 8, 1);
	numberboxoctave = new NumberBox(206, 135, 18, 17, &sub_vram, state->basenote/12, 0, 6, 1);

	buttonswitchsub->registerPushCallback(switchScreens);
	buttonplay->registerPushCallback(startPlay);
	buttonstop->registerPushCallback(stopPlay);
	buttonpause->registerPushCallback(pausePlay);

	buttoninsnote2->registerPushCallback(insNote);
	buttondelnote2->registerPushCallback(delNote);
	buttonemptynote->registerPushCallback(emptyNoteStroke);
	buttonstopnote->registerPushCallback(stopNoteStroke);
	buttonrenameinst->registerPushCallback(showTypewriterForInstRename);
	buttonrenamesample->registerPushCallback(showTypewriterForSampleRename);

	tbmultisample->registerToggleCallback(handleToggleMultiSample);

	numberboxadd->registerChangeCallback(changeAdd);
	numberboxoctave->registerChangeCallback(changeOctave);

	lbinstruments->registerChangeCallback(handleInstChange);
	lbsamples->registerChangeCallback(handleSampleChange);

	buttoninsnote2->setCaption("ins");
	buttondelnote2->setCaption("del");
	buttonemptynote->setCaption("clr");
	buttonstopnote->setCaption("--");
	buttonrenameinst->setCaption("ren");
	buttonrenamesample->setCaption("ren");

	tbmultisample->setCaption("+");

	// <Main Screen>
		buttonswitchmain = new BitButton(233, 1  , 21, 21, &main_vram_back, icon_flp_bin, 18, 18);
		buttonswitchmain->registerPushCallback(switchScreens);

		labelmute = new Label(226, 23, 32, 8, &main_vram_back, false, true);
		labelmute->setCaption("mute");

		buttonunmuteall = new Button(225, 32, 30, 12, &main_vram_back);
		buttonunmuteall->setCaption("none");

		labelnotevol = new Label(230, 44, 23, 10, &main_vram_back, false, true);
		labelnotevol->setCaption("vol");

		nsnotevolume	 = new NumberSlider(225, 54, 30, 17, &main_vram_back, 127, 0, 127, true);
		nsnotevolume->registerChangeCallback(handleNoteVolumeChanged);

		buttonsetnotevol = new Button(225, 70, 30, 12, &main_vram_back);
		buttonsetnotevol->setCaption("set");
		buttonsetnotevol->registerPushCallback(handleSetNoteVol);

		buttoncut         = new Button(225,  86, 30, 12, &main_vram_back);
		buttoncopy        = new Button(225,  99, 30, 12, &main_vram_back);
		buttonpaste       = new Button(225, 112, 30, 12, &main_vram_back);

		buttoncut->setCaption("cut");
		buttoncopy->setCaption("cp");
		buttonpaste->setCaption("pst");

		buttoncolselect   = new Button(225, 125, 30, 12, &main_vram_back);
		buttoninsnote     = new Button(225, 140, 30, 12, &main_vram_back);
		buttondelnote     = new Button(225, 153, 30, 12, &main_vram_back);
		buttonemptynote2  = new Button(225, 166, 30, 12, &main_vram_back);
		buttonstopnote2   = new Button(225, 179, 30, 12, &main_vram_back);

		buttonunmuteall->registerPushCallback(handleUnmuteAll);
		buttoncut->registerPushCallback(handleCut);
		buttoncopy->registerPushCallback(handleCopy);
		buttonpaste->registerPushCallback(handlePaste);
		buttoncolselect->registerPushCallback(handleButtonColumnSelect);
		buttoninsnote->registerPushCallback(insNote);
		buttondelnote->registerPushCallback(delNote);
		buttonemptynote2->registerPushCallback(emptyNoteStroke);
		buttonstopnote2->registerPushCallback(stopNoteStroke);

		buttonstopnote2->setCaption("--");
		buttoncolselect->setCaption("sel");
		buttoninsnote->setCaption("ins");
		buttondelnote->setCaption("del");
		buttonemptynote2->setCaption("clr");

		pv = new PatternView(0, 0, 200, 192, &main_vram_back, state);
		pv->setSong(song);
		pv->registerMuteCallback(handleMuteChannelsChanged);

		gui->registerWidget(labelmute, 0, MAIN_SCREEN);
		gui->registerWidget(buttonunmuteall, 0, MAIN_SCREEN);
		gui->registerWidget(buttonswitchmain, 0, MAIN_SCREEN);
		gui->registerWidget(labelnotevol, 0, MAIN_SCREEN);
		gui->registerWidget(nsnotevolume, 0, MAIN_SCREEN);
		gui->registerWidget(buttonsetnotevol, 0, MAIN_SCREEN);
		gui->registerWidget(buttoncut, 0, MAIN_SCREEN);
		gui->registerWidget(buttoncopy, 0, MAIN_SCREEN);
		gui->registerWidget(buttonpaste, 0, MAIN_SCREEN);
		gui->registerWidget(buttoncolselect, 0, MAIN_SCREEN);
		gui->registerWidget(buttoninsnote, 0, MAIN_SCREEN);
		gui->registerWidget(buttondelnote, 0, MAIN_SCREEN);
		gui->registerWidget(buttonemptynote2, 0, MAIN_SCREEN);
		gui->registerWidget(buttonstopnote2, 0, MAIN_SCREEN);
		gui->registerWidget(pv, 0, MAIN_SCREEN);
	// </Main Screen>
		
	gui->registerWidget(buttonswitchsub, 0, SUB_SCREEN);
	gui->registerWidget(buttonplay, 0, SUB_SCREEN);
	gui->registerWidget(buttonstop, 0, SUB_SCREEN);
	gui->registerWidget(buttonpause, 0, SUB_SCREEN);
	gui->registerWidget(buttonemptynote, 0, SUB_SCREEN);
	gui->registerWidget(buttoninsnote2, 0, SUB_SCREEN);
	gui->registerWidget(buttondelnote2, 0, SUB_SCREEN);
	gui->registerWidget(buttonrenameinst, 0, SUB_SCREEN);
	gui->registerWidget(buttonrenamesample, 0, SUB_SCREEN);
	gui->registerWidget(tbmultisample, 0, SUB_SCREEN);
	gui->registerWidget(cbloop, 0, SUB_SCREEN);
	gui->registerWidget(numberboxadd, 0, SUB_SCREEN);
	gui->registerWidget(numberboxoctave, 0, SUB_SCREEN);
	gui->registerWidget(labeladd, 0, SUB_SCREEN);
	gui->registerWidget(labeloct, 0, SUB_SCREEN);
	gui->registerWidget(kb, 0, SUB_SCREEN);
	gui->registerWidget(buttonstopnote, 0, SUB_SCREEN);
	gui->registerWidget(tbrecord, 0, SUB_SCREEN);
	gui->registerWidget(pixmaplogo, 0, SUB_SCREEN);
	gui->registerWidget(tabbox, 0, SUB_SCREEN);
	gui->registerWidget(lbinstruments, 0, SUB_SCREEN);
	gui->registerWidget(lbsamples, 0, SUB_SCREEN);

	gui->revealAll();

	updateTempoAndBpm();

	gui->drawSubScreen(); // GUI
	drawMainScreen(); // Pattern view. The function also flips buffers
}

void move_to_bottom(void)
{
	state->row = song->getPatternLength(song->getPotEntry(state->potpos))-1;
	pv->updateSelection();
	drawMainScreen();
}

void move_to_top(void)
{
	state->row = 0;
	pv->updateSelection();
	drawMainScreen();
}

// Update the state for certain keypresses
void handleButtons(u16 buttons, u16 buttonsheld)
{
	u16 ptnlen = song->getPatternLength(song->getPotEntry(state->potpos));

	if(!(buttonsheld & mykey_R))
	{
		if(buttons & mykey_UP)
		{
			int newrow = state->row;

			if(fastscroll == false) {
				newrow--;
			} else
			{
				newrow -= 4;
			}
			while(newrow < 0)
				newrow += ptnlen;

			state->row = newrow;

			pv->updateSelection();
			drawMainScreen();

		}
		else if(buttons & mykey_DOWN)
		{
			int newrow = state->row;

			if(fastscroll == false){
				newrow++;
			} else {
				newrow += 4;
			}

			newrow %= ptnlen;

			state->row = newrow;

			pv->updateSelection();
			drawMainScreen();
		}
	}

	if((buttons & mykey_LEFT)&&(!typewriter_active))
	{
		if(state->channel>0) {
			state->channel--;
			pv->updateSelection();
			need_to_redraw = true;
		}
	}
	else if((buttons & mykey_RIGHT)&&(!typewriter_active))
	{
		if(state->channel < song->getChannels()-1)
		{
			state->channel++;
			pv->updateSelection();
			need_to_redraw = true;
		}
	}
	else if(buttons & KEY_START)
	{
#ifdef DEBUG
		iprintf("\x1b[2J"); //was: consoleClear();
#else
		if( (state->playing == false) || (state->pause == true) )
			startPlay();
		else
			pausePlay();
#endif
	}
	else if(buttons & KEY_SELECT)
	{
#ifdef DEBUG
		PrintFreeMem();
		printMallInfo();
#else
		stopPlay();
#endif
	}
}

void VblankHandler(void)
{
	// Check input
	scanKeys();
	u16 keysdown = keysDown() | (keysDownRepeat() & keys_that_are_repeated);
	u16 keysup = keysUp();
	u16 keysheld = keysHeld();
	touchRead(&touch);

	if(keysdown & KEY_TOUCH)
	{
		gui->penDown(touch.px, touch.py);
		drawMainScreen();
	}

	if(keysup & KEY_TOUCH)
	{
		gui->penUp(touch.px, touch.py);
		lastx = -255;
		lasty = -255;
	}

	if( (keysheld & KEY_TOUCH) && ( (abs(touch.px - lastx)>0) || (abs(touch.py - lasty)>0) ) ) // PenMove
	{
		gui->penMove(touch.px, touch.py);
		lastx = touch.px;
		lasty = touch.py;
		if(gui->getActiveScreen() == MAIN_SCREEN)
			drawMainScreen();
	}

	if(keysheld & mykey_R)
	{
		if(keysheld & mykey_DOWN)
			move_to_bottom();
		else if(keysheld & mykey_UP)
			move_to_top();
	}

	if(keysdown & ~KEY_TOUCH)
	{
		if((keysdown & mykey_X)||(keysdown & mykey_L)) {
			switchScreens();
		}

		if(keysdown & mykey_B) {
			fastscroll = true;
		}

		gui->buttonPress(keysdown);
		handleButtons(keysdown, keysheld);
		pv->pleaseDraw();
	}

	if(keysup)
	{
		gui->buttonRelease(keysup);

		if(keysup & mykey_B)
			fastscroll = false;
	}

	// Update pattern view if necessary
	if( need_to_redraw && ( (state->recording == false) || ( (state->recording == true) && (frame == 0) ) ) )
	{
		need_to_redraw = false;
		drawMainScreen();
	}

	frame = (frame + 1) % 2;
}

extern "C" void debug_print_stub(char *string)
{
#ifdef DEBUG
	printf("%s", string);
#endif
}

void fadeIn(void)
{
	for(int i=-16; i <= 0; ++i)
	{
	    setBrightness(3, i);
		swiWaitForVBlank();
	}
}

#ifdef DEBUG
void saveScreenshot(void)
{
	iprintf("Saving screenshot\n");
	u8 *screenbuf = (u8*)malloc(256*192*3*2);

	u16 col;
	for(u32 i=0;i<192*256;++i) {
		col = main_vram_front[i];
		screenbuf[3*i+0] = (col & 63) << 3;
		col >>= 5;
		screenbuf[3*i+1] = (col & 63) << 3;
		col >>= 5;
		screenbuf[3*i+2] = (col & 63) << 3;
	}
	for(u32 i=192*256;i<2*192*256;++i) {
		col = sub_vram[i-192*256];
		screenbuf[3*i+0] = (col & 63) << 3;
		col >>= 5;
		screenbuf[3*i+1] = (col & 63) << 3;
		col >>= 5;
		screenbuf[3*i+2] = (col & 63) << 3;
	}

	static u8 filenr = 0;
	char filename[255] = {0};
	sprintf(filename, "scr%02d.rgb", filenr);

	FILE *fileh;
	fileh = fopen(filename, "w");
	fwrite(screenbuf, 256*192*3*2, 1, fileh);
	fclose(fileh);

	free(screenbuf);
	iprintf("saved\n");

	filenr++;
}

void dumpSample(void)
{
	static u8 smpfilenr = 0;
	char filename[255] = {0};
	sprintf(filename, "smp%02d.raw", smpfilenr);

	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;
	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;
	void *data = smp->getData();
	u32 size = smp->getSize();

	iprintf("saving sample\n");

	FILE *fileh;
	fileh = fopen(filename, "w");
	fwrite(data, size, 1, fileh);
	fclose(fileh);

	iprintf("saved\n");
}
#endif

void applySettings(void)
{
	bool handedness = settings->getHandedness();
	if(handedness == LEFT_HANDED)
		rbghandedness->setActive(0);
	else
		rbghandedness->setActive(1);

	bool samplepreview = settings->getSamplePreview();
	cbsamplepreview->setChecked(samplepreview);

	if(rbsong->getActive() == true)
	{
		fileselector->setDir(settings->getSongPath());
	}
	else if(rbsample->getActive() == true)
	{
		fileselector->setDir(settings->getSamplePath());
	}
}

typedef struct
{
	int argc;
	char** argv;
} dirModeParams;

static int dirModeMain(void* __arg);

int main(int argc, char* argv[])
{
	if (argc > 2)
	{
		printf("usage: %s [xmFile]\n", argv[0]);
		return 0;
	}

	if (!ntxmInstallARM7())
	{
		printf("Can't install NTXM7!\n");
		return 1;
	}

	dirModeParams p = { argc, argv };

	if (DSRequestHardware(dirModeMain, &p, NULL) < 0)
		return 1;

	CommandDeinit();
	ntxmUninstallARM7();

	delete state;
	delete settings;
	delete song;
	clipboard_free();

	delete gui;
	return 0;
}

//---------------------------------------------------------------------------------
int dirModeMain(void* __arg) {
//---------------------------------------------------------------------------------
	auto& _arg = *(dirModeParams*)__arg;
	int argc = _arg.argc;
	char** argv = _arg.argv;

	// Hide everything
#ifndef DEBUG
	setBrightness(3, 16);
#endif

	powerOn(POWER_ALL);

	// Adjust screens so that the main screen is the top screen
	lcdMainOnTop();

	// Main screen: Text and double buffer ERB
	videoSetMode(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);

	// Sub screen: Keyboard tiles, Typewriter tiles and ERB
	videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE | DISPLAY_BG2_ACTIVE);

	vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
	           VRAM_C_SUB_BG_0x06200000 , VRAM_D_LCD);

	// SUB_BG0 for Piano Tiles
	int piano_bg = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 1, 0);
	bgSetScroll(piano_bg, 0, 0);
	bgSetPriority(piano_bg, 2);

	// SUB_BG1 for Typewriter Tiles
	int typewriter_bg = bgInitSub(1, BgType_Text4bpp, BgSize_T_256x256, 12, 1);
	bgSetPriority(typewriter_bg, 0);

	// Pattern view
	int ptn_bg = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bgSetPriority(ptn_bg, 0);

	// Sub screen framebuffer
	int sub_bg = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bgSetPriority(sub_bg, 0);

	// Special effects
	REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG2 | BLEND_SRC_BG0;
	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG2 | BLEND_SRC_BG0;

	swiWaitForVBlank();

	// Set draw loactions for the ERBs
	main_vram_front = (uint16*)BG_BMP_RAM(2);
	main_vram_back = (uint16*)BG_BMP_RAM(8);
	sub_vram  = (uint16*)BG_BMP_RAM_SUB(2);

	// Clear tile mem
	for(u32 i=0; i<(32*1024); ++i) {
		((u16*)BG_BMP_RAM_SUB(0))[i] = 0;
	}

	irqEnable(IRQ_VBLANK);

	state = new State();

	settings = new Settings(true);

	clearMainScreen();
	clearSubScreen();

#ifdef WIFIDEBUG
	// Wireless debugging
	set_verbosity(VERBOSE_INFO | VERBOSE_ERROR);//OR together VERBOSE_INFO/VERBOSE_ERROR/VERBOSE_TRACE to get the level of detail right. I'd recommend VERBOSE_INFO | VERBOSE_ERROR, or VERBOSE_ERROR at the very least

	iprintf("init wifi\n");
	wireless_init(0);	//zero if you've already set up your irqs, non-zero if you haven't yet called irqInit()
	iprintf("connect via wifi\n");
	wireless_connect();                     //connect to the AP

	iprintf("debugger connect\n");
	debugger_connect_tcp(192, 168, 1, 33);	//your IP here
	iprintf("debugger init\n");
	debugger_init();
#endif

	// Init interprocessor communication
	CommandInit();
	RegisterRowCallback(handleNewRow);
	RegisterStopCallback(handleStop);
	RegisterPlaySampleFinishedCallback(handlePreviewSampleFinished);
	RegisterPotPosChangeCallback(handlePotPosChangeFromSong);

	setupSong();

	CommandSetSong(song);

	setupGUI();

	applySettings();
#ifndef DEBUG
	fadeIn();
#endif
	keysSetRepeat(15, int(59.826 / double(REPEAT_FREQ) + 0.5));

#ifdef DEBUG
	iprintf("NitroTracker debug build.\nBuilt %s %s\n<Start> clears messages.\n", __DATE__, __TIME__);
#endif

	if (argc == 2)
		loadSong(argv[1]);

	while(!didExit)
	{
		VblankHandler();
#ifdef WIFIDEBUG
		user_debugger_update();
#endif

#ifdef WIFI
		if( state->dsmi_connected ) {
			handleDSMWRecv();
		}
#endif
		swiWaitForVBlank();
	}

	return 0;
}
