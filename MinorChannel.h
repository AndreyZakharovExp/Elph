//$$---- Form HDR ----
//---------------------------------------------------------------------------

#ifndef MinorChannelH
#define MinorChannelH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TSecondChan : public TForm
{
__published:	// IDE-managed Components
	TEdit *Chan2;
	TEdit *Chan3;
	TEdit *Chan4;
	TEdit *Chan5;
	TLabel *chan2Lbl1;
	TLabel *chan3Lbl1;
	TLabel *chan2Lbl2;
	TLabel *chan3Lbl2;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *Chan6;
	TEdit *Chan7;
	TLabel *Label5;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	TEdit *Chan8;
	TLabel *Label10;
private:	// User declarations
public:		// User declarations
	__fastcall TSecondChan(TComponent* Owner);
	void RenewMinorChanWin();
	TEdit *MChann[maxChannels - 1];//указатели на окна вывода данных со второстепенных каналов
};
//---------------------------------------------------------------------------
extern PACKAGE TSecondChan *SecondChan;
//---------------------------------------------------------------------------
#endif
