
// RunMyDcpPlayerDlg.h : fichier d'en-tête
//

#pragma once
#include <windows.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <shobjidl.h> 
#include <ShlObj_core.h>
#include <string>
using namespace std;

// boîte de dialogue de CRunMyDcpPlayerDlg
class CRunMyDcpPlayerDlg : public CDialogEx
{
// Construction
public:
	CRunMyDcpPlayerDlg(CWnd* pParent = nullptr);	// constructeur standard
	bool GetOpenDirectory(char* out, int max_size, const char* starting_dir);
	void UpdateCommand();
	string ChoosenDir;
	char cwd[MAX_PATH];
	string InitialDir;
// Données de boîte de dialogue
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RUNMYDCPPLAYER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// Prise en charge de DDX/DDV


// Implémentation
protected:
	HICON m_hIcon;

	// Fonctions générées de la table des messages
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChooseDcp();
	CString m_MyDir;
	CComboBox m_ComboAudio;
	afx_msg void OnBnClickedOk();
	CComboBox m_ComboDisplay;
	CString m_Com;
	afx_msg void OnCbnSelendokCombo2();
	afx_msg void OnCbnSelendokComboAudio();
	BOOL m_Back;
	BOOL m_Log;
	BOOL m_ProgressBar;
	BOOL m_Output51;
	afx_msg void OnBnClickedCheckB();
	CComboBox m_ComboCpl;
	afx_msg void OnCbnSelendokCombo1();
	afx_msg void OnBnClickedCheck51();
	afx_msg void OnBnClickedLog();
	afx_msg void OnBnClickedCheckIncrust();
};
