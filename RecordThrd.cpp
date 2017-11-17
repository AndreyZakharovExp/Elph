//$$---- Thread CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <math.h>
#include <algorith.h>
#include <stdio.h>

#pragma hdrstop

#include "RecordThrd.h"
#include "PredStart.h"
#include "BuildProtocol.h"
#include "CalcParams.h"
#include "Gistograms.h"
#include "MinorChannel.h"
#include "ExperimentNotes.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

IADCDevice *r_pADC;//��������� �� ��������� ��������
IADCUtility *r_pUtil;//��������� ��������
ADCParametersDMA  r_a;//��������� ���������� ����������
trac *rSignals;//��������� � ��������� � �����������
trac *rSpntSignals;//��������� � ��������� � �����������

bool rAppEndData;//������� �� �������� ����� � ������������ ��������
__int32 rBlockSize,//������ ����� ������
		rStimPrd,//������ ���������� (������������)
		rSpntRecLen,//����� ����������� ������� � �������� (��� ������ �4)
		rTimeBgn,//������ ������ ������� (������������)
		specK1,//����������� ���������� ��� �������� ������1 //����� �������, � �������� ���������� �������������� �������
		specK2,//����������� ���������� ��� �������� ������2 //���������� �������������� ��������
		rPreTime,//������ ������. ������ (������������)
		rPostTime,//������ �������. ������ (������������)
		rTotNumOfS,//�������� � ����� ����� �������� � ������������
		rRealNumOfS,//�������������� ����� ���������� ��������
		rCountlm,//������� ��� �������� ��������� ������� ��� ������� ���������� ��������
		rSamps,//���������� �������� �������� ��� ���������� ���������� ��������
		*rInstr,//������ � ����������� (�������� ����������)
		rReadOrder[maxChannels];//������ � ������������������� ������ �������
__int64 rNextTic,//������ (� ������ ����������) ������ ���������� ��������������
		rBeginTakt;//���� ����������, ��� ������� ���������� ���������� ��������
short rTypeOfExp,//��� ������������
	  rConstMinus,//��������� ���������� ������������
      rPorog,//��������� ���������� (�������)
	  rPolarity,//���������� �������
	  *rExtractLocMean;//������ ������� (extract) ���������� ��������
double rLocMean[maxChannels],//��������� ������� �� ���� �������; 5 - � ������� (����� ��� ������������� ���������� ������������ �������� �������)
	   *rmGrafik,//������ ��� ���������
	   *rmGrafikM;//������ ��� ��������� ���������� �������� (� ������ �4)
FILE *rDiscontinFile;//����� �����, � ������� ����������� ������ (��� ����������� �����)

//---------------------------------------------------------------------------
__fastcall TRecordThread::TRecordThread(bool CreateSuspended) : TThread(CreateSuspended)
{
	//Graphs->CrntSig->Tag - ���������� ������� ��������
	//PStart->closeWin->Tag - ��������� ����������� ����� � �������� ������ ��������-��������
	rmGrafik = NULL;//������ ��� ���������
	rmGrafikM = NULL;//������ ��� ��������� ���������� �������� (� ������ �4)
}
//---------------------------------------------------------------------------

void TRecordThread::AssignVar(short eTyp, IADCUtility *preUtil, IADCDevice  *preADC)
{
	__int32 i, k;

	rTypeOfExp = eTyp;//��� ������������
	r_pUtil = preUtil;//������ �� ���������
	r_pADC = preADC;//������ �� ���������
	r_a = Experiment->a;//��������� ���������� ����������

	rStimPrd = StrToInt(Experiment->StimPeriod->Text);//������ ���������� (������������)
	rBlockSize = Experiment->dmaBlockSize;//������ ����� ������
	chanls = Experiment->a.m_nChannelNumber;//���������� ����������� �������
	discrT = StrToFloat(Experiment->DiscreTime->Text);//����� ������������� (������������)
	effDT = discrT * (float)chanls;//����������� ����� �������������
	
	if (eTyp == 1)//���������� �������
		recLen = (__int32)floor((float(StrToInt(PStart->LenSpont->Text) * 1000) / effDT) + 0.5);//����� �������� ������� (�������)
	else//���������
		recLen = (__int32)floor((float(StrToInt(PStart->SignalLen->Text) * 1000) / effDT) + 0.5);//����� �������� ������� (�������)

	multiCh = Experiment->RecMode->Checked;//����� ����� ������ (true = ��������������)
	ftChan = 1 + (chanls - 1) * (__int32)multiCh;//���������� ������� � ������� ���������� (full trace channels)
	sampl2mV = Experiment->maxVoltage / float(Experiment->maxADCAmp * r_a.m_nGain);//����������� �������� �������� �� �������� ��� � (�����)������
	eds = Experiment->eds;//����� ��� ��������� ���� ��������� ����� (0xF = 15(dec) = 0000000000001111(bin))
	bwSh = Experiment->bwSh;//�������� ���������� ������ ��� ��������� ���� ���

	//����������� ��������� �������� (��� ������������� � ������� Graphs)
	Graphs->multiCh = multiCh;//����� ����� ������ (true = ��������������)
	Graphs->recLen = recLen;//����� �������� ������� (�������)
	Graphs->discrT = discrT;//����� ������������� (������������)
	Graphs->chanls = chanls;//���������� ����������� �������
	Graphs->ftChan = ftChan;//���������� ������� � ������� ���������� (full trace channels)
	Graphs->effDT = effDT;//����������� ����� ������������� = discrT * chanls
	Graphs->sampl2mV = sampl2mV;//����������� �������� �������� �� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
	Graphs->gRF[0] = 0;//����� ������� ������������� ������
	Graphs->gRF[1] = ftChan;//����� ���������� ������������� ������ + 1

	rConstMinus = (short)PStart->NulMinus->Checked;//��������� ���������� ������������
	rPolarity = -1 + (2 * __int32(!PStart->Invert->Checked));//���������� �������

	k = 0;//����� ������� ������ ������� (����������� �������)
	for (i = Experiment->uiLeadChan->ItemIndex; i < chanls; i++)
	{
		rReadOrder[k] = i;
		k++;
	}
	for (i = 0; i < Experiment->uiLeadChan->ItemIndex; i++)
	{
		rReadOrder[k] = i;
		k++;
	}
	//�������� ��������� ��� "�������" ��������
	for (i = 0; i < maxChannels; i++)
	{
		Graphs->curntSigChannls[i]->XValues->Multiplier = double(effDT) / 1000;
		PStart->exmplChannls[i]->XValues->Multiplier = double(effDT) / 1000;
	}
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::RecSynchroPlotExampl(void)
{
	//����� ���������� ���������� �������

	//PlotStandardSignal(double *sData, __int32 sRecLen, __int32 sEffDT, __int32 sIndBgn)
	PStart->PlotStandardSignal(rmGrafik, recLen, effDT, (rTimeBgn / effDT));//��������� ���������� �������
	PStart->RefreshLines(effDT);//���������� ���- ����-�����
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::RecSynchroCounter(void)
{
	//������� ����� �������� �������
	Graphs->CrntSig->Text = IntToStr(Graphs->CrntSig->Tag);//������� ��������
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::MiniSynchroPlotGraphs(void)
{
	//��������� ���������� ��������
	__int32 j,
			*mspIndex = NULL;//������ ��������

	//������� ������
	Graphs->PlotCurrentSignal(rmGrafik, 0);

	//�������� ������ ��������
	//specK1 = k - kRefresh;//����� �������, � �������� ���������� �������������� �������
	//specK2 = kRefresh;//���������� �������������� ��������
	mspIndex = new __int32[specK2];
	for (j = 0; j < specK2; j++)
		mspIndex[j] = j + specK1;

	//���������� ��������� ��������
	Calculate(rSignals, specK2, rPorog, rPreTime, rPostTime, mspIndex, rTypeOfExp, false, NULL);
	Graphs->AddParamsPoints(rSignals, specK2, mspIndex, rTypeOfExp);//����� ����������, ��������� ����� �� ��������

	delete[] mspIndex; mspIndex = NULL;
	Gists->GistsRefresh(-1);//�������������� �����������
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::InducSynchroPlotter(void)
{
	//��������� ��������� ��������
	__int32 j,
			*ispIndex,//������ ��������
			shft1,//����� ������� �������� ��� ampls � peakInds
			shft2;//����� ������� �������� ��� spans

	ispIndex = NULL;
	//��������� �������� �������
	if (rmGrafik && (specK2 == 1))
		Graphs->PlotCurrentSignal(rmGrafik, (rTimeBgn / effDT));

	//�������� ������ ��������
	//specK1 - ����� �������, � �������� ���������� �������������� �������
	//specK2 - ���������� �������������� ��������
	ispIndex = new __int32[specK2];

	for (j = 0; j < specK2; j++)//���������� ���������� ����� �����
		ispIndex[j] = j + specK1;// * (__int32)(!rAppEndData);

	//���������� ���������
	if (PStart->GetASignal->Tag != 1)//���� ��� ������, �� ������ �������
	{
		Calculate(rSignals, specK2, rPorog, rPreTime, rPostTime, ispIndex, rTypeOfExp, false, NULL);

		shft1 = (3 * ftChan * ispIndex[specK2 - 1]) + (3 * 0);//����� ������� �������� ��� ampls � peakInds
		shft2 = (5 * ftChan * ispIndex[specK2 - 1]) + (5 * 0);//����� ������� �������� ��� spans
		Graphs->AddParamsPoints(rSignals, specK2, ispIndex, rTypeOfExp);//������� ����� �� �������

		j = Graphs->sigAmpls[Graphs->gRF[0]]->YValues->Count() - 1;//����� ��������� ����������� �� ������ �����
		Graphs->HighLightCrnt(&rSignals[ispIndex[specK2 - 1]], shft1, shft2, rTypeOfExp, (rTimeBgn / effDT), j);//�������������
	}

	delete[] ispIndex; ispIndex = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::MiniMd4SynchroPlot(void)
{
	//��������� ���������� �������� � ������ �4
	Graphs->PlotCurrentSignal(rmGrafikM, 0);//��������� �������� �������
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::FromMinorChan(void)
{
	//����� ������ �� �������������� �������
	__int32 i, j;

	if ((chanls > 1) && !multiCh)//���������� ������� ������ ������ � �� �������������� �����
	{
		j = (Graphs->CrntSig->Tag - 1);//����� �������
		for (i = 0; i < (chanls - 1); i++)
			SecondChan->MChann[i]->Text = IntToStr((__int32)floor(rSignals[j].s[recLen + i] * sampl2mV));
	}

}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::CallForClearMemor(void)
{
	//���������� �������� ������ �� ������ (������������ ��� ������ �4)
	Graphs->ClearMemor(this);//�������� ���� ������ ������ �� ������
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::CallForReplotGraphs(void)
{
	//���������� ����� ������� ��������� ����� ����������
	Graphs->ResetVisibility();//��������� ����������
	Graphs->FillParamTable();//��������� ������� ���������� ����������
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::Execute()
{
	//���� ������ (������ �������� ������� ��� ������ ��������)
	__int32 i,
			*index1,//��������� �� ������� ��� Calculate
			pp,//���������� ������� ��������� ���������� (���������� ����������)
			totNumOfSpontS,//������������ ����� ���������� ��������
			realNumOfSpontS,//���������� ��������� ���������� ��������
			maxStimPrd,//������������ ������ ���������� � ��������� (��� ������ �4)
			inducRecLen;//����� ����� ���������� �������
	TDateTime CurrentDateTime;//������� ���� � �����
	AnsiString memoFlNm,//����� ����� �����
			   extendFlNm;//����������� ��� ���� (��� ������ �4)
	SPPanel *panel;
	sPr *evkPrm;//����� ��������� �� ��������� � ����������� ��������� ��������

	while (!Terminated)
	{
		//������� ������ �� ����� ������������
		PStart->GetASignal->Enabled = false;//������ �������
		PStart->StartRec->Enabled = false;//�����
		PStart->Invert->Enabled = false;//�������� �������
		PStart->NulMinus->Enabled = false;//����� ����-�����
		PStart->SignalLen->Enabled = false;//����� �������
		PStart->LenSpont->Enabled = false;//����� ����������� �������
		PStart->BackTime->Enabled = false;//����� ����� ��� ����������� �������

		PStart->CloseWin->Caption = "����";//������ "�������" ������
		PStart->CloseWin->Tag = 0;//������ ������� ������
		Graphs->SigNumUpDwn->Enabled = false;//����� ������ ����������� �������
		Graphs->gphOpen->Enabled = false;//������ �� �������� ������ �� ����� ������
		Experiment->beginFOpen->Enabled = false;//������ �� �������� ������ �� ����� ������
		Experiment->verticalScal->Enabled = false;//������ �� ��������� �������� ������������ �����
		Experiment->timScal->Enabled = false;//������ �� ��������� �������� �������������� �����
		Experiment->DiscreTime->Enabled = false;//����� ������������� ������ ������ �� ����� ������
		Graphs->SignalTrack->OnMouseDown = NULL;//��������� ����������� �� ����� (meanSignalMouseDown)

		//��������� ������� �������� ����� �������
		if (PStart->GetASignal->Tag == 1)//������ ������� (������ �������� �� ���������)
		{
			pp = 1;//���������� ������� ��������� ���������� (���������� ����������)
			rInstr = new __int32[3 * pp];//������ ��� ���������� (�������� ����������)
			rTotNumOfS = 1;//���� ������� �������
			rInstr[0] = 1;//����������
			rInstr[1] = rTotNumOfS;//���� ������� �������
			rInstr[2] = rStimPrd;
		}
		else
		{
			if (ProtoBuild->InstructBox->ComponentCount <= 0)//�������� �� �����
			{
				pp = 1;//���������� ������� ��������� ���������� (���������� ����������)
				rInstr = new __int32[3 * pp];//������ ��� ���������� (�������� ����������)
				rTotNumOfS = StrToInt(Experiment->NumOfsignals->Text);
				rInstr[0] = 1;
				rInstr[1] = rTotNumOfS;
				rInstr[2] = rStimPrd;
			}
			else//���� ����� ��������
			{
				pp = ProtoBuild->InstructBox->ComponentCount;//���������� ������� ��������� ���������� (���������� ����������)
				rInstr = new __int32[3 * pp];//������ ��� ���������� (�������� ����������)
				rTotNumOfS = 0;//���������� �������� ���������� � ���������� ����� ���������� ��������
				for (i = 0; i < pp; i++)
				{
					panel = (SPPanel*)ProtoBuild->InstructBox->Components[i];
					rInstr[(i * 3) + 0] = panel->iType;//��� ����������
					rInstr[(i * 3) + 1] = panel->scCount;//���������� ��������� ��� ���������� � �����
					rInstr[(i * 3) + 2] = panel->period;//������ ���������� ��������� ��� ������������ �����

					if (rInstr[(i * 3) + 0] == 1)//���� ������, ������� ������� ���������
						rTotNumOfS += rInstr[(i * 3) + 1];
					if (rInstr[(i * 3) + 0] == 3)//���� ����, �������� �� ���-�� ��������
						rTotNumOfS = rTotNumOfS * rInstr[(i * 3) + 1];
				}
			}
		}

		//��������� ������� �������� ��������� rTimeBgn
		rPorog = short((float)StrToInt(PStart->Porog->Text) / sampl2mV);//��������� ���������� (�������)
		rPreTime = StrToInt(PStart->PreTime->Text);
		rPostTime = StrToInt(PStart->PostTime->Text);

		//�������� ������, � �������� ������� (������) ������ (����� �������������)
		rTimeBgn = 0;
		if (rTypeOfExp == 3)//��������� - ������������
			rTimeBgn = rPreTime;
		else if (rTypeOfExp == 2)// (���������������) ��� rTypeOfExp = 4 (���������)
			rTimeBgn = rPostTime;

		PStart->timeOfDrawBgn = (float)rTimeBgn;//������� �������� �������-������ � ������ PStart

		//������ �� ����� ������������ ����� (� ������������ ��������)
		//���� �������� ������ �������, �� ��������� ������������ �������
		rAppEndData = ((Experiment->DiscontinWrt->Checked) && (PStart->GetASignal->Tag != 1));//����������� ������ (�� ������ �������)

		if (rAppEndData)//������ ������ ������
			CreatExtendFile();//������� ��� ���������� ���� � ����������� ������

		if ((rTypeOfExp != 4) || (PStart->GetASignal->Tag == 1))//�����, ����� ���������+����������
		{
			rSignals = Graphs->CreatStructSignal(rTotNumOfS, recLen);//��������� � ��������� � �� ����������
			totNumOfSpontS = 0;//���������� ���������� �������� � ������ �4
		}
		else// if (rTypeOfExp == 4)//������������� ���� ��������� � ����������; � rSignals ���������� � ��������� � ����������
		{
			//�������� �������������� ��������� ��� �������� ���������� ��������
			if (ProtoBuild->InstructBox->ComponentCount > 0)
			{
				maxStimPrd = 0;//���������� �������� ����� ���������� ���������� ��� ������� totNumOfSpontS
				for (i = 0; i < pp; i++)
					if ((rInstr[(i * 3) + 0] == 1) && (rInstr[(i * 3) + 2] > maxStimPrd))
						maxStimPrd = rInstr[(i * 3) + 2];//���������� ���������� ��������
			}
			else
				maxStimPrd = StrToInt(Experiment->StimPeriod->Text);//���������� �������� ����� ���������� ����������

			inducRecLen = Graphs->recLen;//����� ����� ���������� �������
			rSpntRecLen = floor((float(StrToFloat(PStart->LenSpont->Text) * 1000) / effDT) + 0.5);//����� ����������� ������� � ������ �4 (�������)
			rmGrafikM = new double[rSpntRecLen * ftChan];//������ ��� ��������� ���������� �������� (� ������ �4)

			//totNumOfSpontS - ����������� ��������� ���������� ���������� �������� � ������ �4
			totNumOfSpontS = rTotNumOfS * (floor(0.5 * maxStimPrd / StrToInt(PStart->LenSpont->Text)) + 1);//(����������) ���������� ���������� ��������
			if (totNumOfSpontS > limitSigNum)//����������� �� ���������� ���������� ��������, ���������� � ������ �4
				totNumOfSpontS = limitSigNum;//��������� ���������� ���������� ���������� ��������
			rSignals = Graphs->CreatStructSignal(rTotNumOfS, recLen);//��������� � ���������� ���������
			evkPrm = Graphs->cPrm;//����� ��������� �� ��������� � ����������� ��������� ��������
			rSpntSignals = Graphs->CreatStructSignal(totNumOfSpontS, rSpntRecLen);//��������� �� ����������� ���������
		}

		rmGrafik = new double[recLen * ftChan];//������ ��� ���������
		GetLocalMean();//��������� ��������� �������

		//--------------------------------------------------------------------
		//--- ������ ������ �� ��������� ����� ������ ------------------------
		StartRec(rTotNumOfS, totNumOfSpontS, pp);//��������� ��������
		//--------------------------------------------------------------------

		//����� ����� ������: ����������, ����� �� ����� � �.�.
		rRealNumOfS = Graphs->CrntSig->Tag;//���������� ������������� ���������� ��������� ��������
		realNumOfSpontS = PStart->BackTime->Tag;//���������� ������������� ���������� ���������� ��������
		Graphs->SigNumUpDwn->Tag = rTotNumOfS;//�������� �������� ����� �������� (����������� ��� ��������)

		if (rRealNumOfS >= 1)//���� ������� �������
		{
			if (PStart->GetASignal->Tag == 1)//���� ������� ������ �������� �������
			{
				index1 = new __int32[1];
				index1[0] = 0;
				//���������� ��������� � ������� ��
				//Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
				//			__int32 *inds, short expT, bool newNull, sPr *avrP)
				Calculate(rSignals, rRealNumOfS, rPorog, rPreTime, rPostTime, index1, rTypeOfExp, false, NULL);
				delete[] index1; index1 = NULL;
				Synchronize(&RecSynchroPlotExampl);//����� ������� � ���� ����������//���������� �������
			}
			else if (!rAppEndData)//���������� ������������ PStart->getASignal->Tag = 2
			{
				if (rTypeOfExp == 4)//��������� + ����������
				{
					memoFlNm = Graphs->SaveDlg->FileName;//��������� ����� �������� �����
					//������� ��������� ���������� �������
					if (realNumOfSpontS >= 1)//���� ���������� �������
					{
						extendFlNm = memoFlNm;//����� ���������� �������� ��� �����
						extendFlNm.Insert("_Spont", extendFlNm.Length() - 3);//����������� ��� �����, ����� ��������� �����������������
						Graphs->SaveDlg->FileName = extendFlNm;//������������� ��� ����� �� ����������� ���������

						Graphs->SigNumUpDwn->Tag = totNumOfSpontS;//�������� �������� ����� �������� (����������� ��� ��������)
						Graphs->recLen = rSpntRecLen;//�������� ��������� ����� ����������� �������
						i = (__int32)Graphs->SaveExpDataToFile(5, rSpntSignals, realNumOfSpontS, true);//��� ������������
						Graphs->recLen = inducRecLen;//����� ��������� ����� ���������� �������
					}
					else//��� ���������� ��������
						Experiment->DevEvents->Text = "��� ���������� ��������";//�������� �� ����������
					Graphs->DeleteStructSignal(rSpntSignals, totNumOfSpontS);//����������� ������, ���� ��� ����������
					rSpntSignals = NULL;//��������� ������� � ������ Graphs->ClearMemor

					//��������� �������� ��������� ������� (������� �� ����������������, �2)
					extendFlNm = memoFlNm;//��������� ����� �������� �����
					extendFlNm.Insert("_Induced", extendFlNm.Length() - 3);//����������� ��� �����, ����� ��������� ��������� �������
					Graphs->SaveDlg->FileName = extendFlNm;//������������� ��� ����� � ���������� ���������
					Graphs->cPrm = evkPrm;//����� ��������� �� ��������� � ����������� ��������� ��������
					Graphs->SigNumUpDwn->Tag = rTotNumOfS;//�������� �������� ����� �������� (����������� ��� ��������)
					if (Graphs->SaveExpDataToFile(2, rSignals, rRealNumOfS, true))//���������� ������ ���������
						Synchronize(&CallForReplotGraphs);//���������� ���������� ���� ��������� ���� ���������� ��������

					Graphs->SaveDlg->FileName = memoFlNm;//��������������� ��� �����
				}
				else
					if (Graphs->SaveExpDataToFile(rTypeOfExp, rSignals, rRealNumOfS, true))//���������� ������
						Synchronize(&CallForReplotGraphs);//���������� ���������� ��������
			}
		}
		else
			Experiment->DevEvents->Text = "��� ��������";

		//�������� ��������� � ��������� � �������, ����� ��� ������������ (pra) ���������� ������
		if ((rRealNumOfS < 1) || (PStart->GetASignal->Tag == 1) || rAppEndData)//��� �������� ��� ������ ������ ������� ��� ����������� ������
		{
			Graphs->DeleteStructSignal(rSignals, rTotNumOfS);//����������� ������, ���� ��� ����������
		}

		rSignals = NULL;//�������� ���������
		delete[] rInstr; rInstr = NULL;//������ ������ � ������������
		if (rAppEndData)//����������� ������
			CompleteDisconFile();//�������� ����� ����������� ������
		delete[] rmGrafik; rmGrafik = NULL;//�������� ������� ��� ���������
		if (rmGrafikM)
		{
			delete[] rmGrafikM; rmGrafikM = NULL;//�������� ������� ��� ��������� ���������� �������� (� ������ �4)
        }
		delete[] rExtractLocMean; rExtractLocMean = NULL;//�������� ������� � ��������� �������

		//��������������� ������
		PStart->GetASignal->Enabled = true;//����� ������� �������
		PStart->StartRec->Enabled = true;//������ ������
		PStart->Invert->Enabled = true;//������ �������� �������
		PStart->NulMinus->Enabled = true;//����� ����-�����
		PStart->SignalLen->Enabled = true;//����� �������
		PStart->LenSpont->Enabled = true;//����� ����������� �������
		PStart->BackTime->Enabled = true;//����� ����� ��� ����������� �������

		PStart->CloseWin->Caption = "�������";//������ "�������" ������
		PStart->CloseWin->Tag = 1;//������ ������� ������

		ExpNotes->addUMark->Enabled = true;//��������� ���������� ����������� (���� ����� ������)

		Graphs->gphOpen->Enabled = true;//��������� ��������� �����
		Experiment->beginFOpen->Enabled = true;//��������� ��������� �����
		Experiment->verticalScal->Enabled = true;//������ �� ��������� �������� ������������ �����
		Experiment->timScal->Enabled = true;//������ �� ��������� �������� �������������� �����
		Experiment->DiscreTime->Enabled = true;//����� ������������� ����� ����� ������
		Graphs->SignalTrack->OnMouseDown = Graphs->SignalTrackMouseDown;//��������� ����������� �� �����

		Suspend();//������ ����� �� �����
	}
}
//---------------------------------------------------------------------------

void TRecordThread::CreatExtendFile()
{
	//������� ��� ���������� ���� � ����������� ������
	__int32 itemWrtRd,//���������� ����������� ������ (�� ����)
			amountOfBytes,//���������� ����, ������������ � ������ �����
			backTimeMini;//����� ����� ��� ���������� ��������
	TDateTime CurrentDateTime;//������� ���� � �����
	AnsiString progDate;//���������� � ��������� � ���� �������� ����� (��� ������������ ������)
	char lett[3];//������-������ (������������ ������ � ����; ����������� �����)

	CurrentDateTime = Now();//������� ���� � �����

	//������-��������� ���� ��� ������-������
	//rDiscontinFile = CreateFile(Graphs->SaveDlg->FileName.c_str(), FILE_ALL_ACCESS, 0, 0, OPEN_ALWAYS, 0, 0);
	if ((FileExists(Graphs->SaveDlg->FileName)) && (Experiment->DiscontinWrt->Tag == 1)) //���������� �� ����
	{
		progDate = "\nED:";//����������� ����� �������� ����� ������ (�������� � ������������ ����)
		progDate += CurrentDateTime.DateTimeString().c_str();
		rDiscontinFile = fopen(Graphs->SaveDlg->FileName.c_str(), "ab+");//��������� ��� ������ � ��������
		itemWrtRd = fseek(rDiscontinFile, 0, SEEK_END);//��� � ����� �����
	}
	else
	{
		//�������� ��������� ����� � ������ ���
		progDate = "ElphAcqu v" + progVer + "\nmade";//~18 ��������
		progDate += CurrentDateTime.DateTimeString().c_str();//19 ��������
		progDate += "\nExpandable file";

		rDiscontinFile = fopen(Graphs->SaveDlg->FileName.c_str(), "wb");//��������� ��� ������
	}
	amountOfBytes = sizeof(char) * progDate.Length();//���������� ����, ������������ � ������ �����
	amountOfBytes += sizeof(bool) + (3 * sizeof(char)) + (5 * sizeof(__int32)) + sizeof(short) + sizeof(float);

	//��������� ��������� ��� �����������
	itemWrtRd = fwrite(progDate.c_str(), sizeof(char), progDate.Length(), rDiscontinFile);
	amountOfBytes -= itemWrtRd * sizeof(char);

	lett[0] = 'P'; lett[1] = 'r'; lett[2] = 'M';//��������� �� ������ ����� ���������� ������� ������
	itemWrtRd = fwrite(&lett, sizeof(char), 3, rDiscontinFile);//��������� ������
	amountOfBytes -= itemWrtRd * sizeof(char);

	//����������, ���������� ��� ������� ������ (�.�. ��� ����������� ���������)
	/* ������������������ ������ ������������ ����������
		multiCh -------(1) - ����� ����� ������ (true = ��������������)
		recLen --------(2) - ����� �������� ������� (�������)
		discrT --------(3) - ����� �������������
		chanls --------(4) - ���������� ����������� �������
		E->adcGain ----(5) - ����������� ��������
		experimentType (6) - ��� ������������, ���������� �������
		E->maxVoltage -(7) - �������� ����������
		E->maxADCAmp --(8) - ������������ ��������� (�������)
	*/

	itemWrtRd = fwrite(&multiCh, sizeof(bool), 1, rDiscontinFile);//1//����� ����� ������ (true = ��������������)
	amountOfBytes -= itemWrtRd * sizeof(bool);
	itemWrtRd = fwrite(&recLen, sizeof(__int32), 1, rDiscontinFile);//2//����� �������� ������� � ��������
	amountOfBytes -= itemWrtRd * sizeof(__int32);
	itemWrtRd = fwrite(&discrT, sizeof(float), 1, rDiscontinFile);//3//����� �������������
	amountOfBytes -= itemWrtRd * sizeof(__int32);
	itemWrtRd = fwrite(&chanls, sizeof(__int32), 1, rDiscontinFile);//4//���������� ����������� �������
	amountOfBytes -= itemWrtRd * sizeof(__int32);
	itemWrtRd = fwrite(&r_a.m_nGain, sizeof(__int32), 1, rDiscontinFile);//5//����������� ��������
	amountOfBytes -= itemWrtRd * sizeof(__int32);
	itemWrtRd = fwrite(&rTypeOfExp, sizeof(short), 1, rDiscontinFile);//6//��� ������������, ���������� �������
	amountOfBytes -= itemWrtRd * sizeof(short);
	itemWrtRd = fwrite(&Experiment->maxVoltage, sizeof(float), 1, rDiscontinFile);//7//�������� ����������
	amountOfBytes -= itemWrtRd * sizeof(float);
	itemWrtRd = fwrite(&Experiment->maxADCAmp, sizeof(__int32), 1, rDiscontinFile);//8//������������ ��������� (� ��������)
	amountOfBytes -= itemWrtRd * sizeof(__int32);

	if (amountOfBytes != 0)
		Experiment->DevEvents->Text = "������ ��������� ������";//��������� �����, ������, ����� �� ������
}
//---------------------------------------------------------------------------

void TRecordThread::CompleteDisconFile()
{
	//��������� ���� � ����������� ������
	__int32 i, j, z,
			*pMarkNums,//������ ����� � �������, ������� ����� ���������
			lettersNum,//����� ��������� ������
			amountOfBytes,//���������� ����, ������������ � ������ �����
			itemWrtRd,//���������� ����������� ������ (�� ����)
			comUserMarks;//����� ���������� ����� ������������
	AnsiString userText;//���������� �� ������������ (������ ������������)
	char lett[3];//������-������ (������������ ������ � ����; ����������� �����)
	bool *markerVsblt;//��������� ����� �� �������

	//��������� ������� �� ���� ������������, ���� ����� ����
	/* ������������������ ������ � ����� � ���������� ������
		lettersNum -(1) - ����� ��������� ������
		comUserMakrs(2) - ���������� ������� �� ���� ������������
		userText ---(3) - ����� �� ������������ � ������� �� ����
		pointNums --(4) - ������ � �������� ����� � ���������
	*/

	//���������� �� ������������ (������ ������������);
	userText = ExpNotes->usersNotes->Lines->Text.c_str();
	comUserMarks = (ExpNotes->addUMark->Tag) - (ExpNotes->nmInRec);//���������� �����, �������� � ������ ������ ����� ������

	if (!userText.IsEmpty() || (comUserMarks > 0))//������������ ������ ��������� ����������
	{
		//��������� � ���������� userText ������� �� ���� ������������
		if (comUserMarks > 0)//������������ ������ ����� �� ���� ������������
		{
			pMarkNums = new __int32[comUserMarks];//������ ����� � �������, ������� ����� ���������
			markerVsblt = new bool[comUserMarks * ftChan];//��������� ����� �� �������

			if (userText.IsEmpty())
				userText = "\n\n\r\r\n\n";//�����������
			else
				userText += "\n\n\r\r\n\n";//�����������

			for (i = 0; i < ExpNotes->addUMark->Tag; i++)//
			{
				if (ExpNotes->theMarker->pointOnGraph >= ExpNotes->npNewRec)//����� ������� � ������ ������ ����� ������
				{
					userText += ExpNotes->theMarker->textMark;
					userText += "||\n||";
				}
				ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
			}
		}
		lettersNum = userText.Length();//����� ��������� ������
		amountOfBytes = (userText.Length() * sizeof(char)) + (3 * sizeof(char)) + //��������� �����
						(2 * sizeof(__int32)) + //���������� �������� � �����
						((comUserMarks * __int32(comUserMarks > 0)) * sizeof(__int32)) + //������ �������� � �������
						((comUserMarks * ftChan * __int32(comUserMarks > 0)) * sizeof(bool));//��������� ����� �� �������

		lett[0] = 'M'; lett[1] = 'r'; lett[2] = 'k';//��������� �� ������ ���������������� ����������
		itemWrtRd = fwrite(&lett, sizeof(char), 3, rDiscontinFile);//��������� ����������� (Mrk)
		amountOfBytes -= itemWrtRd * sizeof(char);

		itemWrtRd = fwrite(&lettersNum, sizeof(__int32), 1, rDiscontinFile);//1//����� ��������� ������
		amountOfBytes -= itemWrtRd * sizeof(__int32);

		itemWrtRd = fwrite(&comUserMarks, sizeof(__int32), 1, rDiscontinFile);//2//���������� �������
		amountOfBytes -= itemWrtRd * sizeof(__int32);

		/* ������������������ ������ ������ � pra-�����:
			refToWrite[29] = userText.c_str();//������ �� ���� �1
			refToWrite[30] = (__int32*)(ExpNotes->pointNums);//������ �� ���� �16	*/

		//������� ���������� ��� �����, ����� ������ ����� � �������
		itemWrtRd = fwrite(userText.c_str(), sizeof(char), userText.Length(), rDiscontinFile);//��������� ������� ������������
		amountOfBytes -= itemWrtRd * sizeof(char);

		//����� ������ ����� �� ������� � �������
		j = 0;//������ � ������� pMarkNums
		for (i = 0; i < ExpNotes->addUMark->Tag; i++)//comUserMarks > 0
		{
			if (ExpNotes->theMarker->pointOnGraph >= ExpNotes->npNewRec)//����� ������� � ������ ������ ����� ������
			{
				pMarkNums[j] = ExpNotes->theMarker->pointOnGraph - ExpNotes->npNewRec;
				for (z = 0; z < ftChan; z++)
					markerVsblt[(j * ftChan) + z] = ExpNotes->theMarker->chanN[z];//��������� ����� �� �������
				j++;//����������� ������ � ������� pMarkNums
			}
			ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
		}

		if (comUserMarks > 0)//������������ ������ ����� �� ���� ������������
		{
			itemWrtRd = fwrite(pMarkNums, sizeof(__int32), comUserMarks, rDiscontinFile);//��������� ������ �������� � �������
			amountOfBytes -= itemWrtRd * sizeof(__int32);
			itemWrtRd = fwrite(markerVsblt, sizeof(bool), (comUserMarks * ftChan), rDiscontinFile);//��������� ��������� ����� �� �������
			amountOfBytes -= itemWrtRd * sizeof(bool);

			delete[] pMarkNums; pMarkNums = NULL;
			delete[] markerVsblt; markerVsblt = NULL;
		}

		if (amountOfBytes != 0)
			Experiment->DevEvents->Text = "������ �������� ������";
	}

	fclose(rDiscontinFile);//��������� ����, � ������� ��������� � ����������� ������
	rDiscontinFile = NULL;
}
//---------------------------------------------------------------------------

void TRecordThread::StartRec(__int32 allExcit, __int32 allSpont, __int32 pp)
{
	//������ �������� ����������
	/*
	allExcit - ����� ���������� ������������ �������� (� ������� �1, 2, 3)
	allSpont - ���������� ���������� ���������� �������� � ������ �4
	pp - ���������� ������� ��������� ���������� (���������� ����������)
	*/

	__int32 i, z, g,
            errC,//���������� ���������
			s_cycls,//���������� �������� �� ����� � ��������
			blockReady,//���������� ����� ������
			notZero,//���������� �������
			numOfS,//����� ��������
			startNum,//������ ��������� ��������
			stimPrd;//������ ����������
	__int64 curnttic, ticps;//����� ��� ������ �� ��������
	bool canPlot;//��������� �� �������� (������� "���������" �����)
	clock_t experimBegin;//������ ������ �������� (������ �� ������ ������ ���������)
	unsigned short cH, cM, cS, cmS,//������� �����
				   tH, tM, tS;//������� �����

	//��������� �������� �����
	s_cycls = 1;//������� ������ (���������� ��������)
	Graphs->CrntSig->Tag = 0;//��������� (��������) ���������� � ����
	PStart->BackTime->Tag = 0;//��������� ��� ���������� �������� ����� ��������

	if ((PStart->GetASignal->Tag != 1) && (ProtoBuild->StarTim->Checked))//�� ������ ������� � ����� ������� ������
	{
		tH = (unsigned short)StrToInt(ProtoBuild->HEdit->Text);//����
		tM = (unsigned short)StrToInt(ProtoBuild->MEdit->Text);//������
		tS = (unsigned short)StrToInt(ProtoBuild->SEdit->Text);//�������
		Now().DecodeTime(&cH, &cM, &cS, &cmS);
		while (((tH > cH) || (tM > cM) || (tS > cS)) &&
			   (PStart->CloseWin->Tag != 1) && (PStart->NextBlock->Tag != 1))
		{
			Now().DecodeTime(&cH, &cM, &cS, &cmS);
			Sleep(100);//��� ������� ���� �������
		}
	}

	z = 0;//���������� ���������
	while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (z <= 2))//������� ����������
		z++;
	if (z > 2)
	{   Experiment->DevEvents->Text = "�� ���������� ������� ����������"; return; }

	//rBeginTakt - ���� ����������, ��� ������� ���������� ���������� ��������
	z = 0;//���������� ���������
	while((!QueryPerformanceCounter((LARGE_INTEGER *)&rBeginTakt)) && (z <= 2))
		z++;
	if (z > 2)
	{   Experiment->DevEvents->Text = "������ QPcounter"; return; }

	experimBegin = clock() / 1000;//������ ������ �������� (������ �� ������ ������ ���������)
	QueryPerformanceCounter((LARGE_INTEGER *)&rNextTic);//������ ������ ���������� ����� (����� ��������� � ������� ��������)

	specK1 = Graphs->CrntSig->Tag;//=0//����� �������, � �������� ���������� �������������� �������
	for (i = 0; (i < pp) && (PStart->CloseWin->Tag == 0); i++)//���� �������� ����������
	{
		if (rInstr[(i * 3) + 0] == 1)//����� ��������� ����������
		{
			numOfS = rInstr[(i * 3) + 1];//���������� ��������� ���������� (��������)
			stimPrd = rInstr[(i * 3) + 2];//������ ������ ��������� ���������� (������������)
			canPlot = bool((stimPrd - __int32(float(recLen * effDT) / 1000)) >= minFreeTime);//��������� �� �������� (������� "���������" �����)

			PStart->NextBlock->Tag = 0;//���� ��������������� 1, �� ������� �� �����

			//��������� ���������� � ���� ������ � ����������� �� ���� ������������
			if (rTypeOfExp == 1)//����������
				Spnt_SgnlAcquisition(numOfS, experimBegin);//����������
			else if ((rTypeOfExp == 2) || (rTypeOfExp == 3))//���������
			{
				Induced_SgnlAcquisition(numOfS, stimPrd, canPlot);//������ ���������
				if (!canPlot && (PStart->GetASignal->Tag != 1))//�������� ������ � �� ������ �������
				{
					if (i < (pp - 1))//�� ��������� ����
						if (rInstr[((i + 1) * 3) + 0] == 2)//��������� ���� - �����
							canPlot = bool((rInstr[((i + 1) * 3) + 2] - __int32(float(recLen * effDT) / 1000)) >= minFreeTime);
					if (canPlot)
					{
						specK2 = Graphs->CrntSig->Tag - specK1;//���������� �������������� ��������
						Synchronize(&InducSynchroPlotter);//��������� ����� �� ������
						specK1 = Graphs->CrntSig->Tag;//����� �������, � �������� ���������� �������������� �������
					}
				}
			}
			else//(rTypeOfExp == 4)//��������� + ����������
				Induced_N_Spnt_SgnlAcquis(allSpont, numOfS, stimPrd, canPlot);//��������� + ����������

			startNum = Graphs->CrntSig->Tag;//���������� �������� ���������
			QueryPerformanceCounter((LARGE_INTEGER *)&rNextTic);//������ (� ������ ����������) ������ ���������� ��������������
		}
		if (rInstr[(i * 3) + 0] == 2)//����� (��������� Sleep)
		{
			errC = 0;//���������� ���������
			while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//������� ����������
				errC++;
			if (errC > 2)
			{   Experiment->DevEvents->Text = "�� ���������� ������� ����������"; return; }
			rNextTic = rBeginTakt + (__int64(rSignals[startNum - 1].appear) * ticps) +
					   (__int64(rInstr[(i * 3) + 2]) * (ticps / 1000));//������ (� ������ ����������) ������ ���������� ��������������

			//������� � �������������� Sleep(�) �� ������ ������� ������ ���������� �����
			//if (rInstr[(i * 3) + 2] > 1500)//��������� ���� ������� ��� ��������� ������� ������
			//	Sleep(floor((rInstr[(i * 3) + 2] - 1000) + 0.5));

			//������� � ������, ������ �������� Sleep(50) (����� ����� �������, ��� 5 ��)
			if (rInstr[(i * 3) + 2] > 1050)//���� ����� ������ ����� �������
			{
				while(!QueryPerformanceCounter((LARGE_INTEGER *)&curnttic)) {};
				while((curnttic < (rNextTic - ticps)) && (PStart->CloseWin->Tag == 0))//���������� ����� �� ���� ������� ������ (rNextTic - ticps)
				{
					Sleep(50);
					while(!QueryPerformanceCounter((LARGE_INTEGER *)&curnttic)) {};
				}
			}
		}
		if (rInstr[(i * 3) + 0] == 3)//���� (���������� ��������� �������)
			if (s_cycls < rInstr[(i * 3) + 1])
			{
				i = -1;//��� �� ������ ������ ���������� (���� ���, �� ����� ���������� �� �������� ����������)
				s_cycls++;
			}
	}
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::Induced_SgnlAcquisition(__int32 iNumOfS, __int32 iStimPrd, bool canPlot)
{
	//���� ������: ���������, ������ �2 � �3
	/*
	iNumOfS - ����� ���������� � ������ ����� ��������
	iStimPrd - ������ ���������� � ������ ����� (������������)
	canPlot - ��������� �� �������� (������� "���������" �����)
	*/

	bool isSynchro;//��������� �� �������������
	short *drvData,
		  convData[maxChannels];//�������� ���������� ��� ��� ��� ���� ������� (������� �� ������, ��� maxChannels)
	unsigned short digitPort4;//�������� ������� ������ ����� �������� ��������� ����� (����� ���������������)
	__int32 i, z, j,
			errC,//������� ������
			ik,//���������� ���������� ��������
			kRefresh,//������� ���������� (���������� ���������� (��� �������� �� ������� ����))
			refreshNum,//������ ���������� ����������
			startNum,//��������� �������� � ������� ��������� ������������ � ���������� � startNum
			stims,//���������� �������� ���������������
			recorded,//������� ���������� ��������
			blockReady,//���������� ����� ������
			minorChanMean[maxChannels],//������� �� �������������� ������ �� ����� ��������������� ��������
			impInterv,//������ ������� �������� ������������� ���������� (� ��������)
			sampRead;//����� ���������� ��������, ������� � ������� ������������� ��������������
    float waitTime;//������������ ����� (���) ����� ����������� ��������������
	unsigned int mask;//������ ���������� � ��������� ������
	__int64 *appear,//������ � ��������� ������ ��������������� (� ������ ����������)
			totaltic,//����� ����� ������ ����������, ��������� ��� �������� ����� (�� ��������� ����� ������� ������� �� �����)
			tpp,//������ �� ����� ��������������
			tp1,
			curnttic,//������� ���� ����������
			ticps,//������ � ������� (������� ����������)
			ticperiod,//������ ���������� �� ������ ����������
			popravka;//��������������� ������� ����������

    drvData = NULL;
	appear = NULL;

	refreshNum = iNumOfS + 1;//�������� ������ ���������� ������ ����� ���������� �������� (�.�. ��� ����������)
	if (Experiment->ggRefresh->Checked)
		refreshNum = StrToInt(Experiment->refreshEvery->Text);

	recorded = 0;//�������� ������� ���������� ��������
	curnttic = 0;
	stims = 0;//���������� �������� ���������������
	appear = new __int64[iNumOfS];//������ � ��������� ������������� ��������

	errC = 0;//���������� ���������
	while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//������� ����������
		errC++;
	if (errC > 2)
	{   Experiment->DevEvents->Text = "������ QPFrequency"; return; }
	ticperiod = (ticps / (__int64)1000) * (__int64)iStimPrd;//������ ���������� �� ������ ����������

	//���������� ����������� ����� �������� ����� ���������� ���������� (� ��������)
	impInterv = floor((1000 * iStimPrd) / (2 * effDT));//������������ ����� ���������� ���������� (�������)
	sampRead = impInterv;//����� ������ ������ ������� ��� �������
	startNum = Graphs->CrntSig->Tag;//����� �������, � �������� ������������ ���������
	ik = startNum;//���������� ���������� �������� (�������� ���������)
	kRefresh = 0;//������� ���������� �� ����
	waitTime = min(float(500), discrT);//������������ ����� (���) ����� ����������� ��������������
	tpp = (float)ticps * (waitTime / (float)1e6);//������ �� ����� ��������������

	z = r_pUtil->Start(&r_a, 0);//����� ����� ������
	if (z != 1)
	{
		Experiment->DevEvents->Text = "������ ADC_Start";
		return;
	}
	z = 0; errC = 0;//��������� ������� �� ���� ������
	while ((z == 0) && (errC <= 200))
	{
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0
		errC++;
	}
	if ((errC > 200) && (z == 0))
	{   Experiment->DevEvents->Text = "��� ������"; return; }
	z = 0; errC = 0;//������������� ���� �� ������ ���
	while ((z == 0) && (errC <= 2))
	{
		z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[1]);//������������� ���� �� ������ ���
		Sleep(1);//��� �������� �� ������� �� ����� 5 ���
		errC++;
	}
	if (errC > 2)
	{   Experiment->DevEvents->Text = "������ ADC_WRITE_DAC"; return; }

	for (i = 0; i < 2; i++)//"�����������" ����
	{
		blockReady = 0;
		while (blockReady == 0)
			blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

		z = 0; errC = 0;//����������� ����������� ���� ������ (�����)
		while ((z == 0) && (errC <= 2))
		{
			z = r_pUtil->FreeBuffer();
			errC++;
		}
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "err FreeBuf (IS r)";
			break;//��������� ���������� ������������ �����
		}
	}

	QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);//������� ���� ����������
	if (curnttic > rNextTic)
		totaltic = curnttic + (iNumOfS * ticperiod) + (2 * ticps);//����� totaltic ��������������
	else
		totaltic = rNextTic + (iNumOfS * ticperiod) + (2 * ticps);//����� totaltic ��������������
    
	while (((ik < (startNum + iNumOfS)) || (recorded > 0)) && (curnttic <= totaltic))
	{
        //���� ������ ���������������
		//curnttic = DaiImpuls(&appear[stims], &iNumOfs, ik, startNum);//������ ������� ������ ��������������
		errC = 0;//���������� ���������
		while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//������� ����������
			errC++;
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "������ QPFrequency";
			iNumOfS = ik - startNum;//������� ������ �� �����
			recorded = 0;//������� ������ �� �����
		}
		ticperiod = (ticps / (__int64)1000) * (__int64)iStimPrd;//������ ���������� �� ������ ����������

		errC = 0;//���������� ��������� (�� ��� ����� ����������� �������� ����������)
		while((!QueryPerformanceCounter((LARGE_INTEGER *)&curnttic)) && (errC <= 2))
			errC++;
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "������ QPCounter";
			iNumOfS = ik - startNum;//������� ������ �� �����
			recorded = 0;//������� ������ �� �����
		}

		popravka = rNextTic - curnttic;//������ �� ������������ ������� ������ �������������� (������ ��������)
		if ((popravka <= 0) && (stims < iNumOfS))//����� �������������� �������
		{
			//SetToDAC();
			z = 0; errC = 0;//������������� ������ �� ������ ���
			while ((z == 0) && (errC <= 2))
			{
				z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[0]);//������������� ������ �� ������ ���
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "������ ADC_WRITE_DAC";
				iNumOfS = ik - startNum;//������� ������ �� �����
				recorded = 0;//������� ������ �� �����
			}
			
			if (((popravka + ticperiod) < 0) || (stims == 0))
				rNextTic = curnttic + ticperiod;//������ (� ������ ����������) ������ ���������� ��������������
			else
				rNextTic = curnttic + ticperiod + popravka;//������ (� ������ ����������) ������ ���������� ��������������
			appear[stims] = curnttic;//���������� ����� ������������� �������
			stims++;//����������� ������� ������� ���������������

			//������ ����� ����� ����������� ��������������
			tp1 = curnttic;
			while ((curnttic - tp1) < tpp)
				QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);

			z = 0; errC = 0;//������������� ���� �� ������ ���
			while ((z == 0) && (errC <= 2))
			{
				z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[1]);//������������� ���� �� ������ ���
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "������ ADC_WRITE_DAC";
				iNumOfS = ik - startNum;//������� ������ �� �����
				recorded = 0;//������� ������ �� �����
			}
		}
		//===== ��������� ����� ������ ��������������� =====

		blockReady = r_pUtil->GetBuffer((void*)drvData, mask);//����������� ���� ������ �� ������ DMA
		if (blockReady != 0)//���� ������ �������
		{
			if ((PStart->CloseWin->Tag == 1) || (PStart->NextBlock->Tag == 1))//�� ���� �� ������������?
				iNumOfS = ik - startNum;//��������������� ����� (�� ������������ ��� �����)

			for (i = 0; i < rBlockSize; i += chanls)//���������� ������� ����� ������
			{
				for (z = 0; z < chanls; z++)
					convData[z] = (drvData[i + rReadOrder[z]] >> bwSh);//�������� ��� ��� (������� 12 (14) ���)

				sampRead++;//������� �������� ����������� ��������
				//���������� "�" oDrvData[i] & 0000000000001111 - �������� ��������� 4 ����
				//0xF = 15(dec) = 0000000000001111(bin); 0xFFF0 = 65520(dec) = 1111111111110000(bin)
				digitPort4 = ((unsigned short)drvData[i]) & eds;//(oDrvData[i] << 12)
				isSynchro = ((digitPort4 > 2) && (sampRead >= impInterv));//��������� �� �������������

				if (recorded > 0)//�������� ������� (����������� recorded < recLen)
				{
					for (z = 0; z < ftChan; z++)//������ ������ � ���� �������
					{
						rSignals[ik].s[(z * recLen) + recorded] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
						rmGrafik[(z * recLen) + recorded] = (double)rSignals[ik].s[(z * recLen) + recorded] * sampl2mV;
					}
					/*else ����� ������ ��������� �������*/
					recorded++;//����������� ������� ���������� �������� ��� ������� �������

					//�����! ������ �� ���������� �������������� �� ���������� ������ (�������������� ������� ����������)
					if (isSynchro && (recorded > impInterv))//������������� ��� ��������, � ������ ��� �� �������
					{
						recorded = recLen;//������� ������ ��������� ����������
						i -= chanls;//��� �����, ����� �� ��������� ����� ����� isSynchro ���� ���� true
					}

					if (recorded >= recLen)//������ ��������� �������
					{
						if (!multiCh)//������� ������� ��� �������������� �������
							for (z = 0; z < chanls - 1; z++)
								rSignals[ik].s[recLen + z] = (short)rLocMean[z + 1];//����-����� �� �������������� ������
								//rSignals[ik].s[recLen + z] = (short)(minorChanMean[z] / recLen);//��������� �������

						recorded = 0;//������ ��������� ������� (�������� ������� ���������� ��������)
						ik++;//����������� ������� ��������� ��������

						Graphs->CrntSig->Tag = ik;//������� ��������
						Synchronize(&RecSynchroCounter);//������� ��������

						//==========================================
						if (rAppEndData)//���� ������ ����������� �����
							DiscontinWrite();//��������� ���������� ������ �� ������ ����
						//==========================================

						if (canPlot)//���� "���������" ��� "������" ����������
						{
							specK1 = Graphs->CrntSig->Tag - 1;//����� �������, � �������� ���������� ������������� �������
							specK2 = 1;//���������� �������������� ��������
							Synchronize(&InducSynchroPlotter);//��������� �������
							Synchronize(&FromMinorChan);//������� ������ �� �������������� ������� (������ ��� ������ ����������)
							kRefresh++;
							if (kRefresh >= refreshNum)
							{
								kRefresh = 0;//������� ���������� �� ����
								Gists->GistsRefresh(-1);//�������������� �����������
							}
							/* // ? ��������� ����� ��� ������� ���������� ����� ����������
							QueryPerformanceCounter((LARGE_INTEGER *)&popravka);
							ind_pUtil->FreeBuffer(); ind_pUtil->Stop(); Sleep(sleepTime);
							if (!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) {ticperiod = (__int64)((ticps / 1000) * stimPrd);}
							ind_pUtil->Start(&ind_a, 0); QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);
							while (blockReady == 0)	{blockReady = ind_pUtil->GetBuffer((void*)drvData, mask);}
							curntsampl += (dopsampls);//(((curnttic - popravka) / ticps) * 1e6) / iDiscrTime);*/
						}
						if (ik == (startNum + iNumOfS))//��� ������� ��������� ��������
							break;//������� �� ����� for (i = 0; i < rBlockSize; i += chanls)
						totaltic = curnttic + ((startNum + iNumOfS) - ik) * ticperiod + (10 * ticps);//���� ������ ���������, ������������ totaltic
					}
				}
				else//����� ���������� ������� (��� ���� recorded = 0)
				{
					if (isSynchro)//������ ���������
					{
						rSignals[ik].appear = (float)((double)(appear[ik - startNum] - rBeginTakt) / (double)ticps);
						for (z = 0; z < ftChan; z++)//������ ������ �� ���� �������
						{
							rSignals[ik].s[(z * recLen) + recorded] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
							rmGrafik[(z * recLen) + recorded] = (double)rSignals[ik].s[(z * recLen) + recorded] * sampl2mV;
						}
						/*else ����� ������ ��������� �������*/
						recorded++;//����������� ������� ���������� �������� ��� ������� �������
						sampRead = 0;//�������� ������� ����������� ��������
						totaltic = curnttic + ((startNum + iNumOfS) - ik) * ticperiod + (10 * ticps);//���� ������ ���������, ������������ totaltic
					}
					else//��������� ���������� �������� ������ � ��������� ������
					{
						for (z = 0; z < chanls; z++)
						{
							rLocMean[z] += (double(convData[z] - rExtractLocMean[rCountlm * chanls + rReadOrder[z]]) / (double)rSamps);
							rExtractLocMean[rCountlm * chanls + rReadOrder[z]] = convData[z];
						}
						rCountlm++;
						if (rCountlm == rSamps)
							rCountlm = 0;
					}
				}
			}
			z = 0; errC = 0;//����������� ����������� ���� ������ (�����)
			while ((z == 0) && (errC <= 2))
			{
				z = r_pUtil->FreeBuffer();
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "err FreeBuf (IS r)";
				iNumOfS = ik - startNum;//������� ������ �� �����
				recorded = 0;//������� ������ �� �����
			}
		}
	}

	z = 1; errC = 0;//���� ����� ������
	while ((z == 1) && (errC <= 2))
	{
		r_pUtil->Stop();
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0
		errC++;
	}
	if (errC > 2)
		Experiment->DevEvents->Text = "������ ADC_Stop";

	//��������� rExtractLocMean ��������� � ����� Execute()
	drvData = NULL;
	delete[] appear; appear = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::Induced_N_Spnt_SgnlAcquis(__int32 allMini, __int32 iNumOfS, __int32 iStimPrd, bool canPlot)
{
	//���� ������: ��������� + ���������� (����� �4)
	/*
	allMini - ����� ���������� ���������� ���������� ��������
	iNumOfS - ����� ���������� � ������ ����� ��������� ��������
	iStimPrd - ������ ���������� � ������ �����
	canPlot - ��������� �� �������� (������� "���������" �����)
	*/

	bool isSynchro;//��������� �� �������������
	short *drvData,
		  convData[maxChannels],//�������� ���������� ��� ���
		  *backBuffer;//�������� ����� ���������� ���� (���������� ������)
	unsigned short digitPort4;//�������� ������� ������ ����� �������� ��������� �����
	__int32 i, g, z, j,
			errC,//������� ������
			ik,//������� ���������� ��������� ��������
			kr,//������� ���������� (���������� ������)
			startNum,//��������� �������� � ������� ��������� ������������ � ���������� � startNum
			mk,//������� ���������� ���������� ��������
			samplBack,//������� ����� (�������������� �� spontanBackTime)
			bI,//������ ������� � ������, � �������� ������ ���������� �����
			stims,//���������� �������� ���������������
			recordedI,//������� ���������� �������� ��� ���������� �������
			recordedM,//������� ���������� �������� ��� ����������� �������
			blockReady,//���������� ����� ������
			frstIndBack,//������ ������ ��� ��������� (���������� ������)
			//refreshNum,//������ ���������� �������� (���������� ������)
			ost,//������� (������� �������� ������ �� ��������� �����, ����������� ������)
			minorChanMean[maxChannels],//������� �� �������������� ������
			impInterv,//������ ������� �������� ������������� ���������� (� ��������)
			sampRead;//����� ���������� ��������, ������� � ������� ������������� ��������������
	float waitTime;//������������ ����� (���) ����� ����������� ��������������
	unsigned int mask;//������ ���������� � ��������� ������
	__int64 *appear,//������ � ��������� ������ ��������������� (� ������ ����������)
			totaltic,//����� ����� ������ ����������, ��������� ��� �������� ����� (�� ��������� ����� ������� ������� �� �����)
			curnttic,//������� ���� ����������
			tpp,//������ �� ����� ��������������
			tp1,
			ticps,//������ � ������� (������� ����������)
			ticperiod,//������ ���������� �� ������ ����������
			popravka;//��������������� ������� ����������
	bool recMini;//��������� �� ���������� ���������� �������
	
	drvData = NULL; backBuffer = NULL; appear = NULL;

	samplBack = chanls * floor((StrToInt(PStart->BackTime->Text) / effDT) + 0.5);//20% ����� ����������� ������� (��� ����� ����� �������)
	//refreshNum = iNumOfS + 1;//�������� ������ ���������� ������ ����� ���������� �������� (�.�. ��� ����������)
	//if (Experiment->ggRefresh->Checked)//����� �� ��������� ������� ����������
	//	refreshNum = StrToInt(Experiment->refreshEvery->Text);

	backBuffer = new short[samplBack];//����������� �����
	for (i = 0; i < samplBack; i++)//������ ���������� ������ backBuffer (������)
		backBuffer[i] = 0;
	bI = 0;
	recordedI = 0;//�������� ������� ���������� ��������
	recordedM = 0;//�������� ������� ���������� ��������
	stims = 0;//���������� �������� ���������������
	appear = new __int64[iNumOfS];//������ � ��������� ������������� ��������

    errC = 0;//���������� ���������
	while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//������� ����������
		errC++;
	if (errC > 2)
	{   Experiment->DevEvents->Text = "�� ���������� ������� ����������"; return; }
	ticperiod = (ticps / (__int64)1000) * (__int64)iStimPrd;

	//���������� ����������� ����� �������� ����� ����������
	impInterv = floor(recLen / 2) + 1;//floor((1000 * iStimPrd * 1) / (2 * discrT));//������������ ����� ���������� ���������� (�������)
	sampRead = impInterv;//����� ������ ������ ������� ��� �������
	startNum = Graphs->CrntSig->Tag;//����� �������, � �������� ������������ ���������
	ik = startNum;//���������� ���������� ��������� �������� (�������� ���������)
	mk = PStart->BackTime->Tag;//���������� ���������� ���������� �������� (�������� ���������)
	kr = 0;//�������� ������� ���������� (���� �������� ��� ����������)
	waitTime = min(float(500), discrT);//������������ ����� (���) ����� ����������� ��������������
	tpp = (float)ticps * (waitTime / (float)1e6);//������ �� ����� ��������������

	z = r_pUtil->Start(&r_a, 0);//����� ����� ������
	if (z != 1)
	{
		Experiment->DevEvents->Text = (FindErrorStrByCode(z, 0));
		Experiment->DevEvents->Text.Insert("Induc ", 1);
		return;
	}

	z = 0; errC = 0;//��������� ������� �� ���� ������
	while ((z == 0) && (errC <= 200))
	{
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0
		errC++;
	}
	if (errC > 200)
	{   Experiment->DevEvents->Text = "������ ADC_StatusRun"; return; }

	z = 0; errC = 0;//������������� ���� �� ������ ���
	while ((z == 0) && (errC <= 2))
	{
		z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[1]);//������������� ���� �� ������ ���
		Sleep(1);//��� �������� �� ������� �� ����� 5 ���
		errC++;
	}
	if (errC > 2)
	{   Experiment->DevEvents->Text = "������ ADC_WRITE_DAC"; return; }

	for (i = 0; i < 2; i++)//"�����������" ����
	{
		blockReady = 0;
		while (blockReady == 0)
			blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

		z = 0; errC = 0;//����������� ����������� ���� ������ (�����)
		while ((z == 0) && (errC <= 2))
		{
			z = r_pUtil->FreeBuffer();
			errC++;
		}
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "������ FreeBuffer";
			break;//��������� ���������� ������������ �����
		}
	}
	if (errC > 2)
		return;//"��������" �� ������

	QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);
	if (curnttic > rNextTic)
		totaltic = curnttic + (iNumOfS * ticperiod) + (2 * ticps);//����� totaltic ��������������
	else
		totaltic = rNextTic + (iNumOfS * ticperiod) + (2 * ticps);//����� totaltic ��������������

	while (((ik < (startNum + iNumOfS)) || (recordedI > 0)) && (curnttic <= totaltic))
	{
		//���� ������ ���������������
		errC = 0;//���������� ���������
		while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//������� ����������
			errC++;
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "������ QPFrequency";
			iNumOfS = ik - startNum;//������� ������ �� �����
			recordedI = 0;//������� ������ �� �����
		}
		ticperiod = (ticps / (__int64)1000) * (__int64)iStimPrd;

		errC = 0;//���������� ��������� (�� ��� ����� ����������� �������� ����������)
		while((!QueryPerformanceCounter((LARGE_INTEGER *)&curnttic)) && (errC <= 2))
			errC++;
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "������ QPCounter";
			iNumOfS = ik - startNum;//������� ������ �� �����
			recordedI = 0;//������� ������ �� �����
		}

		popravka = rNextTic - curnttic;
		if ((popravka <= 0) && (stims < iNumOfS))//����� �������������� �������
		{
			//SetToDAC();//������������� ������ �� ������ ���
			z = 0; errC = 0;//������������� ������ �� ������ ���
			while ((z == 0) && (errC <= 2))
			{
				z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[0]);//������������� ������ �� ������ ���
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "������ ADC_WRITE_DAC";
				iNumOfS = ik - startNum;//������� ������ �� �����
				recordedI = 0;//������� ������ �� �����
			}

            if (((popravka + ticperiod) < 0) || (stims == 0))
				rNextTic = curnttic + ticperiod;//������ (� ������ ����������) ������ ���������� ��������������
			else
				rNextTic = curnttic + ticperiod + popravka;//������ (� ������ ����������) ������ ���������� ��������������
			appear[stims] = curnttic;//���������� ����� ������������� �������
			stims++;//���������� ������� ������� ���������������

			//������ ����� ����� ����������� ��������������
			tp1 = curnttic;
			while ((curnttic - tp1) < tpp)
				QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);

			z = 0; errC = 0;//������������� ���� �� ������ ���
			while ((z == 0) && (errC <= 2))
			{
				z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[1]);//������������� ���� �� ������ ���
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "������ ADC_WRITE_DAC";
				iNumOfS = ik - startNum;//������� ������ �� �����
				recordedI = 0;//������� ������ �� �����
			}
		}

		blockReady = r_pUtil->GetBuffer((void*)drvData, mask);//���������� ���� ������ �� ������ DMA
		if (blockReady != 0)//���� ������ �������
		{
			if ((PStart->CloseWin->Tag == 1) || (PStart->NextBlock->Tag == 1))//�� ���� �� ������������?
				iNumOfS = ik - startNum;//��������������� ����� (�� ������������ ��� �����)

			//����� � ������ �������� (���� � ����� �����)
			for (i = 0; i < rBlockSize; i += chanls)//���������� ������� � ����� ������
			{
				//��������� ���� ��� � ���������� ������������ ������
				for (z = 0; z < chanls; z++)
				{
					convData[z] = (drvData[i + rReadOrder[z]] >> bwSh);//�������� ��� ��� (������� 12 (14) ���)
					backBuffer[bI] = convData[z];//������ � �����
					bI++;//������� �����
				}
				if (bI >= samplBack)
					bI = 0;//������� �� ������ ������������ ������

				sampRead += chanls;//������� �������� ����������� ��������
				recMini = ((mk < allMini) && (PStart->PausSpontan->Tag == 0) && (recordedI == 0));//��������� �� ���������� ���������� �������

				//���������� "�" oDrvData[i] & 0000000000001111 - �������� ��������� 4 ����
				//0xF = 15(dec) = 0000000000001111(bin)
				digitPort4 = ((unsigned short)drvData[i]) & eds;//(oDrvData[i] << 12) >> 12;
				isSynchro = ((digitPort4 > 2) && (sampRead >= impInterv));//��������� �� �������������

				//---------------------------------
				//����� � ������ ��������� ��������
				if (recordedI > 0)//�������� ������� (����������� recorded < toRec)
				{
					for (z = 0; z < ftChan; z++)//������ ������ �� ���� �������
					{
						rSignals[ik].s[(z * recLen) + recordedI] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
						rmGrafik[(z * recLen) + recordedI] = (double)rSignals[ik].s[(z * recLen) + recordedI] * sampl2mV;
					}
					/*else ����� ������ ��������� �������*/
					recordedI++;//����������� ������� ���������� �������� ��� ������� �������

					//�����! ������ �� ���������� �������������� �� ���������� ������ (�������������� ������� ����������)
					if (isSynchro && (recordedI > impInterv))// && (ik < (startNum + iNumOfS)))
					{
						recordedI = recLen;//������� ������ ��������� ����������
						i -= chanls;//��� �����, ����� �� ��������� ����� ����� isSynchro ���� ���� true
					}

					if (recordedI >= recLen)//������ ��������� �������
					{
                        if (!multiCh)//������� ������� ��� �������������� �������
							for (z = 0; z < chanls - 1; z++)
								rSignals[ik].s[recLen + z] = (short)rLocMean[z + 1];//(short)(minorChanMean[z] / recLen);//��������� �������
								
						recordedI = 0;//������ ������� ��������� (�������� ������� ���������� ��������)
						ik++;//����������� ������� ��������� ��������

						Graphs->CrntSig->Tag = ik;//������� ��������
						Synchronize(&RecSynchroCounter);//������� ��������

						//==========================================
						if (rAppEndData)
							DiscontinWrite();//��������� ���������� ������ �� ������ ����
						//==========================================

						if (canPlot)//���� "���������" ("������") ����������
						{
							//��������� �������
							specK1 = Graphs->CrntSig->Tag - 1;//����� �������, � �������� ���������� ������������� �������
							specK2 = 1;//���������� �������������� ��������
							Synchronize(&InducSynchroPlotter);//��������� ��������

							//������� ������ �� �������������� ������� (������ ��� ������ ����������)
							Synchronize(&FromMinorChan);
						}
						if (ik == (startNum + iNumOfS))//��� ������� ��������� ��������
							break;//������� �� ����� for (i = 0; i < rBlockSize; i += chanls)

						//���� ������ ���������, ������������ totaltic
						totaltic = curnttic + ((startNum + iNumOfS) - ik) * ticperiod + ticps;
					}
				}
				else//����� ���������� ������� (��������������, ��� ���� recordedI = 0)
				{
					if (isSynchro)//������ ���������
					{
						rSignals[ik].appear = (float)((double)(appear[ik - startNum] - rBeginTakt) / (double)ticps);
						for (z = 0; z < ftChan; z++)//������ ������ �� ���� �������
						{
							rSignals[ik].s[(z * recLen) + recordedI] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
							rmGrafik[(z * recLen) + recordedI] = (double)rSignals[ik].s[(z * recLen) + recordedI] * sampl2mV;
						}
						/*else ����� ������ ��������� �������*/
						recordedI++;//����������� ������� ���������� �������� ��� ������� �������
						sampRead = 0;//�������� ������� ����������� ��������
						totaltic = curnttic + ((startNum + iNumOfS) - ik) * ticperiod + ticps;//������������ totaltic
					}
					else//����� ���������� ���������� ������������ ��������� ������� (�������� �� ���������� �������)
					{
						for (z = 0; z < chanls; z++)
						{
							rLocMean[z] += (double(convData[z] - rExtractLocMean[rCountlm * chanls + rReadOrder[z]]) / (double)rSamps);
							rExtractLocMean[rCountlm * chanls + rReadOrder[z]] = convData[z];
						}
						rCountlm++;
						if (rCountlm == rSamps)
							rCountlm = 0;
                    }
				}

				//-----------------------------------
				//����� � ������ ���������� ��������
				//������ ���������� �������� ���������������, �����:
				//1) ��� ������� ������� (mk >= allMini)
				//2) ������ ������ ����� ��� ����������� (PStart->pausMini->Tag == 1)
				//3) ��� ������ ���������� ������� (recordedI > 0)
				if (recMini)
				{
					if (recordedM == 0)//���� ��������� ���������� ������
					{
						if ((rPolarity * (convData[0] - (short)rLocMean[0] * rConstMinus)) >= rPorog)
						{
							//������ ��������� (��� �������� ������ ����� ������ ������ ���������� �������, ������ ��. ����-� �����. ���...)
							rSpntSignals[mk].appear = rSignals[ik - 1].appear + (sampRead * discrT * (1e-6));//����� ������������� ������� (� ��������)

							//���������� �� ������ ������ ������
							for (g = bI; g < samplBack; g += chanls)
							{
								for (z = 0; z < ftChan; z++)//���������� ������ �� ���� �������
								{
									rSpntSignals[mk].s[(z * rSpntRecLen) + recordedM] = rPolarity * (backBuffer[g + z] - (short)rLocMean[z] * rConstMinus);
									//rmGrafikM[(z * rSpntRecLen) + recordedM] = (double)rSignals[ik].s[(z * rSpntRecLen) + recordedM] * sampl2mV;
								}
								/*else ����� ������ ��������� �������*/
								recordedM++;//����������� ������� ���������� �������� ��� ������� �������
							}
							for (g = 0; g < bI; g += chanls)//����������� ������ �� ������
							{
								for (z = 0; z < ftChan; z++)//���������� ������ �� ���� �������
								{
									rSpntSignals[mk].s[(z * rSpntRecLen) + recordedM] = rPolarity * (backBuffer[g + z] - (short)rLocMean[z] * rConstMinus);
									//rmGrafikM[(z * rSpntRecLen) + recordedM] = (double)rSignals[ik].s[(z * rSpntRecLen) + recordedM] * sampl2mV;
								}
								/*else ����� ������ ��������� �������*/
								recordedM++;//����������� ������� ���������� �������� ��� ������� �������
							}
						}
						else
						{
							//������������ ��������� ������� (����� ��� ����� ���������)
							for (z = 0; z < chanls; z++)
							{
								rLocMean[z] += (double(convData[z] - rExtractLocMean[rCountlm * chanls + rReadOrder[z]]) / (double)rSamps);
								rExtractLocMean[rCountlm * chanls + rReadOrder[z]] = convData[z];
							}
							rCountlm++;
							if (rCountlm == rSamps)
								rCountlm = 0;
						}
					}
					else//�������� ����������� �������
					{
						for (z = 0; z < ftChan; z++)//���������� ������ �� ���� �������
						{
							rSpntSignals[mk].s[(z * rSpntRecLen) + recordedM] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
							//rmGrafikM[(z * rSpntRecLen) + recordedM] = (double)rSignals[ik].s[(z * rSpntRecLen) + recordedM] * sampl2mV;
						}
						recordedM++;//����������� ������� ���������� �������� ��� ������� �������
						if (recordedM >= rSpntRecLen)//������ ��������� ������� (��������� ������ ���������� �������)
						{
							if (!multiCh)//������� ������� ��� �������������� �������
								for (z = 0; z < chanls - 1; z++)
									rSpntSignals[mk].s[rSpntRecLen + z] = (short)rLocMean[z + 1];//(short)(minorChanMean[z] / rSpntRecLen);//��������� �������

							recordedM = 0;//������ ���������� ������� (�������� ������� ���������� ��������)
							mk++;//���������� ������� ���������� ��������
							kr++;//������� ���������� (���������� ������, ���� �� �����������)
							//==========================================
							//if (rAppEndData)
							//	DiscontinMiniWrite();//��������� ���������� ���������� ������ �� ������ ����
							//==========================================
							PStart->BackTime->Tag = mk;//������� ��������
							//Synchronize(&RecSynchroCounter);//������� ��������
//							Synchronize(&MiniMd4SynchroPlot);//����� �� ������ ������� ���������� ������
						}
					}
				}
				else if ((recordedI > 0) && (recordedM > 0))//�� ����� ������ ����������� ������ ��������� ������
				{
                    //��������������� ���������� ������ ������������� �������
					recordedM = 0;//������ ��������� ������� (�������� ������� ���������� ��������)
					mk++;//������� ���������� ��������
					kr++;//������� ���������� (���������� ������, ���� �� �����������)
					PStart->BackTime->Tag = mk;//������� ��������
				}
			}

			z = 0; errC = 0;//����������� ����������� ���� ������ (�����)
			while ((z == 0) && (errC <= 2))
			{
				z = r_pUtil->FreeBuffer();
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "������ FreeBuffer";
				iNumOfS = ik - startNum;//������� ������ �� �����
				recordedI = 0;//������� ������ �� �����
			}
		}
	}

	z = 1; errC = 0;//���� ����� ������
	while ((z == 1) && (errC <= 2))
	{
		r_pUtil->Stop();
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0
		errC++;
	}
	if (errC > 2)
		Experiment->DevEvents->Text = "������ ADC_Stop";

	//��������� rExtractLocMean ��������� � ����� Execute()
	drvData = NULL;
	delete[] backBuffer; backBuffer = NULL;
	delete[] appear; appear = NULL;
}
//---------------------------------------------------------------------------

void TRecordThread::Spnt_SgnlAcquisition(__int32 mNumOfS, clock_t experimBegin)
{
	//���� ������: ���������� ������� �1
	/*
	mNumOfS - ����� ���������� ��������
	*/

	unsigned int mask;// ������ ���������� � ��������� ������
	__int32 i, g, z, j,//������� � ������
        	k,//������� ��������
			kRefresh,//������� ����������
			samplBack,//������� ����� (�������������� �� spontanBackTime)
			bI,//������ ������� � ������, � �������� ������ ���������� �����
			startNum,//��������� �������� � ������� ��������� ������������ � ���������� � startNum
			recorded,//���������� ���������� ��������
			frstIndBack,//������ ������ ��� ���������
			refreshNum,//������ ���������� �������� (� ��������)
			ost,//������� (������� �������� ������ �� ��������� �����)
			blockReady,//���������� ����� ������
			minorChanMean[maxChannels];//������� �� �������������� ������
	short *drvData,//�������� ���������� ������ ����
		  convData[maxChannels],//�������� ���������� ��� ��� ��� ���� ������� (������� �� ������, ��� maxChannels)
		  *backBuffer;//�������� ����� ���������� ����
	unsigned long int allsampls;//���������� ����� �������� �������
	clock_t timeMoment;//������ ����������� ������� (����������� �� ������ ������ ���������)
	__int64 curnttic;//������� ���� ����������

	drvData = NULL; backBuffer = NULL;
	samplBack = chanls * floor(((StrToInt(PStart->BackTime->Text) * 1000) / effDT) + 0.5);//���������� �������� "�����" ��� ����������� ������� (��� ����� ����� �������)
	refreshNum = mNumOfS + 1;//�������� ������ ���������� ������ ����� ���������� �������� (�.�. ��� ����������)
	if (Experiment->ggRefresh->Checked)
		refreshNum = StrToInt(Experiment->refreshEvery->Text);
	backBuffer = new short[samplBack];//����������� �����; ����� ������ ����� ������� ����� � �������� (��������)
	for (i = 0; i < samplBack; i++)//������ ���������� ������ backBuffer (������)
		backBuffer[i] = 0;
	bI = 0;
	allsampls = 0;//�������� ������� ��������
	recorded = 0;

	QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);//������� ���� ����������
	while (curnttic < rNextTic)
	{
		Sleep(10);//��� ������� ������ ���������� ������ ����� ���������� ��������
		QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);//������� ���� ����������
    }

	z = r_pUtil->Start(&r_a, 0);
	if (z != 1)
	{   Experiment->DevEvents->Text = (FindErrorStrByCode(z,0)); return; }

	z = 0;
	while (z == 0)
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0

	for (i = 0; i < 4; i++)//"�����������" ����
	{
		blockReady = 0;
		while (blockReady == 0)
			blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

		z = 0;//����������� ����������� ���� ������ (�����)
		while (z == 0)
			z = r_pUtil->FreeBuffer();
	}

	kRefresh = 0;//�������� ������� ����������
	startNum = Graphs->CrntSig->Tag;//��������� �������� � ������� ��������� ������������ � ���������� � startNum
	k = startNum;//���������� ��������� ��������

	while (k < (startNum + mNumOfS))
	{
		if (PStart->CloseWin->Tag == 1)//���������� �� ������
			mNumOfS = k - startNum;//������� ������ �� ����� while (k < (startNum + mNumOfS))

		blockReady = 0;
		while(blockReady == 0)
			blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

		for (i = 0; i < rBlockSize; i += chanls)
		{
			for (z = 0; z < chanls; z++)
			{
				convData[z] = (drvData[i + rReadOrder[z]] >> bwSh);//�������� ��� ��� (������� 12 (14) ���)
				backBuffer[bI] = convData[z];//������ � �����
				bI++;//������� �����
			}
			if (bI >= samplBack)
				bI = 0;//������� �� ������ ������������ ������

			if (recorded == 0)//����� �������
			{
				if ((rPolarity * (convData[0] - (short)rLocMean[0] * rConstMinus)) >= rPorog)//������ ���������
				{
					//rSignals[k].appear = (allsampls + i) * discrT * (1e-6);//����� ������������� ������� (� ��������)
					timeMoment = clock();//������ ����������� ������� (����������� �� ������ ������ ���������)
					rSignals[k].appear = ((float)timeMoment / 1000) - (float)experimBegin;

					//���������� �� ������ ������ ������
					for (g = bI; g < samplBack; g += chanls)
					{
						for (z = 0; z < ftChan; z++)//���������� ������ �� ���� �������
						{
							rSignals[k].s[(z * recLen) + recorded] = rPolarity * (backBuffer[g + z] - (short)rLocMean[z] * rConstMinus);
							rmGrafik[(z * recLen) + recorded] = (double)rSignals[k].s[(z * recLen) + recorded] * sampl2mV;
						}
						/*else ����� ������ ��������� �������*/
						recorded++;//���������� ����� ���������� �������� ��� ������� �������
					}
					for (g = 0; g < bI; g += chanls)//����������� ������ �� ������
					{
						for (z = 0; z < ftChan; z++)//���������� ������ �� ���� �������
						{
							rSignals[k].s[(z * recLen) + recorded] = rPolarity * (backBuffer[g + z] - (short)rLocMean[z] * rConstMinus);
							rmGrafik[(z * recLen) + recorded] = (double)rSignals[k].s[(z * recLen) + recorded] * sampl2mV;
						}
						/*else//������������� �����
							for (z = 1; z < chanls; z++)//������� � ������ ����������� �������
								minorChanMean[z - 1] += backBuffer[g + z];//����� ���������� �������*/
						recorded++;//���������� ����� ���������� �������� ��� ������� �������
					}
				}
				else//�������� �������
				{
					for (z = 0; z < chanls; z++)//������������ ��������� �������
					{
						rLocMean[z] += (double(convData[z] - rExtractLocMean[rCountlm * chanls + rReadOrder[z]]) / (double)rSamps);
						rExtractLocMean[rCountlm * chanls + rReadOrder[z]] = convData[z];
					}
					rCountlm++;
					if (rCountlm == rSamps)
						rCountlm = 0;
                }
			}
			else//(recorded > 0)
			{
				//�������� �������
				for (z = 0; z < ftChan; z++)//���������� ������ �� ���� �������
				{
					rSignals[k].s[(z * recLen) + recorded] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
					rmGrafik[(z * recLen) + recorded] = (double)rSignals[k].s[(z * recLen) + recorded] * sampl2mV;
				}
				/*else ����� ������ ��������� �������*/
				recorded++;//���������� ����� ���������� �������� ��� ������� �������

				if (recorded >= recLen)//������ ��������� ������� (��������� ������ ���������� �������)
				{
					if (!multiCh)//������� ������� ��� �������������� �������
						for (z = 0; z < chanls - 1; z++)
							rSignals[k].s[recLen + z] = (short)rLocMean[z + 1];//(short)(minorChanMean[z] / recLen);//��������� �������
					recorded = 0;//������ ������� ��������� (�������� ������� ���������� ��������)
					k++;
					kRefresh++;
					Graphs->CrntSig->Tag = k;//������� ��������
					Synchronize(&RecSynchroCounter);//������� ��������

					//==========================================
					if (rAppEndData)
						DiscontinWrite();//��������� ���������� ������ �� ������ ����
					//==========================================

					Synchronize(&FromMinorChan);//�������������� ������
					if (k == (startNum + mNumOfS))//��� ������� ��������� ��������
						break;//��������� ���� for (i = 0; i < rBlockSize; i += chanls)
				}
			}
		}

		allsampls += rBlockSize;//������� ������������ ��������

		z = 0;//����������� ����������� ���� ������ (�����)
		while (z == 0)
			z = r_pUtil->FreeBuffer();

		if (((kRefresh == refreshNum) || (k == (startNum + mNumOfS))) && (k > 0))//��������� �� �������� �������
		{
			z = 1;
			while (z == 1)
			{
				r_pUtil->Stop();
				r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0
			}

			if (PStart->CloseWin->Tag == 1)//���������� �� ������
				mNumOfS = k - startNum;

			//��������� �������
			recorded = 0;
			specK1 = (k - kRefresh);//����� �������, � �������� ���������� �������������� �������
			specK2 = kRefresh;//���������� �������������� ��������
			Synchronize(&MiniSynchroPlotGraphs);//��������� ��������

			z = r_pUtil->Start(&r_a, 0);
			if (z <= 0)
			{   Experiment->DevEvents->Text = (FindErrorStrByCode(z,0)); return; }
			z = 0;
			while (z == 0)
				r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0

			blockReady = 0;
			while(blockReady == 0)
				blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

			for (i = 0; i < samplBack; i++)
				backBuffer[i] = 0;//���������� ������ backBuffer (������)
			bI = 0;
			kRefresh = 0;//�������� ������� ����������
		}
	}

	//���� ����� ������
	z = 1;
	while (z == 1)
	{
		r_pUtil->Stop();
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, ���� ��� ���� ������, ����� 0
	}

	drvData = NULL;
	delete[] backBuffer; backBuffer = NULL;
}
//---------------------------------------------------------------------------

void TRecordThread::DiscontinWrite()
{
	//��� ���� �������. ����������� ���� ������ (���������� ������� ������ ������� ����� �� ������ ����)

	__int32 i,
			fullSigLen,//������ ����� ������ �� ���� ������� (�������)
			ikNow,//������� ����� �������
			itemWrtRd,//����� ���������� ������ (��� ������� ����� �� ����� �������)
			amountOfBytes;//���������� ����, ������������ ������ ����� (���� + ������)
	char dLett[3];//������-����������� ����� ���������

	dLett[0] = 'S'; dLett[1] = 'i'; dLett[2] = 'g';//������-����������� ����� ���������

	ikNow = Graphs->CrntSig->Tag - 1;//������� ����� �������
	fullSigLen = (recLen * ftChan) + (chanls - ftChan);//������ ����� ������ �� ���� ������� (�������)
	amountOfBytes = (3 * sizeof(char)) + sizeof(float) + (fullSigLen * sizeof(short));//������ ���� �������� ����
	
	//����������� - �������� �������
	itemWrtRd = fwrite(&dLett, sizeof(char), 3, rDiscontinFile);//����������� 'Sig'
	amountOfBytes -= itemWrtRd * sizeof(char);

	//����� �������������
	itemWrtRd = fwrite(&rSignals[ikNow].appear, sizeof(float), 1, rDiscontinFile);//����� �������������
	amountOfBytes -= itemWrtRd * sizeof(float);

	//��� ������ (������ �� ���� �������)
	itemWrtRd = fwrite(rSignals[ikNow].s, sizeof(short), fullSigLen, rDiscontinFile);
	amountOfBytes -= itemWrtRd * sizeof(short);

	if (amountOfBytes != 0)
		Experiment->DevEvents->Text = "������ ������";//��������� �����, ������, ����� �� ������
}
//---------------------------------------------------------------------------

void TRecordThread::GetLocalMean()
{
	//��������� ��������� ������� (��� ������������� ���������� ������������)
	__int32 i, j;

	rSamps = (__int32)floor((float(baseLine) / effDT) + 0.5);
	rExtractLocMean = new short[rSamps * chanls];//������ ��� ���������� ���������� �������� ��������� � Execute()
	Experiment->BeginMean(rExtractLocMean, rSamps * chanls);
	
	for (j = 0; j < chanls; j++)
	{
		rLocMean[j] = 0;//������ ������� �������� ��� ���� �������
		for (i = 0; i < rSamps; i++)
			rLocMean[j] += (double)rExtractLocMean[i * chanls + rReadOrder[j]];
		rLocMean[j] /= (double)rSamps;
	}

	rCountlm = 0;
}
//---------------------------------------------------------------------------

//void TRecordThread::SetToDAC()
//{
//	//������������� ������-���� �� ������ ���
//}
//---------------------------------------------------------------------------

