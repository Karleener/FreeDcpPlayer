
// RunMyDcpPlayerDlg.cpp : fichier d'implémentation
//



#include "pch.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <shobjidl.h> 
#include <ShlObj_core.h>
#include <string>

#include "framework.h"
#include "RunMyDcpPlayer.h"
#include "RunMyDcpPlayerDlg.h"
#include "afxdialogex.h"

#include <direct.h> 

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_audio.h>

#include "CDcpParse.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// boîte de dialogue CAboutDlg utilisée pour la boîte de dialogue 'À propos de' pour votre application

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Données de boîte de dialogue
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

// Implémentation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// boîte de dialogue de CRunMyDcpPlayerDlg



CRunMyDcpPlayerDlg::CRunMyDcpPlayerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RUNMYDCPPLAYER_DIALOG, pParent)
	, m_MyDir(_T(""))
	, m_Com(_T(""))
	, m_Back(TRUE)
	,m_Log(TRUE)
	, m_ProgressBar(TRUE)
	,m_Output51(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRunMyDcpPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MYDIR, m_MyDir);
	DDX_Control(pDX, IDC_COMBO_AUDIO, m_ComboAudio);
	DDX_Control(pDX, IDC_COMBO2, m_ComboDisplay);
	DDX_Text(pDX, IDC_TEXTE_COMMAND, m_Com);
	DDX_Check(pDX, IDC_CHECK_B, m_Back);
	DDX_Check(pDX, IDC_LOG, m_Log);
	DDX_Check(pDX, IDC_CHECK_INCRUST, m_ProgressBar);
	DDX_Check(pDX, IDC_CHECK_51, m_Output51);
	DDX_Control(pDX, IDC_COMBO1, m_ComboCpl);
}

BEGIN_MESSAGE_MAP(CRunMyDcpPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHOOSE_DCP, &CRunMyDcpPlayerDlg::OnBnClickedChooseDcp)
	ON_BN_CLICKED(IDOK, &CRunMyDcpPlayerDlg::OnBnClickedOk)
	ON_CBN_SELENDOK(IDC_COMBO2, &CRunMyDcpPlayerDlg::OnCbnSelendokCombo2)
	ON_CBN_SELENDOK(IDC_COMBO_AUDIO, &CRunMyDcpPlayerDlg::OnCbnSelendokComboAudio)
	ON_BN_CLICKED(IDC_CHECK_B, &CRunMyDcpPlayerDlg::OnBnClickedCheckB)
	ON_CBN_SELENDOK(IDC_COMBO1, &CRunMyDcpPlayerDlg::OnCbnSelendokCombo1)
	ON_BN_CLICKED(IDC_CHECK_51, &CRunMyDcpPlayerDlg::OnBnClickedCheck51)
	ON_BN_CLICKED(IDC_LOG, &CRunMyDcpPlayerDlg::OnBnClickedLog)
	ON_BN_CLICKED(IDC_CHECK_INCRUST, &CRunMyDcpPlayerDlg::OnBnClickedCheckIncrust)
END_MESSAGE_MAP()


// gestionnaires de messages de CRunMyDcpPlayerDlg

BOOL CRunMyDcpPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Ajouter l'élément de menu "À propos de..." au menu Système.

	// IDM_ABOUTBOX doit se trouver dans la plage des commandes système.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Définir l'icône de cette boîte de dialogue.  L'infrastructure effectue cela automatiquement
	//  lorsque la fenêtre principale de l'application n'est pas une boîte de dialogue
	SetIcon(m_hIcon, TRUE);			// Définir une grande icône
	SetIcon(m_hIcon, FALSE);		// Définir une petite icône

	// TODO: ajoutez ici une initialisation supplémentaire
	strcpy_s(cwd, "");
	InitialDir= "";
	ChoosenDir = "";
	const char* Proposition;
	vector<const char*> AudioDevice;
	vector<const char*> DisplayDevice;
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	int n = 0;
	do
	{
		Proposition = SDL_GetAudioDeviceName(n, 0);
		if (Proposition) m_ComboAudio.AddString(Proposition);
		n++;
	} while (Proposition && n < 50);

	for (int i = 0; i < AudioDevice.size(); i++)
	{
		m_ComboAudio.AddString(AudioDevice[i]);
	}

	n = 0;
	do
	{
		Proposition = SDL_GetDisplayName(n);
		if (Proposition) DisplayDevice.push_back(Proposition);
		n++;
	} while (Proposition && n < 50);
	for (int i = 0; i < DisplayDevice.size(); i++)
	{
		string temp(DisplayDevice[i]);
		temp += " (" + to_string(i) + ")";
		m_ComboDisplay.AddString(temp.c_str());
	}

	SDL_Quit();

	_getcwd(cwd, MAX_PATH);
	InitialDir = string(cwd);

	string line;
	ifstream read("config.txt");
	if (read.good())
	{
		getline(read, ChoosenDir);

		getline(read, line);
		m_ComboAudio.SetCurSel(atoi(line.c_str()));
		getline(read, line);
		m_ComboDisplay.SetCurSel(atoi(line.c_str()));
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

		m_ComboAudio.SetCurSel(0);
		m_ComboDisplay.SetCurSel(0);
		m_ComboCpl.SetCurSel(0);
		ChoosenDir = "";
	}

	_getcwd(cwd, MAX_PATH);


	CString Dir(cwd);
	m_Com.Format("MyDcpPlayer %s -a %d -d %d", ChoosenDir.c_str() , m_ComboAudio.GetCurSel(), m_ComboDisplay.GetCurSel());
	UpdateData(false);
	

	return TRUE;  // retourne TRUE, sauf si vous avez défini le focus sur un contrôle
}

void CRunMyDcpPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// Si vous ajoutez un bouton Réduire à votre boîte de dialogue, vous devez utiliser le code ci-dessous
//  pour dessiner l'icône.  Pour les applications MFC utilisant le modèle Document/Vue,
//  cela est fait automatiquement par l'infrastructure.

void CRunMyDcpPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // contexte de périphérique pour la peinture

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Centrer l'icône dans le rectangle client
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Dessiner l'icône
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Le système appelle cette fonction pour obtenir le curseur à afficher lorsque l'utilisateur fait glisser
//  la fenêtre réduite.
HCURSOR CRunMyDcpPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRunMyDcpPlayerDlg::OnBnClickedChooseDcp()
{

	CDcpParse DcpParse(true);

	_getcwd(cwd, MAX_PATH);
	bool r=true;
	//r=GetOpenDirectory(ChoosenDir, MAX_PATH, cwd);

	OPENFILENAME ofn = { 0 };
	TCHAR szFile[260] = { 0 };
	// Initialize remaining fields of OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("All\0ASSETMAP*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		printf("%s", ofn.lpstrFile);

	}

	filesystem::path cdir(ofn.lpstrFile);
	cdir = cdir.parent_path();
	string Temp{ cdir.string() };
	ChoosenDir = string(Temp);

	//std::string temp(ChoosenDir);
	if (ChoosenDir!="")
	{
		//_chdir(ChoosenDir.c_str());
		UpdateData(true);
		m_MyDir.Format("%s", ChoosenDir.c_str());
		UpdateData(false);

	}
	_chdir(InitialDir.c_str());
	vector<string> MXFFiles;
	DcpParse.ParseDCP(MXFFiles,Temp);
	m_ComboCpl.ResetContent();

	for (int i = 0; i < DcpParse.CplVector.size(); i++)
	{
		string temp(DcpParse.CplVector[i]->sAnnotation);
		temp += " (" + to_string(i) + ")";
		m_ComboCpl.AddString(temp.c_str());
	}
	m_ComboCpl.SetCurSel(0);
	UpdateCommand();
}


bool CRunMyDcpPlayerDlg::GetOpenDirectory(char* out, int max_size, const char* starting_dir)
{
	bool ret = false;
	IFileDialog* pfd;
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
	{
		if (starting_dir)
		{
			PIDLIST_ABSOLUTE pidl;
			WCHAR wstarting_dir[MAX_PATH];
			WCHAR* wc = wstarting_dir;
			for (const char* c = starting_dir; *c && wc - wstarting_dir < MAX_PATH - 1; ++c, ++wc)
			{
				*wc = *c == '/' ? '\\' : *c;
			}
			*wc = 0;

			HRESULT hresult = ::SHParseDisplayName(wstarting_dir, 0, &pidl, SFGAO_FOLDER, 0);
			if (SUCCEEDED(hresult))
			{
				IShellItem* psi;
				hresult = ::SHCreateShellItem(NULL, NULL, pidl, &psi);
				if (SUCCEEDED(hresult))
				{
					pfd->SetFolder(psi);
				}
				ILFree(pidl);
			}
		}

		DWORD dwOptions;
		if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
		{
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
		}
		if (SUCCEEDED(pfd->Show(NULL)))
		{
			IShellItem* psi;
			if (SUCCEEDED(pfd->GetResult(&psi)))
			{
				WCHAR* tmp;
				if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &tmp)))
				{
					char* c = out;
					while (*tmp && c - out < max_size - 1)
					{
						*c = (char)*tmp;
						++c;
						++tmp;
					}
					*c = '\0';
					ret = true;
				}
				psi->Release();
			}
		}
		pfd->Release();
	}
	return ret;
}

void CRunMyDcpPlayerDlg::OnBnClickedOk()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
	UpdateCommand();
	string Config = InitialDir + string("/config.txt");
	FILE *fp=fopen(Config.c_str(), "w");
	fprintf(fp, "%s\n", ChoosenDir.c_str());
	fprintf(fp, "%d\n", m_ComboAudio.GetCurSel());
	fprintf(fp, "%d\n", m_ComboDisplay.GetCurSel());
	if (m_Back) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
	if (m_Log) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
	if (m_ProgressBar) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
	if (m_Output51) fprintf(fp, "%d\n", 1); else fprintf(fp, "%d\n", 0);
	fclose(fp);
	string Command = InitialDir + string("/command.txt");
	fp = fopen(Command.c_str(), "a+");
	fprintf(fp, "%s\n", m_Com.GetBuffer(m_Com.GetLength()+1));
	fclose(fp);
	system(m_Com);

	//CDialogEx::OnOK();
}


void CRunMyDcpPlayerDlg::OnCbnSelendokCombo2()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
	UpdateCommand();
}

void CRunMyDcpPlayerDlg::UpdateCommand()
{
	UpdateData(true);
	_chdir(InitialDir.c_str());
	int audio = m_ComboAudio.GetCurSel();
	int display = m_ComboDisplay.GetCurSel();
	int CplIndex = m_ComboCpl.GetCurSel();

	CString Temp;
	Temp.Format("FreeDcpPlayer.exe \"%s\" -a %d -d %d -c %d", ChoosenDir.c_str(), audio, display,CplIndex);
	if (!m_Back) Temp += " -g";
	if (m_ProgressBar) Temp += " -i";
	if (m_Output51) Temp += " -o";

	if (m_Log) m_Com = Temp + " -v 2>freedcpplayer.log & pause";
	else  m_Com = Temp + " & pause";

	UpdateData(false);
}


void CRunMyDcpPlayerDlg::OnCbnSelendokComboAudio()
{
	UpdateCommand();
}



void CRunMyDcpPlayerDlg::OnCbnSelendokCombo1()
{
	UpdateCommand();
}


void CRunMyDcpPlayerDlg::OnBnClickedCheck51()
{
	UpdateCommand();
}
void CRunMyDcpPlayerDlg::OnBnClickedCheckB()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
	UpdateCommand();
}


void CRunMyDcpPlayerDlg::OnBnClickedLog()
{
	UpdateCommand();
}


void CRunMyDcpPlayerDlg::OnBnClickedCheckIncrust()
{
	UpdateCommand();
}
