//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <math.h>
#include <algorith.h>

#pragma hdrstop

#include "TimeCourses.h"
#include "Gistograms.h"
#include "MainForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TGists *Gists;

//---------------------------------------------------------------------------
__fastcall TGists::TGists(TComponent* Owner)
	: TForm(Owner)
{
	//модуль построения гистограмм
	//также выводится зависимость частоты миниатюрных от времени

	allCharts[0] = MiniFreq;//частота появления сигналов
	allCharts[1] = amplG;//амплитуды сигналов
	allCharts[2] = riseTG;//времена нарастания
	allCharts[3] = fallTG;//времена спада
	allCharts[4] = NaAmplG;//амплитуды Na-тока
	allCharts[5] = NaDurG;//длительности Na-тока
	allCharts[6] = KAmplG;//амплитуды К-тока
	allCharts[7] = synDelayG;//синаптические задержка
	allCharts[8] = synTrnsG;//времена синаптического проведения
}
//---------------------------------------------------------------------------
void __fastcall TGists::Resized(TObject *Sender)
{
	//при изменении видимости гистограмм
	__int32 numOfgraphs = 0, // количество высвечиваемых графиков
			i,
			winWidth,//ширина окна графиков
			winHeight,//высота окна графиков
			//индексы в массиве ссылок видимых графиков (чартов
			iVCh[4] = {0, 0, 0, 0};

	//ищем ссылки на видимые графики (чарты)

	for (i = 0; i < 9; i++)
		if (allCharts[i]->Visible)
		{
            iVCh[numOfgraphs] = i;
			numOfgraphs++;
		}

	winWidth = Gists->Width;
	if ((numOfgraphs == 2) | (numOfgraphs == 4))
	{
		 if ((winWidth / 2) < 235)
			 Gists->Width = 235 * 2;
	}
	if (numOfgraphs == 3)
	{
		 if ((winWidth / 3) < 235)
			 Gists->Width = 235 * 3;
	}

	winWidth = Gists->Width - 7;
	winHeight = Gists->Height - 35;

	if (numOfgraphs == 0)//закрываем окно, если нет видимых гистрограмм
		Gists->Close();

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
}
//---------------------------------------------------------------------------

void TGists::RateMiniSigCalc(TLineSeries *graphOnGraphs)
{
	//расчёт и построение графика частоты миниаютрных сигналов
	/*
	graphOnGraphs - ссылка на график, из которого берём данные
	*/

	__int32 i,
			count;//количество сигналов
	float tAppear1,//время возникновения сигнала
		  tAppear2,//время возникновения следующего сигнала
		  difTAp,//разница времён возникновения
		  meanFreq,//средняя частота сигналов
		  lastFreq;//частота в последний момент

	count = graphOnGraphs->Count();
	if (count > 1)
	{
		freqM->Clear();
		MiniFreq->LeftAxis->Automatic = true;
		meanFreq = 0;
		for (i = 0; i < (count - 1); i++)
		{
			tAppear1 = graphOnGraphs->XValues->operator [](i);//время возникновения сигнала //gstSignals[gstIndex[i]].appear;//
			tAppear2 = graphOnGraphs->XValues->operator [](i + 1);//время возникновения следующего сигнала
			if (tAppear2 <= 0)
				tAppear2 = 0;

			difTAp = tAppear2 - tAppear1;
			if (difTAp < 0.001)
				difTAp = 0.001;

			freqM->AddXY(tAppear2, 1 / difTAp);
		}

		lastFreq = 1 / difTAp;//частота в последний момент
		meanFreq = count / tAppear2;//общее число сигналов за всё время

		if ((freqM->MaxYValue() - freqM->MinYValue()) < 1)
		{
			MiniFreq->LeftAxis->Automatic = false;
			MiniFreq->LeftAxis->SetMinMax(lastFreq - 1, lastFreq + 1);
		}

		MiniFreq->Title->Text->Text = "Среняя частота " + FloatToStrF(meanFreq, ffFixed, 5, 2) + " Гц";
	}
}
//---------------------------------------------------------------------------

float TGists::GistsCalculation(TLineSeries *graphOnGraphs, TBarSeries *theGist)
{
	//непосредственно расчёт гистограммы для заданного графика
	/*
	graphOnGraphs - ссылка на график, из которого берём данные
	theGist - ссылка на гистограмму со значениями, входящими в заданный диапазон
	*/

	__int32 i, g,
			count,//количество записанных сигналов (количество значений параметра)
			nOutL,//количество значений, оставшихся перед левой границей
			nOutLplus,//количество значений, уходящих за левую границу
			nOutR,//количество значений, оставшихся за правой   границей
			nOutRplus,//количество значений, уходящих за правую границу
			resultPc,//реальный процент отображаемых значений
			intervals,//количество интервалов, по которым распределяем значения
			*vals;//массив с частотами соответствующих значений параметров
	float y,
		  viewPc,//процент отображаемых значений
		  minY,//левая граница диапазона значений
		  maxY,//правая граница диапазона значений
		  minYback,
		  maxYback,
		  pcStep,//минимальный шаг изменения процентов
		  dY,
		  increm,//шаг перебора значений параметра
		  average;//среднее значение параметра
	bool znak;//знак процентов

	count = graphOnGraphs->Count();//количество значений параметра
	viewPc = ((float)abs(theGist->ParentChart->Tag)) / 100;//процент отображаемых значений
	znak = (theGist->ParentChart->Tag > 0);//знак процентов
	if (count > 1)
	{
		minY = (float)(graphOnGraphs->MinYValue());//левая граница диапазона значений
		maxY = (float)(graphOnGraphs->MaxYValue());//правая граница диапазона значений
		increm = (maxY - minY) / 1000;//шаг перебора значений параметра
		pcStep = 1 / count;//минимальный шаг изменения процентов
		if (pcStep < 0.01)
			pcStep = 0.01;
		nOutL = 0;//количество значений, оставшихся перед левой границей
		nOutR = 0;//количество значений, оставшихся за правой границей
		while ((((1 - viewPc) - ((float)(nOutL + nOutR) / count)) > pcStep) && (minY < maxY))//смещаем границы до достижения заданного процента отображаемых значений
		{
            minYback = minY;
			maxYback = maxY;
			nOutLplus = 0;//количество значений, уходящих за левую границу
			dY = 0;
			while ((((float)nOutLplus / count) < pcStep) && ((minY + dY) < maxY))//смещаем левую границу "на (pcStep * 100)%"
			{
				nOutLplus = 0;//количество значений, уходящих за левую границу
				nOutL = 0;//количество значений, оставшихся перед левой границей
				dY += increm;
				for (i = 0; i < count; i++)
				{
					y = (float)graphOnGraphs->YValues->operator [](i);
					if (y < (minY + dY))
					{
						nOutL++;//увеличиваем количество значений, оставшихся перед левой границей
						if (y >= minY)
							nOutLplus++;//увеличиваем количество значений, уходящих за левую границу
					}
				}
			}
			minY += dY;

			nOutRplus = 0;//количество значений, уходящих за правую границу
			dY = 0;//
			while ((((float)nOutRplus / count) < pcStep) && (minY < (maxY - dY)))//смещаем правую границу "на (pcStep * 100)%"
			{
				nOutRplus = 0;//количество значений, уходящих за правую границу
				nOutR = 0;//количество значений, оставшихся за правой границей
				dY += increm;
				for (i = 0; i < count; i++)
				{
					y = (float)graphOnGraphs->YValues->operator [](i);
					if (y > (maxY - dY))
					{
						nOutR++;//увеличиваем количество значений, оставшихся за правой границей
						if (y <= maxY)
							nOutRplus++;//увеличиваем количество значений, уходящих за правую границу
					}
				}
			}
			maxY -= dY;
		}
		resultPc = floor(((float)((count - nOutR - nOutL) * 100) / count) + 0.5);//реальный процент отображаемых значений
		if ((znak && (viewPc > ((float)resultPc / 100))) ||
			(!znak && (viewPc < ((float)resultPc / 100))))//если проскочили мимо нужной точки
		{
			minY = minYback;
			maxY = maxYback;
		}

		intervals = theGist->Tag;//количество интервалов, по которым распределяем значения
		vals = new __int32[intervals + 2];
		average = 0;//среднее значение
		for (i = 0; i < (intervals + 2); i++)
			vals[i] = 0;//зануляем vals
		increm = (maxY - minY) / intervals;//шаг перебора значений параметра
		for (i = 0; i < count; i++)//перебираем значения параметров
		{
			y = graphOnGraphs->YValues->operator [](i);//берём значение параметра для анализа
			//определяем принадлежность интервалу
			if (y < minY)
				vals[0]++;
			else if (y > maxY)
				vals[intervals + 1]++;
			else
			{
				g = 1 + floor((y - minY) / increm);//определяем интервал, которому принадлежит y
				if (g >= (intervals + 1))//для y = maxY
					g = intervals;
				vals[g]++;
				average += y;
			}
		}
		resultPc = floor(((float)((count - vals[0] - vals[intervals + 1]) * 100) / count) + 0.5);//реальный процент отображаемых значений
		theGist->ParentChart->Tag = resultPc;//реальный процент отображаемых значений
		//строим гистограмму распределения параметра
		theGist->Clear();
		i = 0;
		if (vals[0] > 0)
			theGist->AddXY(minY + (increm * (i - 0.5)), vals[0], "", clGreen);//количество предпредельных значений
		for (i = 1; i < (intervals + 1); i++)
			theGist->AddXY(minY + (increm * (i - 0.5)), vals[i]);//в пределах диапазона
		if (vals[intervals + 1] > 0)
			theGist->AddXY(minY + (increm * (i - 0.5)), vals[intervals + 1], "", clGreen);//количество запредельных значений

		delete[] vals; vals = NULL;
	}
	return (average / count);
}
//---------------------------------------------------------------------------

void __fastcall TGists::GistsClear(TObject *Sender)
{
	//затираем графики при закрытии
	__int32 i;

	for (i = 0; i < 9; i++)
		allCharts[i]->Visible = false;//скрываем все чарты

	freqM->Clear();//частота миниатюрных
	gsAmpls->Clear();//амплитуд постсинаптического сигнала
	gsRiseTs->Clear();//время нарастания
	gsFallTs->Clear();//время спада
	gsSynDels->Clear();//синаптическая задержка
	gsSynTrns->Clear();//время синаптического проведения
	gsNaAmpls->Clear();//амплитуда Na-тока
	gsNaDurs->Clear();//длительность Na-тока
	gsKAmpls->Clear();//амплитуда К-тока
	
	Experiment->RateMS_inTime->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->Ampl_gist->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->RiseTime_gist->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->FallTime_gist->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->SynDel_gist->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->St0_gist->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->NaAmpl_gist->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->NaDur_gist->Font->Style = TFontStyles();//меняем облик кнопки
	Experiment->KAmpl_gist->Font->Style = TFontStyles();//меняем облик кнопки
	
	Experiment->plottedHists = 0;//обнуляем счётчик видимых гистограмм
}
//---------------------------------------------------------------------------

void TGists::GistsRefresh(short toReplot)
{
	//перерисовываем гистограммы
	/*
	toReplot - номер гистограммы, которую нужно перериосовать (-1 - для всех)
	*/
	__int32 count;//количество обрабатываемых сигналов
	float average;//среднее значение параметра

	if ((MiniFreq->Visible) && (toReplot < 0))
		RateMiniSigCalc(Graphs->sigAmpls[0]);//нужно указывать канал для подсчёта частоты gAmplsL

	if ((amplG->Visible) && ((toReplot < 0) || (toReplot == 1)))
	{
		average = GistsCalculation(Graphs->sigAmpls[0], gsAmpls);
		amplG->Title->Text->Text = IntToStr(amplG->Tag) +  "%/" + IntToStr(gsAmpls->Tag) +
			"к. Амплитуда. Сред. = " + IntToStr((__int32)average) + " мВ";
	}

	if ((riseTG->Visible) && ((toReplot < 0) || (toReplot == 2)))
	{
		average = GistsCalculation(Graphs->gRiseTsL, gsRiseTs);
		riseTG->Title->Text->Text = IntToStr(riseTG->Tag) +  "%/" + IntToStr(gsRiseTs->Tag) +
			"к. Время нарастания. Сред. = " + IntToStr((__int32)average) + " мкс";
	}

	if ((fallTG->Visible) && ((toReplot < 0) || (toReplot == 3)))
	{
		average = GistsCalculation(Graphs->gFallTsL, gsFallTs);
		fallTG->Title->Text->Text = IntToStr(fallTG->Tag) +  "%/" + IntToStr(gsFallTs->Tag) +
			"к. Время спада. Сред. = " + IntToStr((__int32)average) + " мкс";
	}

	if ((NaAmplG->Visible) && ((toReplot < 0) || (toReplot == 4)))
	{
		average = GistsCalculation(Graphs->gNaAmplsL, gsNaAmpls);
		NaAmplG->Title->Text->Text = IntToStr(NaAmplG->Tag) +  "%/" + IntToStr(gsNaAmpls->Tag) +
			"к. Na-ток. Сред. = " + IntToStr((__int32)average) + " мВ";
	}

	if ((NaDurG->Visible) && ((toReplot < 0) || (toReplot == 5)))
	{
		average = GistsCalculation(Graphs->gNaDursL, gsNaDurs);
		NaDurG->Title->Text->Text = IntToStr(NaDurG->Tag) +  "%/" + IntToStr(gsNaDurs->Tag) +
			"к. Длительность Na-тока. Сред. = " + IntToStr((__int32)average) + " мкс";
	}

	if ((KAmplG->Visible) && ((toReplot < 0) || (toReplot == 6)))
	{
		average = GistsCalculation(Graphs->gKAmplsL, gsKAmpls);
		KAmplG->Title->Text->Text = IntToStr(KAmplG->Tag) +  "%/" + IntToStr(gsKAmpls->Tag) +
			"к. Амплитуда К-тока. Сред. = " + IntToStr((__int32)average) + " мВ";
	}

	if ((synDelayG->Visible) && ((toReplot < 0) || (toReplot == 7)))
	{
		average = GistsCalculation(Graphs->gSynDelsL, gsSynDels);
		synDelayG->Title->Text->Text = IntToStr(synDelayG->Tag) +  "%/" + IntToStr(gsSynDels->Tag) +
			"к. Синапт. задержка. Сред. = " + IntToStr((__int32)average) + " мкс";
	}

	if ((synTrnsG->Visible) && ((toReplot < 0) || (toReplot == 8)))
	{
		average = GistsCalculation(Graphs->gSynTrnsL, gsSynTrns);
		synTrnsG->Title->Text->Text = IntToStr(synTrnsG->Tag) +  "%/" + IntToStr(gsSynTrns->Tag) +
			"к. Синапт. проведение. Сред. = " + IntToStr((__int32)average) + " мкс";
	}
}
//---------------------------------------------------------------------------

void __fastcall TGists::HistChartMouseDown(TObject *Sender, TMouseButton Button,
	  TShiftState Shift, int X, int Y)
{
	//изменяем количество столбцов гистограммы
	short i;
	float average;
	TChart *theChart;//указатель на обрабатываемый чарт
	TBarSeries *theHist;//указатель на обрабатываемая гистограмма

	theChart = (TChart*)Sender;//обрабатываемый чарт
	theHist = (TBarSeries*)theChart->Series[0];//обрабатываемая гистограмма

	if ((Button == mbLeft) && !Shift.Contains(ssShift))
		theHist->Tag++;//увеличиваем количество столбцов
	else if ((Button == mbRight) && !Shift.Contains(ssShift))
		theHist->Tag--;//уменьшаем количество столбцов
	else if ((Button == mbLeft) && Shift.Contains(ssShift))
		theChart->Tag = abs(theChart->Tag) + 5;//увеличиваем процент отображаемых значений
	else if ((Button == mbRight) && Shift.Contains(ssShift))
		theChart->Tag = (-1) * (abs(theChart->Tag) - 5);//уменьшаем процент отображаемых значений

	if (theHist->Tag <= 0)
		theHist->Tag = 1;
	if (abs(theChart->Tag) < 10)
		theChart->Tag = 10 * Signt(theChart->Tag);
	else if (abs(theChart->Tag) > 100)
		theChart->Tag = 100 * Signt(theChart->Tag);

	for (i = 0; i < 9; i++)
		if (theChart == allCharts[i])//дошли до нужного номера чарта
			break;

	GistsRefresh(i);//обновляем гистограммы
}
//---------------------------------------------------------------------------

__int32 TGists::Signt(__int32 aVal)
{
	//знак числа
	if (aVal > 0)
		return 1;
	else if (aVal < 0)
		return -1;
	else
		return 0;
}
//---------------------------------------------------------------------------

