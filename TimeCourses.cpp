//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <mmsystem.h>
#include <stdio.h>
#include <fstream.h>

#pragma hdrstop

#include "TimeCourses.h"
#include "PredStart.h"
#include "ExperimentNotes.h"
#include "BuildProtocol.h"
#include "Gistograms.h"
#include "CalcParams.h"
#include "RenamOverr.h"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "cspin"
#pragma resource "*.dfm"
TGraphs *Graphs;

const short eheadSBlock = 12,//���������� ������ � ��������� ehead
			dataInHead = 5,//���������� ������, ������ ������� ���� ������ � ���������
			pureData = eheadSBlock - dataInHead;//���������� ������ � ������������������ �������
blockbyte ehead[eheadSBlock];//����� ��������� ������ � �������

trac *gSignals;//��������� ��� �������� � �� �����������

__int32 *devExpParams_int,//��������� ���������� � ������������ (__int32)
		gPreTime,//������ ������. ������ (������������)
		gPostTime,//������ �������. ������ (������������)
		gTimeBgn;//������ ��������� (������������)
short experimentType,//��� ������������ (���������� �������)
	  gPorog;//����� (�������)
float *devExpParams_float;//��������� ���������� � ������������ (float)
bool mask[9],//����� "����������" �������� ��� ������ �����������
	 _isfull;//�������� �� �����-���� ������
	 
//---------------------------------------------------------------------------
__fastcall TGraphs::TGraphs(TComponent* Owner)
	: TForm(Owner)
{
	short i;
	//--------------------------------------------------------------
	//--- ����������� ����� ����. ����� ������ *.pra ---
	i = 0;
	ehead[i].nob = "�������� �����, ������ ���������, ����� �������� � ����� ���������� �������������� �����";
	ehead[i].byteInHead = 0;
	ehead[i].byteInFile = 0;
	ehead[i].szDataType = sizeof(char);//������ ������ � ���������
	ehead[i].szInHDataType = ehead[i].szDataType;
	ehead[i].numOfElements = 64;
	//--------------------------------------------------------------
	i++;//1
	ehead[i].nob = "������ ����� (� ������) � ����������� �� ������������, �������� �������������";//
	ehead[i].szDataType = sizeof(__int32);
	ehead[i].numOfElements = 1;
	//--------------------------------------------------------------
	i++;//2
	ehead[i].nob = "__int32 ��������� ����� ������ (ADCParametersDMA �) + ��� ������������ + ��������� ������������";
	ehead[i].szDataType = sizeof(__int32);
	ehead[i].numOfElements = 15;
	//--------------------------------------------------------------
	i++;//3
	ehead[i].nob = "float ��������� ����� ������ (ADCParametersDMA �) + ��� ������������ + ��������� ������������";
	ehead[i].szDataType = sizeof(float);
	ehead[i].numOfElements = 6;
	//--------------------------------------------------------------
	i++;//4
	ehead[i].nob = "����� ���������� ��������";
	ehead[i].szDataType = sizeof(__int32);
	ehead[i].numOfElements = 1;
	//--------------------------------------------------------------
	//--- ���������� � ���������� ������ ��������� ---
	//��������� byteInFile � numOfElements ��� ��������� ������ �������������� ��� ���������� ������
	i++;//5
	//(1)
	ehead[i].nob = "����� ������� ����� ����� � ���������� �������� � ���������� ���������� (��� ������)";//(1)
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//�������� ������
	//--------------------------------------------------------------
	i++;//6
	ehead[i].nob = "������ (����� ������� ����� ������-������ �����)";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//�������� ������
	//--------------------------------------------------------------
	i++;//7
	ehead[i].nob = "������ (����� ������� ����� ������-������ �����)";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//�������� ������
	//--------------------------------------------------------------
	i++;//8
	ehead[i].nob = "����� ������� ����� ����� � ����������� �� ������������, �������� �������������";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//������
	//--------------------------------------------------------------
	i++;//9
	ehead[i].nob = "����� ������� ����� ����� � �������� ��������, � ������� ��������� �������";//
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(__int32);//������
	//--------------------------------------------------------------
	i++;//10
	ehead[i].nob = "����� ������� ����� ����� � ����������� ����� �� �������";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(bool);//������
	//--------------------------------------------------------------
	i++;//11
	ehead[i].nob = "������ (����� ������� ����� ������-������ �����)";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//�������� ������
	//====================================================================
	//���������� ������� � ������
	for (i = 1; i < eheadSBlock; i++)
		if (i < dataInHead)//������, ������������� ������ � ���������
		{
			ehead[i].szInHDataType = ehead[i].szDataType;
			ehead[i].byteInHead = ehead[i - 1].byteInHead + (ehead[i - 1].numOfElements * ehead[i - 1].szInHDataType);
			ehead[i].byteInFile = ehead[i].byteInHead;
		}
		else//��� ���������
			ehead[i].byteInHead = ehead[i - 1].byteInHead + (1 * ehead[i - 1].szInHDataType);//1 ��������

	//����������� �������� ������� ������� � ����������� ��������
	pValues->ColCount = 5;//10 + maxChannels;//18;//���������� ������� � ������� �� ���������� ����������
	pValues->Cells[0][0] = "��������";
	pValues->Cells[1][0] = "�����, �";
	pValues->Cells[2][0] = "����, ��";//��������� �������� - ����������������� (����� ������� �����)
	pValues->Cells[3][0] = "rise, ���";//����� ���������� (������������)
	pValues->Cells[4][0] = "����, ���";//����� ����� (������������)
	pValues->ColWidths[0] = (pValues->Cells[0][0].Length()) * 7;//������ ��� ������� ������ �����
	pValues->ColWidths[1] = (pValues->Cells[1][0].Length()) * 7;//������ ��� ������� ������ �����

	//��������� ������ ����� ���������� �� ������� (�����)
	allCharts[0] = SignalTrack;//������� (�������) ������
	allCharts[1] = AmplInT;//������� ���������
	allCharts[2] = RiseTInT;//����� ����������
	allCharts[3] = FallTInT;//����� �����
	allCharts[4] = NaAmplInT;//��������� Na ����
	allCharts[5] = NaDurInT;//������������ ���������� ����
	allCharts[6] = KAmplInT;//��������� K ����
	allCharts[7] = SynDelInT;//������������� ��������
	allCharts[8] = SynTrnsInT;//����� �������������� ����������

	AmplInT->Canvas->Font->Name = "Arial";//���������� ����� �������� (������� �� ���� ������������)

	gSignals = NULL;//��������� ��� �������� � �� �����������
	//Graphs->"����� � ���������"->Tag - ����� �������� � ������� Graphs->allCharts
}
//---------------------------------------------------------------------------

bool TGraphs::SaveExpDataToFile(short expType, trac *signals_loc, __int32 numOfSignal, bool saveType)
{
	//���������� � ����� ������� (����� ��������� ����� *.pra)
	/*
	expType - ��� ������������
	signals_loc - ��������� ������ �� ��������� � ���������
	numOfSignal - ���������� ��������� ��������
	saveType - ��� ���������� (true = ���������� ������-��� ����������� ������������)
	*/

	void *refToWrite[eheadSBlock];//������ ������ �� ����� ������������ ������
	unsigned long bytesCounter,//������� ������; ������������ ��� ������� ��������� ehead
				  c,
				  m_nWritten,//����� ���������� ����
				  totWrit;//����� �������� ����
	__int32 i, j, g,
			*sigsWithMark,//������ ��������, � ������� ��������� �����
			fullSigLen,//������ ����� ������ �� ���� ������� (�������)
			dbN;//����� ����� ������ (data block number)
	HANDLE hFile;//����� �����, � ������� ����������� ������
	TDateTime CurrentDateTime;//������� ���� � �����
	AnsiString progDate,//���������� � ��������� � ���� �������� �����
			   userText,//���������� �� ������������ �� �������������
			   versName;//��� ���������, ������
	float frstSignAppear;//����� ������������� ������� ������� (� ����������� ������ ������)
	bool *markerVsblt;//��������� ����� �� �������

	if (expType < 5)
		experimentType = expType;//�������� ��� ������������ � ���������� ����������
	else//if expType == 5 (�.�. ���������� ������� � ������ �4)
		experimentType = 1;

	if (saveType)//���������� ������-��� ����������� ������������
	{
		SigNumUpDwn->Max = numOfSignal;//����� ������� ��������� ������� �������� (���������� ����� ��������������)

		gSignals = signals_loc;//�������� ����� ��� ���������� � ��������
		signals_loc = NULL;//��� �� �������� ��������� ������

		gRF[0] = 0;//������ ������������ �����
		gRF[1] = 1;//��������� ������������ ����� (���������� ����� ����, ������, �����)
		ChannNumb[0]->Checked = true;//� ������ ��������� �������� ������� �����
		
		gInd = new __int32[numOfSignal];//������ ������ ��������
		for (i = 0; i < numOfSignal; i++)
			gInd[i] = i;//��������� ������ ��������

		//���������� ���������
		gPreTime = StrToInt(PStart->PreTime->Text);//���-����� � �������������
		gPostTime = StrToInt(PStart->PostTime->Text);//����-����� � �������������
		gPorog = short((float)StrToInt(PStart->Porog->Text) / sampl2mV);//��������� ���������� (�������)

		//���������� ��������� ������ ��� ���������
		gTimeBgn = 0;
		if (experimentType == 2)
			gTimeBgn = gPostTime;
		else if (experimentType == 3)
			gTimeBgn = gPreTime;
	}

	//--------------------------------------------
	//�������� �����, ������ ���������, ����� �������� � ����� ���������� �������������� �����
	CurrentDateTime = Now();//������� ���� � �����
	versName = "ElphAcqu v";//������ ��������
	versName += progVer;//������ ��������
	if (saveType)//���������� ������-��� ����������� ������������
	{
		progDate = versName;
		progDate += "\r\nmade";
		progDate += CurrentDateTime.DateTimeString().c_str();//19 ��������
		progDate += "\r\nedit";
		progDate += CurrentDateTime.DateTimeString().c_str();//19 ��������
	}
	else //���������� ������������������ �����
	{
		progDate = ExpNotes->pIDateString;//����������� ������ � ����� �������� � ��������� �����

		j = progDate.AnsiPos("\r\nmade");//�������� ����� ������ �����
		if (j > 0)//���� ����� �������
		{
			progDate.Delete(j + 6 + 19, progDate.Length());//������� ���� ���������
			progDate.Delete(1, j - 1);//������� ��������� ����� ������
			progDate.Insert(versName, 1);
		}
		else//���� ����� ��� "made"
		{
			progDate = versName;
			progDate += "\r\nmade";
			progDate += CurrentDateTime.DateTimeString().c_str();//19 ��������
		}

		progDate += "\r\nedit";
		progDate += CurrentDateTime.DateTimeString().c_str();//19 ��������
	}
	//====================================================
	//���������� � ��������� � ���� �������� ������� �����
	g =  ehead[0].numOfElements - progDate.Length();
	if (g < 0)//������������ ����� ������
		progDate.SetLength(ehead[0].numOfElements);
	else if (g > 0)
		progDate += AnsiString::StringOfChar('_', g);
	ExpNotes->pIDateString = progDate;//������������ ���������� � �����

	userText = ExpNotes->usersNotes->Lines->Text;//���������� �� ������������ (������ ������������)
	if ((ExpNotes->addUMark->Tag > 0) && (expType != 5))//� userText ��������� � ������� �� ���� ������������
	{
		userText += "\n\n\r\r\n\n";//�����������
		sigsWithMark = new __int32[ExpNotes->addUMark->Tag];//������ ��������, � ������� ��������� �����
		markerVsblt = new bool[ExpNotes->addUMark->Tag * ftChan];//��������� ����� �� �������
		for (i = 0; i < ExpNotes->addUMark->Tag; i++)
		{
			sigsWithMark[i] = ExpNotes->theMarker->pointOnGraph;
			for (j = 0; j < ftChan; j++)
				markerVsblt[(i * ftChan) + j] = ExpNotes->theMarker->chanN[j];//��������� ����� �� �������
			userText += ExpNotes->theMarker->textMark;//����� �������
			userText += "||\n||";
			ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
		}
	}

	//=====================================
	//��������� ����� ������ � ������
	if (saveType)//���������� ������-��� ����������� ������������
	{
		//__int32 - ���������
		devExpParams_int = new __int32[ehead[2].numOfElements];//������ ������ � ����������� ���������� � ������������ (__int32)
		devExpParams_int[0]	= multiCh;//0//����� ����� ������ (true = ��������������)
		devExpParams_int[1]	= recLen;//1//����� �������� ������� � ��������
		devExpParams_int[2] = (__int32)discrT;//2//����� ������������� (������������)
		devExpParams_int[3]	= chanls;//3//���������� ����������� �������
		devExpParams_int[4]	= Experiment->uiLeadChan->ItemIndex;//4//������� ����� (�������� ������)
		devExpParams_int[5]	= Experiment->a.m_nGain;//5//����������� ��������
		devExpParams_int[6] = ExpNotes->addUMark->Tag * __int32(expType != 5);//6//���������� �������
		devExpParams_int[7] = (__int32)experimentType;//7//��� ������������, ���������� �������
		devExpParams_int[8] = gPreTime;//8//gPreTime (� �������������)
		devExpParams_int[9] = gPostTime;//9//gPostTime (� �������������)
		devExpParams_int[10] = (__int32)gPorog;//10//����������� ����� (�������)
		devExpParams_int[11] = Experiment->maxADCAmp;//11//������������ ��������� (� ��������)
		devExpParams_int[12] = Experiment->minADCAmp;//12//����������� ��������� (� ��������)
		devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//������� ���������� �����
		devExpParams_int[14] = 0;//14//�� ������������

		//float - ���������
		devExpParams_float = new float[ehead[3].numOfElements];//������ ������ � ����������� ���������� � ������������ (float)
		devExpParams_float[0] = sampl2mV;//0//����������� �������� �������� �� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
		devExpParams_float[1] = Experiment->maxVoltage;//1//�������� ������� ���������� (� ������������)
		devExpParams_float[2] = StrToFloat(Experiment->StimPeriod->Text);//2//������ ����������
		devExpParams_float[3] = discrT;//3//����� ������������� (������������)
		devExpParams_float[4] = 0;//4//�� ������������
		devExpParams_float[5] = 0;//5//�� ������������
	}
	else//��������������
	{
		devExpParams_int[6] = ExpNotes->addUMark->Tag * (__int32(expType != 5));//���������� ������� (��������)
		devExpParams_int[7] = (__int32)experimentType;//��� ������������ ����� �������� (��� ��������������)
	}
	fullSigLen = (recLen * ftChan) + (chanls - ftChan);//��������� ����� �������� �� ���� ������� (�������)

	dbN = dataInHead;//5//����� ����������������� ������ (��� ������)
	ehead[dbN].numOfElements = numOfSignal * (sizeof(float) + (fullSigLen * sizeof(short)));//����� ���� �������� �������� �� ���� �������

	dbN++;//6//������ (����� ������-������ ����� ������)
	ehead[dbN].numOfElements = 0;
	dbN++;//7//������ (����� ������-������ ����� ������)
	ehead[dbN].numOfElements = 0;

	dbN++;//8//���������� �������� � ���������������� "������" (userText)
	ehead[dbN].numOfElements = userText.Length();
	dbN++;//9//������ �������� � �������
	ehead[dbN].numOfElements = ExpNotes->addUMark->Tag * (__int32(expType != 5));
	dbN++;//10//��������� ����� �� �������
	ehead[dbN].numOfElements = ftChan * ehead[dbN - 1].numOfElements;

	dbN++;//11//������ (����� ������-������ ����� ������)
	ehead[dbN].numOfElements = 0;

	//��������� byteInFie
	ehead[dataInHead].byteInFile = ehead[eheadSBlock - 1].byteInHead + 1 * ehead[eheadSBlock - 1].szInHDataType;//byteInHead ���������� + 4 �����
	bytesCounter = ehead[dataInHead].byteInFile;
	for (i = dataInHead + 1; i < eheadSBlock; i++)
	{
		c = ehead[i - 1].numOfElements * ehead[i - 1].szDataType;
		bytesCounter += c;
		if (ehead[i].numOfElements > 0)
			ehead[i].byteInFile = bytesCounter;
		else
			ehead[i].byteInFile = 0;//������ ����������
	}

	//������� ������ �� ���������� � ������� (������������ � ��������� �����)
	dbN = 0;//0//������ � �����
	refToWrite[dbN] = progDate.c_str();//������ � �����
	dbN++;//1//���������� �������� � ���������������� "������" (userText)
	refToWrite[dbN] = (__int32*)&ehead[8].numOfElements;//���������� ���� � userText (8 = eheadSBlock - 4)
	//������ �� ������� � ����������� � ������ ������
	dbN++;//2//��� int-���������
	refToWrite[dbN] = (__int32*)devExpParams_int;//int-���������
	dbN++;//3//��� float-���������
	refToWrite[dbN] = (float*)devExpParams_float;//float-���������
	dbN++;//4//���������� ��������
	refToWrite[dbN] = (__int32*)&numOfSignal;//���������� ��������
	//����������������� ������
	dbN++;//5//����� ������� ���� ����� � ������������������ �������
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;

	dbN++;//6//������ (����� ������� ���� ������-������ ����� ������)
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;
	dbN++;//7//������ (����� ������� ���� ������-������ ����� ������)
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;

	dbN++;//8//����� ������� ���� ����� � �������, �������� �������������
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;
	dbN++;//9//����� ������� ���� ����� � �������� ��������, � ������� ����������� �������
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;
	dbN++;//10//����� ������� ���� ����� � ����������� ����� �� �������
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;

	dbN++;//11//������ (����� ������� ���� ������-������ ����� ������)
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;

	//=========================================
	//���� � ����������� *.pra (p - physiological, r - reaction, a - acquisition)
	hFile = CreateFile(SaveDlg->FileName.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);//������-��������� ���� ��� ������
	//=============================

	m_nWritten = 0;//����� ���������� ����
	totWrit = 0;//����� �������� ����

	//���������� ��������� �����
	for (i = 0; i < eheadSBlock; i++)
	{
		if (refToWrite[i])
		{
			if ((unsigned long)ehead[i].byteInHead != totWrit)
			{
                Experiment->DevEvents->Text = "������ ����������";
				return false;
			}

			if (i < dataInHead)//������, ������������� ������ � ���������
				WriteFile(hFile, refToWrite[i], (ehead[i].szInHDataType * ehead[i].numOfElements), &m_nWritten, 0);
			else//��������� �� ��������� ������ � ���� �����
				WriteFile(hFile, refToWrite[i], (ehead[i].szInHDataType * 1), &m_nWritten, 0);

			totWrit += m_nWritten;
			refToWrite[i] = NULL;//�� ������ ������ �������� ������
		}
	}
	//���������� �������
	dbN = dataInHead;
	frstSignAppear = gSignals[gInd[0]].appear;//����� ������������� ������� ������� (� ������ ������)
	for (i = 0; i < numOfSignal; i++)
	{
		//�������� �������� �� ���� �������
		gSignals[gInd[i]].appear -= frstSignAppear;//������� ������� ������������� �� ����� ������� �������
		WriteFile(hFile, (void*)&gSignals[gInd[i]].appear, sizeof(float), &m_nWritten, 0);
		totWrit += m_nWritten;
		WriteFile(hFile, (void*)&gSignals[gInd[i]].s[0], sizeof(short) * fullSigLen, &m_nWritten, 0);
		totWrit += m_nWritten;
	}
	dbN++;//6//������
	//WriteFile(hFile, ������, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0); totWrit += m_nWritten;
	dbN++;//7//������
	//WriteFile(hFile, ������, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0); totWrit += m_nWritten;

	//���������� ��������� ����������, �������� �������������
	dbN++;//8//���������������� ������
	if (ehead[dbN].numOfElements > 0)//!userText.IsEmpty()//������ �� ������
	{
		WriteFile(hFile, (void*)userText.c_str(), (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0);
		totWrit += m_nWritten;
	}

	if ((ExpNotes->addUMark->Tag > 0) && (expType != 5))
	{
		dbN++;//9//������ �������� � ���������
		WriteFile(hFile, (void*)&sigsWithMark[0], (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0);
		totWrit += m_nWritten;

		dbN++;//10//��������� ����� �� �������
		WriteFile(hFile, (void*)&markerVsblt[0], (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0);
		totWrit += m_nWritten;

		delete[] sigsWithMark; sigsWithMark = NULL;//������ ��������, ������� ��������� �����
		delete[] markerVsblt; markerVsblt = NULL;//��������� ����� �� �������
	}
	else
		dbN += 2;//����������� ���������� �������

	dbN++;//11//������
	//WriteFile(hFile, ������, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0); totWrit += m_nWritten;

	CloseHandle(hFile); hFile = NULL;//��������� ����

	//���������� ����� ���������� �����, � ����� �������� ������ (� ������)
	m_nWritten = 0;
	for (g = 0; g < eheadSBlock; g++)
	{
		if (g > (dataInHead - 1))//������ ���� � ��������� � � ���� �����
			m_nWritten += ehead[g].szInHDataType * 1;
		m_nWritten += ehead[g].szDataType * ehead[g].numOfElements;
	}

	if (m_nWritten != totWrit)//���� �� �������, ������ ���-�� ��������
	{
		Experiment->DevEvents->Text = "������ ����������";
		return false;
	}

	//���������� ��������� �������� ��� �����������
	//Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
	//		    __int32 *inds, short expT, bool newNull, short chn, sPr *avrP)
	Calculate(gSignals, numOfSignal, gPorog, gPreTime, gPostTime, gInd, experimentType, false, NULL);
	Graphs->Caption = "������� - " + SaveDlg->FileName;//��������� �������� ���� ��������� �����

	return true;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ReadDataFile(TObject *Sender)
{
	//������ ����� � ������� (���� ���� *.pra)
	bool forwc,//��������� ���������� ������ ����� ������ �� �����
		 *markerVsblt,//��������� ����� �� �������
		 maxMultiChann,//���������������� (������ ��� ������ �������)
		 mOnChn[maxChannels];//�������� ����� � �������
	unsigned long bytesRead,
				  totRead,//����� ��������� ����
				  *beginBytes;//������ � �������� ��������� ������ ������ � ������������������ �������
	__int32 i, j, k, z,
			fullSigLen,//��������� ����� �������� �� ���� �������
			dbN,//����� ����� ������
			*sigsWithMark,//������ ��������, � ������� ��������� �����
			lettersNum,//���������� �������� � ������ ������������
			maxToRec,//������������ ����� ������� (� ��������)
			maxChanNumb,//������������ ���������� �������
			numOfSignal;//����� ��������
	AnsiString allUsersTxt,//����������, �������� �������������
			   usersComent,//����� ����������
			   usersMarks,//������� �� ���� ������������
			   singlMark;//�������� �������
	char *unote,//����������, �������� �������������
		 *pidate;//���������� � ��������� � ���� (Programm Information and Date)
	short expType;//��� ������������
	HANDLE hFile;//����� �����, �� �������� ������ ������

	totRead = 0;//����� ��������� ����
	beginBytes = NULL; sigsWithMark = NULL; markerVsblt = NULL; unote = NULL; pidate = NULL;

	//��������� � ������ ���� � ������������������ �������
	hFile = CreateFile(OpenDlg->FileName.c_str(), GENERIC_READ, FILE_SHARE_READ,
				   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	pidate = new char[ehead[0].numOfElements];//������� ������� ��� ������ ������ �� �����

	//������ ����� �0 (����� ������)
	dbN = 0;
	forwc = ReadFile(hFile, (void*)pidate, (ehead[dbN].numOfElements * ehead[dbN].szDataType), &bytesRead, 0);
	totRead += bytesRead;
	if (!forwc)
	{	Experiment->DevEvents->Text = "������ ������"; return; }

	//��������� ������ ������� �����
	singlMark = pidate;
	j = singlMark.AnsiPos("ElphAcqu v3.9");//�������������� ������
	j += singlMark.AnsiPos("ElphAcqu v4.");//�������������� ������
	j += singlMark.AnsiPos("ElphAcqu v5.");//�������������� ������
	if (j <= 0)//��� ����� ������ � �����
	{
		delete[] pidate; pidate = NULL;
		CloseHandle(hFile);//��������� ���� � ������� ��� ������� ��������� ���������� new
		Experiment->DevEvents->Text = "������ �� ��������������";//ReadOldFormatFile(this);//������ ��� pra-���� ������� �������
		return;
    }

	ExpNotes->PIDates->Lines->SetText(pidate);//��������� ����� � ���� ���� TMemo

	//������� ������� ��� ������ ������ �� �����
	beginBytes = new unsigned long[pureData];//������ pureData ���������� ������� (���� �� ������)
	devExpParams_int = new __int32[ehead[2].numOfElements];//������ ������ � ����������� ���������� � ������������ (__int32)
	devExpParams_float = new float[ehead[3].numOfElements];//������ ������ � ����������� ���������� � ������������ (float)

	dbN++;//1//������ ����� �1
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "������ ������"; return; }

	lettersNum = 0;
	forwc = ReadFile(hFile, (void*)&lettersNum, (ehead[dbN].numOfElements * ehead[dbN].szDataType), &bytesRead, 0);
	totRead += bytesRead;

	dbN++;//2//������ ����� �2
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "������ ������"; return; }

	forwc = ReadFile(hFile, (void*)devExpParams_int, (ehead[dbN].numOfElements * ehead[dbN].szDataType), &bytesRead, 0);
	totRead += bytesRead;

	dbN++;//3//������ ����� �3
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "������ ������"; return; }

	forwc = ReadFile(hFile, (void*)devExpParams_float, (ehead[dbN].numOfElements * ehead[dbN].szDataType), &bytesRead, 0);
	totRead += bytesRead;

	dbN++;//4//������ ����� �4 (���������� ��������)
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "������ ������"; return; }

	numOfSignal = 0;
	forwc = ReadFile(hFile, (void*)&numOfSignal, 1 * ehead[dbN].szDataType, &bytesRead, 0);
	totRead += bytesRead;

	SigNumUpDwn->Tag = numOfSignal;//������ �������� ����� �������� (����� ��� �������� ������)
	SigNumUpDwn->Max = numOfSignal;//����� ������� ��������� ������� �������� (���������� ����� ��������������)

	/* ������������������ ������ � �������� devExpParams_int � devExpParams_float
	//__int32 - ���������
	devExpParams_int[0]	= multiCh;//0//����� ����� ������ (true = ��������������)
	devExpParams_int[1]	= recLen;//1//����� �������� ������� � ��������
	devExpParams_int[2] = (__int32)discrT;//2//����� ������������� (������������)
	devExpParams_int[3]	= chanls;//3//���������� ����������� �������
	devExpParams_int[4]	= Experiment->uiLeadChan->ItemIndex;//4//������� ����� (�������� ������)
	devExpParams_int[5]	= Experiment->adcGain;//5//����������� ��������
	devExpParams_int[6] = ExpNotes->addUMark->Tag * (__int32(expType != 5));//6//���������� �������
	devExpParams_int[7] = experimentType;//7//��� ������������, ���������� �������
	devExpParams_int[8] = gPreTime;//8//gPreTime (� �������������)
	devExpParams_int[9] = gPostTime;//9//gPostTime (� �������������)
	devExpParams_int[10] = (__int32)gPorog;//10//����������� ����� (�������)
	devExpParams_int[11] = Experiment->maxADCAmp;//11//������������ ��������� (� ��������)
	devExpParams_int[12] = Experiment->minADCAmp;//12//����������� ��������� (� ��������)
	devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//������� ���������� �����
	devExpParams_int[14] = 0;//14//�� ������������

	//float - ���������
	devExpParams_float[0] = sampl2mV;//0//����������� �������� �������� �� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
	devExpParams_float[1] = Experiment->maxVoltage;//1//�������� ������� ���������� (� ������������)
	devExpParams_float[2] = StrToFloat(Experiment->stimPeriod->Text);//2//������ ����������
	devExpParams_float[3] = discrT;//3//����� ������������� (������������)
	devExpParams_float[4] = 0;//4//�� ������������
	devExpParams_float[5] =	0;//5//�� ������������*/

	//�������� ��������
	multiCh = (bool)devExpParams_int[0];//����� ����� ������ (true = ��������������)
	recLen = devExpParams_int[1];//����� �������� ������� � ��������
	discrT = devExpParams_float[3];//����� ������������� (������������)
	if (discrT < 1e-6)//����� ���� discrT == 0
		discrT = devExpParams_int[2];//����� ������������� (������������)
	chanls = devExpParams_int[3];//���������� ����������� �������
	ftChan = 1 + (chanls - 1) * (__int32)multiCh;//���������� ������� � ������� ����������
	effDT = discrT * (float)chanls;//����������� ����� ������������� = discrT * chanls
	fullSigLen = (recLen * ftChan) + (chanls - ftChan);//��������� ����� �������� �� ���� ������� (�������)
	sampl2mV = devExpParams_float[0];//����������� �������� �������� �� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
	if (fabs(sampl2mV - (devExpParams_float[1] / (devExpParams_int[11] * devExpParams_int[5]))) > 1e-4)//�� ��������
	{
		sampl2mV = devExpParams_float[1] / float(devExpParams_int[11] * devExpParams_int[5]);//����������� ��������
		Experiment->DevEvents->Text = "�����������";
	}
	//������ ������ ���������
	experimentType = (short)devExpParams_int[7];//�������� ��� ������������ � ���������� ����������
	gPreTime = devExpParams_int[8];//���-����� (������������)
	gPostTime = devExpParams_int[9];//����-����� (������������)
	gPorog = (short)devExpParams_int[10];//����������� ����� (�������)
	//���������� ��������� ������ ��� ���������
	gTimeBgn = 0;
	if (experimentType == 2)
		gTimeBgn = gPostTime;
	else if (experimentType == 3)
		gTimeBgn = gPreTime;

	//������ ������� ������ � ������� ����������������� ������
	//��� pureData ���� ��������� ����� ������ unsigned long
	dbN = dataInHead;//5
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "������ ������"; return; }

	forwc = ReadFile(hFile, (void*)beginBytes, pureData * ehead[dbN].szInHDataType, &bytesRead, 0);
	totRead += bytesRead;

	gSignals = CreatStructSignal(numOfSignal, recLen);//������ ��������� � ����������� ��������
	gInd = new __int32[numOfSignal];//������ ������ � �������� ��������� ��������

	//��������� ������� � ���������
	dbN = dataInHead;
	if (beginBytes[dbN - dataInHead] != totRead)
	{	Experiment->DevEvents->Text = "������ ������"; return; }

	for (i = 0; i < numOfSignal; i++)
	{
		gInd[i] = i;//��������� ������ � �������� ��������� ��������
		ReadFile(hFile, (void*)&gSignals[i].appear, sizeof(float), &bytesRead, 0);
		totRead += bytesRead;
		ReadFile(hFile, gSignals[i].s, sizeof(short) * fullSigLen, &bytesRead, 0);
		totRead += bytesRead;
	}
	//������ (���� ������ ������)
	dbN++;//6//
	//ReadFile(hFile, (void*)unote, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &bytesRead, 0); totRead += bytesRead;
	dbN++;//7//
	//ReadFile(hFile, (void*)unote, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &bytesRead, 0); totRead += bytesRead;

	dbN++;//8//������ ��������� ����������, �������� �������������
	if ((lettersNum > 0) && (beginBytes[dbN - dataInHead] > 0))
	{
		//������ ����: dbN = 8//dbN = dataInHead + 3;//8
		if (beginBytes[dbN - dataInHead] != totRead)
		{	Experiment->DevEvents->Text = "������ ������"; return; }

		unote = new char[lettersNum];//� lettersNum �������� ���������� ���� � ���������������� ������

		ReadFile(hFile, (void*)unote, (ehead[dbN].szDataType * lettersNum), &bytesRead, 0);
		totRead += bytesRead;

		allUsersTxt = "";
		for (i = 0; i < (__int32)bytesRead; i++)//bytesRead = lettersNum
			allUsersTxt += unote[i];

		//���������� ��������� ����������� ("\n\n\r\r\n\n")
		//�� ����� ������� ��� ����������, � ����� ���� - ������� �� ���� ������������
		j = allUsersTxt.AnsiPos("\n\n\r\r\n\n");
		usersComent = allUsersTxt;
		usersComent.Delete(j, usersComent.Length());//���������� � ������������
		ExpNotes->usersNotes->Lines->Text = usersComent;//��������� ����� � ���� TMemo

		if (!usersComent.IsEmpty())//������� �� �����
			usersInfoClick(this);//������� �������

		delete[] unote; unote = NULL;
	}

	dbN++;//9//������ ��������, � ������� ���������  ����� //������ ����: dbN = 9//dbN = dataInHead + 4;//9
	if ((devExpParams_int[6] > 0) && (beginBytes[dbN - dataInHead] > 0))
	{
		if (beginBytes[dbN - dataInHead] != totRead)
		{	Experiment->DevEvents->Text = "������ ������"; return; }

		sigsWithMark = new __int32[devExpParams_int[6]];//������ � �������� ��������, � ������� ��������� �������
		ReadFile(hFile, (void*)sigsWithMark, (ehead[dbN].szDataType * devExpParams_int[6]), &bytesRead, 0);
		totRead += bytesRead;

		dbN++;//10//��������� ����� �� �������
		if (beginBytes[dbN - dataInHead] > 0)
		{
            if (beginBytes[dbN - dataInHead] != totRead)
			{	Experiment->DevEvents->Text = "������ ������"; return; }
			
			markerVsblt = new bool[devExpParams_int[6] * ftChan];//��������� ����� �� �������
			ReadFile(hFile, (void*)markerVsblt, (ehead[dbN].szDataType * devExpParams_int[6] * ftChan), &bytesRead, 0); totRead += bytesRead;
			totRead += bytesRead;
		}

		//���������� �����, ���� ������� �������
		usersMarks = allUsersTxt;
		usersMarks.Delete(1, j + 5);//�������� ��, ����� ����� (j - ��������� �������������� ������)
		j = 1;//����������� ��������
		z = 0;//������� �����
		for (i = 0; ((i < devExpParams_int[6]) && (j > 0)); i++)//���� �� ���������� ���������������� �����
		{
			j = usersMarks.AnsiPos("||\n||");//���� ����������� ����� ���������
			if (j > 0)
			{
				singlMark = usersMarks;//�������� ���������� ����� ������
				singlMark.Delete(j, singlMark.Length() - (j - 2));//�������� ������������ �����

				if (markerVsblt)
					for (k = 0; k < ftChan; k++)//����������� ����� � ������� ������
						mOnChn[k] = markerVsblt[(ftChan * z) + k];//����������� ����� � ������� ������
				else
					for (k = 0; k < ftChan; k++)//����������� ����� �� ���� �������
						mOnChn[k] = true;//����������� ����� �� ���� �������

				ExpNotes->AddMarker(singlMark, sigsWithMark[i], &mOnChn[0]);//��������� �����
				usersMarks.Delete(1, j + 4);//������� �� ����������� ������ ������������ �����
				z++;//����������� ������� �����
			}
		}

		delete[] sigsWithMark; sigsWithMark = NULL;
		delete[] markerVsblt; markerVsblt = NULL;
	}
	else
		dbN += 1;//����������� ���������� �������

	dbN++;//11//
	//ReadFile(hFile, (void*)unote, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &bytesRead, 0); totRead += bytesRead;

	//��������� ���� � ������� ��� ������� ��������� ���������� new
	CloseHandle(hFile);

	delete[] beginBytes; beginBytes = NULL;
	delete[] pidate; pidate = NULL;

	gRF[0] = 0;//������ ������������ ������
	gRF[1] = ftChan;//��������� ������������ ������ (���������� ��� ������ ftChan)

	Calculate(gSignals, numOfSignal, gPorog, gPreTime, gPostTime, gInd, experimentType, false, NULL);//���������� ��������� �������� ��� �����������
	GraphsTabsShow();//��������� ���� � ���������
	Graphs->Caption = "������� - " + OpenDlg->FileName;//��������� �������� ���� ��������� �����

	if (ftChan > 1)
	{
		SAllChan->Checked = false;//��� �����������
		SAllChanClick(this);//������� ��� ������
	}
	else
		ChannNumb[0]->Click();//������� ������ �����

	//FillParamTable();//��������� ������� ���������� ����������
	//ResetVisibility();//��������� ���������� ��������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ReadLineData(TObject *Sender)
{
	//������ ����� � ������� (���� ���� *.drr)
	FILE *discontinFile;
	unsigned long totRead;//����� ��������� ����
	__int32 i, j, z,
			k,//������� ��������
			*locRecLen,//����� �������� ������� � ��������
			locChanls,//���������� ����������� �������
			*locAdcGain,//����������� ��������
			maxG,//������������ ����������� ��������
			fullSigLen,//������ ����� ������ �� ���� ������� (�������)
			nUserMrk,//���������� ���������������� �����
			nBlocks,//���������� ������ (������� ������)
			firstPInBlk,//����� ������ ����� � ������� �����
			comMrkNum,//�������� ������� �� ���� ������������
			itemRd,//���������� ����������� ������ (�� ����)
			*sigsWithMark,//������ � �������� ��������, � ������� ��������� �������
			*brdSig,//������ �������� ��������� ����� �������
			lettersNum,//���������� �������� � ������ ������������
			locMrkNum,//��������� ���������� �������
			numOfSignal;//����� ��������
	AnsiString allUsersTxt,//����������, �������� �������������
			   usersComent,//����� ����������
			   usersMarks,//������� �� ���� ������������
			   singlMark;//�������� �������
	float locDiscrT,//����� �������������
		  timeSepar;//����������� �� �������
	TDateTime CurrentDateTime;//��� ������� ���� � �����
	char lett[3],//������������� ��������� ��� �������
		 *pidate,//���������� � ��������� � ���� (Programm Information and Date)
		 *unote;//����������, �������� �������������
	short expType;//��� ������������
	bool boolBuf,//������ ����������� ��������� ��������
		 locMultiCh,//����� ����� ������ (true = ��������������)
		 mOnChn[maxChannels];//�������� ����� � �������
		 
	maxG = 0;//������������ ����������� ��������
	boolBuf = true;//������ ����������� ��������� ��������
	totRead = 0;//����� ��������� ����

	//������� ������� ��� ������ ������ �� �����
	pidate = new char[ehead[0].numOfElements];
	devExpParams_int = new __int32[ehead[2].numOfElements];//������ ������ � ����������� ���������� � ������������ (__int32)
	devExpParams_float = new float[ehead[3].numOfElements];//������ ������ � ����������� ���������� � ������������ (float)

	discontinFile = fopen(OpenDlg->FileName.c_str(), "rb");//��������� ��� ������ ���� � ������������������ �������
	if (discontinFile == NULL)
	{   Experiment->DevEvents->Text = "������ �������� �����"; return; }

	numOfSignal = 0;//������� ����� ��������
	nUserMrk = 0;//����� ���������� ��������
	nBlocks = 1;//������� ������ (������� ������)

	//������� ���������� ���������� ������� � ������
	//������ ������ � ����� �������� � ��������� �����
	itemRd = fread((void*)pidate, ehead[0].szDataType, ehead[0].numOfElements, discontinFile);//
	ExpNotes->PIDates->Lines->Text = pidate;

    //��� �� ������ ����� � �������� ������ ��������������
	itemRd = fseek(discontinFile, 0, SEEK_SET);//������� �� ������ �����
	itemRd = fread(&lett, 1, 3, discontinFile);//
	while (itemRd > 0)
	{
		lett[0] = lett[1]; lett[1] = lett[2];
		itemRd = fread(&lett[2], 1, 1, discontinFile);//�������������

		if ((lett[0] == 'P') && (lett[1] == 'r') && (lett[2] == 'M') && boolBuf)//����� ����� � ����������� (������ ����������� ��������� ��������)
		{
			/* ������������������ ������ ������������ ����������
			multiCh -------(1) - ����� ����� ������ (true = ��������������)
			recLen --------(2) - ����� �������� ������� (�������)
			discrT --------(3) - ����� �������������
			chanls --------(4) - ���������� ����������� �������
			E->adcGain ----(5) - ����������� ��������
			experimentType (6) - ��� ������������, ���������� �������
			E->maxVoltage -(7) - �������� ����������
			E->maxADCAmp --(8) - ������������ ��������� (�������) */
			itemRd = fread(&multiCh, sizeof(bool), 1, discontinFile);//1//����� ����� ������ (true = ��������������)
			itemRd = fread(&recLen, sizeof(__int32), 1, discontinFile);//2//����� ������� (�������)
			itemRd = fread(&discrT, sizeof(float), 1, discontinFile);//3//����� �������������
			itemRd = fread(&chanls, sizeof(__int32), 1, discontinFile);//4//���������� ������������ �������
			itemRd = fread(&devExpParams_int[5], sizeof(__int32), 1, discontinFile);//5//����������� ��������
			itemRd = fread(&expType, sizeof(short), 1, discontinFile);//6//��� ������������
			devExpParams_int[7] = expType;//��� ������������
			itemRd = fread(&devExpParams_float[1], sizeof(float), 1, discontinFile);//7//�������� ����������
			itemRd = fread(&devExpParams_int[11], sizeof(__int32), 1, discontinFile);//8//������������ ��������� (�������)

			boolBuf = false;//������ ����������� ��������� ��������
			sampl2mV = devExpParams_float[1] / float(devExpParams_int[11] * devExpParams_int[5]);//������������ ��������� ��������
			devExpParams_float[0] = sampl2mV;//0//����������� �������� �������� �� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
			ftChan = 1 + (chanls - 1) * (__int32)multiCh;//���������� ������� � ������� ����������
			effDT = discrT * (float)chanls;//����������� ����� ������������� = discrT * chanls
			fullSigLen = (recLen * ftChan) + (ftChan - chanls);//������ ����� ������ �� ���� ������� (�������)
		}
		else if ((lett[0] == 'M') && (lett[1] == 'r') && (lett[2] == 'k'))//������� � ��������� �� ���� ������������
		{
			//������ ������� �� ���� ������������, ���� ����� ����
			/* ������������������ ������ � ����� � ���������� ������
				lettersNum -(1) - ����� ��������� ������
				comUserMakrs(2) - ���������� ������� �� ���� ������������
				userText ---(3) - ����� �� ������������ � ������� �� ����
				sigsWithMark(4) - ������ � �������� ����� � ���������
			*/
			itemRd = fread(&z, sizeof(__int32), 1, discontinFile);//1//����� ��������� ������
			itemRd = fread(&z, sizeof(__int32), 1, discontinFile);//2//���������� �������
			nUserMrk += z;//������� ���������������� ������� �� ���� ������������
        }
		else if ((lett[0] == 'S') && (lett[1] == 'i') && (lett[2] == 'g'))//��� ������
			numOfSignal++;//������� ����� ��������
		else if ((lett[0] == 'E') && (lett[1] == 'D') && (lett[2] == ':'))//����������� ����� �������� ����� ������
			nBlocks++;//������� ���������� ������
	}

	//���������� ������ ���������� ���������� (��������������)
	gPreTime = 0; gPostTime = 0; gPorog = 0;
	gTimeBgn = 0;
	//����� �4 ����� ������������ ������ � ����������� (pra) ������

	/* ������������������ ������ � �������� devExpParams_int � devExpParams_float
	//__int32 - ���������
	devExpParams_int[0]	= multiCh;//0//����� ����� ������ (true = ��������������)
	devExpParams_int[1]	= recLen;//1//����� �������� ������� � ��������
	devExpParams_int[2] = (__int32)discrT;//2//����� ������������� (������������)
	devExpParams_int[3]	= chanls;//3//���������� ����������� �������
	devExpParams_int[4]	= Experiment->uiLeadChan->ItemIndex;//4//������� ����� (�������� ������)
	devExpParams_int[5]	= Experiment->adcGain;//5//����������� ��������
	devExpParams_int[6] = ExpNotes->addUMark->Tag * (__int32(expType != 5));//6//���������� �������
	devExpParams_int[7] = experimentType;//7//��� ������������, ���������� �������
	devExpParams_int[8] = gPreTime;//8//gPreTime (� �������������)
	devExpParams_int[9] = gPostTime;//9//gPostTime (� �������������)
	devExpParams_int[10] = (__int32)gPorog;//10//����������� ����� (�������)
	devExpParams_int[11] = Experiment->maxADCAmp;//11//������������ ��������� (� ��������)
	devExpParams_int[12] = Experiment->minADCAmp;//12//����������� ��������� (� ��������)
	devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//������� ���������� �����
	devExpParams_int[14] = 0;//14//�� ������������

	//float - ���������
	devExpParams_float[0] = sampl2mV;//0//����������� �������� �������� �� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
	devExpParams_float[1] = Experiment->maxVoltage;//1//�������� ������� ���������� (� ������������)
	devExpParams_float[2] = StrToFloat(Experiment->stimPeriod->Text);//2//������ ����������
	devExpParams_float[3] = discrT;//3//����� ������������� (������������)
	devExpParams_float[4] = 0;//4//�� ������������
	devExpParams_float[5] =	0;//5//�� ������������*/

	//��������� ��������� � ������� devExpParams_int � devExpParams_float
	//__int32 - ���������
	devExpParams_int[4]	= 0;//4//������� ����� (�������� ������)
	devExpParams_int[6] = nUserMrk + nBlocks;//6//���������� ���������������� ����� (+ ���������� ������ (�������) ������)
	devExpParams_int[8] = gPreTime;//8//gPreTime (� �������������)
	devExpParams_int[9] = gPostTime;//9//gPostTime (� �������������)
	devExpParams_int[10] = (__int32)gPorog;//10//����������� ����� (�������)
	devExpParams_int[12] = -devExpParams_int[11];//12//����������� ��������� (� ��������)
	devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//������� ���������� �����
	devExpParams_int[14] = 0;//14//�� ������������
	//float - ���������
	devExpParams_float[4] = 0;//4//�� ������������
	devExpParams_float[5] =	0;//5//�� ������������

	SigNumUpDwn->Tag = numOfSignal;//������ �������� ����� �������� (����� ��� �������� ������)
	SigNumUpDwn->Max = numOfSignal;//����� ������� ��������� ������� �������� (���������� ����� ��������������)

	gSignals = CreatStructSignal(numOfSignal, recLen);//������ ��������� ��� �������� ������

	//��� �� ������ ����� � �������� ������ ��������������
	itemRd = fseek(discontinFile, 0, SEEK_SET);//������� �� ������ �����

	locRecLen = new __int32[nBlocks];//����� �������� ������� � ��������
	locAdcGain = new __int32[nBlocks];//����������� ��������
	brdSig = new __int32[nBlocks + 1];//������ �������� ��������� ����� �������
	k = 0;//�������� ������� ����������� ��������
	firstPInBlk = -1;//����
	comMrkNum = 0;//�������� ������� �� ���� ������������
	nBlocks = 1;//����� ��������� ������� ������ (������� ������)
	//newBlock = false;//������ ����� ����
	timeSepar = 0;//������������� ������� �������������
	gInd = new __int32[numOfSignal];//����� �� ������ ���������� ������ ��������
	itemRd = fread(&lett, 1, 3, discontinFile);//
	while (itemRd > 0)
	{
		lett[0] = lett[1]; lett[1] = lett[2];
		itemRd = fread(&lett[2], 1, 1, discontinFile);//�������������

		if ((lett[0] == 'P') && (lett[1] == 'r') && (lett[2] == 'M'))//����� ����� � �����������
		{
			/* ������������������ ������ ������������ ����������
			multiCh -------(1) - ����� ����� ������ (true = ��������������)
			recLen --------(2) - ����� �������� ������� (�������)
			discrT --------(3) - ����� �������������
			chanls --------(4) - ���������� ����������� �������
			E->adcGain ----(5) - ����������� ��������
			experimentType (6) - ��� ������������, ���������� �������
			E->maxVoltage -(7) - �������� ����������
			E->maxADCAmp --(8) - ������������ ��������� (�������) */
			itemRd = fread(&locMultiCh, sizeof(bool), 1, discontinFile);//����� ����� ������ (true = ��������������)
			itemRd = fread(&locRecLen[nBlocks - 1], sizeof(__int32), 1, discontinFile);//����� �������� ������� � ��������
			itemRd = fread(&locDiscrT, sizeof(float), 1, discontinFile);//����� �������������
			itemRd = fread(&locChanls, sizeof(__int32), 1, discontinFile);//���������� ����������� �������
			itemRd = fread(&locAdcGain[nBlocks - 1], sizeof(__int32), 1, discontinFile);//����������� ��������
			itemRd = fread(&experimentType, sizeof(short), 1, discontinFile);//��� ������������

			if ((multiCh != locMultiCh) || (recLen != locRecLen[nBlocks - 1]) ||	(discrT != locDiscrT) ||
				(chanls != locChanls) || (devExpParams_int[5] != locAdcGain[nBlocks - 1]))//���������� ����� ���������
			{
				/*?��������� ���������� ���� ������ ��� ��������� ���� � ��������� ���?*/

				multiCh = locMultiCh;//����� ����� ������ (true = ��������������)
				recLen = locRecLen[nBlocks - 1];//����� �������� ������� � ��������
				discrT = locDiscrT;//����� �������������
				chanls = locChanls;//���������� ����������� �������
				devExpParams_int[0]	= (__int32)multiCh;//0//���������������� (������ ��� ������ �����)
				devExpParams_int[1]	= recLen;//1//����� �������� ������� � ��������
				devExpParams_int[2]	= (__int32)discrT;//2//����� ������������� (������������)
				devExpParams_int[3]	= chanls;//3//���������� ����������� �������
				devExpParams_int[5] = locAdcGain[nBlocks - 1];//����������� ��������
				devExpParams_int[6] = nUserMrk + nBlocks;//6//���������� ���������������� ����� (+ ���������� ������ (�������) ������)
				devExpParams_int[7] = experimentType;//7//��� ������������, ���������� �������
				
				sampl2mV = devExpParams_float[1] / float(devExpParams_int[11] * devExpParams_int[5]);//������������ ��������� ��������
				devExpParams_float[0] = sampl2mV;//0//����������� �������� �������� �� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
				devExpParams_float[3] = discrT;//3//����� ������������� (������������)
				ftChan = 1 + (locChanls - 1) * (__int32)locMultiCh;//���������� ������� � ������� ����������
                effDT = discrT * (float)chanls;//����������� ����� ������������� = discrT * chanls
				fullSigLen = (locRecLen[nBlocks - 1] * ftChan) + (ftChan - locChanls);//������ ����� ������ �� ���� ������� (�������)
			}

			firstPInBlk = k;//����� ������ ����� � ������� �����
			brdSig[nBlocks - 1] = k;//����� ������� ���������� ����� �������
			//newBlock = true;//������ ����� ����

			if (locAdcGain[nBlocks - 1] > maxG)
				maxG = locAdcGain[nBlocks - 1];//������������ ����������� ��������
		}
		else if (((lett[0] == 'S') && (lett[1] == 'i') && (lett[2] == 'g')) && (k < numOfSignal))//��� ������
		{
			itemRd = fread(&gSignals[k].appear, sizeof(float), 1, discontinFile);//����� ������������� �������
			totRead += itemRd * sizeof(float);
			gSignals[k].appear += timeSepar;//���������� ������ ������������� ����������

			itemRd = fread(gSignals[k].s, sizeof(short), fullSigLen, discontinFile);
			totRead += itemRd * fullSigLen * sizeof(short);

			/*if (newBlock)
			{   singlMark = "����";//����� ������ �����
				singlMark += IntToStr(nBlocks);
				ExpNotes->AddMarker(singlMark, firstPInBlk);
				newBlock = false;//������ ����� ���� }*/

			gInd[k] = k;//����� �� ������ ���������� ������ ��������
			k++;//����������� ������� ����������� ��������
		}
		else if ((lett[0] == 'M') && (lett[1] == 'r') && (lett[2] == 'k'))//������� � ��������� �� ���� ������������
		{
			//��������� � ���������� userText ������� �� ���� ������������ �� ��������� ������
			//������ ������� �� ���� ������������, ���� ����� ����
			/* ������������������ ������ � ����� � ���������� ������
				lettersNum --(1) - ����� ��������� ������
				comUserMakrs-(2) - ���������� ������� �� ���� ������������
				userText ----(3) - ����� �� ������������ � ������� �� ����
				sigsWithMark-(4) - ������ � �������� ����� � ���������
			*/

			itemRd = fread(&z, sizeof(__int32), 1, discontinFile);//1//����� ��������� ������
			unote = new char[z];//����� ��� ����� (����)
			itemRd = fread(&locMrkNum, sizeof(__int32), 1, discontinFile);//2//locMrkNum - ��������� ���������� �������

			//������ ��������� ������
			itemRd = fread((void*)unote, sizeof(char), z, discontinFile);//3//���� �����

			allUsersTxt = "";
			for (i = 0; i < z; i++)//bytesRead = lettersNum
				allUsersTxt += unote[i];

			//���������� ��������� ����������� ("\n\n\r\r\n\n")
			//�� ����� ������� ��� ����������, � ����� ���� - ������� �� ���� ������������
			j = allUsersTxt.AnsiPos("\n\n\r\r\n\n");
			usersComent = allUsersTxt;
			usersComent.Delete(j, usersComent.Length());//���������� � ������������
			ExpNotes->usersNotes->Lines->Text = usersComent;//��������� ����� � ���� TMemo

			if (!usersComent.IsEmpty())//������ �� �����
				usersInfoClick(this);//������� ������� //ExpNotes->Show();

			usersMarks = allUsersTxt;
			usersMarks.Delete(1, j + 5);//������� �� ���� ������������ (����������� � �����, ����� ���������� ��������)

			//������ ������ ��������, � ������� ��������� ������� (���� ������� �������)
			if ((locMrkNum > 0) && (!usersMarks.IsEmpty()))//���������� ������ �������� ������
			{
				sigsWithMark = new __int32[locMrkNum];//������ � �������� ��������, � ������� ��������� �������
				itemRd = fread((void*)sigsWithMark, sizeof(__int32), locMrkNum, discontinFile);//4//������ ����� � ���������

				j = 1;//������� ��������
				for (i = 0; ((i < locMrkNum) && (comMrkNum < devExpParams_int[6])); i++)
				{
					j = usersMarks.AnsiPos("||\n||");//���� ����������� ����� ���������
					if (j > 0)
					{
						singlMark = usersMarks;//�������� ���� ���������� �����
						singlMark.Delete(j, singlMark.Length() - (j - 2));//�������� ����� �����
						if ((singlMark.AnsiPos("��") == 1) && (singlMark.Length() <= 2))//����� �������� ������������ ������
						{
							singlMark = "��";//������ "��������" ����������� ������ �� ��������
							singlMark += IntToStr(nBlocks);//��������� ����� ������ ����� ������
						}

						//�� ����������� ������ ��������� ��� ������ �����
						itemRd = fread((void*)mOnChn, sizeof(bool), ftChan, discontinFile);//4//������ ����� � ���������

						ExpNotes->AddMarker(singlMark, firstPInBlk + sigsWithMark[i], &mOnChn[0]);//��������� �����
						usersMarks.Delete(1, j + 4);//�������� ����������� ����� � ����������� ||\n||
					}
					comMrkNum++;//����� ������� ������� �� ���� ������������
				}
				delete[] sigsWithMark; sigsWithMark = NULL;
			}
			delete[] unote; unote = NULL;
        }
		else if ((lett[0] == 'E') && (lett[1] == 'D') && (lett[2] == ':'))//����������� ����� �������
		{
			timeSepar = (gSignals[k - 1].appear + 1);//����� ������� ��������� �������� ������� 1 �������
			nBlocks++;//������� ������
		}
	}
	SigNumUpDwn->Max = k;//���������� ����������� ����������� ��������
	fclose(discontinFile);//��������� ����
	delete[] pidate; pidate = NULL;

	brdSig[nBlocks] = k;//��� �������������
	for (i = 0; i < nBlocks; i++) //���������� �����
	{
		for (k = brdSig[i]; k < brdSig[i + 1]; k++)
		{
			for (j = 0; j < recLen; j++)
				gSignals[k].s[j] = (short)floor((gSignals[k].s[j] * (maxG / locAdcGain[i])) + 0.5);
		}   
	}
	devExpParams_int[5] = maxG;//����������� ��������
	sampl2mV = devExpParams_float[1] / float(devExpParams_int[11] * devExpParams_int[5]);//������������ ��������� ��������
				
	gRF[0] = 0;//������ ������������ ������
	gRF[1] = ftChan;//��������� ������������ ������ (���������� ��� ������ ftChan)

	Calculate(gSignals, numOfSignal, gPorog, gPreTime, gPostTime, gInd, experimentType, false, NULL);//���������� ��������� �������� ��� �����������
	GraphsTabsShow();//��������� ���� � ���������
	Graphs->Caption = "������� - " + OpenDlg->FileName;//��������� �������� ���� ��������� �����

	if (ftChan > 1)
		SAllChanClick(this);//������� ��� ������
	else
		ChannNumb[0]->Click();//������� ������ �����

	//FillParamTable();//��������� ������� ���������� ����������
	//ResetVisibility();//��������� ���������� ��������
}
//---------------------------------------------------------------------------

void TGraphs::GraphsTabsShow()
{
	//����������� ���� � ��������� (���������)
	__int32 i;

	if (gSignals)
	{
		if (Experiment->TabsCheck->Checked)
		{
			pValues->Visible = true;//����� ����� �������
			//�������� ��������������� ������ �������
			pValues->Row = SigNumUpDwn->Position;//����� ������� ��� ������������ (������� �����)
			pValues->Col = 1;
		}
		else
			pValues->Visible = false;//������� ��������

		//��������� ������ ������� ��������� ��� ���������
		for (i = 0; i < maxChannels; i++)
		{
			if (i < ftChan)
				ChannNumb[i]->Visible = true;//������ ���������� ������ ������ (chanNumb ����)
			else
				ChannNumb[i]->Visible = false;//��������� ������������
		}
		SAllChan->Enabled = bool(ftChan > 1);//������������, ���� �� ������������

		//������������� ��������� ��� �������� ������� � ������������
		for (i = 0; i < maxChannels; i++)
		{
			curntSigChannls[i]->XValues->Multiplier = (double)effDT / 1000;
			csSpecPoint->XValues->Multiplier = (double)effDT / 1000;
			PStart->exmplChannls[i]->XValues->Multiplier = (double)effDT / 1000;
		}
	}

	Graphs->Show();
	Resized(this);
}
//---------------------------------------------------------------------------

void TGraphs::ResetVisibility()
{
	//��������� ����������
	__int32 i;

	Label2->Caption = "�� " + IntToStr(SigNumUpDwn->Max);
	Label2->Visible = true;
	ExpNotes->PIDates->Lines->Text = ExpNotes->pIDateString;//��������� ���������� � �����

	ClearGraphics();//�������� �������
	//AddParamsPoints(trac *apSignals, __int32 apSigNum, __int32 *apIndex, short apExpType)
	AddParamsPoints(gSignals, SigNumUpDwn->Max, gInd, experimentType);//������ ��� �������

	//������ �������� ������ �������������� �����
	signalManager->Visible = true;
	ShowMean->Visible = true;
	expInfo->Visible = true;
	ReCalculat->Visible = true;
	SetExpAs->Visible = true;
	ShowChanN->Visible = true;

	//���������� ������ �������������� � ���������������
	gphSave->Enabled = true;
	parametrExport->Enabled = true;
	signalsExport->Enabled = true;
	prntWin->Enabled = true;//"������" ����

	CrntSig->ReadOnly = false;
	CrntSig->Text = "1";//������� ������ ������
	SigNumUpDwn->Enabled = true;//��������� "�������" �������
	SigNumUpDwn->Position = 1;//������� ������ ������
	ShowSignal(SigNumUpDwn->Position);//������� ������ ������
}
//---------------------------------------------------------------------------

void TGraphs::FillParamTable()
{
	//��������� ������� ���������� ����������
	__int32 i, j, k,
			nrc,//���������� ����������� �������
			sch,//���������� �������������� ������� (� ������������� ������)
			numOfSig,//����� ���������� ��������
			shft1,//����� ������� �������� ��� ampls � peakInds
			shft2;//����� ������� �������� ��� spans
	float apt;
	bool printMark;//���� �� ����� ��� �����������

	nrc = 5 + (5 * __int32(experimentType == 3));//���������� ����������� �������
	sch = (chanls - 1) * __int32(!multiCh);//���������� �������������� ������� (� ������������� ������)
	pValues->ColCount = nrc + sch + 1;//���������� ������� � ������� �� ���������� ����������

	if (experimentType == 3)
	{
		pValues->Cells[5][0] = "Na-���, ��";//������ ��������� ����������������� ������ (��������� ���) (����� �����, ����� ������� �����)
		pValues->Cells[6][0] = "tNa, ���";//������������ ������� ���� (���������� ����, � �������������)
		pValues->Cells[7][0] = "K-���, ��";//������ ��������� ����������������� ������ (�������� ���) (����� �����, ����� ������� �����)
		pValues->Cells[8][0] = "��� ��, ���";//������������� �������� ��� ��������... (� �������������)
		pValues->Cells[9][0] = "st0, ���";//����� �������������� ���������� (� �������������)
	}
	for (i = 1; i < sch; i++)
		pValues->Cells[nrc + i - 1][0] = "�����" + IntToStr(i + 1) + ", ��";//����� i
	pValues->Cells[nrc + i - 1][0] = "������� �� ����";//������� �� ���� ������������

	numOfSig = SigNumUpDwn->Max;//���������� �������� � �����
	pValues->RowCount = numOfSig + 1;//���������� ����� � �������

	printMark = false;//�� ����������� ������ ����� curR = false
	for (j = 0; j < numOfSig; j++)//��������� �������� ���������� � �������
	{
		for (i = 0; i < ExpNotes->addUMark->Tag; i++)//�������� ����� �� �� �����, � ������� ��������� �����
		{
			if (ExpNotes->theMarker->pointOnGraph == j)//������ ����� ��������� � ����� �������
			{
				printMark = true;//��������� �����: ���������� �
				break;
			}
			ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
		}

		if (printMark)//���� ����� ��� �����������
			pValues->Cells[9 + maxChannels][j + 1] = ExpNotes->theMarker->textMark;//������� ������� � ������� ������ �������

		shft1 = (3 * ftChan * gInd[j]) + (3 * gRF[0]);//����� ������� �������� ��� ampls � peakInds
		shft2 = (5 * ftChan * gInd[j]) + (5 * gRF[0]);//����� ������� �������� ��� spans

		pValues->Cells[0][j	+ 1] = IntToStr(j + 1);//����� �������
		pValues->Cells[1][j + 1] = FloatToStrF(gSignals[gInd[j]].appear, ffFixed, 6, 3);//����� ������������� ������� (��������������); � �������� �� ������ ������������
		pValues->Cells[2][j + 1] = FloatToStrF((float)cPrm[0].ampls[shft1] * sampl2mV, ffFixed, 6, 3);//��������� �������� - ����������������� (����� ������� �����)
		pValues->Cells[3][j + 1] = IntToStr(cPrm[0].spans[shft2]);//����� ���������� (������������)
		pValues->Cells[4][j + 1] = IntToStr(cPrm[0].spans[shft2 + 1]);//����� ����� (������������)
		if (experimentType == 3)
		{
			pValues->Cells[5][j + 1] = FloatToStrF((float)cPrm[0].ampls[shft1 + 1] * sampl2mV, ffFixed, 6, 3);//������ ��������� ����������������� ������ (��������� ���) (����� �����, ����� ������� �����)
			pValues->Cells[6][j + 1] = IntToStr(cPrm[0].spans[shft2 + 2]);//������������ ������� ���� (���������� ����, � �������������)
			pValues->Cells[7][j + 1] = FloatToStrF((float)cPrm[0].ampls[shft1 + 2] * sampl2mV, ffFixed, 6, 3);//������ ��������� ����������������� ������ (�������� ���) (����� �����, ����� ������� �����)
			pValues->Cells[8][j + 1] = IntToStr(cPrm[0].spans[shft2 + 3]);//������������� �������� ��� ��������... (� �������������)
			pValues->Cells[9][j + 1] = IntToStr(cPrm[0].spans[shft2 + 4]);//����� �������������� ���������� (� �������������)
		}

		for (i = 1; i < sch; i++)//������� ��������� ������� �� �������������� �������
		{
			k = recLen + i - 1;//����� ������� �������
			pValues->Cells[nrc + i - 1][j + 1] = FloatToStrF((float)gSignals[gInd[j]].s[k] * sampl2mV, ffFixed, 6, 3);
		}
	}
	for (i = 2; i < pValues->ColCount; i++)
		pValues->ColWidths[i] = (pValues->Cells[i][0].Length()) * 7;//������������� ������ �������

	Resized(this);//����� ������� ������� � ����
}
//---------------------------------------------------------------------------

trac* TGraphs::CreatStructSignal(__int32 crNumOfS, __int32 crRecLen)
{
	//�������� ������� � ���������� �������� � �� ���������� (������ � ���� ������)
	/*
	crNumOfS - ������������� ���������� ��������
	crRecLen - ������������� ����� �������
	*/
	__int32 i, j,
			fulChan,//���������� ������� � ������� ����������
			fullSigLen;//��������� ����� �������� �� ���� �������
	trac *crStructSignal = new trac[crNumOfS];

	fullSigLen = (crRecLen * ftChan) + (chanls - ftChan);//��������� ����� �������� �� ���� ������� (�������)

	for (i = 0; i < crNumOfS; i++)
	{
		crStructSignal[i].s = new short[fullSigLen];//�������� �������� �� ���� �������
		for (j = 0; j < fullSigLen; j++)
			crStructSignal[i].s[j] = 0;//�������� �������� �������� ��� ��������
		crStructSignal[i].appear = 0;//����� ������������� ������� (��������������); � �������� �� ������ ������������
	}

    cPrm = new sPr;//������� � ������������ ����������� ��������
	cPrm[0].ampls = new short[3 * crNumOfS * ftChan];//��� ������� �������� (���������)
		//ampls[0] = amplPost - ��������� �������� - ����������������� (����� ������� �����)
		//ampls[1] = ampl1NaPeak - ������ ��������� ����������������� ������ (��������� ���) (����� �����, ����� ������� �����)
		//ampls[2] = ampl2KPeak - ������ ��������� ����������������� ������ (�������� ���) (����� �����, ����� ������� �����)
	cPrm[0].peakInds = new __int32[3 * crNumOfS * ftChan];//��� ������ �������� � �������� ����������
		//peakInds[0] = indMaxPost - ����� ������� � ������������ ��������� amplPost
		//peakInds[1] = indMax1Na - ����� ������� � ������������ ��������� amplPre1
		//peakInds[2] = indMax2K - ����� ������� � ������������ ��������� amplPre2
	cPrm[0].spans = new __int32[5 * crNumOfS * ftChan];//��� ����������� ������� (�������, ����, ���������)
		//spans[0] = riseT - ����� ���������� (������������)
		//spans[1] = fallT - ����� ����� (������������)
		//spans[2] = tSynTrans - ����� �������������� ���������� (� �������������)
		//spans[3] = t1NaPeak - ������������ ������� ���� (���������� ����, � �������������)
		//spans[4] = tDelay - ������������� �������� ��� ��������... (� �������������)

	for (j = 0; j < (3 * crNumOfS * ftChan); j++)
	{
		cPrm[0].ampls[j] = 0;//��������
		cPrm[0].peakInds[j] = 0;//��������
	}
	for (j = 0; j < (5 * crNumOfS * ftChan); j++)
		cPrm[0].spans[j] = 0;//��������

	return crStructSignal;
}
//---------------------------------------------------------------------------

void TGraphs::DeleteStructSignal(trac *delSignal, __int32 delNumOfS)
{
	//�������� ��������� � ��������� (������ � ���� ������)
	/*
	delSignal - ��������� �� ��������� � ���������
	delPrm - ��������� �� ������ � ����������� ����������� ��������
	delNumOfS - ���������� ��������, ��� ������� ���������� ���� ��������������� ������
	*/
	__int32 i;

	for (i = 0; i < delNumOfS; i++)
		delete[] delSignal[i].s;
	delete[] delSignal;	delSignal = NULL;
	
	delete[] cPrm[0].ampls; cPrm[0].ampls = NULL;
	delete[] cPrm[0].peakInds; cPrm[0].peakInds = NULL;
	delete[] cPrm[0].spans; cPrm[0].spans = NULL;
	delete[] cPrm; cPrm = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::gphOpenClick(TObject *Sender)
{
	//��������� ���� � �������� �������� ������������ �������
	//� ���� ������ ���������� ������� ���������� �������� (� ��� ����� ���������� ��������� ������)

	__int32 i;
	AnsiString extntFl;//���������� ������������ �����

	if (OpenDlg->Execute())
	{
		Gists->Close();//�������� ���� � �������������

		//������� �� �� ������
		Experiment->DiscontinWrt->Tag = -1;//����� �� ��������
		ClearMemor(this);//������� ������
		plotFrom = 0;//������ �������

		//�������� ���������� ������������ �����
		extntFl = OpenDlg->FileName.SubString(OpenDlg->FileName.Length() - 2, OpenDlg->FileName.Length());

		//�������� ������ ������� ������ �����
		if (extntFl == "bin")
		{
			ConvertBinary(true);//��������� �������� ������ � ��������� ���(false) ��� � ����������� pra-������(true)
		}
		else
		{
			if (extntFl == "pra")
				ReadDataFile(this);//������������ pra ����
			else if (extntFl == "drr")
				ReadLineData(this);//"��������" drr ����
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::OnClosGraphs(TObject *Sender)
{
	//��� �������� ���� �������� ������
	__int32 i;

	ClearMemor(this);//������� ������
	plotFrom = 0;//������ �����

	//�������� ����� ������ ������ ��������
	for (i = 1; i < 9; i++)
		allCharts[i]->Visible = false;//�������� ��� �����
	_isfull = false;//��� ���������� ��������

	Experiment->Ampl_inT->Font->Style = TFontStyles();//������ ����� ������
	Experiment->Rise_inT->Font->Style = TFontStyles();//������ ����� ������
	Experiment->Fall_inT->Font->Style = TFontStyles();//������ ����� ������
	Experiment->NaAmpl_inT->Font->Style = TFontStyles();//������ ����� ������
	Experiment->KAmpl_inT->Font->Style = TFontStyles();//������ ����� ������
	Experiment->NaDur_inT->Font->Style = TFontStyles();//������ ����� ������
	Experiment->SynDel_inT->Font->Style = TFontStyles();//������ ����� ������
	Experiment->St0_inT->Font->Style = TFontStyles();//������ ����� ������

	Experiment->plottedGrphs = 1;//�������� ������� �������� ��������
	Gists->GistsClear(this);//�������� �����������
	CrntSig->ReadOnly = true;//��������� "�������" �������
}
//---------------------------------------------------------------------------

void TGraphs::ClearGraphics()
{
	//�������� ������� � ������� �������
	//���� ������ �������� �� ������ �� ��������
	__int32 i;

	//�������� ������� ����������
	for (i = 1; i < pValues->RowCount; i++)
		pValues->Rows[i]->Clear();
	pValues->RowCount = 2;//��������� �� �������� ���������� ����� � �������

	csSpecPoint->Clear();
	for (i = 0; i < maxChannels; i++)
	{
		curntSigChannls[i]->Clear();//������� ������� �������
		sigAmpls[i]->Clear();//������� ������� �������� ��������
		allCharts[i]->UndoZoom();//�������� ����������, ������������ �����
	}
	gRiseTsL->Clear();//������ ����� ����������
	gFallTsL->Clear();//������ ����� �����
	gNaAmplsL->Clear();//��������� Na-����
	gNaDursL->Clear();//������������ Na-����
	gKAmplsL->Clear();//��������� K-����
	gSynDelsL->Clear();//������������� ��������
	gSynTrnsL->Clear();//����� �������������� ����������

	curAmpl->Clear();//��������� ����������� ������� (������-�����)
	curRiseT->Clear();//����� ���������� ����������� ������� (������-�����)
	curFallT->Clear();//����� ����� ����������� ������� (������-�����)
	curNaAmpl->Clear();//���� Na-���� ����������� ������� (������-�����)
	curNaDur->Clear();//���������� ������
	curKAmpl->Clear();//���������� ������
	curSynDel->Clear();//���������� ������
	curSynTrns->Clear();//���������� ������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ClearMemor(TObject *Sender)
{
	//����������� ������ �� ��� �������� ����� ������� ������ ������������

    if (Experiment->DiscontinWrt->Tag != 1)//���� ���������� ������ ��� ��������� ���� (�� ��� �������� �����)
	{
		Graphs->ClearGraphics();//�������� ������� � ������� �������
		ExpNotes->DeleteMarkers();//������ �������
		ExpNotes->nmInRec = 0;//���������� ����� ���������� � ���������� ������� ����� ������ (�������� � ������������ ����)
	}
	
	if (gSignals)
	{
		DeleteStructSignal(gSignals, SigNumUpDwn->Tag);//������� ��������� � ���������
		gSignals = NULL;
	}
	if (gInd)
	{
		delete[] gInd;//������� ������ ��������
		gInd = NULL;
	}
	if (devExpParams_int)
	{
		delete[] devExpParams_int;//������� ������ � ����������� ���������� � ������������ (__int32)
		devExpParams_int = NULL;
	}
	if (devExpParams_float)
	{
		delete[] devExpParams_float;//������� ������ � ����������� ���������� � ������������ (float)
		devExpParams_float = NULL;
	}

	CrntSig->ReadOnly = true;
	CrntSig->Text = "0";
	SigNumUpDwn->Enabled = false;
	Label2->Visible = false;
	Graphs->Caption = "�������";

	//������������ ������ �������������� �����
	signalManager->Visible = false;
	ShowMean->Visible = false;
	expInfo->Visible = false;
	ReCalculat->Visible = false;
	SetExpAs->Visible = false;
	ShowChanN->Visible = false;

	//������������ ������ ��������������, ��������������� � �.�.
	gphSave->Enabled = false;
	parametrExport->Enabled = false;
	signalsExport->Enabled = false;
	prntWin->Enabled = false;//"������" ����
}
//---------------------------------------------------------------------------

void TGraphs::ShowSignal(__int32 sn)
{
	//������� ������ � �������� �������
	/*
	sn - ����� ������� (���������������� [1, ..., N])
	*/
	__int32 i, z, k,
			shft1,//����� ������� �������� ��� ampls � peakInds
			shft2,//����� ������� �������� ��� spans
			indBgn,//������ - ������ ���������
			indSN;//����� ������� � ������� �������� (�������� �����)
	double *graphic;//������ � ��������

	indSN = gInd[sn - 1];//���������� ����� ������ ��������
	indBgn = gTimeBgn / effDT;//������ - ������ ���������
	graphic = new double[recLen * (gRF[1] - gRF[0])];//������ � ��������
	k = 0;
	for (z = gRF[0]; z < gRF[1]; z++)
		for (i = 0; i < recLen; i++)
		{
			graphic[k] = gSignals[indSN].s[(z * recLen) + i] * sampl2mV;
			k++;
		}
	PlotCurrentSignal(graphic, indBgn);

	//������������ ��������� �������� ������� �� ��������
	shft1 = (3 * ftChan * indSN) + (3 * 0);//����� ������� �������� ��� ampls � peakInds
	shft2 = (5 * ftChan * indSN) + (5 * 0);//����� ������� �������� ��� spans
	HighLightCrnt(&gSignals[indSN], shft1, shft2, experimentType, indBgn, sn - 1);

	ExpNotes->addUMark->Text = "";//������� �������� ����� � ���� ��� �������
	for (i = 0; i < ExpNotes->addUMark->Tag; i++)//���� � ������� ����������� �����, �� ��������� � � ���� addUMark
	{
		if (((sn - 1) == ExpNotes->theMarker->pointOnGraph) && (ExpNotes->theMarker->chanN[gRF[0]]) &&
			!SAllChan->Checked)//���� ����� �� ������� (�� ������)
		{
			ExpNotes->addUMark->Text = ExpNotes->theMarker->textMark;
			break;
		}
		ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
	}
	delete[] graphic; graphic = NULL;//������� ������ �������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::sigNumUpDwnChangingEx(TObject *Sender,
	  bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	//���������� ������ � �������� ������� ��� ����� ��������� (����������� ��� ����������������)
	//NewValue - �������� ����� ������� ��� ������������ (������� �����)

	if ((NewValue >= 1) && (NewValue <= SigNumUpDwn->Max))
	{
		CrntSig->Text = IntToStr(NewValue);//��������� �������� ��������� ������ �������
		CrntSig->Tag = NewValue;//����� ������ ������� ��� ������������ (����� ��� �������������� �����)
		ShowSignal(NewValue);//������� ������ � �������� �������

		AllowChange = true;//��������� ���������
	}
	else
	{
		CrntSig->Text = IntToStr(SigNumUpDwn->Position);//��������� ������� ��������
		AllowChange = false;//�� ������
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::NextPres(TObject *Sender)
{
	SigNumUpDwn->Position++;//��������� ������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::PrevPres(TObject *Sender)
{
	SigNumUpDwn->Position--;//���������� ������
}
//---------------------------------------------------------------------------

void TGraphs::HighLightCrnt(trac *hSig, __int32 shft1, __int32 shft2, short hExpT, __int32 hIndBgn, __int32 pGr)
{
	//�������� ��������� �������� ������� �� �������� � ������������ ����� �� ��������
	/*
	hSig - ��������� � �������������� ��������
	shft1 - ����� ������� �������� ��� ampls � peakInds
	shft2 - ����� ������� �������� ��� spans
	hExpT - ��� ������������
	hIndBgn - ��������� ��� ��������� ����� �������
	pGr - ����� ����� �� �������
	*/
	__int32 i;

	csSpecPoint->Clear();//����� ����������� �����
	curAmpl->Clear();//���������
	curRiseT->Clear();//����� ����������
	curFallT->Clear();//����� �����

	//�������� ����������� �����
	for (i = gRF[0]; i < gRF[1]; i++)
	{
		if (cPrm[0].peakInds[shft1 + (3 * i)] >= 0)
			csSpecPoint->AddXY((cPrm[0].peakInds[shft1 + (3 * i)] - hIndBgn), (float)cPrm[0].ampls[shft1 + (3 * i)] * sampl2mV);

		if (hExpT == 3)
		{
			csSpecPoint->AddXY((cPrm[0].peakInds[shft1 + (3 * i) + 1] - hIndBgn), (float)cPrm[0].ampls[shft1 + (3 * i) + 1] * sampl2mV);//�������� ����� ��������� Na-����
			csSpecPoint->AddXY((cPrm[0].peakInds[shft1 + (3 * i) + 2] - hIndBgn), (float)cPrm[0].ampls[shft1 + (3 * i) + 2] * (-1) * sampl2mV);//�������� ����� ��������� �-����
		}

		//������������ ��������� ������ �� ��������
		curAmpl->AddXY(hSig->appear + plotFrom, sigAmpls[i]->YValues->operator [](pGr));//(float)cPrm[0].ampls[shft1 + (3 * i)] * sampl2mV);
	}

	i = gRF[0];//������ ������������ �����,
	//������������ ��������� ������ �� ��������
	AmplInT->Title->Text->Text = "���������, " + FloatToStrF((float)cPrm[0].ampls[shft1 + (3 * i)] * sampl2mV, ffFixed, 6, 2) + " ��";

	curRiseT->AddXY(hSig->appear + plotFrom, gRiseTsL->YValues->operator [](pGr));//cPrm[0].spans[shft2]);
	RiseTInT->Title->Text->Text = "����� ����������, " + IntToStr(cPrm[0].spans[shft2 + (5 * i)]) + " ���";

	curFallT->AddXY(hSig->appear + plotFrom, gFallTsL->YValues->operator [](pGr));//cPrm[0].spans[shft2 + 1]);
	FallTInT->Title->Text->Text = "����� �����, " + IntToStr(cPrm[0].spans[shft2 + (5 * i) + 1]) + " ���";

	if (hExpT == 3)
	{
		curNaAmpl->Clear();//��������� Na-����
		curNaAmpl->AddXY(hSig->appear + plotFrom, gNaAmplsL->YValues->operator [](pGr));//(float)cPrm[0].ampls[shft1 + 1] * sampl2mV);
		NaAmplInT->Title->Text->Text = "Na-���, " + FloatToStrF((float)cPrm[0].ampls[shft1 + (3 * i) + 1] * sampl2mV, ffFixed, 6, 2)+ " ��";

		curNaDur->Clear();//������������ Na-����
		curNaDur->AddXY(hSig->appear + plotFrom, gNaDursL->YValues->operator [](pGr));//cPrm[0].spans[shft2 + 2]);
		NaDurInT->Title->Text->Text = "������������ Na-����, " + IntToStr(cPrm[0].spans[shft2 + (5 * i) + 2]) + " ���";

		curKAmpl->Clear();//��������� �-����
		curKAmpl->AddXY(hSig->appear + plotFrom, gKAmplsL->YValues->operator [](pGr));//(float)cPrm[0].ampls[shft1 + 2] * sampl2mV);
		KAmplInT->Title->Text->Text = "K-���, " + FloatToStrF((float)cPrm[0].ampls[shft1 + (3 * i) + 2] * sampl2mV, ffFixed, 6, 2) + " ��";

		curSynDel->Clear();//������������� ��������
		curSynDel->AddXY(hSig->appear + plotFrom, gSynDelsL->YValues->operator [](pGr));//cPrm[0].spans[shft2 + 3]);
		SynDelInT->Title->Text->Text = "������������� ��������, " + IntToStr(cPrm[0].spans[shft2 + (5 * i) + 3]) + " ���";

		curSynTrns->Clear();//����� �������������� ����������
		curSynTrns->AddXY(hSig->appear + plotFrom, gSynTrnsL->YValues->operator [](pGr));//cPrm[0].spans[shft2 + 4]);
		SynTrnsInT->Title->Text->Text = "������ ����, " + IntToStr(cPrm[0].spans[shft2 + (5 * i) + 4]) + " ���";
	}

	hSig = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::DeletSignalClick(TObject *Sender)
{
	//������� �� ������� ������ �������� �������
	__int32 i,
			p;//����� ������������� �������

	p = SigNumUpDwn->Position - 1;//����� ������������� �������

	for (i = gRF[0]; i < gRF[1]; i++)//������� ����� � ��������
		sigAmpls[i]->Delete(p);
	gRiseTsL->Delete(p);//������� ����� � ��������
	gFallTsL->Delete(p);
	if (experimentType == 3)//������� ����� � ��������
	{
		gNaAmplsL->Delete(p);
		gNaDursL->Delete(p);
		gKAmplsL->Delete(p);
		gSynDelsL->Delete(p);
		gSynTrnsL->Delete(p);
	}

	RemoveSignals(SigNumUpDwn->Position - 1);//���������� ���������� ������� �� ������ ���������������
	Label2->Caption = "�� " + IntToStr(SigNumUpDwn->Max);//���������� ����� (����������) ���������� ��������
	if ((p + 1) > SigNumUpDwn->Max)//�������� ��������� �������
		SigNumUpDwn->Position = SigNumUpDwn->Max;//�������� ������ �������
	else
		ShowSignal(p + 1);//���������� ������� (����� ������� ������)
}
//---------------------------------------------------------------------------

void TGraphs::RemoveSignals(__int32 sigToDel)
{
	//������� ������ ������� �� ������� �������� (���� ��� ���������)
	/*
	sigToDel - ����� ���������� ������� (��������� � ����)
	*/
	__int32 i,
			oldNum,//���� ��������
			newNum;//��������� ��������

	oldNum = SigNumUpDwn->Max;
	newNum = oldNum - 1;

	SigNumUpDwn->Max = newNum;//����� (����������) ���������� ��������
	for (i = 0; i < ExpNotes->addUMark->Tag; i++)//���� � ���������� ������� ����������� �����, �� ������� � �
	{
		if (sigToDel == ExpNotes->theMarker->pointOnGraph)
			ExpNotes->DeleteAMark(ExpNotes->theMarker);
		ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
	}

	for (i = sigToDel; i < newNum; i++)//�������� ������ �������� � ������� �������� �����
		gInd[i] = gInd[i + 1];//�������� ������ �������� � ������� �������� �����

	for (i = 0; i < ExpNotes->addUMark->Tag; i++)//�������� ������ ��������, � ������� ��������� �����
	{
		if (ExpNotes->theMarker->pointOnGraph > sigToDel)
			ExpNotes->theMarker->pointOnGraph--;//�������� ������ ��������, � ������� ��������� �����
		ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
	}

	for (i = (sigToDel + 1); i < pValues->RowCount - 1; i++)
	{
		pValues->Rows[i] = pValues->Rows[i + 1];//���������� �����
		pValues->Cells[0][i] = IntToStr(i);//������ ����� �������
	}
	pValues->RowCount--;//������� ������� �� �������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::GphSaveClick(TObject *Sender)
{
	//����� ����� ����� ��� ���������� ������������
	SaveDlg->Title = "������� ��� �����";
	//SaveDlg->Options = TOptin << ofOverwritePrompt;//true;
	SaveDlg->Filter = "*.pra|*.pra";
	SaveDlg->DefaultExt = "pra";

	SaveDlg->FileName = OpenDlg->FileName;//���������� ��������� ��� ������� ������
	if (SaveDlg->Execute())
	{
		CrntSig->ReadOnly = true;//�� ����� ���������� �������� ������� �� ��������� ������ �������
		SigNumUpDwn->Enabled = false;//����� �� ����� ���������� ������ ����������� �������

		if (SaveExpDataToFile(experimentType, NULL, SigNumUpDwn->Max, false))
		{
			FillParamTable();//��������� ������� ���������� ����������
			ResetVisibility();//��������� ����������
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::usersInfoClick(TObject *Sender)
{
	//���������� ���������� �� ������������, �������� �������������

	//�������� �������� ����
	ExpNotes->mainLinesLbl->Caption = "����� �������� �� ������������";
	ExpNotes->usersNotes->Visible = true;//���� ��� ���������������� ������
	ExpNotes->addUMark->Visible = true;//���� ����� ������� �� ���� ������������
	ExpNotes->addMarkLbl->Visible = true;//�������-�����
	ExpNotes->PIDates->Visible = false;//���� ������ � �����

	ExpNotes->Show();//��������� ���� ��������
}
//---------------------------------------------------------------------------

void TGraphs::SetSeriesMultipliers()
{
	//�������� ���������� ������� ��� �������� � ������������ � ���������� ������������
	__int32 i,
			cnt;
	double fmValue;//������ ��� ������������ ��������

	cnt = sigAmpls[gRF[0]]->Count();//gAmplsL ���������� ������������� ��������

	if (cnt >= 1)//�� �������� ���� �����
	{
		if (Experiment->shwAmpls->Checked)//���������� �������� ��� ����
		{
			for (i = gRF[0]; i < gRF[1]; i++)
				sigAmpls[i]->YValues->Multiplier = 1;//gAmplsL
			for (i = 2; i < 9; i++)
				allCharts[i]->Series[0]->YValues->Multiplier = 1;//������� ����� - �������� ������
		}
		else if (Experiment->pCentFirstS->Checked)//���������� �������� �������� ������� �������
		{
			for (i = gRF[0]; i < gRF[1]; i++)
			{
				fmValue = fabs(sigAmpls[i]->YValues->operator [](0));//�������� ������� ��������
				if (fmValue < 1)//������������ �������� ����� ����
					fmValue = 1;//�������� ��������, ����� �������� ������� �� ����
				sigAmpls[i]->YValues->Multiplier *= (double(100) / fmValue);//����� ���������
			}

			for (i = 2; i < 9; i++)
			{
				cnt = allCharts[i]->Series[0]->Count();//���������� ����� � ������ �������
				if (cnt >= 1)//������ ����������
				{
					fmValue = fabs(allCharts[i]->Series[0]->YValues->operator [](0));//�������� ������� ��������
					if (fmValue < 1)//������ �������� ����� ����
						fmValue = 1;//�������� ��������, ����� �������� ������� �� ����
					allCharts[i]->Series[0]->YValues->Multiplier *= (double(100) / fmValue);//����� ���������
				}
			}
		}
		else if (Experiment->pCentMax->Checked)//���������� �������� ������������ ��������
		{
			for (i = gRF[0]; i < gRF[1]; i++)
			{
				fmValue = fabs(sigAmpls[i]->YValues->MaxValue);//������������ �������� � ������ �������
				if (fmValue < 1)//������������ �������� ����� ����
					fmValue = 1;//�������� ��������, ����� �������� ������� �� ����
				sigAmpls[i]->YValues->Multiplier *= (double(100) / fmValue);//����� ���������
			}
			for (i = 2; i < 9; i++)
			{
				cnt = allCharts[i]->Series[0]->Count();//���������� ����� � ������ �������
				if (cnt >= 1)//������ ����������
				{
					fmValue = fabs(allCharts[i]->Series[0]->YValues->MaxValue);//������������ �������� � ������ �������
					if (fmValue < 1)//������������ �������� ����� ����
						fmValue = 1;//�������� ��������, ����� �������� ������� �� ����
					allCharts[i]->Series[0]->YValues->Multiplier *= (double(100) / fmValue);//����� ���������
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TGraphs::parametrExportClick(TObject *Sender)
{
	//��������������� ���������� �������� � ��������� ���� (dat - ������)

	FILE *stream;//����� ������ ������ � ����
	AnsiString bfr;//��������������� ����������
	__int32 i, j,
			shft1,//����� ������� �������� ��� ampls � peakInds
			shft2,//����� ������� �������� ��� spans
			rlSh,//������������ ����� ������� (� ������ ����-�������, �������)
			bgnSigSh;//����� �������, � �������� ���������� ������ ������(� ������ ���������������� � ����-�������)
	markers *printMark;//���� �� ����� ��� �����������

	//������������ ��������� ��������
	SaveDlg->Title = "������� ����������";//���������� ���������
	SaveDlg->Filter = "*.dat|*.dat";//������ ����������� ��������
	SaveDlg->DefaultExt = "dat";//���������� �� ���������

	//�� ��������� ��������� � ������ ��� ��������� ����� ��� ����������
	bfr = OpenDlg->FileName;
	bfr.Delete(bfr.Length() - 3, 4);//������� ����������
	SaveDlg->FileName = bfr;

	if (SaveDlg->Execute())
	{
		stream = fopen(SaveDlg->FileName.c_str(), "w");//������ ���� � ����������� ��������

		//��������� �������� ���������� �������
		fprintf(stream, "time\tamplPost\tArea\triseT\tfallT");
		if (experimentType == 3)//������������: ������� ���� ���������
			fprintf(stream, "\tampl1Na\tampl2K\tt1Na\tsynDelay\tsynTr");
		for (j = 0; ((j < (chanls - 1)) && !multiCh); j++)//���� �� �������������� �����
		{
			bfr = "\tChannel";
			bfr += IntToStr(j + 2);
			fprintf(stream, bfr.c_str());
		}
		if (ExpNotes->addUMark->Tag > 0)//���� ���� ������� �� ���� ������������
			fprintf(stream, "\tMarks");
		fprintf(stream,"\n");//��������� ������ ������ ��������� (�������� ���������� �������)

		//��������� ����������� ���������� �������
		fprintf(stream, "sec\tmV\tmVms\tmks\tmks");//��� ���������� �������
		if (experimentType == 3)//������������: ������� ���� ���������
			fprintf(stream, "\tmV\tmV\tmks\tmks\tmks");//��� ���������� �������

		for (j = 0; ((j < (chanls - 1)) && !multiCh); j++)//���� �� �������������� �����
			fprintf(stream, "\tmV");//��� ������ �� �������������� �������
		if (ExpNotes->addUMark->Tag > 0)//���� ���� ������� �� ���� ������������
			fprintf(stream, "\tMarks");
		fprintf(stream, "\n");//��������� ������ ������ ��������� (����������� ���������� �������)

		//���������� ���������� ������ (�������� ����������)
		printMark = NULL;//�� ����������� ������ ����� printMark = false
		rlSh = recLen - (gPostTime / effDT);//������������ ����� ������� (� ������ ����-�������, �������)
		bgnSigSh = (gRF[0] * recLen) + (gPostTime / effDT);//����� �������, � �������� ���������� ������ ������(� ������ ���������������� � ����-�������)
		for (i = 0; i < SigNumUpDwn->Max; i++)
		{
			shft1 = (3 * ftChan * gInd[i]) + (3 * gRF[0]);//����� ������� �������� ��� ampls � peakInds
			shft2 = (5 * ftChan * gInd[i]) + (5 * gRF[0]);//����� ������� �������� ��� spans
			fprintf(stream, "%8.3f\t%7.2f\t%7.2f\t%d\t%d",
					gSignals[gInd[i]].appear,//����� ������������� ������� (� �������� �� ������ ������������)
					(float)cPrm[0].ampls[shft1] * sampl2mV,//���������
					IntegralCalculate(&gSignals[gInd[i]], rlSh, sampl2mV * effDT, bgnSigSh),//"�������" ��� ��������
					cPrm[0].spans[shft2],
					cPrm[0].spans[shft2 + 1]);
			if (experimentType == 3)//������������: ������� ���� ���������
				fprintf(stream, "\t%7.2f\t%7.2f\t%d\t%d\t%d",
							 (float)cPrm[0].ampls[shft1 + 1] * sampl2mV,
							 (float)cPrm[0].ampls[shft1 + 2] * sampl2mV,
							 cPrm[0].spans[shft2 + 2],
							 cPrm[0].spans[shft2 + 3],
							 cPrm[0].spans[shft2 + 4]);

			if (!multiCh)//���� �� �������������� �����
				for (j = 0; j < (chanls - 1); j++)//��������� ������ �� �������������� �������
					fprintf(stream, "\t%7.2f", gSignals[gInd[i]].s[recLen + j] * sampl2mV);
			if (ExpNotes->addUMark->Tag > 0)//���� ���� ������� �� ���� ������������
			{
				//�������� ����� �� �� �����, � ������� ��������� �������
				for (j = 0; j < ExpNotes->addUMark->Tag; j++)
				{
					if ((ExpNotes->theMarker->pointOnGraph == i) && (ExpNotes->theMarker->chanN[gRF[0]]))
					{
						printMark = ExpNotes->theMarker;//�������� ������� �����
						break;
					}
					ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
				}
				if (printMark)
				{
					fprintf(stream, "\t");
					fprintf(stream, printMark->textMark.c_str());//������� ������� � ������� ������ �������
				}
			}
			fprintf(stream, "\n");
		}
		fclose(stream);//��������� ���� (��������� ����� ������ ������ � ����)
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::signalsExportClick(TObject *Sender)
{
	//��������������� �������� �������� � ��������� ���� (dat - ������)

	FILE *stream;//����� ������ ������ � ����
	AnsiString bfr;//��������������� ����������
	__int32 i, j, z,
			indBgn;//������ - ������ ���������

	//������� ��������
	SaveDlg->Title = "������� ��������";//���������� ���������
	SaveDlg->Filter = "*.dat|*.dat";//������ ����������� ������
	SaveDlg->DefaultExt = "dat";//���������� �� ���������

	//�� ��������� ��������� ��� ��������� ����� � ������
	bfr = OpenDlg->FileName;
	bfr.Delete(bfr.Length() - 3, 4);//������� ����������
	SaveDlg->FileName = bfr;

	if (SaveDlg->Execute())
	{
		stream = fopen(SaveDlg->FileName.c_str(), "w");
		indBgn = gTimeBgn / effDT;//������ - ������ ���������

		//��������� �������� �������
		fprintf(stream, "time\t");
		for (i = 0; i < SigNumUpDwn->Max; i++)
		{
			fprintf(stream, "s_%d\t", (i + 1));
			for (z = 1; z < ftChan; z++)
				fprintf(stream, "sub_%d\t", z);
		}
		fprintf(stream, "\n");

		//��������� ����������� �������
		fprintf(stream, "ms\t");
		for (i = 0; i < ftChan * SigNumUpDwn->Max; i++)
			fprintf(stream, "mV\t");
		fprintf(stream, "\n");

		//��������� �������
		for (j = indBgn; j < recLen; j++)//���������� ������� ������� (�������� ��������)
		{
			fprintf(stream, "%6.3f\t", (float)((j - indBgn) * effDT) / 1000);//�����
			for (i = 0; i < SigNumUpDwn->Max; i++)//���������� ������� � ��������� � dat-����
				for (z = 0; z < ftChan; z++)
					fprintf(stream, "%7.2f\t", ((float)gSignals[gInd[i]].s[(z * recLen) + j] * sampl2mV));//����������
			fprintf(stream, "\n");
		}

		fclose(stream);//��������� ���� (����� ������ ������ � ����)
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ShowMeanClick(TObject *Sender)
{
	//������ ���������� ������
	__int32 i, j, k,
			fullSigLen,//��������� ����� �������� �� ���� ������� (�������)
			indBgn,//������ - ������ ���������
			count,//���������� ��������
			*smIndex,//������ ��������
			shft1,//����� ������� �������� ��� ampls � peakInds
			shft2;//����� ������� �������� ��� spans
	float average;//�������� ���������� ������
	trac *averSgnl;//�������� ��������� �������� �������
	sPr averPrm;//������ � ����������� �������� �������
	double *graphic;//������ � ��������

	fullSigLen = (recLen * ftChan) + (chanls - ftChan);//��������� ����� �������� �� ���� ������� (�������)

	averSgnl = new trac[1];
	averSgnl[0].s = new short[fullSigLen];

	count = SigNumUpDwn->Max;//���������� ��������, �� ������� ���������� ����������
	for (j = 0; j < fullSigLen; j++)//������� �������� ������� (�������� ��������)
	{
		average = 0;//�������
		for (i = 0; i < count; i++)//������� ��������
			average += (float)gSignals[gInd[i]].s[j];

		average /= count;//�������� ������� ��� ������� ������� �������
		averSgnl[0].s[j] = short(floor(average + 0.5));//�������� � ���� ������ short
	}

	//�������� ������ ��������
	smIndex = new __int32[1];
	smIndex[0] = 0;
	averPrm.ampls = new short[3 * ftChan];//��� ������� �������� (���������)
	averPrm.peakInds = new __int32[3 * ftChan];//��� ������ �������� � �������� ����������
	averPrm.spans = new __int32[5 * ftChan];//��� ����������� ������� (�������, ����, ���������)

	//�������� ��������� �������� �������
	//Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
	//		    __int32 *inds, short expT, bool newNull, short chn, sPr *avrP)
	Calculate(averSgnl, 1, gPorog, gPreTime, gPostTime, smIndex, experimentType, true, &averPrm);

	//������� ���������� ������
	indBgn = gTimeBgn / effDT;//������ - ������ ���������
	graphic = new double[recLen * (gRF[1] - gRF[0])];//������ � ��������
	k = 0;
	for (j = gRF[0]; j < gRF[1]; j++)
		for (i = 0; i < recLen; i++)
		{
			graphic[k] = averSgnl[0].s[(j * recLen) + i] * sampl2mV;
			k++;
		}
	PlotCurrentSignal(graphic, indBgn);

	//�������� ����������� �����
	csSpecPoint->Clear();//��������� �������� �����
	for (i = gRF[0]; i < gRF[1]; i++)
	{
		shft1 = (3 * i);//����� ������� �������� ��� ampls � peakInds
		shft2 = (5 * i);//����� ������� �������� ��� spans
		if (averPrm.peakInds[shft1] >= 0)//��������� ����������� �����
			csSpecPoint->AddXY((averPrm.peakInds[shft1] - indBgn), (float)averPrm.ampls[shft1] * sampl2mV);

		if (experimentType == 3)
		{
			csSpecPoint->AddXY((averPrm.peakInds[shft1 + 1] - indBgn),//�������� ����� ��������� Na-����
				(float)averPrm.ampls[shft1 + 1] * sampl2mV);

			csSpecPoint->AddXY((averPrm.peakInds[shft1 + 2] - indBgn),//�������� ����� ��������� �-����
				((float)averPrm.ampls[shft1 + 2] * (-1)) * sampl2mV);
		}
	}

	shft1 = (3 * gRF[0]);//����� ������� �������� ��� ampls � peakInds
	shft2 = (5 * gRF[0]);//����� ������� �������� ��� spans

	//������� �������� ����������
	curAmpl->Clear();//���������
	AmplInT->Title->Text->Text = "���������, " + FloatToStrF((float)averPrm.ampls[shft1] * sampl2mV, ffFixed, 6, 2) + " ��";

	curRiseT->Clear();//����� ����������
	RiseTInT->Title->Text->Text = "����� ����������, " + IntToStr(averPrm.spans[shft2]) + " ���";

	curFallT->Clear();//����� �����
	FallTInT->Title->Text->Text = "����� �����, " + IntToStr(averPrm.spans[shft2 + 1]) + " ���";

	if (experimentType == 3)
	{
		curNaAmpl->Clear();//��������� Na-����
		NaAmplInT->Title->Text->Text = "Na-���, " + FloatToStrF((float)averPrm.ampls[shft1 + 1] * sampl2mV, ffFixed, 6, 2)+ " ��";

		curNaDur->Clear();//������������ Na-����
		NaDurInT->Title->Text->Text = "������������ Na-����, " + IntToStr(averPrm.spans[shft2 + 2]) + " ���";

		curKAmpl->Clear();//��������� �-����
		KAmplInT->Title->Text->Text = "K-���, " + FloatToStrF((float)averPrm.ampls[shft1 + 2] * sampl2mV, ffFixed, 6, 2) + " ��";

		curSynDel->Clear();//������������� ��������
		SynDelInT->Title->Text->Text = "������������� ��������, " + IntToStr(averPrm.spans[shft2 + 3]) + " ���";

		curSynTrns->Clear();//����� �������������� ����������
		SynTrnsInT->Title->Text->Text = "������ ����, " + IntToStr(averPrm.spans[shft2 + 4]) + " ���";
	}

	//�������� ����������
	delete[] averSgnl[0].s; averSgnl[0].s = NULL;
	delete[] averSgnl; averSgnl = NULL;
	delete[] smIndex; smIndex = NULL;
	delete[] averPrm.ampls; averPrm.ampls = NULL;
	delete[] averPrm.peakInds; averPrm.peakInds = NULL;
	delete[] averPrm.spans; averPrm.spans = NULL;
	delete[] graphic; graphic = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::asIntracelClick(TObject *Sender)
{
	//������������� ��� ������������
	/* 1 - ����������
	   2 - ���������-���������������
	   3 - ���������-������������
	   4 - ��������� + ���������� */

	experimentType = 2;//��������������� (K-���, Na-���, ��������. �������� � ������. ���������� ��������)
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::asExtracelClick(TObject *Sender)
{
	//������������� ��� ������������
	experimentType = 3;//������������ (������ ��������� �������������� ������)
	ReCalculatClick(this);//������� ��������� ��������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::asMiniClick(TObject *Sender)
{
	//������������� ��� ������������
	experimentType = 1;//���������� (K-���, Na-���, ��������. �������� � ������. ���������� ��������)
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ReCalculatClick(TObject *Sender)
{
	//����������� ��������� ��������
	__int32 i, j, k,
			locPolar,//���������� ���������
			curS,//����� �������� ������� (� ������� gInd)
			numOfS;
	bool nlMins;//����� ��������� PStart->nulMinus->Checked
	double *grafik;//������ � ���������

	if (PStart->Visible)
		PStart->Close();//������� ���� ����������

	PStart->GetASignal->Caption = "��������";//������ ������� ������
	PStart->StartRec->Visible = false;//������ ������� ������������
	PStart->CloseWin->Visible = false;//������ �������� ���� ����������
	PStart->ExpInfo->Visible = false;//����� ������ ���� ����� ���������������� ����������
	ReCalculat->Tag = experimentType;//������� ��� ������������ � PStart
	PStart->GetASignal->OnClick = PStart->ReCalClick;//������ ������� ������
	PStart->SetControlVis(experimentType);//������ �������� ������ ������ ���������� � ������� ��������
	PStart->Invert->Checked = false;//��������� ������� ���������� ��������������
	PStart->Invert->OnClick = PStart->InvertClick;//��������� ������� �� ����� ����������

	numOfS = SigNumUpDwn->Max;//����� ���������� ��������� ��������
	curS = gInd[(SigNumUpDwn->Position - 1)];//����� �������� ������� (� ������� gInd)

	PStart->Caption = "�������� ����������";

	//������ ������
	grafik = new double[recLen * (gRF[1] - gRF[0])];//������ � ���������
	k = 0;
	for (j = gRF[0]; j < gRF[1]; j++)
		for (i = 0; i < recLen; i++)
		{
			grafik[k] = gSignals[curS].s[(j * recLen) + i] * sampl2mV;
			k++;
		}
	//PlotStandardSignal(double *sData, __int32 sRecLen, __int32 sEffDT, __int32 sIndBgn)
	PStart->PlotStandardSignal(grafik, recLen, effDT, 0);//������ ������� ������ ��� �����������

	PStart->PreTime->Text = IntToStr(devExpParams_int[8]);//���-�����
	PStart->PostTime->Text = IntToStr(devExpParams_int[9]);//����-�����
	PStart->Porog->Text = IntToStr(devExpParams_int[10]);//�����

	PStart->timeOfDrawBgn = 0;//(float)(gTimeBgn);//������� �������� �������-������ � ������ PStart
	PStart->RefreshLines(effDT);//������������ �����
	PStart->Invert->Checked = false;//�� �������������
	PStart->NulMinus->Checked = false;//�� �������� ����-�����

	if (PStart->ShowModal() == mrOk)//���� ����� "��������"
	{
		//����� �������� ���� � ��������� ��������, ���� ��� ����� "��������"
		gPreTime = StrToInt(PStart->PreTime->Text);
		gPostTime = StrToInt(PStart->PostTime->Text);
		gPorog = short((float)StrToInt(PStart->Porog->Text) / sampl2mV);//��������� ���������� (�������)

		//���� ���������� ������� invert, �� �������������� ����������� �������
		if (PStart->Invert->Checked)
			for (j = gRF[0]; j < gRF[1]; j++)
				for (i = 0; i < recLen; i++)
					gSignals[gInd[j]].s[i] = (-1) * gSignals[gInd[j]].s[i];

		//���� ������� "�������� ����-�����", �� ����� ��������� ����-����� ��� ������� ������� ��������
		//newNull = PStart->nulMinus->Chacked;//���������, ��� ����������� �������� ����������

		//���������� �������� ���������� � ������ �����������
		//Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
		//	        __int32 *inds, short expT, bool newNull, short chn, sPr *avrP)
		Calculate(gSignals, numOfS, gPorog, gPreTime, gPostTime, gInd, experimentType, PStart->NulMinus->Checked, NULL);

		//===================================
		//�������� ����� ���������
		devExpParams_int[8] = StrToInt(PStart->PreTime->Text);
		devExpParams_int[9] = StrToInt(PStart->PostTime->Text);
		devExpParams_int[10] = StrToInt(PStart->Porog->Text);

		//��������� ������ ����� ��������� ��������
		gTimeBgn = 0;
		if (experimentType == 2)
			gTimeBgn = gPostTime;
		else if (experimentType == 3)
			gTimeBgn = gPreTime;

		ClearGraphics();//�������� �������
		FillParamTable();//������������� �������
		//AddParamsPoints(trac *apSignals, __int32 apSigNum, __int32 *apIndex, short apExpType)
		AddParamsPoints(gSignals, SigNumUpDwn->Max, gInd, experimentType);//���������� ��� �������
		CopyRefSignal_forGists();//������ � ���������� ���������� � ������ �����������
		ShowSignal(SigNumUpDwn->Position);//������� ��� �� ������, ������� ��� ��������� �� ���������
    }

	//��������������� ������� "��������������"
	PStart->Invert->Checked = false;//PStart->invert->Visible = true;
	PStart->Invert->OnClick = NULL;//��������� ������� �� ����� ����������
	PStart->GetASignal->OnClick = PStart->GetASignalClick;//��������������� �������

	//��������������� ��� ����
	PStart->GetASignal->Caption = "������ �������";//��������������� ������� ������
	PStart->StartRec->Visible = true;//������ ������� ������������
	PStart->CloseWin->Visible = true;//������ �������� ���� ����������
	PStart->ExpInfo->Visible = true;//����� ������ ���� ����� ���������������� ����������

	delete[] grafik; grafik = NULL;
}
//---------------------------------------------------------------------------

void TGraphs::ReplotExamplRecalc()
{
	//����������� ���������� ������� ��� ���������� ���������� ��������

	__int32 i, j, k,
			curS;//����� �������� �������
	double *grafik,//������ ������ � ��������� �� ���� �������
		   locPolar;//����������

	curS = gInd[(SigNumUpDwn->Position - 1)];

	locPolar = 1;
	if (PStart->Invert->Checked)
		locPolar = (-1);

	grafik = new double[recLen * (gRF[1] - gRF[0])];//������ � ���������
	k = 0;
	for (j = gRF[0]; j < gRF[1]; j++)
		for (i = 0; i < recLen; i++)
		{
			grafik[k] = locPolar * gSignals[curS].s[(j * recLen) + i] * sampl2mV;
			k++;
		}
	//PlotStandardSignal(double *sData, __int32 sRecLen, __int32 sEffDT, __int32 sIndBgn)
	PStart->PlotStandardSignal(grafik, recLen, effDT, 0);//������ ������� ������ ��� �����������

	delete[] grafik; grafik = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::GraphChartDblClick(TObject *Sender)
{
	//������������� ���� �� ��� ������ ���� ��������
	__int32 i;
	TChart *theChart;//��������� �� �������������� ����

	_isfull = !_isfull;//�������� �� �����-������ ������

	theChart = (TChart*)Sender;//�������������� ����
	theChart->AllowZoom = false;//��������� �����������

	if (_isfull)
	{
		theChart->Width = Graphs->Width - 7;//����� ������ (�� �� ����)
		theChart->Height = Graphs->Height - 50;//����� ������ (�� �� ����)
		theChart->Left = 0;//����� ����
		theChart->Top = 0;//������� ����

		for (i = 0; i < 9; i++)
			if (i != theChart->Tag)
			{
				mask[i] = allCharts[i]->Visible;//���������� ��������
				allCharts[i]->Visible = false;//������ ���������� ��� ��������� �������
			}
	}
	else
	{
		for (i = 0; i < 9; i++)
			if (i != theChart->Tag)
				allCharts[i]->Visible = mask[i];//��������������� �������� ���������

		Resized(this);//�������������� �������
	}

	theChart->AllowZoom = true;//��������� �����������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::Resized(TObject *Sender)
{
	__int32 numOfgraphs = 0, // ���������� ������������� ��������
			i,
			winWidth,//������ ���� ��������
			winHeight,//������ ���� ��������
			//������� � ������� ������ �� ������� ������� (�����, �� ����� ����)		
			iVCh[5] = {0, 0, 0, 0, 0};

	if (pValues->Visible)
	{
		//��������� ������� �������
		pValues->Top = 0;
		pValues->Left = 0;
		winWidth = 0;
		//winHeight = 0;
		for (i = 0; i < pValues->ColCount; i++)
			winWidth += pValues->ColWidths[i];

		if (winWidth < 415)
			winWidth = 415;

		pValues->Width = winWidth + 30;//���� ������ scrollBar

		Graphs->Width = pValues->Width + 8;
		pValues->Height = Graphs->Height - 58;
	}
	else
	{
		//�������� ������ �� ������� (�����), ������� �����
		for (i = 0; i < 9; i++)
			if (allCharts[i]->Visible)
			{
				iVCh[numOfgraphs] = i;
				numOfgraphs++;
			}
	
		 winWidth = Graphs->Width;
		 if ((numOfgraphs == 2) || (numOfgraphs == 4))
		 {
			 if ((winWidth / 2) < 235)
				 Graphs->Width = 235 * 2;
		 }
		 if (numOfgraphs == 3)
		 {
			 if ((winWidth / 3) < 235)
				 Graphs->Width = 235 * 3;
		 }
	
		 winWidth = Graphs->Width - 7;
		 winHeight = Graphs->Height - 50;
	
		//������ ������ ������
		if (numOfgraphs == 1)
		{
			i = 0;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = 0;
			allCharts[iVCh[i]]->Width = winWidth;
			allCharts[iVCh[i]]->Height = winHeight;
		}
	
		if (numOfgraphs == 2)
		{
			i = 0;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = 0;
			allCharts[iVCh[i]]->Width = winWidth/2;
			allCharts[iVCh[i]]->Height = winHeight;
	
			i = 1;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = winWidth/2;
			allCharts[iVCh[i]]->Width = winWidth/2;
			allCharts[iVCh[i]]->Height = winHeight;
	
		}
	
		if (numOfgraphs == 3)
		{
			i = 0;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = 0;
			allCharts[iVCh[i]]->Width = winWidth/3;
			allCharts[iVCh[i]]->Height = winHeight;
	
			i = 1;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = winWidth/3;
			allCharts[iVCh[i]]->Width = winWidth/3;
			allCharts[iVCh[i]]->Height = winHeight;
	
			i = 2;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = 2 * (winWidth/3);
			allCharts[iVCh[i]]->Width = winWidth/3;
			allCharts[iVCh[i]]->Height = winHeight;
		}
	
		if (numOfgraphs == 4)
		{
			i = 0;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = 0;
			allCharts[iVCh[i]]->Width = winWidth/2;
			allCharts[iVCh[i]]->Height = winHeight/2;
	
			i = 1;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = winWidth/2;
			allCharts[iVCh[i]]->Width = winWidth/2;
			allCharts[iVCh[i]]->Height = winHeight/2;
	
			i = 2;
			allCharts[iVCh[i]]->Top = winHeight/2;
			allCharts[iVCh[i]]->Left = 0;
			allCharts[iVCh[i]]->Width = winWidth/2;
			allCharts[iVCh[i]]->Height = winHeight/2;
	
			i = 3;
			allCharts[iVCh[i]]->Top = winHeight/2;
			allCharts[iVCh[i]]->Left = winWidth/2;
			allCharts[iVCh[i]]->Width = winWidth/2;
			allCharts[iVCh[i]]->Height = winHeight/2;
		}
	
		if (numOfgraphs == 5)
		{
			//SignalTrack->Visible = true - ����� ������
			i = 0;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = 0;
			allCharts[iVCh[i]]->Width = winWidth/3;
			allCharts[iVCh[i]]->Height = winHeight/2;
	
			//AmplInT
			i = 1;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = winWidth/3;
			allCharts[iVCh[i]]->Width = winWidth/3;
			allCharts[iVCh[i]]->Height = winHeight/2;
	
			//NaAmplInT
			i = 2;
			allCharts[iVCh[i]]->Top = 0;
			allCharts[iVCh[i]]->Left = 2 * (winWidth/3);
			allCharts[iVCh[i]]->Width = winWidth/3;
			allCharts[iVCh[i]]->Height = winHeight/2;
	
			//RiseTInT
			i = 3;
			allCharts[iVCh[i]]->Top = winHeight/2;
			allCharts[iVCh[i]]->Left = 0;
			allCharts[iVCh[i]]->Width = winWidth/2;
			allCharts[iVCh[i]]->Height = winHeight/2;
	
			//FallTInT
			i = 4;
			allCharts[iVCh[i]]->Top = winHeight/2;
			allCharts[iVCh[i]]->Left = winWidth/2;
			allCharts[iVCh[i]]->Width = winWidth/2;
			allCharts[iVCh[i]]->Height = winHeight/2;
	
		}
	}
}
//---------------------------------------------------------------------------

void TGraphs::PlotCurrentSignal(double *pcsData, __int32 pcsIndBgn)
{
	//������ ������ ������� (��� ���� �� ���������) ������ �� ����� ����� ������
	/*
	pcsData - ������ � ���������
	pcsIndBgn - ��������� ������ �������
	*/

	__int32 i;

	SignalTrack->AutoRepaint = false;//��������� �������������� ��� ��������� �������� ���������

	for (i = gRF[0]; i < gRF[1]; i++)//������ ������ ��� ������
	{
		curntSigChannls[i]->Clear();//�������� ������ ������
		curntSigChannls[i]->AddArray(&pcsData[pcsIndBgn + ((i - gRF[0]) * recLen)], (recLen - pcsIndBgn) - 1);
	}

	SignalTrack->BottomAxis->SetMinMax(0, double((recLen - pcsIndBgn) * effDT) / 1000);
	SignalTrack->AutoRepaint = true;//���������� ��������������, ����� ������ ���������
	SignalTrack->Repaint();//Refresh();//��������� ����

	pcsData = NULL;
}
//---------------------------------------------------------------------------

void TGraphs::AddParamsPoints(trac *apSignals, __int32 apSigNum, __int32 *apIndex, short apExpType)
{
	//������� ����� �� ������� ����������
	/*
	apSignals - ��������� � ���������
	apSigNum - ���������� �������������� ��������
	apIndex - ������ ��������
	apAmplCoeff - ����������� ��������� ��������
	apExpType - ��� ������������
	*/
	__int32 i, j,
			nrc,//���������� ����������� ������
			shft1,//����� ������� �������� ��� ampls � peakInds
			shft2;//����� ������� �������� ��� spans

	//������������� ������� ����������
	nrc = 4 + (5 * __int32(apExpType == 3));//���������� ����������� ������
	for (i = 1; i < nrc; i++)
		allCharts[i]->AutoRepaint = false;//��������� �������������� ���� ������, ����� SignalTrack

	for (j = 0; j < apSigNum; j++) //apSigNum - 1
	{
		for (i = gRF[0]; i < gRF[1]; i++)
		{
			shft1 = (3 * ftChan * apIndex[j]) + (3 * i);//����� ������� �������� ��� ampls � peakInds
			shft2 = (5 * ftChan * apIndex[j]) + (5 * i);//����� ������� �������� ��� spans
			sigAmpls[i]->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].ampls[shft1] * sampl2mV);
		}

		shft1 = (3 * ftChan * apIndex[j]) + (3 * gRF[0]);//����� ������� �������� ��� ampls � peakInds
		shft2 = (5 * ftChan * apIndex[j]) + (5 * gRF[0]);//����� ������� �������� ��� spans

		gRiseTsL->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].spans[shft2]);
		gFallTsL->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].spans[shft2 + 1]);
		if (apExpType == 3)
		{
			gNaAmplsL->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].ampls[shft1 + 1] * sampl2mV);
			gNaDursL->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].spans[shft2 + 2]);
			gKAmplsL->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].ampls[shft1 + 2] * sampl2mV);
			gSynDelsL->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].spans[shft2 + 3]);
			gSynTrnsL->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].spans[shft2 + 4]);
		}
	}

	for (i = 1; i < nrc; i++)
	{
        allCharts[i]->AutoRepaint = true;//��������������� �������������� ������
		allCharts[i]->Repaint();//Refresh();//��������� ����
	}

	SetSeriesMultipliers();//�������� ���������� ������� ��� ��������
	apSignals = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ChartsClickSeries(TCustomChart *Sender, TChartSeries *Series,
	  int ValueIndex, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	//������� �� ������� �������� ��������
	SigNumUpDwn->Position = ValueIndex + 1;//������� ��������������� ������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::pValuesSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
	//��� ��������� �����-���� ������ ������ ����� �������� �������
	SigNumUpDwn->Position = ARow;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::selAllClick(TObject *Sender)
{
	//�������� ��� ������� �������� �������

//	pValues->Selection.Left = 2;
//	pValues->Selection.Right = 2;
//	pValues->Selection.Top = 4;
//	pValues->Cols->
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::selCopyClick(TObject *Sender)
{
	//�������� ���������� ������� ������� (� ����� ������)
	__int32 i, j,
			colL, colR,
			rowT, rowB;
	AnsiString s;
	HGLOBAL hgBuffer;
	char* chBuffer;

	colL = pValues->Selection.Left;
	colR = pValues->Selection.Right;
	rowT = pValues->Selection.Top;
	rowB = pValues->Selection.Bottom;

	//���������� �����������
	s = "";
	for (i = rowT; i <= rowB; i++)
	{
		for (j = colL; j <= colR; j++)
		{
			s += pValues->Cells[j][i];
			s += "\t";
		}
		s += "\r\n";
	}

	if (OpenClipboard(NULL))//��������� ����� ������
	{
		EmptyClipboard();//������� �����

		hgBuffer = GlobalAlloc(GMEM_DDESHARE, s.Length() + 1);//�������� ������
		chBuffer = (char*)GlobalLock(hgBuffer);//��������� ������
		strcpy(chBuffer, LPCSTR(s.c_str()));//�������� ����� �� ������ chBuffer
		GlobalUnlock(hgBuffer);//������������ ������
		SetClipboardData(CF_TEXT, hgBuffer);//�������� ����� � ����� ������

		CloseClipboard(); //��������� ����� ������
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::amplInTAfterDraw(TObject *Sender)
{
	//������ ����� �� ���� ������������
	__int32 i, j,
			pCount,//���������� ����� �� �������
			pnOnGrph,//����� ����� �� �������
			x0, x1,//� - ���������� ��� �������
			y0, y1,//y - ���������� ��� �������
			fH,//������ �������
			fW;//������ �������

	if (showMarks->Checked)
		for (i = 0; i < ExpNotes->addUMark->Tag; i++)
		{
			for (j = gRF[0]; j < gRF[1]; j++)//j - ����� ������ ������� ������ �������������
			{
				pCount = sigAmpls[j]->Count();//���������� ����� �� �������
				if (ExpNotes->theMarker->chanN[j])//����� �� ����� �� ������ ������
				{
					pnOnGrph = ExpNotes->theMarker->pointOnGraph;//����� ����� �� �������
					if ((pnOnGrph < pCount) && (pnOnGrph >= 0))//������ � ������ ������� ���� �� �������
					{
						x0 = sigAmpls[j]->CalcXPos(pnOnGrph);//x0 ���������� �������
						y0 = sigAmpls[j]->CalcYPos(pnOnGrph);//y0 ���������� �������
						fH = AmplInT->Canvas->TextHeight(ExpNotes->theMarker->textMark);//������ �������
						fW = __int32((float)AmplInT->Canvas->TextWidth(ExpNotes->theMarker->textMark) * 1.15);//������ �������
						x1 = x0 - floor(fW / 2);//x1 ���������� �������
						y1 = y0 - (2 * fH);//y1 ���������� �������

						AmplInT->Canvas->Rectangle(x1, y1, x1 + fW, y1 + fH);//����� ��� �������
						AmplInT->Canvas->TextOutA(x1 + 2, y1, ExpNotes->theMarker->textMark);//���� �������
						AmplInT->Canvas->Line(x0, y0 - fH, x0, y0);//������� (��������� �� �����, � ������� ��������� �������)
					}
				}
			}
			ExpNotes->theMarker = ExpNotes->theMarker->nextM;//��������� � ��������� �����
		}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::showMarksClick(TObject *Sender)
{
	//���������� �� "������� �� ���� ������������"
	showMarks->Checked = !showMarks->Checked;//
	AmplInT->Refresh();//���������� ��� ����� �����
}
//---------------------------------------------------------------------------

void TGraphs::CopyRefSignal_forGists()
{
	//�������� ������ �� ��������� � ��������� � ������� � � ������ ���������� ����������

	if ((gSignals) && (SigNumUpDwn->Max > 1))//���������� �������� ������ ������
		Gists->GistsRefresh(-1);
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::CrntSigKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
	//������� �� ����� ������ �������������
	if (((Key >= '0') && (Key <= '9')) || (Key != '\b'))//�������� ������������ �������� ��������
		SigNumUpDwn->Position = StrToInt(CrntSig->Text);//
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::CrntSigKeyPress(TObject *Sender, char &Key)
{
	//�������� ������������ �������� ������� � ���� ������ �������

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//�������� ������������ �������� ��������
		Key = '\0';//�������� ������������ ������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::CrntSigKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	//����������� ������� ������� (��� ���������� ������ �������)

	if (!CrntSig->ReadOnly)//
	{
		if (Key == VK_UP)//������ "����" - ���������� ������
			SigNumUpDwn->Position--;//
		else if (Key == VK_DOWN)//������ "�����" - ��������� ������
			SigNumUpDwn->Position++;//
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::remGrpSigsClick(TObject *Sender)
{
	//�������� �������� ������ ��������

	if (gSignals)//������� ����������
	{
		ChoosDlg->setsForNumbersChs();//����������� ���� ������� ��� ����� ������� �������� (���������)

		ChoosDlg->sav = false;//������ �������� ������ ��������
		ChoosDlg->Caption = "�������� ������ ��������";
		ChoosDlg->ShowModal();
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::saveGrpSigsClick(TObject *Sender)
{
	//���������� �������� ������ ��������

	if (gSignals)
	{
		ChoosDlg->setsForNumbersChs();//����������� ���� ������� ��� ����� ������� �������� (�����������)

		ChoosDlg->sav = true;//������ ���������� ������ ��������
		ChoosDlg->Caption = "���������� ������ ��������";
		ChoosDlg->ShowModal();
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::dividGrpsClick(TObject *Sender)
{
	//��������� ������� ������ �� ������ �� nInGrp ��������

	if (gSignals)
	{
		ChoosDlg->setsForNOfGrp();//����������� ���� ������� ��� ����� ����� �������� � ������ (nInGrp)
		ChoosDlg->sav = true;//����� ������������� ������� ����
		ChoosDlg->ShowModal();//�������� ������ ��� ����� ������� ����� ��������
	}
}
//---------------------------------------------------------------------------

void TGraphs::RemovEnteredNumbers(__int32 *removIndex, __int32 count)
{
	//���������� ��� �������� ������ ��������
	/*
	svRmIndex - ��������� �� ������ � ��������� ��������
	count - ���������� ��������� ��������
	*/
	__int32 i, j;

	//������� ��� ��������� �������
	for (i = 0; i < count; i++)
		RemoveSignals(removIndex[i]);//���������� ���������� ������� �� ������ ���������������

	if (ChoosDlg->sav)//������ ���������� ���������� ������
		GphSaveClick(this);//��������� ������ �������� � ��������� ������
	else
		ResetVisibility();//��������� ����������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::PrintWinClick(TObject *Sender)
{
	//����������� ��, ��� ���������� � ���� ��������
	__int32 i;
	Graphics::TBitmap *frmImg;
	AnsiString imgFlNm;

	frmImg = Graphs->GetFormImage();

	imgFlNm = OpenDlg->FileName;
	imgFlNm.Delete(imgFlNm.Length() - 3, 4);
	imgFlNm += "_1.bmp";
	i = 2;
	while (FileExists(imgFlNm))
	{
		imgFlNm.Delete(imgFlNm.Length() - 5, 6);
		imgFlNm += "_" + IntToStr(i) + ".bmp";
		i++;
	}

	frmImg->SaveToFile(imgFlNm);//���������� �����
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::SignalTrackMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	//��������� �������� ���������� � ������ �����
	__int32 cp,//����� ����� �������, �� ������� �������
			x0, x1,//� - ���������� ��� �������
			y0, y1,//y - ���������� ��� �������
			fH,//������ �������
			fW;//������ �������
	float gX, gY;//�������� ��������� �� �������
	AnsiString voltText;//������� � ����������� � ������ �����
	TRect rc;

	if ((Button == 0) && gSignals && (curntSigChannls[gRF[0]]->Count() > 0))//���� ������ ������ ������ ���� � ������� ����������
	{
		gX = curntSigChannls[gRF[0]]->XScreenToValue(X);//�-��������� ��������� �����

		//����� ����� ����� �������, ������� ����� ��������� � ���������
		x0 = 0;//����� �������
		x1 = curntSigChannls[gRF[0]]->XValues->Count() - 1;//������ �������
		cp = x0 + floor((x1 - x0) / 2);//����������� ����� ����� �������
		while ((x1 - x0) > 3)
		{
			gY = (float)cp * effDT * 1e-3;//�-���������� ����� �������
			if (gY < gX)
				x0 = cp;
			else
				x1 = cp;

			cp = x0 + floor((x1 - x0) / 2);//��������� ����� ����� �������
		}
		cp = x0;//����� ����� �������, �� ������� ������� (Locate(graphX) - ������� ������� ������������)

		if (cp >= 0)
		{
			x0 = curntSigChannls[gRF[0]]->CalcXPos(cp);//����� ������������ ������� �� �������
			y0 = curntSigChannls[gRF[0]]->CalcYPos(cp);//����� ������������ ������� �� �������
			gY = curntSigChannls[gRF[0]]->YValues->operator [](cp);//(float)gSignals[i].s[indBgn + cp] * sampl2mV;//� ��������� �� gIndBgn
			voltText = "x=" + FloatToStrF(gX, ffFixed, 6, 2) + "; "
					 + "y=" + FloatToStrF(gY, ffFixed, 6, 2);

            //����� ����� �������
			SignalTrack->Canvas->Pen->Width = 2;
			SignalTrack->Canvas->Pen->Color = clGreen;
			SignalTrack->Canvas->Brush->Color = clWhite;
			SignalTrack->Canvas->Font->Size = 10;
			SignalTrack->Canvas->Font->Style = TFontStyles() << fsBold;
			SignalTrack->Canvas->Font->Name = "Arial";

			//������� �������
			fH = (__int32)((float)SignalTrack->Canvas->TextHeight(voltText) * 1.15);
			fW = (__int32)((float)SignalTrack->Canvas->TextWidth(voltText) * 1.05);

			x1 = x0 - floor(fW / 2);
			y1 = y0 - 2 * fH;

			rc.left = x1 + 1; rc.top = y1 + 1;
			rc.right = x1 + fW - 2; rc.Bottom = y1 + fH - 2;

			SignalTrack->Canvas->Rectangle(x1, y1, x1 + fW, y1 + fH);//����� ��� �������
			SignalTrack->Canvas->FillRect(rc);
			SignalTrack->Canvas->TextOutA(x1 + 1, y1, voltText);//���� �������
			SignalTrack->Canvas->Line(x0, y0 - fH, x0, y0);//������� (��������� �� �����, � ������� ��������� �������)
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::expParamsClick(TObject *Sender)
{
	//�������� ���������� �� ������������

	//�������� �������� ����
	ExpNotes->mainLinesLbl->Caption = "���������� � �����";
	ExpNotes->PIDates->Visible = true;//���� ������ � �����
	ExpNotes->usersNotes->Visible = false;//���� ��� ���������������� ������
	ExpNotes->addUMark->Visible = false;//���� ����� ������� �� ���� ������������
	ExpNotes->addMarkLbl->Visible = false;//�������-�����

	ExpNotes->PIDates->Top = ExpNotes->usersNotes->Top;
	ExpNotes->PIDates->Left = ExpNotes->usersNotes->Left;
	ExpNotes->PIDates->Width = ExpNotes->usersNotes->Width;
	ExpNotes->PIDates->Height = ExpNotes->addUMark->Top + ExpNotes->addUMark->Height;

	ExpNotes->Show();
}
//---------------------------------------------------------------------------

void TGraphs::ConvertBinary(bool cTyp)
{
	//������ ��������� ����� � ������������ ����������� ������ ����� ������ (�����������)
	//������� ������ � ��������� ���
	/*
	cTyp - ��� ����������� (false - � �����, true - � pra-������)
	*/

	HANDLE ContinRecFile;//����� �����, �� �������� ������ ������
	FILE *stream;//����� ������ ���������� � ����
	__int32 i, j, k, t,
			recStep,//��� ������
			numOfSig,//����� ���������� ��������
			flSize,//������ �����
			totSigNum,//���������� ���������� ��������
			backLen,//����� ����� (�������)
			realSigNum,//���������� ������������ ��������
			leadChan,//������� ����� (�������� ������)
			nGain;//����������� ��������
	short *dataBlock,//���� � ��������� ��������� �� ������� ������
		  *data,//������ � ������� ��� ���� �������
		  **dataCh,//������ � ������� ����������
		  polarity;//����������
	AnsiString fileParametr,//����� ����������
			   convertedData;//��� ����� � ����������������� �������
	char *pidate;//���������� � ��������� � ���� (Programm Information and Date)
	float discrT1,//�������� ����� ������������� (���)
		  mVOnChan;//������ (����������) �� ������
	bool forwc;//��������� ���������� ������ ����� ������ �� �����
	unsigned long bytesRead,
				  totRead,//����� ��������� ����
				  lpFileSizeHigh;//������� ������ �����

	totRead = 0;
	pidate = new char[40];//������� ������� ��� ������ ������ �� �����

	//��������� � ������ ���� � ������������������ �������
	ContinRecFile = CreateFile(OpenDlg->FileName.c_str(), GENERIC_READ, FILE_SHARE_READ,
				   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	//��������� ��������� ����
	forwc = ReadFile(ContinRecFile, (void*)pidate, sizeof(char) * 37, &bytesRead, 0);
	totRead += bytesRead;
	fileParametr = pidate;
	i = fileParametr.AnsiPos("ElphAcqu v5");//���� ������� � ������ ����� (���������)
	if (i <= 0)//��� ����� ������ � �����
	{
		Experiment->DevEvents->Text = "���� ���� �� ������";//��������� � ������������� ��������� ���� ������������ ���� (�� ��� ���������)
		CloseHandle(ContinRecFile);//��������� ����
		return;
	}

	//����������, ���������� ��� ������ ������ (�.�. ��� ����������� ���������)
	/* ������������������ ������ ������������ ����������
		sampls2mV ----(1) - ����������� �������� �������� �� �������� ��� � (�����)������
		oChanNumb ----(2) - ���������� ������������ �������
		minDiscrTime -(3) - ����������� (��������) ����� �������������
		oDiscrTime ---(4) - ���������������� ����� �������������
	*/
	forwc = ReadFile(ContinRecFile, (void*)&chanls, sizeof(__int32), &bytesRead, 0); //���������� ����������� �������
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&nGain, sizeof(__int32), &bytesRead, 0);//����������� ��������
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&discrT1, sizeof(float), &bytesRead, 0);//�������� ����� ������������� (������������)
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&discrT, sizeof(float), &bytesRead, 0); //����������� ����� ������������� (������������)
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&Experiment->maxADCAmp, sizeof(__int32), &bytesRead, 0);//������������ ��������� ��� (� ��������)
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&Experiment->maxVoltage, sizeof(float), &bytesRead, 0);//�������� ���������� ������� ���������� (�����������)
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&leadChan, sizeof(__int32), &bytesRead, 0);//������� ����� (�������� ������)
	totRead += bytesRead;

	Experiment->a.m_nGain = nGain;//����������� ��������
	sampl2mV = Experiment->maxVoltage / (Experiment->maxADCAmp * nGain);//����������� �������� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
	ftChan = chanls;//���������� ������� � ������� ���������� (full trace channels)
	effDT = discrT * chanls;//����������� ����� ������������� = discrT * chanls
	bytesRead = (37 * sizeof(char)) + (3 * sizeof(float)) + (4 * sizeof(__int32));// * 2) ;//������ ��������� �����
	if (totRead != bytesRead)//������ ������
	{
		Experiment->DevEvents->Text = "������ ������";//��������� �� ������
		CloseHandle(ContinRecFile);//��������� ����
		return;
	}

	convertedData = OpenDlg->FileName;//��� ����� � ����������������� �������
	convertedData.Delete(convertedData.Length() - 3, 4);//������� ����������
	
	if (cTyp)//�������������� � pra-����
	{
		SaveDlg->FileName = convertedData + ".pra";//��� �����

		flSize = GetFileSize(ContinRecFile, &lpFileSizeHigh);//������ �����
		flSize -= ((37 * sizeof(char)) + sizeof(__int32) + (3 * sizeof(float)));//�������� ������ ��������� �����
		flSize = floor((flSize / sizeof(short)) / chanls);//���������� �������� �� ���� �����

		data = new short[chanls * flSize];//������������� ������ ��� ������
		forwc = ReadFile(ContinRecFile, (void*)data, sizeof(short) * chanls * flSize, &bytesRead, 0);
		
		dataCh = new short*[chanls];//������������� ������ ��� ������
		for (i = 0; i < chanls; i++)
			dataCh[i] = new short[flSize];//������������� ������ ��� ������
		csSpecPoint->Clear();//
		j = 0;
		for (i = 0; i < chanls; i++)
			for (t = 0; t < flSize; t++)
			{
				dataCh[i][t] = data[j];
				j++;
			}
		delete[] data; data = NULL;
		PStart->continRec = dataCh;//����� ������ �� ������ �������

		Graphs->gRF[0] = 0;//������ �������� �����
		Graphs->gRF[1] = chanls;//��������� �������� �����
		for (i = 0; i < maxChannels; i++)
		{
			Graphs->curntSigChannls[i]->XValues->Multiplier = double(effDT) / 1000;//�������� ��������� ��� "�������" ��������
			PStart->exmplChannls[i]->XValues->Multiplier = double(effDT) / 1000;//�������� ��������� ��� "�������" ��������
		}
		//����� ������ �� � ���� ���������� � ��� �������
		if (PStart->winDraw(flSize) == mrOk)//������ ������ "����" (��������� ������)
		{
			//���� �������
			recLen = (__int32)floor((float(StrToInt(PStart->LenSpont->Text) * 1000) / effDT) + 0.5);//����� �������� ������� (�������)
			backLen = (__int32)floor((float(StrToInt(PStart->BackTime->Text) * 1000) / effDT) + 0.5);//����� ����� (�������)
			multiCh = true;//����� ����� ������ (true = ��������������)
			ftChan = 1 + (chanls - 1) * (__int32)multiCh;//���������� ������� � ������� ���������� (full trace channels)
			gPorog = short((float)StrToInt(PStart->Porog->Text) / sampl2mV);//��������� ���������� (�������)
			polarity = PStart->postCompInvert;//����������

			ExpNotes->addUMark->Tag = 0;//��� �������

			devExpParams_int = new __int32[ehead[2].numOfElements];//������ ������ � ����������� ���������� � ������������ (__int32)
			devExpParams_int[0]	= multiCh;//0//����� ����� ������ (true = ��������������)
			devExpParams_int[1]	= recLen;//1//����� �������� ������� � ��������
			devExpParams_int[2] = (__int32)discrT;//2//����� ������������� (������������)
			devExpParams_int[3]	= chanls;//3//���������� ����������� �������
			devExpParams_int[4]	= leadChan;//4//������� ����� (�������� ������)
			devExpParams_int[5]	= nGain;//5//����������� ��������
			devExpParams_int[6] = ExpNotes->addUMark->Tag;//6//���������� �������
			devExpParams_int[7] = 1;//7//��� ������������, ���������� �������
			devExpParams_int[8] = 0;//8//gPreTime (� �������������)
			devExpParams_int[9] = 0;//9//gPostTime (� �������������)
			devExpParams_int[10] = (__int32)gPorog;//10//����������� ����� (�������)
			devExpParams_int[11] = Experiment->maxADCAmp;//11//������������ ��������� (� ��������)
			devExpParams_int[12] = -Experiment->maxADCAmp;//12//����������� ��������� (� ��������)
			devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//������� ���������� �����
			devExpParams_int[14] = 0;//14//�� ������������

			//float - ���������
			devExpParams_float = new float[ehead[3].numOfElements];//������ ������ � ����������� ���������� � ������������ (float)
			devExpParams_float[0] = sampl2mV;//0//����������� �������� �������� �� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
			devExpParams_float[1] = Experiment->maxVoltage;//1//�������� ������� ���������� (� ������������)
			devExpParams_float[2] = -1;//2//������ ����������
			devExpParams_float[3] = discrT;//3//����� ������������� (������������)
			devExpParams_float[4] = 0;//4//�� ������������
			devExpParams_float[5] = 0;//5//�� ������������

			totSigNum = floor(flSize / recLen);//���������� ���������� ���������� ��������
			if (totSigNum > limitSigNum)//����������� �� ���������� ��������
				totSigNum = limitSigNum;//��������� ������������ ���������� ��������
			gSignals = Graphs->CreatStructSignal(totSigNum, recLen);//��������� � ��������� � �� ����������

			realSigNum = 0;//���������� ������������ ��������
			i = 0;
			while ((i < flSize) && (realSigNum < totSigNum))//����������� ������� �����
			{
				if ((dataCh[0][i] * polarity) > gPorog)//��������� ������ (��������� ���)
				{
					gSignals[realSigNum].appear = i * effDT * 1e-6;//����� ������������� �������-�������������� (������� �� ������ ������������)
					for (t = 0; t < chanls; t++)//���������� ������
					{
						if ((i - backLen) < 0)//��������� ����� ������� �� ��������
							k = backLen - i;//���������� ����������� �������
						else//���� ������ �������
							k = 0;//���������� ��� �������
						for (j = k; (j < recLen) && ((i + j) < flSize); j++)//���������� �������
						{
							gSignals[realSigNum].s[j] = dataCh[t][i - backLen + j] * polarity;//������� �������
						}
					}
					realSigNum++;//���������� ������������ ��������
					i += (recLen - backLen);//���������� ������� � ������������ ��������
				}
				else
					i++;//��������� ������
			}
			gInd = new __int32[realSigNum];//������� �������� ��������. ��� �������� �������, ������� ��� ����� �� ����� �������
			for (i = 0; i < realSigNum; i++)
            	gInd[i] = i;//������ ���� ��������
			Graphs->SaveExpDataToFile(1, NULL, realSigNum, false);//���������� ������
		}
		//������� �������
		delete[] dataCh; dataCh = NULL;
	}
	else//������������ � �����
	{
		convertedData += ".dat";
		stream = fopen(convertedData.c_str(), "w");//������ ���� � ���������� �������
		fprintf(stream, "time");//��������� ���������
		for (i = 0; i < chanls; i++)
		{
			fileParametr = "\tChann_";
			fileParametr += IntToStr(i + 1);
			fprintf(stream, fileParametr.c_str());//��������� ���������
		}
		fprintf(stream, "\nsec");//��������� ���������
		for (i = 0; i < chanls; i++)
			fprintf(stream, "\tmV");//��������� ���������
		fprintf(stream, "\n");//��������� ���������

		//������, ������������ � ������� ������ � ������ ����
		dataBlock = new short[chanls];//���� � ��������� ��������� �� ������� ������
		t = 0;//������� �������
		recStep = floor(discrT / discrT1);//��� ������
		while (forwc)
		{
			forwc = ReadFile(ContinRecFile, (void*)dataBlock, chanls * sizeof(short), &bytesRead, 0);
			forwc = (bytesRead > 0);
			if (forwc)//���� ���-�� �����������
			{
				fprintf(stream, "%f", (t * discrT * chanls * 1e-6));//��������� ����� (�������)
				//���� (discrTime * ����� ������), �.�. �������� � ���������������� ������������� �� ���������
				for (i = 0; i < chanls; i++)
				{
					mVOnChan = (float)dataBlock[i] * sampl2mV;//������ (����������) �� ������
					fprintf(stream, "\t%6.2f", mVOnChan);//������� �������� ���������� �� ������ � ������ ������
				}
				fprintf(stream, "\n");
			}
			t += recStep;//���������� ������� �������
		}

		delete[] pidate; pidate = NULL;
		delete[] dataBlock; dataBlock = NULL;
		fclose(stream);//��������� ���� (��������� ����� ������ ������ � ����)
		CloseHandle(ContinRecFile);//��������� ����
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ShowTheChannlClick(TObject *Sender)
{
	//�������� ��������� ���������� ������
	TMenuItem *copyOfItem;//����� ��������� �� ������� ������ �������, ��������� ������� (��������� �������������)
	__int32 i;

	//�������� ����� ���������� ������ (�������� ������ �������)
	copyOfItem = (TMenuItem*)Sender;//����������� � ������� ���� ������ ������ �� ������� ��������� ������ �������
	copyOfItem->Checked = true;//������ ��������� ��������� ������ ������� ������� (�������� ChannNumb[0])
	gRF[0] = ShowChanN->IndexOf(copyOfItem);//�������� ����� ���������� ������
	ShowChanN->Caption = "����� " + IntToStr(gRF[0] + 1);//������ ��������� �����
	gRF[1] = gRF[0] + 1;//������������ ���� ��������� �����

	FillParamTable();//��������� ������� ���������� ����������
	ResetVisibility();//��������� ����������
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::SAllChanClick(TObject *Sender)
{
	//������ ����������� �������

	if (!SAllChan->Checked)//�������� ��� ������
	{
		SAllChan->Checked = true;
		ShowChanN->Caption = "��� ������";
		gRF[0] = 0;//������ ������������ ������
		gRF[1] = ftChan;//��������� ������������ ������
		FillParamTable();//��������� ������� ���������� ����������
		ResetVisibility();//��������� ����������
	}
}
//---------------------------------------------------------------------------

