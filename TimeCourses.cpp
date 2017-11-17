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

const short eheadSBlock = 12,//количество блоков в структуре ehead
			dataInHead = 5,//количество блоков, данные которых есть только в заголовке
			pureData = eheadSBlock - dataInHead;//количество блоков с экспериментальными данными
blockbyte ehead[eheadSBlock];//задаём структуру файлов с данными

trac *gSignals;//структура для сигналов с их параметрами

__int32 *devExpParams_int,//параметры устройства и эксперимента (__int32)
		gPreTime,//начало пресин. ответа (микросекунды)
		gPostTime,//начало постсин. ответа (микросекунды)
		gTimeBgn;//начало рисования (микросекунды)
short experimentType,//тип эксперимента (глобальный вариант)
	  gPorog;//порог (отсчёты)
float *devExpParams_float;//параметры устройства и эксперимента (float)
bool mask[9],//маска "открытости" графиков для режима равёртывания
	 _isfull;//развёрнут ли какой-либо график
	 
//---------------------------------------------------------------------------
__fastcall TGraphs::TGraphs(TComponent* Owner)
	: TForm(Owner)
{
	short i;
	//--------------------------------------------------------------
	//--- константная часть файл. Новый формат *.pra ---
	i = 0;
	ehead[i].nob = "название файла, версия программы, время создания и время последнего редактирования файла";
	ehead[i].byteInHead = 0;
	ehead[i].byteInFile = 0;
	ehead[i].szDataType = sizeof(char);//данные только в заголовке
	ehead[i].szInHDataType = ehead[i].szDataType;
	ehead[i].numOfElements = 64;
	//--------------------------------------------------------------
	i++;//1
	ehead[i].nob = "размер блока (в байтах) с информацией об эксперименте, вводимой пользователем";//
	ehead[i].szDataType = sizeof(__int32);
	ehead[i].numOfElements = 1;
	//--------------------------------------------------------------
	i++;//2
	ehead[i].nob = "__int32 параметры сбора данных (ADCParametersDMA а) + тип эксперимента + параметры эксперимента";
	ehead[i].szDataType = sizeof(__int32);
	ehead[i].numOfElements = 15;
	//--------------------------------------------------------------
	i++;//3
	ehead[i].nob = "float параметры сбора данных (ADCParametersDMA а) + тип эксперимента + параметры эксперимента";
	ehead[i].szDataType = sizeof(float);
	ehead[i].numOfElements = 6;
	//--------------------------------------------------------------
	i++;//4
	ehead[i].nob = "число записанных сигналов";
	ehead[i].szDataType = sizeof(__int32);
	ehead[i].numOfElements = 1;
	//--------------------------------------------------------------
	//--- информация о размещении данных измерения ---
	//параметры byteInFile и numOfElements для следующих блоков рассчитываются при сохранении данных
	i++;//5
	//(1)
	ehead[i].nob = "номер первого байта блока с развёртками сигналов и значениями параметров (все каналы)";//(1)
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//условный размер
	//--------------------------------------------------------------
	i++;//6
	ehead[i].nob = "резерв (номер первого байта какого-нибудь блока)";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//условный размер
	//--------------------------------------------------------------
	i++;//7
	ehead[i].nob = "резерв (номер первого байта какого-нибудь блока)";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//условный размер
	//--------------------------------------------------------------
	i++;//8
	ehead[i].nob = "номер первого байта блока с информацией об эксперименте, вводимой пользователем";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//размер
	//--------------------------------------------------------------
	i++;//9
	ehead[i].nob = "номер первого байта блока с номерами сигналов, к которым привязаны заметки";//
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(__int32);//размер
	//--------------------------------------------------------------
	i++;//10
	ehead[i].nob = "номер первого байта блока с видимостями меток на каналах";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(bool);//размер
	//--------------------------------------------------------------
	i++;//11
	ehead[i].nob = "резерв (номер первого байта какого-нибудь блока)";
	ehead[i].szInHDataType = sizeof(unsigned long);
	ehead[i].szDataType = sizeof(char);//условный размер
	//====================================================================
	//рассчитаем размеры в байтах
	for (i = 1; i < eheadSBlock; i++)
		if (i < dataInHead)//данные, располагаемые только в заголовке
		{
			ehead[i].szInHDataType = ehead[i].szDataType;
			ehead[i].byteInHead = ehead[i - 1].byteInHead + (ehead[i - 1].numOfElements * ehead[i - 1].szInHDataType);
			ehead[i].byteInFile = ehead[i].byteInHead;
		}
		else//все остальные
			ehead[i].byteInHead = ehead[i - 1].byteInHead + (1 * ehead[i - 1].szInHDataType);//1 элемента

	//прописываем названия колонок таблицы с параметрами сигналов
	pValues->ColCount = 5;//10 + maxChannels;//18;//количество колонок в таблице по количеству параметров
	pValues->Cells[0][0] = "№сигнала";
	pValues->Cells[1][0] = "время, с";
	pValues->Cells[2][0] = "ампл, мВ";//амплитуда основная - постсинаптическая (после красной линии)
	pValues->Cells[3][0] = "rise, мкс";//время нарастания (микросекунды)
	pValues->Cells[4][0] = "спад, мкс";//время спада (микросекунды)
	pValues->ColWidths[0] = (pValues->Cells[0][0].Length()) * 7;//первые две колонки всегда видны
	pValues->ColWidths[1] = (pValues->Cells[1][0].Length()) * 7;//первые две колонки всегда видны

	//заполняем массив копий указателей на графики (чарты)
	allCharts[0] = SignalTrack;//текущий (средний) сигнал
	allCharts[1] = AmplInT;//главная амплитуда
	allCharts[2] = RiseTInT;//время нарастания
	allCharts[3] = FallTInT;//время спада
	allCharts[4] = NaAmplInT;//амплитуда Na тока
	allCharts[5] = NaDurInT;//длительность натриевого тока
	allCharts[6] = KAmplInT;//амплитуда K тока
	allCharts[7] = SynDelInT;//синаптическая задержка
	allCharts[8] = SynTrnsInT;//время синаптического проведения

	AmplInT->Canvas->Font->Name = "Arial";//определяем стиль надписей (заметок по ходу эксперимента)

	gSignals = NULL;//структура для сигналов с их параметрами
	//Graphs->"чарты с графиками"->Tag - номер элемента в массиве Graphs->allCharts
}
//---------------------------------------------------------------------------

bool TGraphs::SaveExpDataToFile(short expType, trac *signals_loc, __int32 numOfSignal, bool saveType)
{
	//сохранение в новом формате (новая структура фалов *.pra)
	/*
	expType - тип эксперимента
	signals_loc - локальная ссылка на структуру с сигналами
	numOfSignal - количество собранных сигналов
	saveType - тип сохранения (true = сохранение только-что проведённого эксперимента)
	*/

	void *refToWrite[eheadSBlock];//массив ссылок на блоки записываемых данных
	unsigned long bytesCounter,//счётчик байтов; используется при задании структуры ehead
				  c,
				  m_nWritten,//число записанных байт
				  totWrit;//всего записано байт
	__int32 i, j, g,
			*sigsWithMark,//номера сигналов, к которым привязаны метки
			fullSigLen,//полная длина записи на всех каналах (отсчёты)
			dbN;//номер блока данных (data block number)
	HANDLE hFile;//хэндл файла, в который сохраняются данные
	TDateTime CurrentDateTime;//текущие дата и время
	AnsiString progDate,//информация о программе и дате создания файла
			   userText,//информация об эксперименте от пользователяэ
			   versName;//имя программы, версия
	float frstSignAppear;//время возникновения первого сигнала (в сохраняемом сеансе записи)
	bool *markerVsblt;//видимость меток на каналах

	if (expType < 5)
		experimentType = expType;//копируем тип эксперимента в глобальную переменную
	else//if expType == 5 (т.е. спонтанные сигналы в режиме №4)
		experimentType = 1;

	if (saveType)//сохранение только-что проведённого эксперимента
	{
		SigNumUpDwn->Max = numOfSignal;//задаём пределы изменения номеров сигналов (оставшихся после редактирования)

		gSignals = signals_loc;//копируем разом всю информацию о сигналах
		signals_loc = NULL;//тут же обнуляем локальную ссылку

		gRF[0] = 0;//первый отображаемый канал
		gRF[1] = 1;//последний отображаемый канал (отображаем какбы один, первый, канал)
		ChannNumb[0]->Checked = true;//в начале выбранным является нулевой канал
		
		gInd = new __int32[numOfSignal];//создаём массив индексов
		for (i = 0; i < numOfSignal; i++)
			gInd[i] = i;//заполняем массив индексов

		//рассчитаем параметры
		gPreTime = StrToInt(PStart->PreTime->Text);//ПРЕ-время в микросекундах
		gPostTime = StrToInt(PStart->PostTime->Text);//ПОСТ-время в микросекундах
		gPorog = short((float)StrToInt(PStart->Porog->Text) / sampl2mV);//пороговое напряжение (отсчёты)

		//определяем начальный индекс для рисования
		gTimeBgn = 0;
		if (experimentType == 2)
			gTimeBgn = gPostTime;
		else if (experimentType == 3)
			gTimeBgn = gPreTime;
	}

	//--------------------------------------------
	//название файла, версия программы, время создания и время последнего редактирования файла
	CurrentDateTime = Now();//текущие дата и время
	versName = "ElphAcqu v";//версия программ
	versName += progVer;//версия программ
	if (saveType)//сохранение только-что проведённого эксперимента
	{
		progDate = versName;
		progDate += "\r\nmade";
		progDate += CurrentDateTime.DateTimeString().c_str();//19 символов
		progDate += "\r\nedit";
		progDate += CurrentDateTime.DateTimeString().c_str();//19 символов
	}
	else //сохранение отредактированного файла
	{
		progDate = ExpNotes->pIDateString;//редактируем запись о датах создания и изменения файла

		j = progDate.AnsiPos("\r\nmade");//пытаемся найти другую метку
		if (j > 0)//если метка наидена
		{
			progDate.Delete(j + 6 + 19, progDate.Length());//удаляем дату изменения
			progDate.Delete(1, j - 1);//удаляем начальную часть записи
			progDate.Insert(versName, 1);
		}
		else//если метки нет "made"
		{
			progDate = versName;
			progDate += "\r\nmade";
			progDate += CurrentDateTime.DateTimeString().c_str();//19 символов
		}

		progDate += "\r\nedit";
		progDate += CurrentDateTime.DateTimeString().c_str();//19 символов
	}
	//====================================================
	//информация о программе и дате создания данного файла
	g =  ehead[0].numOfElements - progDate.Length();
	if (g < 0)//неправильная длина строки
		progDate.SetLength(ehead[0].numOfElements);
	else if (g > 0)
		progDate += AnsiString::StringOfChar('_', g);
	ExpNotes->pIDateString = progDate;//переписываем информацию о файле

	userText = ExpNotes->usersNotes->Lines->Text;//информация об эксперименте (вводит пользователь)
	if ((ExpNotes->addUMark->Tag > 0) && (expType != 5))//в userText вставляем и заметки по ходу эксперимента
	{
		userText += "\n\n\r\r\n\n";//разделитель
		sigsWithMark = new __int32[ExpNotes->addUMark->Tag];//номера сигналов, к которым привязаны метки
		markerVsblt = new bool[ExpNotes->addUMark->Tag * ftChan];//видимость меток на каналах
		for (i = 0; i < ExpNotes->addUMark->Tag; i++)
		{
			sigsWithMark[i] = ExpNotes->theMarker->pointOnGraph;
			for (j = 0; j < ftChan; j++)
				markerVsblt[(i * ftChan) + j] = ExpNotes->theMarker->chanN[j];//видимость меток на каналах
			userText += ExpNotes->theMarker->textMark;//текст надписи
			userText += "||\n||";
			ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
		}
	}

	//=====================================
	//параметры сбора данных и прочее
	if (saveType)//сохранение только-что проведённого эксперимента
	{
		//__int32 - параметры
		devExpParams_int = new __int32[ehead[2].numOfElements];//создаём массив с параметрами устройства и эксперимента (__int32)
		devExpParams_int[0]	= multiCh;//0//режим сбора данных (true = многоканальный)
		devExpParams_int[1]	= recLen;//1//длина развёртки сигнала в отсчётах
		devExpParams_int[2] = (__int32)discrT;//2//время дискретизации (микросекунды)
		devExpParams_int[3]	= chanls;//3//количество сканируемых каналов
		devExpParams_int[4]	= Experiment->uiLeadChan->ItemIndex;//4//ведущий канал (основной сигнал)
		devExpParams_int[5]	= Experiment->a.m_nGain;//5//коэффициент усиления
		devExpParams_int[6] = ExpNotes->addUMark->Tag * __int32(expType != 5);//6//количество заметок
		devExpParams_int[7] = (__int32)experimentType;//7//тип эксперимента, глобальный вариант
		devExpParams_int[8] = gPreTime;//8//gPreTime (в микросекундах)
		devExpParams_int[9] = gPostTime;//9//gPostTime (в микросекундах)
		devExpParams_int[10] = (__int32)gPorog;//10//амплитудный порог (отсчёты)
		devExpParams_int[11] = Experiment->maxADCAmp;//11//максимальная амплитуда (в отсчётах)
		devExpParams_int[12] = Experiment->minADCAmp;//12//минимальная амплитуда (в отсчётах)
		devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//ведущий аппаратный канал
		devExpParams_int[14] = 0;//14//не используется

		//float - параметры
		devExpParams_float = new float[ehead[3].numOfElements];//создаём массив с параметрами устройства и эксперимента (float)
		devExpParams_float[0] = sampl2mV;//0//коэффициент перевода амплитуд из отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
		devExpParams_float[1] = Experiment->maxVoltage;//1//диапазон входных напряжений (в милливольтах)
		devExpParams_float[2] = StrToFloat(Experiment->StimPeriod->Text);//2//период стимуляции
		devExpParams_float[3] = discrT;//3//время дискретизации (микросекунды)
		devExpParams_float[4] = 0;//4//не используется
		devExpParams_float[5] = 0;//5//не используется
	}
	else//пересохранение
	{
		devExpParams_int[6] = ExpNotes->addUMark->Tag * (__int32(expType != 5));//количество заметок (меняется)
		devExpParams_int[7] = (__int32)experimentType;//тип эксперимента можно поменять (при редактировании)
	}
	fullSigLen = (recLen * ftChan) + (chanls - ftChan);//суммарная длина развёрток на всех каналах (отсчёты)

	dbN = dataInHead;//5//объём экспериментальных данных (все каналы)
	ehead[dbN].numOfElements = numOfSignal * (sizeof(float) + (fullSigLen * sizeof(short)));//длина всех развёрток сигналов на всех каналах

	dbN++;//6//резерв (объём какого-нибудь блока данных)
	ehead[dbN].numOfElements = 0;
	dbN++;//7//резерв (объём какого-нибудь блока данных)
	ehead[dbN].numOfElements = 0;

	dbN++;//8//количество символов в пользовательской "строке" (userText)
	ehead[dbN].numOfElements = userText.Length();
	dbN++;//9//номера сигналов с метками
	ehead[dbN].numOfElements = ExpNotes->addUMark->Tag * (__int32(expType != 5));
	dbN++;//10//видимость меток на каналах
	ehead[dbN].numOfElements = ftChan * ehead[dbN - 1].numOfElements;

	dbN++;//11//резерв (объём какого-нибудь блока данных)
	ehead[dbN].numOfElements = 0;

	//посчитаем byteInFie
	ehead[dataInHead].byteInFile = ehead[eheadSBlock - 1].byteInHead + 1 * ehead[eheadSBlock - 1].szInHDataType;//byteInHead последнего + 4 байта
	bytesCounter = ehead[dataInHead].byteInFile;
	for (i = dataInHead + 1; i < eheadSBlock; i++)
	{
		c = ehead[i - 1].numOfElements * ehead[i - 1].szDataType;
		bytesCounter += c;
		if (ehead[i].numOfElements > 0)
			ehead[i].byteInFile = bytesCounter;
		else
			ehead[i].byteInFile = 0;//нечего записывать
	}

	//запишем ссылки на переменные и массивы (записываемые в заголовок файла)
	dbN = 0;//0//данные о файле
	refToWrite[dbN] = progDate.c_str();//данные о файле
	dbN++;//1//количество символов в пользовательской "строке" (userText)
	refToWrite[dbN] = (__int32*)&ehead[8].numOfElements;//количество байт в userText (8 = eheadSBlock - 4)
	//ссылки на массивы с параметрами и номера байтов
	dbN++;//2//все int-параметры
	refToWrite[dbN] = (__int32*)devExpParams_int;//int-параметры
	dbN++;//3//все float-параметры
	refToWrite[dbN] = (float*)devExpParams_float;//float-параметры
	dbN++;//4//количество сигналов
	refToWrite[dbN] = (__int32*)&numOfSignal;//количество сигналов
	//экспериментальные данные
	dbN++;//5//номер первого байт блока с экспериментальными данными
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;

	dbN++;//6//резерв (номер первого байт какого-нибудь блока данных)
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;
	dbN++;//7//резерв (номер первого байт какого-нибудь блока данных)
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;

	dbN++;//8//номер первого байт блока с текстом, введённым пользователем
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;
	dbN++;//9//номер первого байт блока с номерами сигналов, к которым прикреплены заметки
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;
	dbN++;//10//номер первого байт блока с видимостями меток на каналах
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;

	dbN++;//11//резерв (номер первого байт какого-нибудь блока данных)
	refToWrite[dbN] = (unsigned long*)&ehead[dbN].byteInFile;

	//=========================================
	//файл с расширением *.pra (p - physiological, r - reaction, a - acquisition)
	hFile = CreateFile(SaveDlg->FileName.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);//создаём-открываем файл для записи
	//=============================

	m_nWritten = 0;//число записанных байт
	totWrit = 0;//всего записано байт

	//записываем заголовок файла
	for (i = 0; i < eheadSBlock; i++)
	{
		if (refToWrite[i])
		{
			if ((unsigned long)ehead[i].byteInHead != totWrit)
			{
                Experiment->DevEvents->Text = "ошибка сохранения";
				return false;
			}

			if (i < dataInHead)//данные, располагаемые только в заголовке
				WriteFile(hFile, refToWrite[i], (ehead[i].szInHDataType * ehead[i].numOfElements), &m_nWritten, 0);
			else//указатели на положения блоков в теле файла
				WriteFile(hFile, refToWrite[i], (ehead[i].szInHDataType * 1), &m_nWritten, 0);

			totWrit += m_nWritten;
			refToWrite[i] = NULL;//на всякий случай зануляем ссылку
		}
	}
	//записываем сигналы
	dbN = dataInHead;
	frstSignAppear = gSignals[gInd[0]].appear;//время возникновения первого сигнала (в данной записи)
	for (i = 0; i < numOfSignal; i++)
	{
		//развёртки сигналов на всех каналах
		gSignals[gInd[i]].appear -= frstSignAppear;//смещаем времена возникновения на время первого сигнала
		WriteFile(hFile, (void*)&gSignals[gInd[i]].appear, sizeof(float), &m_nWritten, 0);
		totWrit += m_nWritten;
		WriteFile(hFile, (void*)&gSignals[gInd[i]].s[0], sizeof(short) * fullSigLen, &m_nWritten, 0);
		totWrit += m_nWritten;
	}
	dbN++;//6//резерв
	//WriteFile(hFile, Массив, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0); totWrit += m_nWritten;
	dbN++;//7//резерв
	//WriteFile(hFile, Массив, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0); totWrit += m_nWritten;

	//записываем текстовую информацию, введённую пользователем
	dbN++;//8//пользовательские данные
	if (ehead[dbN].numOfElements > 0)//!userText.IsEmpty()//строка не пустая
	{
		WriteFile(hFile, (void*)userText.c_str(), (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0);
		totWrit += m_nWritten;
	}

	if ((ExpNotes->addUMark->Tag > 0) && (expType != 5))
	{
		dbN++;//9//номера сигналов с заметками
		WriteFile(hFile, (void*)&sigsWithMark[0], (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0);
		totWrit += m_nWritten;

		dbN++;//10//видимость меток на каналах
		WriteFile(hFile, (void*)&markerVsblt[0], (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0);
		totWrit += m_nWritten;

		delete[] sigsWithMark; sigsWithMark = NULL;//номера сигналов, которым привязаны метки
		delete[] markerVsblt; markerVsblt = NULL;//видимость меток на каналах
	}
	else
		dbN += 2;//необходимое приращение индекса

	dbN++;//11//резерв
	//WriteFile(hFile, Массив, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &m_nWritten, 0); totWrit += m_nWritten;

	CloseHandle(hFile); hFile = NULL;//закрываем файл

	//сравниваем число записанных байти, с общим размером данных (в байтах)
	m_nWritten = 0;
	for (g = 0; g < eheadSBlock; g++)
	{
		if (g > (dataInHead - 1))//данные есть в заголовке и в теле файла
			m_nWritten += ehead[g].szInHDataType * 1;
		m_nWritten += ehead[g].szDataType * ehead[g].numOfElements;
	}

	if (m_nWritten != totWrit)//если не совпало, значит где-то ошиблись
	{
		Experiment->DevEvents->Text = "ошибка сохранения";
		return false;
	}

	//рассчитаем параметры сигналов для отображения
	//Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
	//		    __int32 *inds, short expT, bool newNull, short chn, sPr *avrP)
	Calculate(gSignals, numOfSignal, gPorog, gPreTime, gPostTime, gInd, experimentType, false, NULL);
	Graphs->Caption = "Графики - " + SaveDlg->FileName;//дополняем название окна названием файла

	return true;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ReadDataFile(TObject *Sender)
{
	//чтение файла с данными (файл типа *.pra)
	bool forwc,//индикатор успешности чтения блока данных из файла
		 *markerVsblt,//видимость меток на каналах
		 maxMultiChann,//многоканальность (хотябы для одного сигнала)
		 mOnChn[maxChannels];//привязка меток к каналам
	unsigned long bytesRead,
				  totRead,//всего прочитано байт
				  *beginBytes;//массив с номерами начальных байтов блоков с экспериментальными данными
	__int32 i, j, k, z,
			fullSigLen,//суммарная длина сигналов со всех каналов
			dbN,//номер блока данных
			*sigsWithMark,//номера сигналов, к которым привязаны метки
			lettersNum,//количество символов в тексте пользователя
			maxToRec,//максимальная длина сигнала (в отсчётах)
			maxChanNumb,//максимальное количество каналов
			numOfSignal;//число сигналов
	AnsiString allUsersTxt,//информация, вводимая пользователем
			   usersComent,//общий коментарий
			   usersMarks,//заметки по ходу эксперимента
			   singlMark;//нарезаем заметки
	char *unote,//информация, вводимая пользователем
		 *pidate;//информация о программе и даты (Programm Information and Date)
	short expType;//тип эксперимента
	HANDLE hFile;//хэндл файла, из которого читаем данные

	totRead = 0;//всего прочитано байт
	beginBytes = NULL; sigsWithMark = NULL; markerVsblt = NULL; unote = NULL; pidate = NULL;

	//открываем и читаем файл с экспериментальными данными
	hFile = CreateFile(OpenDlg->FileName.c_str(), GENERIC_READ, FILE_SHARE_READ,
				   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	pidate = new char[ehead[0].numOfElements];//готовим массивы для записи данных из файла

	//чтение блока №0 (самое начало)
	dbN = 0;
	forwc = ReadFile(hFile, (void*)pidate, (ehead[dbN].numOfElements * ehead[dbN].szDataType), &bytesRead, 0);
	totRead += bytesRead;
	if (!forwc)
	{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

	//проверяем версию формата файла
	singlMark = pidate;
	j = singlMark.AnsiPos("ElphAcqu v3.9");//поддерживаемый формат
	j += singlMark.AnsiPos("ElphAcqu v4.");//поддерживаемый формат
	j += singlMark.AnsiPos("ElphAcqu v5.");//поддерживаемый формат
	if (j <= 0)//нет такой строки в файле
	{
		delete[] pidate; pidate = NULL;
		CloseHandle(hFile);//закрываем файл и удаляем все объекты созданные оператором new
		Experiment->DevEvents->Text = "формат не поддерживается";//ReadOldFormatFile(this);//читаем как pra-файл старого формата
		return;
    }

	ExpNotes->PIDates->Lines->SetText(pidate);//загружаем текст в поле типа TMemo

	//готовим массивы для записи данных из файла
	beginBytes = new unsigned long[pureData];//храним pureData измеряемых величин (пока не больше)
	devExpParams_int = new __int32[ehead[2].numOfElements];//создаём массив с параметрами устройства и эксперимента (__int32)
	devExpParams_float = new float[ehead[3].numOfElements];//создаём массив с параметрами устройства и эксперимента (float)

	dbN++;//1//чтение блока №1
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

	lettersNum = 0;
	forwc = ReadFile(hFile, (void*)&lettersNum, (ehead[dbN].numOfElements * ehead[dbN].szDataType), &bytesRead, 0);
	totRead += bytesRead;

	dbN++;//2//чтение блока №2
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

	forwc = ReadFile(hFile, (void*)devExpParams_int, (ehead[dbN].numOfElements * ehead[dbN].szDataType), &bytesRead, 0);
	totRead += bytesRead;

	dbN++;//3//чтение блока №3
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

	forwc = ReadFile(hFile, (void*)devExpParams_float, (ehead[dbN].numOfElements * ehead[dbN].szDataType), &bytesRead, 0);
	totRead += bytesRead;

	dbN++;//4//чтение блока №4 (количество сигналов)
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

	numOfSignal = 0;
	forwc = ReadFile(hFile, (void*)&numOfSignal, 1 * ehead[dbN].szDataType, &bytesRead, 0);
	totRead += bytesRead;

	SigNumUpDwn->Tag = numOfSignal;//храним исходное число сигналов (нужно при удалении ссылок)
	SigNumUpDwn->Max = numOfSignal;//задаём пределы изменения номеров сигналов (оставшихся после редактирования)

	/* последовательность данных в массивах devExpParams_int и devExpParams_float
	//__int32 - параметры
	devExpParams_int[0]	= multiCh;//0//режим сбора данных (true = многоканальный)
	devExpParams_int[1]	= recLen;//1//длина развёртки сигнала в отсчётах
	devExpParams_int[2] = (__int32)discrT;//2//время дискретизации (микросекунды)
	devExpParams_int[3]	= chanls;//3//количество сканируемых каналов
	devExpParams_int[4]	= Experiment->uiLeadChan->ItemIndex;//4//ведущий канал (основной сигнал)
	devExpParams_int[5]	= Experiment->adcGain;//5//коэффициент усиления
	devExpParams_int[6] = ExpNotes->addUMark->Tag * (__int32(expType != 5));//6//количество заметок
	devExpParams_int[7] = experimentType;//7//тип эксперимента, глобальный вариант
	devExpParams_int[8] = gPreTime;//8//gPreTime (в микросекундах)
	devExpParams_int[9] = gPostTime;//9//gPostTime (в микросекундах)
	devExpParams_int[10] = (__int32)gPorog;//10//амплитудный порог (отсчёты)
	devExpParams_int[11] = Experiment->maxADCAmp;//11//максимальная амплитуда (в отсчётах)
	devExpParams_int[12] = Experiment->minADCAmp;//12//минимальная амплитуда (в отсчётах)
	devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//ведущий аппаратный канал
	devExpParams_int[14] = 0;//14//не используется

	//float - параметры
	devExpParams_float[0] = sampl2mV;//0//коэффициент перевода амплитуд из отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
	devExpParams_float[1] = Experiment->maxVoltage;//1//диапазон входных напряжений (в милливольтах)
	devExpParams_float[2] = StrToFloat(Experiment->stimPeriod->Text);//2//период стимуляции
	devExpParams_float[3] = discrT;//3//время дискретизации (микросекунды)
	devExpParams_float[4] = 0;//4//не используется
	devExpParams_float[5] =	0;//5//не используется*/

	//атрибуты сигналов
	multiCh = (bool)devExpParams_int[0];//режим сбора данных (true = многоканальный)
	recLen = devExpParams_int[1];//длина развёртки сигнала в отсчётах
	discrT = devExpParams_float[3];//время дискретизации (микросекунды)
	if (discrT < 1e-6)//читай если discrT == 0
		discrT = devExpParams_int[2];//время дискретизации (микросекунды)
	chanls = devExpParams_int[3];//количество сканируемых каналов
	ftChan = 1 + (chanls - 1) * (__int32)multiCh;//количество каналов с полными развёртками
	effDT = discrT * (float)chanls;//эффективное время дискретизации = discrT * chanls
	fullSigLen = (recLen * ftChan) + (chanls - ftChan);//суммарная длина развёрток на всех каналах (отсчёты)
	sampl2mV = devExpParams_float[0];//коэффициент перевода амплитуд из отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
	if (fabs(sampl2mV - (devExpParams_float[1] / (devExpParams_int[11] * devExpParams_int[5]))) > 1e-4)//не сходится
	{
		sampl2mV = devExpParams_float[1] / float(devExpParams_int[11] * devExpParams_int[5]);//коэффициент усиления
		Experiment->DevEvents->Text = "коэффициент";
	}
	//другие важные параметры
	experimentType = (short)devExpParams_int[7];//копируем тип эксперимента в глобальную переменную
	gPreTime = devExpParams_int[8];//ПРЕ-время (микросекунды)
	gPostTime = devExpParams_int[9];//ПОСТ-время (микросекунды)
	gPorog = (short)devExpParams_int[10];//амплитудный порог (отсчёты)
	//определяем начальный индекс для рисования
	gTimeBgn = 0;
	if (experimentType == 2)
		gTimeBgn = gPostTime;
	else if (experimentType == 3)
		gTimeBgn = gPreTime;

	//чтение номеров байтов с блоками экспериментальных данных
	//эти pureData штук элементов имеют формат unsigned long
	dbN = dataInHead;//5
	if ((unsigned long)ehead[dbN].byteInHead != totRead)
	{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

	forwc = ReadFile(hFile, (void*)beginBytes, pureData * ehead[dbN].szInHDataType, &bytesRead, 0);
	totRead += bytesRead;

	gSignals = CreatStructSignal(numOfSignal, recLen);//создаём структуру с параметрами сигналов
	gInd = new __int32[numOfSignal];//создаём массив с номерами доступных сигналов

	//считываем сигналы и параметры
	dbN = dataInHead;
	if (beginBytes[dbN - dataInHead] != totRead)
	{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

	for (i = 0; i < numOfSignal; i++)
	{
		gInd[i] = i;//заполняем массив с номерами доступных сигналов
		ReadFile(hFile, (void*)&gSignals[i].appear, sizeof(float), &bytesRead, 0);
		totRead += bytesRead;
		ReadFile(hFile, gSignals[i].s, sizeof(short) * fullSigLen, &bytesRead, 0);
		totRead += bytesRead;
	}
	//резерв (пока нечего читать)
	dbN++;//6//
	//ReadFile(hFile, (void*)unote, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &bytesRead, 0); totRead += bytesRead;
	dbN++;//7//
	//ReadFile(hFile, (void*)unote, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &bytesRead, 0); totRead += bytesRead;

	dbN++;//8//читаем текстовую информацию, введённую пользователем
	if ((lettersNum > 0) && (beginBytes[dbN - dataInHead] > 0))
	{
		//должно быть: dbN = 8//dbN = dataInHead + 3;//8
		if (beginBytes[dbN - dataInHead] != totRead)
		{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

		unote = new char[lettersNum];//в lettersNum записано количество байт в пользовательской строке

		ReadFile(hFile, (void*)unote, (ehead[dbN].szDataType * lettersNum), &bytesRead, 0);
		totRead += bytesRead;

		allUsersTxt = "";
		for (i = 0; i < (__int32)bytesRead; i++)//bytesRead = lettersNum
			allUsersTxt += unote[i];

		//определяем положение разделителя ("\n\n\r\r\n\n")
		//до этого символа идёт коментарий, а после него - заметки по ходу эксперимента
		j = allUsersTxt.AnsiPos("\n\n\r\r\n\n");
		usersComent = allUsersTxt;
		usersComent.Delete(j, usersComent.Length());//коментарий к эксперименту
		ExpNotes->usersNotes->Lines->Text = usersComent;//загружаем текст в поле TMemo

		if (!usersComent.IsEmpty())//естрока не пуста
			usersInfoClick(this);//откроем блокнот

		delete[] unote; unote = NULL;
	}

	dbN++;//9//номера сигналов, к которым привязаны  метки //должно быть: dbN = 9//dbN = dataInHead + 4;//9
	if ((devExpParams_int[6] > 0) && (beginBytes[dbN - dataInHead] > 0))
	{
		if (beginBytes[dbN - dataInHead] != totRead)
		{	Experiment->DevEvents->Text = "ошибка чтения"; return; }

		sigsWithMark = new __int32[devExpParams_int[6]];//массив с номерами сигналов, к которым привязаны заметки
		ReadFile(hFile, (void*)sigsWithMark, (ehead[dbN].szDataType * devExpParams_int[6]), &bytesRead, 0);
		totRead += bytesRead;

		dbN++;//10//видимость меток на каналах
		if (beginBytes[dbN - dataInHead] > 0)
		{
            if (beginBytes[dbN - dataInHead] != totRead)
			{	Experiment->DevEvents->Text = "ошибка чтения"; return; }
			
			markerVsblt = new bool[devExpParams_int[6] * ftChan];//видимость меток на каналах
			ReadFile(hFile, (void*)markerVsblt, (ehead[dbN].szDataType * devExpParams_int[6] * ftChan), &bytesRead, 0); totRead += bytesRead;
			totRead += bytesRead;
		}

		//выставляем метки, если таковые имеются
		usersMarks = allUsersTxt;
		usersMarks.Delete(1, j + 5);//вырезаем всё, кроме меток (j - положение разделительное строки)
		j = 1;//затравочное значение
		z = 0;//счётчик меток
		for (i = 0; ((i < devExpParams_int[6]) && (j > 0)); i++)//цикл по количеству пользовательских меток
		{
			j = usersMarks.AnsiPos("||\n||");//ищем разделители между заметками
			if (j > 0)
			{
				singlMark = usersMarks;//копируем оставшуюся часть записи
				singlMark.Delete(j, singlMark.Length() - (j - 2));//вырезаем обработанную метку

				if (markerVsblt)
					for (k = 0; k < ftChan; k++)//привязываем метку к нужному каналу
						mOnChn[k] = markerVsblt[(ftChan * z) + k];//привязываем метку к нужному каналу
				else
					for (k = 0; k < ftChan; k++)//привязываем метку ко всем каналам
						mOnChn[k] = true;//привязываем метку ко всем каналам

				ExpNotes->AddMarker(singlMark, sigsWithMark[i], &mOnChn[0]);//добавляем метку
				usersMarks.Delete(1, j + 4);//удаляем из оставшегося текста обработанную метку
				z++;//увеличиваем счётчик меток
			}
		}

		delete[] sigsWithMark; sigsWithMark = NULL;
		delete[] markerVsblt; markerVsblt = NULL;
	}
	else
		dbN += 1;//необходимое приращение индекса

	dbN++;//11//
	//ReadFile(hFile, (void*)unote, (ehead[dbN].szDataType * ehead[dbN].numOfElements), &bytesRead, 0); totRead += bytesRead;

	//закрываем файл и удаляем все объекты созданные оператором new
	CloseHandle(hFile);

	delete[] beginBytes; beginBytes = NULL;
	delete[] pidate; pidate = NULL;

	gRF[0] = 0;//первый отображаемый сигнал
	gRF[1] = ftChan;//последний отображаемый сигнал (отображаем все каналы ftChan)

	Calculate(gSignals, numOfSignal, gPorog, gPreTime, gPostTime, gInd, experimentType, false, NULL);//рассчитаем параметры сигналов для отображения
	GraphsTabsShow();//открываем окно с графиками
	Graphs->Caption = "Графики - " + OpenDlg->FileName;//дополняем название окна названием файла

	if (ftChan > 1)
	{
		SAllChan->Checked = false;//для выводимости
		SAllChanClick(this);//выводим все каналы
	}
	else
		ChannNumb[0]->Click();//выводим первый канал

	//FillParamTable();//заполняем таблицу значениями параметров
	//ResetVisibility();//запускаем прорисовку графиков
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ReadLineData(TObject *Sender)
{
	//чтение файла с данными (файл типа *.drr)
	FILE *discontinFile;
	unsigned long totRead;//всего прочитано байт
	__int32 i, j, z,
			k,//счётчик сигналов
			*locRecLen,//длина развёртки сигнала в отсчётах
			locChanls,//количество сканируемых каналов
			*locAdcGain,//коэффициент усиления
			maxG,//максимальный коэффициент усиления
			fullSigLen,//полная длина данных со всех каналов (отсчёты)
			nUserMrk,//количество пользовательских меток
			nBlocks,//количество блоков (сеансов записи)
			firstPInBlk,//номер первой точки в текущем блоке
			comMrkNum,//счёткчик заметок по ходу эксперимента
			itemRd,//количество прочитанных единиц (не байт)
			*sigsWithMark,//массив с номерами сигналов, к которым привязаны заметки
			*brdSig,//номера сигналов граничных между блоками
			lettersNum,//количество символов в тексте пользователя
			locMrkNum,//локальное колчиество заметок
			numOfSignal;//число сигналов
	AnsiString allUsersTxt,//информация, вводимая пользователем
			   usersComent,//общий коментарий
			   usersMarks,//заметки по ходу эксперимента
			   singlMark;//нарезаем заметки
	float locDiscrT,//время дискретизации
		  timeSepar;//разделитель по времени
	TDateTime CurrentDateTime;//это текущие дата и время
	char lett[3],//идентификатор параметра или сигнала
		 *pidate,//информация о программе и даты (Programm Information and Date)
		 *unote;//информация, вводимая пользователем
	short expType;//тип эксперимента
	bool boolBuf,//первое определение атрибутов сигналов
		 locMultiCh,//режим сбора данных (true = многоканальный)
		 mOnChn[maxChannels];//привязка меток к каналам
		 
	maxG = 0;//максимальный коэффициент усиления
	boolBuf = true;//первое определение атрибутов сигналов
	totRead = 0;//всего прочитано байт

	//готовим массивы для записи данных из файла
	pidate = new char[ehead[0].numOfElements];
	devExpParams_int = new __int32[ehead[2].numOfElements];//создаём массив с параметрами устройства и эксперимента (__int32)
	devExpParams_float = new float[ehead[3].numOfElements];//создаём массив с параметрами устройства и эксперимента (float)

	discontinFile = fopen(OpenDlg->FileName.c_str(), "rb");//открываем для чтения файл с экспериментальными данными
	if (discontinFile == NULL)
	{   Experiment->DevEvents->Text = "ошибка открытия файла"; return; }

	numOfSignal = 0;//счётчик числа сигналов
	nUserMrk = 0;//общее количество сигналов
	nBlocks = 1;//счётчик блоков (сеансов записи)

	//сначала подсчитаем количество сигнало и прочее
	//читаем запись о датах создания и изменения файла
	itemRd = fread((void*)pidate, ehead[0].szDataType, ehead[0].numOfElements, discontinFile);//
	ExpNotes->PIDates->Lines->Text = pidate;

    //идём на начало файла и начинаем искать идентификаторы
	itemRd = fseek(discontinFile, 0, SEEK_SET);//переход на начало файла
	itemRd = fread(&lett, 1, 3, discontinFile);//
	while (itemRd > 0)
	{
		lett[0] = lett[1]; lett[1] = lett[2];
		itemRd = fread(&lett[2], 1, 1, discontinFile);//идентификатор

		if ((lett[0] == 'P') && (lett[1] == 'r') && (lett[2] == 'M') && boolBuf)//метка блока с параметрами (первое определение атрибутов сигналов)
		{
			/* последовательность записи обязательных параметров
			multiCh -------(1) - режим сбора данных (true = многоканальный)
			recLen --------(2) - длина развёртки сигнала (отсчёты)
			discrT --------(3) - время дискретизации
			chanls --------(4) - количество сканируемых каналов
			E->adcGain ----(5) - коэффициент усиления
			experimentType (6) - тип эксперимента, глобальный вариант
			E->maxVoltage -(7) - диапазон напряжений
			E->maxADCAmp --(8) - максимальная амплитуда (отсчёты) */
			itemRd = fread(&multiCh, sizeof(bool), 1, discontinFile);//1//режим сбора данных (true = многоканальный)
			itemRd = fread(&recLen, sizeof(__int32), 1, discontinFile);//2//длина сигнала (отсчёты)
			itemRd = fread(&discrT, sizeof(float), 1, discontinFile);//3//время дискретизации
			itemRd = fread(&chanls, sizeof(__int32), 1, discontinFile);//4//количество используемых каналов
			itemRd = fread(&devExpParams_int[5], sizeof(__int32), 1, discontinFile);//5//коэффициент усиления
			itemRd = fread(&expType, sizeof(short), 1, discontinFile);//6//тип эксперимента
			devExpParams_int[7] = expType;//тип эксперимента
			itemRd = fread(&devExpParams_float[1], sizeof(float), 1, discontinFile);//7//диапазон напряжений
			itemRd = fread(&devExpParams_int[11], sizeof(__int32), 1, discontinFile);//8//максимальная амплитуда (отсчёты)

			boolBuf = false;//первое определение атрибутов сигналов
			sampl2mV = devExpParams_float[1] / float(devExpParams_int[11] * devExpParams_int[5]);//коэффициента пересчёта амплитуд
			devExpParams_float[0] = sampl2mV;//0//коэффициент перевода амплитуд из отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
			ftChan = 1 + (chanls - 1) * (__int32)multiCh;//количество каналов с полными развёртками
			effDT = discrT * (float)chanls;//эффективное время дискретизации = discrT * chanls
			fullSigLen = (recLen * ftChan) + (ftChan - chanls);//полная длина данных со всех каналов (отсчёты)
		}
		else if ((lett[0] == 'M') && (lett[1] == 'r') && (lett[2] == 'k'))//область с заметками по ходу эксперимента
		{
			//читаем заметки по ходу эксперимента, если такие были
			/* последовательность записи в блоке с текстовыми данных
				lettersNum -(1) - длина текстовой записи
				comUserMakrs(2) - количество заметок по ходу эксперимента
				userText ---(3) - текст об эксперименте и заметке по ходу
				sigsWithMark(4) - массив с номерами точек с заметками
			*/
			itemRd = fread(&z, sizeof(__int32), 1, discontinFile);//1//длина текстовой записи
			itemRd = fread(&z, sizeof(__int32), 1, discontinFile);//2//количество заметок
			nUserMrk += z;//счётчик пользовательских заметок по ходу эксперимента
        }
		else if ((lett[0] == 'S') && (lett[1] == 'i') && (lett[2] == 'g'))//сам сигнал
			numOfSignal++;//счётчик числа сигналов
		else if ((lett[0] == 'E') && (lett[1] == 'D') && (lett[2] == ':'))//разделитель между сеансами сбора данных
			nBlocks++;//прирост количества блоков
	}

	//определяем другие глобальные переменные (необязательные)
	gPreTime = 0; gPostTime = 0; gPorog = 0;
	gTimeBgn = 0;
	//режим №4 лучше использовать только в стандартном (pra) режиме

	/* последовательность данных в массивах devExpParams_int и devExpParams_float
	//__int32 - параметры
	devExpParams_int[0]	= multiCh;//0//режим сбора данных (true = многоканальный)
	devExpParams_int[1]	= recLen;//1//длина развёртки сигнала в отсчётах
	devExpParams_int[2] = (__int32)discrT;//2//время дискретизации (микросекунды)
	devExpParams_int[3]	= chanls;//3//количество сканируемых каналов
	devExpParams_int[4]	= Experiment->uiLeadChan->ItemIndex;//4//ведущий канал (основной сигнал)
	devExpParams_int[5]	= Experiment->adcGain;//5//коэффициент усиления
	devExpParams_int[6] = ExpNotes->addUMark->Tag * (__int32(expType != 5));//6//количество заметок
	devExpParams_int[7] = experimentType;//7//тип эксперимента, глобальный вариант
	devExpParams_int[8] = gPreTime;//8//gPreTime (в микросекундах)
	devExpParams_int[9] = gPostTime;//9//gPostTime (в микросекундах)
	devExpParams_int[10] = (__int32)gPorog;//10//амплитудный порог (отсчёты)
	devExpParams_int[11] = Experiment->maxADCAmp;//11//максимальная амплитуда (в отсчётах)
	devExpParams_int[12] = Experiment->minADCAmp;//12//минимальная амплитуда (в отсчётах)
	devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//ведущий аппаратный канал
	devExpParams_int[14] = 0;//14//не используется

	//float - параметры
	devExpParams_float[0] = sampl2mV;//0//коэффициент перевода амплитуд из отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
	devExpParams_float[1] = Experiment->maxVoltage;//1//диапазон входных напряжений (в милливольтах)
	devExpParams_float[2] = StrToFloat(Experiment->stimPeriod->Text);//2//период стимуляции
	devExpParams_float[3] = discrT;//3//время дискретизации (микросекунды)
	devExpParams_float[4] = 0;//4//не используется
	devExpParams_float[5] =	0;//5//не используется*/

	//вписываем параметры в массивы devExpParams_int и devExpParams_float
	//__int32 - параметры
	devExpParams_int[4]	= 0;//4//ведущий канал (основной сигнал)
	devExpParams_int[6] = nUserMrk + nBlocks;//6//количество пользовательских меток (+ количество блоков (сеансов) записи)
	devExpParams_int[8] = gPreTime;//8//gPreTime (в микросекундах)
	devExpParams_int[9] = gPostTime;//9//gPostTime (в микросекундах)
	devExpParams_int[10] = (__int32)gPorog;//10//амплитудный порог (отсчёты)
	devExpParams_int[12] = -devExpParams_int[11];//12//минимальная амплитуда (в отсчётах)
	devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//ведущий аппаратный канал
	devExpParams_int[14] = 0;//14//не используется
	//float - параметры
	devExpParams_float[4] = 0;//4//не используется
	devExpParams_float[5] =	0;//5//не используется

	SigNumUpDwn->Tag = numOfSignal;//храним исходное число сигналов (нужно при удалении ссылок)
	SigNumUpDwn->Max = numOfSignal;//задаём пределы изменения номеров сигналов (оставшихся после редактирования)

	gSignals = CreatStructSignal(numOfSignal, recLen);//создаём структуру для хранения данных

	//идём на начало файла и начинаем искать идентификаторы
	itemRd = fseek(discontinFile, 0, SEEK_SET);//переход на начало файла

	locRecLen = new __int32[nBlocks];//длина развёртки сигнала в отсчётах
	locAdcGain = new __int32[nBlocks];//коэффициент усиления
	brdSig = new __int32[nBlocks + 1];//номера сигналов граничных между блоками
	k = 0;//сквозной счётчик прочитанных сигналов
	firstPInBlk = -1;//флаг
	comMrkNum = 0;//счёткчик заметок по ходу эксперимента
	nBlocks = 1;//снова открываем счётчик блоков (сеансов записи)
	//newBlock = false;//наиден новый блок
	timeSepar = 0;//корректировка времени возникновения
	gInd = new __int32[numOfSignal];//здесь же заодно сформируем массив индексов
	itemRd = fread(&lett, 1, 3, discontinFile);//
	while (itemRd > 0)
	{
		lett[0] = lett[1]; lett[1] = lett[2];
		itemRd = fread(&lett[2], 1, 1, discontinFile);//идентификатор

		if ((lett[0] == 'P') && (lett[1] == 'r') && (lett[2] == 'M'))//метка блока с параметрами
		{
			/* последовательность записи обязательных параметров
			multiCh -------(1) - режим сбора данных (true = многоканальный)
			recLen --------(2) - длина развёртки сигнала (отсчёты)
			discrT --------(3) - время дискретизации
			chanls --------(4) - количество сканируемых каналов
			E->adcGain ----(5) - коэффициент усиления
			experimentType (6) - тип эксперимента, глобальный вариант
			E->maxVoltage -(7) - диапазон напряжений
			E->maxADCAmp --(8) - максимальная амплитуда (отсчёты) */
			itemRd = fread(&locMultiCh, sizeof(bool), 1, discontinFile);//режим сбора данных (true = многоканальный)
			itemRd = fread(&locRecLen[nBlocks - 1], sizeof(__int32), 1, discontinFile);//длина развёртки сигнала в отсчётах
			itemRd = fread(&locDiscrT, sizeof(float), 1, discontinFile);//время дискретизации
			itemRd = fread(&locChanls, sizeof(__int32), 1, discontinFile);//количество сканируемых каналов
			itemRd = fread(&locAdcGain[nBlocks - 1], sizeof(__int32), 1, discontinFile);//коэффициент усиления
			itemRd = fread(&experimentType, sizeof(short), 1, discontinFile);//тип эксперимента

			if ((multiCh != locMultiCh) || (recLen != locRecLen[nBlocks - 1]) ||	(discrT != locDiscrT) ||
				(chanls != locChanls) || (devExpParams_int[5] != locAdcGain[nBlocks - 1]))//обнаружены новые параметры
			{
				/*?сохраняем предыдущий блок данных как отдельный файл и закрываем его?*/

				multiCh = locMultiCh;//режим сбора данных (true = многоканальный)
				recLen = locRecLen[nBlocks - 1];//длина развёртки сигнала в отсчётах
				discrT = locDiscrT;//время дискретизации
				chanls = locChanls;//количество сканируемых каналов
				devExpParams_int[0]	= (__int32)multiCh;//0//многоканальность (хотябы для одного файла)
				devExpParams_int[1]	= recLen;//1//длина развёртки сигнала в отсчётах
				devExpParams_int[2]	= (__int32)discrT;//2//время дискретизации (микросекунды)
				devExpParams_int[3]	= chanls;//3//количество сканируемых каналов
				devExpParams_int[5] = locAdcGain[nBlocks - 1];//коэффициент усиления
				devExpParams_int[6] = nUserMrk + nBlocks;//6//количество пользовательских меток (+ количество блоков (сеансов) записи)
				devExpParams_int[7] = experimentType;//7//тип эксперимента, глобальный вариант
				
				sampl2mV = devExpParams_float[1] / float(devExpParams_int[11] * devExpParams_int[5]);//коэффициента пересчёта амплитуд
				devExpParams_float[0] = sampl2mV;//0//коэффициент перевода амплитуд из отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
				devExpParams_float[3] = discrT;//3//время дискретизации (микросекунды)
				ftChan = 1 + (locChanls - 1) * (__int32)locMultiCh;//количество каналов с полными развёртками
                effDT = discrT * (float)chanls;//эффективное время дискретизации = discrT * chanls
				fullSigLen = (locRecLen[nBlocks - 1] * ftChan) + (ftChan - locChanls);//полная длина данных со всех каналов (отсчёты)
			}

			firstPInBlk = k;//номер первой точки в текущем блоке
			brdSig[nBlocks - 1] = k;//номер сигнала граничного между блоками
			//newBlock = true;//наиден новый блок

			if (locAdcGain[nBlocks - 1] > maxG)
				maxG = locAdcGain[nBlocks - 1];//максимальный коэффициент усиления
		}
		else if (((lett[0] == 'S') && (lett[1] == 'i') && (lett[2] == 'g')) && (k < numOfSignal))//сам сигнал
		{
			itemRd = fread(&gSignals[k].appear, sizeof(float), 1, discontinFile);//время возникновения сигнала
			totRead += itemRd * sizeof(float);
			gSignals[k].appear += timeSepar;//разделение блоков односекундным интервалом

			itemRd = fread(gSignals[k].s, sizeof(short), fullSigLen, discontinFile);
			totRead += itemRd * fullSigLen * sizeof(short);

			/*if (newBlock)
			{   singlMark = "блок№";//метка начала блока
				singlMark += IntToStr(nBlocks);
				ExpNotes->AddMarker(singlMark, firstPInBlk);
				newBlock = false;//наиден новый блок }*/

			gInd[k] = k;//здесь же заодно сформируем массив индексов
			k++;//увеличиваем счётчик прочитанных сигналов
		}
		else if ((lett[0] == 'M') && (lett[1] == 'r') && (lett[2] == 'k'))//область с заметками по ходу эксперимента
		{
			//добавляем в переменную userText заметки по ходу эксперимента из читаемого сеанса
			//читаем заметки по ходу эксперимента, если такие были
			/* последовательность записи в блоке с текстовыми данных
				lettersNum --(1) - длина текстовой записи
				comUserMakrs-(2) - количество заметок по ходу эксперимента
				userText ----(3) - текст об эксперименте и заметке по ходу
				sigsWithMark-(4) - массив с номерами точек с заметками
			*/

			itemRd = fread(&z, sizeof(__int32), 1, discontinFile);//1//длина текстовой записи
			unote = new char[z];//место под текст (весь)
			itemRd = fread(&locMrkNum, sizeof(__int32), 1, discontinFile);//2//locMrkNum - локальное колчиество заметок

			//чтение текстовых данных
			itemRd = fread((void*)unote, sizeof(char), z, discontinFile);//3//весь текст

			allUsersTxt = "";
			for (i = 0; i < z; i++)//bytesRead = lettersNum
				allUsersTxt += unote[i];

			//определяем положение разделителя ("\n\n\r\r\n\n")
			//до этого символа идёт коментарий, а после него - заметки по ходу эксперимента
			j = allUsersTxt.AnsiPos("\n\n\r\r\n\n");
			usersComent = allUsersTxt;
			usersComent.Delete(j, usersComent.Length());//коментарий к эксперименту
			ExpNotes->usersNotes->Lines->Text = usersComent;//загружаем текст в поле TMemo

			if (!usersComent.IsEmpty())//строка не пуста
				usersInfoClick(this);//откроем блокнот //ExpNotes->Show();

			usersMarks = allUsersTxt;
			usersMarks.Delete(1, j + 5);//заметки по ходу эксперимента (расставляем в конце, после прорисовки графиков)

			//читаем номера сигналов, к которым привязаны заметки (если таковые имеются)
			if ((locMrkNum > 0) && (!usersMarks.IsEmpty()))//оставшаяся строка содержит данные
			{
				sigsWithMark = new __int32[locMrkNum];//массив с номерами сигналов, к которым привязаны заметки
				itemRd = fread((void*)sigsWithMark, sizeof(__int32), locMrkNum, discontinFile);//4//номера точек с заметками

				j = 1;//счётчик символов
				for (i = 0; ((i < locMrkNum) && (comMrkNum < devExpParams_int[6])); i++)
				{
					j = usersMarks.AnsiPos("||\n||");//ищем разделители между заметками
					if (j > 0)
					{
						singlMark = usersMarks;//копируем весь оставшийся текст
						singlMark.Delete(j, singlMark.Length() - (j - 2));//вырезаем текст метки
						if ((singlMark.AnsiPos("нБ") == 1) && (singlMark.Length() <= 2))//метка является разделителем блоков
						{
							singlMark = "Б№";//меняем "безликий" разделитель блоков на нумерной
							singlMark += IntToStr(nBlocks);//добавляем номре сеанса сбора данных
						}

						//по отдельности читаем видимость для каждой метки
						itemRd = fread((void*)mOnChn, sizeof(bool), ftChan, discontinFile);//4//номера точек с заметками

						ExpNotes->AddMarker(singlMark, firstPInBlk + sigsWithMark[i], &mOnChn[0]);//добавляем метку
						usersMarks.Delete(1, j + 4);//вырезаем добавленную метку и разделитель ||\n||
					}
					comMrkNum++;//общий счётчик заметок по ходу эксперимента
				}
				delete[] sigsWithMark; sigsWithMark = NULL;
			}
			delete[] unote; unote = NULL;
        }
		else if ((lett[0] == 'E') && (lett[1] == 'D') && (lett[2] == ':'))//разделитель между блоками
		{
			timeSepar = (gSignals[k - 1].appear + 1);//между блоками вставляем интервал времени 1 секунда
			nBlocks++;//счётчик блоков
		}
	}
	SigNumUpDwn->Max = k;//количество безошибочно прочитанных сигналов
	fclose(discontinFile);//закрываем файл
	delete[] pidate; pidate = NULL;

	brdSig[nBlocks] = k;//для совместимости
	for (i = 0; i < nBlocks; i++) //перебираем блоки
	{
		for (k = brdSig[i]; k < brdSig[i + 1]; k++)
		{
			for (j = 0; j < recLen; j++)
				gSignals[k].s[j] = (short)floor((gSignals[k].s[j] * (maxG / locAdcGain[i])) + 0.5);
		}   
	}
	devExpParams_int[5] = maxG;//коэффициент усиления
	sampl2mV = devExpParams_float[1] / float(devExpParams_int[11] * devExpParams_int[5]);//коэффициента пересчёта амплитуд
				
	gRF[0] = 0;//первый отображаемый сигнал
	gRF[1] = ftChan;//последний отображаемый сигнал (отображаем все каналы ftChan)

	Calculate(gSignals, numOfSignal, gPorog, gPreTime, gPostTime, gInd, experimentType, false, NULL);//рассчитаем параметры сигналов для отображения
	GraphsTabsShow();//открываем окно с графиками
	Graphs->Caption = "Графики - " + OpenDlg->FileName;//дополняем название окна названием файла

	if (ftChan > 1)
		SAllChanClick(this);//выводим все каналы
	else
		ChannNumb[0]->Click();//выводим первый канал

	//FillParamTable();//заполняем таблицу значениями параметров
	//ResetVisibility();//запускаем прорисовку графиков
}
//---------------------------------------------------------------------------

void TGraphs::GraphsTabsShow()
{
	//высвечиваем окно с графиками (таблицами)
	__int32 i;

	if (gSignals)
	{
		if (Experiment->TabsCheck->Checked)
		{
			pValues->Visible = true;//видна будет таблица
			//выделяем соответствующую ячейку таблицы
			pValues->Row = SigNumUpDwn->Position;//номер сигнала для пользователя (видимый номер)
			pValues->Col = 1;
		}
		else
			pValues->Visible = false;//таблицу скрываем

		//формируем список каналов доступных для просмотра
		for (i = 0; i < maxChannels; i++)
		{
			if (i < ftChan)
				ChannNumb[i]->Visible = true;//делаем доступными первые каналы (chanNumb штук)
			else
				ChannNumb[i]->Visible = false;//остальные инактивируем
		}
		SAllChan->Enabled = bool(ftChan > 1);//инактивируем, если не используется

		//устанавливаем множители для перевода времени в миллисекунды
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
	//запускаем прорисовку
	__int32 i;

	Label2->Caption = "из " + IntToStr(SigNumUpDwn->Max);
	Label2->Visible = true;
	ExpNotes->PIDates->Lines->Text = ExpNotes->pIDateString;//добавляем информацюи о файле

	ClearGraphics();//затираем графики
	//AddParamsPoints(trac *apSignals, __int32 apSigNum, __int32 *apIndex, short apExpType)
	AddParamsPoints(gSignals, SigNumUpDwn->Max, gInd, experimentType);//рисуем все графики

	//делаем видимыми кнопки редактирования файла
	signalManager->Visible = true;
	ShowMean->Visible = true;
	expInfo->Visible = true;
	ReCalculat->Visible = true;
	SetExpAs->Visible = true;
	ShowChanN->Visible = true;

	//активируем кнопки пересохранения и экспортирования
	gphSave->Enabled = true;
	parametrExport->Enabled = true;
	signalsExport->Enabled = true;
	prntWin->Enabled = true;//"печать" окна

	CrntSig->ReadOnly = false;
	CrntSig->Text = "1";//выводим первый сигнал
	SigNumUpDwn->Enabled = true;//разрешаем "листать" сигналы
	SigNumUpDwn->Position = 1;//выводим первый сигнал
	ShowSignal(SigNumUpDwn->Position);//выводим первый сигнал
}
//---------------------------------------------------------------------------

void TGraphs::FillParamTable()
{
	//заполняем таблицу значениями параметров
	__int32 i, j, k,
			nrc,//количество заполняемых колонок
			sch,//количестов второстепенных каналов (в одноканальном режиме)
			numOfSig,//число записанных сигналов
			shft1,//номер первого элемента для ampls и peakInds
			shft2;//номер первого элемента для spans
	float apt;
	bool printMark;//есть ли метка для отображения

	nrc = 5 + (5 * __int32(experimentType == 3));//количество заполняемых колонок
	sch = (chanls - 1) * __int32(!multiCh);//количестов второстепенных каналов (в одноканальном режиме)
	pValues->ColCount = nrc + sch + 1;//количество колонок в таблице по количеству параметров

	if (experimentType == 3)
	{
		pValues->Cells[5][0] = "Na-ток, мВ";//первая амплитуда пресинаптического ответа (натриевый ток) (после синей, перед красной линии)
		pValues->Cells[6][0] = "tNa, мкс";//длительность первого пика (натриевого тока, в микросекундах)
		pValues->Cells[7][0] = "K-ток, мВ";//вторая амплитуда пресинаптического ответа (калиевый ток) (после синей, перед красной линии)
		pValues->Cells[8][0] = "син зд, мкс";//синаптическая задержка или интервал... (в микросекундах)
		pValues->Cells[9][0] = "st0, мкс";//время синаптического проведения (в микросекундах)
	}
	for (i = 1; i < sch; i++)
		pValues->Cells[nrc + i - 1][0] = "канал" + IntToStr(i + 1) + ", мВ";//канал i
	pValues->Cells[nrc + i - 1][0] = "заметки по ходу";//заметки по ходу эксперимента

	numOfSig = SigNumUpDwn->Max;//количество сигналов в файле
	pValues->RowCount = numOfSig + 1;//количество строк в таблице

	printMark = false;//до обнаружения первой метки curR = false
	for (j = 0; j < numOfSig; j++)//вписываем значения параметров в таблицу
	{
		for (i = 0; i < ExpNotes->addUMark->Tag; i++)//проверим дошли ли до точки, к которой привязана метка
		{
			if (ExpNotes->theMarker->pointOnGraph == j)//данная метка привязана к этому сигналу
			{
				printMark = true;//появилась метка: отображаем её
				break;
			}
			ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
		}

		if (printMark)//есть метка для отображения
			pValues->Cells[9 + maxChannels][j + 1] = ExpNotes->theMarker->textMark;//выводим заметки в крайнюю правую колонку

		shft1 = (3 * ftChan * gInd[j]) + (3 * gRF[0]);//номер первого элемента для ampls и peakInds
		shft2 = (5 * ftChan * gInd[j]) + (5 * gRF[0]);//номер первого элемента для spans

		pValues->Cells[0][j	+ 1] = IntToStr(j + 1);//номер сигнала
		pValues->Cells[1][j + 1] = FloatToStrF(gSignals[gInd[j]].appear, ffFixed, 6, 3);//время возникновения сигнала (синхроимпульса); в секундах от начала эксперимента
		pValues->Cells[2][j + 1] = FloatToStrF((float)cPrm[0].ampls[shft1] * sampl2mV, ffFixed, 6, 3);//амплитуда основная - постсинаптическая (после красной линии)
		pValues->Cells[3][j + 1] = IntToStr(cPrm[0].spans[shft2]);//время нарастания (микросекунды)
		pValues->Cells[4][j + 1] = IntToStr(cPrm[0].spans[shft2 + 1]);//время спада (микросекунды)
		if (experimentType == 3)
		{
			pValues->Cells[5][j + 1] = FloatToStrF((float)cPrm[0].ampls[shft1 + 1] * sampl2mV, ffFixed, 6, 3);//первая амплитуда пресинаптического ответа (натриевый ток) (после синей, перед красной линии)
			pValues->Cells[6][j + 1] = IntToStr(cPrm[0].spans[shft2 + 2]);//длительность первого пика (натриевого тока, в микросекундах)
			pValues->Cells[7][j + 1] = FloatToStrF((float)cPrm[0].ampls[shft1 + 2] * sampl2mV, ffFixed, 6, 3);//вторая амплитуда пресинаптического ответа (калиевый ток) (после синей, перед красной линии)
			pValues->Cells[8][j + 1] = IntToStr(cPrm[0].spans[shft2 + 3]);//синаптическая задержка или интервал... (в микросекундах)
			pValues->Cells[9][j + 1] = IntToStr(cPrm[0].spans[shft2 + 4]);//время синаптического проведения (в микросекундах)
		}

		for (i = 1; i < sch; i++)//выводим одиночные отсчёты на второстепенных каналах
		{
			k = recLen + i - 1;//номер нужного отсчёта
			pValues->Cells[nrc + i - 1][j + 1] = FloatToStrF((float)gSignals[gInd[j]].s[k] * sampl2mV, ffFixed, 6, 3);
		}
	}
	for (i = 2; i < pValues->ColCount; i++)
		pValues->ColWidths[i] = (pValues->Cells[i][0].Length()) * 7;//устанавливаем ширину колонок

	Resized(this);//задаём размеры таблицы и окна
}
//---------------------------------------------------------------------------

trac* TGraphs::CreatStructSignal(__int32 crNumOfS, __int32 crRecLen)
{
	//создание массива с развёртками сигналов и их атрибутами (только в этом модуле)
	/*
	crNumOfS - запрашиваемое количество сигналов
	crRecLen - запрашиваемая длина сигнала
	*/
	__int32 i, j,
			fulChan,//количество каналов с полными развёртками
			fullSigLen;//суммарная длина сигналов со всех каналов
	trac *crStructSignal = new trac[crNumOfS];

	fullSigLen = (crRecLen * ftChan) + (chanls - ftChan);//суммарная длина развёрток на всех каналах (отсчёты)

	for (i = 0; i < crNumOfS; i++)
	{
		crStructSignal[i].s = new short[fullSigLen];//развёртки сигналов на всех каналах
		for (j = 0; j < fullSigLen; j++)
			crStructSignal[i].s[j] = 0;//зануляем значения массивов для сигналов
		crStructSignal[i].appear = 0;//время возникновения сигнала (синхроимпульса); в секундах от начала эксперимента
	}

    cPrm = new sPr;//массивы с вычисляемыми параметрами сигналов
	cPrm[0].ampls = new short[3 * crNumOfS * ftChan];//все пиковые значения (амплитуды)
		//ampls[0] = amplPost - амплитуда основная - постсинаптическая (после красной линии)
		//ampls[1] = ampl1NaPeak - первая амплитуда пресинаптического ответа (натриевый ток) (после синей, перед красной линии)
		//ampls[2] = ampl2KPeak - вторая амплитуда пресинаптического ответа (калиевый ток) (после синей, перед красной линии)
	cPrm[0].peakInds = new __int32[3 * crNumOfS * ftChan];//все номера отсчётов с пиковыми значениями
		//peakInds[0] = indMaxPost - номер отсчёта с максимальным значением amplPost
		//peakInds[1] = indMax1Na - номер отсчёта с максимальным значением amplPre1
		//peakInds[2] = indMax2K - номер отсчёта с максимальным значением amplPre2
	cPrm[0].spans = new __int32[5 * crNumOfS * ftChan];//все характерные времена (периоды, фазы, интервалы)
		//spans[0] = riseT - время нарастания (микросекунды)
		//spans[1] = fallT - время спада (микросекунды)
		//spans[2] = tSynTrans - время синаптического проведения (в микросекундах)
		//spans[3] = t1NaPeak - длительность первого пика (натриевого тока, в микросекундах)
		//spans[4] = tDelay - синаптическая задержка или интервал... (в микросекундах)

	for (j = 0; j < (3 * crNumOfS * ftChan); j++)
	{
		cPrm[0].ampls[j] = 0;//обнуляем
		cPrm[0].peakInds[j] = 0;//обнуляем
	}
	for (j = 0; j < (5 * crNumOfS * ftChan); j++)
		cPrm[0].spans[j] = 0;//обнуляем

	return crStructSignal;
}
//---------------------------------------------------------------------------

void TGraphs::DeleteStructSignal(trac *delSignal, __int32 delNumOfS)
{
	//удаление структуры с сигналами (только в этом модуле)
	/*
	delSignal - указатель на структуру с сигналами
	delPrm - указатель на массив с вычислемыми параметрами сигналов
	delNumOfS - количество сигналов, под которое изначально была зарезервирована память
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
	//открываем файл и начинаем рисовать всевозможные графики
	//в этом модуле прописываю функции прорисовки графиков (к ним будут обращаться остальные модули)

	__int32 i;
	AnsiString extntFl;//расширение открываемого файла

	if (OpenDlg->Execute())
	{
		Gists->Close();//закрытие окна с гистограммами

		//удаляем всё из памяти
		Experiment->DiscontinWrt->Tag = -1;//чтобы всё очистить
		ClearMemor(this);//очистим память
		plotFrom = 0;//рисуем сначала

		//выделяем расширение открываемого файла
		extntFl = OpenDlg->FileName.SubString(OpenDlg->FileName.Length() - 2, OpenDlg->FileName.Length());

		//вызываем нужную функцию чтения файла
		if (extntFl == "bin")
		{
			ConvertBinary(true);//переводим бинарные данные в текстовый вид(false) или в стандартный pra-формат(true)
		}
		else
		{
			if (extntFl == "pra")
				ReadDataFile(this);//традиционный pra файл
			else if (extntFl == "drr")
				ReadLineData(this);//"линейный" drr файл
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::OnClosGraphs(TObject *Sender)
{
	//при закрытии окна затираем данные
	__int32 i;

	ClearMemor(this);//очистим память
	plotFrom = 0;//рисуем снача

	//поменяем облик кнопок вызова графиков
	for (i = 1; i < 9; i++)
		allCharts[i]->Visible = false;//скрываем все чарты
	_isfull = false;//нет развёрнутых графиков

	Experiment->Ampl_inT->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->Rise_inT->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->Fall_inT->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->NaAmpl_inT->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->KAmpl_inT->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->NaDur_inT->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->SynDel_inT->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->St0_inT->Font->Style = TFontStyles();//меняем облик кнопки

	Experiment->plottedGrphs = 1;//обнуляем счётчик открытых графиков
	Gists->GistsClear(this);//затираем гистограммы
	CrntSig->ReadOnly = true;//запрещаем "листать" сигналы
}
//---------------------------------------------------------------------------

void TGraphs::ClearGraphics()
{
	//затираем графики и очищаем таблицу
	//если нажато дописать то грфики не затираем
	__int32 i;

	//очистить таблицу параметров
	for (i = 1; i < pValues->RowCount; i++)
		pValues->Rows[i]->Clear();
	pValues->RowCount = 2;//уменьшаем до минимума количество строк в таблице

	csSpecPoint->Clear();
	for (i = 0; i < maxChannels; i++)
	{
		curntSigChannls[i]->Clear();//очищаем графики каналов
		sigAmpls[i]->Clear();//очищаем графики амплитуд сигналов
		allCharts[i]->UndoZoom();//отменяем увеличение, произведённое ранее
	}
	gRiseTsL->Clear();//график времём нарастания
	gFallTsL->Clear();//график времён спада
	gNaAmplsL->Clear();//амплитуда Na-тока
	gNaDursL->Clear();//длительности Na-тока
	gKAmplsL->Clear();//амплитуда K-тока
	gSynDelsL->Clear();//синаптическая задержка
	gSynTrnsL->Clear();//время синаптического проведения

	curAmpl->Clear();//амплитуда выделенного сигнала (график-точка)
	curRiseT->Clear();//время нарастания выделенного сигнала (график-точка)
	curFallT->Clear();//время спада выделенного сигнала (график-точка)
	curNaAmpl->Clear();//ампл Na-тока выделенного сигнала (график-точка)
	curNaDur->Clear();//выделенный сигнал
	curKAmpl->Clear();//выделенный сигнал
	curSynDel->Clear();//выделенный сигнал
	curSynTrns->Clear();//выделенный сигнал
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ClearMemor(TObject *Sender)
{
	//освобождаем память из под сигналов перед началом нового эксперимента

    if (Experiment->DiscontinWrt->Tag != 1)//если записываем заново или открываем файл (не идёт дозапись файла)
	{
		Graphs->ClearGraphics();//затираем графики и очищаем таблицу
		ExpNotes->DeleteMarkers();//удалем заметки
		ExpNotes->nmInRec = 0;//количество меток записанных в предыдущих сеансах сбора данных (дозапись в существующий файл)
	}
	
	if (gSignals)
	{
		DeleteStructSignal(gSignals, SigNumUpDwn->Tag);//удаляем структуру с сигналами
		gSignals = NULL;
	}
	if (gInd)
	{
		delete[] gInd;//удаляем массив индексов
		gInd = NULL;
	}
	if (devExpParams_int)
	{
		delete[] devExpParams_int;//удаляем массив с параметрами устройства и эксперимента (__int32)
		devExpParams_int = NULL;
	}
	if (devExpParams_float)
	{
		delete[] devExpParams_float;//удаляем массив с параметрами устройства и эксперимента (float)
		devExpParams_float = NULL;
	}

	CrntSig->ReadOnly = true;
	CrntSig->Text = "0";
	SigNumUpDwn->Enabled = false;
	Label2->Visible = false;
	Graphs->Caption = "Графики";

	//инактивируем кнопки редактирования файла
	signalManager->Visible = false;
	ShowMean->Visible = false;
	expInfo->Visible = false;
	ReCalculat->Visible = false;
	SetExpAs->Visible = false;
	ShowChanN->Visible = false;

	//инактивируем кнопки пересохранения, экспортирования и т.д.
	gphSave->Enabled = false;
	parametrExport->Enabled = false;
	signalsExport->Enabled = false;
	prntWin->Enabled = false;//"печать" окна
}
//---------------------------------------------------------------------------

void TGraphs::ShowSignal(__int32 sn)
{
	//выводим сигнал с заданным номером
	/*
	sn - номер сигнала (пользовательский [1, ..., N])
	*/
	__int32 i, z, k,
			shft1,//номер первого элемента для ampls и peakInds
			shft2,//номер первого элемента для spans
			indBgn,//индекс - начало рисования
			indSN;//номер сигнала в массиве индексов (реальный номер)
	double *graphic;//массив с графиком

	indSN = gInd[sn - 1];//определяем какой сигнал выводить
	indBgn = gTimeBgn / effDT;//индекс - начало рисования
	graphic = new double[recLen * (gRF[1] - gRF[0])];//массив с графиком
	k = 0;
	for (z = gRF[0]; z < gRF[1]; z++)
		for (i = 0; i < recLen; i++)
		{
			graphic[k] = gSignals[indSN].s[(z * recLen) + i] * sampl2mV;
			k++;
		}
	PlotCurrentSignal(graphic, indBgn);

	//подсвечиваем параметры текущего сигнала на графиках
	shft1 = (3 * ftChan * indSN) + (3 * 0);//номер первого элемента для ampls и peakInds
	shft2 = (5 * ftChan * indSN) + (5 * 0);//номер первого элемента для spans
	HighLightCrnt(&gSignals[indSN], shft1, shft2, experimentType, indBgn, sn - 1);

	ExpNotes->addUMark->Text = "";//сначала затираем текст в окне для заметок
	for (i = 0; i < ExpNotes->addUMark->Tag; i++)//если к сигналу прикреплена метка, то отобразим её в окне addUMark
	{
		if (((sn - 1) == ExpNotes->theMarker->pointOnGraph) && (ExpNotes->theMarker->chanN[gRF[0]]) &&
			!SAllChan->Checked)//есть метка на сигнале (на канале)
		{
			ExpNotes->addUMark->Text = ExpNotes->theMarker->textMark;
			break;
		}
		ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
	}
	delete[] graphic; graphic = NULL;//удаляем массив графика
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::sigNumUpDwnChangingEx(TObject *Sender,
	  bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	//отображаем сигнал с заданным номером при любом изменение (программном или пользовательском)
	//NewValue - введённый номер сигнала для пользователя (видимый номер)

	if ((NewValue >= 1) && (NewValue <= SigNumUpDwn->Max))
	{
		CrntSig->Text = IntToStr(NewValue);//разрешаем внесённое изменение номера сигнала
		CrntSig->Tag = NewValue;//копия номера сигнала для пользователя (нужна при редактировании меток)
		ShowSignal(NewValue);//выводим сигнал с заданным номером

		AllowChange = true;//разрешаем изменение
	}
	else
	{
		CrntSig->Text = IntToStr(SigNumUpDwn->Position);//оставляем прежнее значение
		AllowChange = false;//не меняем
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::NextPres(TObject *Sender)
{
	SigNumUpDwn->Position++;//следующий сигнал
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::PrevPres(TObject *Sender)
{
	SigNumUpDwn->Position--;//предыдущий сигнал
}
//---------------------------------------------------------------------------

void TGraphs::HighLightCrnt(trac *hSig, __int32 shft1, __int32 shft2, short hExpT, __int32 hIndBgn, __int32 pGr)
{
	//выделяем параметры текущего сигнала на графиках и подсвечиваем точки на графиках
	/*
	hSig - структура с подсвечиваемым сигналом
	shft1 - номер первого элемента для ampls и peakInds
	shft2 - номер первого элемента для spans
	hExpT - тип эксперимента
	hIndBgn - начальная для рисования точка сигнала
	pGr - номер точки на графике
	*/
	__int32 i;

	csSpecPoint->Clear();//сотрём амплитудные точки
	curAmpl->Clear();//амплитуда
	curRiseT->Clear();//время нарастания
	curFallT->Clear();//время спада

	//выделяем амплитудные точки
	for (i = gRF[0]; i < gRF[1]; i++)
	{
		if (cPrm[0].peakInds[shft1 + (3 * i)] >= 0)
			csSpecPoint->AddXY((cPrm[0].peakInds[shft1 + (3 * i)] - hIndBgn), (float)cPrm[0].ampls[shft1 + (3 * i)] * sampl2mV);

		if (hExpT == 3)
		{
			csSpecPoint->AddXY((cPrm[0].peakInds[shft1 + (3 * i) + 1] - hIndBgn), (float)cPrm[0].ampls[shft1 + (3 * i) + 1] * sampl2mV);//выделяем точку амплитуды Na-тока
			csSpecPoint->AddXY((cPrm[0].peakInds[shft1 + (3 * i) + 2] - hIndBgn), (float)cPrm[0].ampls[shft1 + (3 * i) + 2] * (-1) * sampl2mV);//выделяем точку амплитуды К-тока
		}

		//подсвечиваем выбранный сигнал на графиках
		curAmpl->AddXY(hSig->appear + plotFrom, sigAmpls[i]->YValues->operator [](pGr));//(float)cPrm[0].ampls[shft1 + (3 * i)] * sampl2mV);
	}

	i = gRF[0];//первый отображаемый канал,
	//подсвечиваем выбранный сигнал на графиках
	AmplInT->Title->Text->Text = "амплитуда, " + FloatToStrF((float)cPrm[0].ampls[shft1 + (3 * i)] * sampl2mV, ffFixed, 6, 2) + " мВ";

	curRiseT->AddXY(hSig->appear + plotFrom, gRiseTsL->YValues->operator [](pGr));//cPrm[0].spans[shft2]);
	RiseTInT->Title->Text->Text = "время нарастания, " + IntToStr(cPrm[0].spans[shft2 + (5 * i)]) + " мкс";

	curFallT->AddXY(hSig->appear + plotFrom, gFallTsL->YValues->operator [](pGr));//cPrm[0].spans[shft2 + 1]);
	FallTInT->Title->Text->Text = "время спада, " + IntToStr(cPrm[0].spans[shft2 + (5 * i) + 1]) + " мкс";

	if (hExpT == 3)
	{
		curNaAmpl->Clear();//амплитуда Na-тока
		curNaAmpl->AddXY(hSig->appear + plotFrom, gNaAmplsL->YValues->operator [](pGr));//(float)cPrm[0].ampls[shft1 + 1] * sampl2mV);
		NaAmplInT->Title->Text->Text = "Na-ток, " + FloatToStrF((float)cPrm[0].ampls[shft1 + (3 * i) + 1] * sampl2mV, ffFixed, 6, 2)+ " мВ";

		curNaDur->Clear();//длительность Na-тока
		curNaDur->AddXY(hSig->appear + plotFrom, gNaDursL->YValues->operator [](pGr));//cPrm[0].spans[shft2 + 2]);
		NaDurInT->Title->Text->Text = "длительность Na-тока, " + IntToStr(cPrm[0].spans[shft2 + (5 * i) + 2]) + " мкс";

		curKAmpl->Clear();//амплитуда К-тока
		curKAmpl->AddXY(hSig->appear + plotFrom, gKAmplsL->YValues->operator [](pGr));//(float)cPrm[0].ampls[shft1 + 2] * sampl2mV);
		KAmplInT->Title->Text->Text = "K-ток, " + FloatToStrF((float)cPrm[0].ampls[shft1 + (3 * i) + 2] * sampl2mV, ffFixed, 6, 2) + " мВ";

		curSynDel->Clear();//синаптическая задержка
		curSynDel->AddXY(hSig->appear + plotFrom, gSynDelsL->YValues->operator [](pGr));//cPrm[0].spans[shft2 + 3]);
		SynDelInT->Title->Text->Text = "синаптическая задержка, " + IntToStr(cPrm[0].spans[shft2 + (5 * i) + 3]) + " мкс";

		curSynTrns->Clear();//время синаптического проведения
		curSynTrns->AddXY(hSig->appear + plotFrom, gSynTrnsL->YValues->operator [](pGr));//cPrm[0].spans[shft2 + 4]);
		SynTrnsInT->Title->Text->Text = "синапт првд, " + IntToStr(cPrm[0].spans[shft2 + (5 * i) + 4]) + " мкс";
	}

	hSig = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::DeletSignalClick(TObject *Sender)
{
	//реакция на нажатие кнопки удаления сигнала
	__int32 i,
			p;//номер отображаемого сигнала

	p = SigNumUpDwn->Position - 1;//номер отображаемого сигнала

	for (i = gRF[0]; i < gRF[1]; i++)//удаляем точку с графиков
		sigAmpls[i]->Delete(p);
	gRiseTsL->Delete(p);//удаляем точку с графиков
	gFallTsL->Delete(p);
	if (experimentType == 3)//удаляем точку с графиков
	{
		gNaAmplsL->Delete(p);
		gNaDursL->Delete(p);
		gKAmplsL->Delete(p);
		gSynDelsL->Delete(p);
		gSynTrnsL->Delete(p);
	}

	RemoveSignals(SigNumUpDwn->Position - 1);//собственно исключение сигнала из списка рассматриваемых
	Label2->Caption = "из " + IntToStr(SigNumUpDwn->Max);//отображаем новое (оставшееся) количество сигналов
	if ((p + 1) > SigNumUpDwn->Max)//удаление конечного сигнала
		SigNumUpDwn->Position = SigNumUpDwn->Max;//поправка номера сигнала
	else
		ShowSignal(p + 1);//прорисовка сигнала (новый текущий сигнал)
}
//---------------------------------------------------------------------------

void TGraphs::RemoveSignals(__int32 sigToDel)
{
	//удаляем индекс сигнала из массива индексов (если это разрешено)
	/*
	sigToDel - номер удаляемого сигнала (нумерация с нуля)
	*/
	__int32 i,
			oldNum,//было сигналов
			newNum;//останется сигналов

	oldNum = SigNumUpDwn->Max;
	newNum = oldNum - 1;

	SigNumUpDwn->Max = newNum;//новое (оставшееся) количество сигналов
	for (i = 0; i < ExpNotes->addUMark->Tag; i++)//если к удаляемому сигналу прикреплена метка, то удаляем и её
	{
		if (sigToDel == ExpNotes->theMarker->pointOnGraph)
			ExpNotes->DeleteAMark(ExpNotes->theMarker);
		ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
	}

	for (i = sigToDel; i < newNum; i++)//сдвигаем номера сигналов в массиве индексов влево
		gInd[i] = gInd[i + 1];//сдвигаем номера сигналов в массиве индексов влево

	for (i = 0; i < ExpNotes->addUMark->Tag; i++)//сдвигаем номера сигналов, к которым привязаны метки
	{
		if (ExpNotes->theMarker->pointOnGraph > sigToDel)
			ExpNotes->theMarker->pointOnGraph--;//сдвигаем номера сигналов, к которым привязаны метки
		ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
	}

	for (i = (sigToDel + 1); i < pValues->RowCount - 1; i++)
	{
		pValues->Rows[i] = pValues->Rows[i + 1];//перемещаме вверх
		pValues->Cells[0][i] = IntToStr(i);//меняем номер сигнала
	}
	pValues->RowCount--;//удаляем строчку из таблицы
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::GphSaveClick(TObject *Sender)
{
	//выбор имени файла для сохранения эксперимента
	SaveDlg->Title = "Введите имя файла";
	//SaveDlg->Options = TOptin << ofOverwritePrompt;//true;
	SaveDlg->Filter = "*.pra|*.pra";
	SaveDlg->DefaultExt = "pra";

	SaveDlg->FileName = OpenDlg->FileName;//предлагаем сохранить под прежним именем
	if (SaveDlg->Execute())
	{
		CrntSig->ReadOnly = true;//на время сохранения отменяем реакцию на изменение номера сигнала
		SigNumUpDwn->Enabled = false;//также во время сохранения нельзя переключать сигналы

		if (SaveExpDataToFile(experimentType, NULL, SigNumUpDwn->Max, false))
		{
			FillParamTable();//заполняем таблицу значениями параметров
			ResetVisibility();//запускаем прорисовку
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::usersInfoClick(TObject *Sender)
{
	//показываем информацию об эксперименте, введённую пользователем

	//настроим элементы окна
	ExpNotes->mainLinesLbl->Caption = "Общие сведения об эксперименте";
	ExpNotes->usersNotes->Visible = true;//поле для пользовательских данных
	ExpNotes->addUMark->Visible = true;//поле ввода заметок по ходу эксперимента
	ExpNotes->addMarkLbl->Visible = true;//заметки-лэйбл
	ExpNotes->PIDates->Visible = false;//поле данных о файле

	ExpNotes->Show();//открываем окно блокнота
}
//---------------------------------------------------------------------------

void TGraphs::SetSeriesMultipliers()
{
	//поменяем масштабные факторы для графиков в соответствии с указаниями пользователя
	__int32 i,
			cnt;
	double fmValue;//первое или максимальное значение

	cnt = sigAmpls[gRF[0]]->Count();//gAmplsL количество прорисованных сигналов

	if (cnt >= 1)//на графиках есть точки
	{
		if (Experiment->shwAmpls->Checked)//показывать значения как есть
		{
			for (i = gRF[0]; i < gRF[1]; i++)
				sigAmpls[i]->YValues->Multiplier = 1;//gAmplsL
			for (i = 2; i < 9; i++)
				allCharts[i]->Series[0]->YValues->Multiplier = 1;//нулевая серия - основной график
		}
		else if (Experiment->pCentFirstS->Checked)//показываем проценты величины первого сигнала
		{
			for (i = gRF[0]; i < gRF[1]; i++)
			{
				fmValue = fabs(sigAmpls[i]->YValues->operator [](0));//значение первого элемента
				if (fmValue < 1)//максимальное значение очень мало
					fmValue = 1;//заменяем единицей, чтобы избежать деления на ноль
				sigAmpls[i]->YValues->Multiplier *= (double(100) / fmValue);//новый множитель
			}

			for (i = 2; i < 9; i++)
			{
				cnt = allCharts[i]->Series[0]->Count();//количество точек в данном графике
				if (cnt >= 1)//график существует
				{
					fmValue = fabs(allCharts[i]->Series[0]->YValues->operator [](0));//значение первого элемента
					if (fmValue < 1)//первое значение очень мало
						fmValue = 1;//заменяем единицей, чтобы избежать деления на ноль
					allCharts[i]->Series[0]->YValues->Multiplier *= (double(100) / fmValue);//новый множитель
				}
			}
		}
		else if (Experiment->pCentMax->Checked)//показываем проценты максимальной величины
		{
			for (i = gRF[0]; i < gRF[1]; i++)
			{
				fmValue = fabs(sigAmpls[i]->YValues->MaxValue);//максимальное значение в данном графике
				if (fmValue < 1)//максимальное значение очень мало
					fmValue = 1;//заменяем единицей, чтобы избежать деления на ноль
				sigAmpls[i]->YValues->Multiplier *= (double(100) / fmValue);//новый множитель
			}
			for (i = 2; i < 9; i++)
			{
				cnt = allCharts[i]->Series[0]->Count();//количество точек в данном графике
				if (cnt >= 1)//график существует
				{
					fmValue = fabs(allCharts[i]->Series[0]->YValues->MaxValue);//максимальное значение в данном графике
					if (fmValue < 1)//максимальное значение очень мало
						fmValue = 1;//заменяем единицей, чтобы избежать деления на ноль
					allCharts[i]->Series[0]->YValues->Multiplier *= (double(100) / fmValue);//новый множитель
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TGraphs::parametrExportClick(TObject *Sender)
{
	//экспортирование параметров сигналов в текстовый файл (dat - формат)

	FILE *stream;//поток вывода данных в файл
	AnsiString bfr;//вспомогательная переменная
	__int32 i, j,
			shft1,//номер первого элемента для ampls и peakInds
			shft2,//номер первого элемента для spans
			rlSh,//исправленная длина сигнала (с учётом ПОСТ-времени, отсчёты)
			bgnSigSh;//номер отсчёта, с которого начинается данный сигнал(с учётом многоканальности и ПОСТ-времени)
	markers *printMark;//есть ли метка для отображения

	//экспортируем параметры сигналов
	SaveDlg->Title = "Экспорт параметров";//диалоговое сообщение
	SaveDlg->Filter = "*.dat|*.dat";//фильтр отображения сигналов
	SaveDlg->DefaultExt = "dat";//расширение по умолчанию

	//по умолчанию вписываем в диалог имя открытого файла без расширения
	bfr = OpenDlg->FileName;
	bfr.Delete(bfr.Length() - 3, 4);//удаляем расширение
	SaveDlg->FileName = bfr;

	if (SaveDlg->Execute())
	{
		stream = fopen(SaveDlg->FileName.c_str(), "w");//создаём файл с параметрами сигналов

		//вписываем названия измеряемых величин
		fprintf(stream, "time\tamplPost\tArea\triseT\tfallT");
		if (experimentType == 3)//внеклеточные: добавим спец параметры
			fprintf(stream, "\tampl1Na\tampl2K\tt1Na\tsynDelay\tsynTr");
		for (j = 0; ((j < (chanls - 1)) && !multiCh); j++)//если не многоканальный режим
		{
			bfr = "\tChannel";
			bfr += IntToStr(j + 2);
			fprintf(stream, bfr.c_str());
		}
		if (ExpNotes->addUMark->Tag > 0)//если есть заметки по ходу эксперимента
			fprintf(stream, "\tMarks");
		fprintf(stream,"\n");//закончили первую строку заголовка (названия измеряемых величин)

		//вписываем размерности измеряемых величин
		fprintf(stream, "sec\tmV\tmVms\tmks\tmks");//для параметров сигнала
		if (experimentType == 3)//внеклеточные: добавим спец параметры
			fprintf(stream, "\tmV\tmV\tmks\tmks\tmks");//для параметров сигнала

		for (j = 0; ((j < (chanls - 1)) && !multiCh); j++)//если не многоканальный режим
			fprintf(stream, "\tmV");//для данных со второстепенных каналов
		if (ExpNotes->addUMark->Tag > 0)//если есть заметки по ходу эксперимента
			fprintf(stream, "\tMarks");
		fprintf(stream, "\n");//закончили вторую строку заголовка (размерности измеряемых величин)

		//записываем собственно данные (значения параметров)
		printMark = NULL;//до обнаружения первой метки printMark = false
		rlSh = recLen - (gPostTime / effDT);//исправленная длина сигнала (с учётом ПОСТ-времени, отсчёты)
		bgnSigSh = (gRF[0] * recLen) + (gPostTime / effDT);//номер отсчёта, с которого начинается данный сигнал(с учётом многоканальности и ПОСТ-времени)
		for (i = 0; i < SigNumUpDwn->Max; i++)
		{
			shft1 = (3 * ftChan * gInd[i]) + (3 * gRF[0]);//номер первого элемента для ampls и peakInds
			shft2 = (5 * ftChan * gInd[i]) + (5 * gRF[0]);//номер первого элемента для spans
			fprintf(stream, "%8.3f\t%7.2f\t%7.2f\t%d\t%d",
					gSignals[gInd[i]].appear,//время возникновения сигнала (в секундах от начала эксперимента)
					(float)cPrm[0].ampls[shft1] * sampl2mV,//амплитуды
					IntegralCalculate(&gSignals[gInd[i]], rlSh, sampl2mV * effDT, bgnSigSh),//"площадь" под сигналом
					cPrm[0].spans[shft2],
					cPrm[0].spans[shft2 + 1]);
			if (experimentType == 3)//внеклеточные: добавим спец параметры
				fprintf(stream, "\t%7.2f\t%7.2f\t%d\t%d\t%d",
							 (float)cPrm[0].ampls[shft1 + 1] * sampl2mV,
							 (float)cPrm[0].ampls[shft1 + 2] * sampl2mV,
							 cPrm[0].spans[shft2 + 2],
							 cPrm[0].spans[shft2 + 3],
							 cPrm[0].spans[shft2 + 4]);

			if (!multiCh)//если не многоканальный режим
				for (j = 0; j < (chanls - 1); j++)//вписываем данные со второстепенных каналов
					fprintf(stream, "\t%7.2f", gSignals[gInd[i]].s[recLen + j] * sampl2mV);
			if (ExpNotes->addUMark->Tag > 0)//если есть заметки по ходу эксперимента
			{
				//проверим дошли ли до точки, к которой привязана заметка
				for (j = 0; j < ExpNotes->addUMark->Tag; j++)
				{
					if ((ExpNotes->theMarker->pointOnGraph == i) && (ExpNotes->theMarker->chanN[gRF[0]]))
					{
						printMark = ExpNotes->theMarker;//выводить текущую метку
						break;
					}
					ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
				}
				if (printMark)
				{
					fprintf(stream, "\t");
					fprintf(stream, printMark->textMark.c_str());//выводим заметки в крайнюю правую колонку
				}
			}
			fprintf(stream, "\n");
		}
		fclose(stream);//закрываем файл (закрываем поток вывода данных в файл)
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::signalsExportClick(TObject *Sender)
{
	//экспортирование развёрток сигналов в текстовый файл (dat - формат)

	FILE *stream;//поток вывода данных в файл
	AnsiString bfr;//вспомогательная переменная
	__int32 i, j, z,
			indBgn;//индекс - начало рисования

	//экспорт сигналов
	SaveDlg->Title = "Экспорт сигналов";//диалоговое сообщение
	SaveDlg->Filter = "*.dat|*.dat";//фильтр отображения файлов
	SaveDlg->DefaultExt = "dat";//расширение по умолчанию

	//по умолчанию вписываем имя открытого файла в диалог
	bfr = OpenDlg->FileName;
	bfr.Delete(bfr.Length() - 3, 4);//удаляем расширение
	SaveDlg->FileName = bfr;

	if (SaveDlg->Execute())
	{
		stream = fopen(SaveDlg->FileName.c_str(), "w");
		indBgn = gTimeBgn / effDT;//индекс - начало рисования

		//вписываем названия величин
		fprintf(stream, "time\t");
		for (i = 0; i < SigNumUpDwn->Max; i++)
		{
			fprintf(stream, "s_%d\t", (i + 1));
			for (z = 1; z < ftChan; z++)
				fprintf(stream, "sub_%d\t", z);
		}
		fprintf(stream, "\n");

		//вписываем размерности величин
		fprintf(stream, "ms\t");
		for (i = 0; i < ftChan * SigNumUpDwn->Max; i++)
			fprintf(stream, "mV\t");
		fprintf(stream, "\n");

		//вписываем сигналы
		for (j = indBgn; j < recLen; j++)//перебираем моменты времени (развёртки сигналов)
		{
			fprintf(stream, "%6.3f\t", (float)((j - indBgn) * effDT) / 1000);//время
			for (i = 0; i < SigNumUpDwn->Max; i++)//перебираем сигналы и сохраняем в dat-файл
				for (z = 0; z < ftChan; z++)
					fprintf(stream, "%7.2f\t", ((float)gSignals[gInd[i]].s[(z * recLen) + j] * sampl2mV));//напряжение
			fprintf(stream, "\n");
		}

		fclose(stream);//закрываем файл (поток вывода данных в файл)
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ShowMeanClick(TObject *Sender)
{
	//рисуем усреднённый сигнал
	__int32 i, j, k,
			fullSigLen,//суммарная длина развёрток на всех каналах (отсчёты)
			indBgn,//индекс - начало рисования
			count,//количество сигналов
			*smIndex,//массив индексов
			shft1,//номер первого элемента для ampls и peakInds
			shft2;//номер первого элемента для spans
	float average;//содержит усреднённый сигнал
	trac *averSgnl;//содержит параметры среднего сигнала
	sPr averPrm;//массив с параметрами среднего сигнала
	double *graphic;//массив с графиком

	fullSigLen = (recLen * ftChan) + (chanls - ftChan);//суммарная длина развёрток на всех каналах (отсчёты)

	averSgnl = new trac[1];
	averSgnl[0].s = new short[fullSigLen];

	count = SigNumUpDwn->Max;//количество сигналов, по которым производим усреднение
	for (j = 0; j < fullSigLen; j++)//перебор моментов времени (развёртки сигналов)
	{
		average = 0;//обнулим
		for (i = 0; i < count; i++)//перебор сигналов
			average += (float)gSignals[gInd[i]].s[j];

		average /= count;//получаем среднее для данного момента времени
		averSgnl[0].s[j] = short(floor(average + 0.5));//приводим к типу данных short
	}

	//создадим массив индексов
	smIndex = new __int32[1];
	smIndex[0] = 0;
	averPrm.ampls = new short[3 * ftChan];//все пиковые значения (амплитуды)
	averPrm.peakInds = new __int32[3 * ftChan];//все номера отсчётов с пиковыми значениями
	averPrm.spans = new __int32[5 * ftChan];//все характерные времена (периоды, фазы, интервалы)

	//вычислим параметры среднего сигнала
	//Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
	//		    __int32 *inds, short expT, bool newNull, short chn, sPr *avrP)
	Calculate(averSgnl, 1, gPorog, gPreTime, gPostTime, smIndex, experimentType, true, &averPrm);

	//выводим усреднённый сигнал
	indBgn = gTimeBgn / effDT;//индекс - начало рисования
	graphic = new double[recLen * (gRF[1] - gRF[0])];//массив с графиком
	k = 0;
	for (j = gRF[0]; j < gRF[1]; j++)
		for (i = 0; i < recLen; i++)
		{
			graphic[k] = averSgnl[0].s[(j * recLen) + i] * sampl2mV;
			k++;
		}
	PlotCurrentSignal(graphic, indBgn);

	//выделяем амплитудные точки
	csSpecPoint->Clear();//обозначим ключевые точки
	for (i = gRF[0]; i < gRF[1]; i++)
	{
		shft1 = (3 * i);//номер первого элемента для ampls и peakInds
		shft2 = (5 * i);//номер первого элемента для spans
		if (averPrm.peakInds[shft1] >= 0)//положение амплитудной точки
			csSpecPoint->AddXY((averPrm.peakInds[shft1] - indBgn), (float)averPrm.ampls[shft1] * sampl2mV);

		if (experimentType == 3)
		{
			csSpecPoint->AddXY((averPrm.peakInds[shft1 + 1] - indBgn),//выделяем точку амплитуды Na-тока
				(float)averPrm.ampls[shft1 + 1] * sampl2mV);

			csSpecPoint->AddXY((averPrm.peakInds[shft1 + 2] - indBgn),//выделяем точку амплитуды К-тока
				((float)averPrm.ampls[shft1 + 2] * (-1)) * sampl2mV);
		}
	}

	shft1 = (3 * gRF[0]);//номер первого элемента для ampls и peakInds
	shft2 = (5 * gRF[0]);//номер первого элемента для spans

	//выведем значения параметров
	curAmpl->Clear();//амплитуда
	AmplInT->Title->Text->Text = "амплитуда, " + FloatToStrF((float)averPrm.ampls[shft1] * sampl2mV, ffFixed, 6, 2) + " мВ";

	curRiseT->Clear();//время нарастания
	RiseTInT->Title->Text->Text = "время нарастания, " + IntToStr(averPrm.spans[shft2]) + " мкс";

	curFallT->Clear();//время спада
	FallTInT->Title->Text->Text = "время спада, " + IntToStr(averPrm.spans[shft2 + 1]) + " мкс";

	if (experimentType == 3)
	{
		curNaAmpl->Clear();//амплитуда Na-тока
		NaAmplInT->Title->Text->Text = "Na-ток, " + FloatToStrF((float)averPrm.ampls[shft1 + 1] * sampl2mV, ffFixed, 6, 2)+ " мВ";

		curNaDur->Clear();//длительность Na-тока
		NaDurInT->Title->Text->Text = "длительность Na-тока, " + IntToStr(averPrm.spans[shft2 + 2]) + " мкс";

		curKAmpl->Clear();//амплитуда К-тока
		KAmplInT->Title->Text->Text = "K-ток, " + FloatToStrF((float)averPrm.ampls[shft1 + 2] * sampl2mV, ffFixed, 6, 2) + " мВ";

		curSynDel->Clear();//синаптическая задержка
		SynDelInT->Title->Text->Text = "синаптическая задержка, " + IntToStr(averPrm.spans[shft2 + 3]) + " мкс";

		curSynTrns->Clear();//время синаптического проведения
		SynTrnsInT->Title->Text->Text = "синапт првд, " + IntToStr(averPrm.spans[shft2 + 4]) + " мкс";
	}

	//удаление указателей
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
	//устанавливаем тип эксперимента
	/* 1 - спонтанные
	   2 - вызвынные-внутриклеточные
	   3 - вызванные-внеклеточные
	   4 - вызванные + спонтанные */

	experimentType = 2;//внутриклеточный (K-ток, Na-ток, синаптич. задержка и синапт. проведение теряются)
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::asExtracelClick(TObject *Sender)
{
	//устанавливаем тип эксперимента
	experimentType = 3;//внеклеточный (должны появиться дополнительные данные)
	ReCalculatClick(this);//поэтому запускаем пересчёт
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::asMiniClick(TObject *Sender)
{
	//устанавливаем тип эксперимента
	experimentType = 1;//спонтанный (K-ток, Na-ток, синаптич. задержка и синапт. проведение теряются)
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ReCalculatClick(TObject *Sender)
{
	//пересчитаем параметры сигналов
	__int32 i, j, k,
			locPolar,//полярность локальная
			curS,//номер текущего сигнала (в массиве gInd)
			numOfS;
	bool nlMins;//копия параметра PStart->nulMinus->Checked
	double *grafik;//массив с графиками

	if (PStart->Visible)
		PStart->Close();//закроем окно предстарта

	PStart->GetASignal->Caption = "Пересчёт";//меняем надпись кнопки
	PStart->StartRec->Visible = false;//кнопка запуска эксперимента
	PStart->CloseWin->Visible = false;//кнопка закрытия окно предстарта
	PStart->ExpInfo->Visible = false;//кнока вызова окна ввода пользовательской информации
	ReCalculat->Tag = experimentType;//передаём тип эксперимента в PStart
	PStart->GetASignal->OnClick = PStart->ReCalClick;//меняем функцию кнопки
	PStart->SetControlVis(experimentType);//делаем видимыми нужные органы управления и убираем ненужные
	PStart->Invert->Checked = false;//изменение статуса индикатора инвертирования
	PStart->Invert->OnClick = PStart->InvertClick;//изменение реакции на смену индикатора

	numOfS = SigNumUpDwn->Max;//общее количество доступных сигналов
	curS = gInd[(SigNumUpDwn->Position - 1)];//номер текущего сигнала (в массиве gInd)

	PStart->Caption = "Пересчёт параметров";

	//создаём график
	grafik = new double[recLen * (gRF[1] - gRF[0])];//массив с графиками
	k = 0;
	for (j = gRF[0]; j < gRF[1]; j++)
		for (i = 0; i < recLen; i++)
		{
			grafik[k] = gSignals[curS].s[(j * recLen) + i] * sampl2mV;
			k++;
		}
	//PlotStandardSignal(double *sData, __int32 sRecLen, __int32 sEffDT, __int32 sIndBgn)
	PStart->PlotStandardSignal(grafik, recLen, effDT, 0);//рисуем текущий сигнал для наглядности

	PStart->PreTime->Text = IntToStr(devExpParams_int[8]);//пре-время
	PStart->PostTime->Text = IntToStr(devExpParams_int[9]);//пост-время
	PStart->Porog->Text = IntToStr(devExpParams_int[10]);//порог

	PStart->timeOfDrawBgn = 0;//(float)(gTimeBgn);//передаём значение индекса-начала в модуль PStart
	PStart->RefreshLines(effDT);//перерисовать линии
	PStart->Invert->Checked = false;//не инвертировать
	PStart->NulMinus->Checked = false;//не вычетать ноль-линию

	if (PStart->ShowModal() == mrOk)//если нажат "Пересчёт"
	{
		//после закрытия окна и произойдёт пересчёт, если был нажат "Пересчёт"
		gPreTime = StrToInt(PStart->PreTime->Text);
		gPostTime = StrToInt(PStart->PostTime->Text);
		gPorog = short((float)StrToInt(PStart->Porog->Text) / sampl2mV);//пороговое напряжение (отсчёты)

		//если выставлена галочка invert, то предварительно инвертируем сигналы
		if (PStart->Invert->Checked)
			for (j = gRF[0]; j < gRF[1]; j++)
				for (i = 0; i < recLen; i++)
					gSignals[gInd[j]].s[i] = (-1) * gSignals[gInd[j]].s[i];

		//если выбрано "вычетать ноль-линию", то будет расчитана ноль-линия для каждого сигнала отдельно
		//newNull = PStart->nulMinus->Chacked;//указываем, что запускается ПЕРЕсчёт параметров

		//собственно пересчёт параметров с новыми настройками
		//Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
		//	        __int32 *inds, short expT, bool newNull, short chn, sPr *avrP)
		Calculate(gSignals, numOfS, gPorog, gPreTime, gPostTime, gInd, experimentType, PStart->NulMinus->Checked, NULL);

		//===================================
		//запомним новые настройки
		devExpParams_int[8] = StrToInt(PStart->PreTime->Text);
		devExpParams_int[9] = StrToInt(PStart->PostTime->Text);
		devExpParams_int[10] = StrToInt(PStart->Porog->Text);

		//вычисляем первую точку рисования сигналов
		gTimeBgn = 0;
		if (experimentType == 2)
			gTimeBgn = gPostTime;
		else if (experimentType == 3)
			gTimeBgn = gPreTime;

		ClearGraphics();//затираем графики
		FillParamTable();//перезаполняем таблицу
		//AddParamsPoints(trac *apSignals, __int32 apSigNum, __int32 *apIndex, short apExpType)
		AddParamsPoints(gSignals, SigNumUpDwn->Max, gInd, experimentType);//перерисуем все графики
		CopyRefSignal_forGists();//расчёт и построение гистограмм с новыми параметрами
		ShowSignal(SigNumUpDwn->Position);//выводим тот же сигнал, который был нарисован до пересчёта
    }

	//восстанавливаем функции "инвертирования"
	PStart->Invert->Checked = false;//PStart->invert->Visible = true;
	PStart->Invert->OnClick = NULL;//изменение реакции на смену индикатора
	PStart->GetASignal->OnClick = PStart->GetASignalClick;//восстанавливаем функцию

	//восстанавливаем вид окна
	PStart->GetASignal->Caption = "Пример сигнала";//восстанавливаем надпись кнопки
	PStart->StartRec->Visible = true;//кнопка запуска эксперимента
	PStart->CloseWin->Visible = true;//кнопка закрытия окно предстарта
	PStart->ExpInfo->Visible = true;//кнока вызова окна ввода пользовательской информации

	delete[] grafik; grafik = NULL;
}
//---------------------------------------------------------------------------

void TGraphs::ReplotExamplRecalc()
{
	//перерисовка эталонного сигнала при прересчёте параметров сигналов

	__int32 i, j, k,
			curS;//номер текущего сигнала
	double *grafik,//единый массив с сигналами со всех каналов
		   locPolar;//полярность

	curS = gInd[(SigNumUpDwn->Position - 1)];

	locPolar = 1;
	if (PStart->Invert->Checked)
		locPolar = (-1);

	grafik = new double[recLen * (gRF[1] - gRF[0])];//массив с графиками
	k = 0;
	for (j = gRF[0]; j < gRF[1]; j++)
		for (i = 0; i < recLen; i++)
		{
			grafik[k] = locPolar * gSignals[curS].s[(j * recLen) + i] * sampl2mV;
			k++;
		}
	//PlotStandardSignal(double *sData, __int32 sRecLen, __int32 sEffDT, __int32 sIndBgn)
	PStart->PlotStandardSignal(grafik, recLen, effDT, 0);//рисуем текущий сигнал для наглядности

	delete[] grafik; grafik = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::GraphChartDblClick(TObject *Sender)
{
	//разворачиваем чарт на всю ширину окна графиков
	__int32 i;
	TChart *theChart;//указатель на обрабатываемый чарт

	_isfull = !_isfull;//развёрнут ли какой-нибудь график

	theChart = (TChart*)Sender;//обрабатываемый чарт
	theChart->AllowZoom = false;//запрещаем увеличивать

	if (_isfull)
	{
		theChart->Width = Graphs->Width - 7;//новая ширина (на всё окно)
		theChart->Height = Graphs->Height - 50;//новая высота (на всё окно)
		theChart->Left = 0;//левый край
		theChart->Top = 0;//верхний край

		for (i = 0; i < 9; i++)
			if (i != theChart->Tag)
			{
				mask[i] = allCharts[i]->Visible;//запоминаем параметр
				allCharts[i]->Visible = false;//делаем невидимыми все остальные графики
			}
	}
	else
	{
		for (i = 0; i < 9; i++)
			if (i != theChart->Tag)
				allCharts[i]->Visible = mask[i];//восстанавливаем значение параметра

		Resized(this);//перерисовываем графики
	}

	theChart->AllowZoom = true;//разрешаем увеличивать
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::Resized(TObject *Sender)
{
	__int32 numOfgraphs = 0, // количество высвечиваемых графиков
			i,
			winWidth,//ширина окна графиков
			winHeight,//высота окна графиков
			//индексы в массиве ссылок на видимые графики (чарты, не более пяти)		
			iVCh[5] = {0, 0, 0, 0, 0};

	if (pValues->Visible)
	{
		//подгоняем размеры таблицы
		pValues->Top = 0;
		pValues->Left = 0;
		winWidth = 0;
		//winHeight = 0;
		for (i = 0; i < pValues->ColCount; i++)
			winWidth += pValues->ColWidths[i];

		if (winWidth < 415)
			winWidth = 415;

		pValues->Width = winWidth + 30;//плюс ширина scrollBar

		Graphs->Width = pValues->Width + 8;
		pValues->Height = Graphs->Height - 58;
	}
	else
	{
		//копируем ссылки на графики (чарты), которые видны
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
	
		//меняем размер чартов
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
			//SignalTrack->Visible = true - виден всегда
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
	//рисуем быстро текущий (или один из последних) сигнал во время сбора данных
	/*
	pcsData - массив с графиками
	pcsIndBgn - начальный отсчёт сигнала
	*/

	__int32 i;

	SignalTrack->AutoRepaint = false;//отключаем автопрорисовку для ускорения процесса рисования

	for (i = gRF[0]; i < gRF[1]; i++)//рисуем быстро все каналы
	{
		curntSigChannls[i]->Clear();//затираем старый график
		curntSigChannls[i]->AddArray(&pcsData[pcsIndBgn + ((i - gRF[0]) * recLen)], (recLen - pcsIndBgn) - 1);
	}

	SignalTrack->BottomAxis->SetMinMax(0, double((recLen - pcsIndBgn) * effDT) / 1000);
	SignalTrack->AutoRepaint = true;//возвращаем автопрорисовку, чтобы график обновился
	SignalTrack->Repaint();//Refresh();//обновляем чарт

	pcsData = NULL;
}
//---------------------------------------------------------------------------

void TGraphs::AddParamsPoints(trac *apSignals, __int32 apSigNum, __int32 *apIndex, short apExpType)
{
	//добавим точки на графики параметров
	/*
	apSignals - структура с сигналами
	apSigNum - количество обрабатываемых сигналов
	apIndex - массив индексов
	apAmplCoeff - коэффициент пересчёта амплитуд
	apExpType - тип эксперимента
	*/
	__int32 i, j,
			nrc,//количество обновляемых чартов
			shft1,//номер первого элемента для ampls и peakInds
			shft2;//номер первого элемента для spans

	//подрисовываем графики параметров
	nrc = 4 + (5 * __int32(apExpType == 3));//количество обновляемых чартов
	for (i = 1; i < nrc; i++)
		allCharts[i]->AutoRepaint = false;//отключаем автопрорисовку всех чартов, кроме SignalTrack

	for (j = 0; j < apSigNum; j++) //apSigNum - 1
	{
		for (i = gRF[0]; i < gRF[1]; i++)
		{
			shft1 = (3 * ftChan * apIndex[j]) + (3 * i);//номер первого элемента для ampls и peakInds
			shft2 = (5 * ftChan * apIndex[j]) + (5 * i);//номер первого элемента для spans
			sigAmpls[i]->AddXY(apSignals[apIndex[j]].appear + plotFrom, cPrm[0].ampls[shft1] * sampl2mV);
		}

		shft1 = (3 * ftChan * apIndex[j]) + (3 * gRF[0]);//номер первого элемента для ampls и peakInds
		shft2 = (5 * ftChan * apIndex[j]) + (5 * gRF[0]);//номер первого элемента для spans

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
        allCharts[i]->AutoRepaint = true;//восстанавливаем автопрорисовку чартов
		allCharts[i]->Repaint();//Refresh();//обновляем чарт
	}

	SetSeriesMultipliers();//поменяем масштабные факторы для графиков
	apSignals = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ChartsClickSeries(TCustomChart *Sender, TChartSeries *Series,
	  int ValueIndex, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	//кликнул по графику основных амплитуд
	SigNumUpDwn->Position = ValueIndex + 1;//выведем соответствующий сигнал
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::pValuesSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
	//при выделении какой-либо клетки меняем номер текущего сигнала
	SigNumUpDwn->Position = ARow;
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::selAllClick(TObject *Sender)
{
	//выделить все видимые элементы таблицы

//	pValues->Selection.Left = 2;
//	pValues->Selection.Right = 2;
//	pValues->Selection.Top = 4;
//	pValues->Cols->
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::selCopyClick(TObject *Sender)
{
	//копируем выделенную область таблицы (в буфер обмена)
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

	//собственно копирование
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

	if (OpenClipboard(NULL))//открываем буфер обмена
	{
		EmptyClipboard();//очищаем буфер

		hgBuffer = GlobalAlloc(GMEM_DDESHARE, s.Length() + 1);//выделяем память
		chBuffer = (char*)GlobalLock(hgBuffer);//блокируем память
		strcpy(chBuffer, LPCSTR(s.c_str()));//копируем текст по адресу chBuffer
		GlobalUnlock(hgBuffer);//разблокируем память
		SetClipboardData(CF_TEXT, hgBuffer);//помещаем текст в буфер обмена

		CloseClipboard(); //закрываем буфер обмена
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::amplInTAfterDraw(TObject *Sender)
{
	//рисуем метки по ходу эксперимента
	__int32 i, j,
			pCount,//количество точек на графике
			pnOnGrph,//номер точки на графике
			x0, x1,//х - координата для надписи
			y0, y1,//y - координата для надписи
			fH,//высота надписи
			fW;//ширина надписи

	if (showMarks->Checked)
		for (i = 0; i < ExpNotes->addUMark->Tag; i++)
		{
			for (j = gRF[0]; j < gRF[1]; j++)//j - номер канала который сейчас просматриваем
			{
				pCount = sigAmpls[j]->Count();//количество точек на графике
				if (ExpNotes->theMarker->chanN[j])//видна ли метка на данном канале
				{
					pnOnGrph = ExpNotes->theMarker->pointOnGraph;//номер точки на графике
					if ((pnOnGrph < pCount) && (pnOnGrph >= 0))//сигнал с данным номером есть на графике
					{
						x0 = sigAmpls[j]->CalcXPos(pnOnGrph);//x0 координата надписи
						y0 = sigAmpls[j]->CalcYPos(pnOnGrph);//y0 координата надписи
						fH = AmplInT->Canvas->TextHeight(ExpNotes->theMarker->textMark);//высота надписи
						fW = __int32((float)AmplInT->Canvas->TextWidth(ExpNotes->theMarker->textMark) * 1.15);//ширина надписи
						x1 = x0 - floor(fW / 2);//x1 координата надписи
						y1 = y0 - (2 * fH);//y1 координата надписи

						AmplInT->Canvas->Rectangle(x1, y1, x1 + fW, y1 + fH);//рамка для надписи
						AmplInT->Canvas->TextOutA(x1 + 2, y1, ExpNotes->theMarker->textMark);//сама надпись
						AmplInT->Canvas->Line(x0, y0 - fH, x0, y0);//стрелка (указывает на точку, к которой привязана надпись)
					}
				}
			}
			ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
		}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::showMarksClick(TObject *Sender)
{
	//отображать ли "заметки по ходу эксперимента"
	showMarks->Checked = !showMarks->Checked;//
	AmplInT->Refresh();//перерисуем или затрём метки
}
//---------------------------------------------------------------------------

void TGraphs::CopyRefSignal_forGists()
{
	//копируем ссылку на структуру с сигналами и передаём её в модуль построения гистограмм

	if ((gSignals) && (SigNumUpDwn->Max > 1))//количество сигналов больше одного
		Gists->GistsRefresh(-1);
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::CrntSigKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
	//реакция на смену номера пользователем
	if (((Key >= '0') && (Key <= '9')) || (Key != '\b'))//проверим допустимость введённых символов
		SigNumUpDwn->Position = StrToInt(CrntSig->Text);//
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::CrntSigKeyPress(TObject *Sender, char &Key)
{
	//проверка допустимости введения символа в поле номера сигнала

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::CrntSigKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	//отслеживаем нажатие стрелок (для индикатора номера сигнала)

	if (!CrntSig->ReadOnly)//
	{
		if (Key == VK_UP)//кнопка "вниз" - предыдущий сигнал
			SigNumUpDwn->Position--;//
		else if (Key == VK_DOWN)//кнопка "вверх" - следующий сигнал
			SigNumUpDwn->Position++;//
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::remGrpSigsClick(TObject *Sender)
{
	//удаление заданной группы сигналов

	if (gSignals)//сигналы существуют
	{
		ChoosDlg->setsForNumbersChs();//настраиваем окно диалога для ввода номеров сигналов (удаляемых)

		ChoosDlg->sav = false;//задано удаление группы сигналов
		ChoosDlg->Caption = "Удаление группы сигналов";
		ChoosDlg->ShowModal();
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::saveGrpSigsClick(TObject *Sender)
{
	//сохранение заданной группы сигналов

	if (gSignals)
	{
		ChoosDlg->setsForNumbersChs();//настраиваем окно диалога для ввода номеров сигналов (сохраняемых)

		ChoosDlg->sav = true;//задано сохранение группы сигналов
		ChoosDlg->Caption = "Сохранение группы сигналов";
		ChoosDlg->ShowModal();
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::dividGrpsClick(TObject *Sender)
{
	//разбиение массива данных на группы по nInGrp сигналов

	if (gSignals)
	{
		ChoosDlg->setsForNOfGrp();//настраиваем окно диалога для ввода числа сигналов в группе (nInGrp)
		ChoosDlg->sav = true;//будем пересохранять текущий файл
		ChoosDlg->ShowModal();//вызываем диалог для ввода размера групп сигналов
	}
}
//---------------------------------------------------------------------------

void TGraphs::RemovEnteredNumbers(__int32 *removIndex, __int32 count)
{
	//сохранение или удаление группы сигналов
	/*
	svRmIndex - указатель на массив с введёнными номерами
	count - количество удаляемых сигналов
	*/
	__int32 i, j;

	//удаляем все указанные сигналы
	for (i = 0; i < count; i++)
		RemoveSignals(removIndex[i]);//собственно исключение сигнала из списка рассматриваемых

	if (ChoosDlg->sav)//задано сохранение оставшихся файлов
		GphSaveClick(this);//сохраняем группу сигналов с указанным именем
	else
		ResetVisibility();//запускаем прорисовку
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::PrintWinClick(TObject *Sender)
{
	//распечатать всё, что изображено в окне графиков
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

	frmImg->SaveToFile(imgFlNm);//сохранение файла
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::SignalTrackMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	//подрисуем значение напряжения в данной точке
	__int32 cp,//номер точки графика, на которую указали
			x0, x1,//х - координата для надписи
			y0, y1,//y - координата для надписи
			fH,//высота надписи
			fW;//ширина надписи
	float gX, gY;//значения координат на графике
	AnsiString voltText;//надпись с напряжением в данной точке
	TRect rc;

	if ((Button == 0) && gSignals && (curntSigChannls[gRF[0]]->Count() > 0))//если нажата правая кнопка мыши и графики нарисованы
	{
		gX = curntSigChannls[gRF[0]]->XScreenToValue(X);//х-коодината кликнутой точки

		//найдём номер точки графика, которая ближе остальных к кликнутой
		x0 = 0;//левая граница
		x1 = curntSigChannls[gRF[0]]->XValues->Count() - 1;//правая граница
		cp = x0 + floor((x1 - x0) / 2);//затравочный номер точки графика
		while ((x1 - x0) > 3)
		{
			gY = (float)cp * effDT * 1e-3;//х-координата точки графика
			if (gY < gX)
				x0 = cp;
			else
				x1 = cp;

			cp = x0 + floor((x1 - x0) / 2);//уточнённый номер точки графика
		}
		cp = x0;//номер точки графика, на которую указали (Locate(graphX) - требует точного соответствия)

		if (cp >= 0)
		{
			x0 = curntSigChannls[gRF[0]]->CalcXPos(cp);//точка прикрепления надписи на графике
			y0 = curntSigChannls[gRF[0]]->CalcYPos(cp);//точка прикрепления надписи на графике
			gY = curntSigChannls[gRF[0]]->YValues->operator [](cp);//(float)gSignals[i].s[indBgn + cp] * sampl2mV;//с поправкой на gIndBgn
			voltText = "x=" + FloatToStrF(gX, ffFixed, 6, 2) + "; "
					 + "y=" + FloatToStrF(gY, ffFixed, 6, 2);

            //задаём облик надписи
			SignalTrack->Canvas->Pen->Width = 2;
			SignalTrack->Canvas->Pen->Color = clGreen;
			SignalTrack->Canvas->Brush->Color = clWhite;
			SignalTrack->Canvas->Font->Size = 10;
			SignalTrack->Canvas->Font->Style = TFontStyles() << fsBold;
			SignalTrack->Canvas->Font->Name = "Arial";

			//размеры надписи
			fH = (__int32)((float)SignalTrack->Canvas->TextHeight(voltText) * 1.15);
			fW = (__int32)((float)SignalTrack->Canvas->TextWidth(voltText) * 1.05);

			x1 = x0 - floor(fW / 2);
			y1 = y0 - 2 * fH;

			rc.left = x1 + 1; rc.top = y1 + 1;
			rc.right = x1 + fW - 2; rc.Bottom = y1 + fH - 2;

			SignalTrack->Canvas->Rectangle(x1, y1, x1 + fW, y1 + fH);//рамка для надписи
			SignalTrack->Canvas->FillRect(rc);
			SignalTrack->Canvas->TextOutA(x1 + 1, y1, voltText);//сама надпись
			SignalTrack->Canvas->Line(x0, y0 - fH, x0, y0);//стрелка (указывает на точку, к которой привязана надпись)
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::expParamsClick(TObject *Sender)
{
	//показать информацию об эксперименте

	//настроим элементы окна
	ExpNotes->mainLinesLbl->Caption = "Информация о файле";
	ExpNotes->PIDates->Visible = true;//поле данных о файле
	ExpNotes->usersNotes->Visible = false;//поле для пользовательских данных
	ExpNotes->addUMark->Visible = false;//поле ввода заметок по ходу эксперимента
	ExpNotes->addMarkLbl->Visible = false;//заметки-лэйбл

	ExpNotes->PIDates->Top = ExpNotes->usersNotes->Top;
	ExpNotes->PIDates->Left = ExpNotes->usersNotes->Left;
	ExpNotes->PIDates->Width = ExpNotes->usersNotes->Width;
	ExpNotes->PIDates->Height = ExpNotes->addUMark->Top + ExpNotes->addUMark->Height;

	ExpNotes->Show();
}
//---------------------------------------------------------------------------

void TGraphs::ConvertBinary(bool cTyp)
{
	//чтение бинарного файлы с результатами непрерывной записи всего подряд (осциллограф)
	//перевод данных в текстовый вид
	/*
	cTyp - тип конвертации (false - в текст, true - в pra-формат)
	*/

	HANDLE ContinRecFile;//хэндл файла, из которого читаем данные
	FILE *stream;//поток вывода информации в файл
	__int32 i, j, k, t,
			recStep,//шаг записи
			numOfSig,//число записанных сигналов
			flSize,//размер файла
			totSigNum,//предельное количество сигналов
			backLen,//время назад (отсчёты)
			realSigNum,//количество обнаруженных сигналов
			leadChan,//ведущий канал (основной сигнал)
			nGain;//коэффициент усиления
	short *dataBlock,//блок с одиночыми отсчётами от каждого канала
		  *data,//массив с данными для всех каналов
		  **dataCh,//массив с данными поканально
		  polarity;//полярность
	AnsiString fileParametr,//общий коментарий
			   convertedData;//имя файла с конвертированными данными
	char *pidate;//информация о программе и даты (Programm Information and Date)
	float discrT1,//реальное время дискретизации (мкс)
		  mVOnChan;//сигнал (напряжение) на канале
	bool forwc;//индикатор успешности чтения блока данных из файла
	unsigned long bytesRead,
				  totRead,//всего прочитано байт
				  lpFileSizeHigh;//большой размер файла

	totRead = 0;
	pidate = new char[40];//готовим массивы для записи данных из файла

	//открываем и читаем файл с экспериментальными данными
	ContinRecFile = CreateFile(OpenDlg->FileName.c_str(), GENERIC_READ, FILE_SHARE_READ,
				   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	//считываем параметры файл
	forwc = ReadFile(ContinRecFile, (void*)pidate, sizeof(char) * 37, &bytesRead, 0);
	totRead += bytesRead;
	fileParametr = pidate;
	i = fileParametr.AnsiPos("ElphAcqu v5");//ищем отметку о версии файла (программы)
	if (i <= 0)//нет такой строки в файле
	{
		Experiment->DevEvents->Text = "этот файл не открою";//сообщение о невозможности прочитать файл неизвестного типа (не тот заголовок)
		CloseHandle(ContinRecFile);//закрываем файл
		return;
	}

	//переменные, уникальные для данной записи (т.е. все необходимые параметры)
	/* последовательность записи обязательных параметров
		sampls2mV ----(1) - коэффициент перевода амплитуд из отсчётов АЦП в (милли)вольты
		oChanNumb ----(2) - количество используемых каналов
		minDiscrTime -(3) - минимальное (истинное) время дискретизации
		oDiscrTime ---(4) - пользовательское время дискретизации
	*/
	forwc = ReadFile(ContinRecFile, (void*)&chanls, sizeof(__int32), &bytesRead, 0); //количество сканируемых каналов
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&nGain, sizeof(__int32), &bytesRead, 0);//коэффициент усиления
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&discrT1, sizeof(float), &bytesRead, 0);//реальное время дискретизации (микросекунды)
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&discrT, sizeof(float), &bytesRead, 0); //эффективное время дискретизации (микросекунды)
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&Experiment->maxADCAmp, sizeof(__int32), &bytesRead, 0);//максимальная амплитуда АЦП (в отсчётах)
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&Experiment->maxVoltage, sizeof(float), &bytesRead, 0);//диапазон допустимых входных напряжений (милливольты)
	totRead += bytesRead;
	forwc = ReadFile(ContinRecFile, (void*)&leadChan, sizeof(__int32), &bytesRead, 0);//ведущий канал (основной сигнал)
	totRead += bytesRead;

	Experiment->a.m_nGain = nGain;//коэффициент усиления
	sampl2mV = Experiment->maxVoltage / (Experiment->maxADCAmp * nGain);//коэффициент перевода отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
	ftChan = chanls;//количество каналов с полными развёртками (full trace channels)
	effDT = discrT * chanls;//эффективное время дискретизации = discrT * chanls
	bytesRead = (37 * sizeof(char)) + (3 * sizeof(float)) + (4 * sizeof(__int32));// * 2) ;//размер заголовка файла
	if (totRead != bytesRead)//ошибка чтения
	{
		Experiment->DevEvents->Text = "ошибка чтения";//сообщение об ошибке
		CloseHandle(ContinRecFile);//закрываем файл
		return;
	}

	convertedData = OpenDlg->FileName;//имя файла с конвертированными данными
	convertedData.Delete(convertedData.Length() - 3, 4);//удаляем расширение
	
	if (cTyp)//конвертировать в pra-файл
	{
		SaveDlg->FileName = convertedData + ".pra";//имя файла

		flSize = GetFileSize(ContinRecFile, &lpFileSizeHigh);//размер файла
		flSize -= ((37 * sizeof(char)) + sizeof(__int32) + (3 * sizeof(float)));//вычитаем размер заголовка файла
		flSize = floor((flSize / sizeof(short)) / chanls);//количество отсчётов на один канал

		data = new short[chanls * flSize];//предвыделение памяти для данных
		forwc = ReadFile(ContinRecFile, (void*)data, sizeof(short) * chanls * flSize, &bytesRead, 0);
		
		dataCh = new short*[chanls];//предвыделение памяти для данных
		for (i = 0; i < chanls; i++)
			dataCh[i] = new short[flSize];//предвыделение памяти для данных
		csSpecPoint->Clear();//
		j = 0;
		for (i = 0; i < chanls; i++)
			for (t = 0; t < flSize; t++)
			{
				dataCh[i][t] = data[j];
				j++;
			}
		delete[] data; data = NULL;
		PStart->continRec = dataCh;//копия ссылки на массив данными

		Graphs->gRF[0] = 0;//первый рисуемый канал
		Graphs->gRF[1] = chanls;//последний рисуемый канал
		for (i = 0; i < maxChannels; i++)
		{
			Graphs->curntSigChannls[i]->XValues->Multiplier = double(effDT) / 1000;//выставим множители для "быстрых" графиков
			PStart->exmplChannls[i]->XValues->Multiplier = double(effDT) / 1000;//выставим множители для "быстрых" графиков
		}
		//рисум сигнал на в окне ПредСтарта и ждём команды
		if (PStart->winDraw(flSize) == mrOk)//нажата кнопка "Стар" (запустить расчёт)
		{
			//ищем сигналы
			recLen = (__int32)floor((float(StrToInt(PStart->LenSpont->Text) * 1000) / effDT) + 0.5);//длина развёртки сигнала (отсчёты)
			backLen = (__int32)floor((float(StrToInt(PStart->BackTime->Text) * 1000) / effDT) + 0.5);//время назад (отсчёты)
			multiCh = true;//режим сбора данных (true = многоканальный)
			ftChan = 1 + (chanls - 1) * (__int32)multiCh;//количество каналов с полными развёртками (full trace channels)
			gPorog = short((float)StrToInt(PStart->Porog->Text) / sampl2mV);//пороговое напряжение (отсчёты)
			polarity = PStart->postCompInvert;//полярность

			ExpNotes->addUMark->Tag = 0;//нет заметок

			devExpParams_int = new __int32[ehead[2].numOfElements];//создаём массив с параметрами устройства и эксперимента (__int32)
			devExpParams_int[0]	= multiCh;//0//режим сбора данных (true = многоканальный)
			devExpParams_int[1]	= recLen;//1//длина развёртки сигнала в отсчётах
			devExpParams_int[2] = (__int32)discrT;//2//время дискретизации (микросекунды)
			devExpParams_int[3]	= chanls;//3//количество сканируемых каналов
			devExpParams_int[4]	= leadChan;//4//ведущий канал (основной сигнал)
			devExpParams_int[5]	= nGain;//5//коэффициент усиления
			devExpParams_int[6] = ExpNotes->addUMark->Tag;//6//количество заметок
			devExpParams_int[7] = 1;//7//тип эксперимента, глобальный вариант
			devExpParams_int[8] = 0;//8//gPreTime (в микросекундах)
			devExpParams_int[9] = 0;//9//gPostTime (в микросекундах)
			devExpParams_int[10] = (__int32)gPorog;//10//амплитудный порог (отсчёты)
			devExpParams_int[11] = Experiment->maxADCAmp;//11//максимальная амплитуда (в отсчётах)
			devExpParams_int[12] = -Experiment->maxADCAmp;//12//минимальная амплитуда (в отсчётах)
			devExpParams_int[13] = Experiment->a.m_nFirstChannel;//13//ведущий аппаратный канал
			devExpParams_int[14] = 0;//14//не используется

			//float - параметры
			devExpParams_float = new float[ehead[3].numOfElements];//создаём массив с параметрами устройства и эксперимента (float)
			devExpParams_float[0] = sampl2mV;//0//коэффициент перевода амплитуд из отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
			devExpParams_float[1] = Experiment->maxVoltage;//1//диапазон входных напряжений (в милливольтах)
			devExpParams_float[2] = -1;//2//период стимуляции
			devExpParams_float[3] = discrT;//3//время дискретизации (микросекунды)
			devExpParams_float[4] = 0;//4//не используется
			devExpParams_float[5] = 0;//5//не используется

			totSigNum = floor(flSize / recLen);//предельное количество спонтанных сигналов
			if (totSigNum > limitSigNum)//ограничение на количество сигналов
				totSigNum = limitSigNum;//указываем максимальное количество сигналов
			gSignals = Graphs->CreatStructSignal(totSigNum, recLen);//структура с сигналами и их атрибутами

			realSigNum = 0;//количество обнаруженных сигналов
			i = 0;
			while ((i < flSize) && (realSigNum < totSigNum))//анализируем ведущий канал
			{
				if ((dataCh[0][i] * polarity) > gPorog)//обнаружен сигнал (добавляем его)
				{
					gSignals[realSigNum].appear = i * effDT * 1e-6;//время возникновения сигнала-синхроимпульса (секунды от начала эксперимента)
					for (t = 0; t < chanls; t++)//перебираем каналы
					{
						if ((i - backLen) < 0)//начальная часть сигнала не записана
							k = backLen - i;//пропускаем недостающие отсчёты
						else//весь сигнал записан
							k = 0;//записываем все отсчёты
						for (j = k; (j < recLen) && ((i + j) < flSize); j++)//перебираем отсчёты
						{
							gSignals[realSigNum].s[j] = dataCh[t][i - backLen + j] * polarity;//отсчёты сигнала
						}
					}
					realSigNum++;//количество обнаруженных сигналов
					i += (recLen - backLen);//пропускаем участок с обнаруженным сигналом
				}
				else
					i++;//следующий отсчёт
			}
			gInd = new __int32[realSigNum];//индексы принятых сигналов. При удалении сигнала, убираем его номер из этого массива
			for (i = 0; i < realSigNum; i++)
            	gInd[i] = i;//номера всех сигналов
			Graphs->SaveExpDataToFile(1, NULL, realSigNum, false);//сохранение данных
		}
		//удаляем массивы
		delete[] dataCh; dataCh = NULL;
	}
	else//конвертирова в текст
	{
		convertedData += ".dat";
		stream = fopen(convertedData.c_str(), "w");//создаём файл с текстовыми данными
		fprintf(stream, "time");//вписываем заголовок
		for (i = 0; i < chanls; i++)
		{
			fileParametr = "\tChann_";
			fileParametr += IntToStr(i + 1);
			fprintf(stream, fileParametr.c_str());//вписываем заголовок
		}
		fprintf(stream, "\nsec");//вписываем заголовок
		for (i = 0; i < chanls; i++)
			fprintf(stream, "\tmV");//вписываем заголовок
		fprintf(stream, "\n");//вписываем заголовок

		//читаем, конвертируем и выводим данные в другой файл
		dataBlock = new short[chanls];//блок с одиночыми отсчётами от каждого канала
		t = 0;//счётчик времени
		recStep = floor(discrT / discrT1);//шаг записи
		while (forwc)
		{
			forwc = ReadFile(ContinRecFile, (void*)dataBlock, chanls * sizeof(short), &bytesRead, 0);
			forwc = (bytesRead > 0);
			if (forwc)//если что-то прочиталось
			{
				fprintf(stream, "%f", (t * discrT * chanls * 1e-6));//вписываем время (секунды)
				//плюс (discrTime * номер канала), т.к. реальная и пользовательская дискретизации не совпадают
				for (i = 0; i < chanls; i++)
				{
					mVOnChan = (float)dataBlock[i] * sampl2mV;//сигнал (напряжение) на канале
					fprintf(stream, "\t%6.2f", mVOnChan);//выводим значение напряжения на канале в данный момент
				}
				fprintf(stream, "\n");
			}
			t += recStep;//увеличивем счётчик времени
		}

		delete[] pidate; pidate = NULL;
		delete[] dataBlock; dataBlock = NULL;
		fclose(stream);//закрываем файл (закрываем поток вывода данных в файл)
		CloseHandle(ContinRecFile);//закрываем файл
	}
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::ShowTheChannlClick(TObject *Sender)
{
	//показать параметры выбранного канала
	TMenuItem *copyOfItem;//копия указателя на элемент списка каналов, вызвавший функцию (выбранный пользователем)
	__int32 i;

	//выяснить номер выбранного канала (элемента списка каналов)
	copyOfItem = (TMenuItem*)Sender;//преобразуем к нужному типу данных ссылку на объекты вызвавший данную функцию
	copyOfItem->Checked = true;//меняем параметры вызвашего данную функцию объекта (например ChannNumb[0])
	gRF[0] = ShowChanN->IndexOf(copyOfItem);//получаем номер выбранного канала
	ShowChanN->Caption = "Канал " + IntToStr(gRF[0] + 1);//указан выбранный канал
	gRF[1] = gRF[0] + 1;//отображается один выбранный канал

	FillParamTable();//заполняем таблицу значениями параметров
	ResetVisibility();//запускаем прорисовку
}
//---------------------------------------------------------------------------

void __fastcall TGraphs::SAllChanClick(TObject *Sender)
{
	//меняем отображение каналов

	if (!SAllChan->Checked)//рисовать все каналы
	{
		SAllChan->Checked = true;
		ShowChanN->Caption = "Все каналы";
		gRF[0] = 0;//первый отображаемый сигнал
		gRF[1] = ftChan;//последний отображаемый сигнал
		FillParamTable();//заполняем таблицу значениями параметров
		ResetVisibility();//запускаем прорисовку
	}
}
//---------------------------------------------------------------------------

