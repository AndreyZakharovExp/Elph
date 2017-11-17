//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <math.h>
#include <algorith.h>

#pragma hdrstop

#include "TimeCourses.h"
#include "Gistograms.h"
#include "MainForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TGists *Gists;

//---------------------------------------------------------------------------
__fastcall TGists::TGists(TComponent* Owner)
	: TForm(Owner)
{
	//������ ���������� ����������
	//����� ��������� ����������� ������� ����������� �� �������

	allCharts[0] = MiniFreq;//������� ��������� ��������
	allCharts[1] = amplG;//��������� ��������
	allCharts[2] = riseTG;//������� ����������
	allCharts[3] = fallTG;//������� �����
	allCharts[4] = NaAmplG;//��������� Na-����
	allCharts[5] = NaDurG;//������������ Na-����
	allCharts[6] = KAmplG;//��������� �-����
	allCharts[7] = synDelayG;//������������� ��������
	allCharts[8] = synTrnsG;//������� �������������� ����������
}
//---------------------------------------------------------------------------
void __fastcall TGists::Resized(TObject *Sender)
{
	//��� ��������� ��������� ����������
	__int32 numOfgraphs = 0, // ���������� ������������� ��������
			i,
			winWidth,//������ ���� ��������
			winHeight,//������ ���� ��������
			//������� � ������� ������ ������� �������� (������
			iVCh[4] = {0, 0, 0, 0};

	//���� ������ �� ������� ������� (�����)

	for (i = 0; i < 9; i++)
		if (allCharts[i]->Visible)
		{
            iVCh[numOfgraphs] = i;
			numOfgraphs++;
		}

	winWidth = Gists->Width;
	if ((numOfgraphs == 2) | (numOfgraphs == 4))
	{
		 if ((winWidth / 2) < 235)
			 Gists->Width = 235 * 2;
	}
	if (numOfgraphs == 3)
	{
		 if ((winWidth / 3) < 235)
			 Gists->Width = 235 * 3;
	}

	winWidth = Gists->Width - 7;
	winHeight = Gists->Height - 35;

	if (numOfgraphs == 0)//��������� ����, ���� ��� ������� �����������
		Gists->Close();

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
}
//---------------------------------------------------------------------------

void TGists::RateMiniSigCalc(TLineSeries *graphOnGraphs)
{
	//������ � ���������� ������� ������� ����������� ��������
	/*
	graphOnGraphs - ������ �� ������, �� �������� ���� ������
	*/

	__int32 i,
			count;//���������� ��������
	float tAppear1,//����� ������������� �������
		  tAppear2,//����� ������������� ���������� �������
		  difTAp,//������� ����� �������������
		  meanFreq,//������� ������� ��������
		  lastFreq;//������� � ��������� ������

	count = graphOnGraphs->Count();
	if (count > 1)
	{
		freqM->Clear();
		MiniFreq->LeftAxis->Automatic = true;
		meanFreq = 0;
		for (i = 0; i < (count - 1); i++)
		{
			tAppear1 = graphOnGraphs->XValues->operator [](i);//����� ������������� ������� //gstSignals[gstIndex[i]].appear;//
			tAppear2 = graphOnGraphs->XValues->operator [](i + 1);//����� ������������� ���������� �������
			if (tAppear2 <= 0)
				tAppear2 = 0;

			difTAp = tAppear2 - tAppear1;
			if (difTAp < 0.001)
				difTAp = 0.001;

			freqM->AddXY(tAppear2, 1 / difTAp);
		}

		lastFreq = 1 / difTAp;//������� � ��������� ������
		meanFreq = count / tAppear2;//����� ����� �������� �� �� �����

		if ((freqM->MaxYValue() - freqM->MinYValue()) < 1)
		{
			MiniFreq->LeftAxis->Automatic = false;
			MiniFreq->LeftAxis->SetMinMax(lastFreq - 1, lastFreq + 1);
		}

		MiniFreq->Title->Text->Text = "������ ������� " + FloatToStrF(meanFreq, ffFixed, 5, 2) + " ��";
	}
}
//---------------------------------------------------------------------------

float TGists::GistsCalculation(TLineSeries *graphOnGraphs, TBarSeries *theGist)
{
	//��������������� ������ ����������� ��� ��������� �������
	/*
	graphOnGraphs - ������ �� ������, �� �������� ���� ������
	theGist - ������ �� ����������� �� ����������, ��������� � �������� ��������
	*/

	__int32 i, g,
			count,//���������� ���������� �������� (���������� �������� ���������)
			nOutL,//���������� ��������, ���������� ����� ����� ��������
			nOutLplus,//���������� ��������, �������� �� ����� �������
			nOutR,//���������� ��������, ���������� �� ������   ��������
			nOutRplus,//���������� ��������, �������� �� ������ �������
			resultPc,//�������� ������� ������������ ��������
			intervals,//���������� ����������, �� ������� ������������ ��������
			*vals;//������ � ��������� ��������������� �������� ����������
	float y,
		  viewPc,//������� ������������ ��������
		  minY,//����� ������� ��������� ��������
		  maxY,//������ ������� ��������� ��������
		  minYback,
		  maxYback,
		  pcStep,//����������� ��� ��������� ���������
		  dY,
		  increm,//��� �������� �������� ���������
		  average;//������� �������� ���������
	bool znak;//���� ���������

	count = graphOnGraphs->Count();//���������� �������� ���������
	viewPc = ((float)abs(theGist->ParentChart->Tag)) / 100;//������� ������������ ��������
	znak = (theGist->ParentChart->Tag > 0);//���� ���������
	if (count > 1)
	{
		minY = (float)(graphOnGraphs->MinYValue());//����� ������� ��������� ��������
		maxY = (float)(graphOnGraphs->MaxYValue());//������ ������� ��������� ��������
		increm = (maxY - minY) / 1000;//��� �������� �������� ���������
		pcStep = 1 / count;//����������� ��� ��������� ���������
		if (pcStep < 0.01)
			pcStep = 0.01;
		nOutL = 0;//���������� ��������, ���������� ����� ����� ��������
		nOutR = 0;//���������� ��������, ���������� �� ������ ��������
		while ((((1 - viewPc) - ((float)(nOutL + nOutR) / count)) > pcStep) && (minY < maxY))//������� ������� �� ���������� ��������� �������� ������������ ��������
		{
            minYback = minY;
			maxYback = maxY;
			nOutLplus = 0;//���������� ��������, �������� �� ����� �������
			dY = 0;
			while ((((float)nOutLplus / count) < pcStep) && ((minY + dY) < maxY))//������� ����� ������� "�� (pcStep * 100)%"
			{
				nOutLplus = 0;//���������� ��������, �������� �� ����� �������
				nOutL = 0;//���������� ��������, ���������� ����� ����� ��������
				dY += increm;
				for (i = 0; i < count; i++)
				{
					y = (float)graphOnGraphs->YValues->operator [](i);
					if (y < (minY + dY))
					{
						nOutL++;//����������� ���������� ��������, ���������� ����� ����� ��������
						if (y >= minY)
							nOutLplus++;//����������� ���������� ��������, �������� �� ����� �������
					}
				}
			}
			minY += dY;

			nOutRplus = 0;//���������� ��������, �������� �� ������ �������
			dY = 0;//
			while ((((float)nOutRplus / count) < pcStep) && (minY < (maxY - dY)))//������� ������ ������� "�� (pcStep * 100)%"
			{
				nOutRplus = 0;//���������� ��������, �������� �� ������ �������
				nOutR = 0;//���������� ��������, ���������� �� ������ ��������
				dY += increm;
				for (i = 0; i < count; i++)
				{
					y = (float)graphOnGraphs->YValues->operator [](i);
					if (y > (maxY - dY))
					{
						nOutR++;//����������� ���������� ��������, ���������� �� ������ ��������
						if (y <= maxY)
							nOutRplus++;//����������� ���������� ��������, �������� �� ������ �������
					}
				}
			}
			maxY -= dY;
		}
		resultPc = floor(((float)((count - nOutR - nOutL) * 100) / count) + 0.5);//�������� ������� ������������ ��������
		if ((znak && (viewPc > ((float)resultPc / 100))) ||
			(!znak && (viewPc < ((float)resultPc / 100))))//���� ���������� ���� ������ �����
		{
			minY = minYback;
			maxY = maxYback;
		}

		intervals = theGist->Tag;//���������� ����������, �� ������� ������������ ��������
		vals = new __int32[intervals + 2];
		average = 0;//������� ��������
		for (i = 0; i < (intervals + 2); i++)
			vals[i] = 0;//�������� vals
		increm = (maxY - minY) / intervals;//��� �������� �������� ���������
		for (i = 0; i < count; i++)//���������� �������� ����������
		{
			y = graphOnGraphs->YValues->operator [](i);//���� �������� ��������� ��� �������
			//���������� �������������� ���������
			if (y < minY)
				vals[0]++;
			else if (y > maxY)
				vals[intervals + 1]++;
			else
			{
				g = 1 + floor((y - minY) / increm);//���������� ��������, �������� ����������� y
				if (g >= (intervals + 1))//��� y = maxY
					g = intervals;
				vals[g]++;
				average += y;
			}
		}
		resultPc = floor(((float)((count - vals[0] - vals[intervals + 1]) * 100) / count) + 0.5);//�������� ������� ������������ ��������
		theGist->ParentChart->Tag = resultPc;//�������� ������� ������������ ��������
		//������ ����������� ������������� ���������
		theGist->Clear();
		i = 0;
		if (vals[0] > 0)
			theGist->AddXY(minY + (increm * (i - 0.5)), vals[0], "", clGreen);//���������� �������������� ��������
		for (i = 1; i < (intervals + 1); i++)
			theGist->AddXY(minY + (increm * (i - 0.5)), vals[i]);//� �������� ���������
		if (vals[intervals + 1] > 0)
			theGist->AddXY(minY + (increm * (i - 0.5)), vals[intervals + 1], "", clGreen);//���������� ������������ ��������

		delete[] vals; vals = NULL;
	}
	return (average / count);
}
//---------------------------------------------------------------------------

void __fastcall TGists::GistsClear(TObject *Sender)
{
	//�������� ������� ��� ��������
	__int32 i;

	for (i = 0; i < 9; i++)
		allCharts[i]->Visible = false;//�������� ��� �����

	freqM->Clear();//������� �����������
	gsAmpls->Clear();//�������� ������������������ �������
	gsRiseTs->Clear();//����� ����������
	gsFallTs->Clear();//����� �����
	gsSynDels->Clear();//������������� ��������
	gsSynTrns->Clear();//����� �������������� ����������
	gsNaAmpls->Clear();//��������� Na-����
	gsNaDurs->Clear();//������������ Na-����
	gsKAmpls->Clear();//��������� �-����
	
	Experiment->RateMS_inTime->Font->Style = TFontStyles();//������ ����� ������
	Experiment->Ampl_gist->Font->Style = TFontStyles();//������ ����� ������
	Experiment->RiseTime_gist->Font->Style = TFontStyles();//������ ����� ������
	Experiment->FallTime_gist->Font->Style = TFontStyles();//������ ����� ������
	Experiment->SynDel_gist->Font->Style = TFontStyles();//������ ����� ������
	Experiment->St0_gist->Font->Style = TFontStyles();//������ ����� ������
	Experiment->NaAmpl_gist->Font->Style = TFontStyles();//������ ����� ������
	Experiment->NaDur_gist->Font->Style = TFontStyles();//������ ����� ������
	Experiment->KAmpl_gist->Font->Style = TFontStyles();//������ ����� ������
	
	Experiment->plottedHists = 0;//�������� ������� ������� ����������
}
//---------------------------------------------------------------------------

void TGists::GistsRefresh(short toReplot)
{
	//�������������� �����������
	/*
	toReplot - ����� �����������, ������� ����� ������������� (-1 - ��� ����)
	*/
	__int32 count;//���������� �������������� ��������
	float average;//������� �������� ���������

	if ((MiniFreq->Visible) && (toReplot < 0))
		RateMiniSigCalc(Graphs->sigAmpls[0]);//����� ��������� ����� ��� �������� ������� gAmplsL

	if ((amplG->Visible) && ((toReplot < 0) || (toReplot == 1)))
	{
		average = GistsCalculation(Graphs->sigAmpls[0], gsAmpls);
		amplG->Title->Text->Text = IntToStr(amplG->Tag) +  "%/" + IntToStr(gsAmpls->Tag) +
			"�. ���������. ����. = " + IntToStr((__int32)average) + " ��";
	}

	if ((riseTG->Visible) && ((toReplot < 0) || (toReplot == 2)))
	{
		average = GistsCalculation(Graphs->gRiseTsL, gsRiseTs);
		riseTG->Title->Text->Text = IntToStr(riseTG->Tag) +  "%/" + IntToStr(gsRiseTs->Tag) +
			"�. ����� ����������. ����. = " + IntToStr((__int32)average) + " ���";
	}

	if ((fallTG->Visible) && ((toReplot < 0) || (toReplot == 3)))
	{
		average = GistsCalculation(Graphs->gFallTsL, gsFallTs);
		fallTG->Title->Text->Text = IntToStr(fallTG->Tag) +  "%/" + IntToStr(gsFallTs->Tag) +
			"�. ����� �����. ����. = " + IntToStr((__int32)average) + " ���";
	}

	if ((NaAmplG->Visible) && ((toReplot < 0) || (toReplot == 4)))
	{
		average = GistsCalculation(Graphs->gNaAmplsL, gsNaAmpls);
		NaAmplG->Title->Text->Text = IntToStr(NaAmplG->Tag) +  "%/" + IntToStr(gsNaAmpls->Tag) +
			"�. Na-���. ����. = " + IntToStr((__int32)average) + " ��";
	}

	if ((NaDurG->Visible) && ((toReplot < 0) || (toReplot == 5)))
	{
		average = GistsCalculation(Graphs->gNaDursL, gsNaDurs);
		NaDurG->Title->Text->Text = IntToStr(NaDurG->Tag) +  "%/" + IntToStr(gsNaDurs->Tag) +
			"�. ������������ Na-����. ����. = " + IntToStr((__int32)average) + " ���";
	}

	if ((KAmplG->Visible) && ((toReplot < 0) || (toReplot == 6)))
	{
		average = GistsCalculation(Graphs->gKAmplsL, gsKAmpls);
		KAmplG->Title->Text->Text = IntToStr(KAmplG->Tag) +  "%/" + IntToStr(gsKAmpls->Tag) +
			"�. ��������� �-����. ����. = " + IntToStr((__int32)average) + " ��";
	}

	if ((synDelayG->Visible) && ((toReplot < 0) || (toReplot == 7)))
	{
		average = GistsCalculation(Graphs->gSynDelsL, gsSynDels);
		synDelayG->Title->Text->Text = IntToStr(synDelayG->Tag) +  "%/" + IntToStr(gsSynDels->Tag) +
			"�. ������. ��������. ����. = " + IntToStr((__int32)average) + " ���";
	}

	if ((synTrnsG->Visible) && ((toReplot < 0) || (toReplot == 8)))
	{
		average = GistsCalculation(Graphs->gSynTrnsL, gsSynTrns);
		synTrnsG->Title->Text->Text = IntToStr(synTrnsG->Tag) +  "%/" + IntToStr(gsSynTrns->Tag) +
			"�. ������. ����������. ����. = " + IntToStr((__int32)average) + " ���";
	}
}
//---------------------------------------------------------------------------

void __fastcall TGists::HistChartMouseDown(TObject *Sender, TMouseButton Button,
	  TShiftState Shift, int X, int Y)
{
	//�������� ���������� �������� �����������
	short i;
	float average;
	TChart *theChart;//��������� �� �������������� ����
	TBarSeries *theHist;//��������� �� �������������� �����������

	theChart = (TChart*)Sender;//�������������� ����
	theHist = (TBarSeries*)theChart->Series[0];//�������������� �����������

	if ((Button == mbLeft) && !Shift.Contains(ssShift))
		theHist->Tag++;//����������� ���������� ��������
	else if ((Button == mbRight) && !Shift.Contains(ssShift))
		theHist->Tag--;//��������� ���������� ��������
	else if ((Button == mbLeft) && Shift.Contains(ssShift))
		theChart->Tag = abs(theChart->Tag) + 5;//����������� ������� ������������ ��������
	else if ((Button == mbRight) && Shift.Contains(ssShift))
		theChart->Tag = (-1) * (abs(theChart->Tag) - 5);//��������� ������� ������������ ��������

	if (theHist->Tag <= 0)
		theHist->Tag = 1;
	if (abs(theChart->Tag) < 10)
		theChart->Tag = 10 * Signt(theChart->Tag);
	else if (abs(theChart->Tag) > 100)
		theChart->Tag = 100 * Signt(theChart->Tag);

	for (i = 0; i < 9; i++)
		if (theChart == allCharts[i])//����� �� ������� ������ �����
			break;

	GistsRefresh(i);//��������� �����������
}
//---------------------------------------------------------------------------

__int32 TGists::Signt(__int32 aVal)
{
	//���� �����
	if (aVal > 0)
		return 1;
	else if (aVal < 0)
		return -1;
	else
		return 0;
}
//---------------------------------------------------------------------------

