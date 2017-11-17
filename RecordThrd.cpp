//$$---- Thread CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <math.h>
#include <algorith.h>
#include <stdio.h>

#pragma hdrstop

#include "RecordThrd.h"
#include "PredStart.h"
#include "BuildProtocol.h"
#include "CalcParams.h"
#include "Gistograms.h"
#include "MinorChannel.h"
#include "ExperimentNotes.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

IADCDevice *r_pADC;//указатель на интерфейс драйвера
IADCUtility *r_pUtil;//интерфейс драйвера
ADCParametersDMA  r_a;//структура параметров устройства
trac *rSignals;//структура с сигналами и параметрами
trac *rSpntSignals;//структура с сигналами и параметрами

bool rAppEndData;//выбрано ли создание файла с возможностью дозаписи
__int32 rBlockSize,//размер блока данных
		rStimPrd,//период стимуляции (миллисекунды)
		rSpntRecLen,//длина спонтанного сигнала в отсчётах (для режима №4)
		rTimeBgn,//момент начала сигнала (микросекунды)
		specK1,//специальная переменная для передачи номера1 //номер сигнала, с которого начинаются неотрисованные сигналы
		specK2,//специальная переменная для передачи номера2 //количество неотрисованных сигналов
		rPreTime,//начало пресин. ответа (микросекунды)
		rPostTime,//начало постсин. ответа (микросекунды)
		rTotNumOfS,//заданное к сбору число сигналов в эксперименте
		rRealNumOfS,//действительное число записанных сигналов
		rCountlm,//счётчик для перебора элементов выборки при расчёте локального среднего
		rSamps,//количество отсчётов принятых для вычисления локального среднего
		*rInstr,//массив с инструкцией (протокол стимуляции)
		rReadOrder[maxChannels];//массив с последовательностью чтения каналов
__int64 rNextTic,//момент (в тактах процессора) подачи следующего синхроимпульса
		rBeginTakt;//такт процессора, при котором начинается вополнение сценария
short rTypeOfExp,//тип эксперимента
	  rConstMinus,//вычитание постоянной составляющей
      rPorog,//пороговое напряжение (отсчёты)
	  rPolarity,//полярность сигнала
	  *rExtractLocMean;//массив выборки (extract) локального среднего
double rLocMean[maxChannels],//локальное среднее на всех каналах; 5 - с запасом (нужно для нивелирования постоянной составляющей входного сигнала)
	   *rmGrafik,//график для рисования
	   *rmGrafikM;//график для рисования спонтанных сигналов (в режиме №4)
FILE *rDiscontinFile;//хэндл файла, в который сохраняются данные (при прерывистом сборе)

//---------------------------------------------------------------------------
__fastcall TRecordThread::TRecordThread(bool CreateSuspended) : TThread(CreateSuspended)
{
	//Graphs->CrntSig->Tag - глобальный счётчик сигналов
	//PStart->closeWin->Tag - управляет прерыванием сбора и функцией кнопки останова-закрытия
	rmGrafik = NULL;//график для рисования
	rmGrafikM = NULL;//график для рисования спонтанных сигналов (в режиме №4)
}
//---------------------------------------------------------------------------

void TRecordThread::AssignVar(short eTyp, IADCUtility *preUtil, IADCDevice  *preADC)
{
	__int32 i, k;

	rTypeOfExp = eTyp;//тип эксперимента
	r_pUtil = preUtil;//ссылка на интерфейс
	r_pADC = preADC;//ссылка на интерфейс
	r_a = Experiment->a;//структура параметров устройства

	rStimPrd = StrToInt(Experiment->StimPeriod->Text);//период стимуляции (миллисекунды)
	rBlockSize = Experiment->dmaBlockSize;//размер блока данных
	chanls = Experiment->a.m_nChannelNumber;//количество сканируемых каналов
	discrT = StrToFloat(Experiment->DiscreTime->Text);//время дискретизации (микросекунды)
	effDT = discrT * (float)chanls;//эффективное время дискретизации
	
	if (eTyp == 1)//спонтанные сигналы
		recLen = (__int32)floor((float(StrToInt(PStart->LenSpont->Text) * 1000) / effDT) + 0.5);//длина развёртки сигнала (отсчёты)
	else//остальные
		recLen = (__int32)floor((float(StrToInt(PStart->SignalLen->Text) * 1000) / effDT) + 0.5);//длина развёртки сигнала (отсчёты)

	multiCh = Experiment->RecMode->Checked;//режим сбора данных (true = многоканальный)
	ftChan = 1 + (chanls - 1) * (__int32)multiCh;//количество каналов с полными развёртками (full trace channels)
	sampl2mV = Experiment->maxVoltage / float(Experiment->maxADCAmp * r_a.m_nGain);//коэффициент перевода амплитуд из отсчётов АЦП в (милли)вольты
	eds = Experiment->eds;//маска для выделения кода цифрового порта (0xF = 15(dec) = 0000000000001111(bin))
	bwSh = Experiment->bwSh;//величина побитового сдвига при выделении кода АЦП

	//копирование атрибутов сигналов (для использования в модулях Graphs)
	Graphs->multiCh = multiCh;//режим сбора данных (true = многоканальный)
	Graphs->recLen = recLen;//длина развёртки сигнала (отсчёты)
	Graphs->discrT = discrT;//время дискретизации (микросекунды)
	Graphs->chanls = chanls;//количество сканируемых каналов
	Graphs->ftChan = ftChan;//количество каналов с полными развёртками (full trace channels)
	Graphs->effDT = effDT;//эффективное время дискретизации = discrT * chanls
	Graphs->sampl2mV = sampl2mV;//коэффициент перевода амплитуд из отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
	Graphs->gRF[0] = 0;//номер первого отображаемого канала
	Graphs->gRF[1] = ftChan;//номер последнего отображаемого канала + 1

	rConstMinus = (short)PStart->NulMinus->Checked;//вычитание постоянной составляющей
	rPolarity = -1 + (2 * __int32(!PStart->Invert->Checked));//полярность сигнала

	k = 0;//задаём порядок чтения каналов (циклический перебор)
	for (i = Experiment->uiLeadChan->ItemIndex; i < chanls; i++)
	{
		rReadOrder[k] = i;
		k++;
	}
	for (i = 0; i < Experiment->uiLeadChan->ItemIndex; i++)
	{
		rReadOrder[k] = i;
		k++;
	}
	//выставим множители для "быстрых" графиков
	for (i = 0; i < maxChannels; i++)
	{
		Graphs->curntSigChannls[i]->XValues->Multiplier = double(effDT) / 1000;
		PStart->exmplChannls[i]->XValues->Multiplier = double(effDT) / 1000;
	}
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::RecSynchroPlotExampl(void)
{
	//вызов прорисовки эталонного сигнала

	//PlotStandardSignal(double *sData, __int32 sRecLen, __int32 sEffDT, __int32 sIndBgn)
	PStart->PlotStandardSignal(rmGrafik, recLen, effDT, (rTimeBgn / effDT));//рисование эталонного сигнала
	PStart->RefreshLines(effDT);//обновление ПРЕ- ПОСТ-линий
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::RecSynchroCounter(void)
{
	//выводим номер текущего сигнала
	Graphs->CrntSig->Text = IntToStr(Graphs->CrntSig->Tag);//счётчик сигналов
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::MiniSynchroPlotGraphs(void)
{
	//рисование спонтанных сигналов
	__int32 j,
			*mspIndex = NULL;//массив индексов

	//текущий сигнал
	Graphs->PlotCurrentSignal(rmGrafik, 0);

	//создадим массив индексов
	//specK1 = k - kRefresh;//номер сигнала, с которого начинаются необработанные сигналы
	//specK2 = kRefresh;//количество неотрисованных сигналов
	mspIndex = new __int32[specK2];
	for (j = 0; j < specK2; j++)
		mspIndex[j] = j + specK1;

	//рассчитаем параметры сигналов
	Calculate(rSignals, specK2, rPorog, rPreTime, rPostTime, mspIndex, rTypeOfExp, false, NULL);
	Graphs->AddParamsPoints(rSignals, specK2, mspIndex, rTypeOfExp);//вывод параметров, добавляем точки на графиках

	delete[] mspIndex; mspIndex = NULL;
	Gists->GistsRefresh(-1);//перерисовываем гистограммы
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::InducSynchroPlotter(void)
{
	//рисование вызванных сигналов
	__int32 j,
			*ispIndex,//массив индексов
			shft1,//номер первого элемента для ampls и peakInds
			shft2;//номер первого элемента для spans

	ispIndex = NULL;
	//рисование текущего сигнала
	if (rmGrafik && (specK2 == 1))
		Graphs->PlotCurrentSignal(rmGrafik, (rTimeBgn / effDT));

	//создадим массив индексов
	//specK1 - номер сигнала, с которого начинаются необработанные сигналы
	//specK2 - количество неотрисованных сигналов
	ispIndex = new __int32[specK2];

	for (j = 0; j < specK2; j++)//добавление нескольких точек сразу
		ispIndex[j] = j + specK1;// * (__int32)(!rAppEndData);

	//рассчитаем параметры
	if (PStart->GetASignal->Tag != 1)//если идёт запись, то рисуем графики
	{
		Calculate(rSignals, specK2, rPorog, rPreTime, rPostTime, ispIndex, rTypeOfExp, false, NULL);

		shft1 = (3 * ftChan * ispIndex[specK2 - 1]) + (3 * 0);//номер первого элемента для ampls и peakInds
		shft2 = (5 * ftChan * ispIndex[specK2 - 1]) + (5 * 0);//номер первого элемента для spans
		Graphs->AddParamsPoints(rSignals, specK2, ispIndex, rTypeOfExp);//добавим точки на графики

		j = Graphs->sigAmpls[Graphs->gRF[0]]->YValues->Count() - 1;//номер последней добавленной на график точки
		Graphs->HighLightCrnt(&rSignals[ispIndex[specK2 - 1]], shft1, shft2, rTypeOfExp, (rTimeBgn / effDT), j);//подсвечивание
	}

	delete[] ispIndex; ispIndex = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::MiniMd4SynchroPlot(void)
{
	//рисование спонтанных сигналов в режиме №4
	Graphs->PlotCurrentSignal(rmGrafikM, 0);//рисование текущего сигнала
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::FromMinorChan(void)
{
	//вывод данных со второстепенных каналов
	__int32 i, j;

	if ((chanls > 1) && !multiCh)//количество каналов больше одного и не многоканальный режим
	{
		j = (Graphs->CrntSig->Tag - 1);//номер сигнала
		for (i = 0; i < (chanls - 1); i++)
			SecondChan->MChann[i]->Text = IntToStr((__int32)floor(rSignals[j].s[recLen + i] * sampl2mV));
	}

}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::CallForClearMemor(void)
{
	//защищённое удаление данных из памяти (используется для режима №4)
	Graphs->ClearMemor(this);//удаление всех старых данных из памяти
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::CallForReplotGraphs(void)
{
	//защищённый вызов функций прорисовк после сохранения
	Graphs->ResetVisibility();//запускаем прорисовку
	Graphs->FillParamTable();//заполняем таблицу значениями параметров
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::Execute()
{
	//сбор данных (запись пробного сигнала или запуск сценария)
	__int32 i,
			*index1,//указатель на индексы для Calculate
			pp,//количество пунктов протокола стимуляции (количество инструкций)
			totNumOfSpontS,//максимальное число спонтанных сигналов
			realNumOfSpontS,//количество собранных спонтанных сигналов
			maxStimPrd,//максимальный период стимуляции в протоколе (для режима №4)
			inducRecLen;//копия длины вызванного сигнала
	TDateTime CurrentDateTime;//текущие дата и время
	AnsiString memoFlNm,//копия имени файла
			   extendFlNm;//расширенное имя файл (для режима №4)
	SPPanel *panel;
	sPr *evkPrm;//копия указателя на структуру с параметрами вызванных сигналов

	while (!Terminated)
	{
		//убираем кнопки на время эксперимента
		PStart->GetASignal->Enabled = false;//пример сигнала
		PStart->StartRec->Enabled = false;//старт
		PStart->Invert->Enabled = false;//инверсия сигнала
		PStart->NulMinus->Enabled = false;//вычет ноль-линии
		PStart->SignalLen->Enabled = false;//длина сигнала
		PStart->LenSpont->Enabled = false;//длина спонтанного сигнала
		PStart->BackTime->Enabled = false;//время назад для спонтанного сигнала

		PStart->CloseWin->Caption = "Стоп";//меняем "функцию" кнопки
		PStart->CloseWin->Tag = 0;//меняем функцию кнопки
		Graphs->SigNumUpDwn->Enabled = false;//также нельзя переключать сигналы
		Graphs->gphOpen->Enabled = false;//запрет на открытие файлов во вермя записи
		Experiment->beginFOpen->Enabled = false;//запрет на открытие файлов во вермя записи
		Experiment->verticalScal->Enabled = false;//запрет на изменение масштаба вертикальной шкалы
		Experiment->timScal->Enabled = false;//запрет на изменение масштаба горизонтальной шкалы
		Experiment->DiscreTime->Enabled = false;//время дискретизации нельзя менять во время записи
		Graphs->SignalTrack->OnMouseDown = NULL;//запрещаем реагировать на клики (meanSignalMouseDown)

		//определим сколько сигналов нужно собрать
		if (PStart->GetASignal->Tag == 1)//пример сигнала (делаем протокол по умолчанию)
		{
			pp = 1;//количество пунктов протокола стимуляции (количество инструкций)
			rInstr = new __int32[3 * pp];//массив для инструкций (протокол стимуляции)
			rTotNumOfS = 1;//один пробный импульс
			rInstr[0] = 1;//стимуляция
			rInstr[1] = rTotNumOfS;//один пробный импульс
			rInstr[2] = rStimPrd;
		}
		else
		{
			if (ProtoBuild->InstructBox->ComponentCount <= 0)//протокол не задан
			{
				pp = 1;//количество пунктов протокола стимуляции (количество инструкций)
				rInstr = new __int32[3 * pp];//массив для инструкций (протокол стимуляции)
				rTotNumOfS = StrToInt(Experiment->NumOfsignals->Text);
				rInstr[0] = 1;
				rInstr[1] = rTotNumOfS;
				rInstr[2] = rStimPrd;
			}
			else//если задан протокол
			{
				pp = ProtoBuild->InstructBox->ComponentCount;//количество пунктов протокола стимуляции (количество инструкций)
				rInstr = new __int32[3 * pp];//массив для инструкций (протокол стимуляции)
				rTotNumOfS = 0;//пролистаем протокол стимуляции и подсчитаем общее количество сигналов
				for (i = 0; i < pp; i++)
				{
					panel = (SPPanel*)ProtoBuild->InstructBox->Components[i];
					rInstr[(i * 3) + 0] = panel->iType;//тип инструкции
					rInstr[(i * 3) + 1] = panel->scCount;//количество импульсов или повторений в цикле
					rInstr[(i * 3) + 2] = panel->period;//период следования импульсов или длительность паузы

					if (rInstr[(i * 3) + 0] == 1)//если стимул, считаем сколько импульсов
						rTotNumOfS += rInstr[(i * 3) + 1];
					if (rInstr[(i * 3) + 0] == 3)//если цикл, умножаем на кол-во проходов
						rTotNumOfS = rTotNumOfS * rInstr[(i * 3) + 1];
				}
			}
		}

		//определим заранее параметр рисования rTimeBgn
		rPorog = short((float)StrToInt(PStart->Porog->Text) / sampl2mV);//пороговое напряжение (отсчёты)
		rPreTime = StrToInt(PStart->PreTime->Text);
		rPostTime = StrToInt(PStart->PostTime->Text);

		//выбираем индекс, с которого смотрим (рисуем) сигнал (после синхоимпульса)
		rTimeBgn = 0;
		if (rTypeOfExp == 3)//вызванные - внеклеточные
			rTimeBgn = rPreTime;
		else if (rTypeOfExp == 2)// (внутриклеточные) или rTypeOfExp = 4 (смешанные)
			rTimeBgn = rPostTime;

		PStart->timeOfDrawBgn = (float)rTimeBgn;//передаём значение индекса-начала в модуль PStart

		//выбран ли режим прерывистого сбора (с возможностью дозаписи)
		//если получаем пример сигнала, то действуем традиционным образом
		rAppEndData = ((Experiment->DiscontinWrt->Checked) && (PStart->GetASignal->Tag != 1));//непрерывная запись (не пример сигнала)

		if (rAppEndData)//способ записи данных
			CreatExtendFile();//создать или продолжить файл в прерывистом режиме

		if ((rTypeOfExp != 4) || (PStart->GetASignal->Tag == 1))//любой, кроме вызванные+спонтанные
		{
			rSignals = Graphs->CreatStructSignal(rTotNumOfS, recLen);//структура с сигналами и их атрибутами
			totNumOfSpontS = 0;//количество спонтанных сигналов в режиме №4
		}
		else// if (rTypeOfExp == 4)//одновременный сбор вызванных и спонтанных; в rSignals записываем и вызванные и спонтанные
		{
			//создадим дополнительную структуру для хранения спонтанных сигналов
			if (ProtoBuild->InstructBox->ComponentCount > 0)
			{
				maxStimPrd = 0;//наибольший интервал между импульсами стимуляции для расчёта totNumOfSpontS
				for (i = 0; i < pp; i++)
					if ((rInstr[(i * 3) + 0] == 1) && (rInstr[(i * 3) + 2] > maxStimPrd))
						maxStimPrd = rInstr[(i * 3) + 2];//запоминаем наибольший интервал
			}
			else
				maxStimPrd = StrToInt(Experiment->StimPeriod->Text);//наибольший интервал между импульсами стимуляции

			inducRecLen = Graphs->recLen;//копия длины вызванного сигнала
			rSpntRecLen = floor((float(StrToFloat(PStart->LenSpont->Text) * 1000) / effDT) + 0.5);//длина спонтанного сигнала в режиме №4 (отсчёты)
			rmGrafikM = new double[rSpntRecLen * ftChan];//график для рисования спонтанных сигналов (в режиме №4)

			//totNumOfSpontS - максимально возможное количество спонтанных сигналов в режиме №4
			totNumOfSpontS = rTotNumOfS * (floor(0.5 * maxStimPrd / StrToInt(PStart->LenSpont->Text)) + 1);//(предельное) количество спонтанных сигналов
			if (totNumOfSpontS > limitSigNum)//ограничение на количество спонтанных сигналов, собираемых в режиме №4
				totNumOfSpontS = limitSigNum;//указываем предельное количество спонтанных сигналов
			rSignals = Graphs->CreatStructSignal(rTotNumOfS, recLen);//структура с вызванными сигналами
			evkPrm = Graphs->cPrm;//копия указателя на структуру с параметрами вызванных сигналов
			rSpntSignals = Graphs->CreatStructSignal(totNumOfSpontS, rSpntRecLen);//структура со спонтанными сигналами
		}

		rmGrafik = new double[recLen * ftChan];//график для рисования
		GetLocalMean();//вычисляем начальное среднее

		//--------------------------------------------------------------------
		//--- запуск одного из вариантов сбора данных ------------------------
		StartRec(rTotNumOfS, totNumOfSpontS, pp);//запускаем сценарий
		//--------------------------------------------------------------------

		//после сбора данных: сохранение, вывод на экран и т.д.
		rRealNumOfS = Graphs->CrntSig->Tag;//количество действительно записанных вызванных сигналов
		realNumOfSpontS = PStart->BackTime->Tag;//количество действительно записанных спонтанных сигналов
		Graphs->SigNumUpDwn->Tag = rTotNumOfS;//копируем исходное число сигналов (понадобится при удалении)

		if (rRealNumOfS >= 1)//если сигналы имеются
		{
			if (PStart->GetASignal->Tag == 1)//если выбрана запись пробного сигнала
			{
				index1 = new __int32[1];
				index1[0] = 0;
				//рассчитаем параметры и покажем их
				//Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
				//			__int32 *inds, short expT, bool newNull, sPr *avrP)
				Calculate(rSignals, rRealNumOfS, rPorog, rPreTime, rPostTime, index1, rTypeOfExp, false, NULL);
				delete[] index1; index1 = NULL;
				Synchronize(&RecSynchroPlotExampl);//вывод сигнала в окно предстарта//построение примера
			}
			else if (!rAppEndData)//сохранение эксперимента PStart->getASignal->Tag = 2
			{
				if (rTypeOfExp == 4)//вызванные + спонтанные
				{
					memoFlNm = Graphs->SaveDlg->FileName;//резервная копия текущего имени
					//сначала сохраняем спонтанные сигналы
					if (realNumOfSpontS >= 1)//есть спонтанные сигналы
					{
						extendFlNm = memoFlNm;//вновь записываем исходное имя файла
						extendFlNm.Insert("_Spont", extendFlNm.Length() - 3);//подправляем имя файла, чтобы сохранить спонтанныесигналы
						Graphs->SaveDlg->FileName = extendFlNm;//окончательное имя файла со спонтанными сигналами

						Graphs->SigNumUpDwn->Tag = totNumOfSpontS;//копируем исходное число сигналов (понадобится при удалении)
						Graphs->recLen = rSpntRecLen;//временно указываем длину спонтанного сигнала
						i = (__int32)Graphs->SaveExpDataToFile(5, rSpntSignals, realNumOfSpontS, true);//тип эксперимента
						Graphs->recLen = inducRecLen;//вновь указываем длину вызванного сигнала
					}
					else//нет спонтанных сигналов
						Experiment->DevEvents->Text = "нет спонтанных сигналов";//сигналов не обнаружено
					Graphs->DeleteStructSignal(rSpntSignals, totNumOfSpontS);//освобождаем память, если нет сохранения
					rSpntSignals = NULL;//структура удалена в модуле Graphs->ClearMemor

					//сохраняем отдельно вызванные сигналы (считаем их внутриклеточными, №2)
					extendFlNm = memoFlNm;//резервная копия текущего имени
					extendFlNm.Insert("_Induced", extendFlNm.Length() - 3);//подправляем имя файла, чтобы сохранить вызванные сигналы
					Graphs->SaveDlg->FileName = extendFlNm;//окончательное имя файла с вызванными сигналами
					Graphs->cPrm = evkPrm;//копия указателя на структуру с параметрами вызванных сигналов
					Graphs->SigNumUpDwn->Tag = rTotNumOfS;//копируем исходное число сигналов (понадобится при удалении)
					if (Graphs->SaveExpDataToFile(2, rSignals, rRealNumOfS, true))//сохранение прошло нормально
						Synchronize(&CallForReplotGraphs);//защищённая прорисовка либо вызванных либо спонтанных сигналов

					Graphs->SaveDlg->FileName = memoFlNm;//восстанавливаем имя файла
				}
				else
					if (Graphs->SaveExpDataToFile(rTypeOfExp, rSignals, rRealNumOfS, true))//сохранение данных
						Synchronize(&CallForReplotGraphs);//защищённая прорисовка сигналов
			}
		}
		else
			Experiment->DevEvents->Text = "нет сигналов";

		//удаление структуры с сигналами в случаях, когда нет стандартного (pra) сохранения данных
		if ((rRealNumOfS < 1) || (PStart->GetASignal->Tag == 1) || rAppEndData)//нет сигналов или выбран пример сигнала или непрерывная запись
		{
			Graphs->DeleteStructSignal(rSignals, rTotNumOfS);//освобождаем память, если нет сохранения
		}

		rSignals = NULL;//обнуляем указатель
		delete[] rInstr; rInstr = NULL;//удалем массив с инструкциями
		if (rAppEndData)//прерывистая запись
			CompleteDisconFile();//закрытие файла прерывистой записи
		delete[] rmGrafik; rmGrafik = NULL;//удаление графика для рисования
		if (rmGrafikM)
		{
			delete[] rmGrafikM; rmGrafikM = NULL;//удаление графика для рисования спонтанных сигналов (в режиме №4)
        }
		delete[] rExtractLocMean; rExtractLocMean = NULL;//удаление массива с локальным средним

		//восстанавливаем кнопки
		PStart->GetASignal->Enabled = true;//кнопк апример сигнала
		PStart->StartRec->Enabled = true;//кнопка старта
		PStart->Invert->Enabled = true;//кнопка инверсии сигнала
		PStart->NulMinus->Enabled = true;//вычет ноль-линии
		PStart->SignalLen->Enabled = true;//длина сигнала
		PStart->LenSpont->Enabled = true;//длина спонтанного сигнала
		PStart->BackTime->Enabled = true;//время назад для спонтанного сигнала

		PStart->CloseWin->Caption = "Закрыть";//меняем "функцию" кнопки
		PStart->CloseWin->Tag = 1;//меняем функцию кнопки

		ExpNotes->addUMark->Enabled = true;//разрешаем добавление коментариев (окно ввода текста)

		Graphs->gphOpen->Enabled = true;//разрешаем открывать файлы
		Experiment->beginFOpen->Enabled = true;//разрешаем открывать файлы
		Experiment->verticalScal->Enabled = true;//запрет на изменение масштаба вертикальной шкалы
		Experiment->timScal->Enabled = true;//запрет на изменение масштаба горизонтальной шкалы
		Experiment->DiscreTime->Enabled = true;//время дискретизации снова можно менять
		Graphs->SignalTrack->OnMouseDown = Graphs->SignalTrackMouseDown;//разрешаем реагировать на клики

		Suspend();//ставим поток на паузу
	}
}
//---------------------------------------------------------------------------

void TRecordThread::CreatExtendFile()
{
	//создать или продолжить файл в прерывистом режиме
	__int32 itemWrtRd,//количество прочитанных единиц (не байт)
			amountOfBytes,//количество байт, записываемых в данном блоке
			backTimeMini;//время назад для спонтанных сигналов
	TDateTime CurrentDateTime;//текущие дата и время
	AnsiString progDate;//информация о программе и дате создания файла (для прерывистого режима)
	char lett[3];//символ-маркер (терминальная запись в файл; прерывистый режим)

	CurrentDateTime = Now();//текущие дата и время

	//создаём-открываем файл для чтения-записи
	//rDiscontinFile = CreateFile(Graphs->SaveDlg->FileName.c_str(), FILE_ALL_ACCESS, 0, 0, OPEN_ALWAYS, 0, 0);
	if ((FileExists(Graphs->SaveDlg->FileName)) && (Experiment->DiscontinWrt->Tag == 1)) //существует ли файл
	{
		progDate = "\nED:";//разделитель между сеансами сбора данных (дозапись в существующий файл)
		progDate += CurrentDateTime.DateTimeString().c_str();
		rDiscontinFile = fopen(Graphs->SaveDlg->FileName.c_str(), "ab+");//открываем для чтения и дозаписи
		itemWrtRd = fseek(rDiscontinFile, 0, SEEK_END);//идём в конец файла
	}
	else
	{
		//создадим заголовок файла и впишем его
		progDate = "ElphAcqu v" + progVer + "\nmade";//~18 символов
		progDate += CurrentDateTime.DateTimeString().c_str();//19 символов
		progDate += "\nExpandable file";

		rDiscontinFile = fopen(Graphs->SaveDlg->FileName.c_str(), "wb");//открываем для записи
	}
	amountOfBytes = sizeof(char) * progDate.Length();//количество байт, записываемых в данном блоке
	amountOfBytes += sizeof(bool) + (3 * sizeof(char)) + (5 * sizeof(__int32)) + sizeof(short) + sizeof(float);

	//вписываем заголовок или разделитель
	itemWrtRd = fwrite(progDate.c_str(), sizeof(char), progDate.Length(), rDiscontinFile);
	amountOfBytes -= itemWrtRd * sizeof(char);

	lett[0] = 'P'; lett[1] = 'r'; lett[2] = 'M';//указывает на начало блока параметров данного сеанса
	itemWrtRd = fwrite(&lett, sizeof(char), 3, rDiscontinFile);//вписываем маркер
	amountOfBytes -= itemWrtRd * sizeof(char);

	//переменные, уникальные для данного сеанса (т.е. все необходимые параметры)
	/* последовательность записи обязательных параметров
		multiCh -------(1) - режим сбора данных (true = многоканальный)
		recLen --------(2) - длина развёртки сигнала (отсчёты)
		discrT --------(3) - время дискретизации
		chanls --------(4) - количество сканируемых каналов
		E->adcGain ----(5) - коэффициент усиления
		experimentType (6) - тип эксперимента, глобальный вариант
		E->maxVoltage -(7) - диапазон напряжений
		E->maxADCAmp --(8) - максимальная амплитуда (отсчёты)
	*/

	itemWrtRd = fwrite(&multiCh, sizeof(bool), 1, rDiscontinFile);//1//режим сбора данных (true = многоканальный)
	amountOfBytes -= itemWrtRd * sizeof(bool);
	itemWrtRd = fwrite(&recLen, sizeof(__int32), 1, rDiscontinFile);//2//длина развёртки сигнала в отсчётах
	amountOfBytes -= itemWrtRd * sizeof(__int32);
	itemWrtRd = fwrite(&discrT, sizeof(float), 1, rDiscontinFile);//3//время дискретизации
	amountOfBytes -= itemWrtRd * sizeof(__int32);
	itemWrtRd = fwrite(&chanls, sizeof(__int32), 1, rDiscontinFile);//4//количество сканируемых каналов
	amountOfBytes -= itemWrtRd * sizeof(__int32);
	itemWrtRd = fwrite(&r_a.m_nGain, sizeof(__int32), 1, rDiscontinFile);//5//коэффициент усиления
	amountOfBytes -= itemWrtRd * sizeof(__int32);
	itemWrtRd = fwrite(&rTypeOfExp, sizeof(short), 1, rDiscontinFile);//6//тип эксперимента, глобальный вариант
	amountOfBytes -= itemWrtRd * sizeof(short);
	itemWrtRd = fwrite(&Experiment->maxVoltage, sizeof(float), 1, rDiscontinFile);//7//диапазон напряжений
	amountOfBytes -= itemWrtRd * sizeof(float);
	itemWrtRd = fwrite(&Experiment->maxADCAmp, sizeof(__int32), 1, rDiscontinFile);//8//максимальная амплитуда (в отсчётах)
	amountOfBytes -= itemWrtRd * sizeof(__int32);

	if (amountOfBytes != 0)
		Experiment->DevEvents->Text = "ошибка начальной записи";//аварийный выход, видимо, можно не делать
}
//---------------------------------------------------------------------------

void TRecordThread::CompleteDisconFile()
{
	//завершить файл в прерывистом режиме
	__int32 i, j, z,
			*pMarkNums,//номера точек с метками, которые нужно сохранить
			lettersNum,//длина текстовой записи
			amountOfBytes,//количество байт, записываемых в данном блоке
			itemWrtRd,//количество прочитанных единиц (не байт)
			comUserMarks;//общее количество меток пользователя
	AnsiString userText;//информация об эксперименте (вводит пользователь)
	char lett[3];//символ-маркер (терминальная запись в файл; прерывистый режим)
	bool *markerVsblt;//видимость меток на каналах

	//вписываем заметки по ходу эксперимента, если такие были
	/* последовательность записи в блоке с текстовыми данных
		lettersNum -(1) - длина текстовой записи
		comUserMakrs(2) - количество заметок по ходу эксперимента
		userText ---(3) - текст об эксперименте и заметке по ходу
		pointNums --(4) - массив с номерами точек с заметками
	*/

	//информация об эксперименте (вводит пользователь);
	userText = ExpNotes->usersNotes->Lines->Text.c_str();
	comUserMarks = (ExpNotes->addUMark->Tag) - (ExpNotes->nmInRec);//количество меток, введённых в данном сеансе збора данных

	if (!userText.IsEmpty() || (comUserMarks > 0))//пользователь вводил текстовую информацию
	{
		//вставляем в переменную userText заметки по ходу эксперимента
		if (comUserMarks > 0)//пользователь вводил метки по ходу эксперимента
		{
			pMarkNums = new __int32[comUserMarks];//номера точек с метками, которые нужно сохранить
			markerVsblt = new bool[comUserMarks * ftChan];//видимость меток на каналах

			if (userText.IsEmpty())
				userText = "\n\n\r\r\n\n";//разделитель
			else
				userText += "\n\n\r\r\n\n";//разделитель

			for (i = 0; i < ExpNotes->addUMark->Tag; i++)//
			{
				if (ExpNotes->theMarker->pointOnGraph >= ExpNotes->npNewRec)//метка создана в данном сеансе сбора данных
				{
					userText += ExpNotes->theMarker->textMark;
					userText += "||\n||";
				}
				ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
			}
		}
		lettersNum = userText.Length();//длина текстовой записи
		amountOfBytes = (userText.Length() * sizeof(char)) + (3 * sizeof(char)) + //текстовая часть
						(2 * sizeof(__int32)) + //количество символов и меток
						((comUserMarks * __int32(comUserMarks > 0)) * sizeof(__int32)) + //номера сигналов с метками
						((comUserMarks * ftChan * __int32(comUserMarks > 0)) * sizeof(bool));//видимость меток на каналах

		lett[0] = 'M'; lett[1] = 'r'; lett[2] = 'k';//указывает на начало пользовательской информации
		itemWrtRd = fwrite(&lett, sizeof(char), 3, rDiscontinFile);//вписываем разделитель (Mrk)
		amountOfBytes -= itemWrtRd * sizeof(char);

		itemWrtRd = fwrite(&lettersNum, sizeof(__int32), 1, rDiscontinFile);//1//длина текстовой записи
		amountOfBytes -= itemWrtRd * sizeof(__int32);

		itemWrtRd = fwrite(&comUserMarks, sizeof(__int32), 1, rDiscontinFile);//2//количество заметок
		amountOfBytes -= itemWrtRd * sizeof(__int32);

		/* последовательность записи блоков в pra-файле:
			refToWrite[29] = userText.c_str();//ссылка на блок №1
			refToWrite[30] = (__int32*)(ExpNotes->pointNums);//ссылка на блок №16	*/

		//сначала записываем сам текст, потом номера точек с метками
		itemWrtRd = fwrite(userText.c_str(), sizeof(char), userText.Length(), rDiscontinFile);//вписываем заметки пользователя
		amountOfBytes -= itemWrtRd * sizeof(char);

		//затем номера точек на графике с метками
		j = 0;//индекс в массиве pMarkNums
		for (i = 0; i < ExpNotes->addUMark->Tag; i++)//comUserMarks > 0
		{
			if (ExpNotes->theMarker->pointOnGraph >= ExpNotes->npNewRec)//метка создана в данном сеансе сбора данных
			{
				pMarkNums[j] = ExpNotes->theMarker->pointOnGraph - ExpNotes->npNewRec;
				for (z = 0; z < ftChan; z++)
					markerVsblt[(j * ftChan) + z] = ExpNotes->theMarker->chanN[z];//видимость меток на каналах
				j++;//увеличиваем индекс в массиве pMarkNums
			}
			ExpNotes->theMarker = ExpNotes->theMarker->nextM;//переходим к следующей метке
		}

		if (comUserMarks > 0)//пользователь вводил метки по ходу эксперимента
		{
			itemWrtRd = fwrite(pMarkNums, sizeof(__int32), comUserMarks, rDiscontinFile);//вписываем номера сигналов с метками
			amountOfBytes -= itemWrtRd * sizeof(__int32);
			itemWrtRd = fwrite(markerVsblt, sizeof(bool), (comUserMarks * ftChan), rDiscontinFile);//вписываем видимость меток на каналах
			amountOfBytes -= itemWrtRd * sizeof(bool);

			delete[] pMarkNums; pMarkNums = NULL;
			delete[] markerVsblt; markerVsblt = NULL;
		}

		if (amountOfBytes != 0)
			Experiment->DevEvents->Text = "ошибка конечной записи";
	}

	fclose(rDiscontinFile);//закрываем файл, в который сохраняли в прерывистом режиме
	rDiscontinFile = NULL;
}
//---------------------------------------------------------------------------

void TRecordThread::StartRec(__int32 allExcit, __int32 allSpont, __int32 pp)
{
	//запуск сценария стимуляции
	/*
	allExcit - общее количество записываемых сигналов (в режимах №1, 2, 3)
	allSpont - количество собираемых спонтанных сигналов в режиме №4
	pp - количество пунктов протокола стимуляции (количество инструкций)
	*/

	__int32 i, z, g,
            errC,//индикатора зависания
			s_cycls,//количество проходов по циклу в сценарии
			blockReady,//готовность блока данных
			notZero,//количество ненулей
			numOfS,//число сигналов
			startNum,//начало нумерации сигналов
			stimPrd;//период стимуляции
	__int64 curnttic, ticps;//нужно для слежки за временем
	bool canPlot;//разрешено ли рисовать (большое "свободное" время)
	clock_t experimBegin;//момент старта сценария (секунд от начала работы программы)
	unsigned short cH, cM, cS, cmS,//текущее время
				   tH, tM, tS;//целевое время

	//запускаем сценарий здесь
	s_cycls = 1;//счётчик циклов (повторение сценария)
	Graphs->CrntSig->Tag = 0;//нумерация (сквозная) начинается с нуля
	PStart->BackTime->Tag = 0;//нумерация для спонтанных сигналов также сквозная

	if ((PStart->GetASignal->Tag != 1) && (ProtoBuild->StarTim->Checked))//не пример сигнала и ждать момента старта
	{
		tH = (unsigned short)StrToInt(ProtoBuild->HEdit->Text);//часы
		tM = (unsigned short)StrToInt(ProtoBuild->MEdit->Text);//минуты
		tS = (unsigned short)StrToInt(ProtoBuild->SEdit->Text);//секунды
		Now().DecodeTime(&cH, &cM, &cS, &cmS);
		while (((tH > cH) || (tM > cM) || (tS > cS)) &&
			   (PStart->CloseWin->Tag != 1) && (PStart->NextBlock->Tag != 1))
		{
			Now().DecodeTime(&cH, &cM, &cS, &cmS);
			Sleep(100);//ждём десятую долу секунды
		}
	}

	z = 0;//индикатора зависания
	while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (z <= 2))//частота процессора
		z++;
	if (z > 2)
	{   Experiment->DevEvents->Text = "не определена частота процессора"; return; }

	//rBeginTakt - такт процессора, при котором начинается вополнение сценария
	z = 0;//индикатора зависания
	while((!QueryPerformanceCounter((LARGE_INTEGER *)&rBeginTakt)) && (z <= 2))
		z++;
	if (z > 2)
	{   Experiment->DevEvents->Text = "ошибка QPcounter"; return; }

	experimBegin = clock() / 1000;//момент старта сценария (секунд от начала работы программы)
	QueryPerformanceCounter((LARGE_INTEGER *)&rNextTic);//момент старта следующего сбора (сразу совпадает с текущим временем)

	specK1 = Graphs->CrntSig->Tag;//=0//номер сигнала, с которого начинаются неотрисованные сигналы
	for (i = 0; (i < pp) && (PStart->CloseWin->Tag == 0); i++)//цикл перебора инструкций
	{
		if (rInstr[(i * 3) + 0] == 1)//пункт протокола стимуляции
		{
			numOfS = rInstr[(i * 3) + 1];//количество импульсов стимуляции (сигналов)
			stimPrd = rInstr[(i * 3) + 2];//период подачи импульсов стимуляции (миллисекунды)
			canPlot = bool((stimPrd - __int32(float(recLen * effDT) / 1000)) >= minFreeTime);//разрешено ли рисовать (большое "свободное" время)

			PStart->NextBlock->Tag = 0;//если устанавливается 1, то выходим из блока

			//запускаем стимуляцию и сбор данных в зависимости от типа эксперимента
			if (rTypeOfExp == 1)//спонтанные
				Spnt_SgnlAcquisition(numOfS, experimBegin);//спонтанные
			else if ((rTypeOfExp == 2) || (rTypeOfExp == 3))//вызванные
			{
				Induced_SgnlAcquisition(numOfS, stimPrd, canPlot);//только вызванные
				if (!canPlot && (PStart->GetASignal->Tag != 1))//рисовать нельзя и не пример сигнала
				{
					if (i < (pp - 1))//не последний блок
						if (rInstr[((i + 1) * 3) + 0] == 2)//следующий блок - пауза
							canPlot = bool((rInstr[((i + 1) * 3) + 2] - __int32(float(recLen * effDT) / 1000)) >= minFreeTime);
					if (canPlot)
					{
						specK2 = Graphs->CrntSig->Tag - specK1;//количество неотрисованных сигналов
						Synchronize(&InducSynchroPlotter);//добавляем точки на график
						specK1 = Graphs->CrntSig->Tag;//номер сигнала, с которого начинаются неотрисованные сигналы
					}
				}
			}
			else//(rTypeOfExp == 4)//вызванные + спонтанные
				Induced_N_Spnt_SgnlAcquis(allSpont, numOfS, stimPrd, canPlot);//вызванные + спонтанные

			startNum = Graphs->CrntSig->Tag;//продолжаем сквозную нумерацию
			QueryPerformanceCounter((LARGE_INTEGER *)&rNextTic);//момент (в тактах процессора) подачи следующего синхроимпульса
		}
		if (rInstr[(i * 3) + 0] == 2)//пауза (запускаем Sleep)
		{
			errC = 0;//индикатора зависания
			while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//частота процессора
				errC++;
			if (errC > 2)
			{   Experiment->DevEvents->Text = "не определена частота процессора"; return; }
			rNextTic = rBeginTakt + (__int64(rSignals[startNum - 1].appear) * ticps) +
					   (__int64(rInstr[(i * 3) + 2]) * (ticps / 1000));//момент (в тактах процессора) подачи следующего синхроимпульса

			//вариант с использованием Sleep(Х) до самого момента начала следующего сбора
			//if (rInstr[(i * 3) + 2] > 1500)//оставляем одну секунду для уточнения момента старта
			//	Sleep(floor((rInstr[(i * 3) + 2] - 1000) + 0.5));

			//вариант с циклом, внутри которого Sleep(50) (паузы точно длиннее, чем 5 мс)
			if (rInstr[(i * 3) + 2] > 1050)//если ждать больше одной секунды
			{
				while(!QueryPerformanceCounter((LARGE_INTEGER *)&curnttic)) {};
				while((curnttic < (rNextTic - ticps)) && (PStart->CloseWin->Tag == 0))//прекращаем паузу на одну секунду раньше (rNextTic - ticps)
				{
					Sleep(50);
					while(!QueryPerformanceCounter((LARGE_INTEGER *)&curnttic)) {};
				}
			}
		}
		if (rInstr[(i * 3) + 0] == 3)//цикл (повторение протокола сначала)
			if (s_cycls < rInstr[(i * 3) + 1])
			{
				i = -1;//идём на начало списка инструкций (пока так, но можно переходить на заданную инструкцию)
				s_cycls++;
			}
	}
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::Induced_SgnlAcquisition(__int32 iNumOfS, __int32 iStimPrd, bool canPlot)
{
	//сбор данных: вызванные, режимы №2 и №3
	/*
	iNumOfS - число собираемых в данном блоке сигналов
	iStimPrd - период стимуляции в данном блоке (миллисекунды)
	canPlot - разрешено ли рисовать (большое "свободное" время)
	*/

	bool isSynchro;//обнаружен ли синхроимпульс
	short *drvData,
		  convData[maxChannels];//содержит выделенный код АЦП для всех каналов (каналов не больше, чем maxChannels)
	unsigned short digitPort4;//значения старших четырёх битов входного цифрового порта (поиск синхроимпульсов)
	__int32 i, z, j,
			errC,//счётчик ошибок
			ik,//количество записанных сигналов
			kRefresh,//счётчик обновления (обновление гистограмм (для графиков по другому пока))
			refreshNum,//период обновления гистограмм
			startNum,//нумерация сигналов в сложном протоколе продолжается и начинается с startNum
			stims,//количество выданных синхроимпульсов
			recorded,//счётчик записанных отсчётов
			blockReady,//готовность блока данных
			minorChanMean[maxChannels],//среднее на второстепенном канале во время регистрируемого всплеска
			impInterv,//нижняя граница значений межимпульсных интервалов (в отсчётах)
			sampRead;//число записанных отсчётов, начиная с момента возникновения синхроимпульса
    float waitTime;//длительность паузы (мкс) перед отключением синхроимпульса
	unsigned int mask;//хранит информацию о наложении данных
	__int64 *appear,//массив с временами подачи синхроимпульсов (в тактах процессора)
			totaltic,//общее число тактов процессора, отводимое для текущего сбора (по истечении этого времени выходим из цикла)
			tpp,//тактов за перод синхроимпульса
			tp1,
			curnttic,//текущий такт процессора
			ticps,//тактов в секунду (частота процессора)
			ticperiod,//тактов процессора за период стимуляции
			popravka;//авторегулировка частоты стимуляции

    drvData = NULL;
	appear = NULL;

	refreshNum = iNumOfS + 1;//заведомо период обновления больше числа собираемых сигналов (т.е. нет обновления)
	if (Experiment->ggRefresh->Checked)
		refreshNum = StrToInt(Experiment->refreshEvery->Text);

	recorded = 0;//обнуляем счётчик записанных отсчётов
	curnttic = 0;
	stims = 0;//количество выданных синхроимпульсов
	appear = new __int64[iNumOfS];//массив с временами возникновения сигналов

	errC = 0;//индикатора зависания
	while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//частота процессора
		errC++;
	if (errC > 2)
	{   Experiment->DevEvents->Text = "ошибка QPFrequency"; return; }
	ticperiod = (ticps / (__int64)1000) * (__int64)iStimPrd;//тактов процессора за период стимуляции

	//рассчитаем минимальное число отсчётов между импульсами стимуляции (в отсчётах)
	impInterv = floor((1000 * iStimPrd) / (2 * effDT));//полуинтервал между импульсами стимуляции (отсчёты)
	sampRead = impInterv;//чтобы первый сигнал читался без проблем
	startNum = Graphs->CrntSig->Tag;//номер сигнала, с которого продолжается нумерация
	ik = startNum;//количество записанных сигналов (сквозная нумерация)
	kRefresh = 0;//счётчик обновления на ноль
	waitTime = min(float(500), discrT);//длительность паузы (мкс) перед отключением синхроимпульса
	tpp = (float)ticps * (waitTime / (float)1e6);//тактов за перод синхроимпульса

	z = r_pUtil->Start(&r_a, 0);//старт сбора данных
	if (z != 1)
	{
		Experiment->DevEvents->Text = "ошибка ADC_Start";
		return;
	}
	z = 0; errC = 0;//проверяем начался ли сбор данных
	while ((z == 0) && (errC <= 200))
	{
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
		errC++;
	}
	if ((errC > 200) && (z == 0))
	{   Experiment->DevEvents->Text = "нет старта"; return; }
	z = 0; errC = 0;//устанавливаем НОЛЬ на выходе ЦАП
	while ((z == 0) && (errC <= 2))
	{
		z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[1]);//устанавливаем НОЛЬ на выходе ЦАП
		Sleep(1);//ЦАП работает на частоте не более 5 кГц
		errC++;
	}
	if (errC > 2)
	{   Experiment->DevEvents->Text = "ошибка ADC_WRITE_DAC"; return; }

	for (i = 0; i < 2; i++)//"разминочный" сбор
	{
		blockReady = 0;
		while (blockReady == 0)
			blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

		z = 0; errC = 0;//освобождаем захваченный блок памяти (буфер)
		while ((z == 0) && (errC <= 2))
		{
			z = r_pUtil->FreeBuffer();
			errC++;
		}
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "err FreeBuf (IS r)";
			break;//аварийное завершение разминочного сбора
		}
	}

	QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);//текущий такт процессора
	if (curnttic > rNextTic)
		totaltic = curnttic + (iNumOfS * ticperiod) + (2 * ticps);//далее totaltic корректируется
	else
		totaltic = rNextTic + (iNumOfS * ticperiod) + (2 * ticps);//далее totaltic корректируется
    
	while (((ik < (startNum + iNumOfS)) || (recorded > 0)) && (curnttic <= totaltic))
	{
        //блок подачи синхроимпульсов
		//curnttic = DaiImpuls(&appear[stims], &iNumOfs, ik, startNum);//варинт функции подачи синхроимпульса
		errC = 0;//индикатора зависания
		while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//частота процессора
			errC++;
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "ошибка QPFrequency";
			iNumOfS = ik - startNum;//условие выхода из цикла
			recorded = 0;//условие выхода из цикла
		}
		ticperiod = (ticps / (__int64)1000) * (__int64)iStimPrd;//тактов процессора за период стимуляции

		errC = 0;//индикатора зависания (не даём циклу возможность работать бесконечно)
		while((!QueryPerformanceCounter((LARGE_INTEGER *)&curnttic)) && (errC <= 2))
			errC++;
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "ошибка QPCounter";
			iNumOfS = ik - startNum;//условие выхода из цикла
			recorded = 0;//условие выхода из цикла
		}

		popravka = rNextTic - curnttic;//следим за наступлением момента подачи синхроимпульса (расчёт поправок)
		if ((popravka <= 0) && (stims < iNumOfS))//вывод стимулирующего сигнала
		{
			//SetToDAC();
			z = 0; errC = 0;//устанавливаем НЕНОЛЬ на выходе ЦАП
			while ((z == 0) && (errC <= 2))
			{
				z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[0]);//устанавливаем НЕНОЛЬ на выходе ЦАП
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "ошибка ADC_WRITE_DAC";
				iNumOfS = ik - startNum;//условие выхода из цикла
				recorded = 0;//условие выхода из цикла
			}
			
			if (((popravka + ticperiod) < 0) || (stims == 0))
				rNextTic = curnttic + ticperiod;//момент (в тактах процессора) подачи следующего синхроимпульса
			else
				rNextTic = curnttic + ticperiod + popravka;//момент (в тактах процессора) подачи следующего синхроимпульса
			appear[stims] = curnttic;//запоминаем время возникновения сигнала
			stims++;//увеличиваем счётчик поданых синхроимпульсов

			//делаем пауза перед отключением синхроимпульса
			tp1 = curnttic;
			while ((curnttic - tp1) < tpp)
				QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);

			z = 0; errC = 0;//устанавливаем НОЛЬ на выходе ЦАП
			while ((z == 0) && (errC <= 2))
			{
				z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[1]);//устанавливаем НОЛЬ на выходе ЦАП
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "ошибка ADC_WRITE_DAC";
				iNumOfS = ik - startNum;//условие выхода из цикла
				recorded = 0;//условие выхода из цикла
			}
		}
		//===== окончание блока подачи синхроимпульсов =====

		blockReady = r_pUtil->GetBuffer((void*)drvData, mask);//запрашиваем блок данных из буфера DMA
		if (blockReady != 0)//блок данных получен
		{
			if ((PStart->CloseWin->Tag == 1) || (PStart->NextBlock->Tag == 1))//не пора ли остановиться?
				iNumOfS = ik - startNum;//преждевременный выход (из эксперимента или блока)

			for (i = 0; i < rBlockSize; i += chanls)//перебираем отсчёты блока данных
			{
				for (z = 0; z < chanls; z++)
					convData[z] = (drvData[i + rReadOrder[z]] >> bwSh);//выделяем код АЦП (старшие 12 (14) бит)

				sampRead++;//прирост счётчика прочитанных отсчётов
				//логическое "И" oDrvData[i] & 0000000000001111 - выделяем последние 4 бита
				//0xF = 15(dec) = 0000000000001111(bin); 0xFFF0 = 65520(dec) = 1111111111110000(bin)
				digitPort4 = ((unsigned short)drvData[i]) & eds;//(oDrvData[i] << 12)
				isSynchro = ((digitPort4 > 2) && (sampRead >= impInterv));//обнаружен ли синхроимпульс

				if (recorded > 0)//дозапись сигнала (обязательно recorded < recLen)
				{
					for (z = 0; z < ftChan; z++)//запись данных с всех каналов
					{
						rSignals[ik].s[(z * recLen) + recorded] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
						rmGrafik[(z * recLen) + recorded] = (double)rSignals[ik].s[(z * recLen) + recorded] * sampl2mV;
					}
					/*else потом впишем локальное среднее*/
					recorded++;//увеличиваем счётчик записанных отсчётов для данного сигнала

					//ВАЖНО! Следим за наложением синхроимпульса на предыдущую запись (нестабильность частоты стимуляции)
					if (isSynchro && (recorded > impInterv))//синхроимпульс уже появился, а сигнал ещё не записан
					{
						recorded = recLen;//считаем сигнал полностью записанным
						i -= chanls;//шаг назад, чтобы на следующем витке цикла isSynchro тоже было true
					}

					if (recorded >= recLen)//сигнал полностью записан
					{
						if (!multiCh)//запишем средние для второстепенных каналов
							for (z = 0; z < chanls - 1; z++)
								rSignals[ik].s[recLen + z] = (short)rLocMean[z + 1];//ноль-линия на второстепенном канале
								//rSignals[ik].s[recLen + z] = (short)(minorChanMean[z] / recLen);//вычисляем среднее

						recorded = 0;//сигнал полностью записан (обнуляем счётчик записанных отсчётов)
						ik++;//увеличиваем счётчик собранных сигналов

						Graphs->CrntSig->Tag = ik;//счётчик сигналов
						Synchronize(&RecSynchroCounter);//счётчик сигналов

						//==========================================
						if (rAppEndData)//если выбран прерывистый режим
							DiscontinWrite();//вписываем полученный сигнал на жёсткий диск
						//==========================================

						if (canPlot)//если "медленная" или "редкая" стимуляция
						{
							specK1 = Graphs->CrntSig->Tag - 1;//номер сигнала, с которого начинаются неотрисованне сигналы
							specK2 = 1;//количество неотрисованных сигналов
							Synchronize(&InducSynchroPlotter);//подрисуем графики
							Synchronize(&FromMinorChan);//выводим данные со второстепенных каналов (только при редкой стимуляции)
							kRefresh++;
							if (kRefresh >= refreshNum)
							{
								kRefresh = 0;//счётчик обновления на нуль
								Gists->GistsRefresh(-1);//перерисовываем гистограммы
							}
							/* // ? остановка сбора при больших интервалах между импульсами
							QueryPerformanceCounter((LARGE_INTEGER *)&popravka);
							ind_pUtil->FreeBuffer(); ind_pUtil->Stop(); Sleep(sleepTime);
							if (!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) {ticperiod = (__int64)((ticps / 1000) * stimPrd);}
							ind_pUtil->Start(&ind_a, 0); QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);
							while (blockReady == 0)	{blockReady = ind_pUtil->GetBuffer((void*)drvData, mask);}
							curntsampl += (dopsampls);//(((curnttic - popravka) / ticps) * 1e6) / iDiscrTime);*/
						}
						if (ik == (startNum + iNumOfS))//все сигналы полностью записаны
							break;//выходим из цикла for (i = 0; i < rBlockSize; i += chanls)
						totaltic = curnttic + ((startNum + iNumOfS) - ik) * ticperiod + (10 * ticps);//если сигнал обнаружен, корректируем totaltic
					}
				}
				else//поиск очередного сигнала (при этом recorded = 0)
				{
					if (isSynchro)//сигнал обнаружен
					{
						rSignals[ik].appear = (float)((double)(appear[ik - startNum] - rBeginTakt) / (double)ticps);
						for (z = 0; z < ftChan; z++)//запись данных со всех каналов
						{
							rSignals[ik].s[(z * recLen) + recorded] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
							rmGrafik[(z * recLen) + recorded] = (double)rSignals[ik].s[(z * recLen) + recorded] * sampl2mV;
						}
						/*else потом впишем локальное среднее*/
						recorded++;//увеличиваем счётчик записанных отсчётов для данного сигнала
						sampRead = 0;//обнуляем счётчик прочитанных отсчётов
						totaltic = curnttic + ((startNum + iNumOfS) - ik) * ticperiod + (10 * ticps);//если сигнал обнаружен, корректируем totaltic
					}
					else//коррекция локального среднего только в отсутсвии записи
					{
						for (z = 0; z < chanls; z++)
						{
							rLocMean[z] += (double(convData[z] - rExtractLocMean[rCountlm * chanls + rReadOrder[z]]) / (double)rSamps);
							rExtractLocMean[rCountlm * chanls + rReadOrder[z]] = convData[z];
						}
						rCountlm++;
						if (rCountlm == rSamps)
							rCountlm = 0;
					}
				}
			}
			z = 0; errC = 0;//освобождаем захваченный блок памяти (буфер)
			while ((z == 0) && (errC <= 2))
			{
				z = r_pUtil->FreeBuffer();
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "err FreeBuf (IS r)";
				iNumOfS = ik - startNum;//условие выхода из цикла
				recorded = 0;//условие выхода из цикла
			}
		}
	}

	z = 1; errC = 0;//стоп сбора данных
	while ((z == 1) && (errC <= 2))
	{
		r_pUtil->Stop();
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
		errC++;
	}
	if (errC > 2)
		Experiment->DevEvents->Text = "ошибка ADC_Stop";

	//указатель rExtractLocMean удаляется в блоке Execute()
	drvData = NULL;
	delete[] appear; appear = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TRecordThread::Induced_N_Spnt_SgnlAcquis(__int32 allMini, __int32 iNumOfS, __int32 iStimPrd, bool canPlot)
{
	//сбор данных: вызванные + спонтанные (режим №4)
	/*
	allMini - общее количество собираемых спонтанных сигналов
	iNumOfS - число собираемых в данном блоке вызванных сигналов
	iStimPrd - период стимуляции в данном блоке
	canPlot - разрешено ли рисовать (большое "свободное" время)
	*/

	bool isSynchro;//обнаружен ли синхроимпульс
	short *drvData,
		  convData[maxChannels],//содержит выделенный код АЦП
		  *backBuffer;//содержит ранее записанный блок (спонтанный сигнал)
	unsigned short digitPort4;//значения старших четырёх битов входного цифрового порта
	__int32 i, g, z, j,
			errC,//счётчик ошибок
			ik,//счётчик записанных взыванных сигналов
			kr,//счётчик обновления (спонтанный сигнал)
			startNum,//нумерация сигналов в сложном протоколе продолжается и начинается с startNum
			mk,//счётчик записанных спонтанных сигналов
			samplBack,//отчётов назад (рассчитывается из spontanBackTime)
			bI,//индекс отсчёта в буфере, с которого начать считывание назад
			stims,//количество выданных синхроимпульсов
			recordedI,//счётчик записанных отсчётов для вызванного сигнала
			recordedM,//счётчик записанных отсчётов для спонтанного сигнала
			blockReady,//готовность блока данных
			frstIndBack,//первый индекс для бэкБуфера (спонтанный сигнал)
			//refreshNum,//период обновления графиков (спонтанный сигнал)
			ost,//остаток (сколько отсчётов читать из буферного блока, спонтаннный сигнал)
			minorChanMean[maxChannels],//среднее на второстепенном канале
			impInterv,//нижняя граница значений межимпульсных интервалов (в отсчётах)
			sampRead;//число записанных отсчётов, начиная с момента возникновения синхроимпульса
	float waitTime;//длительность паузы (мкс) перед отключением синхроимпульса
	unsigned int mask;//хранит информацию о наложении данных
	__int64 *appear,//массив с временами подачи синхроимпульсов (в тактах процессора)
			totaltic,//общее число тактов процессора, отводимое для текущего сбора (по истечении этого времени выходим из цикла)
			curnttic,//текущий такт процессора
			tpp,//тактов за перод синхроимпульса
			tp1,
			ticps,//тактов в секунду (частота процессора)
			ticperiod,//тактов процессора на период стимуляции
			popravka;//авторегулировка частоты стимуляции
	bool recMini;//разрешено ли записывать спонтанные сигналы
	
	drvData = NULL; backBuffer = NULL; appear = NULL;

	samplBack = chanls * floor((StrToInt(PStart->BackTime->Text) / effDT) + 0.5);//20% длины спонтанного сигнала (без учёта числа каналов)
	//refreshNum = iNumOfS + 1;//заведомо период обновления больше числа собираемых сигналов (т.е. нет обновления)
	//if (Experiment->ggRefresh->Checked)//можно ли обновлять графики параметров
	//	refreshNum = StrToInt(Experiment->refreshEvery->Text);

	backBuffer = new short[samplBack];//циклический буфер
	for (i = 0; i < samplBack; i++)//первое заполнение буфера backBuffer (нулями)
		backBuffer[i] = 0;
	bI = 0;
	recordedI = 0;//обнуляем счётчик записанных отсчётов
	recordedM = 0;//обнуляем счётчик записанных отсчётов
	stims = 0;//количество выданных синхроимпульсов
	appear = new __int64[iNumOfS];//массив с временами возникновения сигналов

    errC = 0;//индикатора зависания
	while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//частота процессора
		errC++;
	if (errC > 2)
	{   Experiment->DevEvents->Text = "не определена частота процессора"; return; }
	ticperiod = (ticps / (__int64)1000) * (__int64)iStimPrd;

	//рассчитаем минимальное число отсчётов между импульсами
	impInterv = floor(recLen / 2) + 1;//floor((1000 * iStimPrd * 1) / (2 * discrT));//полуинтервал между импульсами стимуляции (отсчёты)
	sampRead = impInterv;//чтобы первый сигнал читался без проблем
	startNum = Graphs->CrntSig->Tag;//номер сигнала, с которого продолжается нумерация
	ik = startNum;//количество записанных вызванных сигналов (сквозная нумерация)
	mk = PStart->BackTime->Tag;//количество записанных спонтанных сигналов (сковзная нумерация)
	kr = 0;//обнуляем счётчик обновления (пока работаем без обновлений)
	waitTime = min(float(500), discrT);//длительность паузы (мкс) перед отключением синхроимпульса
	tpp = (float)ticps * (waitTime / (float)1e6);//тактов за перод синхроимпульса

	z = r_pUtil->Start(&r_a, 0);//старт сбора данных
	if (z != 1)
	{
		Experiment->DevEvents->Text = (FindErrorStrByCode(z, 0));
		Experiment->DevEvents->Text.Insert("Induc ", 1);
		return;
	}

	z = 0; errC = 0;//проверяем начался ли сбор данных
	while ((z == 0) && (errC <= 200))
	{
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
		errC++;
	}
	if (errC > 200)
	{   Experiment->DevEvents->Text = "ошибка ADC_StatusRun"; return; }

	z = 0; errC = 0;//устанавливаем НОЛЬ на выходе ЦАП
	while ((z == 0) && (errC <= 2))
	{
		z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[1]);//устанавливаем НОЛЬ на выходе ЦАП
		Sleep(1);//ЦАП работает на частоте не более 5 кГц
		errC++;
	}
	if (errC > 2)
	{   Experiment->DevEvents->Text = "ошибка ADC_WRITE_DAC"; return; }

	for (i = 0; i < 2; i++)//"разминочный" сбор
	{
		blockReady = 0;
		while (blockReady == 0)
			blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

		z = 0; errC = 0;//освобождаем захваченный блок памяти (буфер)
		while ((z == 0) && (errC <= 2))
		{
			z = r_pUtil->FreeBuffer();
			errC++;
		}
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "ошибка FreeBuffer";
			break;//аварийное завершение разминочного сбора
		}
	}
	if (errC > 2)
		return;//"разминка" не прошла

	QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);
	if (curnttic > rNextTic)
		totaltic = curnttic + (iNumOfS * ticperiod) + (2 * ticps);//далее totaltic корректируется
	else
		totaltic = rNextTic + (iNumOfS * ticperiod) + (2 * ticps);//далее totaltic корректируется

	while (((ik < (startNum + iNumOfS)) || (recordedI > 0)) && (curnttic <= totaltic))
	{
		//блок подачи синхроимпульсов
		errC = 0;//индикатора зависания
		while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//частота процессора
			errC++;
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "ошибка QPFrequency";
			iNumOfS = ik - startNum;//условие выхода из цикла
			recordedI = 0;//условие выхода из цикла
		}
		ticperiod = (ticps / (__int64)1000) * (__int64)iStimPrd;

		errC = 0;//индикатора зависания (не даём циклу возможность работать бесконечно)
		while((!QueryPerformanceCounter((LARGE_INTEGER *)&curnttic)) && (errC <= 2))
			errC++;
		if (errC > 2)
		{
			Experiment->DevEvents->Text = "ошибка QPCounter";
			iNumOfS = ik - startNum;//условие выхода из цикла
			recordedI = 0;//условие выхода из цикла
		}

		popravka = rNextTic - curnttic;
		if ((popravka <= 0) && (stims < iNumOfS))//вывод стимулирующего сигнала
		{
			//SetToDAC();//устанавливаем НЕНОЛЬ на выходе ЦАП
			z = 0; errC = 0;//устанавливаем НЕНОЛЬ на выходе ЦАП
			while ((z == 0) && (errC <= 2))
			{
				z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[0]);//устанавливаем НЕНОЛЬ на выходе ЦАП
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "ошибка ADC_WRITE_DAC";
				iNumOfS = ik - startNum;//условие выхода из цикла
				recordedI = 0;//условие выхода из цикла
			}

            if (((popravka + ticperiod) < 0) || (stims == 0))
				rNextTic = curnttic + ticperiod;//момент (в тактах процессора) подачи следующего синхроимпульса
			else
				rNextTic = curnttic + ticperiod + popravka;//момент (в тактах процессора) подачи следующего синхроимпульса
			appear[stims] = curnttic;//запоминаем время возникновения сигнала
			stims++;//увеличивем счётчик поданых синхроимпульсов

			//делаем паузу перед отключением синхроимпульса
			tp1 = curnttic;
			while ((curnttic - tp1) < tpp)
				QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);

			z = 0; errC = 0;//устанавливаем НОЛЬ на выходе ЦАП
			while ((z == 0) && (errC <= 2))
			{
				z = r_pADC->Get(ADC_WRITE_DAC, &stim_out[1]);//устанавливаем НОЛЬ на выходе ЦАП
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "ошибка ADC_WRITE_DAC";
				iNumOfS = ik - startNum;//условие выхода из цикла
				recordedI = 0;//условие выхода из цикла
			}
		}

		blockReady = r_pUtil->GetBuffer((void*)drvData, mask);//запрашивем блок данных из буфера DMA
		if (blockReady != 0)//блок данных получен
		{
			if ((PStart->CloseWin->Tag == 1) || (PStart->NextBlock->Tag == 1))//не пора ли остановиться?
				iNumOfS = ik - startNum;//преждевременный выход (из эксперимента или блока)

			//поиск и запись сигналов (всех в одном цикле)
			for (i = 0; i < rBlockSize; i += chanls)//перебираем отсчёты в блоке данных
			{
				//выделение кода АЦП и заполнение циклического буфера
				for (z = 0; z < chanls; z++)
				{
					convData[z] = (drvData[i + rReadOrder[z]] >> bwSh);//выделяем код АЦП (старшие 12 (14) бит)
					backBuffer[bI] = convData[z];//запись в буфер
					bI++;//смещаем метку
				}
				if (bI >= samplBack)
					bI = 0;//возврат на начало циклического буфера

				sampRead += chanls;//прирост счётчика прочитанных отсчётов
				recMini = ((mk < allMini) && (PStart->PausSpontan->Tag == 0) && (recordedI == 0));//разрешено ли записывать спонтанные сигналы

				//логическое "И" oDrvData[i] & 0000000000001111 - выделяем последние 4 бита
				//0xF = 15(dec) = 0000000000001111(bin)
				digitPort4 = ((unsigned short)drvData[i]) & eds;//(oDrvData[i] << 12) >> 12;
				isSynchro = ((digitPort4 > 2) && (sampRead >= impInterv));//обнаружен ли синхроимпульс

				//---------------------------------
				//поиск и запись ВЫЗВАННЫХ сигналов
				if (recordedI > 0)//дозапись сигнала (обязательно recorded < toRec)
				{
					for (z = 0; z < ftChan; z++)//запись данных со всех каналов
					{
						rSignals[ik].s[(z * recLen) + recordedI] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
						rmGrafik[(z * recLen) + recordedI] = (double)rSignals[ik].s[(z * recLen) + recordedI] * sampl2mV;
					}
					/*else потом впишем локальное среднее*/
					recordedI++;//увеличиваем счётчик записанных отсчётов для данного сигнала

					//ВАЖНО! Следим за наложением синхроимпульса на предыдущую запись (нестабильность частоты стимуляции)
					if (isSynchro && (recordedI > impInterv))// && (ik < (startNum + iNumOfS)))
					{
						recordedI = recLen;//считаем сигнал полностью записанным
						i -= chanls;//шаг назад, чтобы на следующем витке цикла isSynchro тоже было true
					}

					if (recordedI >= recLen)//сигнал полностью записан
					{
                        if (!multiCh)//запишем средние для второстепенных каналов
							for (z = 0; z < chanls - 1; z++)
								rSignals[ik].s[recLen + z] = (short)rLocMean[z + 1];//(short)(minorChanMean[z] / recLen);//вычисляем среднее
								
						recordedI = 0;//сигнал записан полностью (обнуляем счётчик записанных отсчётов)
						ik++;//увеличиваем счётчик собранных сигналов

						Graphs->CrntSig->Tag = ik;//счётчик сигналов
						Synchronize(&RecSynchroCounter);//счётчик сигналов

						//==========================================
						if (rAppEndData)
							DiscontinWrite();//вписываем полученный сигнал на жёсткий диск
						//==========================================

						if (canPlot)//если "медленная" ("редкая") стимуляция
						{
							//подрисуем графики
							specK1 = Graphs->CrntSig->Tag - 1;//номер сигнала, с которого начинаются неотрисованне сигналы
							specK2 = 1;//количество неотрисованных сигналов
							Synchronize(&InducSynchroPlotter);//рисование графиков

							//выводим данные со второстепенных каналов (только при редкой стимуляции)
							Synchronize(&FromMinorChan);
						}
						if (ik == (startNum + iNumOfS))//все сигналы полностью записаны
							break;//выходим из цикла for (i = 0; i < rBlockSize; i += chanls)

						//если сигнал обнаружен, корректируем totaltic
						totaltic = curnttic + ((startNum + iNumOfS) - ik) * ticperiod + ticps;
					}
				}
				else//поиск очередного сигнала (синхроимпульса, при этом recordedI = 0)
				{
					if (isSynchro)//сигнал обнаружен
					{
						rSignals[ik].appear = (float)((double)(appear[ik - startNum] - rBeginTakt) / (double)ticps);
						for (z = 0; z < ftChan; z++)//запись данных со всех каналов
						{
							rSignals[ik].s[(z * recLen) + recordedI] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
							rmGrafik[(z * recLen) + recordedI] = (double)rSignals[ik].s[(z * recLen) + recordedI] * sampl2mV;
						}
						/*else потом впишем локальное среднее*/
						recordedI++;//увеличиваем счётчик записанных отсчётов для данного сигнала
						sampRead = 0;//обнуляем счётчик прочитанных отсчётов
						totaltic = curnttic + ((startNum + iNumOfS) - ik) * ticperiod + ticps;//корректируем totaltic
					}
					else//между импульсами стимуляции корректируем локальное среднее (несмотря на спонтанные сигналы)
					{
						for (z = 0; z < chanls; z++)
						{
							rLocMean[z] += (double(convData[z] - rExtractLocMean[rCountlm * chanls + rReadOrder[z]]) / (double)rSamps);
							rExtractLocMean[rCountlm * chanls + rReadOrder[z]] = convData[z];
						}
						rCountlm++;
						if (rCountlm == rSamps)
							rCountlm = 0;
                    }
				}

				//-----------------------------------
				//поиск и запись СПОНТАННЫХ сигналов
				//запись спонтанных сигналов останавливается, когда:
				//1) все сигналы собраны (mk >= allMini)
				//2) нажата кнопка паузы для миниатюрных (PStart->pausMini->Tag == 1)
				//3) идёт запись вызванного сигнала (recordedI > 0)
				if (recMini)
				{
					if (recordedM == 0)//ищем очередной спонтанный сигнал
					{
						if ((rPolarity * (convData[0] - (short)rLocMean[0] * rConstMinus)) >= rPorog)
						{
							//сигнал обнаружен (это возможно только после полной записи вызванного сигнала, значит вр. возн-я спонт. сиг...)
							rSpntSignals[mk].appear = rSignals[ik - 1].appear + (sampRead * discrT * (1e-6));//время возникновения сигнала (в секундах)

							//записываем из буфера начало спайка
							for (g = bI; g < samplBack; g += chanls)
							{
								for (z = 0; z < ftChan; z++)//записываем данные со всех каналов
								{
									rSpntSignals[mk].s[(z * rSpntRecLen) + recordedM] = rPolarity * (backBuffer[g + z] - (short)rLocMean[z] * rConstMinus);
									//rmGrafikM[(z * rSpntRecLen) + recordedM] = (double)rSignals[ik].s[(z * rSpntRecLen) + recordedM] * sampl2mV;
								}
								/*else потом впишем локальное среднее*/
								recordedM++;//увеличиваем счётчик записанных отсчётов для данного сигнала
							}
							for (g = 0; g < bI; g += chanls)//продолжение записи из буфера
							{
								for (z = 0; z < ftChan; z++)//записываем данные со всех каналов
								{
									rSpntSignals[mk].s[(z * rSpntRecLen) + recordedM] = rPolarity * (backBuffer[g + z] - (short)rLocMean[z] * rConstMinus);
									//rmGrafikM[(z * rSpntRecLen) + recordedM] = (double)rSignals[ik].s[(z * rSpntRecLen) + recordedM] * sampl2mV;
								}
								/*else потом впишем локальное среднее*/
								recordedM++;//увеличиваем счётчик записанных отсчётов для данного сигнала
							}
						}
						else
						{
							//корректируем локальное среднее (также при сборе вызванных)
							for (z = 0; z < chanls; z++)
							{
								rLocMean[z] += (double(convData[z] - rExtractLocMean[rCountlm * chanls + rReadOrder[z]]) / (double)rSamps);
								rExtractLocMean[rCountlm * chanls + rReadOrder[z]] = convData[z];
							}
							rCountlm++;
							if (rCountlm == rSamps)
								rCountlm = 0;
						}
					}
					else//дозапись спонтанного сигнала
					{
						for (z = 0; z < ftChan; z++)//записываем данные со всех каналов
						{
							rSpntSignals[mk].s[(z * rSpntRecLen) + recordedM] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
							//rmGrafikM[(z * rSpntRecLen) + recordedM] = (double)rSignals[ik].s[(z * rSpntRecLen) + recordedM] * sampl2mV;
						}
						recordedM++;//увеличиваем счётчик записанных отсчётов для данного сигнала
						if (recordedM >= rSpntRecLen)//сигнал полностью записан (окончания записи одиночного сигнала)
						{
							if (!multiCh)//запишем средние для второстепенных каналов
								for (z = 0; z < chanls - 1; z++)
									rSpntSignals[mk].s[rSpntRecLen + z] = (short)rLocMean[z + 1];//(short)(minorChanMean[z] / rSpntRecLen);//вычисляем среднее

							recordedM = 0;//сигнал полностьюя записан (обнуляем счётчик записанных отсчётов)
							mk++;//увеличивем счётчик спонтанных сигналов
							kr++;//счётчик обновления (спонтанный сигнал, пока не обновляется)
							//==========================================
							//if (rAppEndData)
							//	DiscontinMiniWrite();//вписываем полученный спонтанный сигнал на жёсткий диск
							//==========================================
							PStart->BackTime->Tag = mk;//счётчик сигналов
							//Synchronize(&RecSynchroCounter);//счётчик сигналов
//							Synchronize(&MiniMd4SynchroPlot);//сразу же рисуем крайний спонтанный сигнал
						}
					}
				}
				else if ((recordedI > 0) && (recordedM > 0))//во время записи спонтанного возник вызванный сигнал
				{
                    //преждевременное завершение записи обнаруженного сигнала
					recordedM = 0;//сигнал полностью записан (обнуляем счётчик записанных отсчётов)
					mk++;//счётчик спонтанных сигналов
					kr++;//счётчик обновления (спонтанный сигнал, пока не обновляется)
					PStart->BackTime->Tag = mk;//счётчик сигналов
				}
			}

			z = 0; errC = 0;//освобождаем захваченный блок памяти (буфер)
			while ((z == 0) && (errC <= 2))
			{
				z = r_pUtil->FreeBuffer();
				errC++;
			}
			if (errC > 2)
			{
				Experiment->DevEvents->Text = "ошибка FreeBuffer";
				iNumOfS = ik - startNum;//условие выхода из цикла
				recordedI = 0;//условие выхода из цикла
			}
		}
	}

	z = 1; errC = 0;//стоп сбора данных
	while ((z == 1) && (errC <= 2))
	{
		r_pUtil->Stop();
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
		errC++;
	}
	if (errC > 2)
		Experiment->DevEvents->Text = "ошибка ADC_Stop";

	//указатель rExtractLocMean удаляется в блоке Execute()
	drvData = NULL;
	delete[] backBuffer; backBuffer = NULL;
	delete[] appear; appear = NULL;
}
//---------------------------------------------------------------------------

void TRecordThread::Spnt_SgnlAcquisition(__int32 mNumOfS, clock_t experimBegin)
{
	//сбор данных: спонтанные сигналы №1
	/*
	mNumOfS - число собираемых сигналов
	*/

	unsigned int mask;// хранит информацию о наложении данных
	__int32 i, g, z, j,//индексы в циклах
        	k,//счётчик сигналов
			kRefresh,//счётчик обновления
			samplBack,//отчётов назад (рассчитывается из spontanBackTime)
			bI,//индекс отсчёта в буфере, с которого начать считывание назад
			startNum,//нумерация сигналов в сложном протоколе продолжается и начинается с startNum
			recorded,//количество записанных отсчётов
			frstIndBack,//первый индекс для бэкБуфера
			refreshNum,//период обновления графиков (в сигналах)
			ost,//остаток (сколько отсчётов читать из буферного блока)
			blockReady,//готовность блока данных
			minorChanMean[maxChannels];//среднее на второстепенном канале
	short *drvData,//содержит записанный сейчас блок
		  convData[maxChannels],//содержит выделенный код АЦП для всех каналов (каналов не больше, чем maxChannels)
		  *backBuffer;//содержит ранее записанный блок
	unsigned long int allsampls;//абсолютный номер текущего отсчёта
	clock_t timeMoment;//момент обнаружения сигнала (миллисекунд от начала работы программы)
	__int64 curnttic;//текущий такт процессора

	drvData = NULL; backBuffer = NULL;
	samplBack = chanls * floor(((StrToInt(PStart->BackTime->Text) * 1000) / effDT) + 0.5);//количество отсчётов "назад" для спонтанного сигнала (без учёта числа каналов)
	refreshNum = mNumOfS + 1;//заведомо период обновления больше числа собираемых сигналов (т.е. нет обновления)
	if (Experiment->ggRefresh->Checked)
		refreshNum = StrToInt(Experiment->refreshEvery->Text);
	backBuffer = new short[samplBack];//циклический буфер; длина буфера равна времени назад в отсчётах (примерно)
	for (i = 0; i < samplBack; i++)//первое заполнение буфера backBuffer (нулями)
		backBuffer[i] = 0;
	bI = 0;
	allsampls = 0;//обнуляем счётчик отсчётов
	recorded = 0;

	QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);//текущий такт процессора
	while (curnttic < rNextTic)
	{
		Sleep(10);//ждём времени начала следующего сеанса сбора спонтанных сигналов
		QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);//текущий такт процессора
    }

	z = r_pUtil->Start(&r_a, 0);
	if (z != 1)
	{   Experiment->DevEvents->Text = (FindErrorStrByCode(z,0)); return; }

	z = 0;
	while (z == 0)
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0

	for (i = 0; i < 4; i++)//"разминочный" сбор
	{
		blockReady = 0;
		while (blockReady == 0)
			blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

		z = 0;//освобождаем захваченный блок памяти (буфер)
		while (z == 0)
			z = r_pUtil->FreeBuffer();
	}

	kRefresh = 0;//обнуляем счётчик обновления
	startNum = Graphs->CrntSig->Tag;//нумерация сигналов в сложном протоколе продолжается и начинается с startNum
	k = startNum;//количество собранных сигналов

	while (k < (startNum + mNumOfS))
	{
		if (PStart->CloseWin->Tag == 1)//остановить ли запись
			mNumOfS = k - startNum;//условие выхода из цикла while (k < (startNum + mNumOfS))

		blockReady = 0;
		while(blockReady == 0)
			blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

		for (i = 0; i < rBlockSize; i += chanls)
		{
			for (z = 0; z < chanls; z++)
			{
				convData[z] = (drvData[i + rReadOrder[z]] >> bwSh);//выделяем код АЦП (старшие 12 (14) бит)
				backBuffer[bI] = convData[z];//запись в буфер
				bI++;//смещаем метку
			}
			if (bI >= samplBack)
				bI = 0;//возврат на начало циклического буфера

			if (recorded == 0)//поиск сигнала
			{
				if ((rPolarity * (convData[0] - (short)rLocMean[0] * rConstMinus)) >= rPorog)//сигнал обнаружен
				{
					//rSignals[k].appear = (allsampls + i) * discrT * (1e-6);//время возникновения сигнала (в секундах)
					timeMoment = clock();//момент обнаружения сигнала (миллисекунд от начала работы программы)
					rSignals[k].appear = ((float)timeMoment / 1000) - (float)experimBegin;

					//записываем из буфера начало спайка
					for (g = bI; g < samplBack; g += chanls)
					{
						for (z = 0; z < ftChan; z++)//записываем данные со всех каналов
						{
							rSignals[k].s[(z * recLen) + recorded] = rPolarity * (backBuffer[g + z] - (short)rLocMean[z] * rConstMinus);
							rmGrafik[(z * recLen) + recorded] = (double)rSignals[k].s[(z * recLen) + recorded] * sampl2mV;
						}
						/*else потом впишем локальное среднее*/
						recorded++;//приращение числа записанных отсчётов для данного сигнала
					}
					for (g = 0; g < bI; g += chanls)//продолжение записи из буфера
					{
						for (z = 0; z < ftChan; z++)//записываем данные со всех каналов
						{
							rSignals[k].s[(z * recLen) + recorded] = rPolarity * (backBuffer[g + z] - (short)rLocMean[z] * rConstMinus);
							rmGrafik[(z * recLen) + recorded] = (double)rSignals[k].s[(z * recLen) + recorded] * sampl2mV;
						}
						/*else//одноканальный режим
							for (z = 1; z < chanls; z++)//среднее в период регистрации сигнала
								minorChanMean[z - 1] += backBuffer[g + z];//потом подсчитаем среднее*/
						recorded++;//приращение числа записанных отсчётов для данного сигнала
					}
				}
				else//дозапись сигнала
				{
					for (z = 0; z < chanls; z++)//корректируем локальное среднее
					{
						rLocMean[z] += (double(convData[z] - rExtractLocMean[rCountlm * chanls + rReadOrder[z]]) / (double)rSamps);
						rExtractLocMean[rCountlm * chanls + rReadOrder[z]] = convData[z];
					}
					rCountlm++;
					if (rCountlm == rSamps)
						rCountlm = 0;
                }
			}
			else//(recorded > 0)
			{
				//дозапись сигнала
				for (z = 0; z < ftChan; z++)//записываем данные со всех каналов
				{
					rSignals[k].s[(z * recLen) + recorded] = rPolarity * (convData[z] - (short)rLocMean[z] * rConstMinus);
					rmGrafik[(z * recLen) + recorded] = (double)rSignals[k].s[(z * recLen) + recorded] * sampl2mV;
				}
				/*else потом впишем локальное среднее*/
				recorded++;//приращение числа записанных отсчётов для данного сигнала

				if (recorded >= recLen)//сигнал полностью записан (окончания записи одиночного сигнала)
				{
					if (!multiCh)//запишем средние для второстепенных каналов
						for (z = 0; z < chanls - 1; z++)
							rSignals[k].s[recLen + z] = (short)rLocMean[z + 1];//(short)(minorChanMean[z] / recLen);//вычисляем среднее
					recorded = 0;//сигнал записан полностью (обнуляем счётчик записанных отсчётов)
					k++;
					kRefresh++;
					Graphs->CrntSig->Tag = k;//счётчик сигналов
					Synchronize(&RecSynchroCounter);//счётчик сигналов

					//==========================================
					if (rAppEndData)
						DiscontinWrite();//вписываем полученный сигнал на жёсткий диск
					//==========================================

					Synchronize(&FromMinorChan);//второстепенные каналы
					if (k == (startNum + mNumOfS))//все сигналы полностью записаны
						break;//прерываем цикл for (i = 0; i < rBlockSize; i += chanls)
				}
			}
		}

		allsampls += rBlockSize;//счётчик обработанных отсчётов

		z = 0;//освобождаем захваченный блок памяти (буфер)
		while (z == 0)
			z = r_pUtil->FreeBuffer();

		if (((kRefresh == refreshNum) || (k == (startNum + mNumOfS))) && (k > 0))//разрешено ли обновить графики
		{
			z = 1;
			while (z == 1)
			{
				r_pUtil->Stop();
				r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
			}

			if (PStart->CloseWin->Tag == 1)//остановить ли запись
				mNumOfS = k - startNum;

			//обновляем графики
			recorded = 0;
			specK1 = (k - kRefresh);//номер сигнала, с которого начинаются неотрисованные сигналы
			specK2 = kRefresh;//количество неотрисованных сигналов
			Synchronize(&MiniSynchroPlotGraphs);//рисование графиков

			z = r_pUtil->Start(&r_a, 0);
			if (z <= 0)
			{   Experiment->DevEvents->Text = (FindErrorStrByCode(z,0)); return; }
			z = 0;
			while (z == 0)
				r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0

			blockReady = 0;
			while(blockReady == 0)
				blockReady = r_pUtil->GetBuffer((void*)drvData, mask);

			for (i = 0; i < samplBack; i++)
				backBuffer[i] = 0;//заполнение буфера backBuffer (нулями)
			bI = 0;
			kRefresh = 0;//обнуляем счётчик обновления
		}
	}

	//стоп сбора данных
	z = 1;
	while (z == 1)
	{
		r_pUtil->Stop();
		r_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
	}

	drvData = NULL;
	delete[] backBuffer; backBuffer = NULL;
}
//---------------------------------------------------------------------------

void TRecordThread::DiscontinWrite()
{
	//все типы режимов. Прерывистый сбор данных (сохранение каждого нового сигнала сразу на жёсткий диск)

	__int32 i,
			fullSigLen,//полная длина данных со всех каналов (отсчёты)
			ikNow,//текущий номер сигнала
			itemWrtRd,//число записанных единиц (как правило пишем по одной единице)
			amountOfBytes;//количество байт, записываемых данном блоке (инфо + сигнал)
	char dLett[3];//символ-разделитель между сигналами

	dLett[0] = 'S'; dLett[1] = 'i'; dLett[2] = 'g';//символ-разделитель между сигналами

	ikNow = Graphs->CrntSig->Tag - 1;//текущий номер сигнала
	fullSigLen = (recLen * ftChan) + (chanls - ftChan);//полная длина данных со всех каналов (отсчёты)
	amountOfBytes = (3 * sizeof(char)) + sizeof(float) + (fullSigLen * sizeof(short));//должно быть записано байт
	
	//разделитель - развёртка сигнала
	itemWrtRd = fwrite(&dLett, sizeof(char), 3, rDiscontinFile);//разделитель 'Sig'
	amountOfBytes -= itemWrtRd * sizeof(char);

	//время возникновения
	itemWrtRd = fwrite(&rSignals[ikNow].appear, sizeof(float), 1, rDiscontinFile);//время возникновения
	amountOfBytes -= itemWrtRd * sizeof(float);

	//сам сигнал (данные со всех каналов)
	itemWrtRd = fwrite(rSignals[ikNow].s, sizeof(short), fullSigLen, rDiscontinFile);
	amountOfBytes -= itemWrtRd * sizeof(short);

	if (amountOfBytes != 0)
		Experiment->DevEvents->Text = "ошибка записи";//аварийный выход, видимо, можно не делать
}
//---------------------------------------------------------------------------

void TRecordThread::GetLocalMean()
{
	//вычисляем начальное среднее (для нивелирования постоянной составляющей)
	__int32 i, j;

	rSamps = (__int32)floor((float(baseLine) / effDT) + 0.5);
	rExtractLocMean = new short[rSamps * chanls];//массив для вычисления локального среднего удаляется в Execute()
	Experiment->BeginMean(rExtractLocMean, rSamps * chanls);
	
	for (j = 0; j < chanls; j++)
	{
		rLocMean[j] = 0;//массив средних значений для всех каналов
		for (i = 0; i < rSamps; i++)
			rLocMean[j] += (double)rExtractLocMean[i * chanls + rReadOrder[j]];
		rLocMean[j] /= (double)rSamps;
	}

	rCountlm = 0;
}
//---------------------------------------------------------------------------

//void TRecordThread::SetToDAC()
//{
//	//устанавливаем НЕНОЛЬ-НОЛЬ на выходе ЦАП
//}
//---------------------------------------------------------------------------

