//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <process.h>

#pragma hdrstop

#include "MainForm.h"
#include "PredStart.h"
#include "ExperimentNotes.h"
#include "BuildProtocol.h"
#include "CalcParams.h"
#include "MinorChannel.h"
#include "RenamOverr.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TPStart *PStart;

IADCDevice *preS_pADC;//��������� �� ��������� ��������
IADCUtility *preS_pUtil;//��������� �� ��������� ��������
__int32 timeIncrement;//��� ��������� ��������� "������� ����" (����� � �������������)
float multipler;//��������� ��� ��������� ��������� �����
short typeOfExp;//��� ������������

//---------------------------------------------------------------------------
__fastcall TPStart::TPStart(TComponent* Owner) : TForm(Owner)
{
	//������ ���������������� ������

	preRecThread = new TRecordThread(true);//������ ����� ������������ (����������������)

	//PStart->closeWin->Tag - ��������� ����������� ����� � �������� ������ ��������-��������
	//PStart->GetASignal->Tag - ����� ����� ������ �����: ������ ������� ��� ��������
	//Graphs->CrntSig->Tag - ������� ���������� �������� (��������� ��� ���������� � ������� �1, 2 � 3)
	//PStart->BackTime->Tag - ������� ���������� �������� � ������ �4
	//PStart->PausSpontan->Tag - ���� ���������� ������ ���������� � ������ 4
	//Graphs->sigNumUpDwn->Tag - ������������ ���������� �������� (��������������� ����� �������)
	//Experiment->DiscontinWrt->Tag - ���������� ������ � drr-���� ��� ������� ������ ������
	//ExpNotes->addUMark->Tag - ���������� ������� �� ���� ������������ (����� ������������ �� �������)
	//ProtoBuild->Periods->Tag - ���������� ������ ����� ���������� ����������

	timeIncrement = 5;//��� ��������� ��������� "������� ����" (����� � �������������
	multipler = 1;//��������� ��� ��������� ��������� �����
}
//---------------------------------------------------------------------------

void TPStart::PredStartWindow(IADCUtility *p0, IADCDevice *p1, short expType)
{
	/*
	expType - ��� ������������
	p0 - ��������� �� ��������� ��������
	p1 - ��������� �� ��������� ��������
	*/
	__int32 i;

	preS_pUtil = p0; p0 = NULL;//����� ������
	preS_pADC = p1; p1 = NULL;//����� ������

	//������������� ����������� ��������� ������
	GetASignal->Enabled = true;//������ ������� �������
	StartRec->Enabled = true;//������ ������� �����
	ExpInfo->Enabled = true;//������ ���������� �� ������������

	CloseWin->Caption = "�������";//������ "�������" ������
	CloseWin->Tag = 1;//������ ������� ������

	//������� ��� ����� � �������
	preTLine->Clear();
	postTLine->Clear();
	porogLine->Clear();
	for (i = 0; i < maxChannels; i++)
		exmplChannls[i]->Clear();

	ExpNotes->usersNotes->Clear();//�������� ������ � ��������
	typeOfExp = expType;//�������� ��� ������������ � ���������� ����������

	if (typeOfExp == 1)//���������� �������
	{
		PStart->Caption = "���������: ����������";
		PreTSel->Position = PreTSel->Min;
		PostTSel->Position = PostTSel->Min;
	}
	else if (typeOfExp == 2)//��������� (���������������)
	{
		PStart->Caption = "���������: ���������-���������������";
		PreTime->Text = "0";//��������� � 0 ����� preTime, ����� �� ������
	}
	else if (typeOfExp == 3)//��������� (������������)
		PStart->Caption = "���������: ���������-������������";
	else if (typeOfExp == 4)//���������+����������
		PStart->Caption = "���������: ��������� + ����������";
	else//������ ���� ������������
	{
		Experiment->DevEvents->Text = "������ ���� ������������";
		onCloseWin(this);//���� ��� � �� ���������, ������� ������� ��� ����������
	}

	Graphs->Show();//������ ������� ���� ��������
	PStart->Show();//��������� ���� ����������
	Experiment->DevEvents->Text = "OK";
}
//---------------------------------------------------------------------------

void __fastcall TPStart::StartRecClick(TObject *Sender)
{
	//����� ����� ������: 1 - ��������� ������ ��� 2 - ������ ��������
	__int32 i,
			lastPnt;//����� ��������� ����� �� �������
	AnsiString theFileName,//������ ��� �����
			   theDir,//��������� ����������
			   anpth;//��������� ����
	TDateTime CurrentDateTime;//������� ���� � �����

	GetASignal->Tag = 2;//������ ������ ��������

	theFileName = Experiment->FilePathName->Text;//���������� ����� ��� �� ������ filePathName �� ������� �����
	theDir = ExtractFileDir(theFileName);

	//���� ������ ������ ��� ���������� �� ����������, ���������� ��������� ��� (�� ���� � �������)
	if ((theFileName.IsEmpty()) || (!DirectoryExists(theDir)))
	{
		for (i = 0; i < 15; i++)
		{
			anpth = char(i + 67);//char(67) = "C" - ����� ���������� �����
			anpth += ":\\";
			if (GetDriveType(anpth.c_str()) == DRIVE_FIXED)//
			{
				//anpth += "WINDOWS";//���������� � ����� ��������
				if (DirectoryExists(anpth))
				{
					anpth.Delete(4, anpth.Length());
					break;
				}
			}
		}
		CurrentDateTime = Now();//������� ���� � �����
		theFileName = anpth + "Signal_" + CurrentDateTime.FormatString("yyyy-mm-dd_hh-nn-ss") + "." + Experiment->defltExtPraDrr;
		Experiment->FilePathName->Text = theFileName;//���������� ��� ����� �� ������� �����
	}

	Graphs->SaveDlg->FileName = theFileName;//�������� ��� �����
	Experiment->DiscontinWrt->Tag = 0;//��������� ����� ������� ������: 0 - ������� ������ ��� 1 - ��������
	Graphs->plotFrom = 0;//���������� �������� � ����� ������� �������

	//���� ������ ���� ����������, ��������� ������� ������
	if (FileExists(Graphs->SaveDlg->FileName.c_str()))
	{
		ChoosDlg->setsForFileChs();//����������� ���� ������� ��� ������� � ������ �����
		i = ChoosDlg->ShowModal();

		if (i == mrOk)//������� ������ ����
			Experiment->SaveToClick(this);
		else if (i == mrYes)//�������� ������ � ������������ ����
		{
			Experiment->DiscontinWrt->Tag = 1;//������� �������� � ������������ ����

			lastPnt = Graphs->sigAmpls[0]->XValues->Count() - 1;
			if (lastPnt >= 0)
				Graphs->plotFrom = Graphs->sigAmpls[0]->XValues->operator [](lastPnt) + 1;//���������� �������� � ����� ������� �������
		}
		else if (i == mrIgnore)//��������� � �������
			Experiment->DiscontinWrt->Tag = 0;//������� �������� ������������ ���� (������� ��������)
		//else if (i == mrCancel)//�������� ������ ��������
	}

	if (i != mrCancel)//���� ������� �� ���� ��������
		StartOfRecordThread(this);//������ ������� ������������� ������ ��� ������ (��������)
}
//---------------------------------------------------------------------------

void __fastcall TPStart::GetASignalClick(TObject *Sender)
{
	//����� ����� ������: 1 - ��������� ������ ��� 2 - ������ ��������
	GetASignal->Tag = 1;//������� ��������� ������� �������
	Experiment->DiscontinWrt->Tag = 1;//������� ��������� � �����

	StartOfRecordThread(this);//������ ������� ������������� ������ ��� ������ (��������)
}
//---------------------------------------------------------------------------

void __fastcall TPStart::StartOfRecordThread(TObject *Sender)
{
	//����� ������ RecordThread
	__int32 i;
	AnsiString singlMark;//����� ������ �����
	bool mOnChn[maxChannels];//�������� ����� � �������

	if (!Experiment->InitDevice())//������ �������������
		return;

	Experiment->DevEvents->Text = "OK";

	//����� ������� ������ ������������ ������� ������ � ��������� ��������
	Graphs->ClearMemor(this);//�������� ���� ������ ������ �� ������
	Graphs->csSpecPoint->Clear();//�������� ����-�����

	//���������� � ������� ������ ������
	ExpNotes->nmInRec = ExpNotes->addUMark->Tag;//���������� ����� ���������� � ���������� ������� ����� ������ (�������� � ������������ ����)
	ExpNotes->npNewRec = Graphs->sigAmpls[0]->Count();//���������� ��������, ���������� � ���������� ������� ����� ������ (�������� � ������������ ����)
	if ((GetASignal->Tag == 2) && (Experiment->DiscontinWrt->Tag == 1))//���� ������� �������� � ������������ ����
	{
		//����� ������ ������� �� ������ ������ � ����� �����
		singlMark = "��";//����� ������ ����� //singlMark += IntToStr(nBlocks);//����� �����
		for (i = 0; i < maxChannels; i++)//����������� ����� �� ���� �������
			mOnChn[i] = true;//����������� ����� �� ���� �������
		ExpNotes->AddMarker(singlMark, ExpNotes->npNewRec, &mOnChn[0]);
	}

	preRecThread->AssignVar(typeOfExp, preS_pUtil, preS_pADC);
	CloseWin->SetFocus();//������� ����� �� ������ ��������� ������������
	preRecThread->Resume();//��������� ���������������� ����� preRecThread
}
//---------------------------------------------------------------------------

void TPStart::PlotStandardSignal(double *sData, __int32 sRecLen, float sEffDT, __int32 sIndBgn)
{
	//������ ������ ��������� ������
	/*
	sData - ������ � ���������
	sRecLen - ����� ������� (�������)
	sEffDT - ����������� ����� ������������� = discrT * chanls
	sIndBgn - ��������� ������ �������
	*/

	__int32 i,
			ns1,//������ �������� �����
			ns2;//��������� �������� �����

	ns1 = Graphs->gRF[0];//������ �������� �����
	ns2 = Graphs->gRF[1];//��������� �������� �����
	StandardSgnl->AutoRepaint = false;//��������� �������������� ��� ��������� �������� ���������

	for (i = 0; i < maxChannels; i++)//������ ��������� ������
		exmplChannls[i]->Clear();//�������� ������ ������
	for (i = ns1; i < ns2; i++)//������ ��������� ������
		exmplChannls[i]->AddArray(&sData[sIndBgn + ((i - ns1) * sRecLen)], (sRecLen - sIndBgn) - 1);//��������� ����� �����

	StandardSgnl->BottomAxis->SetMinMax(0, double((sRecLen - sIndBgn) * sEffDT) / 1000);//������������� ������� �������������� ���
	StandardSgnl->AutoRepaint = true;//���������� ��������������, ����� ������ ���������
	StandardSgnl->Repaint();//Refresh();//��������� ����

	sData = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TPStart::AmpPorogSelClick(TObject *Sender, TUDBtnType Button)
{
	Porog->Text = IntToStr(AmpPorogSel->Position);
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PreTSelClick(TObject *Sender, TUDBtnType Button)
{
	if (PreTSel->Position >= PostTSel->Position)
		PreTSel->Position = PostTSel->Position - PostTSel->Increment;

	PreTime->Text = IntToStr((__int32)(PreTSel->Position * multipler));
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PostTSelClick(TObject *Sender, TUDBtnType Button)
{
	if (PostTSel->Position <= PreTSel->Position)
		PostTSel->Position = PreTSel->Position + PreTSel->Increment;

	PostTime->Text = IntToStr((__int32)(PostTSel->Position * multipler));
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PreTimeChange(TObject *Sender)
{
	//�������������� ���-�����
	__int32 i,
			preTnow, postTnow;
	float loc_max,//������������ �������� �� �������
		  loc_min;//����������� �������� �� �������

	loc_max = exmplChannls[0]->MaxYValue();//������������ �������� �� �������
	loc_min = exmplChannls[0]->MinYValue();//������������ �������� �� �������
	for (i = 1; i < maxChannels; i++)
	{
		if (exmplChannls[i]->MaxYValue() > loc_max)
			loc_max = exmplChannls[i]->MaxYValue();
		if (exmplChannls[i]->MinYValue() < loc_min)
			loc_min = exmplChannls[i]->MinYValue();
	}
	
	preTnow = StrToInt(PreTime->Text);
	postTnow = StrToInt(PostTime->Text);

	if (preTnow >= postTnow)
		preTnow = postTnow - PostTSel->Increment;

	PreTSel->Position = (short)(((float)preTnow) / multipler);

	preTLine->Clear();
	preTLine->AddXY((((float)(PreTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_min);
	preTLine->AddXY((((float)(PreTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_max);
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PostTimeChange(TObject *Sender)
{
    //������������� ����-�����
	__int32 i,
			preTnow, postTnow;
	float loc_max,//������������ �������� �� �������
		  loc_min;//����������� �������� �� �������

	loc_max = exmplChannls[0]->MaxYValue();//������������ �������� �� �������
	loc_min = exmplChannls[0]->MinYValue();//������������ �������� �� �������
	for (i = 1; i < maxChannels; i++)
	{
		if (exmplChannls[i]->MaxYValue() > loc_max)
			loc_max = exmplChannls[i]->MaxYValue();
		if (exmplChannls[i]->MinYValue() < loc_min)
			loc_min = exmplChannls[i]->MinYValue();
	}

	postTnow = StrToInt(PostTime->Text);
	preTnow = StrToInt(PreTime->Text);

	if (postTnow <= preTnow)
		postTnow = preTnow + PreTSel->Increment;

	PostTSel->Position = short((float)postTnow / multipler);

	postTLine->Clear();
	postTLine->AddXY((((float)(PostTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_min);
	postTLine->AddXY((((float)(PostTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_max);
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PorogChange(TObject *Sender)
{                                                   
	//������ ��������� ����� ���������� ���������� �������� ������������� ��������
	__int32 i,
			porogSet;
	float loc_max,//������������ �������� �� �������
		  loc_min;//����������� �������� �� �������

	loc_max = exmplChannls[0]->MaxYValue();//������������ �������� �� �������
	loc_min = exmplChannls[0]->MinYValue();//������������ �������� �� �������
	for (i = 1; i < maxChannels; i++)
	{
		if (exmplChannls[i]->Count() > 0)
		{
			if (exmplChannls[i]->MaxYValue() > loc_max)//������ ����� ��������
				loc_max = exmplChannls[i]->MaxYValue();//����� ��������
			if (exmplChannls[i]->MinYValue() < loc_min)//������ ����� �������
				loc_min = exmplChannls[i]->MinYValue();//����� �������
		}
	}
	porogSet = StrToInt(Porog->Text);//��������� �����

	if (porogSet < AmpPorogSel->Min)//���������� � ������ ��������
		porogSet = AmpPorogSel->Min;//��������� �����
	if (porogSet > AmpPorogSel->Max)//���������� � ������� ��������
		porogSet = AmpPorogSel->Max;//��������� �����

	AmpPorogSel->Position = porogSet;//������� ��������� ����� ������

	porogLine->Clear();//�������� ����� ������
	preTLine->Clear();//�������� ������������ �����
	postTLine->Clear();//�������� ������������ �����
	
	porogLine->AddXY(0, (AmpPorogSel->Position));//������ ����� ������
	porogLine->AddXY(((float(PostTSel->Max) * multipler) - timeOfDrawBgn) / 1000, AmpPorogSel->Position);//������ ����� ������

	preTLine->AddXY(((float(PreTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_min);//������ ������������ �����
	preTLine->AddXY(((float(PreTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_max);//������ ������������ �����

	postTLine->AddXY(((float(PostTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_min);//������ ������������ �����
	postTLine->AddXY(((float(PostTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_max);//������ ������������ �����
}
//---------------------------------------------------------------------------

void TPStart::RefreshLines(float rlEffDT)
{
	//���������� �����, ����������� ��������� ���� ��������
	/*
	rlEffDT - ����������� ����� ������������� = discrT * chanls
	*/
	__int32 i;

	//��������� multipler ����� ��� ����������� ������ ��������� ��������� ��������� position � UpDown
	
	PreTSel->Min = 0;
	PostTSel->Min = 0;
	if ((PStart->exmplChannls[0]->Count() * rlEffDT) > 32000)
	{
		PreTSel->Max = 32000;//� �������������
		PostTSel->Max = 32000;//� �������������
		multipler = (float)(PStart->exmplChannls[0]->Count() * rlEffDT) / 32000;//��������� ��� ��������� ��������� �����
	}
	else
	{
		PreTSel->Max = PStart->exmplChannls[0]->Count() * rlEffDT;//� �������������
		PostTSel->Max = PStart->exmplChannls[0]->Count() * rlEffDT;//� �������������
		multipler = 1;//��������� ��� ��������� ��������� �����
	}
	PreTSel->Increment = timeIncrement;
	PostTSel->Increment = timeIncrement;

	PorogChange(this);
}
//---------------------------------------------------------------------------

void __fastcall TPStart::ExpInfoClick(TObject *Sender)
{
	//�������� ���� ��� ������ ���������� �� ������������

	//�������� �������� ����
	ExpNotes->mainLinesLbl->Caption = "����� �������� �� ������������";
	ExpNotes->usersNotes->Visible = true;//���� ��� ���������������� ������
	ExpNotes->addUMark->Visible = true;//���� ����� ������� �� ���� ������������
	ExpNotes->addMarkLbl->Visible = true;//�������-�����
	ExpNotes->PIDates->Visible = false;//���� ������ � �����

	ExpNotes->Show();
}
//---------------------------------------------------------------------------

void __fastcall TPStart::SignalLenChange(TObject *Sender)
{
	//��������� ����� �� ������� ������������� ����� ���������� �������
	__int32 sLen;//����� ���������� �������

	sLen = StrToInt(SignalLen->Text);//����� ���������� �������
	if (Experiment->DiscontinWrt->Checked)//������� ������ ������ �� ����
	{
		//ProtoBuild->Periods->Tag - ���������� ������ ����� ���������� ����������
		if ((ProtoBuild->Periods->Tag - sLen) < minFreeTime)//������� �������� �������� ����� ���������
		{
			SignalLen->Text = IntToStr(sLen - 1);//��������� ����� �������
			return;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TPStart::LenSpontChange(TObject *Sender)
{
	//��������� ����� �� ������� ������������� ����� ����������� �������

	__int32 backTimLen,//����� ����� ��� ����������� �������
			sLen;//������ ����� ������������� ������� (������������)

	sLen = StrToInt(LenSpont->Text);//����� ������� � �������������
	backTimLen = StrToInt(BackTime->Text);//����� ����� ��� ����������� �������
	//������ ����������� �� ����� ������������� �������
	if (sLen > limitSigLen)//����� ������� ������ ���� ������
	{
		LenSpont->Text = IntToStr(limitSigLen);//����������� ����� �������
		return;
	}
	if (sLen < 1)//���� ������� ������� ��� ������������� �����
	{
		LenSpont->Text = "1";//��������� ����������� �����
		return;
	}
	if (sLen < backTimLen)//���� ����� ������� ������ ����� �����
	{
		LenSpont->Text = IntToStr(backTimLen + 1);//������ ����� ������� ������ ������� �����
		return;
	}
}
//---------------------------------------------------------------------------

void __fastcall TPStart::BackTimeChange(TObject *Sender)
{
	//��������� ����� �� ������� ������������� ����� �����

	__int32 spntBackLen,//����� ����� ��� ����������� �������
			sLen;//������ ����� ������������� ������� (������������)

	spntBackLen = StrToInt(BackTime->Text);
	sLen = StrToInt(LenSpont->Text);//������ ����� �������
	if (spntBackLen < 1)//���� ������� ������� ��� ������������� �����
	{
		BackTime->Text = "1";//��������� ����������� �����
		return;
	}
	if (spntBackLen > sLen)//���� ����� ����� ��������� ����� �������
	{
		BackTime->Text = IntToStr(sLen - 1);//������ ����� ����� �� ���� ������������ ������ ����� �������
		return;
	}
}
//---------------------------------------------------------------------------

void __fastcall TPStart::InvertClick(TObject *Sender)
{
	Graphs->ReplotExamplRecalc();//�������� ������� ����������� ������� � ��������������� ����
}
//---------------------------------------------------------------------------

void __fastcall TPStart::OffLineInvert(TObject *Sender)
{
	//�������������� � ������ "�������������"
	postCompInvert = 1 - (2 * (short(Invert->Checked)));//����� ��������� ����� ��������������
	TimeBarChange(this);//������ ������ ������
}
//---------------------------------------------------------------------------

void __fastcall TPStart::ReCalClick(TObject * Sender)
{
	PStart->ModalResult = mrOk;//������ ������ ��������� ��� ������ ������ ��������
}
//---------------------------------------------------------------------------

void __fastcall TPStart::CloseWinClick(TObject *Sender)
{
	if (CloseWin->Tag == 1)
		PStart->Close();//��������� �����
	else
		CloseWin->Tag = 1;//������ ������� ������
}
//---------------------------------------------------------------------------

void __fastcall TPStart::NextBlockClick(TObject *Sender)
{
	NextBlock->Tag = 1;//��� ������� �� ������ "��. ����" ���������� ������� � ���������� ����� ���������
}
//---------------------------------------------------------------------------

void __fastcall TPStart::onCloseWin(TObject *Sender)
{
	//��������� ����
	if (preS_pUtil != NULL)
		preS_pUtil = NULL;
	else if (preS_pADC != NULL)
		preS_pADC = NULL;

    //��������� ���������
	Experiment->progPrms[2] = StrToInt(PStart->LenSpont->Text);//2 - ����� ����������� �������
	Experiment->progPrms[3] = StrToInt(PStart->Porog->Text);//3 - ����� ��������
	Experiment->progPrms[4] = StrToInt(PStart->BackTime->Text);//4 - ����� �����
	Experiment->progPrms[5] = StrToInt(PStart->SignalLen->Text);//5 - ����� �������
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PausSpontanClick(TObject *Sender)
{
	//������������ ��� ����������� ������ ���������� �������� � ������ �4

	//���� pausMini->Tag == 0, �� ���������� ������ ���������� ��������
	//���� pausMini->Tag == 1, �� ����������������

	if (PausSpontan->Tag == 0)
	{
		PausSpontan->Tag = 1;//������ �� �����
		PausSpontan->Caption = "����� ����";
	}
	else//pausMini->Tag == 1
	{
		PausSpontan->Tag = 0;//���������� ������
		PausSpontan->Caption = "����� ����";
    }
}
//---------------------------------------------------------------------------

void __fastcall TPStart::StandardSgnlMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
	//��� ������� ������ ������ ����� ����������, ����� ����� ����� � �����
	//�������, � ������ � � ��� �����
	__int32 chMDexpType,//��� ������������
			b,//�-���������� ����� ���-������ � ������ ������
			c;//�-���������� ����� ����-������ � ������ ������
	double xX;

	chMDexpType = Graphs->ReCalculat->Tag;//��� ������������

	if (Button == 1)//���� ������ ������ ������ �����
	{

		b = preTLine->CalcXPosValue(preTLine->XValues->operator [](0)) - X;
		b = b * b;//������ abs

		c = postTLine->CalcXPosValue(postTLine->XValues->operator [](0)) - X;
		c = c * c;//������ abs

		if ((b < c) && (chMDexpType == 3))
		{
			xX = preTLine->XScreenToValue(X);//����� ����� ����� Series
			PreTime->Text = IntToStr((__int32)(floor(xX * 1000) + timeOfDrawBgn));
		}
		else
		{
			xX = postTLine->XScreenToValue(X);//����� ����� ����� Series
			PostTime->Text = IntToStr((__int32)(floor(xX * 1000) + timeOfDrawBgn));
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TPStart::CheckForKeyPStartEdits(TObject *Sender, char &Key)
{
	//�������� ������������ �������� �������, ���������������� ������� �������
	//��� ��������� ����� ����� �������� ������

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//�������� ������������ �������� ��������
		Key = '\0';//�������� ������������ ������
}
//---------------------------------------------------------------------------

void __fastcall TPStart::CheckForKeyPPorogEdit(TObject *Sender, char &Key)
{
	//�������� ������������ �������� �������, ���������������� ������� �������
	//��� ���� ������������ ������ ��� ��������

	if (((Key < '0') || (Key > '9')) && (Key != '\b') && (Key != '-'))//�������� ������������ �������� ��������
		Key = '\0';//�������� ������������ ������
}
//---------------------------------------------------------------------------

void TPStart::SetControlVis(short expType)
{
	//������������� ��������� ��������� � ����������� �� ���� ������������
	/*
	expType - ��� ������������
	*/

	if (expType == 1)//���������� ������
	{
		SignalLen->Visible = false;//������ ����� ���������� �������
		SLenLbl1->Visible = false;//����� ���������� ������ (����� 1)
		SLenLbl2->Visible = false;//����� ���������� ������ (����� 2)
		SLenLbl3->Visible = false;//����� ���������� ������ (����� 3)

		LenSpont->Visible = true;//����� ����� ����������� �������
		LenSpntLbl1->Visible = true;//����� ����������� ������� (����� 1)
		LenSpntLbl2->Visible = true;//����� ����������� ������� (����� 2)
		LenSpntLbl3->Visible = true;//����� ����������� ������� (����� 3)

		BackTime->Visible = true;//����� ������� �����
		BckTmLbl1->Visible = true;//������� ����� (����� 1)
		BckTmLbl2->Visible = true;//������� ����� (����� 2)
		BckTmLbl3->Visible = true;//������� ����� (����� 3)

		PreTime->Visible = false;//����� ���-������� (������)
		PreTSel->Visible = false;//����� ���-������� (������)
		PreTimeLbl->Visible = false;//���-����� (�����)

		PostTime->Visible = false;//����� ����-������� (������)
		PostTSel->Visible = false;//����� ����-������� (������)
		PostTimeLbl->Visible = false;//����-������� (�����)

		PausSpontan->Visible = false;//������ ����� ����� ���������� �������� � ������ �4
	}
	else if ((expType == 2) || (expType == 3))//��������� ������� (������- ��� ������������)
	{
		SignalLen->Visible = true;//������ ����� ���������� �������
		SLenLbl1->Visible = true;//����� ���������� ������ (����� 1)
		SLenLbl2->Visible = true;//����� ���������� ������ (����� 2)
		SLenLbl3->Visible = true;//����� ���������� ������ (����� 3)

		LenSpont->Visible = false;//����� ����� ����������� �������
		LenSpntLbl1->Visible = false;//����� ����������� ������� (����� 1)
		LenSpntLbl2->Visible = false;//����� ����������� ������� (����� 2)
		LenSpntLbl3->Visible = false;//����� ����������� ������� (����� 3)

		BackTime->Visible = false;//����� ������� �����
		BckTmLbl1->Visible = false;//������� ����� (����� 1)
		BckTmLbl2->Visible = false;//������� ����� (����� 2)
		BckTmLbl3->Visible = false;//������� ����� (����� 3)

		PreTime->Visible = (expType == 3);//����� ���-������� (������)
		PreTSel->Visible = (expType == 3);//����� ���-������� (������)
		PreTimeLbl->Visible = (expType == 3);//���-����� (�����)

		PostTime->Visible = true;//����� ����-������� (������)
		PostTSel->Visible = true;//����� ����-������� (������)
		PostTimeLbl->Visible = true;//����-������� (�����)

		PausSpontan->Visible = false;//������ ����� ����� ���������� �������� � ������ �4
	}
	else if (expType == 4)//��������� + ����������
	{
        SignalLen->Visible = true;//������ ����� ���������� �������
		SLenLbl1->Visible = true;//����� ���������� ������ (����� 1)
		SLenLbl2->Visible = true;//����� ���������� ������ (����� 2)
		SLenLbl3->Visible = true;//����� ���������� ������ (����� 3)

		LenSpont->Visible = true;//����� ����� ����������� �������
		LenSpntLbl1->Visible = true;//����� ����������� ������� (����� 1)
		LenSpntLbl2->Visible = true;//����� ����������� ������� (����� 2)
		LenSpntLbl3->Visible = true;//����� ����������� ������� (����� 3)

		BackTime->Visible = true;//����� ������� �����
		BckTmLbl1->Visible = true;//������� ����� (����� 1)
		BckTmLbl2->Visible = true;//������� ����� (����� 2)
		BckTmLbl3->Visible = true;//������� ����� (����� 3)

		PreTime->Visible = false;//����� ���-������� (������)
		PreTSel->Visible = false;//����� ���-������� (������)
		PreTimeLbl->Visible = false;//���-����� (�����)

		PostTime->Visible = true;//����� ����-������� (������)
		PostTSel->Visible = true;//����� ����-������� (������)
		PostTimeLbl->Visible = true;//����-������� (�����)

		PausSpontan->Visible = true;//������ ������������ ����� ���������� ��������
	}
	else
    	Experiment->DevEvents->Text = "������ ���� ������������";//��������� �� ������
}
//---------------------------------------------------------------------------

__int32 TPStart::winDraw(__int32 samplPerCh)
{
	//�������� ���� ���������� ��� ��������� ����� ����������� �������
	/*
	samplPerCh - ���������� �������� �� ���� �����
	*/
	__int32 i;

	//������������� ����������� ��������� ������
	StartRec->Enabled = true;//������ ������
	CloseWin->Caption = "�������";//������ "�������" ������
	CloseWin->Tag = 1;//������ ������� ������

	//������� ��� ����� � �������
	preTLine->Clear();
	postTLine->Clear();
	porogLine->Clear();
	ExpNotes->usersNotes->Clear();//�������� ������ � ��������

	PStart->Caption = "�������������";//����� ������
	SetControlVis(1);//������ �������� ������ ������ ���������� � ������� ��������

	GetASignal->Visible = false;//������ ������ �������
	ExpInfo->Visible = false;//������ ������ �������
	StartRec->OnClick = ReCalClick;//������ ������� �� �������
	postCompInvert = 1 - (2 * (short(Invert->Checked)));//������� ��������� ����� ��������������
	Invert->OnClick = OffLineInvert;//��������� ������� �� ����� ����������

	TimeScaleBar->Max = 5e4;//������������ ����� ���� (������������)
	TimeScaleBar->Min = 500;//����������� ����� ���� (������������)
	TimeScaleBar->PageSize = 50;//��� ��������� ����� ���� (������������)
	TimeScaleBar->Position = 500;//��������� ����� ���� (������������)
	TimeScaleBar->Visible = true;//������ ������� �������
	TimeBar->Max = (samplPerCh * Graphs->effDT * 1e-3) - TimeScaleBar->Position;//������������ ��������� ����� ��������� (������������)
	TimeBar->Min = 0;//����������� ��������� ����� ��������� (������������)
	TimeBar->PageSize = 1e3;//��� ��������� ����� ���� (������������)
	TimeBar->Position = 0;//��������� ����� ��������� (������������)
	TimeBar->Visible = true;//������ ������� �������

	PostTSel->Max = 32000;//����� ���� �������
	PreTSel->Position = 0;//������� ������������ �����
	PostTSel->Position = 0;//������� ������������ �����
	multipler = (float)(exmplChannls[0]->Count() * Graphs->effDT) / 32000;//��������� ��� ��������� ��������� �����

	AmpPorogSel->Max = short(Experiment->maxADCAmp * Graphs->sampl2mV);//�����������
	AmpPorogSel->Min = (-1) * AmpPorogSel->Max;//�����������
	AmpPorogSel->Increment = (AmpPorogSel->Max / 1000) + (__int32(AmpPorogSel->Max <= 1000));//����� �������� �� 1000 �����

	TimeBarChange(this);//��������� ���������� �������

	Graphs->Show();//������ ������� ���� ��������
	i = PStart->ShowModal();//��������� ���� ����������

	GetASignal->Visible = true;//������ ������ �������
	ExpInfo->Visible = true;//������ ������ �������
	StartRec->OnClick = StartRecClick;//��������������� ������� �� �������
	Invert->OnClick = NULL;//��������� ������� �� ����� ����������

	TimeScaleBar->Visible = false;
	TimeBar->Visible = false;

	return i;//���������� ��� ������� ("������" ��� "�������")
}
//---------------------------------------------------------------------------

void __fastcall TPStart::TimeBarChange(TObject *Sender)
{
	//������� �� ��������� �������� ��� �������

	__int32 i, j,
			startInd,//��������� ������
			windowLen;//����� ���� ��������� (�������)
	float eff_dt,//����������� ����� ������������� = discrT * chanls (������������)
		  sampl2mV;//����������� ��������� �������� � �����������
	double *grafik;//������ � ������ ������

	eff_dt = Graphs->effDT * 1e-3;//����������� ����� ������������� = discrT * chanls (������������)
	startInd = floor(TimeBar->Position / eff_dt);//��������� ������
	windowLen = floor(TimeScaleBar->Position / eff_dt);//����� ���� �������� (�������)
	sampl2mV = Graphs->sampl2mV;//����������� ��������� �������� � �����������

	grafik = new double[windowLen * Graphs->chanls];//������ � ������ ������

	for (i = 0; i < Graphs->chanls; i++)//������ ��������� ������
		for (j = 0; j < windowLen; j++)
			grafik[(i * windowLen) + j] = (postCompInvert * continRec[i][startInd + j]) * sampl2mV;
	//PlotStandardSignal(double *sData, __int32 sRecLen, float sEffDT, __int32 sIndBgn)
	PlotStandardSignal(grafik, windowLen, Graphs->effDT, 0);
	multipler = (float(exmplChannls[0]->Count() * Graphs->effDT)) / 32000;//��������� ��� ��������� ��������� �����

	PorogChange(this);//���������� �����
	delete[] grafik; grafik = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TPStart::FormActivate(TObject *Sender)
{
	//����� ��������� �������� ���� ���������� ��������� ���� ��������
	//���� ����������� ���� �������� SetsElph.ini, ������ �� ���� ������
	PStart->LenSpont->Text = IntToStr(Experiment->progPrms[2]);//����� ����������� �������
	PStart->Porog->Text = IntToStr(Experiment->progPrms[3]);//�����
	PStart->BackTime->Text = IntToStr(Experiment->progPrms[4]);//����� �����
	PStart->SignalLen->Text = IntToStr(Experiment->progPrms[5]);//����� ���������� �������
	PStart->Top = Experiment->progPrms[13];//���� ����
	PStart->Left = Experiment->progPrms[14];//����� ���� ����
	PStart->OnActivate = NULL;//��������� ������������ ������� ����� �� "���������"
}
//---------------------------------------------------------------------------

