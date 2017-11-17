//---------------------------------------------------------------------------

#include <math.h>
#include <algorith.h>

#pragma hdrstop

#include "CalcParams.h"
#include "TimeCourses.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
short *cAmpls;//��� ������� �������� (���������)
		//cAmpls[0] = amplPost - ��������� �������� - ����������������� (����� ������� �����)
		//cAmpls[1] = ampl1NaPeak - ������ ��������� ����������������� ������ (��������� ���) (����� �����, ����� ������� �����)
		//cAmpls[2] = ampl2KPeak - ������ ��������� ����������������� ������ (�������� ���) (����� �����, ����� ������� �����)
__int32 *cPeakInds,//��� ������ �������� � �������� ����������
			//cPeakInds[0] = indMaxPost - ����� ������� � ������������ ��������� amplPost
			//cPeakInds[1] = indMax1Na - ����� ������� � ������������ ��������� amplPre1
			//cPeakInds[2] = indMax2K - ����� ������� � ������������ ��������� amplPre2
		*cSpans,//��� ����������� ������� (�������, ����, ���������)
			//cSpans[0] = riseT - ����� ���������� (������������)
			//cSpans[1] = fallT - ����� ����� (������������)
			//cSpans[2] = t1NaPeak - ������������ ������� ���� (���������� ����, ������������)
			//cSpans[3] = tDelay - ������������� �������� ��� ��������... (������������)
			//cSpans[4] = tSynTrans - ����� �������������� ���������� (������������)
		cRecLen,//����� �������� ������� (�������)
		cFtCh,//���������� ������� � ������� ����������
		cEffDT,//����������� ����� ������������� = discrT * chanls
		cRF[2];//������������������ ������ ������� (0 - ������ ������������ �����, 1 - ��������� ������������ �����)
float cSmpl2vM;//����������� �������� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)

//---------------------------------------------------------------------------

void Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
			   __int32 *inds, short expT, bool newNull, sPr *avrP)
{
	//��������� �������� (������� ����������)
	/*
	sgnl - ��������� � ��������� � �� ����������
	grph - ��������� �� ������ ��������� ������ (TGraphs *)
	numOfS - ���������� ��������
	porog - ����������� ����� (�������)
	preTime - ���-����� (� �������������)
	postTime - ����-����� (� �������������)
	inds - ������ �������� ��������, ������� ����� ����������
	expT - ��� ������������
	newNull - ���������� �� ������ ����-����� (��� ������� ������� ��������, newNull = (PStart->nulMinus->Checked && (PStart->nulMinus->Tag == 1))
	avrP - ��������� �� ������ � ����������� (sPr *)
	*/

	__int32 i, j, z,
			p20p,//������ � ������ 20% ���������
			postSampl,//������, � �������� "���������� �����������������" ������
			preSampl,//������, � �������� "���������� ����������������" ������
			beginSampl,//������, � �������� ���������� �������� ������� ���������� ������ (��� ���� ������������ ���������)
			ftCh,//���������� �������������� ������� � ������� ����������
			shft1,//����� ������� �������� ��� ampls � peakInds
			shft2;//����� ������� �������� ��� spans
	short *allNewNulls;//������ � "������ ������"

	if (avrP)//������ ���������� ��� ����������� �������
	{
		cAmpls = avrP->ampls;//��� ������� �������� (���������)
		cPeakInds = avrP->peakInds;//��� ������ �������� � �������� ����������
		cSpans = avrP->spans;//��� ����������� ������� (�������, ����, ���������)
	}
	else//������ ���������� ��� ���� ��������
	{
		cAmpls = Graphs->cPrm[0].ampls;//��� ������� �������� (���������)
		cPeakInds = Graphs->cPrm[0].peakInds;//��� ������ �������� � �������� ����������
		cSpans = Graphs->cPrm[0].spans;
	}

	cRecLen = Graphs->recLen;//����� �������� ������� � ��������
	cFtCh = Graphs->ftChan;//���������� ������� � ������� ����������
	cEffDT = Graphs->effDT;//����������� ����� ������������� = discrT * chanls
	cRF[0] = Graphs->gRF[0];//������ ������������ �����
	cRF[1] = Graphs->gRF[1];//��������� ������������ �����
	ftCh = cRF[1] - cRF[0];//���������� �������������� ������� � ������� ����������


	//���� ������� "�������� ����-�����" (PStart->nulMinus->Checked), ��
	//����� ��������� ����-����� ��� ������� ������� �������� (PStart->nulMinus->Tag ������ ���� 1)
	if (newNull)
	{
		allNewNulls = new short[numOfS * ftCh];
		postSampl = postTime / cEffDT;
		for (i = 0; i < numOfS; i++)
			for (j = 0; j < ftCh; j++)
			{
				beginSampl = (cRF[0] + j) * cRecLen;//������, � �������� ���������� �������� ������� ���������� ������ (��� ���� ������������ ���������)
				ReCalcNulls(&allNewNulls[(i * ftCh) + j], &sgnl[inds[i]].s[beginSampl], numOfS, postSampl);//���������� ��������� ������� �����
				//���������� ������ � ������������ � ����� ����
				for (z = beginSampl; z < beginSampl + cRecLen; z++)
					sgnl[inds[i]].s[z] -= allNewNulls[(i * ftCh) + j];
			}
	}

	preSampl = preTime / cEffDT;//������, � �������� "���������� ����������������" ������
	postSampl = postTime / cEffDT;//��������� ����-����� � ���������� ��������
	for (i = 0; i < numOfS; i++)
		for (j = cRF[0]; j < cRF[1]; j++)
		{
			beginSampl = j * cRecLen;//������, � �������� ���������� �������� ������� ���������� ������ (��� ���� ������������ ���������)
			shft1 = (3 * cFtCh * inds[i]) + (3 * j);//����� ������� �������� ��� cAmpls � cPeakInds
			shft2 = (5 * cFtCh * inds[i]) + (5 * j);//����� ������� �������� ��� cSpans

			AmplitudeCalculate(sgnl[inds[i]].s, shft1, beginSampl, postSampl, porog, expT);//�������� �������� ���������
			p20p = RiseFallTime(sgnl[inds[i]].s, shft1, shft2, beginSampl, postSampl);//������� ���������� � ����� ��������� �������
			if (expT == 3)//��� ����������� ������������ ��������
				SpecParams1(sgnl[inds[i]].s, shft1, shft2, beginSampl, preSampl, postSampl, (cAmpls[shft1] > porog), p20p);
		}

	if (newNull)
	{
		delete[] allNewNulls; allNewNulls = NULL;
	}
	sgnl = NULL; inds = NULL;
}
//---------------------------------------------------------------------------

void AmplitudeCalculate(short *signal, __int32 shft1, __int32 beginSampl, __int32 postSampl, short porog, short expTp)
{
	//���������� ��������� �������
	/*
	signal - ������ � ��������
	shft1 - ����� ������� �������� ��� cAmpls � cPeakInds
	beginSampl - ������, � �������� ���������� �������� ������� ���������� ������ (��� ���� ������������ ���������)
	postSampl - ���������� ����� ������� ������ ����������������� ����� �������
	porog - ����� "�������" �������
	expTp - ��� ������������
	*/

	__int32 i,
			averagInterv,//�������� ��������� ����������; �������� ����������� "�������" ��������� (� �������������)
			sampCI,//������������� �������� � �������� (� ���)
			leftBoard,//����� ������� �������������� ���������
			rightBoard;//������ ������� �������������� ���������
	float meanAmpl;

	averagInterv = 10;//������������
	sampCI = floor(averagInterv / cEffDT) + 1;//������������� �������� � �������� (� ���)

	//���������� �������� ���������
	cAmpls[shft1] = signal[beginSampl + postSampl];//����������� �������� (������� �� ������� [leftBoard; rightBoard])
	cPeakInds[shft1] = postSampl;//������ ������� � ����������� ��������� (� ���������� ����������)
	for (i = postSampl; i < cRecLen; i++)
	{
		if (signal[beginSampl + i] > cAmpls[shft1])// && (signal[i - 1] <= signal[i]) && (signal[i + 1] <= signal[i]))
		{
			cAmpls[shft1] = signal[beginSampl + i];
			cPeakInds[shft1] = i;
		}
	}

	meanAmpl = 0;//��������� ������� � ������� ����������� ���������
	leftBoard = max(postSampl, cPeakInds[shft1] - sampCI);//����� ������� �������
	rightBoard = min(cRecLen, cPeakInds[shft1] + sampCI);//������ ������� �������
	for (i = leftBoard; i < rightBoard; i++)
		meanAmpl += (float)signal[beginSampl + i];

	cAmpls[shft1] = (short)floor((meanAmpl / (float)(rightBoard - leftBoard)) + 0.5);//����������
	cAmpls[shft1] *= short(cAmpls[shft1] >= porog);//����������� ����� ������� �������

	signal = NULL;
}
//---------------------------------------------------------------------------

__int32 RiseFallTime(short *signal, __int32 shft1, __int32 shft2, __int32 beginSampl, __int32 postSampl)
{
	//������������ ������� ���������� � ����� ��������
	/*
	signal - ������ � ��������
	shft1 - ����� ������� �������� ��� cAmpls � cPeakInds
	shft2 - ����� ������� �������� ��� cSpans
	beginSampl - ����� ������� - ������ ���������� ����� ������
	postSampl - ���������� ����� ������� ������ ����������������� ����� �������
	*/

	short ampl20, ampl50, ampl80;//���������� ��� ������ ����� ���������� � �����
	__int32 i, z,
			ind20,//����� ����� 20% ���������
			ind80,//����� ����� 80% ���������
			ind50Fall;//����� ����� ����� 50% ���������
			//ind20Fall;//����� ����� ����� 20% ���������

	ind20 = 0; ind80 = 0;
	ind50Fall = 0; //ind20Fall = 0;

	//����������� ���������. ������ ����������� ������� ��������������� amplPost (amplPost > 0)
	ampl20 = (short)floor(((float)cAmpls[shft1] * 0.2) + 0.5);//���������� ��� ������������� ����� (��� ��� ����� -0.5)
	ampl50 = (short)floor(((float)cAmpls[shft1] * 0.5) + 0.5);
	ampl80 = (short)floor(((float)cAmpls[shft1] * 0.8) + 0.5);

	//������������ ����� ���������� (����� ���������� = t(80%) - t(20%))
	for (i = postSampl; (i < cPeakInds[shft1]) && (i < cRecLen - 1); i += 2)
	{
		if ((signal[beginSampl + i] < ampl20) && (signal[beginSampl + i + 1] < ampl20))
			ind20 = i;
		if ((signal[beginSampl + i] < ampl80) && (signal[beginSampl + i + 1] < ampl80))
			ind80 = i;
	}

	//������ ������ �����
	for (i = cPeakInds[shft1]; i < (cRecLen - 1); i += 2)
	{
		if ((signal[beginSampl + i] > ampl50) && (signal[beginSampl + i + 1] > ampl50))
			ind50Fall = i;//������ �������: ��������

		/*if ((signal[beginSampl + i] > ampl20) && (signal[beginSampl + i + 1] > ampl20))
			ind20Fall = i;//������ �������: �����~e-���� */
	}

	if (ind80 > ind20)
	{
		cSpans[shft2] = (ind80 - ind20) * cEffDT;//����� ���������� = t(80%) - t(20%) (������������)
		if (ind50Fall > cPeakInds[shft1])//������ �������: ��������
			cSpans[shft2 + 1] = (ind50Fall - cPeakInds[shft1]) * cEffDT;//����� ��������� = t(50%Fall) - t(100%) (������������)
		else
			cSpans[shft2 + 1] = -1;

		/*if (ind20Fall > cPeakInds[shft1])//������ �������: �����~e-����
			cSpans[shft2 + 1] = (ind20Fall - cPeakInds[shft1]) * cEffDT;//����� �����~�-����� = t(20%Fall) - t(100%) (������������)
		else
			cSpans[shft2 + 1] = -1;*/
	}
	else
	{
		cSpans[shft2] = -1;
		cSpans[shft2 + 1] = -1;
	}
	signal = NULL;//��������� ��������� (������)
	return ind20;//���������� ����� ����� � 20% ���������
}
//---------------------------------------------------------------------------

void SpecParams1(short *signal, __int32 shft1, __int32 shft2, __int32 beginSampl, __int32 preSampl, __int32 postSampl,
				 bool overPorog, __int32 ind20)
{
	//������������ ��������� ��������� ������������ �������� (����������� ���������)
	/*
	signal - ������ � ��������
	shft1 - ����� ������� �������� ��� cAmpls � cPeakInds
	shft2 - ����� ������� �������� ��� cSpans
	beginSampl - ����� ������� - ������ ���������� ����� ������
	preSampl - ���-�����
	postSampl - ����-�����
	overPorog - �������� ������ (true, ���� ������)
	ind20 - ���� ����� ����� ������ 20% ��������� ��������� �������
	*/

	__int32 i, z,
			tB, tE,//�������� ���������� ��� �������� �������������
			psevdP,//��������������
			psevdI;//������������
	short locMean;//��������� ������� (�� ������, ���� ������ ��� ��� ������ ����)
	//float fLocMean;//���� ����� ������� �������
	bool fB, fE;//����� ��� ������ ������ � ����� ����

	if (preSampl <= 0)//������ �� ��������������� ��������
		preSampl = 1;
	if (postSampl <= 0)//������ �� ��������������� ��������
		postSampl = 1;

	locMean = 0;//���� ����� ������� ������� ����� ����

	//�������� ��������� (�����������������, ����� ������� ����� - amplPost) ����������� � AmplitudeCalculate
	//������� ���������� � ����� ������������������ ������ ������������ � RiseFallTime

	//-------------------------------------------------------------------------------------
	//��������� ������ ������������ ampl1NaPeak (���������� ���� ������ ���� �������������)
	//������ postSampl ����� ����� ������������ ������ ������� � �������� ����������
	fB = false;//������������ � ����������� ����-�����
	cAmpls[shft1 + 1] = signal[beginSampl + preSampl];
	cPeakInds[shft1 + 1] = preSampl;
	for (i = (beginSampl + preSampl); i <= (beginSampl + postSampl); i++)
	{
		if ((signal[i] > cAmpls[shft1 + 1]) &&
			(signal[i - 1] <= signal[i]) &&
			(signal[i + 1] <= signal[i]))
		{
			cAmpls[shft1 + 1] = signal[i];//������ ��������� ����������������� ������ (��������� ���) (����� �����, ����� ������� �����) ampl1NaPeak
			cPeakInds[shft1 + 1] = i - beginSampl;//����� ������� � ������ ���������������� ���������� indMax1Na
		}
	}

	cSpans[shft2 + 4] = cPeakInds[shft1 + 1] * cEffDT;//��������� ������ (����� �������������� ����������)

	//��������� ������������ ������� ������� (���������� ����); ����� � ��������
	//�� ���� ��� ����� � ����� �� ����������� ����� ����������� � ���� (��������������,
	//��� ������ ��� � ���������� ���������� ������������)
	tB = 0;//������ ������� ���� (��������� ���)
	tE = 0;//����� ������� ���� (��������� ���)
	fB = false;//����� ������ true, �������� �����
	fE = false;//����� ������ true, �������� �����
	for (i = 1; ((i < (cPeakInds[shft1 + 1] - preSampl)) && (!fB) && (!fE)); i++)
	{
		if (!fE)
		{
			tE = cPeakInds[shft1 + 1] + i;//���� "������ ����" - ����� ����
			fE = ((signal[beginSampl + tE] - locMean) < 0);
		}

		if (!fB)
		{
			tB = cPeakInds[shft1 + 1] - i;//���� "����� ����" - ������ ����
			fB = ((signal[beginSampl + tB] - locMean) < 0);
		}
	}
	if (tE > tB)
		cSpans[shft2 + 2] = (tE - tB) * cEffDT;//������������ ���������� ���� (������������), tNa
	else
		cSpans[shft2 + 2] = -1;

	//-------------------------------------------------------------------------------------------
	//��������� ������ ������������ ampl1KPeak (���������� ������� ���� ������ ���� �������������)
	//�.�. ���������� �������������, �� ������ ����� ���������� �������� �� ��������� ����� indMaxPre1 � indMaxPost
	cAmpls[shft1 + 2] = signal[beginSampl + cPeakInds[shft1 + 1]];//��������
	cPeakInds[shft1 + 2] = cPeakInds[shft1 + 1];//������
	psevdP = signal[beginSampl + postSampl];//��������������
	psevdI = postSampl;//������������
	z = postSampl;//��� ������ ������ �������
	for (i = cPeakInds[shft1 + 1]; i <= postSampl; i++)
	{
		//����� ���� �-���� (���������� ��������) �� ���� Na-����
		if (signal[beginSampl + i] < cAmpls[shft1 + 2])// && (signal[i - 1] >= signal[i]) && (signal[i + 1] >= signal[i]))
		{
			cAmpls[shft1 + 2] = signal[beginSampl + i];
			cPeakInds[shft1 + 2] = i;//����� ������� �� ������ ���������������� ���������� indMax1K
		}

		//����� ���� �-���� (���������� ��������) � ������ ������� (�� ����� postSampl)
		if (signal[beginSampl + z] < psevdP)// && (signal[z - 1] >= signal[i]) && (signal[z + 1] >= signal[i]))
		{
			psevdP = signal[beginSampl + z];
			psevdI = z;//����� ������� �� ������ ���������������� ���������� indMax2K
		}
		z--;
	}

	if (psevdI > cPeakInds[shft1 + 2])//��������� �������� ���������� �� ������ �����
	{
		cPeakInds[shft1 + 2] = floor(((cPeakInds[shft1 + 2] + psevdI) / 2) + 0.5);//������ ��������� �����
		cAmpls[shft1 + 2] = signal[beginSampl + cPeakInds[shft1 + 2]];//�������� ��������� � ��������� �����
	}
	cAmpls[shft1 + 2] *= (-1);//�������� � �������������� �������� ������������� K-����

	//-----------------------------------------------------------------------------------
	if (overPorog)//������ (����������������� �����) ������
	{
		//��������� ������������� �������� (������ ��������)

		/*
		//11111_�������_11111//�� ���� �-���� ��� ����� �� ����� ������ ��������� (������������������) �������
		tE = 0;//����� ������� ���� (��������� ���)
		fE = false;//����� ������ true, �������� �����
		for (i = 1; ((i < postSampl) && (!fE)); i++)
		{
			if (!fE)
			{
				tE = cPeakInds[shft1 + 2] + i;//���� "������ ����" - ����� ����
				fE = ((signal[beginSampl + tE] - locMean) > 0);
			}
		}
		if (tE > cPeakInds[shft1 + 1])
			cSpans[shft2 + 3] = (tE - cPeakInds[shft1 + 1]) * cEffDT;//������������� ��������
		else
			cSpans[shft2 + 3] = -1; */

		//22222_�������_22222//�� ���� Na-���� �� ����� 20% ��������� �������� ���� (������������������ �������)
		if (ind20 > cPeakInds[shft1 + 1])
			cSpans[shft2 + 3] = (ind20 - cPeakInds[shft1 + 1]) * cEffDT;//�� ����� 20% ��������� �������� ����
		else
			cSpans[shft2 + 3] = -1;

		/*
		//33333_�������_33333//�� ���� Na-���� �� ���� �������� ���� (������������������ �������)
		if (cPeakInds[shft1] > cPeakInds[shft1 + 1])
			cSpans[shft2 + 3] = (cPeakInds[shft1] - cPeakInds[shft1 + 1]) * cEffDT;//�� ���� �������� ����
		else
			cSpans[shft2 + 3] = -1; */
	}
	else
		cSpans[shft2 + 3] = -1;

	signal = NULL;//�� ������ ������ �������� ���������
}
//---------------------------------------------------------------------------

float IntegralCalculate(trac *signal, __int32 intgrRecLen, float voltTime, __int32 bgnSig)
{
	//���������� ��������� �������
	/*
	signal - ������ �� ������ � ��������
	intgrRecLen - ����� �������������� ����� ������� � �������� (� ������ ����-�������)
	voltTime - ����������� �������� � ������������ ������� ��������� (sampl2mV * discrT * chanls = [��� * ��])
	bgnSig - ����� �������, � �������� ���������� �������������� ������ � ������ ������ (� ������ ���������������� � ����-�������)
	*/

	__int32 i;
	float integr;//������� ��� �������� ��*�� (���������� �� ������������)

	integr = 0;
	for (i = bgnSig; i < (bgnSig + intgrRecLen); i++)
		integr += (float)signal->s[i];

	integr *= (voltTime * 1e-3);//��������� ������������ ������� ��������� (��� * ��)

	return integr;
}
//---------------------------------------------------------------------------

void SmoothBeforCalc(trac *smoothSig, __int32 beginSampl, __int32 preSampl, __int32 postSampl)
{
	//����������� ������� ����� �������� ����������
	/*
	smoothSig - ��������� �� ���������� (������������) ��������
	beginSampl - ����� ������� - ������ ���������� ����� ������
	preSampl - ���-�����
	postSampl - ����-�����
	*/
	short *pureSmooth;//���������������� ������ ��� ����������
	__int32 i, j, k, z,
            jmax,//����� ������� �������
			timeToSm,//���������� ����������� (� �������������)
			sampToSm;//���������� ����������� (� ��������)
	float trgFn,//������� �������
		  bestTrgFn,//������ �������� ������� �������
		  p0, p1;//��������� ���������

	timeToSm = 10;//���� ������������� ����������� ������
	sampToSm = floor(timeToSm / cEffDT);

	pureSmooth = new short[cRecLen];
	//�������� �������� ������ �� ��������������� ������
	for (i = 0; i < cRecLen; i++)
		pureSmooth[i] = smoothSig->s[beginSampl + i];

	/*
	//========================================
	//����������� ������� ����������� ��������
	for (k = 0; k < 1; k++)//������� ����������� = 10
	{
		for (i = 0; i < cRecLen; i++)
		{
			for (j = 1; j <= sampToSm; j++)
			{
				pureSmooth[i] += smoothSig->s[beginSampl + max(i - j, 0)];
				pureSmooth[i] += smoothSig->s[beginSampl + min(i + j, recorded - 1)];
			}
			pureSmooth[i] /= ((2 * sampToSm) + 1);
		}

		for (i = 0; i < recorded; i++)
			smoothSig-s[beginSampl + i] = pureSmooth[i];//�������� �������� ������
	}
	*/

	//============================================================
	//����������� ������� ������������� ����������

	sampToSm *= 2;
	p0 = smoothSig->s[beginSampl + preSampl]; p1 = 0;
	for (k = preSampl; k < cRecLen; k += sampToSm)
	{
		//���������� ������������� (����� p1 - ���� �������)
		p0 = p0 + (p1 * sampToSm);//����� ��������� �������� p0
		bestTrgFn = 1e20; jmax = 0;
		for (j = -89; j < 89; j++)
		{
			p1 = tan(j * (3.141592 / 180)) * 5e-2;//����������� ������� ��������� �������

			trgFn = 0;
			for (i = 0; (i < sampToSm) && ((k + i) < cRecLen); i++)
				trgFn += ((smoothSig->s[beginSampl + k + i] - (p0 + (p1 * i))) *
						  (smoothSig->s[beginSampl + k + i] - (p0 + (p1 * i))));

			if (trgFn < bestTrgFn)
			{
				bestTrgFn = trgFn;
				jmax = j;//����� ������� �������
			}
		}

		p1 = tan(jmax * (3.141592 / 180)) * 5e-2;//����������� ������� ��������� �������

		for (i = 0; (i < sampToSm) && ((k + i) < cRecLen); i++)
			pureSmooth[k + i] = (short)(floor((p0 + (p1 * i)) + 0.5));
	}

	//������������ ��������� � ����� ��� ����������� �������
	for (i = 0; i < cRecLen; i++)
    	smoothSig->s[beginSampl + i] = pureSmooth[i];

	smoothSig = NULL;
	delete[] pureSmooth; pureSmooth = NULL;
}
//---------------------------------------------------------------------------

void ReCalcNulls(short *allNulls, short *signal, __int32 nSigs, __int32 postSampl)
{
	//��������������� ����-������ ��� ���� ��������
	/*
	allNulls - ��������� �� ������ � ������ ������
	signal - ��������� �� ������ � ��������
	nSigs - ���������� ��������
	postSampl - ����� ������ ������� (� ��������)
	*/

	__int32 i;
	float locMean;//������� �� �������� �������

	//====������=�������======
	/*
	locMean = 0;
	if (postSampl > 0)
	{
		for (i = 0; i < postSampl; i++)
			locMean += (float)signal[i];

		locMean /= (float)(postSampl);//���� ����� ������� �������
	}

	allNulls[0] = (short)locMean;
	*/

	//====������=�������======
	allNulls[0] = signal[postSampl];//�� ���� ����������� �������� � ����� postSampl
}
//---------------------------------------------------------------------------

