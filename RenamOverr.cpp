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
	//настраиваем окно для выбора номеров сигналов
	Bevel1->Visible = false;
	OKBtn->Visible = false;
	CancelBtn->Visible = false;
	AppEnd->Visible = false;
	OverWrite->Visible = false;

	Label1->Caption = "Введите номера сигналов и нажмите ввод";
	Label2->Visible = true;
	Label2->Caption = "Пример: 1-25,45,47,100-200";
	UnderDL->Visible = true;
	UnderDL->Checked = false;

	chooseSigs->Visible = true;
	sigsInGrp->Visible = false;
}
//---------------------------------------------------------------------------

void TChoosDlg::setsForNOfGrp()
{
	//настраиваем окно для ввода числа сигналов в группе (для процедуры разбиения файла на несколько штук)
	ChoosDlg->Caption = "Ведите количество сигналов в группе";

	Bevel1->Visible = false;
	OKBtn->Visible = false;
	CancelBtn->Visible = false;
	AppEnd->Visible = false;
	OverWrite->Visible = false;
	UnderDL->Visible = false;

	Label1->Caption = "Введите число от 1 до " + IntToStr(Graphs->SigNumUpDwn->Max);
	Label2->Visible = false;

	sigsInGrp->Clear();//убираем имеющуюся надпись
	sigsInGrp->Visible = true;
}
//---------------------------------------------------------------------------

void TChoosDlg::setsForFileChs()
{
	//настраиваем окно для проверки выбранного файла
	ChoosDlg->Caption = "Предупреждение";
	Bevel1->Visible = true;
	OKBtn->Visible = true;
	CancelBtn->Visible = true;
	OverWrite->Visible = true;
	if (Experiment->DiscontinWrt->Checked)
		AppEnd->Visible = true;
	else
    	AppEnd->Visible = false;

	Label1->Caption = "Указанный файл уже существует!";
	Label2->Visible = false;

	chooseSigs->Visible = false;
	sigsInGrp->Visible = false;
	UnderDL->Visible = false;
}
//---------------------------------------------------------------------
void __fastcall TChoosDlg::chooseSigsKeyPress(TObject *Sender, char &Key)
{
	//отслеживаем нажатия кнопок
	if (Key == '\r')//нажата кнопка ввода
	{
		Experiment->DevEvents->Text = "принято";
		CompilNumbers();//при нажатии кнопки ввода формируем окончательный список сигналов
	}
	else if (((Key < '0') || (Key > '9')) && (Key != '\b') && (Key != '-') && (Key != ','))//проверим допустимость символа
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TChoosDlg::sigsInGrpKeyPress(TObject *Sender, char &Key)
{
	//отслеживаем нажатые кнопки. Ппри нажатии кнопки ввода, после набора номеров,
	//продолжаем действия по сохранению групп сигналов

	if (Key == '\r')//нажата кнопка ввода
	{
		Experiment->DevEvents->Text = "принято";
		MultiplCompilNumbers();//задаём группы номеров сигналов для сохранения
	}
	else if ((Key < '0') || (Key > '9'))//проверяем допустимость введённых символов
		Key = '\0';
}
//---------------------------------------------------------------------------

void TChoosDlg::CompilNumbers()
{
	//формируем массив с номерами удаляемых сигналов
		//удаляются либо указанные сигналы, либо все не указанные
	__int32 i, j, p,
			*digits,//массив с введёнными номерами сигналов (пользовательские [1, ..., N])
			*sigNumbers,//все указанные номера сигналов (программные - от нуля [0, ..., N - 1])
			*invertSN,//инвертированный sigNumbers
			maxNum,//количество видимых сигналов
			kD,//счётчик элементов массива digits
			kS,//счётчик элементов массива sigNumbers
			strLen;//длина введённой строки и размер массива с номерами сигналов
	AnsiString subStr, subStr2;//части введённой строки с номерами
	bool flg;

	maxNum = Graphs->SigNumUpDwn->Max;//количество видимых сигналов
	strLen = chooseSigs->Text.Length();//длина строки и размер массива
	digits = new __int32[strLen];//размер массива с запасом
	flg = false;
	kD = 0;//счётчик элементов массива digits
	for (i = 1; i <= strLen; i++)
	{
		subStr = chooseSigs->Text.SubString(i, 1);
		if ((subStr != ',') && (subStr != '-') && !flg)//цифра (начало очередного номера)
		{
			flg = true;//обнаружено начало очередного номера сигнала
			p = i;//позиция, с которой начинается очередной номер сигнала
		}

		if (((subStr == ',') || (subStr == '-') || (i == strLen)) && flg)//номер полностью прочитан (от p до i - p)
		{
			j = 1 * __int32((i == strLen) && (subStr != '-') && (subStr != ','));//последний символ является цифрой
			subStr2 = chooseSigs->Text.SubString(p, i - p + j);//вырезаем номер сигнала из строки
			digits[kD] = StrToInt(subStr2);
			if (digits[kD] == 0)
				digits[kD] = 1;//заменяем ноль на единицу (пользовательская нумерация)
			kD++;//увеличиваем счётчик элементов массива digits
			flg = false;//номер сигнала полностью считан
		}

		if (subStr == '-')
			if (kD > 0)//в массиве digits уже есть элементы
				if (digits[kD - 1] > 0)//предыдущий символ не был разделителем
				{
					digits[kD] = -1;//условный разделитель (интервал)
					kD++;//увеличиваем счётчик элементов массива digits
				}
			else
			{
                digits[kD] = -1;//условный разделитель (интервал)
				kD++;//увеличиваем счётчик элементов массива digits
            }
	}

	//формируем массив указанных пользователем номеров сигналов
	sigNumbers = new __int32[maxNum];//все указанные номера сигналов (программные - от нуля [0, ..., N - 1])
	kS = 0;//счётчик элементов массива sigNumbers
	for (i = 0; i < kD; i++)
	{
		if (digits[i] > 0)//указан номер сигнала
		{
			sigNumbers[kS] = digits[i] - 1;//переходим к нумерации от нуля
			kS++;//увеличиваем счётчик элементов массива sigNumbers
		}
		else//(digits[i] < 0) указан условный разделитель (интервал)
		{
			if (i > 0)//не первый элемент массива digits
				strLen = digits[i - 1] - 1;//левая граница интервала (нумерация от нуля)
			else
				strLen = 1;//левая граница интервала
			if (i < (kD - 1))//не последний элемент массива digits
				p = digits[i + 1];//правая граница интервала
			else
				p = maxNum;//правая граница интервала

			kS--;//на один уменьшаем счётчик элементов массива sigNumbers (затираем один номер)
			for (j = strLen; j < p; j++)
			{
				sigNumbers[kS] = j;//нумерация от нуля
				kS++;//увеличиваем счётчик элементов массива sigNumbers
			}
			i++;//переходим к следующему необработанному элементу массива digits
        }
	}
	//отсортируем номера удаляемых сигналов по убыванию от максимального
	//также исключим некорректные и повторяющиеся номера
	i = 0;
	while (i < kS)
	{
		if ((sigNumbers[i] >= maxNum) || (sigNumbers[i] < 0))//номер не корректен
		{
			kS--;//количество корректных номеров уменьшается
			for (j = i; j < kS; j++)//удаляем некорректный номер
				sigNumbers[j] = sigNumbers[j + 1];//сдвигаем влево номера, стоящие справа от i
		}
		j = i + 1;
		while (j < kS)
		{
			if (sigNumbers[j] > sigNumbers[i])//наиден больший номер
			{
				p = sigNumbers[i];//меняем номера местами
				sigNumbers[i] = sigNumbers[j];//меняем номера местами
				sigNumbers[j] = p;//меняем номера местами
			}
			else if (sigNumbers[j] == sigNumbers[i])//номера совпадают
			{
				kS--;//количество уникальных номеров уменьшается
				for (p = j; p < kS; p++)//удаляем повторяющийся номер
					sigNumbers[p] = sigNumbers[p + 1];//сдвигаем влево номера, стоящие справа от i
			}
			j++;//сравниваем со следующим элементом (номером)
		}
		i++;//переходим к следующему номеру
	}

	if (sav)//необходимо сохранить сигналы, номера которых указаны (инвертируем номера)
	{
		invertSN = new __int32[maxNum];//инвертированный sigNumbers
		p = 0;//счётчик элементов массива invertSN
		for (j = maxNum - 1; j > sigNumbers[0]; j--)
		{
			invertSN[p] = j;//(нумерация от нуля)
			p++;
		}
		for (i = 0; i < kS - 1; i++)
			for (j = sigNumbers[i] - 1; j > sigNumbers[i + 1]; j--)
			{
				invertSN[p] = j;//(нумерация от нуля)
				p++;
			}
		for (j = sigNumbers[kS - 1] - 1; j >= 0; j--)
		{
			invertSN[p] = j;//(нумерация от нуля)
			p++;
		}
		Graphs->RemovEnteredNumbers(invertSN, p);//передаём номера удаляемых сигналов (там же сохраняем остальные)

		delete[] invertSN; invertSN = NULL;
	}
	else
		Graphs->RemovEnteredNumbers(sigNumbers, kS);//передаём номера удаляемых сигналов

	ChoosDlg->Close();//выходим из подпрограммы
	
	delete[] digits; digits = NULL;
	delete[] sigNumbers; sigNumbers = NULL;
}
//---------------------------------------------------------------------------

void TChoosDlg::MultiplCompilNumbers()
{
	//задаём группы номеров сигналов для сохранения
	__int32 i, j,
			nInGrp,//заданное число сигналов в группе
			*sigsIndx;//массив с введёнными номерами сигналов
			
	if (!ChoosDlg->sigsInGrp->Text.IsEmpty())
	{
		nInGrp = StrToInt(ChoosDlg->sigsInGrp->Text);//заданное число сигналов в группе
		i = 0;
		while ((i + nInGrp) < Graphs->SigNumUpDwn->Max)//сохраняем "целые" группы сигналов
		{
			sigsIndx = new __int32[nInGrp];//массив индексов
			for (j = 0; j < nInGrp; j++)
				sigsIndx[j] = j + i + 1;//номер первого сигнала в группе
			i += j;

			//передаём массив в модуль Graphs->SaveRemoveGrpSigs
			Graphs->RemovEnteredNumbers(sigsIndx, 3);
			delete[] sigsIndx; sigsIndx = NULL;
		}
		if ((i > 1) && (i < Graphs->SigNumUpDwn->Max))//сохраняем остатки
		{
			sigsIndx = new __int32[3];//массив индексов
			i++;
			sigsIndx[0] = i;//номер первого сигнала в группе
			sigsIndx[1] = -1;//вместо тире ("-")
			sigsIndx[2] = Graphs->SigNumUpDwn->Max;//номер последнего сигнала в группе

			//передаём массив в модуль Graphs->SaveRemoveGrpSigs
			Graphs->RemovEnteredNumbers(sigsIndx, 3);
			delete[] sigsIndx; sigsIndx = NULL;
		}
	}

	ChoosDlg->Close();//выходим из подпрограммы
}
//---------------------------------------------------------------------------

void __fastcall TChoosDlg::UnderDLClick(TObject *Sender)
{
	//находим сигналы, лежащие ниже пороговой линии ("under dead line")
	__int32 i, k,
			*sigsIndx,//массив с введёнными номерами сигналов
			shft1;//номер первого элемента для ampls и peakInds
			//shft2;//номер первого элемента для spans

	//Graphs->gInd - индексы принятых сигналов. При удалении сигнала, убираем его номер из этого массива
	//Graphs->gRF[2] - последовательность чтения каналов с полными развёртками (0 - первый отображаемый канал, 1 - последний отображаемый канал + 1)
	//Graphs->cPrm - массивы с вычисляемыми параметрами сигналов

	if (UnderDL->Checked)
	{
		k = 0;//счётчик нужных сигналов
		sigsIndx = new __int32[Graphs->SigNumUpDwn->Max];//размер массива с запасом
		for (i = 0; i < Graphs->SigNumUpDwn->Max; i++)//перебираем все сигналы и ищем нужные
		{
			shft1 = (3 * Graphs->ftChan * Graphs->gInd[i]) + (3 * Graphs->gRF[0]);//номер первого элемента для ampls и peakInds
			//shft2 = (5 * Graphs->ftChan * Graphs->gInd[i]) + (5 * Graphs->gRF[0]);//номер первого элемента для spans

			if (Graphs->cPrm[0].ampls[shft1] <= 0)//сигнал ниже порога
			{
				sigsIndx[k] = i;
				k++;
			}
		}

		//передаём массив номеров в модуль Graphs->SaveRemoveGrpSigs. Там же сохраним/удалим эти сигналы
		Graphs->RemovEnteredNumbers(sigsIndx, k);
		Experiment->DevEvents->Text = "принято";
		ChoosDlg->Close();//выходим из подпрограммы
	}
}
//---------------------------------------------------------------------------

