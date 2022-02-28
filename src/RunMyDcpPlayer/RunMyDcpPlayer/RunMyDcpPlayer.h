
// RunMyDcpPlayer.h : fichier d'en-tête principal de l'application PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "incluez 'pch.h' avant d'inclure ce fichier pour PCH"
#endif

#include "resource.h"		// symboles principaux


// CRunMyDcpPlayerApp :
// Consultez RunMyDcpPlayer.cpp pour l'implémentation de cette classe
//

class CRunMyDcpPlayerApp : public CWinApp
{
public:
	CRunMyDcpPlayerApp();

// Substitutions
public:
	virtual BOOL InitInstance();

// Implémentation

	DECLARE_MESSAGE_MAP()
};

extern CRunMyDcpPlayerApp theApp;
