//----------------------------------------------------------------------------
#ifndef RenamOverrH
#define RenamOverrH
//----------------------------------------------------------------------------
#include <vcl\ExtCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\Classes.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Windows.hpp>
#include <vcl\System.hpp>
//----------------------------------------------------------------------------
class TChoosDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *OverWrite;
	TBevel *Bevel1;
	TLabel *Label1;
	TEdit *chooseSigs;
	TEdit *sigsInGrp;
	TButton *AppEnd;
	TButton *CancelBtn;
	TLabel *Label2;
	TCheckBox *UnderDL;
	void __fastcall chooseSigsKeyPress(TObject *Sender, char &Key);
	void __fastcall sigsInGrpKeyPress(TObject *Sender, char &Key);
	void __fastcall UnderDLClick(TObject *Sender);
private:
public:
	virtual __fastcall TChoosDlg(TComponent* AOwner);
	void setsForNumbersChs();
	void setsForFileChs();
	void setsForNOfGrp();
	void CompilNumbers();
	void MultiplCompilNumbers();
	bool sav;//сохранение (true) или удаление (false) указанных сигналов
};
//----------------------------------------------------------------------------
extern PACKAGE TChoosDlg *ChoosDlg;
//----------------------------------------------------------------------------
#endif    
