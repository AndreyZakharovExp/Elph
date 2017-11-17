//$$---- Thread HDR ----
//---------------------------------------------------------------------------

#ifndef RecordThrdH
#define RecordThrdH

//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <time.h>
#include "MainForm.h"
//---------------------------------------------------------------------------
class TRecordThread : public TThread
{
private:
protected:
	void __fastcall Execute();
	void __fastcall RecSynchroPlotExampl(void);
	void __fastcall RecSynchroCounter(void);
	void __fastcall MiniSynchroPlotGraphs(void);
	void __fastcall InducSynchroPlotter(void);
	void __fastcall FromMinorChan(void);
	void __fastcall MiniMd4SynchroPlot(void);
	void __fastcall CallForClearMemor(void);
	void __fastcall CallForReplotGraphs(void);
public:
	__fastcall TRecordThread(bool CreateSuspended);
	void AssignVar(short eTyp, IADCUtility *preUtil, IADCDevice  *preADC);
	void CreatExtendFile();
	void CompleteDisconFile();
	void StartRec(__int32 allExcit, __int32 allSpont, __int32 pp);
	void Spnt_SgnlAcquisition(__int32 mNumOfS, clock_t experimBegin);
	void __fastcall Induced_SgnlAcquisition(__int32 iNumOfS, __int32 iStimPrd, bool canPlot);
	void __fastcall Induced_N_Spnt_SgnlAcquis(__int32 allMini, __int32 iNumOfS, __int32 iStimPrd, bool canPlot);
	void DiscontinWrite();
	void GetLocalMean();
//	void SetToDAC();
	unsigned short eds;//маска для выделения кода цифрового порта
	__int32 stim_out[2];//синхроимпульса (два значения: ноль и неноль)
	short bwSh;//величина побитового сдвига при выделении кода АЦП
	//атрибуты сигналов
	bool multiCh;//режим сбора данных (true = многоканальный)
	__int32 recLen,//длина развёртки сигнала (отсчёты)
			chanls,//количество сканируемых каналов
			ftChan;//количество каналов с полными развёртками (full trace channels)
	float discrT,//время дискретизации (микросекунды)
		  effDT,//эффективное время дискретизации = discrT * chanls
		  sampl2mV;//коэффициент перевода отсчётов АЦП в милливольты = maxVoltage/(maxADCAmp * ADCGain)
};
//---------------------------------------------------------------------------
#endif
