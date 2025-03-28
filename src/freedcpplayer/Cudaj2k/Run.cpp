/*****************************************************************************
 * Copyright (C) 2022 Karleener
 *
 * Author:  Karleener
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3.0 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "Run.h"

//#include <direct.h> 

//#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_audio.h>
#include <iostream>
#include <fstream>
#include <wx/msgdlg.h>

#include "CDcpParse.h"

using namespace std;

int main_dcpplayer(int argc, const char** argv,bool &IsPlaying);

Run::Run(wxWindow* parent)
	:
	RunDlg(parent), m_MyDir(_T(""))
	, m_Com(_T(""))
	, m_Back(true)
	, m_Log(true)
	, m_ProgressBar(true)
	, m_Output51(false)
	, m_HalfResolution(false)
	, m_Play(false)
	, m_IsPlaying(false)
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
		getline(read, line);
		if (line == "0") m_HalfResolution= false; else m_HalfResolution = true;
		getline(read, line);
		if (line == "0") m_Play = false; else m_Play = true;
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

	m_Com = "Please choose a DCP folder";


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
	if (DcpParse.CplOk && DcpParse.VideoOk && DcpParse.SoundOk && DcpParse.EditRateOk)
	{
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
	else
		m_staticText_Command->SetLabelText("Dcp parsing error, please choose another folder");

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

void Run::m_checkBox_HalfOnCheckBox(wxCommandEvent& event)
{
	UpdateCommand();
}

void Run::m_checkBox_playOnCheckBox(wxCommandEvent& event)
{
	UpdateCommand();
}

void Run::m_slider_volume(wxCommandEvent& event)
{
	UpdateCommand();
 }

void Run::m_button_runOnButtonClick( wxCommandEvent& event )
{
	if (!m_IsPlaying)
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
		if (m_HalfResolution) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
		if (m_Play) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
		fclose(fp);

		m_Com = "Player is running - Press esc to stop before any change";
		m_staticText_Command->SetLabelText(m_Com);

		m_IsPlaying = true;
		int res = main_dcpplayer(argv.size() - 1, (const char**)argv.data(), m_IsPlaying);
		if (res==0)
			m_Com = "Player ended";
		else 
			m_Com = "Player Error, see freedcpplayer.log";
		m_staticText_Command->SetLabelText(m_Com);
	}
	//system(m_Com);
}

void Run::m_button_quitOnButtonClick( wxCommandEvent& event )
{
	if (!m_IsPlaying)
	{
		EndModal(true);
		Destroy();
	}
	else
	{
		m_Com = "Player is running - Please clic on the display window and press 'Esc' to stop the player first";
		m_staticText_Command->SetLabelText(m_Com);
	}
}

void Run::m_button_helpOnButtonClick(wxCommandEvent& event)
{


	const char* mes =
		"o Press space bar for play / pause\n"
		"o Use left and right arrows for fast forward and rewind\n"
		"o Use page up and page down for very fast forward and rewind\n"
		"o Use Up and Down arrows for image per image in paused mode\n"
		"o Use double mouse left click in the picture as an horizontal slider to move forward or backward\n"
		"o Press  ESC key to end the program\n"
		"o Press i for progess bar activation\n"
		"o Press j for fps information\n"
		"o This version is restricted to Uncrypted 5.1  or Stereo \n"
		"o A Cuda based NVIDIA GPU with at least 6GB is required\n\n"
		"Portions of this software are copyright(c) <2006 - 2021> The FreeType\n"
		"Project(www.freetype.org).All rights reserved.\n"
		"Portions of this software are copyright(c) asdcplib 2003 - 2015 John Hurst\n"
		"This software contains source code provided by NVIDIA Corporation.\n"
		"This software is provided 'as-is', without any express or implied\n"
		"warranty.In no event will the authors be held liable for any damages\n"
		"arising from the use of this software.\n"
		"copyright(c) <2022> Johel Miteran - Karleener\n";

	wxMessageBox(mes);


}

void Run::UpdateCommand()
{
	argv.clear();
	arguments.clear();
	UpdateData(true);
	m_Com = "Ready";
#if defined(WIN64) || defined(_WIN64)
	_chdir(InitialDir.c_str());
#else
	chdir(InitialDir.c_str());
#endif

	int audio = m_choiceAudio->GetCurrentSelection();
	int display = m_choiceDisplay->GetCurrentSelection();
	int CplIndex = m_choiceCPL->GetCurrentSelection();
	int AudioGain = m_sliderVolume->GetValue()+6; // -6dB is coded as 0 and +6dB is coded as 12 in order to avoid "-" in command list

#if defined(WIN64) || defined(_WIN64)
	arguments.push_back(InitialDir+"/RunDcpPlayer.exe");
#else
	arguments.push_back(InitialDir + "/RunDcpPlayer");
#endif
	arguments.push_back(ChoosenDir);
	arguments.push_back("-a");
	arguments.push_back(to_string(audio));
	arguments.push_back("-d");
	arguments.push_back(to_string(display));
	arguments.push_back("-c");
	arguments.push_back(to_string(CplIndex));
	arguments.push_back("-l");
	arguments.push_back(to_string(AudioGain));

	if (!m_Back) arguments.push_back("-g");
	if (m_ProgressBar) arguments.push_back("-i");
	if (m_Output51) arguments.push_back("-o");
	if (m_HalfResolution) arguments.push_back("-s");
	if (m_Play) arguments.push_back("-p");
	if (m_Log)
	{
		arguments.push_back("-v");
		arguments.push_back("2>freedcpplayer.log");
	}

	for (const auto& arg : arguments)
		argv.push_back((char*)arg.data());
	argv.push_back(nullptr);

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
	m_checkBox_half->SetValue(m_HalfResolution);
	m_checkBox_play->SetValue(m_Play);
	m_staticText_Command->SetLabelText(m_Com);
	return true;

}

bool Run::TransferDataFromWindow()
{
	m_Back = m_checkBox_full_screen->GetValue();
	m_ProgressBar=m_checkBox_Progress->GetValue();
	m_Log=m_checkBox_log->GetValue();
	m_Output51=m_checkBox_51->GetValue();
	m_Play=m_checkBox_play->GetValue();
	m_HalfResolution = m_checkBox_half->GetValue();

	return true;

}
