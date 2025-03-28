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

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/filepicker.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/slider.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class RunDlg
///////////////////////////////////////////////////////////////////////////////
class RunDlg : public wxDialog
{
	private:

	protected:
		wxDirPickerCtrl* m_dirPicker2;
		wxStaticText* m_staticText_DCPDIR;
		wxStaticText* m_staticText2;
		wxChoice* m_choiceAudio;
		wxStaticText* m_staticText3;
		wxChoice* m_choiceDisplay;
		wxStaticText* m_staticText4;
		wxChoice* m_choiceCPL;
		wxCheckBox* m_checkBox_full_screen;
		wxCheckBox* m_checkBox_Progress;
		wxCheckBox* m_checkBox_log;
		wxCheckBox* m_checkBox_51;
		wxCheckBox* m_checkBox_half;
		wxCheckBox* m_checkBox_play;
		wxButton* m_button_run;
		wxButton* m_button_help;
		wxButton* m_button_quit;
		wxStaticText* m_staticText_infoCommand;
		wxStaticText* m_staticText_Command;
		wxStaticText* m_About1;
		wxStaticText* m_About2;
		wxStaticText* m_staticText_Volume;
		wxSlider* m_sliderVolume;


		// Virtual event handlers, override them in your derived class
		virtual void RunDcpPlayerDlgOnInitDialog( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void m_dirPicker2OnDirChanged( wxFileDirPickerEvent& event ) { event.Skip(); }

		virtual void m_choiceAudioOnChoice( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_choiceDisplayOnChoice( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_choiceCPLOnChoice( wxCommandEvent& event ) { event.Skip(); }

		virtual void m_checkBox_full_screenOnCheckBox( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_checkBox_ProgressOnCheckBox( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_checkBox_logOnCheckBox( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_checkBox_51OnCheckBox( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_checkBox_halfOnCheckBox(wxCommandEvent& event) { event.Skip(); }
		virtual void m_checkBox_playOnCheckBox(wxCommandEvent& event) { event.Skip(); }
		
		virtual void m_button_runOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_button_helpOnButtonClick(wxCommandEvent& event) { event.Skip(); }
		virtual void m_button_quitOnButtonClick( wxCommandEvent& event ) { event.Skip(); }

		virtual void m_slider_volume(wxCommandEvent& event) { event.Skip(); }


	public:

		RunDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxDEFAULT_DIALOG_STYLE );

		~RunDlg();

};

