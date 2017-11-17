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
	theMarker = NULL;//пока нет добавленных меток
}
//---------------------------------------------------------------------------

void __fastcall TExpNotes::addUMarkKeyPress(TObject *Sender, char &Key)
{
	//при нажатии кнопки ввода отображаем набранный текст
	__int32 i, j, k,
			pnG;//номер сигнала на графике, к которому прикрепляется метка
	bool flag,
		 chN[maxChannels];//номера указанных пользователем каналов, которым принадлежит метка
	AnsiString infoCopy,
			   aNumber;//число - номер канала

	if (Key == '\r')//нажали enter - добавляем метку на график с главными амплитудами (пока только туда)
	{
		//определяем номер сигнала, которому нужно прикрепить метку
		//определяем номер выделенного сигнала (во время записи это номер последнего запсанного сигнала)
		pnG = Graphs->CrntSig->Tag - 1;//номер точки (сигнала) на графике

		//определяем каналы
		for (i = 0; i < Graphs->ftChan; i++)//инициализация
			chN[i] = false;//инициализация

		flag = false;//для всех каналов
		j = addUMark->Text.AnsiPos(":");//определяем положение разделителя
		infoCopy = addUMark->Text;
		i = 2;
		while (j >= i)
		{
			i = infoCopy.AnsiPos(",");//определяем положение разделителя номеров каналов
			if ((i == 0) || (i > j))
				i = j;//указан один номер канала
			j -= i;//поправка на прочтённый номер
			aNumber = infoCopy;
			infoCopy.Delete(1, i);//удаляем прочитанный номер канала
			aNumber.Delete(i, aNumber.Length());//вырезаем число из начала строки
			k = StrToInt(aNumber) - 1;
			if ((k >= 0) && (k < Graphs->ftChan))//если номер канала входит в диапазон
			{
				chN[k] = true;//выбранный канал
				flag = true;//не для всех каналов
			}
		}
		if (!flag)
			for (i = 0; i < Graphs->ftChan; i++)//инициализация
				chN[i] = true;//метка видна на всех каналах

		AddMarker(infoCopy, pnG, &chN[0]);//добавление текста метки в массив
		addUMark->Text = "";//зануляем строку для меток
	}
}
//---------------------------------------------------------------------------

void TExpNotes::AddMarker(AnsiString addedMark, __int32 pnG, bool *chN)
{
	//добавляем метки на график с главными амплитудами (пока только туда)
	/*
	addedMark - текст добавляемой метки
	pnG - номер сигнала на графике, к которому прикрепляется метка
	chN - указатель на массив с видимостями меток на каналах
	*/
	__int32 i, j, k,
			mij;
	bool flag1, flag2,
		 all0,
		 mNew1, mNew2;
	markers *eC[maxChannels];//указатели на метки

	for (i = 0; i < Graphs->ftChan; i++)
		eC[i] = NULL;//обнуляем указатели

	flag1 = false;//при совпадении надписей станет true
	flag2 = false;//при не совпадении надписей станет true
	mij = 0;
	for (i = 0; i < addUMark->Tag; i++)//проверим, есть ли уже на данном номере сигнала метки
	{
		if (pnG == theMarker->pointOnGraph)//номера сигналов совпали (проверим каналы)
			if (theMarker->textMark == addedMark)//надписи совпадают
			{
				flag1 = true;//есть такая метка на данном номере сигнала
				k = i;//уникальный номер (нет разных меток с одинаковыми текстами и номерами каналов)
				eC[mij] = theMarker;//сохраняем адрес метки для последующего редактирования
				mij++;
			}
			else//введён другой текст
			{
				flag2 = true;//есть другая метка на данном номере сигнала
				eC[mij] = theMarker;//сохраняем адрес метки для последующего редактирования
				mij++;
			}
		theMarker = theMarker->nextM;//переходим к следующей метке
	}

	if (!flag1 && !flag2 && !addedMark.IsEmpty())//создаём новую метку (добавляем элемент в циклический динамический массив theMarker)
	{
		CreateAMark();//создание маркера: добавление элемента в циклический динамический массив theMarker
		theMarker->textMark = addedMark;//текст метки
		theMarker->pointOnGraph = pnG;//номер сигнала на графике
		for (i = 0; i < Graphs->ftChan; i++)
			theMarker->chanN[i] = chN[i];//номера каналов, на которых отображается метка
	}
	else if (flag1 && flag2)//есть такая метка, и есть другие метки (пустых меток не бывает)
	{
		for (i = 0; i < mij; i++)//перебираем запомненные элементы массива theMarker
			for (j = 0; j < Graphs->ftChan; j++)
				if (chN[j])
				{
					eC[i]->textMark = addedMark;//меняем текст метки на указанных каналах
					eC[i]->chanN[j] = true;//устанавливаем привязку метки к каналу
				}
	}
	else if (!flag1 && flag2)//нет такой метки, но есть другие метки
	{
		if (addedMark.IsEmpty())//введена пустая строка (зануление-удаление меток)
		{
			for (i = 0; i < mij; i++)
				for (j = 0; j < Graphs->ftChan; j++)
					if (chN[j])
						eC[i]->chanN[j] = false;//нет меток на данном канале
		}
		else//введена не пустая строка
		{
			mNew1 = false;//нет сигнала о необходимости создания новой метки
			mNew2 = false;//изменения учтены
			for (i = 0; i < mij; i++)
			{
				for (j = 0; j < Graphs->ftChan; j++)
					if (chN[j])//на канале должна появиться новая метка
						eC[i]->chanN[j] = false;//на этом канале отобразится другая метка
					else if (!chN[j] && eC[i]->chanN[j] && !mNew2)//не совпадают массивы видимостей
						mNew1 = true;//сигнал о необходимости создания новой метки
				if (mNew1 && !mNew2)//нужно добавить другую надпись, поэтому создадим элемент массива theMarker
				{
					all0 = true;//проверим, не нужно ли вообще удалить метку (укоротить массив theMarker)
					for (j = 0; j < Graphs->ftChan; j++)
						all0 = (all0 && (!eC[i]->chanN[j]));
					if (all0)//все элементы массива chanN обращаются в false, тогда используем эту метку для создания новой
					{
						eC[i]->textMark = addedMark;
						for (j = 0; j < Graphs->ftChan; j++)
							eC[i]->chanN[i] = chN[i];//номера каналов, на которых отображается метка
					}
					else
					{
						CreateAMark();//создание маркера: добавление элемента в циклический динамический массив theMarker
						theMarker->textMark = addedMark;//текст метки
						theMarker->pointOnGraph = pnG;//номер сигнала на графике
						for (i = 0; i < Graphs->ftChan; i++)
							theMarker->chanN[i] = chN[i];//номера каналов, на которых отображается метка
					}
					mNew2 = true;//изменения учтены
				}
			}
		}
	}
	else if (flag1 && !flag2)//есть такая метка, и других меток нет (меняем отображение на каналах)
		for (i = 0; i < Graphs->ftChan; i++)
			eC[k]->chanN[i] = (eC[k]->chanN[i] || chN[i]);//привязываем метку к другим каналам (k определили выше)

	//удалим лишние совпадющие и пустые элементы массива theMarker
	for (i = 0; i < mij; i++)
		if (eC[i])
		{
			for (j = (i + 1); j < mij; j++)
				if (eC[j])
					if ((eC[i]->pointOnGraph == eC[j]->pointOnGraph) && (eC[i]->textMark == eC[j]->textMark))
					{
						DeleteAMark(eC[j]);//удаляем лишний элемент массива theMarker
						eC[j] = NULL;//обнуляем указатель на исключённый элемент
					}

			all0 = true;//проверим, не нужно ли вообще удалить метку (укоротить массив theMarker)
			for (j = 0; j < Graphs->ftChan; j++)//по количеству используемых каналов!
				all0 = (all0 && (!eC[i]->chanN[j]));
			if (all0)//все элементы массива chanN обращаются в false, тогда удаляем метку (укорачиваем массив theMarker)
			{
				DeleteAMark(eC[i]);//удаляем пустой элемент массива theMarker
				eC[j] = NULL;//обнуляем указатель на исключённый элемент
			}
		}

	Graphs->amplInTAfterDraw(this);//запускаем перерисовку графиков с метками
}
//---------------------------------------------------------------------------

void TExpNotes::CreateAMark()
{
	//создание маркера: добавление элемента в циклический динамический массив theMarker
	markers *buf;//копия указателя на текущую метку (может быть нулём)

	buf = theMarker;//копия указателя на текущую метку (может быть нулём)
	theMarker = new markers[1];//создаём элемент циклического динамического массива с метками
	if (addUMark->Tag == 0)//нет добавленных меток
	{
		theMarker->nextM = theMarker;//единственная метка ссылается на себя
		theMarker->prevM = theMarker;//единственная метка ссылается на себя
	}
	else
	{
		theMarker->nextM = buf->nextM;//новый указывает на следующего после текущего
		theMarker->prevM = buf;//новый указывает на текущий
		buf->nextM->prevM = theMarker;//следующий после текущего указывает на новый
		buf->nextM = theMarker;//текущий указывает на новый
	}
	addUMark->Tag++;//увеличиваем счётчик меток
}
//---------------------------------------------------------------------------

void TExpNotes::DeleteAMark(markers *delM)
{
	//удаление заданной метки //корректируем динамический массив (укорачиваем его)
	/*
	delM - указатель на удаляему метку
	*/

	if ((addUMark->Tag > 0) && (theMarker))
	{
		delM->prevM->nextM = delM->nextM;//предыдущий будет указывать на следующего
		delM->nextM->prevM = delM->prevM;//следующий будет указывать на предыдущего
		delete[] delM;

		addUMark->Tag--;//количество меток уменьшилось
		if (addUMark->Tag == 0)
			theMarker = NULL;
	}
}
//---------------------------------------------------------------------------

void TExpNotes::DeleteMarkers()
{
	//удаляем заметки графика из массива
	while ((addUMark->Tag > 0) && theMarker)
		DeleteAMark(theMarker);
}
//---------------------------------------------------------------------------

