#include "Run.h"

//#include <direct.h> 

//#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_audio.h>
#include <iostream>
#include <fstream>

#include "CDcpParse.h"


using namespace std;

Run::Run( wxWindow* parent )
:
RunDlg( parent ),  m_MyDir(_T(""))
, m_Com(_T(""))
, m_Back(TRUE)
, m_Log(TRUE)
, m_ProgressBar(TRUE)
, m_Output51(FALSE)
{

}

void Run::RunDcpPlayerDlgOnInitDialog( wxInitDialogEvent& event )
{
	strcpy(cwd, "");
	InitialDir = "";
	ChoosenDir = "";
	const char* Proposition;


	int n = 0;
	do
	{
		Proposition = SDL_GetAudioDeviceName(n, 0);
		wxString mystring = wxString::FromUTF8(Proposition);
		if (Proposition) AudioDevice.push_back(mystring);
		n++;
	} while (Proposition && n < 50);
	n = 0;
	do
	{
		Proposition = SDL_GetDisplayName(n);
		wxString mystring = wxString::FromUTF8(Proposition);
		if (Proposition) DisplayDevice.push_back(mystring);
		n++;
	} while (Proposition && n < 50);

	SDL_Quit();

	for (int i = 0; i < AudioDevice.size(); i++)
	{
		m_choiceAudio->Append(AudioDevice[i]);
	}
 
	for (int i = 0; i < DisplayDevice.size(); i++)
	{
		wxString temp;
		temp.Printf(" (%d)",i);
		//temp += " (" + to_string(i) + ")";
		m_choiceDisplay->Append(DisplayDevice[i]+temp);
	}

#if defined(WIN64) || defined(_WIN64)
	_getcwd(cwd, MAX_PATH);
#else
	getcwd(cwd, MAX_PATH);
#endif
	InitialDir = string(cwd);

	string line;
	ifstream read("config.txt");
	if (read.good())
	{
		getline(read, ChoosenDir);

		getline(read, line);
		m_choiceAudio->SetSelection(atoi(line.c_str()));
		getline(read, line);
		m_choiceDisplay->SetSelection(atoi(line.c_str()));
		getline(read, line);
		if (line == "0") m_Back = false; else m_Back = true;
		getline(read, line);
		if (line == "0") m_Log = false; else m_Log = true;
		getline(read, line);
		if (line == "0") m_ProgressBar = false; else m_ProgressBar = true;
		getline(read, line);
		if (line == "0") m_Output51 = false; else m_Output51 = true;
		//_chdir(ChoosenDir.c_str());
	}

	else
	{

		m_choiceAudio->SetSelection(0);
		m_choiceDisplay->SetSelection(0);
		m_choiceCPL->SetSelection(0);
		ChoosenDir = "";
	}

#if defined(WIN64) || defined(_WIN64)
	_getcwd(cwd, MAX_PATH);
#else
	getcwd(cwd, MAX_PATH);
#endif

	wxString Dir(cwd);
	#if defined(WIN64) || defined(_WIN64)
	m_Com.Printf("FreeDcpPlayer.exe \"%s\" -a %d -d %d", ChoosenDir.c_str(), m_choiceAudio->GetCurrentSelection(), m_choiceDisplay->GetCurrentSelection());
	#else 
	m_Com.Printf("./freedcpplayer \"%s\" -a %d -d %d", ChoosenDir.c_str(), m_choiceAudio->GetCurrentSelection(), m_choiceDisplay->GetCurrentSelection());
	#endif

	UpdateData(false);
 }

void Run::m_dirPicker2OnDirChanged( wxFileDirPickerEvent& event )
{
	CDcpParse DcpParse(true);
	//filesystem::path cdir(m_dirPicker2->GetPath());
	//cdir = cdir.parent_path();
	//string Temp{ cdir.string() };
	ChoosenDir = string(m_dirPicker2->GetPath());

	//std::string temp(ChoosenDir);
	if (ChoosenDir != "")
	{
		//_chdir(ChoosenDir.c_str());
		UpdateData(true);
		m_MyDir.Format("%s", ChoosenDir.c_str());
		UpdateData(false);

	}

#if defined(WIN64) || defined(_WIN64)
	_chdir(InitialDir.c_str());
#else
	chdir(InitialDir.c_str());
#endif


	vector<string> MXFFiles;
	DcpParse.ParseDCP(MXFFiles, ChoosenDir);
	m_choiceCPL->Clear();

	for (int i = 0; i < DcpParse.CplVector.size(); i++)
	{
		string temp(DcpParse.CplVector[i]->sAnnotation);
		temp += " (" + to_string(i) + ")";
		m_choiceCPL->Append(temp.c_str());
	}
	m_choiceCPL->SetSelection(0);
	UpdateCommand();
}

void Run::m_choiceAudioOnChoice( wxCommandEvent& event )
{
	UpdateCommand();
}

void Run::m_choiceDisplayOnChoice( wxCommandEvent& event )
{
	UpdateCommand();
}

void Run::m_choiceCPLOnChoice( wxCommandEvent& event )
{
	UpdateCommand();
}

void Run::m_checkBox_full_screenOnCheckBox( wxCommandEvent& event )
{
	UpdateCommand();
}

void Run::m_checkBox_ProgressOnCheckBox( wxCommandEvent& event )
{
	UpdateCommand();
}

void Run::m_checkBox_logOnCheckBox( wxCommandEvent& event )
{
	UpdateCommand();
}

void Run::m_checkBox_51OnCheckBox( wxCommandEvent& event )
{
	UpdateCommand();
}

void Run::m_button_runOnButtonClick( wxCommandEvent& event )
{
	UpdateCommand();
	string Config = InitialDir + string("/config.txt");
	FILE* fp = fopen(Config.c_str(), "w");
	fprintf(fp, "%s\n", ChoosenDir.c_str());
	fprintf(fp, "%d\n", m_choiceAudio->GetCurrentSelection());
	fprintf(fp, "%d\n", m_choiceDisplay->GetCurrentSelection());
	if (m_Back) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
	if (m_Log) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
	if (m_ProgressBar) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
	if (m_Output51) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
	fclose(fp);
	string Command = InitialDir + string("/command.txt");
	ofstream CommandFile;
	CommandFile.open (Command,ios::app);
	CommandFile << m_Com << endl;
	CommandFile.close();
	system(m_Com);
}

void Run::m_button_quitOnButtonClick( wxCommandEvent& event )
{
	EndModal(true);
	Destroy();
}


void Run::UpdateCommand()
{
	UpdateData(true);
#if defined(WIN64) || defined(_WIN64)
	_chdir(InitialDir.c_str());
#else
	chdir(InitialDir.c_str());
#endif

	int audio = m_choiceAudio->GetCurrentSelection();
	int display = m_choiceDisplay->GetCurrentSelection();
	int CplIndex = m_choiceCPL->GetCurrentSelection();

	wxString Temp;
	#if defined(WIN64) || defined(_WIN64)
	Temp.Printf("FreeDcpPlayer.exe \"%s\" -a %d -d %d -c %d", ChoosenDir.c_str(), audio, display, CplIndex);
	#else 
	Temp.Printf("./freedcpplayer \"%s\" -a %d -d %d -c %d", ChoosenDir.c_str(), audio, display, CplIndex);
	#endif
	if (!m_Back) Temp += " -g";
	if (m_ProgressBar) Temp += " -i";
	if (m_Output51) Temp += " -o";
	#if defined(WIN64) || defined(_WIN64)
		if (m_Log) Temp = Temp + " -v 2>freedcpplayer.log & pause";
		else  m_Com = Temp + " & pause";
	#else 
		if (m_Log) Temp = Temp + " -v 2>freedcpplayer.log";
	#endif
	m_Com=Temp;
	UpdateData(false);
}

void Run::UpdateData(bool sens)
{
	if (!sens) TransferDataToWindow();
	else TransferDataFromWindow();

}

bool Run::TransferDataToWindow()
{
	m_checkBox_full_screen->SetValue(m_Back);
	m_checkBox_Progress->SetValue(m_ProgressBar);
	m_checkBox_log->SetValue(m_Log);
	m_checkBox_51->SetValue(m_Output51);
	m_staticText_Command->SetLabelText(m_Com);
	return true;

}

bool Run::TransferDataFromWindow()
{
	m_Back = m_checkBox_full_screen->GetValue();
	m_ProgressBar=m_checkBox_Progress->GetValue();
	m_Log=m_checkBox_log->GetValue();
	m_Output51=m_checkBox_51->GetValue();

	return true;

}
