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

 /**
  * @file FreeDcpPlayer.h
  * @screen reels using Nvidia nvjp2k library
  */

#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include "AS_DCP.h"
#include "KM_error.h"
#include "nvjpeg2k.h"

using namespace std;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#include <filesystem>
const std::string separator = "\\";
namespace fs = std::filesystem;
#else
#include <sys/time.h> // timings
#include <experimental/filesystem>
const std::string separator = "/";
namespace fs = std::experimental::filesystem::v1;
#endif

#define CHECK_CUDA(call)                                                                                          \
    {                                                                                                             \
        cudaError_t _e = (call);                                                                                  \
        if (_e != cudaSuccess)                                                                                    \
        {                                                                                                         \
            std::cout << "CUDA Runtime failure: '#" << _e << "' at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return EXIT_FAILURE;                                                                                     \
        }                                                                                                         \
    }

#define CHECK_NVJPEG2K(call)                                                                                \
    {                                                                                                       \
        nvjpeg2kStatus_t _e = (call);                                                                       \
        if (_e != NVJPEG2K_STATUS_SUCCESS)                                                                  \
        {                                                                                                   \
            std::cout << "NVJPEG failure: '#" << _e << "' at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return EXIT_FAILURE;                                                                            \
        }                                                                                                   \
    }

typedef unsigned char uchar;

#define SPACEBAR 0x20
#define ESCAPE 0x1B

#define NUM_COMPONENTS 3
#define BUFFER_AUDIO 100
const ui32_t FRAME_BUFFER_SIZE = 4 * Kumu::Megabyte;

enum MajorMode_t
{
	MMT_NONE,
	MMT_EXTRACT,
	MMT_GOP_START,
};


struct SMemoire
{
	SDL_Window* mywin;
	SDL_Texture* Background_Tx;
	SDL_Rect dstRect;
	SDL_Surface* scr;
	unsigned short* chanR;
	unsigned short* chanG;
	unsigned short* chanB;
	int height;
	int width;
	unsigned short int* pLut26;
	unsigned short* pLut22;
	unsigned char* pLut22_c;
	int win_w;
	int win_h;
	int base; // for subtitle
	float Scalef;
	Uint32 FrameCount;
	bool IncrustPosition;
	float DisplayFps;
	bool IncrustFps;

};
struct LineSub
{
	string Valign;
	float Vposition;
	string Text;
};

struct SubTitle
{
	int SpotNumber;
	string TimeIn;
	string TimeOut;
	string FadeUpTime;
	string FadeDownTime;
	vector<LineSub> Line;
};

struct SFiveDotOne // 5.1
{
	uchar L[3];
	uchar R[3];
	uchar C[3];
	uchar LFE[3];
	uchar BL[3];
	uchar BR[3];
};

struct SSevenDotOne // 7.1
{
	uchar L[3];
	uchar R[3];
	uchar C[3];
	uchar LFE[3];
	uchar LSS[3];
	uchar RSS[3];
	uchar LRS[3];
	uchar RRS[3];
};


struct SStereo24b
{
	uchar L[3];
	uchar R[3];
};

struct SStereo
{
	short int L;
	short int R;
};

struct SFiveDotOne16B
{
	short L;
	short R;
	short C;
	short LFE;
	short BL;
	short BR;
};





