//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>

#pragma hdrstop

#include "MainForm.h"
#include "BuildProtocol.h"
#include "PredStart.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProtoBuild *ProtoBuild;

//---------------------------------------------------------------------------
__fastcall SPPanel::SPPanel(TComponent* Owner) : TPanel(Owner)
{
}

//---------------------------------------------------------------------------
__fastcall TProtoBuild::TProtoBuild(TComponent* Owner) : TForm(Owner)
{
	//���� ����������: 1 - ��� ���������� (1 - ����������, 2 - �����, 3 - ����)
	//2 - ���-�� �������� ��� ���������� � �����, 3 - ������������ ����� ��� ������ ����������

	//InstructBox->ComponentCount - ���������� ������ (�����������)


	//ProtoBuild->LoadProtocolClick(this);


	//ProtoBuild->Width = InstructBox->Left + InstructBox->Width;//��������� ������ ����
	//ProtoBuild->Periods->Tag - ���������� ������ ����� ���������� ����������
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::StimBlockClick(TObject *Sender)
{
	//������� ����� ����������
	__int32 stimPrd,
			stimNums;

	StimsNum->Visible = !StimsNum->Visible;
	Periods->Visible = !Periods->Visible;
	Label1->Visible = !Label1->Visible;

	//������������ ������
	Silence->Enabled = false;
	Repeat->Enabled = false;
	DelLast->Enabled = false;
	if (!StimsNum->Visible)
	{
		//��������� �������� �� ������������
		stimPrd = StrToInt(Periods->Text);
		if (stimPrd < 5)//������ ������ ������� ���������� = 5 ��
		{
			stimPrd = 5;
			Periods->Text = IntToStr(stimPrd);
		}
		if (stimPrd > 1e7)//������� ������ ������� ���������� = 1e7 �� (��� 10000 �)
		{
			stimPrd = 1e7;
			Periods->Text = IntToStr(stimPrd);
		}

		stimNums = StrToInt(StimsNum->Text);
		if (stimNums < 1)
		{
			stimNums = 1;
			StimsNum->Text = IntToStr(stimNums);
		}

		StimCreat(stimNums, stimPrd);//������� � �������� ���� ����������

		//��������������� ������
		Silence->Enabled = true;
		Repeat->Enabled = true;
		DelLast->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void TProtoBuild::StimCreat(__int32 stimNums, __int32 stimPrd)
{
	//������� � �������� ���� ����������
	/*
	stimNums - ���������� ��������
	stimPrd - ������ ���������� ��������
	*/

	SPPanel *panel,
			*leftPanel;

	panel = new SPPanel(InstructBox);//������ ����� ���� (����������)
	panel->Parent = InstructBox;//��������� ��������
	panel->Caption = "����: " + IntToStr(stimNums) + "(���), " + IntToStr(stimPrd) + "(��)";

	if (InstructBox->ComponentCount > 1)
	{
		leftPanel = (SPPanel*)InstructBox->Components[InstructBox->ComponentCount - 2];//���������� ����
		panel->Left = leftPanel->Left + leftPanel->Width;
	}
	else
		panel->Left = 0;
		
	panel->Width = 10 + 5 * panel->Caption.Length();
	panel->Visible = true;
	panel->iType = 1;//��� ���������� (����������)
	panel->scCount = stimNums;//���������� ���������
	panel->period = stimPrd;//������ ���������� ���������
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::SilenceClick(TObject *Sender)
{
	//����� ������ ��������
	//����������, ����� ����� ���� ����� 100 ��, �.�. ������ �����
	// ������������ ����� ������ ��������

	__int32 prd;

	Periods->Visible = !Periods->Visible;
	Label2->Visible = !Label2->Visible;

	//������������ ������
	StimBlock->Enabled = false;
	Repeat->Enabled = false;
	DelLast->Enabled = false;
	if (!Periods->Visible)
	{
		//��������� �������� �� ������������
		prd = StrToInt(Periods->Text);
		if (prd < 5)
		{
			prd = 5;
			Periods->Text = IntToStr(prd);
		}

		SilenceCreat(prd);//������� � �������� ��������

		//��������������� ������
		StimBlock->Enabled = true;
		Repeat->Enabled = true;
		DelLast->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void TProtoBuild::SilenceCreat(__int32 prd)
{
	//������� � �������� ��������
	/*
	prd - ������������ �����
	*/

	SPPanel *panel,
			*leftPanel;

	panel = new SPPanel(InstructBox);
	panel->Parent = InstructBox;
	panel->Caption = "�����: " + IntToStr(prd) + "(��)";

	if (InstructBox->ComponentCount > 1)
	{
		leftPanel = (SPPanel*)InstructBox->Components[InstructBox->ComponentCount - 2];//���������� ����
		panel->Left = leftPanel->Left + leftPanel->Width;
	}
	else
		panel->Left = 0;

	panel->Width = 10 + 5 * panel->Caption.Length();
	panel->Visible = true;
	panel->iType = 2;//��� ���������� (�����)
	panel->period = prd;//������ ��������
	panel->scCount = prd;//��������� ��������
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::RepeatClick(TObject *Sender)
{
	//����� ���������� �������� ��������

	__int32 reps;

	repeats->Visible = !repeats->Visible;
	Label3->Visible = !Label3->Visible;
	Label4->Visible = !Label4->Visible;

	//������������ ������
	StimBlock->Enabled = false;
	Silence->Enabled = false;
	DelLast->Enabled = false;
	if (!repeats->Visible)
	{
		//��������� �������� �� ������������
		reps = StrToInt(repeats->Text);
		if (reps < 1)
		{
			reps = 1;
			repeats->Text = IntToStr(reps);
		}

		RepeatCreat(reps);//������� � �������� ����

		//��������������� ������
		StimBlock->Enabled = true;
		Silence->Enabled = true;
		DelLast->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void TProtoBuild::RepeatCreat(__int32 reps)
{
	//������� � �������� ����
	/*
	reps - ���������� �������� � �����
	*/
	
	SPPanel *panel,
			*leftPanel;

	panel = new SPPanel(InstructBox);
	panel->Parent = InstructBox;
	panel->Caption = "����: " + IntToStr(reps) + "(�������)";

	if (InstructBox->ComponentCount > 1)
	{
		leftPanel = (SPPanel*)InstructBox->Components[InstructBox->ComponentCount - 2];//���������� ����
		panel->Left = leftPanel->Left + leftPanel->Width;
	}
	else
		panel->Left = 0;

	panel->Width = 17 + 5 * panel->Caption.Length();
	panel->Visible = true;
	panel->iType = 3;//��� ���������� (����)
	panel->scCount = reps;//���������� ��������
	panel->period = reps;//��������� ��������
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::DelLastClick(TObject *Sender)
{
	//������ ���������� �������� �� ��������� �����

	if (InstructBox->ComponentCount > 0)
	{
		//InstructBox->RemoveComponent(InstructBox->Components[InstructBox->ComponentCount - 1]);
		InstructBox->Controls[InstructBox->ComponentCount - 1]->Free();//������� ������-���� (������ delete)
	}
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::CancelAllClick(TObject *Sender)
{
	//�������� ���� ��������
	while (InstructBox->ComponentCount > 0)
		DelLastClick(this);
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::EndOfProtoBuild(TObject *Sender)
{
	//����������� ��� ��������
	//��������� �������� �������� ����������
	//����� ��������� �� ��������� �� ����� ������� ����������� ������ � ���������
	__int32 i;
	SPPanel *panel;

	//������������� � ������� ��� ���������� ���������
	if (InstructBox->ComponentCount >= 1)//�������� ������� � ����
	{
		Experiment->SetScenar->Caption = "�������������";
		Experiment->NumOfsignals->Enabled = false;//����� ���������� ��������
		Experiment->nsLbl1->Enabled = false;//���������� �������� (����� 1)
		Experiment->nsLbl2->Enabled = false;//���������� �������� (����� 2)
		Experiment->StimFreq->Enabled = false;//����� ������� ����������
		Experiment->StimPeriod->Enabled = false;//����� ������� ���������
		Experiment->stimLbl1->Enabled = false;//���������� (����� 1)
		Experiment->stimLbl2->Enabled = false;//���������� (����� 2)
		Experiment->stimLbl3->Enabled = false;//���������� (����� 3)
		Experiment->stimLbl4->Enabled = false;//���������� (����� 4)

		//�������� ���������� �������� ����� ���������� � ������ ���������
		for (i = 0; i < InstructBox->ComponentCount; i++)
		{
			panel = (SPPanel*)InstructBox->Components[i];
			if (((panel->iType == 1) || (panel->iType == 2)) && (panel->period < Periods->Tag))
				Periods->Tag = panel->period;//���������� ������ ����� ���������� ����������
		}

		PStart->NextBlock->Visible = true;//������ �������� � ���������� ����� ����������
	}
	else//�������� ������
	{
		Experiment->SetScenar->Caption = "������ ����������";
		Experiment->NumOfsignals->Enabled = true;//����� ���������� ��������
		Experiment->nsLbl1->Enabled = true;//���������� �������� (����� 1)
		Experiment->nsLbl2->Enabled = true;//���������� �������� (����� 2)
		Experiment->StimFreq->Enabled = true;//����� ������� ����������
		Experiment->StimPeriod->Enabled = true;//����� ������� ���������
		Experiment->stimLbl1->Enabled = true;//���������� (����� 1)
		Experiment->stimLbl2->Enabled = true;//���������� (����� 2)
		Experiment->stimLbl3->Enabled = true;//���������� (����� 3)
		Experiment->stimLbl4->Enabled = true;//���������� (����� 4)

		Periods->Tag = StrToInt(Experiment->StimPeriod->Text);//���������� ������ ����� ���������� ����������
		PStart->NextBlock->Visible = false;//������ �������� � ���������� ����� ����������
	}
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::CloseClicked(TObject *Sender)
{
	//��������� �������� ���������
	ProtoBuild->Close();
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::SaveProtocolClick(TObject *Sender)
{
	//��������� ������� � ��������� ����
	FILE *stream;
	__int32 i;
	SPPanel *panel;

	if (SaveDialog1->Execute())
	{
		//*.spf (s - stimulation, p - protocol, f - file)
		stream = fopen(SaveDialog1->FileName.c_str(), "w");

		for (i = 0; i < InstructBox->ComponentCount; i++)
		{
			panel = (SPPanel*)InstructBox->Components[i];
			fprintf(stream, "%d\t%d\t%d\n", panel->iType, panel->scCount, panel->period);
		}

		fclose(stream);
	}
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::LoadProtocolClick(TObject *Sender)
{
	//��������� �������� �� ���������� �����
	FILE *stream;
	__int32 d1, d2, d3;

	if (OpenDialog1->Execute())
	{
		//������������������ ���� �������� *.spf (s - stimulation, p - protocol, f - file)
		stream = fopen(OpenDialog1->FileName.c_str(), "r");

		CancelAllClick(this);//�������� ���������� �������
		while (fscanf(stream, "%d%d%d", &d1, &d2, &d3) != EOF)
		{
			//d[0] - ��� ����������, d[1] - ����������, d[2] - ������������
			if (d1 == 1)//����������
				StimCreat(d2, d3);//������� � �������� ���� ����������
			else if (d1 == 2)//�����
				SilenceCreat(d3);//������� � �������� �����
			else if (d1 == 3)//����
				RepeatCreat(d2);//������� � �������� ����
			else
			{
				CancelAllClick(this);//�������� ���������� �������
				Experiment->DevEvents->Text = "������ ������ ���������";
				break;//������ ������
			}
		}

		fclose(stream);
	}
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::CheckForKeyOtherEdits(TObject *Sender, char &Key)
{
	//�������� ������������ �������� �������, ���������������� ������� �������
	//��� ��������� ����� ����� �������� ������

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//�������� ������������ �������� ��������
		Key = '\0';//�������� ������������ ������
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::StarTimClick(TObject *Sender)
{
	//����� ������� ������ �� ����� ����������
	if (StarTim->Checked)//������� ������� �������
	{
		ProtoBuild->Width = STLbl->Left + STLbl->Width + 12;//����������� ������ ����
	}
	else//������ ������� ������
	{
		ProtoBuild->Width = DelLast->Left + DelLast->Width + 12;//��������� ������ ����
    }
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::UpDownClick(TObject *Sender, TUDBtnType Button)
{
	TUpDown *objUpDwn;//������ "������ �����-����"
	TEdit *objEdit;//������ "���� � �������"

	objUpDwn = (TUpDown*)Sender;//������ "������ �����-����"
	if (objUpDwn->Tag == 0)//����
		objEdit = HEdit;//������ "���� � �������"
	else if (objUpDwn->Tag == 1)//������
		objEdit = MEdit;//������ "���� � �������"
	else if (objUpDwn->Tag == 2)//�������
		objEdit = SEdit;//������ "���� � �������"

	objEdit->Text = IntToStr(objUpDwn->Position);//��������� �������
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::EditChange(TObject *Sender)
{
	__int32 i;
	TEdit *objEdit;//������ "���� � �������"
	TUpDown *objUpDwn;//������ "������ �����-����"

	objEdit = (TEdit*)Sender;//������ "���� � �������"
	i = StrToInt(objEdit->Text);//�������� �����

	if (objEdit->Tag == 0)//����
		objUpDwn = HUpDown;//������ "������ �����-����"
	else if (objEdit->Tag == 1)//������
		objUpDwn = MUpDown;//������ "������ �����-����"
	else if (objEdit->Tag == 2)//�������
		objUpDwn = SUpDown;//������ "������ �����-����"

	if ((i > 0) && (i <= objUpDwn->Max))
		objUpDwn->Position = i;//��������� �������
	else
		objEdit->Text = IntToStr(objUpDwn->Position);//���������� �������� ��������
}
//---------------------------------------------------------------------------

