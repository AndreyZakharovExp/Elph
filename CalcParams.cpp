//---------------------------------------------------------------------------

#include <math.h>
#include <algorith.h>

#pragma hdrstop

#include "CalcParams.h"
#include "TimeCourses.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
short *cAmpls;//все пиковые значения (амплитуды)
		//cAmpls[0] = amplPost - амплитуда основная - постсинаптическая (после красной линии)
		//cAmpls[1] = ampl1NaPeak - первая амплитуда пресинаптического ответа (натриевый ток) (после синей, перед красной линии)
		//cAmpls[2] = ampl2KPeak - вторая амплитуда пресинаптического ответа (калиевый ток) (после синей, перед красной линии)
__int32 *cPeakInds,//все номера отсчётов с пиковыми значениями
			//cPeakInds[0] = indMaxPost - номер отсчёта с максимальным значением amplPost
			//cPeakInds[1] = indMax1Na - номер отсчёта с максимальным значением amplPre1
			//cPeakInds[2] = indMax2K - номер отсчёта с максимальным значением amplPre2
		*cSpans,//все характерные времена (периоды, фазы, интервалы)
			//cSpans[0] = riseT - время нарастания (микросекунды)
			//cSpans[1] = fallT - время спада (микросекунды)
			//cSpans[2] = t1NaPeak - длительность первого пика (натриевого тока, микросекунды)
			//cSpans[3] = tDelay - синаптическая задержка или интервал... (микросекунды)
			//cSpans[4] = tSynTrans - время синаптического проведения (микросекунды)
		cRecLen,//длина развёртки сигнала (отсчёты)
		cFtCh,//количество каналов с полными развёртками
		cEffDT,//эффективное время дискретизации = discrT * chanls
		cRF[2];//последовательность чтения каналов (0 - первый отображаемый канал, 1 - последний отображаемый канал)
float cSmpl2vM;//коэффициент перевода отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)

//---------------------------------------------------------------------------

void Calculate(trac *sgnl, __int32 numOfS, short porog, __int32 preTime, __int32 postTime,
			   __int32 *inds, short expT, bool newNull, sPr *avrP)
{
	//обработка сигналов (рассчёт параметров)
	/*
	sgnl - структура с сигналами и их атрибутами
	grph - указатель на модуль обработки даннах (TGraphs *)
	numOfS - количество сигналов
	porog - амплитудный порог (отсчёты)
	preTime - ПРЕ-время (в микросекундах)
	postTime - ПОСТ-время (в микросекундах)
	inds - массив индексов сигналов, которые нужно обработать
	expT - тип эксперимента
	newNull - рассчитать ли заново нуль-линию (для каждого сигнала отдельно, newNull = (PStart->nulMinus->Checked && (PStart->nulMinus->Tag == 1))
	avrP - указатель на массив с параметрами (sPr *)
	*/

	__int32 i, j, z,
			p20p,//индекс с точкой 20% амплитуды
			postSampl,//отсчёт, с которого "начинается постсинаптический" сигнал
			preSampl,//отсчёт, с которого "начинается пресинаптический" сигнал
			beginSampl,//отсчёт, с которого начинается развёртка сигнала выбранного канала (для него рассчитываем параметры)
			ftCh,//количество обрабатываемых каналов с полными развёртками
			shft1,//номер первого элемента для ampls и peakInds
			shft2;//номер первого элемента для spans
	short *allNewNulls;//массив с "новыми нулями"

	if (avrP)//расчёт параметров для усреднённого сигнала
	{
		cAmpls = avrP->ampls;//все пиковые значения (амплитуды)
		cPeakInds = avrP->peakInds;//все номера отсчётов с пиковыми значениями
		cSpans = avrP->spans;//все характерные времена (периоды, фазы, интервалы)
	}
	else//расчёт параметров для всех сигналов
	{
		cAmpls = Graphs->cPrm[0].ampls;//все пиковые значения (амплитуды)
		cPeakInds = Graphs->cPrm[0].peakInds;//все номера отсчётов с пиковыми значениями
		cSpans = Graphs->cPrm[0].spans;
	}

	cRecLen = Graphs->recLen;//длина развёртки сигнала в отсчётах
	cFtCh = Graphs->ftChan;//количество каналов с полными развёртками
	cEffDT = Graphs->effDT;//эффективное время дискретизации = discrT * chanls
	cRF[0] = Graphs->gRF[0];//первый отображаемый канал
	cRF[1] = Graphs->gRF[1];//последний отображаемый канал
	ftCh = cRF[1] - cRF[0];//количество обрабатываемых каналов с полными развёртками


	//если выбрано "вычетать ноль-линию" (PStart->nulMinus->Checked), то
	//будет расчитана ноль-линия для каждого сигнала отдельно (PStart->nulMinus->Tag должно быть 1)
	if (newNull)
	{
		allNewNulls = new short[numOfS * ftCh];
		postSampl = postTime / cEffDT;
		for (i = 0; i < numOfS; i++)
			for (j = 0; j < ftCh; j++)
			{
				beginSampl = (cRF[0] + j) * cRecLen;//отсчёт, с которого начинается развёртка сигнала выбранного канала (для него рассчитываем параметры)
				ReCalcNulls(&allNewNulls[(i * ftCh) + j], &sgnl[inds[i]].s[beginSampl], numOfS, postSampl);//собственно процедура расчёта нулей
				//передвинем сигнал в соответствие с новым нулём
				for (z = beginSampl; z < beginSampl + cRecLen; z++)
					sgnl[inds[i]].s[z] -= allNewNulls[(i * ftCh) + j];
			}
	}

	preSampl = preTime / cEffDT;//отсчёт, с которого "начинается пресинаптический" сигнал
	postSampl = postTime / cEffDT;//переводим пост-время в количество отсчётов
	for (i = 0; i < numOfS; i++)
		for (j = cRF[0]; j < cRF[1]; j++)
		{
			beginSampl = j * cRecLen;//отсчёт, с которого начинается развёртка сигнала выбранного канала (для него рассчитываем параметры)
			shft1 = (3 * cFtCh * inds[i]) + (3 * j);//номер первого элемента для cAmpls и cPeakInds
			shft2 = (5 * cFtCh * inds[i]) + (5 * j);//номер первого элемента для cSpans

			AmplitudeCalculate(sgnl[inds[i]].s, shft1, beginSampl, postSampl, porog, expT);//вычислим основную амплитуду
			p20p = RiseFallTime(sgnl[inds[i]].s, shft1, shft2, beginSampl, postSampl);//времена нарастания и спада основного сигнала
			if (expT == 3)//для многофазных внеклеточных сигналов
				SpecParams1(sgnl[inds[i]].s, shft1, shft2, beginSampl, preSampl, postSampl, (cAmpls[shft1] > porog), p20p);
		}

	if (newNull)
	{
		delete[] allNewNulls; allNewNulls = NULL;
	}
	sgnl = NULL; inds = NULL;
}
//---------------------------------------------------------------------------

void AmplitudeCalculate(short *signal, __int32 shft1, __int32 beginSampl, __int32 postSampl, short porog, short expTp)
{
	//вычисление амплитуды сигнала
	/*
	signal - массив с сигналом
	shft1 - номер первого элемента для cAmpls и cPeakInds
	beginSampl - отсчёт, с которого начинается развёртка сигнала выбранного канала (для него рассчитываем параметры)
	postSampl - внутренний номер отсчёта начала постсинаптической части сигнала
	porog - порог "наличия" сигнала
	expTp - тип эксперимента
	*/

	__int32 i,
			averagInterv,//половина интервала усреднения; диапазон определения "средней" амплитуды (в микросекундах)
			sampCI,//доверительный интервал в отсчётах (в МЗР)
			leftBoard,//левая граница доверительного интервала
			rightBoard;//правая граница доверительного интервала
	float meanAmpl;

	averagInterv = 10;//микросекунды
	sampCI = floor(averagInterv / cEffDT) + 1;//доверительный интервал в отсчётах (в МЗР)

	//вычисление основной амплитуды
	cAmpls[shft1] = signal[beginSampl + postSampl];//амплитудное значение (среднее на отрезке [leftBoard; rightBoard])
	cPeakInds[shft1] = postSampl;//индекс отсчёта с амплитудным значением (с абсолютным максимумом)
	for (i = postSampl; i < cRecLen; i++)
	{
		if (signal[beginSampl + i] > cAmpls[shft1])// && (signal[i - 1] <= signal[i]) && (signal[i + 1] <= signal[i]))
		{
			cAmpls[shft1] = signal[beginSampl + i];
			cPeakInds[shft1] = i;
		}
	}

	meanAmpl = 0;//расчитаем среднее в области абсолютного максимума
	leftBoard = max(postSampl, cPeakInds[shft1] - sampCI);//левая граница области
	rightBoard = min(cRecLen, cPeakInds[shft1] + sampCI);//правая граница области
	for (i = leftBoard; i < rightBoard; i++)
		meanAmpl += (float)signal[beginSampl + i];

	cAmpls[shft1] = (short)floor((meanAmpl / (float)(rightBoard - leftBoard)) + 0.5);//округление
	cAmpls[shft1] *= short(cAmpls[shft1] >= porog);//амплитудный порог наличия сигнала

	signal = NULL;
}
//---------------------------------------------------------------------------

__int32 RiseFallTime(short *signal, __int32 shft1, __int32 shft2, __int32 beginSampl, __int32 postSampl)
{
	//рассчитываем времена нарастания и спада сигналов
	/*
	signal - массив с сигналом
	shft1 - номер первого элемента для cAmpls и cPeakInds
	shft2 - номер первого элемента для cSpans
	beginSampl - номер отсчёта - начала сигнальной части записи
	postSampl - внутренний номер отсчёта начала постсинаптической части сигнала
	*/

	short ampl20, ampl50, ampl80;//переменные для поиска времён нарастания и спада
	__int32 i, z,
			ind20,//номер точки 20% амплитуды
			ind80,//номер точки 80% амплитуды
			ind50Fall;//номер точки спада 50% амплитуды
			//ind20Fall;//номер точки спада 20% амплитуды

	ind20 = 0; ind80 = 0;
	ind50Fall = 0; //ind20Fall = 0;

	//амплитудные ориентиры. Должно выполняться условие положительности amplPost (amplPost > 0)
	ampl20 = (short)floor(((float)cAmpls[shft1] * 0.2) + 0.5);//округление для положительных чисел (для отр нужно -0.5)
	ampl50 = (short)floor(((float)cAmpls[shft1] * 0.5) + 0.5);
	ampl80 = (short)floor(((float)cAmpls[shft1] * 0.8) + 0.5);

	//рассчитываем время нарастания (время нарастания = t(80%) - t(20%))
	for (i = postSampl; (i < cPeakInds[shft1]) && (i < cRecLen - 1); i += 2)
	{
		if ((signal[beginSampl + i] < ampl20) && (signal[beginSampl + i + 1] < ampl20))
			ind20 = i;
		if ((signal[beginSampl + i] < ampl80) && (signal[beginSampl + i + 1] < ampl80))
			ind80 = i;
	}

	//расчёт времен спада
	for (i = cPeakInds[shft1]; i < (cRecLen - 1); i += 2)
	{
		if ((signal[beginSampl + i] > ampl50) && (signal[beginSampl + i + 1] > ampl50))
			ind50Fall = i;//первый вариант: полуспад

		/*if ((signal[beginSampl + i] > ampl20) && (signal[beginSampl + i + 1] > ampl20))
			ind20Fall = i;//второй вариант: почти~e-спад */
	}

	if (ind80 > ind20)
	{
		cSpans[shft2] = (ind80 - ind20) * cEffDT;//время нарастания = t(80%) - t(20%) (микросекунды)
		if (ind50Fall > cPeakInds[shft1])//первый вариант: полуспад
			cSpans[shft2 + 1] = (ind50Fall - cPeakInds[shft1]) * cEffDT;//время полуспада = t(50%Fall) - t(100%) (микросекунды)
		else
			cSpans[shft2 + 1] = -1;

		/*if (ind20Fall > cPeakInds[shft1])//второй вариант: почти~e-спад
			cSpans[shft2 + 1] = (ind20Fall - cPeakInds[shft1]) * cEffDT;//время почти~е-спада = t(20%Fall) - t(100%) (микросекунды)
		else
			cSpans[shft2 + 1] = -1;*/
	}
	else
	{
		cSpans[shft2] = -1;
		cSpans[shft2 + 1] = -1;
	}
	signal = NULL;//обнуление указателя (ссылки)
	return ind20;//возвращаем номер точки с 20% амплитуды
}
//---------------------------------------------------------------------------

void SpecParams1(short *signal, __int32 shft1, __int32 shft2, __int32 beginSampl, __int32 preSampl, __int32 postSampl,
				 bool overPorog, __int32 ind20)
{
	//рассчитываем параметры вызванных внеклеточных сигналов (специальные параметры)
	/*
	signal - массив с сигналом
	shft1 - номер первого элемента для cAmpls и cPeakInds
	shft2 - номер первого элемента для cSpans
	beginSampl - номер отсчёта - начала сигнальной части записи
	preSampl - пре-время
	postSampl - пост-время
	overPorog - возникли сигнал (true, если возник)
	ind20 - если нужно будет индекс 20% амплитуды основного сигнала
	*/

	__int32 i, z,
			tB, tE,//буферные переменные для рассчёта длительностей
			psevdP,//псевдопараметр
			psevdI;//псевдоиндекс
	short locMean;//локальное среднее (на случай, если запись шла без вычета нуля)
	//float fLocMean;//ноль линия данного сигнала
	bool fB, fE;//флаги для поиска начала и конца пика

	if (preSampl <= 0)//защита от неположительных значений
		preSampl = 1;
	if (postSampl <= 0)//защита от неположительных значений
		postSampl = 1;

	locMean = 0;//ноль линия данного сигнала равно нулю

	//основная амплитуда (постсинаптическая, после красной линии - amplPost) вычисляется в AmplitudeCalculate
	//времена нарастания и спада постсинаптического ответа рассчитываем в RiseFallTime

	//-------------------------------------------------------------------------------------
	//вычисляем первую преАмплитуду ampl1NaPeak (полярность пика должна быть положительной)
	//вместо postSampl здесь можно использовать индекс отсчёта с основной амплитудой
	fB = false;//сигнализатор о пересечении ноль-линии
	cAmpls[shft1 + 1] = signal[beginSampl + preSampl];
	cPeakInds[shft1 + 1] = preSampl;
	for (i = (beginSampl + preSampl); i <= (beginSampl + postSampl); i++)
	{
		if ((signal[i] > cAmpls[shft1 + 1]) &&
			(signal[i - 1] <= signal[i]) &&
			(signal[i + 1] <= signal[i]))
		{
			cAmpls[shft1 + 1] = signal[i];//первая амплитуда пресинаптического ответа (натриевый ток) (после синей, перед красной линии) ampl1NaPeak
			cPeakInds[shft1 + 1] = i - beginSampl;//номер отсчёта с первой пресинаптической амплитудой indMax1Na
		}
	}

	cSpans[shft2 + 4] = cPeakInds[shft1 + 1] * cEffDT;//латентный период (время синаптического проведения)

	//вычисляем длительность первого преПика (натриевого тока); здесь в отсчётах
	//от пика идём вперёд и назад до обнаружения точек пересечения с нулём (предполагается,
	//что запись шла с вычитанием постоянной состовляющей)
	tB = 0;//начало первого пика (натриевый ток)
	tE = 0;//конец первого пика (натриевый ток)
	fB = false;//когда станет true, закончим поиск
	fE = false;//когда станет true, закончим поиск
	for (i = 1; ((i < (cPeakInds[shft1 + 1] - preSampl)) && (!fB) && (!fE)); i++)
	{
		if (!fE)
		{
			tE = cPeakInds[shft1 + 1] + i;//ищем "правый ноль" - конец пика
			fE = ((signal[beginSampl + tE] - locMean) < 0);
		}

		if (!fB)
		{
			tB = cPeakInds[shft1 + 1] - i;//ищем "левый ноль" - начало пика
			fB = ((signal[beginSampl + tB] - locMean) < 0);
		}
	}
	if (tE > tB)
		cSpans[shft2 + 2] = (tE - tB) * cEffDT;//длительность натриевого тока (микросекунды), tNa
	else
		cSpans[shft2 + 2] = -1;

	//-------------------------------------------------------------------------------------------
	//вычисляем вторую преАмплитуду ampl1KPeak (полярность второго пика должна быть отрицательной)
	//т.к. полярность отрицательная, то искать будем наименьшее значение на интервале между indMaxPre1 и indMaxPost
	cAmpls[shft1 + 2] = signal[beginSampl + cPeakInds[shft1 + 1]];//параметр
	cPeakInds[shft1 + 2] = cPeakInds[shft1 + 1];//индекс
	psevdP = signal[beginSampl + postSampl];//псевдопараметр
	psevdI = postSampl;//псевдоиндекс
	z = postSampl;//идём искать другой стороны
	for (i = cPeakInds[shft1 + 1]; i <= postSampl; i++)
	{
		//поиск пика К-тока (локального минимума) от пика Na-тока
		if (signal[beginSampl + i] < cAmpls[shft1 + 2])// && (signal[i - 1] >= signal[i]) && (signal[i + 1] >= signal[i]))
		{
			cAmpls[shft1 + 2] = signal[beginSampl + i];
			cPeakInds[shft1 + 2] = i;//номер отсчёта со второй пресинаптической амплитудой indMax1K
		}

		//поиск пика К-тока (локального минимума) с другой стороны (от точки postSampl)
		if (signal[beginSampl + z] < psevdP)// && (signal[z - 1] >= signal[i]) && (signal[z + 1] >= signal[i]))
		{
			psevdP = signal[beginSampl + z];
			psevdI = z;//номер отсчёта со второй пресинаптической амплитудой indMax2K
		}
		z--;
	}

	if (psevdI > cPeakInds[shft1 + 2])//получился диапазон одинаковых по уровню точек
	{
		cPeakInds[shft1 + 2] = floor(((cPeakInds[shft1 + 2] + psevdI) / 2) + 0.5);//индекс срединной точки
		cAmpls[shft1 + 2] = signal[beginSampl + cPeakInds[shft1 + 2]];//значение параметра в срединной точке
	}
	cAmpls[shft1 + 2] *= (-1);//приводим к положительному значению отрицательные K-токи

	//-----------------------------------------------------------------------------------
	if (overPorog)//сигнал (постсинаптический ответ) возник
	{
		//вычисляем синаптическую задержку (разные варианты)

		/*
		//11111_вариант_11111//от пика К-тока идём вперёд до точки начала основного (постсинаптического) сигнала
		tE = 0;//конец первого пика (натриевый ток)
		fE = false;//когда станет true, закончим поиск
		for (i = 1; ((i < postSampl) && (!fE)); i++)
		{
			if (!fE)
			{
				tE = cPeakInds[shft1 + 2] + i;//ищем "правый ноль" - конец пика
				fE = ((signal[beginSampl + tE] - locMean) > 0);
			}
		}
		if (tE > cPeakInds[shft1 + 1])
			cSpans[shft2 + 3] = (tE - cPeakInds[shft1 + 1]) * cEffDT;//синаптическая задержка
		else
			cSpans[shft2 + 3] = -1; */

		//22222_вариант_22222//от пика Na-тока до точки 20% амплитуды основной фазы (постсинаптического сигнала)
		if (ind20 > cPeakInds[shft1 + 1])
			cSpans[shft2 + 3] = (ind20 - cPeakInds[shft1 + 1]) * cEffDT;//до точки 20% амплитуды основной фазы
		else
			cSpans[shft2 + 3] = -1;

		/*
		//33333_вариант_33333//от пика Na-тока до пика основной фазы (постсинаптического сигнала)
		if (cPeakInds[shft1] > cPeakInds[shft1 + 1])
			cSpans[shft2 + 3] = (cPeakInds[shft1] - cPeakInds[shft1 + 1]) * cEffDT;//до пика основной фазы
		else
			cSpans[shft2 + 3] = -1; */
	}
	else
		cSpans[shft2 + 3] = -1;

	signal = NULL;//на всякий случай обнуляем указатель
}
//---------------------------------------------------------------------------

float IntegralCalculate(trac *signal, __int32 intgrRecLen, float voltTime, __int32 bgnSig)
{
	//вычисление амплитуды сигнала
	/*
	signal - ссылка на массив с сигналом
	intgrRecLen - длина обрабатываемой части сигнала в отсчётах (с учётом ПОСТ-времени)
	voltTime - коэффициент перевода в используемые единциы измерения (sampl2mV * discrT * chanls = [мкс * мВ])
	bgnSig - номер отсчёта, с которого начинается обрабатываемый сигнал в данной записи (с учётом многоканальности и ПОСТ-времени)
	*/

	__int32 i;
	float integr;//площадь под сигналом мВ*мс (милливольт на миллисекунду)

	integr = 0;
	for (i = bgnSig; i < (bgnSig + intgrRecLen); i++)
		integr += (float)signal->s[i];

	integr *= (voltTime * 1e-3);//переводим используемые единицы измерения (мкс * мВ)

	return integr;
}
//---------------------------------------------------------------------------

void SmoothBeforCalc(trac *smoothSig, __int32 beginSampl, __int32 preSampl, __int32 postSampl)
{
	//сглаживание сигнала перед расчётом параметров
	/*
	smoothSig - структура со сглаженным (сглаживаемым) сигналом
	beginSampl - номер отсчёта - начала сигнальной части записи
	preSampl - пре-время
	postSampl - пост-время
	*/
	short *pureSmooth;//вспомогательнный массив для сгаживания
	__int32 i, j, k, z,
            jmax,//номер лучшего решения
			timeToSm,//полупериод сглаживания (в микросекундах)
			sampToSm;//полупериод сглаживания (в отсчётах)
	float trgFn,//целевая функция
		  bestTrgFn,//лучшее значение целевой функции
		  p0, p1;//параметры полиномов

	timeToSm = 10;//дать пользователью возможность выбора
	sampToSm = floor(timeToSm / cEffDT);

	pureSmooth = new short[cRecLen];
	//копируем исходный сигнал во вспомогательный массив
	for (i = 0; i < cRecLen; i++)
		pureSmooth[i] = smoothSig->s[beginSampl + i];

	/*
	//========================================
	//сглаживание методом скользящего среднего
	for (k = 0; k < 1; k++)//степень сглаживания = 10
	{
		for (i = 0; i < cRecLen; i++)
		{
			for (j = 1; j <= sampToSm; j++)
			{
				pureSmooth[i] += smoothSig->s[beginSampl + max(i - j, 0)];
				pureSmooth[i] += smoothSig->s[beginSampl + min(i + j, recorded - 1)];
			}
			pureSmooth[i] /= ((2 * sampToSm) + 1);
		}

		for (i = 0; i < recorded; i++)
			smoothSig-s[beginSampl + i] = pureSmooth[i];//копируем исходный сигнал
	}
	*/

	//============================================================
	//сглаживание методом аппроксимации полиномами

	sampToSm *= 2;
	p0 = smoothSig->s[beginSampl + preSampl]; p1 = 0;
	for (k = preSampl; k < cRecLen; k += sampToSm)
	{
		//собственно аппроксимация (поиск p1 - угла наклона)
		p0 = p0 + (p1 * sampToSm);//сразу вычисляем параметр p0
		bestTrgFn = 1e20; jmax = 0;
		for (j = -89; j < 89; j++)
		{
			p1 = tan(j * (3.141592 / 180)) * 5e-2;//коэффициент наклона линейного участка

			trgFn = 0;
			for (i = 0; (i < sampToSm) && ((k + i) < cRecLen); i++)
				trgFn += ((smoothSig->s[beginSampl + k + i] - (p0 + (p1 * i))) *
						  (smoothSig->s[beginSampl + k + i] - (p0 + (p1 * i))));

			if (trgFn < bestTrgFn)
			{
				bestTrgFn = trgFn;
				jmax = j;//номер лучшего решения
			}
		}

		p1 = tan(jmax * (3.141592 / 180)) * 5e-2;//коэффициент наклона линейного участка

		for (i = 0; (i < sampToSm) && ((k + i) < cRecLen); i++)
			pureSmooth[k + i] = (short)(floor((p0 + (p1 * i)) + 0.5));
	}

	//переписываем результат в масси для сглаженного сигнала
	for (i = 0; i < cRecLen; i++)
    	smoothSig->s[beginSampl + i] = pureSmooth[i];

	smoothSig = NULL;
	delete[] pureSmooth; pureSmooth = NULL;
}
//---------------------------------------------------------------------------

void ReCalcNulls(short *allNulls, short *signal, __int32 nSigs, __int32 postSampl)
{
	//перерассчитывем нуль-уровни для всех сигналов
	/*
	allNulls - указатель на массив с новыми нулями
	signal - указатель на массив с сигналом
	nSigs - количество сигналов
	postSampl - точка начала сигнала (в отсчётах)
	*/

	__int32 i;
	float locMean;//среднее на заданном участке

	//====первый=вариант======
	/*
	locMean = 0;
	if (postSampl > 0)
	{
		for (i = 0; i < postSampl; i++)
			locMean += (float)signal[i];

		locMean /= (float)(postSampl);//ноль линия данного сигнала
	}

	allNulls[0] = (short)locMean;
	*/

	//====второй=вариант======
	allNulls[0] = signal[postSampl];//за ноль принимается значение в точке postSampl
}
//---------------------------------------------------------------------------

