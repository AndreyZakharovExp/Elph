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
	//окно вывода данных, поступающих со второстепенных каналов
	MChann[0] = Chan2;//указатели на окна вывода данных со второстепенных каналов
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
	//обновляем вид окна с данными со второстепенных каналов
	__int32 i,
			nChann;

	nChann = Experiment->a.m_nChannelNumber;
	//если кол-во активных каналов больше одного и режим сбора не многоканальный
	if ((nChann > 1) && (!Experiment->RecMode->Checked))
	{
		SecondChan->Height = 30 + (51 * (nChann - 1));//подгоняем ширину окна с данными со второстепенных каналов
		SecondChan->Caption = "Каналы 2";
		for (i = 1; i < (nChann - 1); i++)
		{
            SecondChan->Caption = SecondChan->Caption + ", ";
			SecondChan->Caption = SecondChan->Caption + IntToStr(i + 2);
		}
		SecondChan->Show();
	}
	else//иначе закрываем окно, если оно было
		SecondChan->Close();
}
//---------------------------------------------------------------------------

