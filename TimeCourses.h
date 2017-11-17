//$$---- Form HDR ----
//---------------------------------------------------------------------------

#ifndef TimeCoursesH
#define TimeCoursesH
#include <Chart.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Series.hpp>
#include <StdCtrls.hpp>
#include <TeEngine.hpp>
#include <TeeProcs.hpp>
#include <Grids.hpp>
#include <ValEdit.hpp>
#include <vcl\Clipbrd.hpp>
#include <math.h>
#include <algorith.h>
//---------------------------------------------------------------------------
#include "IADCDevice.h"
#include <DB.hpp>
#include <DBTables.hpp>
#include <DBCtrls.hpp>
#include <Mask.hpp>
#include "MainForm.h"
#include "RecordThrd.h"
//---------------------------------------------------------------------------

struct blockbyte //��������� ������ � �������
{
	AnsiString 	   nob;				//(1) = nameOfBlock => ���������� ����� ������
	unsigned short byteInHead;      //(2) ����� ����� � ��������� (��������� ����� ����� - ������ ������)
	unsigned long  byteInFile;		//(3) ����� ����� � ����� ������� ����� (���� ����, ������ ����� ���)
	short 		   szInHDataType,   //(4) ������ ���� ������, ���������� � ���������
				   szDataType;      //(5) ������ ���� ������, ���������� � ���� �����
	unsigned long  numOfElements;   //(6) ����� ���������, �������� � ���� ���� ������

};

class TGraphs : public TForm
{
__published:	// IDE-managed Components
	TChart *SignalTrack;
	TChart *AmplInT;
	TChart *RiseTInT;
	TChart *FallTInT;
	TChart *NaAmplInT;
	TChart *NaDurInT;
	TChart *KAmplInT;
	TChart *SynDelInT;
	TChart *SynTrnsInT;
	TMainMenu *MainMenu1;
	TMenuItem *gphFile;
	TMenuItem *gphOpen;
	TMenuItem *gphSave;
	TMenuItem *signalManager;
	TOpenDialog *OpenDlg;
	TSaveDialog *SaveDlg;
	TMenuItem *ShowMean;
	TMenuItem *expInfo;
	TMenuItem *usersInfo;
	TMenuItem *expParams;
	TPointSeries *csSpecPoint;
	TPointSeries *curAmpl;
	TLineSeries *gRiseTsL;
	TPointSeries *curRiseT;
	TLineSeries *gFallTsL;
	TPointSeries *curFallT;
	TLineSeries *gNaAmplsL;
	TPointSeries *curNaAmpl;
	TLineSeries *gNaDursL;
	TPointSeries *curNaDur;
	TLineSeries *gKAmplsL;
	TPointSeries *curKAmpl;
	TLineSeries *gSynDelsL;
	TPointSeries *curSynDel;
	TLineSeries *gSynTrnsL;
	TPointSeries *curSynTrns;
	TMenuItem *parametrExport;
	TMenuItem *SetExpAs;
	TLabel *Label1;
	TLabel *Label2;
	TEdit *CrntSig;
	TUpDown *SigNumUpDwn;
	TMenuItem *asIntracel;
	TMenuItem *asMini;
	TMenuItem *ReCalculat;
	TLineSeries *gAmplsL;
	TStringGrid *pValues;
	TPopupMenu *PopupMenu1;
	TMenuItem *selCopy;
	TMenuItem *prevSignal;
	TMenuItem *nextSignal;
	TMenuItem *deletSignal;
	TMenuItem *remGrpSigs;
	TMenuItem *saveGrpSigs;
	TMenuItem *signalsExport;
	TMenuItem *asExtracel;
	TMenuItem *showMarks;
	TMenuItem *prntWin;
	TMenuItem *dividGrps;
	TMenuItem *selAll;
	TMenuItem *ShowChanN;
	TMenuItem *SAllChan;
	void __fastcall Resized(TObject *Sender);
	void __fastcall GraphChartDblClick(TObject *Sender);
	void __fastcall gphOpenClick(TObject *Sender);
	void __fastcall OnClosGraphs(TObject *Sender);
	void __fastcall ClearMemor(TObject *Sender);
	void __fastcall ReadDataFile(TObject *Sender);
	void __fastcall ReadLineData(TObject *Sender);
	void __fastcall DeletSignalClick(TObject *Sender);
	void __fastcall GphSaveClick(TObject *Sender);
	void __fastcall usersInfoClick(TObject *Sender);
	void __fastcall NextPres(TObject *Sender);
	void __fastcall PrevPres(TObject *Sender);
	void __fastcall parametrExportClick(TObject *Sender);
	void __fastcall ShowMeanClick(TObject *Sender);
	void __fastcall ReCalculatClick(TObject *Sender);
	void __fastcall asIntracelClick(TObject *Sender);
	void __fastcall asExtracelClick(TObject *Sender);
	void __fastcall asMiniClick(TObject *Sender);
	void __fastcall ChartsClickSeries(TCustomChart *Sender, TChartSeries *Series,
          int ValueIndex, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall pValuesSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
	void __fastcall selCopyClick(TObject *Sender);
	void __fastcall amplInTAfterDraw(TObject *Sender);
	void __fastcall remGrpSigsClick(TObject *Sender);
	void __fastcall saveGrpSigsClick(TObject *Sender);
	void __fastcall CrntSigKeyPress(TObject *Sender, char &Key);
	void __fastcall CrntSigKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall signalsExportClick(TObject *Sender);
	void __fastcall showMarksClick(TObject *Sender);
	void __fastcall PrintWinClick(TObject *Sender);
	void __fastcall dividGrpsClick(TObject *Sender);
	void __fastcall SignalTrackMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall expParamsClick(TObject *Sender);
	void __fastcall selAllClick(TObject *Sender);
	void __fastcall ShowTheChannlClick(TObject *Sender);
	void __fastcall SAllChanClick(TObject *Sender);
	void __fastcall sigNumUpDwnChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall CrntSigKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
private:	// User declarations
public:		// User declarations
	__fastcall TGraphs(TComponent* Owner);
	trac* CreatStructSignal(__int32 crNumOfS, __int32 crRecLen);
	void DeleteStructSignal(trac *delSignal, __int32 delNumOfS);
	bool SaveExpDataToFile(short expType, trac *signals_loc, __int32 numOfSignal, bool saveType);
	void HighLightCrnt(trac *hSig, __int32 shft1, __int32 shft2, short hExpT, __int32 hIndBgn, __int32 pGr);
	void PlotCurrentSignal(double *pcsData, __int32 pcsIndBgn);
	void ClearGraphics();
	void GraphsTabsShow();
	void FillParamTable();
	void ResetVisibility();
	void AddParamsPoints(trac *apSignals, __int32 apSigNum, __int32 *apIndex, short apExpType);
	void ShowSignal(__int32 sn);
	void ReplotExamplRecalc();
	void CopyRefSignal_forGists();
	void SetSeriesMultipliers();
	void RemovEnteredNumbers(__int32 *svRmIndex, __int32 srILen);
	void RemoveSignals(__int32 sigToDel);
	void ConvertBinary(bool cTyp);
	TChart *allCharts[9];//������ ����� ���������� �� ��� ������� (�����)
	TFastLineSeries *curntSigChannls[maxChannels];//������ ���������� �� ������� �������� �� ���� ������� � ���� "������� ������"
	TLineSeries *sigAmpls[maxChannels];//��������� ��������
	float plotFrom;//���������� �������� � ����� ������� �������
	__int32 *gInd,//������� �������� ��������. ��� �������� �������, ������� ��� ����� �� ����� �������
        	gRF[2];//������������������ ������ ������� � ������� ���������� (0 - ������ ������������ �����, 1 - ��������� ������������ ����� + 1)
	TMenuItem *ChannNumb[maxChannels];//������ ���������� �� ������� � ���� ShowChanN
	sPr *cPrm;//������� � ������������ ����������� ��������
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
extern PACKAGE TGraphs *Graphs;
//---------------------------------------------------------------------------
#endif
