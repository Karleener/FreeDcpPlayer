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
  * @file CPlayer.h
  * @screen reels using Nvidia nvjp2k library
  */

#pragma once
#define _CRTDBG_MAP_ALLOC

#include "FreeDcpPlayer.h"
#include "CommandOptions.h"
#include<vector>
#include<thread>
#include<chrono>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_audio.h>

#include "pugixml.hpp"
#include "nvjpeg2k.h"

#include "CDcpParse.h"

using namespace std;

enum STATE
{
	PAUSE,
	PLAY
};

struct partial_decode_info
{
	unsigned int win_tilex0;
	unsigned int win_tilex1;
	unsigned int win_tiley0;
	unsigned int win_tiley1;
	unsigned int tile_id;
};

struct decode_params_t
{
	std::string input_dir;
	int batch_size;
	int total_images;
	int dev;
	int warmup;

	nvjpeg2kDecodeState_t nvjpeg2k_decode_states[1];
	nvjpeg2kHandle_t nvjpeg2k_handle;
	cudaStream_t stream[1];
	std::vector<nvjpeg2kStream_t> jpeg2k_streams;
	bool verbose;
	bool write_decoded;
	std::string output_dir;

	unsigned int win_x0;
	unsigned int win_x1;
	unsigned int win_y0;
	unsigned int win_y1;
	bool partial_decode;
};



class CPlayer
{
public :

	CPlayer(CommandOptions& Options_i, const Kumu::IFileReaderFactory& fileReaderFactory_i, fs::path full_path_i);
	//CPlayer();
	~CPlayer();

	STATE NextState;
	FILE* fp_log;

	int getTickCount();
	int getTickFrequency();
	double Duration(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end);
	float Convert24bto16b(uint8_t byte2, uint8_t byte1, uint8_t byte0);
	float ApplyGain(float ech, float gain);
	std::chrono::time_point<std::chrono::system_clock> MyGetCurrentTime();

	bool SelectAudioDeviceInitAudio();
	float AudioGainDB ;
	const char* MyAudioDevice;
	double tickf;

	CReel *ptrReel;
	CommandOptions Options;
	const Kumu::IFileReaderFactory *fileReaderFactory;
	vector<string> MxfFiles;
	fs::path full_path;

	bool HaveSub;
	thread* Af1 ;
	thread* Af2 ;
	thread* Af3 ;
	thread* Af4 ;
	thread* Af5;
	thread* Af6;
	thread* Af7;
	thread* Af8;
	thread* AfSub ;
	bool BlackBackground = Options.BlackBackground;

	AESDecContext* Context;
	HMACContext* HMAC ;
	AESDecContext* ContextPCM ;
	HMACContext* HMACPCM ;
	JP2K::MXFReader    *pReader;
	JP2K::FrameBuffer  *pFrameBuffer;
	JP2K::PictureDescriptor PDesc;
	bool VideoSuccess;
	ui32_t             frame_count;
	ui32_t last_frame;


	SDL_AudioSpec spec;

	SDL_Window* mywin;
	SDL_Renderer* Renderer;
	SDL_Event event;
	SDL_AudioDeviceID Audiodev;
	SDL_Surface* out;
	SDL_Surface* out_swap;
	Uint32 rmask, gmask, bmask, amask;
	int win_h,win_w;

	vector<int> IndiceSub;
	vector <SubTitle> MySubTitles;

	TTF_Font* Font;
	TTF_Font* Font64;
	TTF_Font* Font32;

	PCM::MXFReader     *pReaderPCM;
	PCM::FrameBuffer   *pFrameBufferPCM;
	PCM::AudioDescriptor ADesc;
	bool AudioSuccess;


	int NbSampleperImage;
	double tpsframe;
	double Derive;
	int NbBlock;
	ui32_t start_frame;
	ui32_t offset_frame;
	float scalef;
	int NumBlock;
	ui32_t CurrentFrameNumber;

	SMemoire Mem;

	nvjpeg2kHandle_t nvjpeg2k_handle;
	nvjpeg2kStream_t nvjpeg2k_stream;
	nvjpeg2kDecodeState_t decode_state;
	size_t length;
	unsigned char* bitstream_buffer;
	nvjpeg2kDecodeParams_t decode_params;
	nvjpeg2kImage_t output_image;
	nvjpeg2kImageInfo_t image_info;
	nvjpeg2kImageComponentInfo_t image_comp_info[NUM_COMPONENTS];
	unsigned short* decode_output[NUM_COMPONENTS];
	size_t pitch_in_bytes[NUM_COMPONENTS];

	int height, width;
	int bytes_per_element ;
	double timerinitialGlobal;
	double timePerimage;
	std::chrono::time_point<std::chrono::system_clock> AtimerinitialGlobal;
	std::chrono::time_point<std::chrono::system_clock> AtimePerimage;

	int NbProcFrame;
	bool RestartLoop;
	int BlockOffset;
	bool WaitLastStop;
	bool OutEscape ;
	Uint32 TimeCodeRate;
	Uint32 TheoreticalFrame;
	int FontSize;
	int DecodeLevel;

	std::vector<unsigned short> vchanR;
	std::vector<unsigned short> vchanG;
	std::vector<unsigned short> vchanB;

	unsigned short int* Lut26;
	unsigned short int* Lut22;
	unsigned char* Lut22_c;

	unsigned char * GlobalBufferOneFrame;
	unsigned char* AudioForDevice;


	Uint32 NumFrameAudio;
	//Uint32 initial_start_frame;


	Uint32 SizeAudioStruct;
	Uint32 SizeAudioDeviceStruct;
	Result_t InitialisationReaders(CDcpParse& DcpParse, bool FirstTime,  CReel *ptrReel_i);
	Result_t InitialisationJ2K();
	bool PrepareXYZ2RGBLUT();
	Result_t Read_timed_text_file(const Kumu::IFileReaderFactory& fileReaderFactory, string inputFile, fs::path full_path);
	static void PrepareFirstAudioBuffering(void *Param);
	Result_t DecodeAndScreenFirstFrame(bool WaitAfterFirstFrame);
	Result_t MainLoop(bool WaitAfterFirstFrame);
	void StateMachine();
	void EndAndClear(bool Lasttime);
	float Synchronisation();

	static void ThreadQuarter1(void* Param);
	static void ThreadQuarter2(void* Param);
	static void ThreadQuarter3(void* Param);
	static void ThreadQuarter4(void* Param);

	static void ThreadQuarter5(void* Param);
	static void ThreadQuarter6(void* Param);
	static void ThreadQuarter7(void* Param);
	static void ThreadQuarter8(void* Param);

	static void RenderImageWithSub(SDL_Renderer* Renderer, TTF_Font* Font, vector<SubTitle>& MySubTitles, int width, int height, vector<int>& IndiceSub, Uint32 i, SMemoire& Mem);
	static bool get_text_and_rect(SDL_Renderer* renderer, int x, int y, const char* text, TTF_Font* font, SDL_Texture** texture, SDL_Rect* rect);


	int From51toStereo(const SFiveDotOne* GlobalBufferOneFrame, SStereo* AudioDeviceStereo, int NbSamples);
	int FromStereotoStereo(const SStereo24b* GlobalBufferOneFrame, SStereo* AudioDeviceStereo, int NbSamples);
	int From51to51_16B(const SFiveDotOne* GlobalBufferOneFrame, SFiveDotOne16B* AudioDevice, int NbSamples);
	int FromXchannelstoStereo(const unsigned char *GlobalBufferOneFrame, SStereo* AudioDeviceStereo, int NbSamples);
	int FromXchannelsto51(const unsigned char* GlobalBufferOneFrame, SFiveDotOne16B* AudioDevice, int NbSamples);
	Uint32 DecodeTime(string chaineTps, double frame_rate,bool TypeCs);
	SDL_Window* win_init_render(int w, int h, SDL_Renderer** Renderer, bool BlackBackGround, int NumDisplay, bool FullScreen);
	void RefreshAndContinue();
	void Swap(SDL_Surface* &out1, SDL_Surface* &out2);


};

