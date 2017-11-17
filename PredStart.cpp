//$$---- Form CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#include <process.h>

#pragma hdrstop

#include "MainForm.h"
#include "PredStart.h"
#include "ExperimentNotes.h"
#include "BuildProtocol.h"
#include "CalcParams.h"
#include "MinorChannel.h"
#include "RenamOverr.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TPStart *PStart;

IADCDevice *preS_pADC;//указатель на интерфейс драйвера
IADCUtility *preS_pUtil;//указатель на интерфейс драйвера
__int32 timeIncrement;//шаг изменения положения "опорных лини" (время в микросекундах)
float multipler;//множитель для уточнения положения линий
short typeOfExp;//тип эксперимента

//---------------------------------------------------------------------------
__fastcall TPStart::TPStart(TComponent* Owner) : TForm(Owner)
{
	//модуль предварительного старта

	preRecThread = new TRecordThread(true);//создаём поток эксперимента (приостановленным)

	//PStart->closeWin->Tag - управляет прерыванием сбора и функцией кнопки останова-закрытия
	//PStart->GetASignal->Tag - задаёт режим старта сбора: пример сигнала или сценарий
	//Graphs->CrntSig->Tag - счётчик записанных сигналов (вызванных или спонтанных в режимах №1, 2 и 3)
	//PStart->BackTime->Tag - счётчик спонтанных сигналов в режиме №4
	//PStart->PausSpontan->Tag - флаг разрешения записи спонтанных в режиме 4
	//Graphs->sigNumUpDwn->Tag - максимальное количество сигналов (зарезервировано перед стартом)
	//Experiment->DiscontinWrt->Tag - дописывать данные в drr-файл или стереть старые данные
	//ExpNotes->addUMark->Tag - количество заметок по ходу эксперимента (всего присутствует на графике)
	//ProtoBuild->Periods->Tag - наименьший период между импульсами стимуляции

	timeIncrement = 5;//шаг изменения положения "опорных лини" (время в микросекундах
	multipler = 1;//множитель для уточнения положения линий
}
//---------------------------------------------------------------------------

void TPStart::PredStartWindow(IADCUtility *p0, IADCDevice *p1, short expType)
{
	/*
	expType - тип эксперимента
	p0 - указатель на интерфейс драйвера
	p1 - указатель на интерфейс драйвера
	*/
	__int32 i;

	preS_pUtil = p0; p0 = NULL;//копии ссылок
	preS_pADC = p1; p1 = NULL;//копии ссылок

	//устанавливаем необходимые параметры кнопок
	GetASignal->Enabled = true;//кнопка примера сигнала
	StartRec->Enabled = true;//кнопка запуска сбора
	ExpInfo->Enabled = true;//кнопка информации об эксперименте

	CloseWin->Caption = "Закрыть";//меняем "функцию" кнопки
	CloseWin->Tag = 1;//меняем функцию кнопки

	//стираем все линии с графика
	preTLine->Clear();
	postTLine->Clear();
	porogLine->Clear();
	for (i = 0; i < maxChannels; i++)
		exmplChannls[i]->Clear();

	ExpNotes->usersNotes->Clear();//затираем записи в блокноте
	typeOfExp = expType;//копируем тип эксперимента в глобальную переменную

	if (typeOfExp == 1)//спонтанные сигналы
	{
		PStart->Caption = "Предстарт: Спонтанные";
		PreTSel->Position = PreTSel->Min;
		PostTSel->Position = PostTSel->Min;
	}
	else if (typeOfExp == 2)//вызванные (внутриклеточные)
	{
		PStart->Caption = "Предстарт: Вызванные-ВНУТРИклеточные";
		PreTime->Text = "0";//установим в 0 линию preTime, чтобы не мешала
	}
	else if (typeOfExp == 3)//вызванные (внеклеточные)
		PStart->Caption = "Предстарт: Вызванные-ВНЕклеточные";
	else if (typeOfExp == 4)//вызванные+спонтанные
		PStart->Caption = "Предстарт: Вызванные + Спонтанные";
	else//ошибка типа эксперимента
	{
		Experiment->DevEvents->Text = "ошибка типа эксперимента";
		onCloseWin(this);//окно так и не откроется, поэтому закроем все переменные
	}

	Graphs->Show();//делаем видимым окно графиков
	PStart->Show();//открываем окно предстарта
	Experiment->DevEvents->Text = "OK";
}
//---------------------------------------------------------------------------

void __fastcall TPStart::StartRecClick(TObject *Sender)
{
	//задаём режим записи: 1 - примерный сигнал или 2 - запуск сценария
	__int32 i,
			lastPnt;//номер последней точки на графике
	AnsiString theFileName,//полное имя файла
			   theDir,//указанная директория
			   anpth;//системный диск
	TDateTime CurrentDateTime;//текущие дата и время

	GetASignal->Tag = 2;//выбран запуск сценария

	theFileName = Experiment->FilePathName->Text;//записываем новое имя из строки filePathName на главной форме
	theDir = ExtractFileDir(theFileName);

	//если строка пустая или директория не существует, генерируем случайное имя (по дате и времени)
	if ((theFileName.IsEmpty()) || (!DirectoryExists(theDir)))
	{
		for (i = 0; i < 15; i++)
		{
			anpth = char(i + 67);//char(67) = "C" - буква системного диска
			anpth += ":\\";
			if (GetDriveType(anpth.c_str()) == DRIVE_FIXED)//
			{
				//anpth += "WINDOWS";//сохранение в папку виндоуса
				if (DirectoryExists(anpth))
				{
					anpth.Delete(4, anpth.Length());
					break;
				}
			}
		}
		CurrentDateTime = Now();//текущие дата и время
		theFileName = anpth + "Signal_" + CurrentDateTime.FormatString("yyyy-mm-dd_hh-nn-ss") + "." + Experiment->defltExtPraDrr;
		Experiment->FilePathName->Text = theFileName;//отображаем имя файла на главной форме
	}

	Graphs->SaveDlg->FileName = theFileName;//итоговое имя файла
	Experiment->DiscontinWrt->Tag = 0;//указывает какой вариант выбран: 0 - создать заново или 1 - дописать
	Graphs->plotFrom = 0;//продолжаем рисовать с этого момента времени

	//если данный файл существует, предложим выбрать другой
	if (FileExists(Graphs->SaveDlg->FileName.c_str()))
	{
		ChoosDlg->setsForFileChs();//настраиваем окно диалога для запроса о выборе файла
		i = ChoosDlg->ShowModal();

		if (i == mrOk)//выбрать другой файл
			Experiment->SaveToClick(this);
		else if (i == mrYes)//дописать данные в существующий файл
		{
			Experiment->DiscontinWrt->Tag = 1;//выбрано дописать в существующий файл

			lastPnt = Graphs->sigAmpls[0]->XValues->Count() - 1;
			if (lastPnt >= 0)
				Graphs->plotFrom = Graphs->sigAmpls[0]->XValues->operator [](lastPnt) + 1;//продолжаем рисовать с этого момента времени
		}
		else if (i == mrIgnore)//сохранить с заменой
			Experiment->DiscontinWrt->Tag = 0;//выбрано заменить существующий файл (графики затрутся)
		//else if (i == mrCancel)//отменить запуск сценария
	}

	if (i != mrCancel)//если задание не было отменено
		StartOfRecordThread(this);//модуль запуска параллельного потока для записи (сценарий)
}
//---------------------------------------------------------------------------

void __fastcall TPStart::GetASignalClick(TObject *Sender)
{
	//задаём режим записи: 1 - примерный сигнал или 2 - запуск сценария
	GetASignal->Tag = 1;//выбрано получение примера сигнала
	Experiment->DiscontinWrt->Tag = 1;//графики останутся в окнах

	StartOfRecordThread(this);//модуль запуска параллельного потока для записи (сценарий)
}
//---------------------------------------------------------------------------

void __fastcall TPStart::StartOfRecordThread(TObject *Sender)
{
	//старт потока RecordThread
	__int32 i;
	AnsiString singlMark;//метка начала блока
	bool mOnChn[maxChannels];//привязка меток к каналам

	if (!Experiment->InitDevice())//ошибка инициализации
		return;

	Experiment->DevEvents->Text = "OK";

	//перед началом нового эксперимента стираем данные о последнем открытом
	Graphs->ClearMemor(this);//удаление всех старых данных из памяти
	Graphs->csSpecPoint->Clear();//затираем спец-точки

	//информация о текущем сеансе записи
	ExpNotes->nmInRec = ExpNotes->addUMark->Tag;//количество меток записанных в предыдущих сеансах сбора данных (дозапись в существующий файл)
	ExpNotes->npNewRec = Graphs->sigAmpls[0]->Count();//количество сигналов, записанных в предыдущих сеансах сбора данных (дозапись в существующий файл)
	if ((GetASignal->Tag == 2) && (Experiment->DiscontinWrt->Tag == 1))//если выбрано дописать в существующий файл
	{
		//сразу ставим заметку на первый сигнал в новом блоке
		singlMark = "нБ";//метка начала блока //singlMark += IntToStr(nBlocks);//номер блока
		for (i = 0; i < maxChannels; i++)//привязываем метку ко всем каналам
			mOnChn[i] = true;//привязываем метку ко всем каналам
		ExpNotes->AddMarker(singlMark, ExpNotes->npNewRec, &mOnChn[0]);
	}

	preRecThread->AssignVar(typeOfExp, preS_pUtil, preS_pADC);
	CloseWin->SetFocus();//наводим фокус на кнопку остановки эксперимента
	preRecThread->Resume();//запускаем приостановленный поток preRecThread
}
//---------------------------------------------------------------------------

void TPStart::PlotStandardSignal(double *sData, __int32 sRecLen, float sEffDT, __int32 sIndBgn)
{
	//рисуем быстро эталонный сигнал
	/*
	sData - массив с графиками
	sRecLen - длина сигнала (отсчёты)
	sEffDT - эффективное время дискретизации = discrT * chanls
	sIndBgn - начальный отсчёт сигнала
	*/

	__int32 i,
			ns1,//первый рисуемый канал
			ns2;//последний рисуемый канал

	ns1 = Graphs->gRF[0];//первый рисуемый канал
	ns2 = Graphs->gRF[1];//последний рисуемый канал
	StandardSgnl->AutoRepaint = false;//отключаем автопрорисовку для ускорения процесса рисования

	for (i = 0; i < maxChannels; i++)//рисуем указанные каналы
		exmplChannls[i]->Clear();//затираем старый график
	for (i = ns1; i < ns2; i++)//рисуем указанные каналы
		exmplChannls[i]->AddArray(&sData[sIndBgn + ((i - ns1) * sRecLen)], (sRecLen - sIndBgn) - 1);//добавляем новые точки

	StandardSgnl->BottomAxis->SetMinMax(0, double((sRecLen - sIndBgn) * sEffDT) / 1000);//устанавливаем пределы горизонтальной оси
	StandardSgnl->AutoRepaint = true;//возвращаем автопрорисовку, чтобы график обновился
	StandardSgnl->Repaint();//Refresh();//обновляем чарт

	sData = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TPStart::AmpPorogSelClick(TObject *Sender, TUDBtnType Button)
{
	Porog->Text = IntToStr(AmpPorogSel->Position);
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PreTSelClick(TObject *Sender, TUDBtnType Button)
{
	if (PreTSel->Position >= PostTSel->Position)
		PreTSel->Position = PostTSel->Position - PostTSel->Increment;

	PreTime->Text = IntToStr((__int32)(PreTSel->Position * multipler));
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PostTSelClick(TObject *Sender, TUDBtnType Button)
{
	if (PostTSel->Position <= PreTSel->Position)
		PostTSel->Position = PreTSel->Position + PreTSel->Increment;

	PostTime->Text = IntToStr((__int32)(PostTSel->Position * multipler));
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PreTimeChange(TObject *Sender)
{
	//перерисовываем ПРЕ-линию
	__int32 i,
			preTnow, postTnow;
	float loc_max,//максимальное значение на каналах
		  loc_min;//минимальное значение на каналах

	loc_max = exmplChannls[0]->MaxYValue();//максимальное значение на каналах
	loc_min = exmplChannls[0]->MinYValue();//максимальное значение на каналах
	for (i = 1; i < maxChannels; i++)
	{
		if (exmplChannls[i]->MaxYValue() > loc_max)
			loc_max = exmplChannls[i]->MaxYValue();
		if (exmplChannls[i]->MinYValue() < loc_min)
			loc_min = exmplChannls[i]->MinYValue();
	}
	
	preTnow = StrToInt(PreTime->Text);
	postTnow = StrToInt(PostTime->Text);

	if (preTnow >= postTnow)
		preTnow = postTnow - PostTSel->Increment;

	PreTSel->Position = (short)(((float)preTnow) / multipler);

	preTLine->Clear();
	preTLine->AddXY((((float)(PreTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_min);
	preTLine->AddXY((((float)(PreTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_max);
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PostTimeChange(TObject *Sender)
{
    //прерисовываем ПОСТ-линию
	__int32 i,
			preTnow, postTnow;
	float loc_max,//максимальное значение на каналах
		  loc_min;//минимальное значение на каналах

	loc_max = exmplChannls[0]->MaxYValue();//максимальное значение на каналах
	loc_min = exmplChannls[0]->MinYValue();//максимальное значение на каналах
	for (i = 1; i < maxChannels; i++)
	{
		if (exmplChannls[i]->MaxYValue() > loc_max)
			loc_max = exmplChannls[i]->MaxYValue();
		if (exmplChannls[i]->MinYValue() < loc_min)
			loc_min = exmplChannls[i]->MinYValue();
	}

	postTnow = StrToInt(PostTime->Text);
	preTnow = StrToInt(PreTime->Text);

	if (postTnow <= preTnow)
		postTnow = preTnow + PreTSel->Increment;

	PostTSel->Position = short((float)postTnow / multipler);

	postTLine->Clear();
	postTLine->AddXY((((float)(PostTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_min);
	postTLine->AddXY((((float)(PostTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_max);
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PorogChange(TObject *Sender)
{                                                   
	//меняем положение линии порогового напряжения согласно выставленному значению
	__int32 i,
			porogSet;
	float loc_max,//максимальное значение на каналах
		  loc_min;//минимальное значение на каналах

	loc_max = exmplChannls[0]->MaxYValue();//максимальное значение на каналах
	loc_min = exmplChannls[0]->MinYValue();//максимальное значение на каналах
	for (i = 1; i < maxChannels; i++)
	{
		if (exmplChannls[i]->Count() > 0)
		{
			if (exmplChannls[i]->MaxYValue() > loc_max)//наиден новый максимум
				loc_max = exmplChannls[i]->MaxYValue();//новый максимум
			if (exmplChannls[i]->MinYValue() < loc_min)//наиден новый минимум
				loc_min = exmplChannls[i]->MinYValue();//новый минимум
		}
	}
	porogSet = StrToInt(Porog->Text);//вставляем текст

	if (porogSet < AmpPorogSel->Min)//сравниваем с нижним пределом
		porogSet = AmpPorogSel->Min;//вставляем текст
	if (porogSet > AmpPorogSel->Max)//сравниваем с верхним пределом
		porogSet = AmpPorogSel->Max;//вставляем текст

	AmpPorogSel->Position = porogSet;//текущее положение линии порога

	porogLine->Clear();//затираем линию порога
	preTLine->Clear();//затираем вертикальную линию
	postTLine->Clear();//затираем вертикальную линию
	
	porogLine->AddXY(0, (AmpPorogSel->Position));//рисуем линию порога
	porogLine->AddXY(((float(PostTSel->Max) * multipler) - timeOfDrawBgn) / 1000, AmpPorogSel->Position);//рисуем линию порога

	preTLine->AddXY(((float(PreTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_min);//рисуем вертикальную линию
	preTLine->AddXY(((float(PreTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_max);//рисуем вертикальную линию

	postTLine->AddXY(((float(PostTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_min);//рисуем вертикальную линию
	postTLine->AddXY(((float(PostTSel->Position) * multipler) - timeOfDrawBgn) / 1000, loc_max);//рисуем вертикальную линию
}
//---------------------------------------------------------------------------

void TPStart::RefreshLines(float rlEffDT)
{
	//обновление линий, разделяющих различные фазы сигналов
	/*
	rlEffDT - эффективное время дискретизации = discrT * chanls
	*/
	__int32 i;

	//множитель multipler нужен для преодоления узости интервала изменения параметра position в UpDown
	
	PreTSel->Min = 0;
	PostTSel->Min = 0;
	if ((PStart->exmplChannls[0]->Count() * rlEffDT) > 32000)
	{
		PreTSel->Max = 32000;//в микросекундах
		PostTSel->Max = 32000;//в микросекундах
		multipler = (float)(PStart->exmplChannls[0]->Count() * rlEffDT) / 32000;//множитель для уточнения положения линий
	}
	else
	{
		PreTSel->Max = PStart->exmplChannls[0]->Count() * rlEffDT;//в микросекундах
		PostTSel->Max = PStart->exmplChannls[0]->Count() * rlEffDT;//в микросекундах
		multipler = 1;//множитель для уточнения положения линий
	}
	PreTSel->Increment = timeIncrement;
	PostTSel->Increment = timeIncrement;

	PorogChange(this);
}
//---------------------------------------------------------------------------

void __fastcall TPStart::ExpInfoClick(TObject *Sender)
{
	//вызываем окно для записи информации об эксперименте

	//настроим элементы окна
	ExpNotes->mainLinesLbl->Caption = "Общие сведения об эксперименте";
	ExpNotes->usersNotes->Visible = true;//поле для пользовательских данных
	ExpNotes->addUMark->Visible = true;//поле ввода заметок по ходу эксперимента
	ExpNotes->addMarkLbl->Visible = true;//заметки-лэйбл
	ExpNotes->PIDates->Visible = false;//поле данных о файле

	ExpNotes->Show();
}
//---------------------------------------------------------------------------

void __fastcall TPStart::SignalLenChange(TObject *Sender)
{
	//проверяем можно ли принять запрашиваемую длину вызванного сигнала
	__int32 sLen;//длина вызванного сигнала

	sLen = StrToInt(SignalLen->Text);//длина вызванного сигнала
	if (Experiment->DiscontinWrt->Checked)//выбрана прямая запись на диск
	{
		//ProtoBuild->Periods->Tag - наименьший период между импульсами стимуляции
		if ((ProtoBuild->Periods->Tag - sLen) < minFreeTime)//слишком короткий интервал между сигналами
		{
			SignalLen->Text = IntToStr(sLen - 1);//уменьшаем длину сигнала
			return;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TPStart::LenSpontChange(TObject *Sender)
{
	//проверяем можно ли принять запрашиваемую длину спонтанного сигнала

	__int32 backTimLen,//время назад для спонтанного сигнала
			sLen;//полная длина записываемого сигнала (миллисекунды)

	sLen = StrToInt(LenSpont->Text);//длина сигнала в миллисекундах
	backTimLen = StrToInt(BackTime->Text);//время назад для спонтанного сигнала
	//вводим ограничение на длину записываемого сигнала
	if (sLen > limitSigLen)//длина сигнала должна быть меньше
	{
		LenSpont->Text = IntToStr(limitSigLen);//ограничение длины сигнала
		return;
	}
	if (sLen < 1)//если введено нулевое или отрицательное число
	{
		LenSpont->Text = "1";//вставляем минимальное время
		return;
	}
	if (sLen < backTimLen)//если длина сигнала меньше времи назад
	{
		LenSpont->Text = IntToStr(backTimLen + 1);//делаем длину сигнала больше времени назад
		return;
	}
}
//---------------------------------------------------------------------------

void __fastcall TPStart::BackTimeChange(TObject *Sender)
{
	//проверяем можно ли принять запрашиваемое время назад

	__int32 spntBackLen,//время назад для спонтанного сигнала
			sLen;//полная длина записываемого сигнала (миллисекунды)

	spntBackLen = StrToInt(BackTime->Text);
	sLen = StrToInt(LenSpont->Text);//полная длина сигнала
	if (spntBackLen < 1)//если введено нулевое или отрицательное число
	{
		BackTime->Text = "1";//вставляем минимальное время
		return;
	}
	if (spntBackLen > sLen)//если время назад превышает длину сигнала
	{
		BackTime->Text = IntToStr(sLen - 1);//делаем время назад на одну миллисекунду короче длины сигнала
		return;
	}
}
//---------------------------------------------------------------------------

void __fastcall TPStart::InvertClick(TObject *Sender)
{
	Graphs->ReplotExamplRecalc();//вызываем функцию перерисовки сигнала в инвертированном виде
}
//---------------------------------------------------------------------------

void __fastcall TPStart::OffLineInvert(TObject *Sender)
{
	//инвертирование в режиме "Постобработки"
	postCompInvert = 1 - (2 * (short(Invert->Checked)));//новое состояние флага инвертирования
	TimeBarChange(this);//рисуем сигнал заново
}
//---------------------------------------------------------------------------

void __fastcall TPStart::ReCalClick(TObject * Sender)
{
	PStart->ModalResult = mrOk;//нажата кнопка пересчёта или начала поиска сигналов
}
//---------------------------------------------------------------------------

void __fastcall TPStart::CloseWinClick(TObject *Sender)
{
	if (CloseWin->Tag == 1)
		PStart->Close();//закрываем окном
	else
		CloseWin->Tag = 1;//меняем функцию кнопки
}
//---------------------------------------------------------------------------

void __fastcall TPStart::NextBlockClick(TObject *Sender)
{
	NextBlock->Tag = 1;//при нажатии на кнопку "сл. блок" происходит переход к следующему блоку протокола
}
//---------------------------------------------------------------------------

void __fastcall TPStart::onCloseWin(TObject *Sender)
{
	//закрываем окно
	if (preS_pUtil != NULL)
		preS_pUtil = NULL;
	else if (preS_pADC != NULL)
		preS_pADC = NULL;

    //сохраняем параметры
	Experiment->progPrms[2] = StrToInt(PStart->LenSpont->Text);//2 - длина спонтанного сигнала
	Experiment->progPrms[3] = StrToInt(PStart->Porog->Text);//3 - порог детекции
	Experiment->progPrms[4] = StrToInt(PStart->BackTime->Text);//4 - время назад
	Experiment->progPrms[5] = StrToInt(PStart->SignalLen->Text);//5 - длина сигнала
}
//---------------------------------------------------------------------------

void __fastcall TPStart::PausSpontanClick(TObject *Sender)
{
	//приостановка или продолжение записи спонтанных сигналов в режиме №4

	//если pausMini->Tag == 0, то продолжаем запись спонтанных сигналов
	//если pausMini->Tag == 1, то приостанавливаем

	if (PausSpontan->Tag == 0)
	{
		PausSpontan->Tag = 1;//ставим на паузу
		PausSpontan->Caption = "старт спнт";
	}
	else//pausMini->Tag == 1
	{
		PausSpontan->Tag = 0;//продолжаем запись
		PausSpontan->Caption = "пауза спнт";
    }
}
//---------------------------------------------------------------------------

void __fastcall TPStart::StandardSgnlMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
	//при нажатии правой кнопки мышки определяем, какая линия ближе к точке
	//нажатия, и двигем её в эту точку
	__int32 chMDexpType,//тип эксперимента
			b,//х-расстояние между ПРЕ-линией и точкой щелчка
			c;//х-расстояние между ПОСТ-линией и точкой щелчка
	double xX;

	chMDexpType = Graphs->ReCalculat->Tag;//тип эксперимента

	if (Button == 1)//если нажата правая кнопка мышки
	{

		b = preTLine->CalcXPosValue(preTLine->XValues->operator [](0)) - X;
		b = b * b;//вместо abs

		c = postTLine->CalcXPosValue(postTLine->XValues->operator [](0)) - X;
		c = c * c;//вместо abs

		if ((b < c) && (chMDexpType == 3))
		{
			xX = preTLine->XScreenToValue(X);//можно взять любой Series
			PreTime->Text = IntToStr((__int32)(floor(xX * 1000) + timeOfDrawBgn));
		}
		else
		{
			xX = postTLine->XScreenToValue(X);//можно взять любой Series
			PostTime->Text = IntToStr((__int32)(floor(xX * 1000) + timeOfDrawBgn));
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TPStart::CheckForKeyPStartEdits(TObject *Sender, char &Key)
{
	//проверка допустимости введения символа, соответствующего нажатой клавише
	//для остальных полей ввода числовых данных

	if (((Key < '0') || (Key > '9')) && (Key != '\b'))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void __fastcall TPStart::CheckForKeyPPorogEdit(TObject *Sender, char &Key)
{
	//проверка допустимости введения символа, соответствующего нажатой клавише
	//для поля амплитудного порога для сигналов

	if (((Key < '0') || (Key > '9')) && (Key != '\b') && (Key != '-'))//проверим допустимость введённых символов
		Key = '\0';//зануляем некорректный символ
}
//---------------------------------------------------------------------------

void TPStart::SetControlVis(short expType)
{
	//устанавливаем видимость контролов в зависимости от типа эксперимента
	/*
	expType - тип эксперимента
	*/

	if (expType == 1)//спонтанные сигнал
	{
		SignalLen->Visible = false;//выбора длины вызванного сигнала
		SLenLbl1->Visible = false;//длина вызванного сигнал (лэйбл 1)
		SLenLbl2->Visible = false;//длина вызванного сигнал (лэйбл 2)
		SLenLbl3->Visible = false;//длина вызванного сигнал (лэйбл 3)

		LenSpont->Visible = true;//выбор длины спонтанного сигнала
		LenSpntLbl1->Visible = true;//длина спонтанного сигнала (лэйбл 1)
		LenSpntLbl2->Visible = true;//длина спонтанного сигнала (лэйбл 2)
		LenSpntLbl3->Visible = true;//длина спонтанного сигнала (лэйбл 3)

		BackTime->Visible = true;//выбор времени назад
		BckTmLbl1->Visible = true;//временя назад (лэйбл 1)
		BckTmLbl2->Visible = true;//временя назад (лэйбл 2)
		BckTmLbl3->Visible = true;//временя назад (лэйбл 3)

		PreTime->Visible = false;//выбор пре-времени (эдитор)
		PreTSel->Visible = false;//выбор пре-времени (кнопки)
		PreTimeLbl->Visible = false;//пре-время (лэйбл)

		PostTime->Visible = false;//выбор пост-времени (эдитор)
		PostTSel->Visible = false;//выбор пост-времени (кнопки)
		PostTimeLbl->Visible = false;//пост-времени (лэйбл)

		PausSpontan->Visible = false;//кнопка паузы сбора спонтанных сигналов в режиме №4
	}
	else if ((expType == 2) || (expType == 3))//вызванные сигналы (внутри- или внеклеточные)
	{
		SignalLen->Visible = true;//выбора длины вызванного сигнала
		SLenLbl1->Visible = true;//длина вызванного сигнал (лэйбл 1)
		SLenLbl2->Visible = true;//длина вызванного сигнал (лэйбл 2)
		SLenLbl3->Visible = true;//длина вызванного сигнал (лэйбл 3)

		LenSpont->Visible = false;//выбор длины спонтанного сигнала
		LenSpntLbl1->Visible = false;//длина спонтанного сигнала (лэйбл 1)
		LenSpntLbl2->Visible = false;//длина спонтанного сигнала (лэйбл 2)
		LenSpntLbl3->Visible = false;//длина спонтанного сигнала (лэйбл 3)

		BackTime->Visible = false;//выбор времени назад
		BckTmLbl1->Visible = false;//временя назад (лэйбл 1)
		BckTmLbl2->Visible = false;//временя назад (лэйбл 2)
		BckTmLbl3->Visible = false;//временя назад (лэйбл 3)

		PreTime->Visible = (expType == 3);//выбор пре-времени (эдитор)
		PreTSel->Visible = (expType == 3);//выбор пре-времени (кнопки)
		PreTimeLbl->Visible = (expType == 3);//пре-время (лэйбл)

		PostTime->Visible = true;//выбор пост-времени (эдитор)
		PostTSel->Visible = true;//выбор пост-времени (кнопки)
		PostTimeLbl->Visible = true;//пост-времени (лэйбл)

		PausSpontan->Visible = false;//кнопка паузы сбора спонтанных сигналов в режиме №4
	}
	else if (expType == 4)//вызванные + спонтанные
	{
        SignalLen->Visible = true;//выбора длины вызванного сигнала
		SLenLbl1->Visible = true;//длина вызванного сигнал (лэйбл 1)
		SLenLbl2->Visible = true;//длина вызванного сигнал (лэйбл 2)
		SLenLbl3->Visible = true;//длина вызванного сигнал (лэйбл 3)

		LenSpont->Visible = true;//выбор длины спонтанного сигнала
		LenSpntLbl1->Visible = true;//длина спонтанного сигнала (лэйбл 1)
		LenSpntLbl2->Visible = true;//длина спонтанного сигнала (лэйбл 2)
		LenSpntLbl3->Visible = true;//длина спонтанного сигнала (лэйбл 3)

		BackTime->Visible = true;//выбор времени назад
		BckTmLbl1->Visible = true;//временя назад (лэйбл 1)
		BckTmLbl2->Visible = true;//временя назад (лэйбл 2)
		BckTmLbl3->Visible = true;//временя назад (лэйбл 3)

		PreTime->Visible = false;//выбор пре-времени (эдитор)
		PreTSel->Visible = false;//выбор пре-времени (кнопки)
		PreTimeLbl->Visible = false;//пре-время (лэйбл)

		PostTime->Visible = true;//выбор пост-времени (эдитор)
		PostTSel->Visible = true;//выбор пост-времени (кнопки)
		PostTimeLbl->Visible = true;//пост-времени (лэйбл)

		PausSpontan->Visible = true;//кнопка приостановки сбора спонтанных сигналов
	}
	else
    	Experiment->DevEvents->Text = "Ошибка типа эксперимента";//сообщение об ошибке
}
//---------------------------------------------------------------------------

__int32 TPStart::winDraw(__int32 samplPerCh)
{
	//вызываем окно предстарта для рисования ранее записанного сигнала
	/*
	samplPerCh - количество отсчётов на один канал
	*/
	__int32 i;

	//устанавливаем необходимые параметры кнопок
	StartRec->Enabled = true;//кнопка старта
	CloseWin->Caption = "Закрыть";//меняем "функцию" кнопки
	CloseWin->Tag = 1;//меняем функцию кнопки

	//стираем все линии с графика
	preTLine->Clear();
	postTLine->Clear();
	porogLine->Clear();
	ExpNotes->usersNotes->Clear();//затираем записи в блокноте

	PStart->Caption = "Постобработка";//режим работы
	SetControlVis(1);//делаем видимыми нужные органы управления и убираем ненужные

	GetASignal->Visible = false;//кнопка пример сигнала
	ExpInfo->Visible = false;//кнопка пример сигнала
	StartRec->OnClick = ReCalClick;//меняем реакцию на нажатие
	postCompInvert = 1 - (2 * (short(Invert->Checked)));//текущее состояние флага инвертирования
	Invert->OnClick = OffLineInvert;//изменение реакции на смену индикатора

	TimeScaleBar->Max = 5e4;//максимальная длина окна (миллисекунды)
	TimeScaleBar->Min = 500;//минимальная длина окна (миллисекунды)
	TimeScaleBar->PageSize = 50;//шаг изменения длины окна (миллисекунды)
	TimeScaleBar->Position = 500;//начальная длина окна (миллисекунды)
	TimeScaleBar->Visible = true;//делаем контрол видимым
	TimeBar->Max = (samplPerCh * Graphs->effDT * 1e-3) - TimeScaleBar->Position;//максимальное начальное время рисования (миллисекунды)
	TimeBar->Min = 0;//минимальное начальное время рисования (миллисекунды)
	TimeBar->PageSize = 1e3;//шаг изменения длины окна (миллисекунды)
	TimeBar->Position = 0;//начальное время рисования (миллисекунды)
	TimeBar->Visible = true;//делаем контрол видимым

	PostTSel->Max = 32000;//левый край графика
	PreTSel->Position = 0;//убираем вертикальные линии
	PostTSel->Position = 0;//убираем вертикальные линии
	multipler = (float)(exmplChannls[0]->Count() * Graphs->effDT) / 32000;//множитель для уточнения положения линий

	AmpPorogSel->Max = short(Experiment->maxADCAmp * Graphs->sampl2mV);//милливольты
	AmpPorogSel->Min = (-1) * AmpPorogSel->Max;//милливольты
	AmpPorogSel->Increment = (AmpPorogSel->Max / 1000) + (__int32(AmpPorogSel->Max <= 1000));//делим диапазон на 1000 шагов

	TimeBarChange(this);//рисование начального участка

	Graphs->Show();//делаем видимым окно графиков
	i = PStart->ShowModal();//открываем окно предстарта

	GetASignal->Visible = true;//кнопка пример сигнала
	ExpInfo->Visible = true;//кнопка пример сигнала
	StartRec->OnClick = StartRecClick;//восстанавливаем реакцию на нажатие
	Invert->OnClick = NULL;//изменение реакции на смену индикатора

	TimeScaleBar->Visible = false;
	TimeBar->Visible = false;

	return i;//возвращаем код команды ("отмена" или "принять")
}
//---------------------------------------------------------------------------

void __fastcall TPStart::TimeBarChange(TObject *Sender)
{
	//реакция на изменение масштаба или времени

	__int32 i, j,
			startInd,//начальный отсчёт
			windowLen;//длина окна рисования (отсчёты)
	float eff_dt,//эффективное время дискретизации = discrT * chanls (миллисекунды)
		  sampl2mV;//коэффициент пересчёта отсчётов в милливольты
	double *grafik;//массив с куском записи

	eff_dt = Graphs->effDT * 1e-3;//эффективное время дискретизации = discrT * chanls (миллисекунды)
	startInd = floor(TimeBar->Position / eff_dt);//начальный отсчёт
	windowLen = floor(TimeScaleBar->Position / eff_dt);//длина окна рисоания (отсчёты)
	sampl2mV = Graphs->sampl2mV;//коэффициент пересчёта отсчётов в милливольты

	grafik = new double[windowLen * Graphs->chanls];//массив с куском записи

	for (i = 0; i < Graphs->chanls; i++)//рисуем указанные каналы
		for (j = 0; j < windowLen; j++)
			grafik[(i * windowLen) + j] = (postCompInvert * continRec[i][startInd + j]) * sampl2mV;
	//PlotStandardSignal(double *sData, __int32 sRecLen, float sEffDT, __int32 sIndBgn)
	PlotStandardSignal(grafik, windowLen, Graphs->effDT, 0);
	multipler = (float(exmplChannls[0]->Count() * Graphs->effDT)) / 32000;//множитель для уточнения положения линий

	PorogChange(this);//перерисуем линии
	delete[] grafik; grafik = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TPStart::FormActivate(TObject *Sender)
{
	//после окончания создания окна предстарта применяем файл настроек
	//если существовал файл настроек SetsElph.ini, возьмём из него данные
	PStart->LenSpont->Text = IntToStr(Experiment->progPrms[2]);//длина спонтанного сигнала
	PStart->Porog->Text = IntToStr(Experiment->progPrms[3]);//порог
	PStart->BackTime->Text = IntToStr(Experiment->progPrms[4]);//время назад
	PStart->SignalLen->Text = IntToStr(Experiment->progPrms[5]);//длина вызванного сигнала
	PStart->Top = Experiment->progPrms[13];//верх окна
	PStart->Left = Experiment->progPrms[14];//левый край окна
	PStart->OnActivate = NULL;//отключаем реагирование главной формы на "активацию"
}
//---------------------------------------------------------------------------

