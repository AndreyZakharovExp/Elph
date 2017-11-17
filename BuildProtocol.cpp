//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>

#pragma hdrstop

#include "MainForm.h"
#include "BuildProtocol.h"
#include "PredStart.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProtoBuild *ProtoBuild;

//---------------------------------------------------------------------------
__fastcall SPPanel::SPPanel(TComponent* Owner) : TPanel(Owner)
{
}

//---------------------------------------------------------------------------
__fastcall TProtoBuild::TProtoBuild(TComponent* Owner) : TForm(Owner)
{
	//виды инструкций: 1 - тип инструкции (1 - стимуляция, 2 - пауза, 3 - цикл)
	//2 - кол-во стимулов или повторений в цикле, 3 - длительность паузы или период стимуляции

	//InstructBox->ComponentCount - количество блоков (компонентов)


	//ProtoBuild->LoadProtocolClick(this);


	//ProtoBuild->Width = InstructBox->Left + InstructBox->Width;//уменьшаем ширину окна
	//ProtoBuild->Periods->Tag - наименьший период между импульсами стимуляции
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::StimBlockClick(TObject *Sender)
{
	//задание блока стимуляции
	__int32 stimPrd,
			stimNums;

	StimsNum->Visible = !StimsNum->Visible;
	Periods->Visible = !Periods->Visible;
	Label1->Visible = !Label1->Visible;

	//инактивируем кнопки
	Silence->Enabled = false;
	Repeat->Enabled = false;
	DelLast->Enabled = false;
	if (!StimsNum->Visible)
	{
		//проверяем значение на корректность
		stimPrd = StrToInt(Periods->Text);
		if (stimPrd < 5)//нижний предел периода стимуляции = 5 мс
		{
			stimPrd = 5;
			Periods->Text = IntToStr(stimPrd);
		}
		if (stimPrd > 1e7)//верхний предел периода стимуляции = 1e7 мс (или 10000 с)
		{
			stimPrd = 1e7;
			Periods->Text = IntToStr(stimPrd);
		}

		stimNums = StrToInt(StimsNum->Text);
		if (stimNums < 1)
		{
			stimNums = 1;
			StimsNum->Text = IntToStr(stimNums);
		}

		StimCreat(stimNums, stimPrd);//заносим в протокол блок стимуляции

		//восстанавливаем кнопки
		Silence->Enabled = true;
		Repeat->Enabled = true;
		DelLast->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void TProtoBuild::StimCreat(__int32 stimNums, __int32 stimPrd)
{
	//заносим в протокол блок стимуляции
	/*
	stimNums - количество стимулов
	stimPrd - период следования стимулов
	*/

	SPPanel *panel,
			*leftPanel;

	panel = new SPPanel(InstructBox);//создаём новый блок (инструкцию)
	panel->Parent = InstructBox;//указываем родителя
	panel->Caption = "Стим: " + IntToStr(stimNums) + "(имп), " + IntToStr(stimPrd) + "(мс)";

	if (InstructBox->ComponentCount > 1)
	{
		leftPanel = (SPPanel*)InstructBox->Components[InstructBox->ComponentCount - 2];//предыдущий блок
		panel->Left = leftPanel->Left + leftPanel->Width;
	}
	else
		panel->Left = 0;
		
	panel->Width = 10 + 5 * panel->Caption.Length();
	panel->Visible = true;
	panel->iType = 1;//тип инструкции (стимуляция)
	panel->scCount = stimNums;//количество импульсов
	panel->period = stimPrd;//период следования импульсов
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::SilenceClick(TObject *Sender)
{
	//задаём период ожидания
	//желательно, чтобы пауза была более 100 мс, т.к. каждый старт
	// непрерывного сбора создаёт задержку

	__int32 prd;

	Periods->Visible = !Periods->Visible;
	Label2->Visible = !Label2->Visible;

	//инактивируем кнопки
	StimBlock->Enabled = false;
	Repeat->Enabled = false;
	DelLast->Enabled = false;
	if (!Periods->Visible)
	{
		//проверяем значение на корректность
		prd = StrToInt(Periods->Text);
		if (prd < 5)
		{
			prd = 5;
			Periods->Text = IntToStr(prd);
		}

		SilenceCreat(prd);//заносим в протокол ожидание

		//восстанавливаем кнопки
		StimBlock->Enabled = true;
		Repeat->Enabled = true;
		DelLast->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void TProtoBuild::SilenceCreat(__int32 prd)
{
	//заносим в протокол ожидание
	/*
	prd - длительность паузы
	*/

	SPPanel *panel,
			*leftPanel;

	panel = new SPPanel(InstructBox);
	panel->Parent = InstructBox;
	panel->Caption = "Пауза: " + IntToStr(prd) + "(мс)";

	if (InstructBox->ComponentCount > 1)
	{
		leftPanel = (SPPanel*)InstructBox->Components[InstructBox->ComponentCount - 2];//предыдущий блок
		panel->Left = leftPanel->Left + leftPanel->Width;
	}
	else
		panel->Left = 0;

	panel->Width = 10 + 5 * panel->Caption.Length();
	panel->Visible = true;
	panel->iType = 2;//тип инструкции (пауза)
	panel->period = prd;//период ожидания
	panel->scCount = prd;//дублируем значение
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::RepeatClick(TObject *Sender)
{
	//задаём количество повторов сценария

	__int32 reps;

	repeats->Visible = !repeats->Visible;
	Label3->Visible = !Label3->Visible;
	Label4->Visible = !Label4->Visible;

	//инактивируем кнопки
	StimBlock->Enabled = false;
	Silence->Enabled = false;
	DelLast->Enabled = false;
	if (!repeats->Visible)
	{
		//проверяем значение на корректность
		reps = StrToInt(repeats->Text);
		if (reps < 1)
		{
			reps = 1;
			repeats->Text = IntToStr(reps);
		}

		RepeatCreat(reps);//заносим в протокол цикл

		//восстанавливаем кнопки
		StimBlock->Enabled = true;
		Silence->Enabled = true;
		DelLast->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void TProtoBuild::RepeatCreat(__int32 reps)
{
	//заносим в протокол цикл
	/*
	reps - количество проходов в цикле
	*/
	
	SPPanel *panel,
			*leftPanel;

	panel = new SPPanel(InstructBox);
	panel->Parent = InstructBox;
	panel->Caption = "Цикл: " + IntToStr(reps) + "(повторы)";

	if (InstructBox->ComponentCount > 1)
	{
		leftPanel = (SPPanel*)InstructBox->Components[InstructBox->ComponentCount - 2];//предыдущий блок
		panel->Left = leftPanel->Left + leftPanel->Width;
	}
	else
		panel->Left = 0;

	panel->Width = 17 + 5 * panel->Caption.Length();
	panel->Visible = true;
	panel->iType = 3;//тип инструкция (цикл)
	panel->scCount = reps;//количество повторов
	panel->period = reps;//дублируем значение
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::DelLastClick(TObject *Sender)
{
	//отмена последнего действия по установке блока

	if (InstructBox->ComponentCount > 0)
	{
		//InstructBox->RemoveComponent(InstructBox->Components[InstructBox->ComponentCount - 1]);
		InstructBox->Controls[InstructBox->ComponentCount - 1]->Free();//удаляем панель-блок (вместо delete)
	}
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::CancelAllClick(TObject *Sender)
{
	//отменяем весь протокол
	while (InstructBox->ComponentCount > 0)
		DelLastClick(this);
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::EndOfProtoBuild(TObject *Sender)
{
	//выполняется при закрытии
	//применяем заданный протокол стимуляции
	//нужно проверить не превышает ли длина сигнала минимальный период в протоколе
	__int32 i;
	SPPanel *panel;

	//сигнализируем о наличии или отсутствии протокола
	if (InstructBox->ComponentCount >= 1)//протокол вступил в силу
	{
		Experiment->SetScenar->Caption = "Редактировать";
		Experiment->NumOfsignals->Enabled = false;//выбор количества сигналов
		Experiment->nsLbl1->Enabled = false;//количество сигналов (лэйбл 1)
		Experiment->nsLbl2->Enabled = false;//количество сигналов (лэйбл 2)
		Experiment->StimFreq->Enabled = false;//выбор частоты стимуляции
		Experiment->StimPeriod->Enabled = false;//выбор периода стимуляцц
		Experiment->stimLbl1->Enabled = false;//стимуляция (лэйбл 1)
		Experiment->stimLbl2->Enabled = false;//стимуляция (лэйбл 2)
		Experiment->stimLbl3->Enabled = false;//стимуляция (лэйбл 3)
		Experiment->stimLbl4->Enabled = false;//стимуляция (лэйбл 4)

		//выбираем наименьший интервал между импульсами в данном протоколе
		for (i = 0; i < InstructBox->ComponentCount; i++)
		{
			panel = (SPPanel*)InstructBox->Components[i];
			if (((panel->iType == 1) || (panel->iType == 2)) && (panel->period < Periods->Tag))
				Periods->Tag = panel->period;//наименьший период между импульсами стимуляции
		}

		PStart->NextBlock->Visible = true;//кнопка перехода к следующему блоку стимуляции
	}
	else//протокол отменён
	{
		Experiment->SetScenar->Caption = "Задать стимуляцию";
		Experiment->NumOfsignals->Enabled = true;//выбор количества сигналов
		Experiment->nsLbl1->Enabled = true;//количество сигналов (лэйбл 1)
		Experiment->nsLbl2->Enabled = true;//количество сигналов (лэйбл 2)
		Experiment->StimFreq->Enabled = true;//выбор частоты стимуляции
		Experiment->StimPeriod->Enabled = true;//выбор периода стимуляцц
		Experiment->stimLbl1->Enabled = true;//стимуляция (лэйбл 1)
		Experiment->stimLbl2->Enabled = true;//стимуляция (лэйбл 2)
		Experiment->stimLbl3->Enabled = true;//стимуляция (лэйбл 3)
		Experiment->stimLbl4->Enabled = true;//стимуляция (лэйбл 4)

		Periods->Tag = StrToInt(Experiment->StimPeriod->Text);//наименьший период между импульсами стимуляции
		PStart->NextBlock->Visible = false;//кнопка перехода к следующему блоку стимуляции
	}
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::CloseClicked(TObject *Sender)
{
	//закрываем редактор протокола
	ProtoBuild->Close();
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::SaveProtocolClick(TObject *Sender)
{
	//сохраняем протокл в указанный файл
	FILE *stream;
	__int32 i;
	SPPanel *panel;

	if (SaveDialog1->Execute())
	{
		//*.spf (s - stimulation, p - protocol, f - file)
		stream = fopen(SaveDialog1->FileName.c_str(), "w");

		for (i = 0; i < InstructBox->ComponentCount; i++)
		{
			panel = (SPPanel*)InstructBox->Components[i];
			fprintf(stream, "%d\t%d\t%d\n", panel->iType, panel->scCount, panel->period);
		}

		fclose(stream);
	}
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::LoadProtocolClick(TObject *Sender)
{
	//загрузить протокол из указанного файла
	FILE *stream;
	__int32 d1, d2, d3;

	if (OpenDialog1->Execute())
	{
		//специализированный файл настроек *.spf (s - stimulation, p - protocol, f - file)
		stream = fopen(OpenDialog1->FileName.c_str(), "r");

		CancelAllClick(this);//отменить пердыдущие команды
		while (fscanf(stream, "%d%d%d", &d1, &d2, &d3) != EOF)
		{
			//d[0] - тип инструкции, d[1] - количество, d[2] - длительность
			if (d1 == 1)//стимуляция
				StimCreat(d2, d3);//заносим в протокол блок стимуляции
			else if (d1 == 2)//пауза
				SilenceCreat(d3);//заносим в протокол пауза
			else if (d1 == 3)//цикл
				RepeatCreat(d2);//заносим в протокол цикл
			else
			{
				CancelAllClick(this);//отменить пердыдущие команды
				Experiment->DevEvents->Text = "ошибка чтения протокола";
				break;//ошибка чтения
			}
		}

		fclose(stream);
	}
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::CheckForKeyOtherEdits(TObject *Sender, char &Key)
{
	//проверка допустимости введения символа, соответствующего нажатой клавише
	//для остальных полей ввода числовых данных

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::StarTimClick(TObject *Sender)
{
	//выбор времени старта по часам компьютера
	if (StarTim->Checked)//выбрано задание времени
	{
		ProtoBuild->Width = STLbl->Left + STLbl->Width + 12;//увеличиваем ширину окна
	}
	else//отмена момента старта
	{
		ProtoBuild->Width = DelLast->Left + DelLast->Width + 12;//уменьшаем ширину окна
    }
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::UpDownClick(TObject *Sender, TUDBtnType Button)
{
	TUpDown *objUpDwn;//объект "кнопки вверх-вниз"
	TEdit *objEdit;//объект "окно с текстом"

	objUpDwn = (TUpDown*)Sender;//объект "кнопки вверх-вниз"
	if (objUpDwn->Tag == 0)//часы
		objEdit = HEdit;//объект "окно с текстом"
	else if (objUpDwn->Tag == 1)//минуты
		objEdit = MEdit;//объект "окно с текстом"
	else if (objUpDwn->Tag == 2)//секунды
		objEdit = SEdit;//объект "окно с текстом"

	objEdit->Text = IntToStr(objUpDwn->Position);//изменение времени
}
//---------------------------------------------------------------------------

void __fastcall TProtoBuild::EditChange(TObject *Sender)
{
	__int32 i;
	TEdit *objEdit;//объект "окно с текстом"
	TUpDown *objUpDwn;//объект "кнопки вверх-вниз"

	objEdit = (TEdit*)Sender;//объект "окно с текстом"
	i = StrToInt(objEdit->Text);//введённое время

	if (objEdit->Tag == 0)//часы
		objUpDwn = HUpDown;//объект "кнопки вверх-вниз"
	else if (objEdit->Tag == 1)//минуты
		objUpDwn = MUpDown;//объект "кнопки вверх-вниз"
	else if (objEdit->Tag == 2)//секунды
		objUpDwn = SUpDown;//объект "кнопки вверх-вниз"

	if ((i > 0) && (i <= objUpDwn->Max))
		objUpDwn->Position = i;//изменение времени
	else
		objEdit->Text = IntToStr(objUpDwn->Position);//возвращаем исходное значение
}
//---------------------------------------------------------------------------

