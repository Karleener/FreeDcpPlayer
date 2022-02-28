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

// version 0.2.1
// change
// subtitle with UTF8 encoding
// use TTF font embedded in DCP or defaut if not found
// parse multi CPL and multi reel DCP
// SMPTE and Interop
// stero and 5.1 soundtrack

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#include <crtdbg.h>

#include "KM_fileio.h"
#include "WavFileWriter.h"
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
  -f <start-frame>  - Starting frame number, default 0\n\
  -g                - Black Background, default false\n\
  -i                - show progress bar, default false\n\
  -o				- Activate 5.1 sound output if input is 5.1\n\
  -h | -help        - Show help\n\
  -V                - Show version information\n\
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
		 o This version is restricted to Uncrypted 5.1 and 2K DCP\n\
		 o A Cuda based GPU with at least 6GB is required\n\n\
		Portions of this software are copyright (c) <2006-2021> The FreeType\n\
		Project(www.freetype.org).All rights reserved.\n\
		Portions of this software are copyright (c) asdcplib 2003-2015 John Hurst\n\n\
\n\
		This software is provided 'as-is', without any express or implied\n\
		warranty.In no event will the authors be held liable for any damages\n\
		arising from the use of this software.\n\
		copyright (c) <2006 - 2021> Johel Miteran - Karleener\n\n\
");
}

//

//





// read ASSETMAP
// ASSUMING ONE VIDEO mxf file, ONE AUDIO mxf file, optionnally one SUBTITLE mxf file
void ParseDCP(vector<string>& MxfFiles, string MyPath)
{
	pugi::xml_document doc;
	//string source = "c:\\video\\cro\\ASSETMAP.xml";
	//string source2 = "L:\\LACC DCP\\scope2k court\\accompagnantes\\Accompagnantes_SHR-2_S_Fr-EN_FR-NR_51_2K_karleener_20171222_24fps_SMPTE_OV\\ASSETMAP.xml";
	//string Mypath = "c:\\video\\cro\\"; //"L:\\LACC DCP\\scope2k court\\accompagnantes\\Accompagnantes_SHR-2_S_Fr-EN_FR-NR_51_2K_karleener_20171222_24fps_SMPTE_OV\\";
	string source = MyPath + "/ASSETMAP.xml";
	pugi::xml_parse_result res = doc.load_file(source.c_str());
	vector <string> NomFichiermxf;
	MxfFiles.resize(3);
	MxfFiles[0] = ""; // video file name
	MxfFiles[1] = ""; // audio file name
	MxfFiles[2] = ""; // subtitle file name

	if (res)
	{
		//cout << "XML [" << source << "] parsed without errors, attr value: [" << doc.child("AssetMap").child("AssetList").child("Asset").child("ChunkList").child("Chunk").child("Path").first_child().value() << "]\n\n";

		pugi::xml_node assets = doc.child("AssetMap").child("AssetList");
		for (pugi::xml_node asset = assets.first_child(); asset; asset = asset.next_sibling())
		{
			//std::cout << "asset:";

			for (pugi::xml_node attr = asset.first_child(); attr; attr = attr.next_sibling())
			{
				//std::cout << " " << attr.name() << "=" << attr.value()<<endl;
				string temp = attr.name();
				if (temp == "ChunkList")
				{
					for (pugi::xml_node chunk = attr.first_child(); chunk; chunk = chunk.next_sibling())
					{
						for (pugi::xml_node Path = chunk.first_child(); Path; Path = Path.next_sibling())
						{
							filesystem::path tempp = Path.first_child().value();
							if (string(Path.name()) =="Path") 
								if (tempp.extension() == ".mxf")
									NomFichiermxf.push_back(tempp.string());
						}
					}

				}
			}

			//std::cout << std::endl;
		}

	}

	Kumu::FileReaderFactory defFactory;
	EssenceType_t EssenceType;
	for (int i = 0; i < NomFichiermxf.size(); i++)
	{
		string FileName = MyPath + '/'+ NomFichiermxf[i];
		Result_t resEssense = ASDCP::EssenceType(FileName, EssenceType, defFactory);

		if (ASDCP_SUCCESS(resEssense))
		{
			switch (EssenceType)
			{
			case ESS_MPEG2_VES:

				break;

			case ESS_JPEG_2000:
				MxfFiles[0] = FileName;
				break;

			case ESS_JPEG_2000_S:

				break;

			case ESS_PCM_24b_48k:
			case ESS_PCM_24b_96k:
				MxfFiles[1] = FileName;
				break;

			case ESS_TIMED_TEXT:
				MxfFiles[2] = FileName;
				break;

			case ESS_DCDATA_UNKNOWN:

				break;

			case ESS_DCDATA_DOLBY_ATMOS:

				break;

			default:
				fprintf(stderr, "%s: Unknown file type, not ASDCP essence.\n", FileName.c_str());
				break;
			}
		} // if success
	} //for

} // end dcp parse


int main(int argc, const char** argv)
{
	//SetConsoleOutputCP(CP_UTF8);

#ifdef _DEBUG
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	printf("FreeDcpPlayer version 0.2.1 Multi reel based\n\n");
	fs::path full_path = (fs::path(argv[0]).parent_path());

	//Without file name
	//std::cout << full_path.stem() << std::endl;


	vector<string> MxfFiles;
	Kumu::FileReaderFactory defaultFactory;

	CommandOptions Options(argc, argv);
	if (Options.verbose_flag) fprintf(stderr, "FreeDcpPlayer version 0.2.1 Multi reel based\n");
	if (Options.version_flag)
		banner();

	if (Options.help_flag)
		usage();

	if (Options.version_flag || Options.help_flag)
		return 0;

	if (Options.error_flag)
	{
		fprintf(stderr, "There was a problem. Type %s -h for help.\n", PROGRAM_NAME);
		return 3;
	}

	//ParseDCP(MxfFiles, Options.input_filename);
	CDcpParse DcpParse(Options.verbose_flag);
	string DcpPath = Options.input_filename;
	DcpParse.ParseDCP(MxfFiles, Options.input_filename);
	int CplIndex = Options.NumCpl;
	int Totalduration = 0;
	if (DcpParse.CplVector.size() <= CplIndex)
	{
		fprintf(stderr, "CPL index %d not found",CplIndex); return 3;
	}
	if (!DcpParse.VideoOk || !DcpParse.SoundOk)
	{
		fprintf(stderr, "No Video found or no audio found"); return 3;
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
	bool WaitAfterFirstFrame=true;


	pPlayer = new CPlayer(Options, defaultFactory, full_path);
	AudioSelectedOk = pPlayer->SelectAudioDeviceInitAudio();

	if (AudioSelectedOk && DcpParse.VideoOk && DcpParse.SoundOk)
	{
		// Process first reel
		Result_t Result = pPlayer->InitialisationReaders(DcpParse, true, DcpParse.CplVector[CplIndex]->VecReel[0]);
		if (ASDCP_SUCCESS(Result))	Result = pPlayer->InitialisationJ2K(); else return RESULT_FAIL;
		if (ASDCP_SUCCESS(Result))  Result = pPlayer->MainLoop(WaitAfterFirstFrame); else return RESULT_FAIL;
		if (DcpParse.CplVector[CplIndex]->VecReel.size() > 1) pPlayer->EndAndClear(false); else pPlayer->EndAndClear(true);
 

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
			if (ASDCP_SUCCESS(Result))	Result = pPlayer->InitialisationJ2K(); else return RESULT_FAIL;
			if (ASDCP_SUCCESS(Result))  Result = pPlayer->MainLoop(false); else return RESULT_FAIL;
			if (k == DcpParse.CplVector[CplIndex]->VecReel.size() - 1) pPlayer->EndAndClear(true);
			else pPlayer->EndAndClear(false);
			if (pPlayer->OutEscape) break;
		}

	} // if audio device is selected
	delete pPlayer;

	return 0;
} // end main


//
// end freedcpplayer.cpp
//
