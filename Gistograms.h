//$$---- Form HDR ----
//---------------------------------------------------------------------------

#ifndef GistogramsH
#define GistogramsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Chart.hpp>
#include <ExtCtrls.hpp>
#include <TeEngine.hpp>
#include <TeeProcs.hpp>
#include <Series.hpp>
//---------------------------------------------------------------------------
class TGists : public TForm
{
__published:	// IDE-managed Components
	TChart *MiniFreq;
	TLineSeries *freqM;
	TChart *amplG;
	TBarSeries *gsAmpls;
	TBarSeries *gsRiseTs;
	TBarSeries *gsFallTs;
	TBarSeries *gsNaAmpls;
	TBarSeries *gsNaDurs;
	TBarSeries *gsKAmpls;
	TBarSeries *gsSynDels;
	TBarSeries *gsSynTrns;
	TChart *riseTG;
	TChart *fallTG;
	TChart *synDelayG;
	TChart *synTrnsG;
	TChart *NaAmplG;
	TChart *NaDurG;
	TChart *KAmplG;
	void __fastcall Resized(TObject *Sender);
	void __fastcall GistsClear(TObject *Sender);
	void __fastcall HistChartMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
private:	// User declarations
public:		// User declarations
	__fastcall TGists(TComponent* Owner);
	void GistsRefresh(short toReplot);
	void RateMiniSigCalc(TLineSeries *graphOnGraphs);
	float GistsCalculation(TLineSeries *graphOnGraphs, TBarSeries *theGist);
	TChart *allCharts[9];//массив ссылок на графики (чарты)
	__int32 Signt(__int32 aVal);
};
//---------------------------------------------------------------------------
extern PACKAGE TGists *Gists;
//---------------------------------------------------------------------------
#endif
