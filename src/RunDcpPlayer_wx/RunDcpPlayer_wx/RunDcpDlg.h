///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.10.1-0-g8feb16b3)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

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
		wxButton* m_button_run;
		wxButton* m_button_quit;
		wxStaticText* m_staticText_infoCommand;
		wxStaticText* m_staticText_Command;
		wxStaticText* m_About1;
		wxStaticText* m_About2;


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
		
		virtual void m_button_runOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_button_quitOnButtonClick( wxCommandEvent& event ) { event.Skip(); }


	public:

		RunDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 798,538 ), long style = wxDEFAULT_DIALOG_STYLE );

		~RunDlg();

};

