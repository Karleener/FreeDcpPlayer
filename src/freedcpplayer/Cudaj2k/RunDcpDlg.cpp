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

#include "RunDcpDlg.h"

///////////////////////////////////////////////////////////////////////////

RunDlg::RunDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );



	m_staticText_DCPDIR = new wxStaticText(this, wxID_ANY, wxT("DCP folder"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText_DCPDIR->Wrap(-1);
	fgSizer1->Add(m_staticText_DCPDIR, 0, wxALL, 5);

	m_dirPicker2 = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Select a folder"), wxDefaultPosition, wxSize(500, -1), wxDIRP_DEFAULT_STYLE );
	fgSizer1->Add( m_dirPicker2, 0, wxALL, 5 );

	// Ajout du texte indicatif pour le volume
	m_staticText_Volume = new wxStaticText(this, wxID_ANY, wxT("Volume (dB)"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(m_staticText_Volume, 0, wxALL, 5);

	// Ajout du slider pour le volume (-6 dB à +6 dB)
	m_sliderVolume = new wxSlider(this, wxID_ANY, 0, -6, 6, wxDefaultPosition, wxSize(500, -1), wxSL_HORIZONTAL | wxSL_LABELS);
	fgSizer1->Add(m_sliderVolume, 0, wxALL, 5);

	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Audio device"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	fgSizer1->Add( m_staticText2, 0, wxALL, 5 );

	wxArrayString m_choiceAudioChoices;
	m_choiceAudio = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 500,100 ), m_choiceAudioChoices, 0 );
	m_choiceAudio->SetSelection( 0 );
	fgSizer1->Add( m_choiceAudio, 0, wxALL, 5 );

	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("Display device"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	fgSizer1->Add( m_staticText3, 0, wxALL, 5 );

	wxArrayString m_choiceDisplayChoices;
	m_choiceDisplay = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 500,100 ), m_choiceDisplayChoices, 0 );
	m_choiceDisplay->SetSelection( 0 );
	fgSizer1->Add( m_choiceDisplay, 0, wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("CPL to be played"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	fgSizer1->Add( m_staticText4, 0, wxALL, 5 );

	wxArrayString m_choiceCPLChoices;
	m_choiceCPL = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 500,100 ), m_choiceCPLChoices, 0 );
	m_choiceCPL->SetSelection( 0 );
	fgSizer1->Add( m_choiceCPL, 0, wxALL, 5 );

	m_checkBox_full_screen = new wxCheckBox( this, wxID_ANY, wxT("Full screen"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_checkBox_full_screen, 0, wxALL, 5 );

	m_checkBox_Progress = new wxCheckBox( this, wxID_ANY, wxT("Progress bar"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_checkBox_Progress, 0, wxALL, 5 );

	m_checkBox_log = new wxCheckBox( this, wxID_ANY, wxT("Record log file"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_checkBox_log, 0, wxALL, 5 );

	m_checkBox_51 = new wxCheckBox( this, wxID_ANY, wxT("Enable 5.1 output"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_checkBox_51, 0, wxALL, 5 );

	m_checkBox_half = new wxCheckBox(this, wxID_ANY, wxT("Enable half resolution decoding"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(m_checkBox_half, 0, wxALL, 5);

	m_checkBox_play= new wxCheckBox(this, wxID_ANY, wxT("Play without initial pause"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(m_checkBox_play, 0, wxALL, 5);
	//wxSize MySize = wxDefaultSize;
	//MySize.SetHeight(MySize.GetHeight() * 2);
	//MySize.SetWidth(MySize.GetWidth() * 2);

	m_button_run = new wxButton( this, wxID_ANY, wxT("Run FreeDcpPlayer"), wxPoint( -1,-1 ), wxDefaultSize, 0 );

	//m_button_run->SetDefault();
	fgSizer1->Add( m_button_run, 0, wxALL, 5 );

	m_button_quit = new wxButton( this, wxID_ANY, wxT("Quit"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_button_quit, 0, wxALL|wxALIGN_RIGHT, 5 );

	m_button_help = new wxButton(this, wxID_ANY, wxT("      Help       "), wxPoint(-1, -1), wxDefaultSize, 0);
	fgSizer1->Add(m_button_help, 0, wxALL, 5);
	//m_staticText_infoCommand = new wxStaticText(this, wxID_ANY, wxT("Command"), wxDefaultPosition, wxDefaultSize, 0);
	//m_staticText_infoCommand->Wrap(-1);
	//fgSizer1->Add(m_staticText_infoCommand, 0, wxALL, 5);

	m_staticText_Command = new wxStaticText(this, wxID_ANY, wxT("Command"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText_Command->Wrap(-1);
	fgSizer1->Add(m_staticText_Command, 0, wxALL, 5);

	m_About1 = new wxStaticText(this, wxID_ANY, wxT("Version 0.6.3.2"), wxDefaultPosition, wxDefaultSize, 0);
	m_About1->Wrap(-1);
	fgSizer1->Add(m_About1, 0, wxALL, 5);

	m_About2 = new wxStaticText(this, wxID_ANY, wxT("By Karleener - 2022"), wxDefaultPosition, wxDefaultSize, 0);
	m_About2->Wrap(-1);
	fgSizer1->Add(m_About2, 0, wxALL, 5);


	this->SetSizer( fgSizer1 );
	this->Layout();
	this->Centre( wxBOTH );
	this->Fit();

	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( RunDlg::RunDcpPlayerDlgOnInitDialog ) );
	m_dirPicker2->Connect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( RunDlg::m_dirPicker2OnDirChanged ), NULL, this );
	m_choiceAudio->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( RunDlg::m_choiceAudioOnChoice ), NULL, this );
	m_choiceDisplay->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( RunDlg::m_choiceDisplayOnChoice ), NULL, this );
	m_choiceCPL->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( RunDlg::m_choiceCPLOnChoice ), NULL, this );
	m_checkBox_full_screen->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RunDlg::m_checkBox_full_screenOnCheckBox ), NULL, this );
	m_checkBox_Progress->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RunDlg::m_checkBox_ProgressOnCheckBox ), NULL, this );
	m_checkBox_log->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RunDlg::m_checkBox_logOnCheckBox ), NULL, this );
	m_checkBox_51->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RunDlg::m_checkBox_51OnCheckBox ), NULL, this );
	m_checkBox_half->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(RunDlg::m_checkBox_halfOnCheckBox), NULL, this);
	m_checkBox_play->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(RunDlg::m_checkBox_playOnCheckBox), NULL, this);

	m_button_run->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RunDlg::m_button_runOnButtonClick ), NULL, this );
	m_button_help->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RunDlg::m_button_helpOnButtonClick), NULL, this);
	m_button_quit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RunDlg::m_button_quitOnButtonClick ), NULL, this );
	m_sliderVolume->Connect(wxEVT_SLIDER, wxCommandEventHandler(RunDlg::m_slider_volume), NULL, this);

}

RunDlg::~RunDlg()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( RunDlg::RunDcpPlayerDlgOnInitDialog ) );
	m_dirPicker2->Disconnect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( RunDlg::m_dirPicker2OnDirChanged ), NULL, this );

	m_choiceAudio->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( RunDlg::m_choiceAudioOnChoice ), NULL, this );
	m_choiceDisplay->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( RunDlg::m_choiceDisplayOnChoice ), NULL, this );
	m_choiceCPL->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( RunDlg::m_choiceCPLOnChoice ), NULL, this );

	m_checkBox_full_screen->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RunDlg::m_checkBox_full_screenOnCheckBox ), NULL, this );
	m_checkBox_Progress->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RunDlg::m_checkBox_ProgressOnCheckBox ), NULL, this );
	m_checkBox_log->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RunDlg::m_checkBox_logOnCheckBox ), NULL, this );
	m_checkBox_51->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( RunDlg::m_checkBox_51OnCheckBox ), NULL, this );
	m_checkBox_half->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(RunDlg::m_checkBox_halfOnCheckBox), NULL, this);
	m_checkBox_play->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(RunDlg::m_checkBox_playOnCheckBox), NULL, this);

	m_button_run->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RunDlg::m_button_runOnButtonClick ), NULL, this );
	m_button_help->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RunDlg::m_button_helpOnButtonClick), NULL, this);
	m_button_quit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RunDlg::m_button_quitOnButtonClick ), NULL, this );
	m_sliderVolume->Disconnect(wxEVT_SLIDER, wxCommandEventHandler(RunDlg::m_slider_volume), NULL, this);


}


//void RunDlg::OnVolumeSliderChanged(wxCommandEvent& event)
//{
//	int volume = m_sliderVolume->GetValue();
//	wxLogMessage("Volume réglé à : %d dB", volume);
//}
