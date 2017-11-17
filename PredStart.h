//$$---- Form HDR ----
//---------------------------------------------------------------------------

#ifndef PredStartH
#define PredStartH
#include <Chart.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
#include <Series.hpp>
#include <StdCtrls.hpp>
#include <TeEngine.hpp>
#include <TeeProcs.hpp>
//---------------------------------------------------------------------------
#include "TimeCourses.h"
#include "RecordThrd.h"
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TPStart : public TForm
{
__published:	// IDE-managed Components
	TButton *GetASignal;
	TChart *StandardSgnl;
	TButton *StartRec;
	TButton *CloseWin;
	TUpDown *PreTSel;
	TUpDown *AmpPorogSel;
	TUpDown *PostTSel;
	TEdit *PreTime;
	TEdit *PostTime;
	TButton *ExpInfo;
	TEdit *Porog;
	TLabel *PorogLbl;
	TEdit *SignalLen;
	TLabel *SLenLbl3;
	TCheckBox *Invert;
	TCheckBox *NulMinus;
	TLabel *PreTimeLbl;
	TLabel *PostTimeLbl;
	TLabel *SLenLbl1;
	TEdit *BackTime;
	TLabel *BckTmLbl3;
	TLabel *BckTmLbl1;
	TFastLineSeries *porogLine;
	TFastLineSeries *preTLine;
	TFastLineSeries *postTLine;
	TLabel *BckTmLbl2;
	TLabel *SLenLbl2;
	TButton *NextBlock;
	TButton *PausSpontan;
	TTrackBar *TimeScaleBar;
	TTrackBar *TimeBar;
	TLabel *LenSpntLbl3;
	TLabel *LenSpntLbl1;
	TLabel *LenSpntLbl2;
	TEdit *LenSpont;
	void __fastcall AmpPorogSelClick(TObject *Sender, TUDBtnType Button);
	void __fastcall PreTSelClick(TObject *Sender, TUDBtnType Button);
	void __fastcall PostTSelClick(TObject *Sender, TUDBtnType Button);
	void __fastcall PreTimeChange(TObject *Sender);
	void __fastcall PostTimeChange(TObject *Sender);
    void __fastcall PorogChange(TObject *Sender);
	void __fastcall onCloseWin(TObject *Sender);
	void __fastcall LenSpontChange(TObject *Sender);
	void __fastcall CloseWinClick(TObject *Sender);
	void __fastcall ExpInfoClick(TObject *Sender);
	void __fastcall GetASignalClick(TObject *Sender);
	void __fastcall BackTimeChange(TObject *Sender);
	void __fastcall StartRecClick(TObject *Sender);
	void __fastcall StartOfRecordThread(TObject *Sender);
	void __fastcall InvertClick(TObject *Sender);
	void __fastcall OffLineInvert(TObject *Sender);
	void __fastcall NextBlockClick(TObject *Sender);
	void __fastcall PausSpontanClick(TObject *Sender);
	void __fastcall StandardSgnlMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall CheckForKeyPStartEdits(TObject *Sender, char &Key);
	void __fastcall CheckForKeyPPorogEdit(TObject *Sender, char &Key);
	void __fastcall ReCalClick(TObject * Sender);
	void __fastcall TimeBarChange(TObject *Sender);
	void __fastcall SignalLenChange(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TPStart(TComponent* Owner);
	void PredStartWindow(IADCUtility *p0, IADCDevice *p1, short p3);
	void PlotStandardSignal(double *sData, __int32 sRecLen, float sEffDT, __int32 sIndBgn);
	void RefreshLines(float rlEffDT);
	void SetControlVis(short expType);
	__int32 winDraw(__int32 samplPerCh);
	float timeOfDrawBgn;//врем€ в мкс, от которого начинаем рисовать
	TRecordThread *preRecThread;//указатель на объект-поток
	TFastLineSeries *exmplChannls[maxChannels];//массив указателей на графики сигналов со всех каналов в окне "пример сигнала"
	short** continRec,//отсчЄты непрерывной записи поканально
		  postCompInvert;//пол€рность
};
//---------------------------------------------------------------------------
extern PACKAGE TPStart *PStart;
//---------------------------------------------------------------------------
#endif
