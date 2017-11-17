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
	unsigned short eds;//����� ��� ��������� ���� ��������� �����
	__int32 stim_out[2];//�������������� (��� ��������: ���� � ������)
	short bwSh;//�������� ���������� ������ ��� ��������� ���� ���
	//�������� ��������
	bool multiCh;//����� ����� ������ (true = ��������������)
	__int32 recLen,//����� �������� ������� (�������)
			chanls,//���������� ����������� �������
			ftChan;//���������� ������� � ������� ���������� (full trace channels)
	float discrT,//����� ������������� (������������)
		  effDT,//����������� ����� ������������� = discrT * chanls
		  sampl2mV;//����������� �������� �������� ��� � ����������� = maxVoltage/(maxADCAmp * ADCGain)
};
//---------------------------------------------------------------------------
#endif
