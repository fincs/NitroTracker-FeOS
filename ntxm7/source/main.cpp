#include <nds.h>
#include <ntxm/ntxm7.h>
extern "C"
{
#include <ntxm/tobmic.h>
}

int FIFO_NTXM;
NTXM7* ntxm7;

static void ntxmTimerHandler()
{
	ntxm7->timerHandler();
}

extern "C" int arm7_main(int fifoCh)
{
	FIFO_NTXM = fifoCh;
	ntxm7 = new NTXM7(ntxmTimerHandler);
	if (!ntxm7) return 1;

	powerOn(POWER_SOUND);
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	REG_SOUNDCNT = SOUND_ENABLE;
	REG_MASTER_VOLUME = 127;

	irqSet(IRQ_TIMER0, ntxmTimerHandler);
	irqSet(IRQ_TIMER1, tob_ProcessMicrophoneTimerIRQ);
	irqEnable(IRQ_TIMER0 | IRQ_TIMER1);
	return 0;
}

extern "C" void arm7_fini()
{
	TIMER0_CR = 0;
	TIMER1_CR = 0;
	irqDisable(IRQ_TIMER0 | IRQ_TIMER1);
	irqSet(IRQ_TIMER0, 0);
	irqSet(IRQ_TIMER1, 0);

	for (int i = 0; i < 16; i ++)
		SCHANNEL_CR(i) = 0;

	REG_SOUNDCNT &= ~SOUND_ENABLE;
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_AMP ) | PM_SOUND_MUTE );
	powerOff((PM_Bits)POWER_SOUND);
	delete ntxm7;
}
