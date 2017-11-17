//$$---- Form HDR ----
//---------------------------------------------------------------------------

#ifndef experimentNotesH
#define experimentNotesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "MainForm.h"
//---------------------------------------------------------------------------
struct markers //структура дл€ циклического динамического массива с метками
{
	AnsiString textMark;//собственно текст метки
	__int32 pointOnGraph;//номер сигнала на графике, к которому прив€зан данна€ метка
	bool chanN[maxChannels];//номера каналов, которым принадлежит данна€ метка
	markers *nextM,//адрес следующей метки в циклическом динамическом массиве
			*prevM;//адрес предыдущей метки в циклическом динамическом массиве
};

class TExpNotes : public TForm
{
__published:	// IDE-managed Components
	TMemo *usersNotes;
	TMemo *PIDates;
	TLabel *mainLinesLbl;
	TEdit *addUMark;
	TLabel *addMarkLbl;
	void __fastcall addUMarkKeyPress(TObject *Sender, char &Key);
private:	// User declarations
public:		// User declarations
	__fastcall TExpNotes(TComponent* Owner);
	void AddMarker(AnsiString addedMark, __int32 pnG, bool *chN);
	void CreateAMark();
	void DeleteAMark(markers *delM);
	void DeleteMarkers();
	markers *theMarker;//указатель на некую метку в динамическом массиве дл€ меток
	__int32 npNewRec,//количество сигналов, записанных в предыдущих сеансах сбора данных (дозапись в существующий файл)
			nmInRec;//количество меток, записанных в предыдущих сеансах сбора данных (дозапись в существующий файл)
	AnsiString pIDateString;//информацю€ о файле
};
//---------------------------------------------------------------------------
extern PACKAGE TExpNotes *ExpNotes;
//---------------------------------------------------------------------------
#endif
