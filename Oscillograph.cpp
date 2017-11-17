//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <math.h>
#include <algorith.h>
#include <process.h>
#include <time.h>

#pragma hdrstop

#include "MainForm.h"
#include "Oscillograph.h"
#include "PredStart.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "cspin"
#pragma resource "*.dfm"
TOscil *Oscil;

IADCDevice *osc_pADC = NULL;//указатель на интерфейс драйвера
IADCUtility *osc_pUtil = NULL;//указатель на интерфейс
ADCParametersDMA osc_a;//структура параметро устройства

__int32 oBlockSize,//размер блока данных
		grpLeft,//левая граница окна рисования в осциллографе
		grpTop,//верхняя граница окна рисования в осциллографе
		grpWid,//ширина окна рисования в осциллографе
		grpHeig,//высота окна рисования в осциллографе
        drawStep,//шаг по отсчётам при рисовании (исключаем избыточные точки)
		vScan,//вертикальная развёртка
		nulLine,//вертикальная координата ноль-линни в окне
		nulLineShift,//смещение нуль-линии
        nporog,//уровень порога детекции спонтанных сигналов
		oStim_out[2],//значение амплитуды синхроимпульса
		oToRec,//длина записи по одному каналу
		oChanNumb,//число используемых каналов
		recStep,//шаг записи в файл
		oGain,//коэффициент усиления
		oshowSamp,// перевод в отсчёты при данной частоте дискретизации
		impPM,//текущее значение частоты стимуляции
		visualTime;//величина временного окна осциллографа (в миллисекундах)
float oDiscrTime,//пользовательское время дискретизации (микросекунды)
	  minDiscrTime;//минимальное время дискретизации (микросекунды)
short oConstCompMinus,//вычитать ли постоянную составляющую
	  oPolarity;
HANDLE ContinRecFile;

//---------------------------------------------------------------------------
__fastcall TOscil::TOscil(TComponent* Owner)
	: TForm(Owner)
{
	//osciloscope
	chkChnBoxs[0] = checkChann0;//копии указателей на чекбоксы
	chkChnBoxs[1] = checkChann1;
	chkChnBoxs[2] = checkChann2;
	chkChnBoxs[3] = checkChann3;
	chkChnBoxs[4] = checkChann4;
	chkChnBoxs[5] = checkChann5;
	chkChnBoxs[6] = checkChann6;
	chkChnBoxs[7] = checkChann7;
}
//---------------------------------------------------------------------------

void __fastcall TOscil::winoscil(IADCUtility* opUtil, IADCDevice *opADC)
{
	__int32 z;
	float freqDiscr;

	osc_pUtil = opUtil; opUtil = NULL;
	osc_pADC = opADC; opADC = NULL;
	osc_a = Experiment->a;

	oBlockSize = Experiment->dmaBlockSize;//размер блока данных, получаемых методом GetBuffer
	oChanNumb = osc_a.m_nChannelNumber;
	oGain = osc_a.m_nGain;//коэффициент усиления
	oDiscrTime = StrToFloat(Experiment->DiscreTime->Text);//пользовательское время дискретизации
	minDiscrTime = 1e6 / Experiment->maxADCFreq;//минимальное время дискретизации (микросекунды)
	osc_a.m_fFreqStart = Experiment->maxADCFreq;//устанавливаем максимальную частоту дискретизации
	recStep = floor(oDiscrTime / minDiscrTime);//шаг записи в файл
	for (z = 0; z < maxChannels; z++)
		if (z < oChanNumb)
			chkChnBoxs[z]->Visible = true;//доступны только oChanNumb первых канала
		else
		{
			chkChnBoxs[z]->Visible = false;//остальные каналы не испльзуются
			chkChnBoxs[z]->Checked = false;
		}
	vScale->Max = Experiment->maxVoltage;//максимальный делитель вертикальной развёртки
	vScale->Position = Experiment->maxVoltage;//начальный делитель вертикальной развёртке
	vScale->Frequency = vScale->Max / 20;
	timeScale->Min = 50;//минимальное "окно" осциллографа +floor(float(minDiscrTime * oChanNumb) / 1000)
	MinTmpWinLbl->Caption = IntToStr(timeScale->Min) + " мс";//отображаем минимальную возможную длину окна
	CrntTmpWinLbl->Caption = IntToStr(timeScale->Position) + " мс";//отображаем выставленную величину временного окна
	NulShift->Min = - Experiment->maxVoltage;//низшее положение нулевой линии
	NulShift->Max = Experiment->maxVoltage;//высшее положение нулевой линии
	NulShift->Position = 0;//начальное положение ноль-линии
	Porog->MinValue = -Experiment->maxVoltage;//минимальный порог детекции сигналов
	Porog->MaxValue = Experiment->maxVoltage;//максимальный порог детекции сигналов
	Porog->Value = StrToInt(PStart->Porog->Text);//порог детекции сигналов (копируем)
	Alternating->Checked = PStart->NulMinus->Checked;//переменный сигнал
	ConstCompon->Checked = !PStart->NulMinus->Checked;//постоянная состовляющая

	oConstCompMinus = (short)(Alternating->Checked);//вычитать ли постоянную составляющую
	oPolarity = (-1) + (2 * (__int32)(!invert->Checked));//полярность сигнала

	_beginthread(StartOsc, 8192, NULL);//старт потока рисования в осциллографе

	Oscil->ModalResult = mrNone;//по умолчанию осциллограф закрывается без результирующего значения
	Oscil->ShowModal();//активация окна осциллографа
}
//---------------------------------------------------------------------------

void StartOsc(void *pParam)
{
	__int32 i, j, z,
			grafik[maxChannels][2],//значения напряжения на каналах, подправленные для рисования
			grafikBack[maxChannels][2],//прежние значения grafik
			recorded,//счётчик прочитанных отсчёты (одного канала)
			errC,//счётчки ошибок
			tctPperiod,//миллисекунд за период стимуляции
			oCountlm,//количество отсчётов принятых для вычисления локального среднего (ограничено)
			blkready,//готовность блока данных
			recSchet,//увеличиваем счётчик записи в файл
			drawSchet,//счётчк прорисовки
			sampsMean,//количество отсчётов принятых для вычисления локального среднего
			readOrder[maxChannels];//массив с последовательностью чтения каналов
	unsigned int mask;//хранит информацию о наложении данных
	unsigned long dwNumWritten;
	float gr_pnt[maxChannels],//значения напряжения на каналах, не подправленные для рисования
		  samplTomV,//коэффициент перевода амплитуд из отсчётов АЦП в милливольты
		  waitTime;//длительность паузы (мкс) перед отключением синхроимпульса
	short *oDrvData,
		  *extrLocMean,//ссылка на массив выборки (extract) локального среднего
		  convData[maxChannels],//выделенный аналоговый вход АЦП
		  userData[maxChannels],//данные с учётом настроек пользователя
		  bwSh;//величина побитового сдвига при выделении кода АЦП
	TRect rectClr;
	TCanvas *drawOn;//указатель на канву, на которой рисуем (ускорение процесса)
	TPen *canvasPen;//указатель на карандаш канвы (ускорение процесса)
	clock_t curMoment,//текущий моментв времени (с точность до миллисекунд)
			nextImpuls;//момент подачи импульса стимуляции (синхроимпульса)
	unsigned short digitPort4,//значения старших четырёх битов входного цифрового порта
				   eds;//маска для выделения кода цифрового порта
	double locMean[maxChannels];//локальное среднее (нужно для нивелирования постоянной составляющей входного сигнала)
	char buf;
	__int64 curnttic,//текущий такт процессора
			tpp,//тактов за перод синхроимпульса
			tp1,
			ticps;//тактов в секунду (частота процессора)

	oDrvData = NULL;
	Oscil->FormResize(NULL);//определяем границы рисования
	Oscil->timeScaleChange(NULL);//
	Oscil->vScaleChange(NULL);//
	Oscil->FormResize(NULL);//определяем границы рисования
	Oscil->PorogChange(NULL);//
	drawSchet = drawStep;//счётчки прорисовки
	recSchet = recStep;//счётчик записи в файл
	recorded = 0;//счётчик прочитанных отсчётов (одного канала)
	blkready = 0;// готовность блока данных
	nextImpuls = clock();//время следующего импульса (секунд от начала работы программы)
	digitPort4 = 0;
	drawOn = Oscil->Canvas;//указатель на канву, на которой рисуем (ускорение процесса)
	canvasPen = drawOn->Pen;//указатель карандаш канвы (ускорение процесса)
	if (PStart->preRecThread->stim_out[0] <= 0)//ориентир для модуля RecordThread->StartRec и Oscil->StartOsc
		eds = 0;//программа не будет реагировать на шум ЦАП
	else
	{
		eds = Experiment->eds;//маска для выделения кода цифрового порта (0xF = 15(dec) = 0000000000001111(bin))
		oStim_out[0] = PStart->preRecThread->stim_out[0];
		oStim_out[1] = PStart->preRecThread->stim_out[1];
	}
	bwSh = Experiment->bwSh;//величина побитового сдвига при выделении кода АЦП

	z = 0;//задаём порядок чтения каналов (циклический перебор)
	for (i = Experiment->uiLeadChan->ItemIndex; i < oChanNumb; i++)
	{
		readOrder[z] = i;
		z++;
	}
	for (i = 0; i < Experiment->uiLeadChan->ItemIndex; i++)
	{
		readOrder[z] = i;
		z++;
	}

	Oscil->timeScaleChange(NULL);//предварительно определяем oToRec
	oPolarity = -1 + (2 * __int32(!Oscil->invert->Checked));
	samplTomV = Experiment->maxVoltage / (float)Experiment->maxADCAmp;//коэффициент перевода амплитуд из отсчётов АЦП в милливольты
	for (i = 0; i < maxChannels; i++)
	{
		grafik[i][1] = 0;//y-координата
		grafikBack[i][0] = grpLeft;//x-координата
		grafikBack[i][1] = 0;//y-координата
	}

	//== вычисляем начальное среднее ==
	sampsMean = (__int32)floor((float(baseLine) / (minDiscrTime * float(oChanNumb))) + 0.5);
	extrLocMean = new short[sampsMean * oChanNumb];
	Experiment->BeginMean(extrLocMean, sampsMean * oChanNumb);

	for (z = 0; z < oChanNumb; z++)
	{
		locMean[z] = 0;
		for (i = 0; i < sampsMean; i++)
			locMean[z] += (double)extrLocMean[i * oChanNumb + readOrder[z]];
		locMean[z] /= (double)sampsMean;
	}
	oCountlm = 0;

	z = osc_pUtil->Start(&osc_a, 0);//старт сбора
	if (z != 1)
	{
		Experiment->DevEvents->Text = (FindErrorStrByCode(z, 0));
		Experiment->DevEvents->Text.Insert("Oscil ", 1);
		return;
	}
	z = 0;
	while (z == 0)
		osc_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0

	z = 0; errC = 0;//устанавливаем НОЛЬ на выходе ЦАП
	while ((z == 0) && (errC <= 2))
	{
		z = osc_pADC->Get(ADC_WRITE_DAC, &oStim_out[1]);
		Sleep(1);
		errC++;
	}

	waitTime = min(float(500), minDiscrTime);//длительность паузы (мкс) перед отключением синхроимпульса
	errC = 0;//индикатора зависания
	while ((!QueryPerformanceFrequency((LARGE_INTEGER *)&ticps)) && (errC <= 2))//частота процессора
		errC++;
	tpp = (float)ticps * (waitTime / (float)1e6);//тактов за перод синхроимпульса

	Oscil->Tag = 1;//разрешаем рисовать в осциллографе
	while (Oscil->Tag > 0)//цикл рисования (остановится только при закрытии осциллографа)
	{
		//подаём стимуляционный импульс
		if ((Oscil->OneImpulse->Tag == 1) || (impPM > 0))//если заданая стимуляция (один или более импульсов)
		{
			curMoment = clock();//текущий момент времени
			if ((curMoment >= nextImpuls) || (Oscil->OneImpulse->Tag == 1))//пора подавать синхроимпульс
			{
				z = 0; errC = 0;//устанавливаем НЕНОЛЬ на выходе ЦАП
				while ((z == 0) && (errC <= 2))
				{
					z = osc_pADC->Get(ADC_WRITE_DAC, &oStim_out[0]);
					errC++;
				}

				if (impPM > 0)
				{
					tctPperiod = 60000 / impPM;//миллисекунд в периоде
					nextImpuls = curMoment + (clock_t)tctPperiod;//время следующего импульса
				}
				Oscil->OneImpulse->Tag = 0;//один импульс подан (снимаем флаг)

				QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);
				//делаем пауза перед отключением синхроимпульса
				tp1 = curnttic;
				while ((curnttic - tp1) < tpp)
					QueryPerformanceCounter((LARGE_INTEGER *)&curnttic);

				z = 0; errC = 0;//устанавливаем НОЛЬ на выходе ЦАП
				while ((z == 0) && (errC <= 2))
				{
					z = osc_pADC->Get(ADC_WRITE_DAC, &oStim_out[1]);
					errC++;
				}
			}
		}

		blkready = osc_pUtil->GetBuffer((void*)oDrvData, mask);//запрашиваем блок данных из буфера DMA
		if (blkready != 0)//блок данных готов
		{
			for (i = 0; i < oBlockSize; i += oChanNumb)
			{
				if (digitPort4 == 0)//ждём новый синхроимпульс
					digitPort4 = oDrvData[i] & eds;//eds - маска (unsigned short)

				for (z = 0; z < oChanNumb; z++)
				{
					convData[z] = oDrvData[i + readOrder[z]] >> bwSh;//выделяем код АЦП (старшие 12 (14) бит)
					userData[z] = oPolarity * (convData[z] - short(locMean[z] * oConstCompMinus));

					//постоянно корректируем локальное среднее
					locMean[z] += (double(convData[z] - extrLocMean[oCountlm * oChanNumb + readOrder[z]]) / (double)sampsMean);
					extrLocMean[oCountlm * oChanNumb + readOrder[z]] = convData[z];//массив локальных значений
				}
				oCountlm++;
				if (oCountlm >= sampsMean)
					oCountlm = 0;

				if ((Oscil->ContRec->Tag == 1) && (recSchet >= recStep))//если включена запись, записываем всё подряд
				{
					WriteFile(ContinRecFile, (void*)&userData[0], sizeof(short) * oChanNumb, &dwNumWritten, NULL);//записываем отдельно только аналоговые данные
					recSchet = 0;
				}

				if (drawSchet >= drawStep)//рисование
				{
					for (z = 0; z < oChanNumb; z++)
					{
						gr_pnt[z] = oPolarity * ((float)convData[z] - (locMean[z] * (float)oConstCompMinus)) * samplTomV;//значения напряжения на каналах, не подправленные для рисования
						grafik[z][0] = grpLeft + __int32(float(recorded * grpWid) / oToRec);//x-координата
						grafik[z][1] = floor(nulLine + (((-1.0) * (gr_pnt[z] * grpHeig)) / vScan) + 0.5);//значения напряжения на каналах, подправленные для рисования
					}

					drawOn->Lock();//заблокируем канву

					canvasPen->Color = clBtnFace;//цвет фона для затирания старого
					if (digitPort4 != 0)//при появлении синхроимпульса
					{
						recorded = oToRec;//переходим к рисованию с начала окна
						rectClr = Rect(grafik[0][0], grpTop, grpLeft + grpWid, grpTop + grpHeig);
						drawOn->FillRect(rectClr);//затираем старое
						digitPort4 = 0;
					}
					z = min(grafik[0][0] + 10, grpLeft + grpWid);//x - координата
					drawOn->MoveTo(z, grpTop);//затираем старое
					drawOn->LineTo(z, grpTop + grpHeig);//затираем старое

					//рисуем сигналы на каналах
					if (recorded >= oToRec)//дошли до конца окна осциллографа
					{
						rectClr = Rect(grpLeft, grpTop, min(grpLeft + 10, grpLeft + grpWid), grpTop + grpHeig);
						drawOn->FillRect(rectClr);//затираем старое

						for (z = 0; z < oChanNumb; z++)
						{
							if (Oscil->chkChnBoxs[z]->Checked)
							{
								//готовим курсор для рисования графика (grafik)
								canvasPen->Color = channColor[z];
								drawOn->MoveTo(grpLeft, grafikBack[z][1]);
								drawOn->LineTo(grpLeft, grafik[z][1]);
							}
							grafikBack[z][0] = grpLeft;
							grafikBack[z][1] = grafik[z][1];
						}
						recorded = 0;
					}
					else
						for (z = 0; z < oChanNumb; z++)
						{
							if (Oscil->chkChnBoxs[z]->Checked)
							{
								//готовим курсор для рисования данных с канала z
								canvasPen->Color = channColor[z];
								drawOn->MoveTo(grafikBack[z][0], grafikBack[z][1]);
								drawOn->LineTo(grafik[z][0], grafik[z][1]);
								if (gr_pnt[z] > nporog)//при наличии всплеска по амплитуде
									drawOn->LineTo(grafik[z][0], nulLine);//закрашиваем область под всплеском
							}
							grafikBack[z][0] = grafik[z][0];
							grafikBack[z][1] = grafik[z][1];
						}

					//рисуем порог-линию
					canvasPen->Color = clYellow;//жёлтая пороговая линия
					drawOn->MoveTo(grpLeft, nulLine + ((-1.0) * nporog * grpHeig) / vScan);
					drawOn->LineTo(grpLeft + grpWid, nulLine + ((-1.0) * nporog * grpHeig) / vScan);

					//рисуем ноль-линию
					canvasPen->Color = clSkyBlue;//небесноголубая ноль-линия
					drawOn->MoveTo(grpLeft, nulLine);
					drawOn->LineTo(grpLeft + grpWid, nulLine);

					drawOn->Unlock();//разблокируем канву

					drawSchet = 0;
				}
				recorded++;//увеличивем счётчит прочитанных отсчётов (одного канала)
				drawSchet++;//увеличиваем счётчик прорисовки
				recSchet++;//увеличиваем счётчик записи в файл
			}
			z = 0;//освобождаем захваченный блок памяти (буфер)
			while (z == 0)
				z = osc_pUtil->FreeBuffer();
		}
	}

	z = 1;//останавливаем сбор данных
	while (z == 1)
	{
		osc_pUtil->Stop();
		osc_pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
	}

	oDrvData = NULL;
	delete[] extrLocMean; extrLocMean = NULL;

	Oscil->Tag = -2;//выход из потока рисования в осциллографе
}
//---------------------------------------------------------------------------

void __fastcall TOscil::timeScaleChange(TObject *Sender)
{
	//меняем временную развёртку
	visualTime = Oscil->timeScale->Position;//величина временного окна осциллографа (в миллисекундах)
	CrntTmpWinLbl->Caption = IntToStr(visualTime) + " мс";//отображаем выставленную величину временного окна
	oToRec = floor(float(visualTime * 1e3) / (minDiscrTime * oChanNumb)) + 1;//длина записи по одному каналу (отсчёты)
	drawStep = floor(float(oToRec) / grpWid);//шаг по отсчётам при рисовании (исключаем избыточные точки)
}
//---------------------------------------------------------------------------

void __fastcall TOscil::onOscClose(TObject *Sender)
{
	//закрываем объект
	if (Oscil->Tag > 0)
		Oscil->Tag = -1;//остановка цикла рисования в осциллографе

	Experiment->DevEvents->Text = "wait for end...";
	while (Oscil->Tag >= -1){};//ждём завершения цикла рисования в осциллографе
	Experiment->DevEvents->Text = "OK";

	if (ContRec->Tag == 1)//если запись не остановлена пользователем
		ContRecClick(this);//завершаем запись файла автоматически
}
//---------------------------------------------------------------------------

void __fastcall TOscil::acceptParamtrClick(TObject *Sender)
{
	PStart->Porog->Text = Porog->Text;
	PStart->NulMinus->Checked = Alternating->Checked;
	PStart->Invert->Checked = invert->Checked;
	Oscil->ModalResult = mrYes;//закрываем окно
}
//---------------------------------------------------------------------------

void __fastcall TOscil::PorogChange(TObject *Sender)
{
	nporog = Porog->Value * oGain;//уровень порога детекции спонтанных сигналов
}
//---------------------------------------------------------------------------

void __fastcall TOscil::invertClick(TObject *Sender)
{
	oPolarity = (-1) + (2 * (__int32)!invert->Checked);//меняем видимую полярность сигнала
}
//---------------------------------------------------------------------------

void __fastcall TOscil::AlternatingClick(TObject *Sender)
{
	oConstCompMinus = (short)Alternating->Checked;//вычитать постоянную составляющую
}
//---------------------------------------------------------------------------

void __fastcall TOscil::ConstComponClick(TObject *Sender)
{
	oConstCompMinus = (short)!ConstCompon->Checked;//не вычитать постоянную составляющую
}
//---------------------------------------------------------------------------

void __fastcall TOscil::FormResize(TObject *Sender)
{
	//при изменении размера окна корректируем параметры рисования

	grpLeft = Oscil->vScale->Left + Oscil->vScale->Width;//левая граница окна рисования в осциллографе
	grpTop = 0;//верхняя граница окна рисования в осциллографе
	grpWid = Oscil->NulShift->Left - grpLeft;//ширина окна рисования в осциллографе
	grpHeig = Oscil->timeScale->Top - grpTop;//высота окна рисования в осциллографе
	nulLine = floor(grpTop + (grpHeig / 2) + ((nulLineShift * grpHeig) / vScan) + 0.5);//вертикальная координата ноль-линни в окне
	drawStep = floor((float)oToRec / grpWid);//шаг по отсчётам при рисовании (исключаем избыточные точки)
}
//---------------------------------------------------------------------------

void __fastcall TOscil::RightKeyPress1(TObject *Sender, char &Key)
{
	//проверка допустимости введения символа, соответствующего нажатой клавише

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TOscil::RightKeyPress2(TObject *Sender, char &Key)
{
	if (((Key < '0') || (Key > '9')) && (Key != '\b') && (Key != '-'))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TOscil::OneImpulseClick(TObject *Sender)
{
	//при нажатии даём один импульс стимуляции
	Oscil->OneImpulse->Tag = 1;
}
//---------------------------------------------------------------------------

void __fastcall TOscil::ContRecClick(TObject *Sender)
{
	//старт-стоп непрерывной записи потока данных на диск
	__int32 bufInt;//буфер
	unsigned long amountOfBytes,//количество байт, записываемых данном блоке
				  bytesWritten;//число записанных байт
	AnsiString recordPrm;//информация о программе и дате создания файла
	TDateTime CurrentDateTime;//текущие дата и время
	float sampls2mV;//коэффициент перевода амплитуд из отсчётов АЦП в (милли)вольты

	if (ContRec->Tag == 0)//если запись не включена
	{
		if (oDiscrTime >= (20 * minDiscrTime))//пользовательское время дискретизации больше нижнего предела
		{
			//выбор имени файла
			Graphs->SaveDlg->Title = "Введите имя файла";//диалоговое сообщение
			Graphs->SaveDlg->Filter = "*.bin|*.bin";//фильтр отображения сигналов
			Graphs->SaveDlg->DefaultExt = "bin";//расширение по умолчанию

			if (Graphs->SaveDlg->Execute())
			{
				CurrentDateTime = Now();//текущие дата и время
				//создаём файл для непрерывной записи
				ContinRecFile = CreateFile(Graphs->SaveDlg->FileName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE,
										   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				//формируем заголовок файла
				recordPrm = "ElphAcqu v" + progVer + "\nmade";//~18 символов
				recordPrm += CurrentDateTime.DateTimeString().c_str();//19 символов

				amountOfBytes = sizeof(char) * recordPrm.Length();//количество байт, записываемых в данном блоке
				amountOfBytes += (3 * sizeof(float)) + (4 * sizeof(__int32));//количество байт, записываемых в данном блоке

				//вписываем заголовочные данные
				WriteFile(ContinRecFile, (void*)recordPrm.c_str(), sizeof(char) * recordPrm.Length(), &bytesWritten, NULL);
				amountOfBytes -= bytesWritten;

				//переменные, уникальные для данной записи (т.е. все необходимые параметры)
				/* последовательность записи обязательных параметров
					sampls2mV ----(1) - коэффициент перевода амплитуд из отсчётов АЦП в (милли)вольты
					oChanNumb ----(2) - количество используемых каналов
					minDiscrTime -(3) - минимальное (истинное) время дискретизации
					oDiscrTime ---(4) - пользовательское время дискретизации
					maxADCAmp ----(5) - максимальная амплитуда АЦП (в отсчётах)
				*/

				WriteFile(ContinRecFile, (void*)&oChanNumb, sizeof(__int32), &bytesWritten, NULL);//2//количество используемых каналов
				amountOfBytes -= bytesWritten;
                WriteFile(ContinRecFile, (void*)&oGain, sizeof(__int32), &bytesWritten, NULL);//8//коэффициент усиления
				amountOfBytes -= bytesWritten;
				WriteFile(ContinRecFile, (void*)&minDiscrTime, sizeof(float), &bytesWritten, NULL);//3//минимальное (истинное) время дискретизации
				amountOfBytes -= bytesWritten;
				WriteFile(ContinRecFile, (void*)&oDiscrTime, sizeof(float), &bytesWritten, NULL);//4//пользовательское время дискретизации (эффективное, для записи)
				amountOfBytes -= bytesWritten;
				WriteFile(ContinRecFile, (void*)&Experiment->maxADCAmp, sizeof(__int32), &bytesWritten, NULL);//5//максимальная амплитуда АЦП (в отсчётах)
				amountOfBytes -= bytesWritten;
				WriteFile(ContinRecFile, (void*)&Experiment->maxVoltage, sizeof(float), &bytesWritten, NULL);//6//диапазон допустимых входных напряжений (милливольты)
				amountOfBytes -= bytesWritten;
				bufInt = Experiment->uiLeadChan->ItemIndex;
				WriteFile(ContinRecFile, (void*)&bufInt, sizeof(__int32), &bytesWritten, NULL);//7//ведущий канал (основной сигнал)
				amountOfBytes -= bytesWritten;

				if (amountOfBytes != 0)
				{
					Experiment->DevEvents->Text = "ошибка начальной записи";//аварийный выход, видимо, можно не делать
					CloseHandle(ContinRecFile);//завершаем запись файла
					return;
				}

				ContRec->Caption = "Стоп";
				ContRec->Tag = 1;//запись данных в файл включена
			}
		}
		else
			Experiment->DevEvents->Text = "мин. вр. дискр. 40мкс";
	}
	else
	{
		ContRec->Tag = 0;//остановить запись, готов к новой записи
		CloseHandle(ContinRecFile);//завершаем запись файла
		ContRec->Caption = "Запись";
	}
}
//---------------------------------------------------------------------------

void __fastcall TOscil::NulShiftChange(TObject *Sender)
{
	nulLineShift = Oscil->NulShift->Position;//смещение ноль-линии
	nulLine = floor(grpTop + (grpHeig / 2) + ((nulLineShift * grpHeig) / vScan) + 0.5);//вертикальная координата ноль-линни в окне
}
//---------------------------------------------------------------------------

void __fastcall TOscil::vScaleChange(TObject *Sender)
{
	vScan = Oscil->vScale->Position;//изменение вертикальное развёртки
	NulShiftChange(this);//скорректируем положение нуль-линии
}
//---------------------------------------------------------------------------

void __fastcall TOscil::ImpPerMinChange(TObject *Sender)
{
	impPM = ImpPerMin->Value;//текущее значение частоты стимуляции
}
//---------------------------------------------------------------------------
