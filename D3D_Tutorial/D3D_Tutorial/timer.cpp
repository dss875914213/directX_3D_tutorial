#include "timer.h"

long MyGetTickCount()
{
	static BOOL init = FALSE;
	static BOOL hires = FALSE;
	static _int64 freq = 1;
	if (!init)
	{
		hires = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		if (!hires)
			freq = 1000;
		init = TRUE;
	}
	_int64 now;
	if (hires)
		QueryPerformanceCounter((LARGE_INTEGER*)&now);
	else
		now = GetTickCount();
	return (long)(1000.0f * (double)now / (double)freq);
}

long GetTickCountDIFF(long lBegin, long lNow)
{
	if (0 == lBegin)
		return 0;
	if (0 == lNow)
		lNow = MyGetTickCount();
	long lDiff = lNow - lBegin;
	if (lDiff < 0)
		lDiff = 0xFFFFFFFF - lBegin + lNow + 1;
	return lDiff;
}


