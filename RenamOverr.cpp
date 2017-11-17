//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "RenamOverr.h"
#include "MainForm.h"
#include "TimeCourses.h"
#include "PredStart.h"

//---------------------------------------------------------------------
#pragma resource "*.dfm"
TChoosDlg *ChoosDlg;

//--------------------------------------------------------------------- 
__fastcall TChoosDlg::TChoosDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------------

void TChoosDlg::setsForNumbersChs()
{
	//����������� ���� ��� ������ ������� ��������
	Bevel1->Visible = false;
	OKBtn->Visible = false;
	CancelBtn->Visible = false;
	AppEnd->Visible = false;
	OverWrite->Visible = false;

	Label1->Caption = "������� ������ �������� � ������� ����";
	Label2->Visible = true;
	Label2->Caption = "������: 1-25,45,47,100-200";
	UnderDL->Visible = true;
	UnderDL->Checked = false;

	chooseSigs->Visible = true;
	sigsInGrp->Visible = false;
}
//---------------------------------------------------------------------------

void TChoosDlg::setsForNOfGrp()
{
	//����������� ���� ��� ����� ����� �������� � ������ (��� ��������� ��������� ����� �� ��������� ����)
	ChoosDlg->Caption = "������ ���������� �������� � ������";

	Bevel1->Visible = false;
	OKBtn->Visible = false;
	CancelBtn->Visible = false;
	AppEnd->Visible = false;
	OverWrite->Visible = false;
	UnderDL->Visible = false;

	Label1->Caption = "������� ����� �� 1 �� " + IntToStr(Graphs->SigNumUpDwn->Max);
	Label2->Visible = false;

	sigsInGrp->Clear();//������� ��������� �������
	sigsInGrp->Visible = true;
}
//---------------------------------------------------------------------------

void TChoosDlg::setsForFileChs()
{
	//����������� ���� ��� �������� ���������� �����
	ChoosDlg->Caption = "��������������";
	Bevel1->Visible = true;
	OKBtn->Visible = true;
	CancelBtn->Visible = true;
	OverWrite->Visible = true;
	if (Experiment->DiscontinWrt->Checked)
		AppEnd->Visible = true;
	else
    	AppEnd->Visible = false;

	Label1->Caption = "��������� ���� ��� ����������!";
	Label2->Visible = false;

	chooseSigs->Visible = false;
	sigsInGrp->Visible = false;
	UnderDL->Visible = false;
}
//---------------------------------------------------------------------
void __fastcall TChoosDlg::chooseSigsKeyPress(TObject *Sender, char &Key)
{
	//����������� ������� ������
	if (Key == '\r')//������ ������ �����
	{
		Experiment->DevEvents->Text = "�������";
		CompilNumbers();//��� ������� ������ ����� ��������� ������������� ������ ��������
	}
	else if (((Key < '0') || (Key > '9')) && (Key != '\b') && (Key != '-') && (Key != ','))//�������� ������������ �������
		Key = '\0';//�������� ������������ ������
}
//---------------------------------------------------------------------------

void __fastcall TChoosDlg::sigsInGrpKeyPress(TObject *Sender, char &Key)
{
	//����������� ������� ������. ���� ������� ������ �����, ����� ������ �������,
	//���������� �������� �� ���������� ����� ��������

	if (Key == '\r')//������ ������ �����
	{
		Experiment->DevEvents->Text = "�������";
		MultiplCompilNumbers();//����� ������ ������� �������� ��� ����������
	}
	else if ((Key < '0') || (Key > '9'))//��������� ������������ �������� ��������
		Key = '\0';
}
//---------------------------------------------------------------------------

void TChoosDlg::CompilNumbers()
{
	//��������� ������ � �������� ��������� ��������
		//��������� ���� ��������� �������, ���� ��� �� ���������
	__int32 i, j, p,
			*digits,//������ � ��������� �������� �������� (���������������� [1, ..., N])
			*sigNumbers,//��� ��������� ������ �������� (����������� - �� ���� [0, ..., N - 1])
			*invertSN,//��������������� sigNumbers
			maxNum,//���������� ������� ��������
			kD,//������� ��������� ������� digits
			kS,//������� ��������� ������� sigNumbers
			strLen;//����� �������� ������ � ������ ������� � �������� ��������
	AnsiString subStr, subStr2;//����� �������� ������ � ��������
	bool flg;

	maxNum = Graphs->SigNumUpDwn->Max;//���������� ������� ��������
	strLen = chooseSigs->Text.Length();//����� ������ � ������ �������
	digits = new __int32[strLen];//������ ������� � �������
	flg = false;
	kD = 0;//������� ��������� ������� digits
	for (i = 1; i <= strLen; i++)
	{
		subStr = chooseSigs->Text.SubString(i, 1);
		if ((subStr != ',') && (subStr != '-') && !flg)//����� (������ ���������� ������)
		{
			flg = true;//���������� ������ ���������� ������ �������
			p = i;//�������, � ������� ���������� ��������� ����� �������
		}

		if (((subStr == ',') || (subStr == '-') || (i == strLen)) && flg)//����� ��������� �������� (�� p �� i - p)
		{
			j = 1 * __int32((i == strLen) && (subStr != '-') && (subStr != ','));//��������� ������ �������� ������
			subStr2 = chooseSigs->Text.SubString(p, i - p + j);//�������� ����� ������� �� ������
			digits[kD] = StrToInt(subStr2);
			if (digits[kD] == 0)
				digits[kD] = 1;//�������� ���� �� ������� (���������������� ���������)
			kD++;//����������� ������� ��������� ������� digits
			flg = false;//����� ������� ��������� ������
		}

		if (subStr == '-')
			if (kD > 0)//� ������� digits ��� ���� ��������
				if (digits[kD - 1] > 0)//���������� ������ �� ��� ������������
				{
					digits[kD] = -1;//�������� ����������� (��������)
					kD++;//����������� ������� ��������� ������� digits
				}
			else
			{
                digits[kD] = -1;//�������� ����������� (��������)
				kD++;//����������� ������� ��������� ������� digits
            }
	}

	//��������� ������ ��������� ������������� ������� ��������
	sigNumbers = new __int32[maxNum];//��� ��������� ������ �������� (����������� - �� ���� [0, ..., N - 1])
	kS = 0;//������� ��������� ������� sigNumbers
	for (i = 0; i < kD; i++)
	{
		if (digits[i] > 0)//������ ����� �������
		{
			sigNumbers[kS] = digits[i] - 1;//��������� � ��������� �� ����
			kS++;//����������� ������� ��������� ������� sigNumbers
		}
		else//(digits[i] < 0) ������ �������� ����������� (��������)
		{
			if (i > 0)//�� ������ ������� ������� digits
				strLen = digits[i - 1] - 1;//����� ������� ��������� (��������� �� ����)
			else
				strLen = 1;//����� ������� ���������
			if (i < (kD - 1))//�� ��������� ������� ������� digits
				p = digits[i + 1];//������ ������� ���������
			else
				p = maxNum;//������ ������� ���������

			kS--;//�� ���� ��������� ������� ��������� ������� sigNumbers (�������� ���� �����)
			for (j = strLen; j < p; j++)
			{
				sigNumbers[kS] = j;//��������� �� ����
				kS++;//����������� ������� ��������� ������� sigNumbers
			}
			i++;//��������� � ���������� ��������������� �������� ������� digits
        }
	}
	//����������� ������ ��������� �������� �� �������� �� �������������
	//����� �������� ������������ � ������������� ������
	i = 0;
	while (i < kS)
	{
		if ((sigNumbers[i] >= maxNum) || (sigNumbers[i] < 0))//����� �� ���������
		{
			kS--;//���������� ���������� ������� �����������
			for (j = i; j < kS; j++)//������� ������������ �����
				sigNumbers[j] = sigNumbers[j + 1];//�������� ����� ������, ������� ������ �� i
		}
		j = i + 1;
		while (j < kS)
		{
			if (sigNumbers[j] > sigNumbers[i])//������ ������� �����
			{
				p = sigNumbers[i];//������ ������ �������
				sigNumbers[i] = sigNumbers[j];//������ ������ �������
				sigNumbers[j] = p;//������ ������ �������
			}
			else if (sigNumbers[j] == sigNumbers[i])//������ ���������
			{
				kS--;//���������� ���������� ������� �����������
				for (p = j; p < kS; p++)//������� ������������� �����
					sigNumbers[p] = sigNumbers[p + 1];//�������� ����� ������, ������� ������ �� i
			}
			j++;//���������� �� ��������� ��������� (�������)
		}
		i++;//��������� � ���������� ������
	}

	if (sav)//���������� ��������� �������, ������ ������� ������� (����������� ������)
	{
		invertSN = new __int32[maxNum];//��������������� sigNumbers
		p = 0;//������� ��������� ������� invertSN
		for (j = maxNum - 1; j > sigNumbers[0]; j--)
		{
			invertSN[p] = j;//(��������� �� ����)
			p++;
		}
		for (i = 0; i < kS - 1; i++)
			for (j = sigNumbers[i] - 1; j > sigNumbers[i + 1]; j--)
			{
				invertSN[p] = j;//(��������� �� ����)
				p++;
			}
		for (j = sigNumbers[kS - 1] - 1; j >= 0; j--)
		{
			invertSN[p] = j;//(��������� �� ����)
			p++;
		}
		Graphs->RemovEnteredNumbers(invertSN, p);//������� ������ ��������� �������� (��� �� ��������� ���������)

		delete[] invertSN; invertSN = NULL;
	}
	else
		Graphs->RemovEnteredNumbers(sigNumbers, kS);//������� ������ ��������� ��������

	ChoosDlg->Close();//������� �� ������������
	
	delete[] digits; digits = NULL;
	delete[] sigNumbers; sigNumbers = NULL;
}
//---------------------------------------------------------------------------

void TChoosDlg::MultiplCompilNumbers()
{
	//����� ������ ������� �������� ��� ����������
	__int32 i, j,
			nInGrp,//�������� ����� �������� � ������
			*sigsIndx;//������ � ��������� �������� ��������
			
	if (!ChoosDlg->sigsInGrp->Text.IsEmpty())
	{
		nInGrp = StrToInt(ChoosDlg->sigsInGrp->Text);//�������� ����� �������� � ������
		i = 0;
		while ((i + nInGrp) < Graphs->SigNumUpDwn->Max)//��������� "�����" ������ ��������
		{
			sigsIndx = new __int32[nInGrp];//������ ��������
			for (j = 0; j < nInGrp; j++)
				sigsIndx[j] = j + i + 1;//����� ������� ������� � ������
			i += j;

			//������� ������ � ������ Graphs->SaveRemoveGrpSigs
			Graphs->RemovEnteredNumbers(sigsIndx, 3);
			delete[] sigsIndx; sigsIndx = NULL;
		}
		if ((i > 1) && (i < Graphs->SigNumUpDwn->Max))//��������� �������
		{
			sigsIndx = new __int32[3];//������ ��������
			i++;
			sigsIndx[0] = i;//����� ������� ������� � ������
			sigsIndx[1] = -1;//������ ���� ("-")
			sigsIndx[2] = Graphs->SigNumUpDwn->Max;//����� ���������� ������� � ������

			//������� ������ � ������ Graphs->SaveRemoveGrpSigs
			Graphs->RemovEnteredNumbers(sigsIndx, 3);
			delete[] sigsIndx; sigsIndx = NULL;
		}
	}

	ChoosDlg->Close();//������� �� ������������
}
//---------------------------------------------------------------------------

void __fastcall TChoosDlg::UnderDLClick(TObject *Sender)
{
	//������� �������, ������� ���� ��������� ����� ("under dead line")
	__int32 i, k,
			*sigsIndx,//������ � ��������� �������� ��������
			shft1;//����� ������� �������� ��� ampls � peakInds
			//shft2;//����� ������� �������� ��� spans

	//Graphs->gInd - ������� �������� ��������. ��� �������� �������, ������� ��� ����� �� ����� �������
	//Graphs->gRF[2] - ������������������ ������ ������� � ������� ���������� (0 - ������ ������������ �����, 1 - ��������� ������������ ����� + 1)
	//Graphs->cPrm - ������� � ������������ ����������� ��������

	if (UnderDL->Checked)
	{
		k = 0;//������� ������ ��������
		sigsIndx = new __int32[Graphs->SigNumUpDwn->Max];//������ ������� � �������
		for (i = 0; i < Graphs->SigNumUpDwn->Max; i++)//���������� ��� ������� � ���� ������
		{
			shft1 = (3 * Graphs->ftChan * Graphs->gInd[i]) + (3 * Graphs->gRF[0]);//����� ������� �������� ��� ampls � peakInds
			//shft2 = (5 * Graphs->ftChan * Graphs->gInd[i]) + (5 * Graphs->gRF[0]);//����� ������� �������� ��� spans

			if (Graphs->cPrm[0].ampls[shft1] <= 0)//������ ���� ������
			{
				sigsIndx[k] = i;
				k++;
			}
		}

		//������� ������ ������� � ������ Graphs->SaveRemoveGrpSigs. ��� �� ��������/������ ��� �������
		Graphs->RemovEnteredNumbers(sigsIndx, k);
		Experiment->DevEvents->Text = "�������";
		ChoosDlg->Close();//������� �� ������������
	}
}
//---------------------------------------------------------------------------

