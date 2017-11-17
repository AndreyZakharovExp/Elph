//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <math.h>
#include <algorith.h>
#include <process.h>
#include <time.h>

#pragma hdrstop

#include "MainForm.h"
#include "Oscillograph.h"
#include "PredStart.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "cspin"
#pragma resource "*.dfm"
TOscil *Oscil;

IADCDevice *osc_pADC = NULL;//��������� �� ��������� ��������
IADCUtility *osc_pUtil = NULL;//��������� �� ���������
ADCParametersDMA osc_a;//��������� ��������� ����������

__int32 oBlockSize,//������ ����� ������
		grpLeft,//����� ������� ���� ��������� � ������������
		grpTop,//������� ������� ���� ��������� � ������������
		grpWid,//������ ���� ��������� � ������������
		grpHeig,//������ ���� ��������� � ������������
        drawStep,//��� �� �������� ��� ��������� (��������� ���������� �����)
		vScan,//������������ ��������
		nulLine,//������������ ���������� ����-����� � ����
		nulLineShift,//�������� ����-�����
        nporog,//������� ������ �������� ���������� ��������
		oStim_out[2],//�������� ��������� ��������������
		oToRec,//����� ������ �� ������ ������
		oChanNumb,//����� ������������ �������
		recStep,//��� ������ � ����
		oGain,//����������� ��������
		oshowSamp,// ������� � ������� ��� ������ ������� �������������
		impPM,//������� �������� ������� ����������
		visualTime;//�������� ���������� ���� ������������ (� �������������)
float oDiscrTime,//���������������� ����� ������������� (������������)
	  minDiscrTime;//����������� ����� ������������� (������������)
short oConstCompMinus,//�������� �� ���������� ������������
	  oPolarity;
HANDLE ContinRecFile;

//---------------------------------------------------------------------------
__fastcall TOscil::TOscil(TComponent* Owner)
	: TForm(Owner)
{
	//osciloscope
	chkChnBoxs[0] = checkChann0;//����� ���������� �� ��������
	chkChnBoxs[1] = checkChann1;
	chkChnBoxs[2] = checkChann2;
	chkChnBoxs[3] = checkChann3;
	chkChnBoxs[4] = checkChann4;
	chkChnBoxs[5] = checkChann5;
	chkChnBoxs[6] = checkChann6;
	chkChnBoxs[7] = checkChann7;
}
//---------------------------------------------------------------------------

void __fastcall TOscil::winoscil(IADCUtility* opUtil, IADCDevice *opADC)
{
	__int32 z;
	float freqDiscr;

	osc_pUtil = opUtil; opUtil = NULL;
	osc_pADC = opADC; opADC = NULL;
	osc_a = Experiment->a;

	oBlockSize = Experiment->dmaBlockSize;//������ ����� ������, ���������� ������� GetBuffer
	oChanNumb = osc_a.m_nChannelNumber;
	oGain = osc_a.m_nGain;//����������� ��������
	oDiscrTime = StrToFloat(Experiment->DiscreTime->Text);//���������������� ����� �������������
	minDiscrTime = 1e6 / Experiment->maxADCFreq;//����������� ����� ������������� (������������)
	osc_a.m_fFreqStart = Experiment->maxADCFreq;//������������� ������������ ������� �������������
	recStep = floor(oDiscrTime / minDiscrTime);//��� ������ � ����
	for (z = 0; z < maxChannels; z++)
		if (z < oChanNumb)
			chkChnBoxs[z]->Visible = true;//�������� ������ oChanNumb ������ ������
		else
		{
			chkChnBoxs[z]->Visible = false;//��������� ������ �� �����������
			chkChnBoxs[z]->Checked = false;
		}
	vScale->Max = Experiment->maxVoltage;//������������ �������� ������������ ��������
	vScale->Position = Experiment->maxVoltage;//��������� �������� ������������ ��������
	vScale->Frequency = vScale->Max / 20;
	timeScale->Min = 50;//����������� "����" ������������ +floor(float(minDiscrTime * oChanNumb) / 1000)
	MinTmpWinLbl->Caption = IntToStr(timeScale->Min) + " ��";//���������� ����������� ��������� ����� ����
	CrntTmpWinLbl->Caption = IntToStr(timeScale->Position) + " ��";//���������� ������������ �������� ���������� ����
	NulShift->Min = - Experiment->maxVoltage;//������ ��������� ������� �����
	NulShift->Max = Experiment->maxVoltage;//������ ��������� ������� �����
	NulShift->Position = 0;//��������� ��������� ����-�����
	Porog->MinValue = -Experiment->maxVoltage;//����������� ����� �������� ��������
	Porog->MaxValue = Experiment->maxVoltage;//������������ ����� �������� ��������
	Porog->Value = StrToInt(PStart->Porog->Text);//����� �������� �������� (��������)
	Alternating->Checked = PStart->NulMinus->Checked;//���������� ������
	ConstCompon->Checked = !PStart->NulMinus->Checked;//���������� ������������

	oConstCompMinus = (short)(Alternating->Checked);//�������� �� ���������� ������������
	oPolarity = (-1) + (2 * (__int32)(!invert->Checked));//���������� �������

	_beginthread(StartOsc, 8192, NULL);//����� ������ ��������� � ������������

	Oscil->ModalResult = mrNone;//�� ��������� ����������� ����������� ��� ��������������� ��������
	Oscil->ShowModal();//��������� ���� ������������
}
//---------------------------------------------------------------------------

void StartOsc(void *pParam)
{
	__int32 i, j, z,
			grafik[maxChannels][2],//�������� ���������� �� �������, ������������� ��� ���������
			grafikBack[maxChannels][2],//������� �������� grafik
			recorded,//������� ����������� ������� (������ ������)
			errC,//������� ������
			tctPperiod,//����������� �� ������ ����������
			oCountlm,//���������� �������� �������� ��� ���������� ���������� �������� (����������)
			blkready,//���������� ����� ������
			recSchet,//����������� ������� ������ � ����
			drawSchet,//������ ����������
			sampsMean,//���������� �������� �������� ��� ���������� ���������� ��������
			readOrder[maxChannels];//������ � ������������������� ������ �������
	unsigned int mask;//������ ���������� � ��������� ������
	unsigned long dwNumWritten;
	float gr_pnt[maxChannels],//�������� ���������� �� �������, �� ������������� ��� ���������
		  samplTomV,//����������� �������� �������� �� �������� ��� � �����������
		  waitTime;//������������ ����� (���) ����� ����������� ��������������
	short *oDrvData,
		  *extrLocMean,//������ �� ������ ������� (extract) ���������� ��������
		  convData[maxChannels],//���������� ���������� ���� ���
		  userData[maxChannels],//������ � ������ �������� ������������
		  bwSh;//�������� ���������� ������ ��� ��������� ���� ���
	TRect rectClr;
	TCanvas *drawOn;//��������� �� �����, �� ������� ������ (��������� ��������)
	TPen *canvasPen;//��������� �� �������� ����� (��������� ��������)
	clock_t curMoment,//������� ������� ������� (� �������� �� �����������)
			nextImpuls;//������ ������ �������� ���������� (��������������)
	unsigned short digitPort4,//�������� ������� ������ ����� �������� ��������� �����
				   eds;//����� ��� ��������� ���� ��������� �����
	double locMean[maxChannels];//��������� ������� (����� ��� ������������� ���������� ������������ �������� �������)
	char buf;
	__int64 curnttic,//������� ���� ����������
			tpp,//������ �� ����� ��������������
			tp1,
			ticps;//������ � ������� (������� ����������)

	oDrvData = NULL;
	Oscil->FormResize(NULL);//���������� ������� ���������
	Oscil->timeScaleChange(NULL);//
	Oscil->vScaleChange(NULL);//
	Oscil->FormResize(NULL);//���������� ������� ���������
	Oscil->PorogChange(NULL);//
	drawSchet = drawStep;//������� ����������
	recSchet = recStep;//������� ������ � ����
	recorded = 0;//������� ����������� �������� (������ ������)
	blkready = 0;// ���������� ����� ������
	nextImpuls = clock();//����� ���������� �������� (������ �� ������ ������ ���������)
	digitPort4 = 0;
	drawOn = Oscil->Canvas;//��������� �� �����, �� ������� ������ (��������� ��������)
	canvasPen = drawOn->Pen;//��������� �������� ����� (��������� ��������)
	if (PStart->preRecThread->stim_out[0] <= 0)//�������� ��� ������ RecordThread->StartRec � Oscil->StartOsc
		eds = 0;//��������� �� ����� ����������� �� ��� ���
	else
	{
		eds = Experiment->eds;//����� ��� ��������� ���� ��������� ����� (0xF = 15(dec) = 0000000000001111(bin))
		oStim_out[0] = PStart->preRecThread->stim_out[0];
		oStim_out[1] = PStart->preRecThread->stim_out[1];
	}
	bwSh = Experiment->bwSh;//�������� ���������� ������ ��� ��������� ���� ���

	z = 0;//����� ������� ������ ������� (����������� �������)
	for (i = Experiment->uiLeadChan->ItemIndex; i < oChanNumb; i++)
	{
		readOrder[z] = i;
		z++;
	}
	for (i = 0; i < Experiment->uiLeadChan->ItemIndex; i++)
	{
		readOrder[z] = i;
		z++;
	}

	Oscil->timeScaleChange(NULL);//�������������� ���������� oToRec
	oPolarity = -1 + (2 * __int32(!Oscil->invert->Checked));
	samplTomV = Experiment->maxVoltage / (float)Experiment->maxADCAmp;//����������� �������� �������� �� �������� ��� � �����������
	for (i = 0; i < maxChannels; i++)
	{
		grafik[i][1] = 0;//y-����������
		grafikBack[i][0] = grpLeft;//x-����������
		grafikBack[i][1] = 0;//y-����������
	}

	//== ��������� ��������� ������� ==
	sampsMean = (__int32)floor((float(baseLine) / (minDiscrTime * float(oChanNumb))) + 0.5);
	extrLocMean = new short[sampsMean * oChanNumb];
	Experiment->BeginMean(extrLocMean, sampsMean * oChanNumb);

	for (z = 0; z < oChanNumb; z++)
	{
		locMean[z] = 0;
		for (i = 0; i < sampsMean; i++)
			locMean[z] += (double)extrLocMean[i * oChanNumb + readOrder[z]];
		locMean[z] /= (double)sampsMean;
	}
	oCountlm = 0;

	z = osc_pUtil->Start(&osc_a, 0);//����� �����
	if (z != 1)
	{
		Experiment->DevEvents->Text = (FindErrorStrByCode(z, 0));
		Experiment->DevEvents->Text.Insert("Oscil ", 1);
		return;
	}
	z = 0;
	while (z == 0)
		osc_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0

	z = 0; errC = 0;//������������� ���� �� ������ ���
	while ((z == 0) && (errC <= 2))
	{
		z = osc_pADC->Get(ADC_WRITE_DAC, &oStim_out[1]);
		Sleep(1);
		errC++;
	}

	waitTime = min(float(500), minDiscrTime);//������������ ����� (���) ����� ����������� ��������������
	errC = 0;//���������� ���������
	while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//������� ����������
		errC++;
	tpp = (float)ticps * (waitTime / (float)1e6);//������ �� ����� ��������������

	Oscil->Tag = 1;//��������� �������� � ������������
	while (Oscil->Tag > 0)//���� ��������� (����������� ������ ��� �������� ������������)
	{
		//����� �������������� �������
		if ((Oscil->OneImpulse->Tag == 1) || (impPM > 0))//���� ������� ���������� (���� ��� ����� ���������)
		{
			curMoment = clock();//������� ������ �������
			if ((curMoment >= nextImpuls) || (Oscil->OneImpulse->Tag == 1))//���� �������� �������������
			{
				z = 0; errC = 0;//������������� ������ �� ������ ���
				while ((z == 0) && (errC <= 2))
				{
					z = osc_pADC->Get(ADC_WRITE_DAC, &oStim_out[0]);
					errC++;
				}

				if (impPM > 0)
				{
					tctPperiod = 60000 / impPM;//����������� � �������
					nextImpuls = curMoment + (clock_t)tctPperiod;//����� ���������� ��������
				}
				Oscil->OneImpulse->Tag = 0;//���� ������� ����� (������� ����)

				QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);
				//������ ����� ����� ����������� ��������������
				tp1 = curnttic;
				while ((curnttic - tp1) < tpp)
					QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);

				z = 0; errC = 0;//������������� ���� �� ������ ���
				while ((z == 0) && (errC <= 2))
				{
					z = osc_pADC->Get(ADC_WRITE_DAC, &oStim_out[1]);
					errC++;
				}
			}
		}

		blkready = osc_pUtil->GetBuffer((void*)oDrvData, mask);//����������� ���� ������ �� ������ DMA
		if (blkready != 0)//���� ������ �����
		{
			for (i = 0; i < oBlockSize; i += oChanNumb)
			{
				if (digitPort4 == 0)//��� ����� �������������
					digitPort4 = oDrvData[i] & eds;//eds - ����� (unsigned short)

				for (z = 0; z < oChanNumb; z++)
				{
					convData[z] = oDrvData[i + readOrder[z]] >> bwSh;//�������� ��� ��� (������� 12 (14) ���)
					userData[z] = oPolarity * (convData[z] - short(locMean[z] * oConstCompMinus));

					//��������� ������������ ��������� �������
					locMean[z] += (double(convData[z] - extrLocMean[oCountlm * oChanNumb + readOrder[z]]) / (double)sampsMean);
					extrLocMean[oCountlm * oChanNumb + readOrder[z]] = convData[z];//������ ��������� ��������
				}
				oCountlm++;
				if (oCountlm >= sampsMean)
					oCountlm = 0;

				if ((Oscil->ContRec->Tag == 1) && (recSchet >= recStep))//���� �������� ������, ���������� �� ������
				{
					WriteFile(ContinRecFile, (void*)&userData[0], sizeof(short) * oChanNumb, &dwNumWritten, NULL);//���������� �������� ������ ���������� ������
					recSchet = 0;
				}

				if (drawSchet >= drawStep)//���������
				{
					for (z = 0; z < oChanNumb; z++)
					{
						gr_pnt[z] = oPolarity * ((float)convData[z] - (locMean[z] * (float)oConstCompMinus)) * samplTomV;//�������� ���������� �� �������, �� ������������� ��� ���������
						grafik[z][0] = grpLeft + __int32(float(recorded * grpWid) / oToRec);//x-����������
						grafik[z][1] = floor(nulLine + (((-1.0) * (gr_pnt[z] * grpHeig)) / vScan) + 0.5);//�������� ���������� �� �������, ������������� ��� ���������
					}

					drawOn->Lock();//����������� �����

					canvasPen->Color = clBtnFace;//���� ���� ��� ��������� �������
					if (digitPort4 != 0)//��� ��������� ��������������
					{
						recorded = oToRec;//��������� � ��������� � ������ ����
						rectClr = Rect(grafik[0][0], grpTop, grpLeft + grpWid, grpTop + grpHeig);
						drawOn->FillRect(rectClr);//�������� ������
						digitPort4 = 0;
					}
					z = min(grafik[0][0] + 10, grpLeft + grpWid);//x - ����������
					drawOn->MoveTo(z, grpTop);//�������� ������
					drawOn->LineTo(z, grpTop + grpHeig);//�������� ������

					//������ ������� �� �������
					if (recorded >= oToRec)//����� �� ����� ���� ������������
					{
						rectClr = Rect(grpLeft, grpTop, min(grpLeft + 10, grpLeft + grpWid), grpTop + grpHeig);
						drawOn->FillRect(rectClr);//�������� ������

						for (z = 0; z < oChanNumb; z++)
						{
							if (Oscil->chkChnBoxs[z]->Checked)
							{
								//������� ������ ��� ��������� ������� (grafik)
								canvasPen->Color = channColor[z];
								drawOn->MoveTo(grpLeft, grafikBack[z][1]);
								drawOn->LineTo(grpLeft, grafik[z][1]);
							}
							grafikBack[z][0] = grpLeft;
							grafikBack[z][1] = grafik[z][1];
						}
						recorded = 0;
					}
					else
						for (z = 0; z < oChanNumb; z++)
						{
							if (Oscil->chkChnBoxs[z]->Checked)
							{
								//������� ������ ��� ��������� ������ � ������ z
								canvasPen->Color = channColor[z];
								drawOn->MoveTo(grafikBack[z][0], grafikBack[z][1]);
								drawOn->LineTo(grafik[z][0], grafik[z][1]);
								if (gr_pnt[z] > nporog)//��� ������� �������� �� ���������
									drawOn->LineTo(grafik[z][0], nulLine);//����������� ������� ��� ���������
							}
							grafikBack[z][0] = grafik[z][0];
							grafikBack[z][1] = grafik[z][1];
						}

					//������ �����-�����
					canvasPen->Color = clYellow;//����� ��������� �����
					drawOn->MoveTo(grpLeft, nulLine + ((-1.0) * nporog * grpHeig) / vScan);
					drawOn->LineTo(grpLeft + grpWid, nulLine + ((-1.0) * nporog * grpHeig) / vScan);

					//������ ����-�����
					canvasPen->Color = clSkyBlue;//�������������� ����-�����
					drawOn->MoveTo(grpLeft, nulLine);
					drawOn->LineTo(grpLeft + grpWid, nulLine);

					drawOn->Unlock();//������������ �����

					drawSchet = 0;
				}
				recorded++;//���������� ������� ����������� �������� (������ ������)
				drawSchet++;//����������� ������� ����������
				recSchet++;//����������� ������� ������ � ����
			}
			z = 0;//����������� ����������� ���� ������ (�����)
			while (z == 0)
				z = osc_pUtil->FreeBuffer();
		}
	}

	z = 1;//������������� ���� ������
	while (z == 1)
	{
		osc_pUtil->Stop();
		osc_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0
	}

	oDrvData = NULL;
	delete[] extrLocMean; extrLocMean = NULL;

	Oscil->Tag = -2;//����� �� ������ ��������� � ������������
}
//---------------------------------------------------------------------------

void __fastcall TOscil::timeScaleChange(TObject *Sender)
{
	//������ ��������� ��������
	visualTime = Oscil->timeScale->Position;//�������� ���������� ���� ������������ (� �������������)
	CrntTmpWinLbl->Caption = IntToStr(visualTime) + " ��";//���������� ������������ �������� ���������� ����
	oToRec = floor(float(visualTime * 1e3) / (minDiscrTime * oChanNumb)) + 1;//����� ������ �� ������ ������ (�������)
	drawStep = floor(float(oToRec) / grpWid);//��� �� �������� ��� ��������� (��������� ���������� �����)
}
//---------------------------------------------------------------------------

void __fastcall TOscil::onOscClose(TObject *Sender)
{
	//��������� ������
	if (Oscil->Tag > 0)
		Oscil->Tag = -1;//��������� ����� ��������� � ������������

	Experiment->DevEvents->Text = "wait for end...";
	while (Oscil->Tag >= -1){};//��� ���������� ����� ��������� � ������������
	Experiment->DevEvents->Text = "OK";

	if (ContRec->Tag == 1)//���� ������ �� ����������� �������������
		ContRecClick(this);//��������� ������ ����� �������������
}
//---------------------------------------------------------------------------

void __fastcall TOscil::acceptParamtrClick(TObject *Sender)
{
	PStart->Porog->Text = Porog->Text;
	PStart->NulMinus->Checked = Alternating->Checked;
	PStart->Invert->Checked = invert->Checked;
	Oscil->ModalResult = mrYes;//��������� ����
}
//---------------------------------------------------------------------------

void __fastcall TOscil::PorogChange(TObject *Sender)
{
	nporog = Porog->Value * oGain;//������� ������ �������� ���������� ��������
}
//---------------------------------------------------------------------------

void __fastcall TOscil::invertClick(TObject *Sender)
{
	oPolarity = (-1) + (2 * (__int32)!invert->Checked);//������ ������� ���������� �������
}
//---------------------------------------------------------------------------

void __fastcall TOscil::AlternatingClick(TObject *Sender)
{
	oConstCompMinus = (short)Alternating->Checked;//�������� ���������� ������������
}
//---------------------------------------------------------------------------

void __fastcall TOscil::ConstComponClick(TObject *Sender)
{
	oConstCompMinus = (short)!ConstCompon->Checked;//�� �������� ���������� ������������
}
//---------------------------------------------------------------------------

void __fastcall TOscil::FormResize(TObject *Sender)
{
	//��� ��������� ������� ���� ������������ ��������� ���������

	grpLeft = Oscil->vScale->Left + Oscil->vScale->Width;//����� ������� ���� ��������� � ������������
	grpTop = 0;//������� ������� ���� ��������� � ������������
	grpWid = Oscil->NulShift->Left - grpLeft;//������ ���� ��������� � ������������
	grpHeig = Oscil->timeScale->Top - grpTop;//������ ���� ��������� � ������������
	nulLine = floor(grpTop + (grpHeig / 2) + ((nulLineShift * grpHeig) / vScan) + 0.5);//������������ ���������� ����-����� � ����
	drawStep = floor((float)oToRec / grpWid);//��� �� �������� ��� ��������� (��������� ���������� �����)
}
//---------------------------------------------------------------------------

void __fastcall TOscil::RightKeyPress1(TObject *Sender, char &Key)
{
	//�������� ������������ �������� �������, ���������������� ������� �������

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//�������� ������������ �������� ��������
		Key = '\0';//�������� ������������ ������
}
//---------------------------------------------------------------------------

void __fastcall TOscil::RightKeyPress2(TObject *Sender, char &Key)
{
	if (((Key < '0') || (Key > '9')) && (Key != '\b') && (Key != '-'))//�������� ������������ �������� ��������
		Key = '\0';//�������� ������������ ������
}
//---------------------------------------------------------------------------

void __fastcall TOscil::OneImpulseClick(TObject *Sender)
{
	//��� ������� ��� ���� ������� ����������
	Oscil->OneImpulse->Tag = 1;
}
//---------------------------------------------------------------------------

void __fastcall TOscil::ContRecClick(TObject *Sender)
{
	//�����-���� ����������� ������ ������ ������ �� ����
	__int32 bufInt;//�����
	unsigned long amountOfBytes,//���������� ����, ������������ ������ �����
				  bytesWritten;//����� ���������� ����
	AnsiString recordPrm;//���������� � ��������� � ���� �������� �����
	TDateTime CurrentDateTime;//������� ���� � �����
	float sampls2mV;//����������� �������� �������� �� �������� ��� � (�����)������

	if (ContRec->Tag == 0)//���� ������ �� ��������
	{
		if (oDiscrTime >= (20 * minDiscrTime))//���������������� ����� ������������� ������ ������� �������
		{
			//����� ����� �����
			Graphs->SaveDlg->Title = "������� ��� �����";//���������� ���������
			Graphs->SaveDlg->Filter = "*.bin|*.bin";//������ ����������� ��������
			Graphs->SaveDlg->DefaultExt = "bin";//���������� �� ���������

			if (Graphs->SaveDlg->Execute())
			{
				CurrentDateTime = Now();//������� ���� � �����
				//������ ���� ��� ����������� ������
				ContinRecFile = CreateFile(Graphs->SaveDlg->FileName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE,
										   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				//��������� ��������� �����
				recordPrm = "ElphAcqu v" + progVer + "\nmade";//~18 ��������
				recordPrm += CurrentDateTime.DateTimeString().c_str();//19 ��������

				amountOfBytes = sizeof(char) * recordPrm.Length();//���������� ����, ������������ � ������ �����
				amountOfBytes += (3 * sizeof(float)) + (4 * sizeof(__int32));//���������� ����, ������������ � ������ �����

				//��������� ������������ ������
				WriteFile(ContinRecFile, (void*)recordPrm.c_str(), sizeof(char) * recordPrm.Length(), &bytesWritten, NULL);
				amountOfBytes -= bytesWritten;

				//����������, ���������� ��� ������ ������ (�.�. ��� ����������� ���������)
				/* ������������������ ������ ������������ ����������
					sampls2mV ----(1) - ����������� �������� �������� �� �������� ��� � (�����)������
					oChanNumb ----(2) - ���������� ������������ �������
					minDiscrTime -(3) - ����������� (��������) ����� �������������
					oDiscrTime ---(4) - ���������������� ����� �������������
					maxADCAmp ----(5) - ������������ ��������� ��� (� ��������)
				*/

				WriteFile(ContinRecFile, (void*)&oChanNumb, sizeof(__int32), &bytesWritten, NULL);//2//���������� ������������ �������
				amountOfBytes -= bytesWritten;
                WriteFile(ContinRecFile, (void*)&oGain, sizeof(__int32), &bytesWritten, NULL);//8//����������� ��������
				amountOfBytes -= bytesWritten;
				WriteFile(ContinRecFile, (void*)&minDiscrTime, sizeof(float), &bytesWritten, NULL);//3//����������� (��������) ����� �������������
				amountOfBytes -= bytesWritten;
				WriteFile(ContinRecFile, (void*)&oDiscrTime, sizeof(float), &bytesWritten, NULL);//4//���������������� ����� ������������� (�����������, ��� ������)
				amountOfBytes -= bytesWritten;
				WriteFile(ContinRecFile, (void*)&Experiment->maxADCAmp, sizeof(__int32), &bytesWritten, NULL);//5//������������ ��������� ��� (� ��������)
				amountOfBytes -= bytesWritten;
				WriteFile(ContinRecFile, (void*)&Experiment->maxVoltage, sizeof(float), &bytesWritten, NULL);//6//�������� ���������� ������� ���������� (�����������)
				amountOfBytes -= bytesWritten;
				bufInt = Experiment->uiLeadChan->ItemIndex;
				WriteFile(ContinRecFile, (void*)&bufInt, sizeof(__int32), &bytesWritten, NULL);//7//������� ����� (�������� ������)
				amountOfBytes -= bytesWritten;

				if (amountOfBytes != 0)
				{
					Experiment->DevEvents->Text = "������ ��������� ������";//��������� �����, ������, ����� �� ������
					CloseHandle(ContinRecFile);//��������� ������ �����
					return;
				}

				ContRec->Caption = "����";
				ContRec->Tag = 1;//������ ������ � ���� ��������
			}
		}
		else
			Experiment->DevEvents->Text = "���. ��. �����. 40���";
	}
	else
	{
		ContRec->Tag = 0;//���������� ������, ����� � ����� ������
		CloseHandle(ContinRecFile);//��������� ������ �����
		ContRec->Caption = "������";
	}
}
//---------------------------------------------------------------------------

void __fastcall TOscil::NulShiftChange(TObject *Sender)
{
	nulLineShift = Oscil->NulShift->Position;//�������� ����-�����
	nulLine = floor(grpTop + (grpHeig / 2) + ((nulLineShift * grpHeig) / vScan) + 0.5);//������������ ���������� ����-����� � ����
}
//---------------------------------------------------------------------------

void __fastcall TOscil::vScaleChange(TObject *Sender)
{
	vScan = Oscil->vScale->Position;//��������� ������������ ��������
	NulShiftChange(this);//������������� ��������� ����-�����
}
//---------------------------------------------------------------------------

void __fastcall TOscil::ImpPerMinChange(TObject *Sender)
{
	impPM = ImpPerMin->Value;//������� �������� ������� ����������
}
//---------------------------------------------------------------------------
