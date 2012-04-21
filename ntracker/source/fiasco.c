#include <feos.h>

#define MAX_OBJ 16

typedef void (*dtorFn)(void* obj);

static void* objList[MAX_OBJ];
static dtorFn dtorList[MAX_OBJ];
static int listPos = 0;

void* __dso_handle;
int __aeabi_atexit(void* obj, dtorFn destructor, void* dsoHandle)
{
	if (listPos == MAX_OBJ) return 0;
	objList[listPos] = obj;
	dtorList[listPos++] = destructor;
	return 1; // 1 = success, 0 = failure
}

FEOSFINI static void runDtors()
{
	int i;
	for (i = 0; i < listPos; i ++)
		dtorList[i](objList[i]);
}
