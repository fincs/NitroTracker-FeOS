// Extra ARM7-care-taker code

#include <feos.h>
#include "ntxm/ntxm9.h"
#include <sndlock.h>

int refcount;
int FIFO_NTXM;
instance_t hARM7;
sndlock_t hSndLock;

bool ntxmInstallARM7()
{
	if (++refcount == 1)
	{
		hARM7 = nullptr;
		if (SndLock_Acquire("ntxm9", &hSndLock))
			hARM7 = FeOS_LoadARM7("/data/FeOS/arm7/ntxm7.fx2", &FIFO_NTXM);

		if (!hARM7)
			refcount --;
		return !!hARM7;
	}
	return true;
}

void ntxmUninstallARM7()
{
	if (--refcount == 0)
	{
		FeOS_FreeARM7(hARM7, FIFO_NTXM);
		SndLock_Release(hSndLock);
	}
}
