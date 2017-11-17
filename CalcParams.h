//---------------------------------------------------------------------------

#ifndef calcParamsH
#define calcParamsH
//---------------------------------------------------------------------------
#include "MainForm.h"
#include "RecordThrd.h"
//---------------------------------------------------------------------------

void Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
			   __int32 *inds, short expT, bool newNull, sPr *avrP);
void AmplitudeCalculate(short *signal, __int32 shft1, __int32 indBgn, __int32 postSampl, short porog, short expTp);
__int32 RiseFallTime(short *signal, __int32 shft1, __int32 shft2, __int32 indBgn, __int32 postSampl);
void SpecParams1(short *signal, __int32 shft1, __int32 shft2, __int32 indBgn, __int32 preSampl, __int32 postSampl,
				 bool overPorog, __int32 ind20);
float IntegralCalculate(trac *signal, __int32 intgrRecLen, float voltTime, __int32 bgnSig);
void SmoothBeforCalc(trac *smoothSig1, __int32 indBgn, __int32 preSampl, __int32 postSampl);
void ReCalcNulls(short *allNulls, short *signal, __int32 nSigs, __int32 postSampl);

#endif
