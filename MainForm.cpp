//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <fstream.h>

#pragma hdrstop

#include "MainForm.h"
#include "Oscillograph.h"
#include "PredStart.h"
#include "ExperimentNotes.h"
#include "BuildProtocol.h"
#include "Gistograms.h"
#include "MinorChannel.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TExperiment *Experiment;

extern HINSTANCE glbhInst, glbhPrvInst;
extern LPSTR glbszCmdLine;
extern int glbiCmdLine;

DllClient LAxClient;//объект для работы с библиотеками (обладает набором методов для загрузки драйвера в память)
IADCDevice *pADC;//указатель на интерфейс драйвера
IADCUtility *pUtil;//указатель на интерфейс драйвера
IFactory *Factory;//указатель на объект библиотеки


//PStart->closeWin->Tag - управляет прерыванием сбора и функцией кнопки останова-закрытия
//PStart->GetASignal->Tag - задаёт режим старта сбора: пример сигнала или сценарий
//Graphs->CrntSig->Tag - счётчик записанных сигналов (вызванных или спонтанных в режимах №1, 2 и 3)
//PStart->BackTime->Tag - счётчик спонтанных сигналов в режиме №4
//PStart->PausSpontan->Tag - флаг разрешения записи спонтанных в режиме 4
//Graphs->sigNumUpDwn->Tag - максимальное количество сигналов (зарезервировано перед стартом)
//Experiment->DiscontinWrt->Tag - дописывать данные в drr-файл или стереть старые данные
//ExpNotes->addUMark->Tag - количество заметок по ходу эксперимента (всего присутствует на графике)
//Experiment->"кнопки вызова графиков и гистограмм"->Tag - номера элементов в массивах Graphs->allCharts и Gists->allCharts
//Graphs->"чарты с графиками"->Tag - номер элемента в массиве Graphs->allCharts
//ProtoBuild->Periods->Tag - наименьший период между импульсами стимуляции

//---------------------------------------------------------------------------
__fastcall TExperiment::TExperiment(TComponent* Owner) : TForm(Owner)
{
//	char *name = "LA2USB";
	__int32 i, z,
			index_def,//здесь будет лежать индекс по умолчанию
			listSize;//здесь храним размеры списков
	unsigned int edsI;

	defltExtPraDrr = "pra";//первое определение расширения для файлов (по умолчанию)
	plottedGrphs = 1;//обнуляем счётчик отображаемых графиков//одновременно можно вывести не более 5-ти графиков//текущий сигнал всегда виден (поэтому уже 1)
	plottedHists = 0;//обнуляем счётчик отображаемых гистограмм//не более четырёх (4) гистограмм
	Experiment->Caption = Experiment->Caption + progVer;//версия программы

	pUtil = NULL;//интерфейс драйвера
	pADC = NULL;//указатель на интерфейс драйвера

	//загружаем библиотеку и получаем интерфейс драйвера IADCUtility

	/* вариант 1 */
	pUtil = (IADCUtility*)LAxClient.LoadLibrary("LAUtility", _T("IADCUtility"), 0);

//	/* или вариант 2 */
//	Factory = (IFactory*)LAxClient.LoadRegistered("Virtual Tools LTD", "Base library", "1.0", "LAUtility");//загружаем библиотеку с интерфейсом
//	if (!Factory) return;
//	pUtil = (IADCUtility*)Factory->Create(_T("IADCUtility"), 0);//получаем интерфейс драйвера
//	if (!pUtil) return;

//	 /* вариант 3 */
//	 //загрузка с указанием пути
//	 AnsiString szPath = "C:\\Program Files\\Rudshel\\Drivers\\LA-2USB.dll";
//	 AnsiString szExportName = "LAUtility";
//	 pUtil = (IADCUtility*)LAxClient.LoadByPath(szPath.c_str(), szExportName.c_str());

	if (pUtil)
		z = pUtil->Setup("LA2USB", 1, 1, 2048);//настройка параметров платы
	if ((pUtil == NULL) || (z != 1))//устройство сбора данных не подключено или нет драйвера
	{
		DevEvents->Text = "Нет платы или драйвер";
		InducIntracell->Enabled = false;//инактивируем кнопки
		InducExtracell->Enabled = false;
		SpontanSignal->Enabled = false;
		InducSspontS->Enabled = false;
		ShowOscil->Enabled = false;
		DiscreTime->Enabled = false;
		uiGain->Enabled = false;
		uiChanAct->Enabled = false;
		uiLeadChan->Enabled = false;
		uiBase->Enabled = false;

		if (pUtil)
		{   pUtil->Release(); pUtil = NULL; }
		return;//пропускаем остальные этапы настройки оборудования и ПО
	}
	else//нужно попытаться загрузить драйвер, чтобы задать парамтры для программы
	{
		z = pUtil->Get(ADCUTILITY_GET_IADCDEVICE, &pADC);//получаем от интерфейса ADCUtility интерфейс драйвера ADCDevice
		if (pADC == NULL)
		{
			DevEvents->Text = FindErrorStrByCode(-11, 0);
			return;//пропускаем остальные этапы настройки оборудования и ПО
		}
	}

	//получаем список базовых адресов
	pADC->Get(ADC_GET_BASELIST_SIZE, &listSize);//получаем размер списка
	pADC->Get(ADC_GET_BASELIST, &baseList);//получаем сам список
	uiBase->Clear();//очищаем комбобокс
	for (i = 0; i < listSize; i++)
		uiBase->Items->Add(IntToHex(baseList[i], 3) + "h");//преобразуем адреса в шестнадцатиричную систему и добавляем их в комбобокс
	pADC->Get(ADC_GET_DEFAULTBASEINDEX, &index_def);//узнаем базовый стандартный (по умолчанию) адрес
	uiBase->ItemIndex = index_def;//выбираем его в комбобоксе
	adcBase = baseList[uiBase->ItemIndex];//сохраняем для дальнейшего использования


	//получаем список каналов DMA
	
	/*const __int32 *drqList;
	pADC->Get(ADC_GET_DRQLIST_SIZE, &listSize);//получаем размер списка
	pADC->Get(ADC_GET_DRQLIST, &drqList);//получаем сам список
//	uiDRQ->Clear();//формируем комбобокс для списка каналов DMA
//	for (i = 0; i < listSize; i++)
//		uiDRQ->Items->Add(drqList[i]);//добавляем список в комбобокс
	pADC->Get(ADC_GET_DEFAULTDRQINDEX, &index_def);//узнаем индекс по умолчанию
//	uiDRQ->ItemIndex = index_def;//выбираем его//если он равен -1, то выбираем 0
	adcDRQ = drqList[index_def];//(drqList[uiDRQ->ItemIndex] == -1) ? 0 : drqList[uiDRQ->ItemIndex];
	*/
	adcDRQ = 1;//номер канала ПДП //DRQ_NUMBER_DEFAULT = 0;//по умолчанию

	uiChanAct->Clear();//формируем список для выбора количества активных каналов
	for (i = 0; i < maxChannels; i++)
		uiChanAct->Items->Add(i + 1);
	uiChanAct->ItemIndex = -1;//по умолчанию используется 1 канал

	//получаем список коэффициентов усиления
	pADC->Get(ADC_GET_GAINLIST_SIZE, &listSize);//Получаем размер списка
	pADC->Get(ADC_GET_GAINLIST, &gainList);//Получаем сам список
	uiGain->Clear();//формируем комбобокс для списка коэффициентов усилений
	for (i = 0; i < listSize; i++)
		uiGain->Items->Add(gainList[i]);
	uiGain->ItemIndex = -1;//по умолчанию устанавливаем минимальное усиление

	pADC->Get(ADC_GET_MEMORYSIZE, &maxMemoSize);//максимальный размер памяти под буфер
	pADC->Get(ADC_GET_MAXFREQ, &maxADCFreq);//узнаём максимальную частоту дискретизации
	pADC->Get(ADC_GET_MINFREQ, &minADCFreq);//узнаём минимальную частоту дискретизации
	pADC->Get(ADC_GET_MAXAMP, &maxADCAmp);//узнаем максимальную амплитуду АЦП
	pADC->Get(ADC_GET_MINAMP, &minADCAmp);//узнаем минимальную амплитуду АЦП
	z = pADC->Get(ADC_GET_CHANNELMASK, &i);//маска для выделения номера канала
	pADC->Get(ADC_GET_DATABITS, &i);//разрядность АЦП
	pADC->Get(ADC_GET_DATASIZE, &z);//размер в байтах, необходимый для записи одного значения
	bwSh = (8 * z) - i;//величина побитового сдвига при выделении кода АЦП
	pADC->Get(ADC_GET_DATAMASK, &edsI);//маска для выделения кода АЦП при использовании данных "как есть"
	eds = (unsigned short)~edsI;//маска для выделения кода цифрового порта (0xF = 15(dec) = 0000000000001111(bin))

//	z = pADC->Get(ADC_GET_CAPABILITY_EX, &adcProps[0]);//поддерживается произвольный коэффициент усиления для каждого канала
//	z = adcProps[0] & ADC_CAPS_CHANNELDIR_GROWTH;
//	z = adcProps[0] & ADC_CAPS_GAINSPERCHANNEL;
//	z = adcProps[0] & ADC_CAPS_DIGITAL_IN;
//	z = adcProps[1] & ADC_CAPS1_USB;
//	z = adcProps[2] & ADC_CAPS2_DAC;
//	z = adcProps[2] & ADC_CAPS2_FIFO;

	//первое заполнение структуры по умолчанию
	a.m_nStartOf = ADCPARAM_START_TIMER;//старт от таймера
	a.m_nIntOf = 0;//прерывание по таймеру счетчику
	a.m_nDMABlockSize = maxMemoSize;//размер блока DMA
	a.m_nDMAMode = 16777216;//режим с авто инициализацией. //0 - режим работы DMA одиночный
	a.m_fFreqStart = maxADCFreq;//частота дискретизации
	a.m_nTimerStart = 0;//номер канала таймера для пакетов
	a.m_fFreqPack = 0;//частота дискретизации внутри пакета
	a.m_nTimerPack = 1;//номер канала таймера для дискретизации внутри пакета
	a.m_nFirstChannel = 0;//номер первого канала для сканирования (всегда 0)
	a.m_nChannelNumber = 1;//количество каналов
	a.m_nGain = 1;//коэффициент усиления

//	mChannels[0] = 0;//номера каналов
//	mGains[0] = 1;//коэффиценты усиления
//	aEx.m_pnGains = &mGains[0];//(__int32*) коэффициенты усиления для каждого канала
//	aEx.m_pnChannels = &mChannels[0];//(__int32*) номера каналов
//	aEx.m_nSize = 1;//количество каналов
//	aEx.m_nControl = ADC_DMAEX_CONTROL_MODE1;//управляющее слово (ADC_DMAEX_CONTROL_MODEDIFF)
//	aEx.m_nSyncLevel[2];//уровни синхронизации

	pADC->Get(ADC_GET_RANGE_BIPOLAR, &maxVoltage);//определяем диапазон входных напряжений
	maxVoltage = 1000 * maxVoltage;//переводим в милливольты только при определении

	z = pUtil->Get(ADCUTILITY_SET_NOCONVERT, 0);//получаем данные "как есть" без конвертации
	if (z == 0)
	{
		DevEvents->Text = FindErrorStrByCode(z, 0);
		return;
	}

	z = -1;//узнаём режим сбора данных
	pADC->Get(ADC_GET_ADCMODE, &z);//pADC->Get(ADC_SET_ADCMODE_DIFF, &z);//установка дифференциального режима
	if (z != 0)//0 - однополюсный режим, 1 - дифференциальный
	{
		DevEvents->Text = "не однополюсный режим";
		return;
	}
}
//---------------------------------------------------------------------------

void TExperiment::DefindDAC()
{
	//автовыбор канала ЦАП (для подачи стиуляционного импульса)
	__int32 i, j, z, k,
			blockReady,//готовность блока данных
			ones,//количество ненулей
			zeros,//количество нулей
			errC,//счётчик ошибок
			stim_out[2];//синхроимпульса (два значения: ноль и неноль)
	unsigned short digitPort4;//значения старших четырёх битов входного цифрового порта (поиск синхроимпульсов)
	short *drvData;
	unsigned int mask;//хранит информацию о наложении данных

	z = pUtil->Start(&a, 0);//старт сбора данных
	if (z != 1)
	{
		DevEvents->Text = "ошибка ADC_Start";
		return;
	}
	z = 0; errC = 0;//проверяем начался ли сбор данных
	while ((z == 0) && (errC <= 2))
	{
		pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
		errC++;
	}
	if (errC > 2)
	{
		DevEvents->Text = "ошибка ADC_StatusRun";
		return;
	}
	for (j = 0; j < 4; j++)//при установке нуля должент быть нуль, при единице - единица
	{
		if (j <= 1)//(Experiment->dacB->Checked)
		{
			stim_out[0] = 0x00000;//ЦАП0
			stim_out[1] = 0x00000;//ЦАП0
		}
		else
		{
			stim_out[0] = 0x10000;//ЦАП1
			stim_out[1] = 0x10000;//ЦАП1
		}
		stim_out[0] = (2048 + 1700) | stim_out[0];//напряжение на ЦАП в области логической единицы (НЕНОЛЬ = 1)
		stim_out[1] = (2048 + 100) | stim_out[1];//напряжение на ЦАП близко к логическому нулю (НОЛЬ = 0)

		ones = 0;//счётчик обнаруженных синхроимпульсов (ненулей)
		zeros = 0;//количество обнаруженных нулей
		for (k = 0; k < 2; k++)//уровень сигнала (неноль (k=0) или ноль (k=1))
		{
			z = 0; errC = 0;//устанавливаем НОЛЬ или НЕНОЛЬ на выходе ЦАП
			while ((z == 0) && (errC <= 2))
			{
				z = pADC->Get(ADC_WRITE_DAC, &stim_out[k]);//устанавливаем НОЛЬ или НЕНОЛЬ на текущем канале ЦАП
				Sleep(1);//ЦАП работает на частоте не более 5 кГц
				errC++;
			}
			if (errC > 2)
			{
				DevEvents->Text = "ошибка ADC_WRITE_DAC";
				break;
			}

			i = 0;//ищем нужный сигнал в десяти блоках
			while ((i < 10) && ((zeros < dmaBlockSize) || (ones < dmaBlockSize)))//ждём на входе АЦП нужный уровень сигнала (снимаемого с ЦАП)
			{
				blockReady = 0;
				while (blockReady == 0)
					blockReady = pUtil->GetBuffer((void*)drvData, mask);

				for (z = 0; z < dmaBlockSize; z++)
				{
					//логическое "И" drvData[z] & 0000000000001111 - выделяем последние 4 (2) бита
					digitPort4 = ((unsigned short)drvData[z]) & eds;//выделение кода ЦАП
					if (digitPort4 > 2)// > 8
						ones++;//обнаружен НЕНОЛЬ (синхроимпульс)
					else// if (digitPort4 <= 1)
						zeros++;//обнаружен НОЛЬ
				}

				z = 0; errC = 0;//освобождаем захваченный блок памяти (буфер)
				while ((z == 0) && (errC <= 2))
				{
					z = pUtil->FreeBuffer();
					errC++;
				}
				if (errC > 2)
				{
					DevEvents->Text = "ошибка FreeBuffer";
					break;//аварийное завершение разминочного сбора
				}

				i++;//количество просмотренных блоков
			}
		}
		if ((zeros >= dmaBlockSize) && (ones >= dmaBlockSize))//есть рабочий подключенный канал
			break;//выходим из цикла
	}
	z = 1; errC = 0;//стоп сбора данных
	while ((z == 1) && (errC <= 2))
	{
		pUtil->Stop();
		pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
		errC++;
	}
	if (errC > 2)
	{
		DevEvents->Text = "ошибка ADC_Stop";
		return;
	}
	if ((zeros < dmaBlockSize) || (ones < dmaBlockSize))//если цикл был прокручен до конца, значит есть проблемы с ЦАП
	{
		DevEvents->Text = "нет ЦАП!";
		InducIntracell->Enabled = false;//нет возможности "вызывать" сигнал
		InducExtracell->Enabled = false;//нет возможности "вызывать" сигнал
		InducSspontS->Enabled = false;//нет возможности "вызывать" сигнал
		Oscil->ImpPerMin->Enabled = false;//нет возможности "вызывать" сигнал
		Oscil->OneImpulse->Enabled = false;//нет возможности "вызывать" сигнал
		PStart->preRecThread->stim_out[0] = 0;
		PStart->preRecThread->stim_out[1] = 0;
	}
	else
	{
		PStart->preRecThread->stim_out[0] = stim_out[0];
		PStart->preRecThread->stim_out[1] = stim_out[1];
    }
}
//---------------------------------------------------------------------------

bool TExperiment::InitDevice()
{
	//инициализация (задание) параметров сбора данных
	__int32 z;
	//char *name = "LA2USB";
	
	if (aChanged)//были сделаны изменения параметров сбора
	{
		dmaBlockSize = a.m_nChannelNumber * ((__int32)floor(2048 / a.m_nChannelNumber));//размер блока данных кратный числу каналов
		z = pUtil->Setup("LA2USB", adcBase, adcDRQ, dmaBlockSize);//adcDRQ = 1?
		if (z != 1)
		{
			DevEvents->Text = FindErrorStrByCode(z, 0);//DevEvents->Text.Insert("initDev ", 1);
			return false;
		}
		aChanged = false;//все изменения приняты
	}

	return true;//возвращаем структуру
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::uiBaseChange(TObject *Sender)
{
	// изменили базовый адрес, узнаем на что
	adcBase = baseList[uiBase->ItemIndex];
	aChanged = true;//требуется переинициализация
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::DiscreTimeChange(TObject *Sender)
{
	//изменили время дискретизации - меняем установленную частоту дискретизации
	float dt,//время дискретизации в микросекундах
		  freq,//получаемая чатота дискретизаци
		  freqD;//желаемая частота дискретизации

	dt = StrToFloat(DiscreTime->Text);//время дискретизации в микросекундах

	if (dt < 2)
	{
		DiscreTime->Text = "2";
		return;
	}

	freqD = (float)1e6 / dt;//частота дискретизации
	a.m_fFreqStart = freqD;//частота дискретизации
	if (pADC->Init(ADC_INIT_MODE_INIT, &a, &freq) > 0)//проверим возможность установки такой частоты
		if (fabs(freqD - freq) >= 1e-3)//частоту дискретизации нужно поменять (может быть, что freq = a.m_fFreqStart)
		{
			dt = (float)1e6 / freq;//переводим разрешённую частоту в время дискретизации
			DiscreTime->Text = FloatToStrF(dt, ffFixed, 5, 2);
			return;
		}
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::StimFreqChange(TObject *Sender)
{
	//при изменении частоты стимуляции отображаем эти изменения
	float stimFrq;//частота стимуляции (Гц)
	__int32 stimPrd;//период стимуляции (мс)

	stimFrq = StrToFloat(StimFreq->Text);//частота стимуляции (Гц)
	if (stimFrq <= 0)
	{
		StimFreq->Text = FloatToStrF(0.001, ffFixed, 3, 3);
		return;
	}
	if (stimFrq > 200)
	{
		StimFreq->Text = FloatToStrF(200, ffFixed, 3, 3);
		return;
	}

	stimPrd = floor((1000 / stimFrq) + 0.5);//период стимуляции (мс) //округление для положительных чисел (для отр нужно -0.5)
	StimPeriod->OnChange = NULL;
	StimPeriod->Text = IntToStr(stimPrd);//меняем запись в поле для периода стимуляции
	StimPeriod->OnChange = StimPeriodChange;
	StimPeriodChange(this);
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::StimPeriodChange(TObject *Sender)
{
	//при изменении периода стимуляции отображаем эти изменения
	float stimFrq;//частота стимуляции (Гц)
	__int32 stimPrd;//период стимуляции (мс)

	stimPrd = StrToInt(StimPeriod->Text);//период стимуляции (мс)

	if (stimPrd < 5)//нижний предел периода стимуляции = 5 мс
	{
		StimPeriod->Text = "5";
		return;
	}
	if (stimPrd > 1e7)//верхний предел периода стимуляции = 1e7 мс (или 10000 с)
	{
		StimPeriod->Text = "10000000";
		return;
	}

	stimFrq = 1000 / (float)stimPrd;//частота стимуляции (Гц)
	StimFreq->OnChange = NULL;
	StimFreq->Text = FloatToStrF(stimFrq, ffFixed, 3, 3);//меняем запись в поле для частоты стимуляции
	StimFreq->SelStart = StimFreq->Text.AnsiPos(",") - 1;//положение курсора
	StimFreq->OnChange = StimFreqChange;
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::SaveToClick(TObject *Sender)
{
	//выбор имени файла
	Graphs->SaveDlg->Title = "Введите имя файла";
	Graphs->SaveDlg->Filter = "*." + defltExtPraDrr + "|*." + defltExtPraDrr;
	Graphs->SaveDlg->DefaultExt = defltExtPraDrr;

	if (Graphs->SaveDlg->Execute())
		FilePathName->Text = Graphs->SaveDlg->FileName;//записываем новое имя в строку filePathName
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::ShowOscilClick(TObject *Sender)
{
	__int32 z;

	if (!PStart->Visible)//если окно предстарта закрыто, можно запустить осциллограф
	{
		//для осциллографа инициализация производится здесь
		if (!InitDevice())//ошибка инициализации
			return;
		z = pUtil->AddRef();//увеличение счётчика ссылок на интерфейс
		Oscil->winoscil(pUtil, pADC);//открываем окно осциллографа
		z = pUtil->Release();//уменьшение счётчика ссылок на интерфейс
		if (z <= 0)
		{
			DevEvents->Text = "deleted";
			return;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::SpontanSignalClick(TObject *Sender)
{
	//спонтанные сигналы (тип эксперимента 1)
	__int32 z;

	if (PStart->StartRec->Enabled)//если другой эксперимент не начат, можно продолжать действие
	{
		PStart->SetControlVis(1);//делаем видимыми нужные органы управления и убираем ненужные

		pUtil->AddRef();//увеличение счётчика ссылок на интерфейс
		PStart->PredStartWindow(pUtil, pADC, 1);//выходим в окно предстарта

		z = pUtil->Release();//уменьшение счётчика ссылок на интерфейс
		if (z <= 0)
		{
			DevEvents->Text = "уже удалён!";
			return;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::InducIntracellClick(TObject *Sender)
{
	//вызванные сигналы, внутриклеточные (тип эксперимента 2)
	__int32 z;

	if (PStart->StartRec->Enabled)//если другой эксперимент не начат, можно продолжать действие
	{
		PStart->SetControlVis(2);//делаем видимыми нужные органы управления и убираем ненужные

		//выходим в окно предстарта
		pUtil->AddRef();//увеличение счётчика ссылок на интерфейс
		PStart->PredStartWindow(pUtil, pADC, 2);//внутриклеточная регистрация

		z = pUtil->Release();
		if (z <= 0)
		{
			DevEvents->Text = "уже удалён!";
			return;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::InducExtracellClick(TObject *Sender)
{
	//вызванные сигналы, внеклеточные (тип эксперимента 3)
	__int32 z;

	if (PStart->StartRec->Enabled)//если другой эксперимент не начат, можно продолжать действие
	{
		PStart->SetControlVis(3);//делаем видимыми нужные органы управления и убираем ненужные

		pUtil->AddRef();//увеличение счётчика ссылок на интерфейс
		PStart->PredStartWindow(pUtil, pADC, 3);//выходим в окно предстарта

		z = pUtil->Release();//уменьшение счётчика ссылок на интерфейс
		if (z <= 0)
		{
			DevEvents->Text = "уже удалён!";
			return;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::InducSspontSClick(TObject *Sender)
{
	//вызванные и спонтанные одновременно (тип эксперимента 4)
	__int32 z;

	if (PStart->StartRec->Enabled)//если другой эксперимент не начат, можно продолжать действие
	{
		PStart->SetControlVis(4);//делаем видимыми нужные органы управления и убираем ненужные

		pUtil->AddRef();//увеличение счётчика ссылок на интерфейс
		PStart->PredStartWindow(pUtil, pADC, 4);//вызванные + спонтанные
	
		z = pUtil->Release();//уменьшение счётчика ссылок на интерфейс
		if (z <= 0)
		{
			DevEvents->Text = "уже удалён!";
			return;
		}
    }
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::ParameterClick(TObject *Sender)
{
	//отображаем или скрываем график временного хода выбранного параметра
	short nk;//номер объекта
	bool viden;//виден ли график
	TButton *btn;

	btn = (TButton*)Sender;
	nk = btn->Tag;//номер объекта
	viden = !Graphs->allCharts[nk]->Visible;//виден ли график

	if ((viden) && (plottedGrphs < 5))
	{
		Graphs->allCharts[nk]->Visible = viden;//делаем видимым
		btn->Font->Style = TFontStyles() << fsBold;//меняем облик кнопки
		plottedGrphs++;//увеличиваем счётчик отображаемых графиков
		Graphs->GraphsTabsShow();
	}

	if (!viden)
	{
		Graphs->allCharts[nk]->Visible = viden;//делаем невидимым
		btn->Font->Style = TFontStyles();//меняем облик кнопки
		plottedGrphs--;//уменьшаем счётчик отображаемых графиков
		Graphs->GraphsTabsShow();
	}
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::HistogramClick(TObject *Sender)
{
	//отображаем или скрываем гистограмму распределения выбранного параметра
	short nk;//номер объекта
	bool viden;//виден ли график
	TButton *btn;

    btn = (TButton*)Sender;
	nk = btn->Tag;//номер объекта
	viden = !Gists->allCharts[nk]->Visible;//виден ли график

	if ((viden) && (plottedHists < 4))
	{
		Gists->allCharts[nk]->Visible = viden;//делаем видимым
		btn->Font->Style = TFontStyles() << fsBold;//меняем облик кнопки
		plottedHists++;//увеличиваем счётчик отображаемых гистограмм
		Graphs->CopyRefSignal_forGists();//запускаем рассчёт и построение графика
	}

	if (!viden)
	{
		Gists->allCharts[nk]->Visible = viden;//делаем невидимым
		btn->Font->Style = TFontStyles();//меняем облик кнопки
		plottedHists--;//уменьшаем счётчик отображаемых гистограмм
	}

	Gists->Show();
	Gists->Resized(this);
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::beginFOpenClick(TObject *Sender)
{
	//при открытии файла для редактирования запускаем окно с графиками
	//и в него добавляем кнопочки для манипуляций

	Graphs->gphOpenClick(this);//открываем файл для редактирования
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::NumOfsignalsChange(TObject *Sender)
{
	__int32 numOfS;

	numOfS = StrToInt(NumOfsignals->Text);
	if (numOfS > limitSigNum)//превышение предельного количества записываемых сигналов
	{
		numOfS = limitSigNum;//указываем максимальное количество сигналов
		NumOfsignals->Text = IntToStr(numOfS);
	}
	else if (numOfS <= 0)//нулевое или отрицательное число
	{
		numOfS = 1;//указываем минимальное количество сигналов
		NumOfsignals->Text = IntToStr(numOfS);
	}
}
//---------------------------------------------------------------------------

void TExperiment::BeginMean(short *extractLocM, __int32 fullToRec)
{
	/*
    extractLocM - указатель на массив для "пристрелочных" данных
	fullToRec - общее количество отсчётов (на всех каналах), которое необходимо записать
	*/
	__int32 i, z,
			block,//размера блока данных
			ready;
	unsigned int mask;
	short *drvData;

	drvData = NULL;

	i = 0;//общее число отсчётов

	z = pUtil->Start(&a, 0);
	if (z != 1)
	{
		DevEvents->Text = FindErrorStrByCode(z, 0);
		DevEvents->Text.Insert("BeginMean ", 1);
		return;
	}

	while (z == 0)
		pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0

	while (i < fullToRec)
	{
		ready = 0;
		while(ready == 0)
			ready = pUtil->GetBuffer((void*)drvData, mask);
		for (z = 0; ((z < dmaBlockSize) && (i < fullToRec)); z++)//+= chanNumb)
		{
			//побитовые операции для выделения данных АЦП oDrvData[i] & 1111111111110000 - выделяем первые 12 (14) бит
			extractLocM[i] = (drvData[z] >> bwSh);//выделяем код АЦП
			i++;
		}

		z = 0;//освобождаем захваченный блок памяти (буфер)
		while (z == 0)
			z = pUtil->FreeBuffer();
	}
	
	z = 1;
	while (z == 1)
	{
		pUtil->Stop();
		pUtil->Get(ADCUTILITY_STATUS_RUN, &z);//1, если идёт сбор данных, иначе 0
	}
	drvData = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::ggRefreshClick(TObject *Sender)
{
	refreshEvery->Enabled = ggRefresh->Checked;//обновлять графики каждые Х сигналов
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::SetScenarClick(TObject *Sender)
{
	ProtoBuild->ShowModal();//открываем окно для задания протокола стимуляции
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::refreshEveryChange(TObject *Sender)
{
	//реакция на изменение частоты обновления графиков и гистограмм
	__int32 refreshFreq, dig1, dig2;

	refreshFreq = StrToInt(refreshEvery->Text);//частота обновления графиков и гистограмм

	if (refreshFreq <= 1)//ограничение на частоту
	{
		refreshEvery->Text = "2";
		return;
	}

	//подравим форму слова "сигнал"
	dig2 = refreshFreq / 100;
	dig1 = refreshFreq - dig2 * 100;//две последних цифры
	dig2 = dig1 / 10;//вторая цифра
	dig1 = dig1 - (dig2 * 10);//первая цифра
	if (dig2 == 1)
		Label13->Caption = "сигналов";
	else
	{
		if (dig1 == 1)
			Label13->Caption = "сигнал";
		else if ((dig1 >= 2) && (dig1 <= 4))
			Label13->Caption = "сигнала";
		else
			Label13->Caption = "сигналов";
	}
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::TabsCheckClick(TObject *Sender)
{
	Graphs->GraphsTabsShow();//меняем таблицу на графики и наоборот
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::uiChanActChange(TObject *Sender)
{
	//меняем количество сканируемых каналов
	__int32 i,
			mi;//количество вариантов ведущих каналов

	//записываем в глобальную переменную число активных каналов
	a.m_nChannelNumber = uiChanAct->ItemIndex + 1;//кол-во активных каналов
	if (a.m_nChannelNumber <= 1)//один активный канал
	{
		RecMode->Enabled = false;//нельзя поменять (только одноканальный режим записи)
		RecMode->Checked = false;//только одноканальный режим записи
		mChnlRecModLbl->Enabled = false;//лэйбл
	}
	else
	{
		RecMode->Enabled = true;//можно поменять (только одноканальный режим записи)
		mChnlRecModLbl->Enabled = true;//лэйбл
    }
	SecondChan->RenewMinorChanWin();//обновляем вид окна с данными со второстепенных каналов

	uiLeadChan->Clear();//формируем список каналов, которые могут быть ведущими
	if (a.m_nChannelNumber == 1)
		mi = maxChannels;//количество вариантов ведущих каналов
	else
		mi = a.m_nChannelNumber;//количество вариантов ведущих каналов

	for (i = 0; i < mi; i++)
		uiLeadChan->Items->Add(i + 1);//добавляем элемент списка

	uiLeadChan->ItemIndex = 0;//по умолчанию начинаем сканировать с первого канала
	uiLeadChanChange(this);
	aChanged = true;//поменять размре блока и переинициализировать
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::uiGainChange(TObject *Sender)
{
	//копируем в глобальную переменную
	a.m_nGain = gainList[uiGain->ItemIndex];//коэффициент усиления

	//тут же поменяем пределы изменения порога амплитуды
	PStart->AmpPorogSel->Max = short(maxVoltage / a.m_nGain);//милливольты
	PStart->AmpPorogSel->Min = (-1) * PStart->AmpPorogSel->Max;//милливольты
	PStart->AmpPorogSel->Increment = (PStart->AmpPorogSel->Max / 1000) + (__int32(PStart->AmpPorogSel->Max <= 1000));//делим диапазон на 1000 шагов
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::uiLeadChanChange(TObject *Sender)
{
	if (a.m_nChannelNumber == 1)
		a.m_nFirstChannel = uiLeadChan->ItemIndex;//0;//номер первого читаемого канала
	else//плата читает n каналов начиная с m_nFirstChannel, т.е. m_nFirstChannel, m_nFirstChannel + 1 и т.д.
		a.m_nFirstChannel = 0;//номер первого читаемого канала
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::VerticAxisMode(TObject *Sender)
{
	//проценты максимального значения
	short i;
	TMenuItem* userMode;
	AnsiString title1, title2;//подписи осей

	userMode = (TMenuItem*)Sender;//указатель на объект, вызвавший данный метод
	userMode->Checked = true;//помечаем как выбранный
	Graphs->SetSeriesMultipliers();//меняем масштабные факторы для графиков в соответствии с указаниями пользователя

	//изменим подписи вертикальных осей
	if (userMode->Tag == 0)//отображаем в процентах
	{
		title1 = "напряжение, %";
		title2 = "время, %";
	}
	else//отображаем величины как есть
	{
		title1 = "напряжение, мВ";
		title2 = "время, мкс";
	}
	for (i = 1; i < 9; i++)
		if ((i == 1) || (i == 4) || (i == 6))//амплитудные величины
			Graphs->allCharts[i]->LeftAxis->Title->Caption = title1;//вертикальные оси
		else//временные величины
			Graphs->allCharts[i]->LeftAxis->Title->Caption = title2;//вертикальные оси
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::HorizontAxisMode(TObject *Sender)
{
	//время в секундах
	short i, j;
	double min_mltplr;//множитель горизонтальной оси
	TMenuItem* userMode;
	AnsiString title;//подпись оси

	userMode = (TMenuItem*)Sender;//указатель на объект, вызвавший данный метод
	userMode->Checked = true;//помечаем как выбранный

	if (userMode->Tag == 1)//время в минутах
	{
		title = "время, мин";//подпись оси
		min_mltplr = (double)1 / (double)60;//множитель горизонтальной оси
	}
	else if (userMode->Tag == 2)//номера сигналов по горизонтальной оси
	{
		title = "номер сигнала";//подпись оси
		min_mltplr = 1;//множитель горизонтальной оси
	}
	else//время в секундах //(userMode->Tag == 0) или по умолчанию
	{
		title = "время, с";//подпись оси
		min_mltplr = 1;//множитель горизонтальной оси
    }

	for (i = 1; i < 9; i++)
	{
		Graphs->allCharts[i]->BottomAxis->Title->Caption = title;//подпись оси
		for (j = 0; j < Graphs->allCharts[i]->SeriesCount(); j++)
			Graphs->allCharts[i]->Series[j]->XValues->Multiplier = min_mltplr;//множители горизонтальных осей
	}
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::CheckForKeyStimFreqEdit(TObject *Sender, char &Key)
{
	//проверка допустимости введения символа, соответствующего нажатой клавише
	//для поля частоты стимуляции

	if (((Key < '0') || (Key > '9')) && (Key != '\b') && (Key != ','))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::CheckForKeyPorogEdit(TObject *Sender, char &Key)
{
	//проверка допустимости введения символа, соответствующего нажатой клавише
	//для поля амплитудного порога для сигналов

	if (((Key < '0') || (Key > '9')) && (Key != '\b') && (Key != '-'))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::CheckForKeyOtherEdit(TObject *Sender, char &Key)
{
	//проверка допустимости введения символа, соответствующего нажатой клавише

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::ReleasMem(TObject *Sender)
{
	unsigned long int totWrt,//общее количество записанных байт
					  m_nWritten;//число записанных байт
	__int32 i;
	HANDLE hFile;//хэндл файла, в который сохраняются данные
	AnsiString anpth;//имя файла настроек в системном каталоге Windows
	bool canWrite;//найден системный каталог Windows

	//сохраним настройки программы в файл SetsElph.ini
	canWrite = false;
	for (i = 0; i < 15; i++)
	{
		anpth = char(i + 65);//пытаемся сохранить на диск С
		anpth += ":\\";
		if (GetDriveType(anpth.c_str()) == DRIVE_FIXED)//
		{
			anpth += "WINDOWS";
			if (DirectoryExists(anpth))//наидена ли папка WINDOWS (на системном диске)
			{
				canWrite = true;
				break;
			}
		}
	};

	if (canWrite)
	{
		anpth += "\\SetsElph.ini";
		hFile = CreateFile(anpth.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);//хэндл файла, в который сохраняются данные

		/*
		последовательность записи данных в файле настроек
		0 - stimPeriod        период стимуляции
		1 - numOfsignals      число сигналов для записи
		2 - LenSpont          длина спонтанного сигнала
		3 - threshold         порог детекции
		4 - miniBackTime      время назад
		5 - SignalLen         длина вызванного сигнала
		6 - recModeClick	  многоканальность
		7 - uiGain            номер коэффициента усиления
		8 - uiChanAct         номер в списке количества каналов (количество каналов)
		*/

		progPrms[0] = StrToInt(StimPeriod->Text);//0 - период стимуляции
		progPrms[1] = StrToInt(NumOfsignals->Text);//1 - число сигналов для записи
//		progPrms[2] = StrToInt(PStart->LenSpont->Text);//2 - длина спонтанного сигнала
//		progPrms[3] = StrToInt(PStart->Porog->Text);//3 - порог детекции
//		progPrms[4] = StrToInt(PStart->BackTime->Text);//4 - время назад
//		progPrms[5] = StrToInt(PStart->SignalLen->Text);//5 - длина сигнала
		progPrms[6] = (__int32)RecMode->Checked;//6 - многоканальность
		if (pUtil)
		{
			progPrms[7] = uiGain->ItemIndex;//7 - номер коэффициента усиления
			progPrms[8] = uiChanAct->ItemIndex;//8 - номер в списке количества каналов (количество каналов)
        }

		//сохраняем конфигурацию окон
		progPrms[9] = Experiment->Top;	progPrms[10] = Experiment->Left;
		progPrms[11] = Oscil->Top;	    progPrms[12] = Oscil->Left;
		progPrms[13] = PStart->Top;     progPrms[14] = PStart->Left;
		progPrms[15] = Graphs->Top;     progPrms[16] = Graphs->Left;
		progPrms[17] = Gists->Top;	    progPrms[18] = Gists->Left;
		progPrms[19] = SecondChan->Top; progPrms[20] = SecondChan->Left;
		progPrms[21] = ProtoBuild->Top; progPrms[22] = ProtoBuild->Left;
		progPrms[23] = ExpNotes->Top;   progPrms[24] = ExpNotes->Left;

		//записываем всё и сразу
		totWrt = 0;//общее количество записанных байт
		m_nWritten = 0;//число записанных байт
		for (i = 0; i < paramsToSave; i++)
		{
			WriteFile(hFile, &progPrms[i], sizeof(__int32), &m_nWritten, 0);
			totWrt += m_nWritten;
		}
		CloseHandle(hFile);//закрываем файл
	}

	if (pUtil)
	{
		pUtil->Release(); pUtil = NULL;
		pADC = NULL;
	}
	if (&LAxClient)
		LAxClient.Free();//выгрузка драйвера или библиотеки
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::FormActivate(TObject *Sender)
{
	//после окончания создания главного окна программы вызываем файл настроек
	//если найдём файл настроек SetsElph.ini, возьмём из него данные
	__int32 i,
			screenW,//ширина экрана
			screenH;//высота экрана
	unsigned long int bytesRead;
	AnsiString opnFile,//имя открываемого файла
			   anpth;//имя файла настроек в системном каталоге Windows
	bool readCor;//индикатор правильности чтения
	HANDLE hFile = NULL;//хэндл файла, в который сохраняются данные

	readCor = false;//начальное значение (если станет true, значит параметры прочитаны)

	for (i = 0; i < 15; i++)
	{
		anpth = char(i + 65);
		anpth += ":\\";
		if (GetDriveType(anpth.c_str()) == DRIVE_FIXED)//
		{
			anpth += "WINDOWS";
			if (DirectoryExists(anpth))
			{
				anpth += "\\SetsElph.ini";
				break;
			}
		}
	}

	if (FileExists(anpth.c_str()))
	{
		hFile = CreateFile(anpth.c_str(), GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		/*	последовательность записи данных в файле настроек
		0 - stimPeriod        период стимуляции;
		1 - numOfsignals      число сигналов для записи
		2 - LenSpont          длина спонтанного сигнал
		3 - threshold         порог
		4 - miniBackTime      время назад;
		5 - SignalLen         длина вызванного сигнала
		6 - recMode			  многоканальность
		7 - uiGain            номер коэффициента усиления
		8 - uiChanAct         номер в списке количества каналов
		//конфигурация окон
		9 - Experiment->Top;  10 - Experiment->Left;
		11 - Oscil->Top;      12 - Oscil->Left;
		13 - PStart->Top;     14 - PStart->Left;
		15 - Graphs->Top;     16 - Graphs->Left;
		17 - Gists->Top;      18 - Gists->Left;
		19 - SecondChan->Top; 20 - SecondChan->Left;
		21 - ProtoBuild->Top; 22 - ProtoBuild->Left;
		23 - ExpNotes->Top;   24 - ExpNotes->Left;
		*/

		for (i = 0; i < paramsToSave; i++)
		{
			progPrms[i] = 0;
			readCor = ReadFile(hFile, &progPrms[i], sizeof(__int32), &bytesRead, 0);
			if (!readCor)//ошибка чтения данных из файла настроек
				break;
		}

		CloseHandle(hFile);//закрываем файл
	}

	if (readCor)//если файл настроек прочитался правильно
	{
		StimPeriod->Text = IntToStr(progPrms[0]);//период стимуляции
		ProtoBuild->Periods->Tag = progPrms[0];//наименьший период между импульсами стимуляции
		NumOfsignals->Text = IntToStr(progPrms[1]);
//		PStart->LenSpont->Text = InToStr(progPrms[2]);
//		PStart->Porog->Text = IntToStr(progPrms[3]);
//		PStart->BackTime->Text = IntToStr(progPrms[4]);
//		PStart->SignalLen->Text = IntToStr(progPrms[5]);
		RecMode->Checked = (bool)progPrms[6];

		screenW = GetSystemMetrics(SM_CXSCREEN);//ширина экрана
		screenH = GetSystemMetrics(SM_CYSCREEN);//высота экрана
		for (i = 9; i <= 23; i += 2)//корректируем координаты окон
		{
			//проверяем y - координату левого верхнего угла окна
			if (progPrms[i] < 0)
				progPrms[i] = 0;
			else if (progPrms[i] > (screenH - 10))
				progPrms[i] = screenH - 10;

			//проверяем x - координату левого верхнего угла окна
			if (progPrms[i + 1] < 0)
				progPrms[i + 1] = 0;
			else if (progPrms[i + 1] > (screenW - 10))
				progPrms[i + 1] = screenW - 10;
		}

		Experiment->Top = progPrms[9];  Experiment->Left = progPrms[10];
		/* настраиваем остальные окна (раньше они настраивались при вызове) */
		Oscil->Top = progPrms[11];      Oscil->Left = progPrms[12];
//		PStart->Top = progPrms[13];     PStart->Left = progPrms[14];
		Graphs->Top = progPrms[15];     Graphs->Left = progPrms[16];
		Gists->Top = progPrms[17];      Gists->Left = progPrms[18];
		SecondChan->Top = progPrms[19]; SecondChan->Left = progPrms[20];
		ProtoBuild->Top = progPrms[21]; ProtoBuild->Left = progPrms[22];
		ExpNotes->Top = progPrms[23];   ExpNotes->Left = progPrms[24];
	}
	else//выставляем умолчания
	{
		StimPeriod->Text = 1000;//период стимуляции
		ProtoBuild->Periods->Tag = 1000;//наименьший период между импульсами стимуляции
		NumOfsignals->Text = 10;//количество сигналов
		progPrms[2] = 10;//PStart->LenSpnt длина спонтанного сигнала
		progPrms[3] = 10;//PStart->Porog->Text = 10;//порог
		progPrms[4] = 5;//PStart->BackTime->Text = 5;//время назад
		progPrms[5] = 20;//PStart->SignalLen->Text = 20;//длина вызванного сигнала
		progPrms[7] = 0;//uiGain->ItemIndex = 0;
		progPrms[8] = 0;//uiChanAct->ItemIndex = 0;
	}

	//формируем списки каналов и графики под каждый канал
	for (i = 0; i < maxChannels; i++)
	{
		//формируем список доступных для просмотра каналов в меню окна Graphs
		Graphs->ChannNumb[i] = NewItem("канал " + IntToStr(i + 1), NULL, false, true, //Caption, ShortCut, AChecked, AEnabled,
									   Graphs->ShowTheChannlClick, 0, "Chann" + IntToStr(i));//AOnClick, hCtx, AName
		Graphs->ChannNumb[i]->RadioItem = true;//взаимосвязанные элементы меню
		Graphs->ShowChanN->Insert(i, Graphs->ChannNumb[i]);//привязываем к меню

		Graphs->curntSigChannls[i] = new TFastLineSeries(this);//текущие сигналы на каналах (Graphs->SignalTrack)
		Graphs->sigAmpls[i] = new TLineSeries(this);//амплитуды сигналов
		PStart->exmplChannls[i] = new TFastLineSeries(this);//текущие сигналы на каналах в (PStart->StandardSgnl)

		Oscil->chkChnBoxs[i]->Checked = false;//оформляем осциллограф (флажки отображения каналов)
		Oscil->chkChnBoxs[i]->Visible = false;//оформляем осциллограф (флажки отображения каналов)
	}
	for (i = maxChannels - 1; i >= 0; i--)//добавляем графики в чарты в обратном порядке (нулевой канал будет на первом плане)
	{
		Graphs->SignalTrack->AddSeries(Graphs->curntSigChannls[i]);//добавим графики текущих сигналов
		Graphs->AmplInT->AddSeries(Graphs->sigAmpls[i]);//добавим графики амплитуд сигналов
		PStart->StandardSgnl->AddSeries(PStart->exmplChannls[i]);

		Graphs->curntSigChannls[i]->LinePen->Color = channColor[i];//цвет из массива channColor
		Graphs->sigAmpls[i]->Pointer->Visible = true;//точки видны
		Graphs->sigAmpls[i]->Pointer->Style = psCircle;//круг
		Graphs->sigAmpls[i]->SeriesColor = channColor[i];//цвет из массива channColor
		PStart->exmplChannls[i]->LinePen->Color = channColor[i];//цвет из массива channColor
	}
	Graphs->SignalTrack->RemoveSeries(Graphs->csSpecPoint);
	Graphs->SignalTrack->AddSeries(Graphs->csSpecPoint);//удаляем-добавляем, чтобы точки рисовались поверх всего
	Graphs->AmplInT->RemoveSeries(Graphs->curAmpl);
	Graphs->AmplInT->AddSeries(Graphs->curAmpl);//удаляем-добавляем, чтобы точки рисовались поверх всего

	Graphs->ChannNumb[0]->Visible = true;//нулевой канал доступен всегда
	Graphs->ChannNumb[0]->Checked = true;//выбран по умолчанию нулевой канал
	Graphs->ShowChanN->Caption = "Канал 1";//указан выбранный канал
	Oscil->chkChnBoxs[0]->Checked = true;//по умолчанию выбранн только нулевой канал

	if (pUtil)//если устройство сбора данных подключено
	{
		DiscreTime->Text = FloatToStrF((float)progPrms[2] / (float)100, ffFixed, 5, 2);
		uiGain->ItemIndex = progPrms[7];
		uiGainChange(this);//изменение коэффициента усиления
		uiChanAct->ItemIndex = progPrms[8];
		uiChanActChange(this);//изменение количества каналов
		InitDevice();//инициализация
		DefindDAC();//выбор канала ЦАП
	}

	//вписываем имя файл по умолчанию для сохранения экспермента
	anpth.Delete(4, anpth.Length());
	anpth += "Signal." + defltExtPraDrr;
	FilePathName->Text = anpth;

	//glbszCmdLine - содержит имя файла, указанного для открытия (командная строка)
	opnFile = (AnsiString)glbszCmdLine;
	if (!opnFile.IsEmpty())//если кликнули по файлу, то попытаемся его открыть
	{
		//указыаем какой файл нужно открыть
		opnFile.Delete(opnFile.Length(), 1);
		opnFile.Delete(1, 1);

		Graphs->OpenDlg->FileName = opnFile;//копируем имя файла в OpenDialog
		Graphs->ClearMemor(this);//очистим память
		Graphs->ReadDataFile(this);//вызываем функцию чтения файла, а затем - прорисовки графиков
	}

	Experiment->OnActivate = NULL;//отключаем реагирование главной формы на "активацию"
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::DiscontinWrtClick(TObject *Sender)
{
	//выбор способа записи данных на диск (установка расширения по умолчанию)
	__int32 fnmLen;//длина строки
	AnsiString subStr;

	if (DiscontinWrt->Checked)//запись сразу на жёсткий диск
		defltExtPraDrr = "drr";//устанавливаем альтернативное расширение для файлов (drr = DiRect Record)
	else//традиционный способ записи
		defltExtPraDrr = "pra";//устанавливаем традиционное расширение для файлов (pra = Physiological Reaction Acquisition)

	//изменим расширение в строке filePathName
	fnmLen = FilePathName->Text.Length();
	subStr = FilePathName->Text;
	subStr.Delete(fnmLen - 2, 3);//удаляем прежнее расширение
	subStr += defltExtPraDrr;
	FilePathName->Text = subStr;// += defltExtPraDrr;//приписываем новое расширение
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::RecModeClick(TObject *Sender)
{
	SecondChan->RenewMinorChanWin();//обновляем вид окна с данными со второстепенных каналов
}
//---------------------------------------------------------------------------

void __fastcall TExperiment::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	//перед закрытием определим, можно ли закрывать программу
	__int32 i;
	
	if (PStart->StartRec->Enabled)//если не идёт сбор данных, можно закрывать программу
	{
		Graphs->ClearMemor(this);//удаление всех старых данных из памяти
		Graphs->SignalTrack->RemoveAllSeries();//удаляем графики
		PStart->StandardSgnl->RemoveAllSeries();//удаляем графики
		for (i = 0; i < maxChannels; i++)
		{
			delete(Graphs->curntSigChannls[i]); Graphs->curntSigChannls[i] = NULL;//графики сигналов на каналах
			delete(Graphs->sigAmpls[i]); Graphs->sigAmpls[i] = NULL;//графики амплитуд сигналов
			delete(PStart->exmplChannls[i]); PStart->exmplChannls[i] = NULL;//графики сигналов на каналах
		}
		CanClose = true;//разрешаем выйти из программы (сбор данных не идёт)
	}
	else
		CanClose = false;//пока нельзя закрыть программу, поскольку идёт сбор данных

	if (PStart->preRecThread)//поток сбора данных
	{
		PStart->preRecThread->Terminate();//удаляем поток сбора данных
		PStart->preRecThread = NULL;
	}
}
//---------------------------------------------------------------------------

