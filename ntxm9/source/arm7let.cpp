// Extra ARM7-care-taker code

#include <feos.h>
#include "ntxm/ntxm9.h"

int refcount;
int FIFO_NTXM;
instance_t hARM7;

bool ntxmInstallARM7()
{
	if (++refcount == 1)
	{
		hARM7 = FeOS_LoadARM7("/data/FeOS/arm7/ntxm7.fx2", &FIFO_NTXM);
		return !!hARM7;
	}
	return true;
}

void ntxmUninstallARM7()
{
	if (--refcount == 0)
		FeOS_FreeARM7(hARM7, FIFO_NTXM);
}
