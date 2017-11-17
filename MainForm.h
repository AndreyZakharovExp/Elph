//$$---- Form HDR ----
//---------------------------------------------------------------------------

#ifndef MainFormH
#define MainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>

#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Chart.hpp>
#include <Series.hpp>
#include <TeEngine.hpp>
#include <TeeProcs.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
#include "IADCDevice.h"
#include "IADCUtility.h"
#include "IFactory.h"
#include "DllClient.h"
#include "REGKEY\VT_Drivers.ri"
#include "REGKEY\VT_Library.ri"
#include "ADC_Errors.h"

//---------------------------------------------------------------------------
//const char *name = "LA2USB";
const __int32 maxChannels = 8,//максимальное число используемых каналов
			  baseLine = 3e5,//длина отрезка в микросекундах, на котором опред-ся лок. среднее
			  minFreeTime = 500,//минимальная длина "свободного" времени (мс), при которой допускается рисование
			  paramsToSave = 25,//количество сохраняемых параметров настройки программы
			  limitSigLen = 1e5,//ограничение длины сигнала (миллисекунды)
			  limitSigNum = 5e5;//ограничение количества собираемых сигналов (штук)
const TColor channColor[] = {clRed, //канал 0 //константный массив с цветами каналов
							 clGreen, //канал 1
							 clBlue, //канал 2
							 clBlack, //канал 3
							 clYellow, //канал 4
							 clWhite, //канал 5
							 clSkyBlue, //канал 6
							 clNavy};//канал 7
const AnsiString progVer = "5.0";//версия программы

//---------------------------------------------------------------------------

struct trac //структура с развёртками сигналов
{
	short *s;//развёртки сигналов со всех каналов (либо отдельные отсчёты для не многоканального режима)
	float appear;//время возникновения сигнала-синхроимпульса (секунды от начала эксперимента)
};
struct sPr //структура с вычисляемыми параметрами сигналов
{
	short *ampls;//все пиковые значения (амплитуды)
			  //ampls[0] = amplPost - амплитуда основная - постсинаптическая (после красной линии)
			  //ampls[1] = ampl1NaPeak - первая амплитуда пресинаптического ответа (натриевый ток) (после синей, перед красной линии)
			  //ampls[2] = ampl2KPeak - вторая амплитуда пресинаптического ответа (калиевый ток) (после синей, перед красной линии)
	__int32 *peakInds,//все номера отсчётов с пиковыми значениями
				//peakInds[0] = indMaxPost - номер отсчёта с максимальным значением amplPost
				//peakInds[1] = indMax1Na - номер отсчёта с максимальным значением amplPre1
				//peakInds[2] = indMax2K - номер отсчёта с максимальным значением amplPre2
			*spans;//все характерные времена (периоды, фазы, интервалы)
				//spans[0] = riseT - время нарастания (микросекунды)
				//spans[1] = fallT - время спада (микросекунды)
				//spans[2] = t1NaPeak - длительность первого пика (натриевого тока, микросекунды)
				//spans[3] = tDelay - синаптическая задержка или интервал... (микросекунды)
				//spans[4] = tSynTrans - время синаптического проведения (микросекунды)
};

//---------------------------------------------------------------------------

class TExperiment : public TForm
{
__published:	// IDE-managed Components
	TGroupBox *deviceBox;
	TGroupBox *exptBox;
	TGroupBox *expSet;
	TComboBox *uiBase;
	TLabel *Label2;
	TEdit *DevEvents;
	TComboBox *uiChanAct;
	TLabel *Label3;
	TEdit *StimFreq;
	TLabel *stimLbl2;
	TLabel *stimLbl3;
	TButton *InducIntracell;
	TEdit *StimPeriod;
	TLabel *stimLbl4;
	TEdit *NumOfsignals;
	TLabel *nsLbl1;
	TButton *SaveTo;
	TButton *SpontanSignal;
	TButton *ShowOscil;
	TButton *InducExtracell;
	TEdit *FilePathName;
	TComboBox *uiGain;
	TLabel *Label21;
	TGroupBox *visualization;
	TButton *Ampl_gist;
	TLabel *Label1;
	TLabel *Label20;
	TButton *RiseTime_gist;
	TButton *FallTime_gist;
	TButton *Ampl_inT;
	TButton *Rise_inT;
	TButton *Fall_inT;
	TMainMenu *MainMenu1;
	TMenuItem *beginFOpen;
	TButton *SetScenar;
	TCheckBox *ggRefresh;
	TLabel *Label12;
	TLabel *Label13;
	TEdit *refreshEvery;
	TButton *InducSspontS;
	TButton *RateMS_inTime;
	TButton *NaAmpl_inT;
	TCheckBox *TabsCheck;
	TLabel *Label10;
	TEdit *DiscreTime;
	TLabel *nsLbl2;
	TLabel *stimLbl1;
	TLabel *Label24;
	TButton *KAmpl_inT;
	TButton *St0_inT;
	TButton *SynDel_inT;
	TButton *NaDur_inT;
	TButton *NaAmpl_gist;
	TButton *NaDur_gist;
	TButton *St0_gist;
	TButton *KAmpl_gist;
	TButton *SynDel_gist;
	TMenuItem *verticalScal;
	TMenuItem *shwAmpls;
	TMenuItem *pCentFirstS;
	TMenuItem *pCentMax;
	TCheckBox *RecMode;
	TLabel *mChnlRecModLbl;
	TMenuItem *timScal;
	TMenuItem *inSeconds;
	TMenuItem *inMinutes;
	TMenuItem *inSignalNum;
	TCheckBox *DiscontinWrt;
	TLabel *DiscntWrtLbl;
	TComboBox *uiLeadChan;
	TLabel *Label8;
	void __fastcall uiBaseChange(TObject *Sender);
	void __fastcall InducIntracellClick(TObject *Sender);
	void __fastcall DiscreTimeChange(TObject *Sender);
	void __fastcall StimFreqChange(TObject *Sender);
	void __fastcall StimPeriodChange(TObject *Sender);
	void __fastcall SaveToClick(TObject *Sender);
	void __fastcall SpontanSignalClick(TObject *Sender);
	void __fastcall ShowOscilClick(TObject *Sender);
	void __fastcall InducExtracellClick(TObject *Sender);
	void __fastcall beginFOpenClick(TObject *Sender);
	void __fastcall NumOfsignalsChange(TObject *Sender);
	void __fastcall ggRefreshClick(TObject *Sender);
	void __fastcall SetScenarClick(TObject *Sender);
	void __fastcall InducSspontSClick(TObject *Sender);
	void __fastcall ReleasMem(TObject *Sender);
	void __fastcall refreshEveryChange(TObject *Sender);
	void __fastcall TabsCheckClick(TObject *Sender);
	void __fastcall uiChanActChange(TObject *Sender);
	void __fastcall uiGainChange(TObject *Sender);
	void __fastcall VerticAxisMode(TObject *Sender);
	void __fastcall CheckForKeyStimFreqEdit(TObject *Sender, char &Key);
	void __fastcall CheckForKeyPorogEdit(TObject *Sender, char &Key);
	void __fastcall CheckForKeyOtherEdit(TObject *Sender, char &Key);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall HorizontAxisMode(TObject *Sender);
	void __fastcall DiscontinWrtClick(TObject *Sender);
	void __fastcall RecModeClick(TObject *Sender);
	void __fastcall uiLeadChanChange(TObject *Sender);
	void __fastcall ParameterClick(TObject *Sender);
	void __fastcall HistogramClick(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
private:	// User declarations
public:		// User declarations
	__fastcall TExperiment(TComponent* Owner);
	void BeginMean(short *extractLocM,__int32 toRec);
	bool InitDevice();
	void DefindDAC();
    ADCParametersDMA a;//структура параметров устройства
	ADCParametersDMAEX aEx;//структура параметров устройства
	short plottedGrphs,//cчётчик высвеченных графиков
		  plottedHists,//счётчик отображаемых гистограмм
		  bwSh;//величина побитового сдвига при выделении кода АЦП
	__int32 dmaBlockSize,//размер блока данных, получаемых методом GetBuffer
			*baseList,//указатель на список адресов
			*gainList,//указатель на список коэффицентов усиления
			mChannels[maxChannels],//номера каналов
			mGains[maxChannels],//коэффиценты усиления
			maxADCAmp,//максимальная амплитуда АЦП (в отсчётах)
			minADCAmp,//минимальная амплитуда АЦП (в отсчётах)
			adcBase,//текущий базовый адрес устройства сбора данных
			adcDRQ,//канал DMA
			maxMemoSize,//максимальный размер памяти под буфер
			progPrms[paramsToSave],//массив с настройками программы
			adcProps[4];//наборы флагов, показывающие все функции устройства
	float maxVoltage,//диапазон допустимых входных напряжений (милливольты)
		  maxADCFreq,//максимальная частота дискретизации
		  minADCFreq;//минимальная частота дискретизации
	bool aChanged;//произошли ли изменения параметров, требующие переинициализации (InitDevice)
	AnsiString defltExtPraDrr;//расширение по умолчанию (*.pra или *.dpr)
	unsigned short eds;//маска для выделения кода цифрового порта
};
//---------------------------------------------------------------------------
extern PACKAGE TExperiment *Experiment;

//---------------------------------------------------------------------------
#endif
