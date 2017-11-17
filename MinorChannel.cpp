//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MainForm.h"
#include "MinorChannel.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSecondChan *SecondChan;
//---------------------------------------------------------------------------
__fastcall TSecondChan::TSecondChan(TComponent* Owner)
	: TForm(Owner)
{
	//���� ������ ������, ����������� �� �������������� �������
	MChann[0] = Chan2;//��������� �� ���� ������ ������ �� �������������� �������
	MChann[1] = Chan3;//
	MChann[2] = Chan4;//
	MChann[3] = Chan5;//
	MChann[4] = Chan6;//
	MChann[5] = Chan7;//
	MChann[6] = Chan8;//
}
//---------------------------------------------------------------------------

void TSecondChan::RenewMinorChanWin()
{
	//��������� ��� ���� � ������� �� �������������� �������
	__int32 i,
			nChann;

	nChann = Experiment->a.m_nChannelNumber;
	//���� ���-�� �������� ������� ������ ������ � ����� ����� �� ��������������
	if ((nChann > 1) && (!Experiment->RecMode->Checked))
	{
		SecondChan->Height = 30 + (51 * (nChann - 1));//��������� ������ ���� � ������� �� �������������� �������
		SecondChan->Caption = "������ 2";
		for (i = 1; i < (nChann - 1); i++)
		{
            SecondChan->Caption = SecondChan->Caption + ", ";
			SecondChan->Caption = SecondChan->Caption + IntToStr(i + 2);
		}
		SecondChan->Show();
	}
	else//����� ��������� ����, ���� ��� ����
		SecondChan->Close();
}
//---------------------------------------------------------------------------

