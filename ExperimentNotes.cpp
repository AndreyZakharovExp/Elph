//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ExperimentNotes.h"
#include "TimeCourses.h"
#include "MainForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TExpNotes *ExpNotes;

//---------------------------------------------------------------------------
__fastcall TExpNotes::TExpNotes(TComponent* Owner)
	: TForm(Owner)
{
	theMarker = NULL;//���� ��� ����������� �����
}
//---------------------------------------------------------------------------

void __fastcall TExpNotes::addUMarkKeyPress(TObject *Sender, char &Key)
{
	//��� ������� ������ ����� ���������� ��������� �����
	__int32 i, j, k,
			pnG;//����� ������� �� �������, � �������� ������������� �����
	bool flag,
		 chN[maxChannels];//������ ��������� ������������� �������, ������� ����������� �����
	AnsiString infoCopy,
			   aNumber;//����� - ����� ������

	if (Key == '\r')//������ enter - ��������� ����� �� ������ � �������� ����������� (���� ������ ����)
	{
		//���������� ����� �������, �������� ����� ���������� �����
		//���������� ����� ����������� ������� (�� ����� ������ ��� ����� ���������� ���������� �������)
		pnG = Graphs->CrntSig->Tag - 1;//����� ����� (�������) �� �������

		//���������� ������
		for (i = 0; i < Graphs->ftChan; i++)//�������������
			chN[i] = false;//�������������

		flag = false;//��� ���� �������
		j = addUMark->Text.AnsiPos(":");//���������� ��������� �����������
		infoCopy = addUMark->Text;
		i = 2;
		while (j >= i)
		{
			i = infoCopy.AnsiPos(",");//���������� ��������� ����������� ������� �������
			if ((i == 0) || (i > j))
				i = j;//������ ���� ����� ������
			j -= i;//�������� �� ��������� �����
			aNumber = infoCopy;
			infoCopy.Delete(1, i);//������� ����������� ����� ������
			aNumber.Delete(i, aNumber.Length());//�������� ����� �� ������ ������
			k = StrToInt(aNumber) - 1;
			if ((k >= 0) && (k < Graphs->ftChan))//���� ����� ������ ������ � ��������
			{
				chN[k] = true;//��������� �����
				flag = true;//�� ��� ���� �������
			}
		}
		if (!flag)
			for (i = 0; i < Graphs->ftChan; i++)//�������������
				chN[i] = true;//����� ����� �� ���� �������

		AddMarker(infoCopy, pnG, &chN[0]);//���������� ������ ����� � ������
		addUMark->Text = "";//�������� ������ ��� �����
	}
}
//---------------------------------------------------------------------------

void TExpNotes::AddMarker(AnsiString addedMark, __int32 pnG, bool *chN)
{
	//��������� ����� �� ������ � �������� ����������� (���� ������ ����)
	/*
	addedMark - ����� ����������� �����
	pnG - ����� ������� �� �������, � �������� ������������� �����
	chN - ��������� �� ������ � ����������� ����� �� �������
	*/
	__int32 i, j, k,
			mij;
	bool flag1, flag2,
		 all0,
		 mNew1, mNew2;
	markers *eC[maxChannels];//��������� �� �����

	for (i = 0; i < Graphs->ftChan; i++)
		eC[i] = NULL;//�������� ���������

	flag1 = false;//��� ���������� �������� ������ true
	flag2 = false;//��� �� ���������� �������� ������ true
	mij = 0;
	for (i = 0; i < addUMark->Tag; i++)//��������, ���� �� ��� �� ������ ������ ������� �����
	{
		if (pnG == theMarker->pointOnGraph)//������ �������� ������� (�������� ������)
			if (theMarker->textMark == addedMark)//������� ���������
			{
				flag1 = true;//���� ����� ����� �� ������ ������ �������
				k = i;//���������� ����� (��� ������ ����� � ����������� �������� � �������� �������)
				eC[mij] = theMarker;//��������� ����� ����� ��� ������������ ��������������
				mij++;
			}
			else//����� ������ �����
			{
				flag2 = true;//���� ������ ����� �� ������ ������ �������
				eC[mij] = theMarker;//��������� ����� ����� ��� ������������ ��������������
				mij++;
			}
		theMarker = theMarker->nextM;//��������� � ��������� �����
	}

	if (!flag1 && !flag2 && !addedMark.IsEmpty())//������ ����� ����� (��������� ������� � ����������� ������������ ������ theMarker)
	{
		CreateAMark();//�������� �������: ���������� �������� � ����������� ������������ ������ theMarker
		theMarker->textMark = addedMark;//����� �����
		theMarker->pointOnGraph = pnG;//����� ������� �� �������
		for (i = 0; i < Graphs->ftChan; i++)
			theMarker->chanN[i] = chN[i];//������ �������, �� ������� ������������ �����
	}
	else if (flag1 && flag2)//���� ����� �����, � ���� ������ ����� (������ ����� �� ������)
	{
		for (i = 0; i < mij; i++)//���������� ����������� �������� ������� theMarker
			for (j = 0; j < Graphs->ftChan; j++)
				if (chN[j])
				{
					eC[i]->textMark = addedMark;//������ ����� ����� �� ��������� �������
					eC[i]->chanN[j] = true;//������������� �������� ����� � ������
				}
	}
	else if (!flag1 && flag2)//��� ����� �����, �� ���� ������ �����
	{
		if (addedMark.IsEmpty())//������� ������ ������ (���������-�������� �����)
		{
			for (i = 0; i < mij; i++)
				for (j = 0; j < Graphs->ftChan; j++)
					if (chN[j])
						eC[i]->chanN[j] = false;//��� ����� �� ������ ������
		}
		else//������� �� ������ ������
		{
			mNew1 = false;//��� ������� � ������������� �������� ����� �����
			mNew2 = false;//��������� ������
			for (i = 0; i < mij; i++)
			{
				for (j = 0; j < Graphs->ftChan; j++)
					if (chN[j])//�� ������ ������ ��������� ����� �����
						eC[i]->chanN[j] = false;//�� ���� ������ ����������� ������ �����
					else if (!chN[j] && eC[i]->chanN[j] && !mNew2)//�� ��������� ������� ����������
						mNew1 = true;//������ � ������������� �������� ����� �����
				if (mNew1 && !mNew2)//����� �������� ������ �������, ������� �������� ������� ������� theMarker
				{
					all0 = true;//��������, �� ����� �� ������ ������� ����� (��������� ������ theMarker)
					for (j = 0; j < Graphs->ftChan; j++)
						all0 = (all0 && (!eC[i]->chanN[j]));
					if (all0)//��� �������� ������� chanN ���������� � false, ����� ���������� ��� ����� ��� �������� �����
					{
						eC[i]->textMark = addedMark;
						for (j = 0; j < Graphs->ftChan; j++)
							eC[i]->chanN[i] = chN[i];//������ �������, �� ������� ������������ �����
					}
					else
					{
						CreateAMark();//�������� �������: ���������� �������� � ����������� ������������ ������ theMarker
						theMarker->textMark = addedMark;//����� �����
						theMarker->pointOnGraph = pnG;//����� ������� �� �������
						for (i = 0; i < Graphs->ftChan; i++)
							theMarker->chanN[i] = chN[i];//������ �������, �� ������� ������������ �����
					}
					mNew2 = true;//��������� ������
				}
			}
		}
	}
	else if (flag1 && !flag2)//���� ����� �����, � ������ ����� ��� (������ ����������� �� �������)
		for (i = 0; i < Graphs->ftChan; i++)
			eC[k]->chanN[i] = (eC[k]->chanN[i] || chN[i]);//����������� ����� � ������ ������� (k ���������� ����)

	//������ ������ ���������� � ������ �������� ������� theMarker
	for (i = 0; i < mij; i++)
		if (eC[i])
		{
			for (j = (i + 1); j < mij; j++)
				if (eC[j])
					if ((eC[i]->pointOnGraph == eC[j]->pointOnGraph) && (eC[i]->textMark == eC[j]->textMark))
					{
						DeleteAMark(eC[j]);//������� ������ ������� ������� theMarker
						eC[j] = NULL;//�������� ��������� �� ����������� �������
					}

			all0 = true;//��������, �� ����� �� ������ ������� ����� (��������� ������ theMarker)
			for (j = 0; j < Graphs->ftChan; j++)//�� ���������� ������������ �������!
				all0 = (all0 && (!eC[i]->chanN[j]));
			if (all0)//��� �������� ������� chanN ���������� � false, ����� ������� ����� (����������� ������ theMarker)
			{
				DeleteAMark(eC[i]);//������� ������ ������� ������� theMarker
				eC[j] = NULL;//�������� ��������� �� ����������� �������
			}
		}

	Graphs->amplInTAfterDraw(this);//��������� ����������� �������� � �������
}
//---------------------------------------------------------------------------

void TExpNotes::CreateAMark()
{
	//�������� �������: ���������� �������� � ����������� ������������ ������ theMarker
	markers *buf;//����� ��������� �� ������� ����� (����� ���� ����)

	buf = theMarker;//����� ��������� �� ������� ����� (����� ���� ����)
	theMarker = new markers[1];//������ ������� ������������ ������������� ������� � �������
	if (addUMark->Tag == 0)//��� ����������� �����
	{
		theMarker->nextM = theMarker;//������������ ����� ��������� �� ����
		theMarker->prevM = theMarker;//������������ ����� ��������� �� ����
	}
	else
	{
		theMarker->nextM = buf->nextM;//����� ��������� �� ���������� ����� ��������
		theMarker->prevM = buf;//����� ��������� �� �������
		buf->nextM->prevM = theMarker;//��������� ����� �������� ��������� �� �����
		buf->nextM = theMarker;//������� ��������� �� �����
	}
	addUMark->Tag++;//����������� ������� �����
}
//---------------------------------------------------------------------------

void TExpNotes::DeleteAMark(markers *delM)
{
	//�������� �������� ����� //������������ ������������ ������ (����������� ���)
	/*
	delM - ��������� �� �������� �����
	*/

	if ((addUMark->Tag > 0) && (theMarker))
	{
		delM->prevM->nextM = delM->nextM;//���������� ����� ��������� �� ����������
		delM->nextM->prevM = delM->prevM;//��������� ����� ��������� �� �����������
		delete[] delM;

		addUMark->Tag--;//���������� ����� �����������
		if (addUMark->Tag == 0)
			theMarker = NULL;
	}
}
//---------------------------------------------------------------------------

void TExpNotes::DeleteMarkers()
{
	//������� ������� ������� �� �������
	while ((addUMark->Tag > 0) && theMarker)
		DeleteAMark(theMarker);
}
//---------------------------------------------------------------------------

