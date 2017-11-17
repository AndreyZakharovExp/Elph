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
struct markers //��������� ��� ������������ ������������� ������� � �������
{
	AnsiString textMark;//���������� ����� �����
	__int32 pointOnGraph;//����� ������� �� �������, � �������� �������� ������ �����
	bool chanN[maxChannels];//������ �������, ������� ����������� ������ �����
	markers *nextM,//����� ��������� ����� � ����������� ������������ �������
			*prevM;//����� ���������� ����� � ����������� ������������ �������
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
	markers *theMarker;//��������� �� ����� ����� � ������������ ������� ��� �����
	__int32 npNewRec,//���������� ��������, ���������� � ���������� ������� ����� ������ (�������� � ������������ ����)
			nmInRec;//���������� �����, ���������� � ���������� ������� ����� ������ (�������� � ������������ ����)
	AnsiString pIDateString;//���������� � �����
};
//---------------------------------------------------------------------------
extern PACKAGE TExpNotes *ExpNotes;
//---------------------------------------------------------------------------
#endif
