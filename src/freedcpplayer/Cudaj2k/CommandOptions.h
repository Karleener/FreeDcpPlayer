/*
Copyright (c) 2003-2016, John Hurst
Copyright (c) 2022, Johel Miteran
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
/*! \file    CommandOptions.h
*/


#pragma once
#include "FreeDcpPlayer.h"

using namespace ASDCP;

//------------------------------------------------------------------------------------------
//
// command line option parser class

static const char* PROGRAM_NAME = "FreeDcpPlayer";  // program name for messages

// Increment the iterator, test for an additional non-option command line argument.
// Causes the caller to return if there are no remaining arguments or if the next
// argument begins with '-'.
#define TEST_EXTRA_ARG(i,c)						\
  if ( ++i >= argc || argv[(i)][0] == '-' ) {				\
    fprintf(stderr, "Argument not found for option -%c.\n", (c));	\
    return;								\
  }


class CommandOptions
{


public:
	CommandOptions() {};
	bool IncrustPosition;
	bool FullScreen;
	int AudioDevice;
	MajorMode_t mode;
	bool BlackBackground;
	bool   error_flag;     // true if the given options are in error or not complete
	bool   key_flag;       // true if an encryption key was given
	bool   read_hmac;      // true if HMAC values are to be validated
	bool   split_wav;      // true if PCM is to be extracted to stereo WAV files
	bool   mono_wav;       // true if PCM is to be extracted to mono WAV files
	bool   verbose_flag;   // true if the verbose option was selected
	ui32_t fb_dump_size;   // number of bytes of frame buffer to dump
	bool   no_write_flag;  // true if no output files are to be written
	bool   version_flag;   // true if the version display option was selected
	bool   help_flag;      // true if the help display option was selected
	bool   stereo_image_flag; // if true, expect stereoscopic JP2K input (left eye first)
	ui32_t number_width;   // number of digits in a serialized filename (for JPEG extract)
	ui32_t start_frame;    // frame number to begin processing
	ui32_t duration;       // number of frames to be processed
	bool   duration_flag;  // true if duration argument given
	bool   j2c_pedantic;   // passed to JP2K::SequenceParser::OpenRead
	ui32_t picture_rate;   // fps of picture when wrapping PCM
	ui32_t fb_size;        // size of picture frame buffer
	const char* file_prefix; // filename pre for files written by the extract mode
	byte_t key_value[KeyLen];  // value of given encryption key (when key_flag is true)
	byte_t key_id_value[UUIDlen];// value of given key ID (when key_id_flag is true)
	PCM::ChannelFormat_t channel_fmt; // audio channel arrangement
	const char* input_filename;
	std::string prefix_buffer;
	const char* extension; // file extension to use for unknown D-Cinema Data track files.
	int NumDisplay;
	int NumCpl;
	bool Output51;
	bool IncrustFps;
	bool HalfResolution;
	bool PlayDirect;
	float AudioGain;

	//
	Rational PictureRate()
	{
		if (picture_rate == 16) return EditRate_16;
		if (picture_rate == 18) return EditRate_18;
		if (picture_rate == 20) return EditRate_20;
		if (picture_rate == 22) return EditRate_22;
		if (picture_rate == 23) return EditRate_23_98;
		if (picture_rate == 24) return EditRate_24;
		if (picture_rate == 25) return EditRate_25;
		if (picture_rate == 30) return EditRate_30;
		if (picture_rate == 48) return EditRate_48;
		if (picture_rate == 50) return EditRate_50;
		if (picture_rate == 60) return EditRate_60;
		if (picture_rate == 96) return EditRate_96;
		if (picture_rate == 100) return EditRate_100;
		if (picture_rate == 120) return EditRate_120;
		if (picture_rate == 192) return EditRate_192;
		if (picture_rate == 200) return EditRate_200;
		if (picture_rate == 240) return EditRate_240;
		return EditRate_24;
	}

	//
	CommandOptions(int argc, const char** argv) :
		IncrustPosition(false),FullScreen(true), AudioDevice(0), mode(MMT_EXTRACT), BlackBackground(false), error_flag(true), key_flag(false), read_hmac(false), split_wav(false),
		mono_wav(false), verbose_flag(false), fb_dump_size(0), no_write_flag(false),
		version_flag(false), help_flag(false), stereo_image_flag(false), number_width(6),
		start_frame(0), duration(0xffffffff), duration_flag(false), j2c_pedantic(true),
		picture_rate(24), fb_size(FRAME_BUFFER_SIZE), file_prefix(0),
		channel_fmt(PCM::CF_NONE), input_filename(0), extension("dcdata"), NumDisplay(0), NumCpl(0),Output51(false), IncrustFps(false),HalfResolution(false), PlayDirect(false), AudioGain(0.0F)
	{
		memset(key_value, 0, KeyLen);
		memset(key_id_value, 0, UUIDlen);

		for (int i = 1; i < argc; ++i)
		{

			if ((strcmp(argv[i], "-help") == 0))
			{
				help_flag = true;
				continue;
			}

			if (argv[i][0] == '-'
				&& (isalpha(argv[i][1]) || isdigit(argv[i][1]))
				&& argv[i][2] == 0)
			{
				switch (argv[i][1])
				{
				case '1': mono_wav = true; break;
				case '2': split_wav = true; break;
				case '3': stereo_image_flag = true; break;

				case 'a':
					TEST_EXTRA_ARG(i, 'a');
					AudioDevice = Kumu::xabs(strtol(argv[i], 0, 10));
					break;
				case 'b':
					TEST_EXTRA_ARG(i, 'b');
					fb_size = Kumu::xabs(strtol(argv[i], 0, 10));
					break;
				case 'c':
					TEST_EXTRA_ARG(i, 'b');
					NumCpl = Kumu::xabs(strtol(argv[i], 0, 10));
					break;

				case 'd':
					TEST_EXTRA_ARG(i, 'd');
					//duration_flag = true;
					NumDisplay = Kumu::xabs(strtol(argv[i], 0, 10));
					break;

				case 'e':
					TEST_EXTRA_ARG(i, 'e');
					extension = argv[i];
					break;

				case 'f':
					TEST_EXTRA_ARG(i, 'f');
					start_frame = Kumu::xabs(strtol(argv[i], 0, 10));
					break;
				case 'g':BlackBackground = true; FullScreen = false; break;
				case 'G': mode = MMT_GOP_START; break;
				case 'h': help_flag = true; break;
				case 'i':  IncrustPosition = true; break;
				case 'j':  IncrustFps = true; break;
				case 'k': key_flag = true;
					TEST_EXTRA_ARG(i, 'k');
					{
						ui32_t length;
						Kumu::hex2bin(argv[i], key_value, KeyLen, &length);

						if (length != KeyLen)
						{
							fprintf(stderr, "Unexpected key length: %u, expecting %u characters.\n", length, KeyLen);
							return;
						}
					}
					break;
				case 'l':
					TEST_EXTRA_ARG(i, 'l');
					AudioGain =(float) Kumu::xabs(strtol(argv[i], 0, 10)) -6.0F; // gain from -6dB to 6dB
					if (AudioGain > 6.0F) AudioGain = 6.0F;
					break;
				case 'm': read_hmac = true; break;
				case 'o': Output51 = true; break; // activate 5.1 output

				case 'p': PlayDirect = true; break; // activate play direct without first pause

				case 's': HalfResolution = true; break; // activate half resolution decoding

				case 'V': version_flag = true; break;
				case 'v': verbose_flag = true; break;
				case 'W': no_write_flag = true; break;

				case 'w':
					TEST_EXTRA_ARG(i, 'w');
					number_width = Kumu::xabs(strtol(argv[i], 0, 10));
					break;

				case 'Z': j2c_pedantic = false; break;
				case 'z': j2c_pedantic = true; break;

				default:
					fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
					return;
				}
			}
			else
			{
				if (argv[i][0] != '-')
				{
					if (input_filename == 0)
					{
						input_filename = argv[i];
					}
					else if (file_prefix == 0)
					{
						file_prefix = argv[i];
					}
				}
				else
				{
					fprintf(stderr, "Unrecognized argument: %s\n", argv[i]);
					return;
				}
			}
		}

		if (help_flag || version_flag)
		{
			return;
		}

		if ((mode == MMT_EXTRACT || mode == MMT_GOP_START) && input_filename == 0)
		{
			fputs("Option requires at least one folder name argument.\n", stderr);
			return;
		}

		if (mode == MMT_EXTRACT && file_prefix == 0)
		{
			prefix_buffer = Kumu::PathSetExtension(input_filename, "") + "_";
			file_prefix = prefix_buffer.c_str();
		}

		error_flag = false;
	}
};
