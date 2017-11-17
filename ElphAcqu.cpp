//$$---- EXE CPP ----
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("MainForm.cpp", Experiment);
USEFORM("PredStart.cpp", PStart);
USEFORM("BuildProtocol.cpp", ProtoBuild);
USEFORM("Gistograms.cpp", Gists);
USEFORM("MinorChannel.cpp", SecondChan);
USEFORM("RenamOverr.cpp", ChoosDlg);
USEFORM("TimeCourses.cpp", Graphs);
USEFORM("Oscillograph.cpp", Oscil);
USEFORM("ExperimentNotes.cpp", ExpNotes);
//---------------------------------------------------------------------------
HINSTANCE glbhInst,
		  glbhPrvInst;
LPSTR glbszCmdLine;
int glbiCmdLine;

//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrvInst, LPSTR szCmdLine, int iCmdLine)
{
	try
	{
		glbhInst = hInst;
		glbhPrvInst = hPrvInst;
		glbszCmdLine = szCmdLine;
		glbiCmdLine = iCmdLine;
		
		Application->Initialize();
		Application->Title = "Elph";
		Application->CreateForm(__classid(TExperiment), &Experiment);
		Application->CreateForm(__classid(TPStart), &PStart);
		Application->CreateForm(__classid(TProtoBuild), &ProtoBuild);
		Application->CreateForm(__classid(TGists), &Gists);
		Application->CreateForm(__classid(TSecondChan), &SecondChan);
		Application->CreateForm(__classid(TChoosDlg), &ChoosDlg);
		Application->CreateForm(__classid(TGraphs), &Graphs);
		Application->CreateForm(__classid(TOscil), &Oscil);
		Application->CreateForm(__classid(TExpNotes), &ExpNotes);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
