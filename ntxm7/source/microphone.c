/*---------------------------------------------------------------------------------
	$Id: microphone.c,v 1.4 2005/09/07 18:06:27 wntrmute Exp $
 
	Microphone control for the ARM7
 
  Copyright (C) 2005
			Michael Noland (joat)
			Jason Rogers (dovoto)
			Dave Murphy (WinterMute)
			Chris Double (doublec)
 
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.
 
  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:
 
  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.
 
	$Log: microphone.c,v $
	Revision 1.4  2005/09/07 18:06:27  wntrmute
	use new register names
	
	Revision 1.3  2005/08/23 17:06:10  wntrmute
	converted all endings to unix
 
	Revision 1.2  2005/07/14 08:00:57  wntrmute
	resynchronise with ndslib
 
	Revision 1.1  2005/07/12 17:32:20  wntrmute
	added microphone functions
 
---------------------------------------------------------------------------------*/


/*
Modified by Tob.
This version sets the mic gain register and reads 12 bit samples instead of 8.
=> Much louder and clearer recording.
*/

#include <nds.h>
#include <nds/arm7/audio.h>
#include <nds/timers.h>
#include "ntxm/tobmic.h"


//---------------------------------------------------------------------------------
void tob_MIC_On()
{
//---------------------------------------------------------------------------------
	tob_PM_SetAmp(PM_AMP_ON);
	tob_PM_SetGain(3);
}

//---------------------------------------------------------------------------------
void tob_MIC_Off()
{
//---------------------------------------------------------------------------------
	tob_PM_SetAmp(PM_AMP_OFF);
}

//---------------------------------------------------------------------------------
// Turn on the Microphone Amp. Code based on neimod's example.
//---------------------------------------------------------------------------------
void tob_PM_SetAmp(u8 control)
{
//---------------------------------------------------------------------------------
	SerialWaitBusy();
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz | SPI_CONTINUOUS;
	REG_SPIDATA = PM_AMP_OFFSET;

	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz;
	REG_SPIDATA = control;

	SerialWaitBusy();
}

//---------------------------------------------------------------------------------
void tob_PM_SetGain(u8 gain)
{
//---------------------------------------------------------------------------------
	SerialWaitBusy();
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz | SPI_CONTINUOUS;
	REG_SPIDATA = PM_GAIN_OFFSET;

	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz;
	REG_SPIDATA = gain;

	SerialWaitBusy();
}

//---------------------------------------------------------------------------------
// Read a byte from the microphone. Code based on neimod's example.
//---------------------------------------------------------------------------------
/*
u8 tob_MIC_ReadData8()
{
//---------------------------------------------------------------------------------
	static u16 result, result2;

	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_MICROPHONE | SPI_BAUD_2MHz | SPI_CONTINUOUS;
	REG_SPIDATA = 0xEC;  // Touchscreen command format for AUX

	SerialWaitBusy();

	REG_SPIDATA = 0x00;

	SerialWaitBusy();

	result = REG_SPIDATA;
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_TOUCH | SPI_BAUD_2MHz;
	REG_SPIDATA = 0x00;

	SerialWaitBusy();

	result2 = REG_SPIDATA;

	return (((result & 0x7F) << 1) | ((result2>>7)&1));
}
*/
//---------------------------------------------------------------------------------
// Read a byte from the microphone. Code based on neimod's example.
//---------------------------------------------------------------------------------
u16 tob_MIC_ReadData12()
{
//---------------------------------------------------------------------------------
	static u16 result, result2;

	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_MICROPHONE | SPI_BAUD_2MHz | SPI_CONTINUOUS;
	REG_SPIDATA = 0xE4;  // Touchscreen command format for AUX, 12bit

	SerialWaitBusy();

	REG_SPIDATA = 0x00;

	SerialWaitBusy();

	result = REG_SPIDATA;
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_TOUCH | SPI_BAUD_2MHz;
	REG_SPIDATA = 0x00;

	SerialWaitBusy();

	result2 = REG_SPIDATA;

	return (((result & 0x7F) << 5) | ((result2>>3)&0x1F));
}

static u16* microphone_buffer = 0;
static int microphone_buffer_length = 0;
static int current_length = 0;


//---------------------------------------------------------------------------------
void tob_StartRecording(u16* buffer, int length)
{
//---------------------------------------------------------------------------------
	microphone_buffer = buffer;
	microphone_buffer_length = length;
	current_length = 0;

	// tob_MIC_On(); // Commented out because the mic is turned on before manually
	
	// Setup a timer
	TIMER1_DATA = TIMER_FREQ(16384);
	TIMER1_CR = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;
	//irqDisable(IRQ_VBLANK | IRQ_VCOUNT);
}

//---------------------------------------------------------------------------------
int tob_StopRecording()
{
//---------------------------------------------------------------------------------
	TIMER1_CR &= ~TIMER_ENABLE;
	//tob_MIC_Off(); // Commted out because the mic is turned off manually
	microphone_buffer = 0;
	//irqEnable(IRQ_VBLANK | IRQ_VCOUNT);
	return current_length*2; // *2 because of 16 bit
}

//---------------------------------------------------------------------------------
void tob_ProcessMicrophoneTimerIRQ()
{
//---------------------------------------------------------------------------------
	if(microphone_buffer && microphone_buffer_length > 0) {
		// Read data from the microphone. Data from the Mic is unsigned, flipping
		// the highest bit makes it signed.
		*microphone_buffer++ = (tob_MIC_ReadData12() << 4) ^ 0x8000; // Read, make signed, make louder
		--microphone_buffer_length;
		current_length++;
	}
}
