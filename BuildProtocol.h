//$$---- Form HDR ----
//---------------------------------------------------------------------------

#ifndef BuildProtocolH
#define BuildProtocolH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <ExtDlgs.hpp>
//---------------------------------------------------------------------------
class SPPanel : public TPanel
{
	public:
    	__fastcall SPPanel(TComponent* Owner);
		__int32 iType,//тип инструкции (блока)
				scCount,//кол-во стимулов или повторений в цикле
				period;//длительность паузы или период стимуляции
};

class TProtoBuild : public TForm
{
__published:	// IDE-managed Components
	TScrollBox *InstructBox;
	TButton *StimBlock;
	TButton *Silence;
	TEdit *StimsNum;
	TEdit *Periods;
	TLabel *Label1;
	TButton *Repeat;
	TEdit *repeats;
	TButton *DelLast;
	TLabel *Label2;
	TLabel *Label3;
	TButton *AcceptProt;
	TButton *saveProtocol;
	TButton *loadProtocol;
	TSaveDialog *SaveDialog1;
	TButton *CancelAll;
	TCheckBox *StarTim;
	TEdit *HEdit;
	TLabel *STLbl;
	TUpDown *HUpDown;
	TLabel *HLbl;
	TEdit *MEdit;
	TUpDown *MUpDown;
	TLabel *MLbl;
	TEdit *SEdit;
	TUpDown *SUpDown;
	TLabel *SLbl;
	TLabel *Label4;
	TOpenDialog *OpenDialog1;
	void __fastcall StimBlockClick(TObject *Sender);
	void __fastcall RepeatClick(TObject *Sender);
	void __fastcall SilenceClick(TObject *Sender);
	void __fastcall DelLastClick(TObject *Sender);
	void __fastcall EndOfProtoBuild(TObject *Sender);
	void __fastcall SaveProtocolClick(TObject *Sender);
	void __fastcall LoadProtocolClick(TObject *Sender);
	void __fastcall CancelAllClick(TObject *Sender);
	void __fastcall CloseClicked(TObject *Sender);
	void __fastcall CheckForKeyOtherEdits(TObject *Sender, char &Key);
	void __fastcall StarTimClick(TObject *Sender);
	void __fastcall UpDownClick(TObject *Sender, TUDBtnType Button);
	void __fastcall EditChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TProtoBuild(TComponent* Owner);
	void StimCreat(__int32 stimNums, __int32 stimPrd);
	void SilenceCreat(__int32 prd);
	void RepeatCreat(__int32 reps);
};
//---------------------------------------------------------------------------
extern PACKAGE TProtoBuild *ProtoBuild;
//---------------------------------------------------------------------------
#endif
