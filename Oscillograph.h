//$$---- Form HDR ----
//---------------------------------------------------------------------------

#ifndef OscillographH
#define OscillographH
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

#include <Buttons.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include "cspin.h"
#include <Graphics.hpp>

//---------------------------------------------------------------------------
class TOscil : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TTrackBar *NulShift;
	TTrackBar *timeScale;
	TTrackBar *vScale;
	TLabel *Label2;
	TButton *acceptParamtr;
	TRadioButton *Alternating;
	TRadioButton *ConstCompon;
	TCheckBox *invert;
	TLabel *Label3;
	TLabel *chanlsLbl;
	TCheckBox *checkChann0;
	TCheckBox *checkChann1;
	TCheckBox *checkChann2;
	TCheckBox *checkChann3;
	TCheckBox *checkChann4;
	TCheckBox *checkChann5;
	TCheckBox *checkChann6;
	TCheckBox *checkChann7;
	TCSpinEdit *ImpPerMin;
	TButton *OneImpulse;
	TButton *ContRec;
	TCSpinEdit *Porog;
	TPanel *TxtPanel;
	TLabel *MinTmpWinLbl;
	TLabel *CrntTmpWinLbl;
	TLabel *MaxTmpWinLbl;
	TLabel *Label4;
	TLabel *Label5;
	void __fastcall timeScaleChange(TObject *Sender);
	void __fastcall onOscClose(TObject *Sender);
	void __fastcall winoscil(IADCUtility* o_pUtil, IADCDevice *o_pADC);
	void __fastcall acceptParamtrClick(TObject *Sender);
	void __fastcall PorogChange(TObject *Sender);
	void __fastcall invertClick(TObject *Sender);
	void __fastcall AlternatingClick(TObject *Sender);
	void __fastcall ConstComponClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall RightKeyPress1(TObject *Sender, char &Key);
	void __fastcall RightKeyPress2(TObject *Sender, char &Key);
	void __fastcall OneImpulseClick(TObject *Sender);
	void __fastcall ContRecClick(TObject *Sender);
	void __fastcall NulShiftChange(TObject *Sender);
	void __fastcall vScaleChange(TObject *Sender);
	void __fastcall ImpPerMinChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TOscil(TComponent* Owner);
	TCheckBox *chkChnBoxs[maxChannels];//копии указателей на чекбоксы
};
void StartOsc(void *pParam);
//---------------------------------------------------------------------------
extern PACKAGE TOscil *Oscil;
//---------------------------------------------------------------------------
#endif
