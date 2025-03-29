/*
Copyright (c) 2003-2016, John Hurst for Asdcplib
Copyright (c) 2022, Johel Miteran for all freedcpplayer add on
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*! \file    freedcpplayer.cpp
	\version $Id$

*/

//version 0.6.3
// improve 5.1 to 2.0 mixdown
// adding gain audio parameter in dB
//-l 1 add one dB 
// -l 0 is native audio
//-l -1 remove one dB
// audiogain is also alplied to all audio format

//version 0.6.2
//change
// adding multichannel audio support with 5.1 or stero mixdown
// adding entrypoint different from 0

//version 0.6.1 
//change
// adding command line option allowing batch run

//version 0.6.0
//change
// use version 0.6.0 of nvjpeg2k_dll, allowing 4K cine decoding
// half or full resolution decoding



// version 0.3.2
// change
// subtitle with UTF8 encoding
// use TTF font embedded in DCP or defaut if not found
// parse multi CPL and multi reel DCP
// SMPTE and Interop
// stero and 5.1 soundtrack
// 06 - 03- 2022 accelerated rendering up to 4k in real time
#include <stdlib.h>
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif



#include "KM_fileio.h"
//#include "WavFileWriter.h"
#include "AS_DCP.h"
#include "KM_platform.h"
#include "KM_error.h"

#include <ctime>
#include<thread>
#include<iostream>

#include "pugixml.hpp"
#include "nvjpeg2k.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include<SDL_audio.h>

#include "FreeDcpPlayer.h"
#include "CPlayer.h"
#include "CDcpParse.h"

#include "CommandOptions.h"

using namespace ASDCP;
using namespace std;


void banner(FILE* stream = stdout)
{
	fprintf(stream, "\n\
%s (FreeDcpPlayer %s)\n\n\
From asdcplib Copyright (c) 2003-2015 John Hurst\n\n\
Specify the -h (help) option for further information about %s\n\n\
 Copyright (c) 2022 Johel Miteran\n\nBased on \n\
SDL2\nSDL_TTF\nfreetype-2.11.1\n\n",PROGRAM_NAME, "0.2.0" , PROGRAM_NAME);
}

//
void usage(FILE* stream = stdout)
{
	fprintf(stream, "\
USAGE: %s [-h|-help] [-V]\n\
\n\
       %s [-d <Display number>]\n\
       [-a <AudioDevice>] [-f <starting-frame>]  [-v] \n\
        <input-folder> \n\n",
		PROGRAM_NAME, PROGRAM_NAME);

	fprintf(stream, "\
Options:\n\
   <input-folder>   - Give the full path of the DCP (assetmap.xml) without last /\n\
  -a <AudioDEvice>  - Audio device number, default 0.\n\
  -c <CPL mumber>	- CPL number to play, default 0.\n\
  -d <Display>      - Display number, default 0\n\
  -i                - show progress bar, default false\n\
  -o				- Activate 5.1 sound output if input is 5.1\n\
  -p				- Play direclty without pause\n\
  -h | -help        - Show help\n\
  -l				- Audio gain from 0 to 12 coding -6dB to 6dB, default 6=0dB\n\
  -j				- Display fps\n\
  -s				- Play half resolution\n\
  -v                - Verbose, prints informative messages to stderr\n\
\n\
  NOTES: o There is no option grouping, all options must be distinct arguments.\n\
         o All option arguments must be separated from the option by whitespace.\n\
		 o Use left and right arrows for fast forward and rewind\n\
		 o Use Up and Down arrows for image per image in paused mode\n\
		 o Use double mouse left click in the picture as an horizontal slider to move forward or backward\n\
		 o Press  ESC key to end the program\n\
		 o Press space bar for play/pause\n\n\
		 o Example  : FreeDcpPlayer \"c:/mydcp\" -a 0 -d 0\n\
		 o This version is restricted to Uncrypted 5.1 or stereo 2K or 4K DCP\n\
		 o A Cuda based GPU with at least 6GB is required\n\n\
		Portions of this software are copyright (c) <2006-2021> The FreeType\n\
		Project(www.freetype.org).All rights reserved.\n\
		Portions of this software are copyright (c) asdcplib 2003-2015 John Hurst\n\n\
\n\
		This software is provided 'as-is', without any express or implied\n\
		warranty.In no event will the authors be held liable for any damages\n\
		arising from the use of this software.\n\
		VERSION 0.6.3.2\n\
		copyright (c) <2006 - 2021> Johel Miteran - Karleener\n\n\
");
}

//

//


int main_dcpplayer(int argc, const char** argv,bool &IsPlaying)
{
	

#ifdef _DEBUG
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	
	fs::path full_path = (fs::path(argv[0]).parent_path());

	fs::path PathLog = full_path;
	PathLog /= "freedcpplayer.log";
	string LogFileName{ PathLog.string() };
	if (exists(PathLog))
	{
		fs::remove(PathLog);
	}
	FILE *fp_log = NULL;
	fp_log = fopen(LogFileName.c_str(), "a+");

	vector<string> MxfFiles;
	Kumu::FileReaderFactory defaultFactory;

	CommandOptions Options(argc, argv);

#if defined(WIN64) || defined(_WIN64)
	HANDLE consoleHandleOut, consoleHandleError;
	if (AttachConsole(ATTACH_PARENT_PROCESS)) 
	{
		// Redirect unbuffered STDOUT to the console
		consoleHandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (consoleHandleOut != INVALID_HANDLE_VALUE) 
		{
			freopen("CONOUT$", "w", stdout);
			setvbuf(stdout, NULL, _IONBF, 0);
		}
	}
#endif
	if (Options.help_flag) usage();
	if (Options.verbose_flag) fprintf(fp_log, "FreeDcpPlayer version 0.6.3.2 \n");

	if (Options.error_flag)
	{
		fprintf(fp_log, "There was a problem. Did you choose a DCP folder ?\n");
		if (fp_log) fclose(fp_log);
		IsPlaying = false;
		return 3;
	}
	
	CDcpParse DcpParse(Options.verbose_flag);
	string DcpPath = Options.input_filename;
	DcpParse.ParseDCP(MxfFiles, Options.input_filename);
	int CplIndex = Options.NumCpl;
	int Totalduration = 0;
	if (DcpParse.CplVector.size() <= CplIndex)
	{
		fprintf(fp_log, "CPL index %d not found",CplIndex); IsPlaying = false; if (fp_log) fclose(fp_log); return 3;
	}
	if (!DcpParse.VideoOk || !DcpParse.SoundOk)
	{
		fprintf(fp_log, "No Video found or no audio found"); IsPlaying = false; if (fp_log) fclose(fp_log); return 3;
	}

	for (int k = 0; k < DcpParse.CplVector[CplIndex]->VecReel.size(); k++)
	{
		string Video = DcpParse.CplVector[CplIndex]->VecReel[k]->ptrMainPicture->sPath;
		Uint32 duration = DcpParse.CplVector[CplIndex]->VecReel[k]->ptrMainPicture->iDuration;
		Totalduration += duration;
	}
	vector<JP2K::MXFReader*> VectVideoReader;
	VectVideoReader.resize(Totalduration);
	CPlayer* pPlayer=NULL;
	bool AudioSelectedOk=false;
	bool WaitAfterFirstFrame;




	pPlayer = new CPlayer(Options, defaultFactory, full_path);
	if (Options.PlayDirect)
	{
		WaitAfterFirstFrame = false;
		pPlayer->RestartLoop = true;
	}
	else
	{
		WaitAfterFirstFrame = true;
		pPlayer->RestartLoop = false;
	}
	AudioSelectedOk = pPlayer->SelectAudioDeviceInitAudio();

	if (AudioSelectedOk && DcpParse.VideoOk && DcpParse.SoundOk)
	{
		// Process first reel
		Result_t Result = pPlayer->InitialisationReaders(DcpParse, true, DcpParse.CplVector[CplIndex]->VecReel[0]);
		if (ASDCP_SUCCESS(Result))	Result = pPlayer->InitialisationJ2K(); 
		else 
		{
			IsPlaying = false; 
			if (fp_log) fclose(fp_log); 
			return RESULT_FAIL;
		}
		if (ASDCP_SUCCESS(Result))  Result = pPlayer->MainLoop(WaitAfterFirstFrame); 	
		else
		{
			IsPlaying = false; 
			if (fp_log) fclose(fp_log);
			return RESULT_FAIL;
		}
		if (ASDCP_SUCCESS(Result))
		{
			if (pPlayer->OutEscape) pPlayer->EndAndClear(true);
			else
			{
				if (DcpParse.CplVector[CplIndex]->VecReel.size() > 1) pPlayer->EndAndClear(false);
				else pPlayer->EndAndClear(true);
			}
		}
		else
		{
			IsPlaying = false;
			if (fp_log) fclose(fp_log);
			return RESULT_FAIL;
		}
 

		for (int k = 1; k < DcpParse.CplVector[CplIndex]->VecReel.size() && !pPlayer->OutEscape; k++)
		{
			// compute global duration
			// for global navigation
			// to be added in future version
			//string Video = DcpParse.CplVector[ClpIndex]->VecReel[k]->ptrMainPicture->sPath;
			//JP2K::MXFReader *pReader = new JP2K::MXFReader(defaultFactory);
			//Result_t resultVideo = pReader->OpenRead(Video); // read video
			//Uint32 duration = DcpParse.CplVector[ClpIndex]->VecReel[k]->ptrMainPicture->iDuration;
			//for (int d = 0; d < duration; d++) VectVideoReader.push_back(pReader);

			Result_t Result = pPlayer->InitialisationReaders(DcpParse, false, DcpParse.CplVector[CplIndex]->VecReel[k]);
			if (ASDCP_SUCCESS(Result))	Result = pPlayer->InitialisationJ2K(); 	
			else
			{
				IsPlaying = false; 
				if (fp_log) fclose(fp_log);
				return RESULT_FAIL;
			}
			if (ASDCP_SUCCESS(Result))  Result = pPlayer->MainLoop(false);
			else
			{
				IsPlaying = false; 
				if (fp_log) fclose(fp_log);
				return RESULT_FAIL;
			}
			if (k == DcpParse.CplVector[CplIndex]->VecReel.size() - 1) pPlayer->EndAndClear(true);
			else pPlayer->EndAndClear(false);
			if (pPlayer->OutEscape) break;
		}

	} // if audio device is selected
	delete pPlayer;
	IsPlaying = false;
	if (fp_log) fclose(fp_log);
	return 0;
} // end main


//
// end freedcpplayer.cpp
//
