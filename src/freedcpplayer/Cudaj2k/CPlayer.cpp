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
  * @file CPlayer.cpp
  * @screen reels using Nvidia nvjp2k library 
  */

#include "CPlayer.h"


float MyCoefXYZ[3][3] = { {3.2404542, -1.5371385, -0.4985314},
			{-0.9692660, 1.8760108, 0.0415560},
			{0.0556434, -0.2040259,  1.0572252 } };

CPlayer::CPlayer(CommandOptions& Options_i, const Kumu::IFileReaderFactory& fileReaderFactory_i, fs::path full_path_i)
	
{
	Options = Options_i;
	fileReaderFactory = &fileReaderFactory_i;
	full_path = full_path_i;
	MyAudioDevice = NULL;
	mywin = NULL;
	Renderer = NULL;
	ptrReel = NULL;
	tickf = getTickFrequency();
	NextState = PAUSE;
	RestartLoop = false;
	Derive = 0.0;
	HaveSub = false;
	Af1 = Af2 = Af3 = Af4 =AfSub  = NULL;
	Font = Font32 = Font64 = NULL;
	BlackBackground = Options.BlackBackground;
	WaitLastStop = true;
	FontSize = 64;

	Context = 0;
	HMAC = 0;
	ContextPCM = 0;
	HMACPCM = 0;
	frame_count = 0;
	last_frame = 0;
	SizeAudioStruct = 0;
	SizeAudioDeviceStruct = 0;
	TimeCodeRate = 0;

	NbBlock = BUFFER_AUDIO; // pre-fill audio buffer frame number
	bytes_per_element = 2; // 12 bit images
	Lut26 = NULL;
	Lut22 = NULL;

	start_frame = Options.start_frame;
	NumBlock = 0;
	out = out_swap = NULL;
	win_w=win_h=0;
	nvjpeg2k_handle = NULL;
	AudioSuccess = VideoSuccess = false;

	output_image.pixel_data = NULL;

	pReaderPCM = NULL;
	pFrameBufferPCM = NULL;
	pReader = NULL;
	pFrameBuffer = NULL;
	GlobalBufferOneFrame = NULL;
	AudioForDevice = NULL;


	fs::path PathLog = full_path;
	PathLog /= "freedcpplayer.log";
	string LogFileName{ PathLog.string() };
	fp_log = NULL;
	fp_log = fopen(LogFileName.c_str(), "a+");

	PrepareXYZ2RGBLUT();

}

Result_t CPlayer::InitialisationReaders(CDcpParse &DcpParse, bool FirstTime, CReel *ptrReel_i)
{
	Font = Font32 = Font64 =  NULL;
	OutEscape = false;
	TimeCodeRate = 0;
	Derive = 0.0;
	if (out) SDL_FreeSurface(out);
	if (out_swap) SDL_FreeSurface(out_swap);
	out = out_swap = NULL;
	if (mywin) SDL_GetWindowSize(mywin, &win_w,&win_h);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	start_frame = Options.start_frame;
	offset_frame = start_frame-1;
	string MyPath = DcpParse.DcpPath;
	if (MyPath=="")
	{
		if (Options.verbose_flag) fprintf(fp_log, "\nError in DCP path"); return RESULT_FAIL;
	}
	MxfFiles.resize(4);
	ptrReel = ptrReel_i;
	if (DcpParse.VideoOk && ptrReel!=NULL) MxfFiles[0] = MyPath + '/' + ptrReel->ptrMainPicture->sPath;
	if (DcpParse.SoundOk && ptrReel != NULL) MxfFiles[1] = MyPath + '/' + ptrReel->ptrMainSound->sPath;
	if (DcpParse.SubOk && ptrReel != NULL)
	{
		MxfFiles[2] = MyPath + '/' + ptrReel->ptrSubtitle->sPath;
		MxfFiles[3] = MyPath + '/' + ptrReel->ptrSubtitle->sFont;
	}

	if (!DcpParse.SoundOk)
	{
		if (Options.verbose_flag) fprintf(fp_log, "\nMissing audio file"); 
		return RESULT_FAIL;
	}
	if (!DcpParse.VideoOk)
	{
		if (Options.verbose_flag) fprintf(fp_log, "\nMissing video file"); 
		return RESULT_FAIL;
	}

	pReader=new JP2K::MXFReader(*fileReaderFactory);
	pFrameBuffer=new JP2K::FrameBuffer(Options.fb_size);

	Result_t resultVideo = pReader->OpenRead(MxfFiles[0]); // read video
	if (!ASDCP_SUCCESS(resultVideo)) { 
		if (Options.verbose_flag)fprintf(fp_log, "Error: video file not found\n"); 
	return RESULT_FAIL; }
	else VideoSuccess = true;

	string NomAudio = MxfFiles[1];

	Kumu::FileReaderFactory defFactory;
	EssenceType_t EssenceType;
	Result_t result = ASDCP::EssenceType(NomAudio, EssenceType, defFactory);
	pReaderPCM = new PCM::MXFReader(defFactory);
	pFrameBufferPCM= new PCM::FrameBuffer;
	Result_t resultAudio = pReaderPCM->OpenRead(NomAudio); // read video
	if (!ASDCP_SUCCESS(resultAudio)) {
		if (Options.verbose_flag) 
		fprintf(fp_log, "Error: audio file not found\n"); 
	return RESULT_FAIL; }
	pReaderPCM->FillAudioDescriptor(ADesc);
	if (ADesc.EditRate != EditRate_23_98
		&& ADesc.EditRate != EditRate_24
		&& ADesc.EditRate != EditRate_25
		&& ADesc.EditRate != EditRate_30
		&& ADesc.EditRate != EditRate_48
		&& ADesc.EditRate != EditRate_50
		&& ADesc.EditRate != EditRate_60)
		ADesc.EditRate = Options.PictureRate();
	if (Options.fb_size != FRAME_BUFFER_SIZE)
	{
		pFrameBufferPCM->Capacity(Options.fb_size);
	}
	else
	{
		pFrameBufferPCM->Capacity(PCM::CalcFrameBufferSize(ADesc));
	}
#ifdef HAVE_OPENSSL
	if (ASDCP_SUCCESS(result) && Options.key_flag)
	{
		ContextPCM = new AESDecContext;
		result = ContextPCM->InitKey(Options.key_value);

		if (ASDCP_SUCCESS(result) && Options.read_hmac)
		{
			WriterInfo Info;
			pReaderPCM->FillWriterInfo(Info);
			if (Info.UsesHMAC)
			{
				HMACMCP = new HMACContext;
				result = HMACPCM->InitKey(Options.key_value, Info.LabelSetType);
			}
			else
			{
				fputs("File does not contain HMAC values, ignoring -m option.\n", fp_log);
			}
		}
	}
#endif // HAVE_OPENSSL

	// prepare audio
	spec.freq = ADesc.AudioSamplingRate.Numerator;
	spec.format = AUDIO_S16LSB;
	if (Options.Output51 == false) spec.channels = 2; // stereo
	else  spec.channels = 6; // 5.1 output
	NbSampleperImage = spec.freq / ADesc.EditRate.Numerator;
	tpsframe = 1.0 / (double)ADesc.EditRate.Quotient();
	NbBlock = BUFFER_AUDIO;
	if (ADesc.ChannelCount == 6 && Options.Output51==false)
	{
		SizeAudioStruct = sizeof(SFiveDotOne);
		SizeAudioDeviceStruct = sizeof(SStereo);
	}
	if (ADesc.ChannelCount == 6 && Options.Output51 == true)
	{
		SizeAudioStruct = sizeof(SFiveDotOne);
		SizeAudioDeviceStruct = sizeof(SFiveDotOne16B);
	}
	if (ADesc.ChannelCount == 2)
	{
		SizeAudioStruct = sizeof(SStereo24b);
		SizeAudioDeviceStruct = sizeof(SStereo);
	}
	if (!(ADesc.ChannelCount == 2 || ADesc.ChannelCount == 6)) { if (Options.verbose_flag) fprintf(fp_log, "Audio 5.1 and stereo are the only audio format supported\nYou audio file contains %d channels\n\n",ADesc.ChannelCount);  return RESULT_FAIL; }

	GlobalBufferOneFrame = (unsigned char*)malloc(NbSampleperImage * SizeAudioStruct * NbBlock);

	//AudioForDevice = (SStereo*)malloc(NbSampleperImage * sizeof(SStereo) * NbBlock);
	AudioForDevice = (unsigned char*)malloc(NbSampleperImage * SizeAudioDeviceStruct * NbBlock);
	//FiveDotOneAudio = (SFiveDotUn16B*)malloc(NbSampleperImage * sizeof(SFiveDotUn16B) * NbBlock);
	spec.samples = 2048;
	spec.callback = NULL;
	spec.userdata = NULL;

	if (FirstTime)
	{
		Audiodev = SDL_OpenAudioDevice(MyAudioDevice, 0, &spec, &spec, 0);
		SDL_memset(&spec, 0, sizeof(spec));
		TTF_Init();
		NextState = PLAY;
	}
	else
	{
		NextState = PLAY;
		RestartLoop = true;
	}
	return RESULT_OK;


}

Result_t CPlayer::InitialisationJ2K()
{
	// prepare J2K decoding
	nvjpeg2kCreateSimple(&nvjpeg2k_handle);
	nvjpeg2kDecodeStateCreate(nvjpeg2k_handle, &decode_state);
	nvjpeg2kStreamCreate(&nvjpeg2k_stream);
	nvjpeg2kDecodeParamsCreate(&decode_params);

	nvjpeg2kStatus_t etat;
	etat = nvjpeg2kDecodeParamsSetRGBOutput(decode_params, 0); // modif test
	if (etat != NVJPEG2K_STATUS_SUCCESS) { if (Options.verbose_flag)fprintf(fp_log, "Cuda prepare RGB output failed\n"); 
	return RESULT_FAIL; }

	output_image.pixel_data = NULL;
	//PrepareXYZ2RGBLUT();

	pReader->FillPictureDescriptor(PDesc);
	//frame_count = PDesc.ContainerDuration;
	frame_count = ptrReel->ptrMainPicture->iDuration;
	if (Options.verbose_flag)
	{
		fprintf(fp_log, "Frame Buffer size: %u\n", Options.fb_size);
		JP2K::PictureDescriptorDump(PDesc);
	}
	if (MxfFiles[2] != "")
	{

		IndiceSub.resize(frame_count);
		Result_t resultSub = Read_timed_text_file(*fileReaderFactory, MxfFiles[2], full_path);
		if (!ASDCP_SUCCESS(resultSub)) { if (Options.verbose_flag)  fprintf(fp_log, "error: subtitle file not found\n");  return RESULT_FAIL; }
		if (Font != NULL) HaveSub = true; else  HaveSub = false;
	}
	else HaveSub = false;
	//temporaire
	HaveSub = true;
	if (Font32 == NULL) {
		if (Options.verbose_flag) fprintf(fp_log, "\nError: font not found\nDefault font will be used\n");
		fs::path PathFont = full_path ;
		PathFont /= "NotoMono-Regular.ttf";
		string FontFileName2{ PathFont.string() };
		//if (win_h != 0) FontSize = (32 * win_h) / 1080; else FontSize = 32;
		Font32 = TTF_OpenFont(FontFileName2.c_str(), 32);
		Font64 = TTF_OpenFont(FontFileName2.c_str(), 64);
		if (Font32 == NULL || Font64==NULL) if (Options.verbose_flag) fprintf(fp_log, "\nError: font %s not found\nSubtitle will be ignored\n", FontFileName2.c_str());
	}
	return RESULT_OK;

}

bool CPlayer::PrepareXYZ2RGBLUT()
{
	if (Lut26 != NULL) free(Lut26);
	if (Lut22 != NULL) free(Lut22);
	Lut26 = (unsigned short*)malloc(65536 * sizeof(unsigned short));
	Lut22 = (unsigned short*)malloc(65536 * sizeof(unsigned short));
	if (Lut26 == NULL || Lut22 == NULL) return false;
	double gamma26 = 2.6;
	double gamma22 = 1.0 / 2.2;
	static unsigned short int maxs = 0XFFFF;
	for (int i = 0; i <= 0xFFFF; i++)
	{
		double p = /*saturate_cast<ushort>*/(maxs * pow((double)i / (double(maxs)), gamma26));
		Lut26[i] = (unsigned short int)p; // faire un cast avec verif
		p = /*saturate_cast<ushort>*/(maxs * pow((double)i / (double(maxs)), gamma22));
		Lut22[i] = (unsigned short int)p; // faire un cast avec verif
	}
	return true;
}

Result_t CPlayer::Read_timed_text_file(const Kumu::IFileReaderFactory& fileReaderFactory, string inputFile, fs::path full_path)
{
	AESDecContext* Context = 0;
	HMACContext* HMAC = 0;
	TimedText::MXFReader     Reader(fileReaderFactory);
	TimedText::FrameBuffer   FrameBuffer;
	TimedText::TimedTextDescriptor TDesc;
	string extension;
	SubTitle Sub;
	Sub.SpotNumber = 0;
	Sub.TimeIn = "";
	Sub.TimeOut = "";
	Sub.FadeUpTime = "";
	Sub.FadeDownTime = "";
	LineSub Line;
	Line.Text = ""; Line.Valign = ""; Line.Vposition = 0.0f;
	MySubTitles.clear();
	Sub.Line.push_back(Line);
	MySubTitles.push_back(Sub);
	pugi::xml_document doc;
	pugi::xml_parse_result res;
	double frame_rate = ADesc.EditRate.Quotient();
	Uint32 StartFrameSub = 0;

	extension = inputFile.substr(inputFile.size() - 3, inputFile.size());
	
	if (extension == "mxf" || extension == "MXF")
	{
		Result_t result = Reader.OpenRead(inputFile);
		if (ASDCP_SUCCESS(result))
		{
			Reader.FillTimedTextDescriptor(TDesc);
			frame_rate = TDesc.EditRate.Quotient(); // shoud be the same as audio
			FrameBuffer.Capacity(Options.fb_size);

			if (Options.verbose_flag)
				TimedText::DescriptorDump(TDesc);
		}
#ifdef HAVE_OPENSSL
		if (ASDCP_SUCCESS(result) && Options.key_flag)
		{
			Context = new AESDecContext;
			result = Context->InitKey(Options.key_value);

			if (ASDCP_SUCCESS(result) && Options.read_hmac)
			{
				WriterInfo Info;
				Reader.FillWriterInfo(Info);

				if (Info.UsesHMAC)
				{
					HMAC = new HMACContext;
					result = HMAC->InitKey(Options.key_value, Info.LabelSetType);
				}
				else
				{
					fputs("File does not contain HMAC values, ignoring -m option.\n", fp_log);
				}
			}
		}
#endif // HAVE_OPENSSL
		bool IsMxf = false;
		TimedText::ResourceList_t::const_iterator ri;
		ui32_t write_count;
		std::string XMLDoc;
		std::string out_path = full_path.string();

		if (ASDCP_SUCCESS(result))
		{
			result = Reader.ReadTimedTextResource(XMLDoc, Context, HMAC);
			res = doc.load_string(XMLDoc.c_str());
			IsMxf = true;
		}
		if (res)
		{
			string sTimeCodeRate = doc.child("dcst:SubtitleReel").child("dcst:TimeCodeRate").first_child().value();
			if (sTimeCodeRate != "") TimeCodeRate = stoi(sTimeCodeRate.c_str());
			string StartTime= doc.child("dcst:SubtitleReel").child("dcst:StartTime").first_child().value();
			if (StartTime!="") StartFrameSub= DecodeTime(StartTime, frame_rate, true);
			pugi::xml_node Subtitles = doc.child("dcst:SubtitleReel").child("dcst:SubtitleList").child("dcst:Font");
			for (pugi::xml_node Subtitle = Subtitles.first_child(); Subtitle; Subtitle = Subtitle.next_sibling())
			{
				//std::cout << "asset:";
				SubTitle Sub;
				for (pugi::xml_attribute attr = Subtitle.first_attribute(); attr; attr = attr.next_attribute())
				{
					//std::cout << " " << attr.name() << "=" << attr.value()<<endl;
					string NomAtt = attr.name();
					if (NomAtt == "SpotNumber") Sub.SpotNumber = stoi(attr.value());
					if (NomAtt == "TimeIn") Sub.TimeIn = attr.value();
					if (NomAtt == "TimeOut") Sub.TimeOut = attr.value();
					if (NomAtt == "FadeUpTime") Sub.FadeUpTime = attr.value();
					if (NomAtt == "FadeDownTime") Sub.FadeDownTime = attr.value();
				}
				for (pugi::xml_node end = Subtitle.first_child(); end; end = end.next_sibling())
				{
					LineSub Line;
					Line.Text = ""; Line.Valign = ""; Line.Vposition = 0.0f;

					for (pugi::xml_attribute attr = end.first_attribute(); attr; attr = attr.next_attribute())
					{
						string NomAtt = attr.name();
						if (NomAtt == "Valign") Line.Valign = attr.value();
						if (NomAtt == "Vposition") Line.Vposition = stof(attr.value());
					}
					Line.Text = end.first_child().value();
					Sub.Line.push_back(Line);
				}
				MySubTitles.push_back(Sub);

				//std::cout << std::endl;
			}
			//Uint32 Duration = TDesc.ContainerDuration;
			Uint32 frame_count = IndiceSub.size();
			Uint32 CurrentFrame = 0;
			for (Uint32 i = 1; i < MySubTitles.size(); i++)
			{
				
				Uint32 FrameIn = min(StartFrameSub+ DecodeTime(MySubTitles[i].TimeIn, frame_rate,true),frame_count);
				Uint32 FrameOut = min(StartFrameSub+ DecodeTime(MySubTitles[i].TimeOut, frame_rate,true),frame_count);
				for (Uint32 t = CurrentFrame; t < FrameIn; t++)
				{
					IndiceSub[t] = 0; // empty
				}
				for (Uint32 t = FrameIn; t < FrameOut; t++)
				{
					IndiceSub[t] = i; // empty
				}
				CurrentFrame = FrameOut;
			}
			for (Uint32 t = CurrentFrame; t < frame_count; t++)
			{
				IndiceSub[t] = 0; // empty
			}
		}


		//if (ASDCP_SUCCESS(result) && (!Options.no_write_flag))
		//{
		//	Kumu::FileWriter Writer;
		//	result = Writer.OpenWrite("c:\\video\\test.xml");

		//	if (ASDCP_SUCCESS(result))
		//		result = Writer.Write(reinterpret_cast<const byte_t*>(XMLDoc.c_str()), XMLDoc.size(), &write_count);
		//}

		fs::path PathFont = full_path;
		PathFont /= "sub.ttf";
		string FontFileName{ PathFont.string() };
		for (ri = TDesc.ResourceList.begin(); ri != TDesc.ResourceList.end() && ASDCP_SUCCESS(result); ri++)
		{
			result = Reader.ReadAncillaryResource(ri->ResourceID, FrameBuffer, Context, HMAC);
			if (ASDCP_SUCCESS(result) && (!Options.no_write_flag))
			{
				Kumu::FileWriter Writer;
				result = Writer.OpenWrite(FontFileName);
				if (ASDCP_SUCCESS(result))
					result = Writer.Write(FrameBuffer.RoData(), FrameBuffer.Size(), &write_count);
				if (Options.verbose_flag)
					FrameBuffer.Dump(fp_log, Options.fb_dump_size);
			}
		}
		//if (win_h != 0) FontSize = (32 * win_h) / 1080; else FontSize = 32;
		Font32 = TTF_OpenFont(FontFileName.c_str(), 32);
		Font64 = TTF_OpenFont(FontFileName.c_str(), 64);
	} // if extention is MXF


	else // try xml file
	{
		res = doc.load_file(inputFile.c_str());
		if (res)
		{
			pugi::xml_node Subtitles = doc.child("DCSubtitle").child("Font");
			for (pugi::xml_node Subtitle = Subtitles.first_child(); Subtitle; Subtitle = Subtitle.next_sibling())
			{
				//std::cout << "asset:";
				SubTitle Sub;
				for (pugi::xml_attribute attr = Subtitle.first_attribute(); attr; attr = attr.next_attribute())
				{
					//std::cout << " " << attr.name() << "=" << attr.value()<<endl;
					string NomAtt = attr.name();
					if (NomAtt == "SpotNumber") Sub.SpotNumber = stoi(attr.value());
					if (NomAtt == "TimeIn") Sub.TimeIn = attr.value();
					if (NomAtt == "TimeOut") Sub.TimeOut = attr.value();
					if (NomAtt == "FadeUpTime") Sub.FadeUpTime = attr.value();
					if (NomAtt == "FadeDownTime") Sub.FadeDownTime = attr.value();
				}
				pugi::xml_node TryText = Subtitle.first_child().child("Text"); // Subtitle/Font/Text
				for (pugi::xml_node end = Subtitle.first_child(); end; end = end.next_sibling())
				{
					LineSub Line;
					pugi::xml_node text;
					Line.Text = ""; Line.Valign = ""; Line.Vposition = 0.0f;
					if (TryText != NULL)  text = end.child("Text"); else text = end;
					for (pugi::xml_attribute attr = text.first_attribute(); attr; attr = attr.next_attribute())
					{
						string NomAtt = attr.name();
						if (NomAtt == "VAlign") Line.Valign = attr.value();
						if (NomAtt == "VPosition") Line.Vposition = stof(attr.value());
					}
					Line.Text = text.first_child().value();
					Sub.Line.push_back(Line);
				}

				MySubTitles.push_back(Sub);

				//std::cout << std::endl;
			}
			//Uint32 Duration = TDesc.ContainerDuration;
			Uint32 frame_count = IndiceSub.size();
			Uint32 CurrentFrame = 0;
			for (Uint32 i = 1; i < MySubTitles.size(); i++)
			{
				double frame_rate = ptrReel->ptrMainPicture->dFrameRate; // use video frame rate
				Uint32 FrameIn = DecodeTime(MySubTitles[i].TimeIn, frame_rate,false);
				Uint32 FrameOut = DecodeTime(MySubTitles[i].TimeOut, frame_rate,false);
				for (Uint32 t = CurrentFrame; t < FrameIn; t++)
				{
					IndiceSub[t] = 0; // empty
				}
				for (Uint32 t = FrameIn; t < FrameOut; t++)
				{
					IndiceSub[t] = i; // empty
				}
				CurrentFrame = FrameOut;
			}
			for (Uint32 t = CurrentFrame; t < frame_count; t++)
			{
				IndiceSub[t] = 0; // empty
			}
		}
		//if (win_h != 0) FontSize = (32 * win_h) / 1080; else FontSize = 32;
		if (MxfFiles[3] != "")
		{
			Font32 = TTF_OpenFont(MxfFiles[3].c_str(), 32);
			Font32 = TTF_OpenFont(MxfFiles[3].c_str(), 64);
		}
	}

	return RESULT_OK;
}


void CPlayer::PrepareFirstAudioBuffering(void *Param)
{
	CPlayer* pPlayer = (CPlayer*)Param;
	int r=0;
	int NbBlockBufferAudio = BUFFER_AUDIO;// pPlayer->NbBlock;
	pPlayer->AudioSuccess = false;
	pPlayer->last_frame = pPlayer->frame_count;
	ui32_t start_frame = pPlayer->start_frame;
	if (start_frame >= pPlayer->last_frame) { 
		if (pPlayer->Options.verbose_flag) fprintf(pPlayer->fp_log, "start_frame=%d is greater or equal than last_frame=%d\n", start_frame, pPlayer->last_frame);
		return ; }
	SDL_ClearQueuedAudio(pPlayer->Audiodev);
	SDL_PauseAudioDevice(pPlayer->Audiodev, SDL_TRUE);
#ifdef HAVE_OPENSSL
	if (ASDCP_SUCCESS(result) && Options.key_flag)
	{
		Context = new AESDecContext;
		result = Context->InitKey(Options.key_value);

		if (ASDCP_SUCCESS(result) && Options.read_hmac)
		{
			WriterInfo Info;
			Reader.FillWriterInfo(Info);

			if (Info.UsesHMAC)
			{
				HMAC = new HMACContext;
				result = HMAC->InitKey(Options.key_value, Info.LabelSetType);
			}
			else
			{
				fputs("File does not contain HMAC values, ignoring -m option.\n", fp_log);
			}
		}
	}
#endif // HAVE_OPENSSL

	int NumBlock = 0;
	Result_t resultAudio = pPlayer->pReaderPCM->ReadFrame(start_frame, *(pPlayer->pFrameBufferPCM), pPlayer->ContextPCM, pPlayer->HMACPCM);
	int lenaudio = 0;
	// first NbBlock frame audio decoding for buffering
 	for (ui32_t i = start_frame; ASDCP_SUCCESS(resultAudio) && i < start_frame + NbBlockBufferAudio; i++)
	{
		// read one audio frame and send it to the device
		resultAudio = pPlayer->pReaderPCM->ReadFrame(i, *(pPlayer->pFrameBufferPCM), pPlayer->ContextPCM, pPlayer->HMACPCM);
		if (ASDCP_SUCCESS(resultAudio))
		{
			const byte_t* p = pPlayer->pFrameBufferPCM->RoData();
			memcpy(pPlayer->GlobalBufferOneFrame + (NumBlock * pPlayer->NbSampleperImage)* pPlayer->SizeAudioStruct, p, pPlayer->pFrameBufferPCM->Size());
			lenaudio++;
		}
		NumBlock++;
		if (NumBlock >= NbBlockBufferAudio) NumBlock = 0;
	}
	if (lenaudio > 0)
	{
		if (pPlayer->ADesc.ChannelCount==6 && pPlayer->Options.Output51==false) pPlayer->From51toStereo((SFiveDotOne*)pPlayer->GlobalBufferOneFrame, (SStereo*)pPlayer->AudioForDevice, pPlayer->NbSampleperImage * lenaudio);
		if (pPlayer->ADesc.ChannelCount == 6 && pPlayer->Options.Output51 == true) pPlayer->From51to51_16B((SFiveDotOne*)pPlayer->GlobalBufferOneFrame, (SFiveDotOne16B*)pPlayer->AudioForDevice, pPlayer->NbSampleperImage * lenaudio);
		if (pPlayer->ADesc.ChannelCount == 2) pPlayer->FromStereotoStereo((SStereo24b*)pPlayer->GlobalBufferOneFrame, (SStereo*)pPlayer->AudioForDevice, pPlayer->NbSampleperImage * lenaudio);
		r= SDL_QueueAudio(pPlayer->Audiodev, (const void*)pPlayer->AudioForDevice, pPlayer->NbSampleperImage * pPlayer->SizeAudioDeviceStruct * lenaudio);
		if (r != 0) {
			if(pPlayer->Options.verbose_flag) fprintf(pPlayer->fp_log,"Audio device error\n"); 
			pPlayer->AudioSuccess = false;
		}
	}
	if (ASDCP_SUCCESS(resultAudio)) pPlayer->AudioSuccess = true;
	
	return ;
}


Result_t CPlayer::DecodeAndScreenFirstFrame(bool WaitAfterFirstFrame)
{

	//int octetsenv = NbSampleperImage * sizeof(SStereo) * NbBlock;
	NbBlock = BUFFER_AUDIO;
	thread *tPrepAudio = new thread(PrepareFirstAudioBuffering,this);

	BlockOffset = NbBlock;
	NbBlock = 1;
	NumBlock = 0;
	NbProcFrame = 1;
	NumFrameAudio = start_frame;
	//initial_start_frame = start_frame;

	// start process for the first frame
	Result_t resultVideo = pReader->ReadFrame(start_frame, *pFrameBuffer, Context, HMAC);
	bitstream_buffer = pFrameBuffer->Data();
	length = pFrameBuffer->Size();
	nvjpeg2kStatus_t etat = nvjpeg2kStreamParse(nvjpeg2k_handle, bitstream_buffer, length, 0, 0, nvjpeg2k_stream);
	if (etat != NVJPEG2K_STATUS_SUCCESS) {
		if (Options.verbose_flag) fprintf(fp_log, "\n nvjpeg2kStreamParse failed\n Image format not supported\n");
		tPrepAudio->join(); delete tPrepAudio;	return RESULT_FAIL;
	}
	etat = nvjpeg2kStreamGetImageInfo(nvjpeg2k_stream, &image_info);
	if (etat != NVJPEG2K_STATUS_SUCCESS) {
		if (Options.verbose_flag) fprintf(fp_log, "\n nvjpeg2kStreamGetImageInfo failed\n");
		tPrepAudio->join(); delete tPrepAudio; return RESULT_FAIL;
	}
	for (unsigned int c = 0; c < image_info.num_components; c++)
	{
		etat = nvjpeg2kStreamGetImageComponentInfo(nvjpeg2k_stream, &image_comp_info[c], c);
		if (etat != NVJPEG2K_STATUS_SUCCESS) { if (Options.verbose_flag) fprintf(fp_log, "\n nvjpeg2kStreamGetImageComponentInfo failed\n");  tPrepAudio->join(); delete tPrepAudio; return RESULT_FAIL; }
	}
	for (int c = 0; c < NUM_COMPONENTS; c++)
	{
		cudaError_t er = cudaMallocPitch((void**)&decode_output[c], (size_t*)&pitch_in_bytes[c], image_comp_info[c].component_width * bytes_per_element, image_comp_info[c].component_height);
		if (er != cudaSuccess) {
			if (Options.verbose_flag) fprintf(fp_log, "\n cudaMallocPitch failed for component=%d\nArchitecure Nvidia PASCAL and above required\n", c);
			tPrepAudio->join(); delete tPrepAudio; return RESULT_FAIL;
		}
	}
	height = image_comp_info[0].component_height;
	width = image_comp_info[0].component_width;

	if (mywin == NULL)
	{
		mywin = win_init_render(width, height, &Renderer, BlackBackground, Options.NumDisplay, Options.FullScreen);
		if (mywin == NULL) { if (Options.verbose_flag) fprintf(fp_log, "\n Unable to open display window=%d\n", Options.NumDisplay);   tPrepAudio->join(); delete tPrepAudio; return RESULT_FAIL; }
	}
 // we create new surface since width and weight can change with cpl change
	if (out) SDL_FreeSurface(out);
	if (out_swap) SDL_FreeSurface(out_swap);
	out = SDL_CreateRGBSurface(0, width, height, 32,rmask, gmask, bmask, amask);
	SDL_SetSurfaceBlendMode(out, SDL_BLENDMODE_NONE);
	out_swap = SDL_CreateRGBSurface(0, width, height, 32,rmask, gmask, bmask, amask);
	SDL_SetSurfaceBlendMode(out_swap, SDL_BLENDMODE_NONE);

	if (win_h > 1440) Font = Font64;
	else Font = Font32;

	float scalex = (float(width) / float(win_w));
	float scaley = (float(height) / float(win_h));
	scalef = max(scalex, scaley);
	int hf = height / scalef;
	int wf = width / scalef;
	int linits = win_h / 2 - hf / 2;
	int cinits = win_w / 2 - wf / 2;

	vchanR.resize(width * height);
	vchanG.resize(width * height);
	vchanB.resize(width * height);
	output_image.pixel_data = (void**)decode_output;
	output_image.pixel_type = NVJPEG2K_UINT16;
	output_image.pitch_in_bytes = pitch_in_bytes;
	output_image.num_components = NUM_COMPONENTS;
	bool loop = true;

	nvjpeg2kStatus_t status = nvjpeg2kDecodeImage(nvjpeg2k_handle, decode_state, nvjpeg2k_stream, decode_params, &output_image, 0); // 0 corresponds to cudaStream_t
	if (etat != NVJPEG2K_STATUS_SUCCESS) { if (Options.verbose_flag) fprintf(fp_log, "\n Cuda decoding failed\n"); tPrepAudio->join(); delete tPrepAudio; return RESULT_FAIL; }
	cudaError_t er = cudaDeviceSynchronize();
	if (er != cudaSuccess) { if (Options.verbose_flag) fprintf(fp_log, "\n Cuda synchronization error\n"); tPrepAudio->join();delete tPrepAudio; return RESULT_FAIL; }
	unsigned short* chanR = (unsigned short*)vchanR.data();
	unsigned short* chanG = (unsigned short*)vchanG.data();
	unsigned short* chanB = (unsigned short*)vchanB.data();
	er = cudaMemcpy2D(chanR, (size_t)width * sizeof(unsigned short), output_image.pixel_data[0], pitch_in_bytes[0], width * sizeof(unsigned short), height, cudaMemcpyDeviceToHost);
	if (er != cudaSuccess) { if (Options.verbose_flag) fprintf(fp_log, "\n cudaMemcpy2D Red failed\n");  	tPrepAudio->join();	delete tPrepAudio; return RESULT_FAIL; }
	er = cudaMemcpy2D(chanG, (size_t)width * sizeof(unsigned short), output_image.pixel_data[1], pitch_in_bytes[1], width * sizeof(unsigned short), height, cudaMemcpyDeviceToHost);
	if (er != cudaSuccess) { if (Options.verbose_flag) fprintf(fp_log, "\n cudaMemcpy2D  Green failed\n"); 	tPrepAudio->join();	delete tPrepAudio; return RESULT_FAIL; }
	er = cudaMemcpy2D(chanB, (size_t)width * sizeof(unsigned short), output_image.pixel_data[2], pitch_in_bytes[2], width * sizeof(unsigned short), height, cudaMemcpyDeviceToHost);
	if (er != cudaSuccess) { if (Options.verbose_flag) fprintf(fp_log, "\n cudaMemcpy2D Blue failed\n"); 	tPrepAudio->join();	delete tPrepAudio; return RESULT_FAIL; }

	Mem.mywin = mywin;
	Mem.width = width;
	Mem.height = height;
	Mem.pLut26 = Lut26;
	Mem.pLut22 = Lut22;
	Mem.scr = out;

	Mem.win_w = win_w;
	Mem.win_h = win_h;
	Mem.dstRect = { cinits, linits, wf, hf };
	Mem.base = win_h - (win_h - (height) / scalef) / 2;
	Mem.Scalef = scalef;
	Mem.FrameCount = frame_count;
	Mem.IncrustPosition = Options.IncrustPosition;
	Mem.IncrustFps = Options.IncrustFps;
	Mem.DisplayFps = 0.0;


	Mem.chanB = chanB;
	Mem.chanR = chanR;
	Mem.chanG = chanG;
	Af1 = new thread(ThreadQuarter1, &Mem);
	Af2 = new thread(ThreadQuarter2, &Mem);
	Af3 = new thread(ThreadQuarter3, &Mem);
	Af4 = new thread(ThreadQuarter4, &Mem);
	if (Af1 && Af2 && Af3 && Af4)
	{
		//	// wait for end of image drawing
		Af1->join(); delete Af1; Af1 = NULL;
		Af2->join(); delete Af2; Af2 = NULL;
		Af3->join(); delete Af3; Af3 = NULL;
		Af4->join(); delete Af4; Af4 = NULL;
	}

  	RenderImageWithSub(Renderer, Font, ref(MySubTitles), width, height, ref(IndiceSub), start_frame, ref(Mem));

	tPrepAudio->join();
	delete tPrepAudio;
	return RESULT_OK;
} 

Result_t CPlayer::MainLoop(bool WaitAfterFirstFrame)
{
	if (WaitAfterFirstFrame) NextState = PAUSE;
	else NextState = PLAY;
	
	do
	{
		Result_t resultDecod = DecodeAndScreenFirstFrame(WaitAfterFirstFrame);
		if (!ASDCP_SUCCESS(resultDecod)) return RESULT_FAIL;
		offset_frame = start_frame-1;

		AtimerinitialGlobal = MyGetCurrentTime();
		AtimePerimage = AtimerinitialGlobal;
		// start audio
		if (NextState == PLAY &&  RestartLoop ==true) SDL_PauseAudioDevice(Audiodev, SDL_FALSE);
   		RestartLoop = false;
		cudaError_t er;

		CurrentFrameNumber = start_frame + 1;
		
		while (VideoSuccess && CurrentFrameNumber < last_frame && CurrentFrameNumber >= start_frame)
		{//SDL_RaiseWindow(mywin);
			//RestartLoop = false;
			StateMachine();

			if (RestartLoop || OutEscape)
			{
				start_frame = CurrentFrameNumber;
				break;
			}

			if (NextState == PLAY)
			{
				//read one video frame
				Result_t resultVideo = pReader->ReadFrame(CurrentFrameNumber, *pFrameBuffer, Context, HMAC);
				if (ASDCP_SUCCESS(resultVideo)) VideoSuccess = true; else VideoSuccess = false;
				// read one audio frame and send it to the device
				if (NumFrameAudio + BlockOffset < last_frame)
				{
					Result_t resultAudio = pReaderPCM->ReadFrame(NumFrameAudio + BlockOffset, *pFrameBufferPCM, ContextPCM, HMACPCM);
					const byte_t* p = pFrameBufferPCM->RoData();
					memcpy(GlobalBufferOneFrame, p, pFrameBufferPCM->Size());
					if (ADesc.ChannelCount == 6 && Options.Output51 == false) 	From51toStereo((SFiveDotOne*)GlobalBufferOneFrame, (SStereo*)AudioForDevice, NbSampleperImage);
					if (ADesc.ChannelCount == 6 && Options.Output51==true ) 	From51to51_16B((SFiveDotOne*)GlobalBufferOneFrame, (SFiveDotOne16B*)AudioForDevice, NbSampleperImage);
					if (ADesc.ChannelCount == 2)	FromStereotoStereo((SStereo24b*)GlobalBufferOneFrame, (SStereo*)AudioForDevice, NbSampleperImage);
					SDL_QueueAudio(Audiodev, (const void*)AudioForDevice, NbSampleperImage * SizeAudioDeviceStruct);
				}
				// decode video
				NumFrameAudio++;
				bitstream_buffer = pFrameBuffer->Data();
				length = pFrameBuffer->Size();
				//FILE* temp = fopen("c:\\video\\temp.bin", "wb"); fwrite(bitstream_buffer, length, 1, temp); fclose(temp);
				nvjpeg2kStatus_t etat = nvjpeg2kStreamParse(nvjpeg2k_handle, bitstream_buffer, length, 0, 0, nvjpeg2k_stream);
				if (etat != NVJPEG2K_STATUS_SUCCESS) {
					if (Options.verbose_flag) fprintf(fp_log, "\n nvjpeg2kStreamParse in loop failed\n ");
					//return RESULT_FAIL;
				}
				nvjpeg2kStatus_t status = nvjpeg2kDecodeImage(nvjpeg2k_handle, decode_state, nvjpeg2k_stream, decode_params, &output_image, 0); // 0 corresponds to cudaStream_t
				if (status != NVJPEG2K_STATUS_SUCCESS) { if (Options.verbose_flag) fprintf(fp_log, "\n Cuda decoding in loop failed\n");  return RESULT_FAIL; }
				// image is decoded, still in the GPU memory



				if (Af1 && Af2 && Af3 && Af4)
				{
					// wait for end of previous image processing from XYZ to RGB
					Af1->join(); delete Af1; Af1 = NULL;
					Af2->join(); delete Af2; Af2 = NULL;
					Af3->join(); delete Af3; Af3 = NULL;
					Af4->join(); delete Af4; Af4 = NULL;

					RenderImageWithSub(Renderer, Font, ref(MySubTitles), width, height, ref(IndiceSub), CurrentFrameNumber, ref(Mem));
				} // if treads ok

				// wait for the end of decoding
				er = cudaDeviceSynchronize();
				if (er != cudaSuccess) { if (Options.verbose_flag) fprintf(fp_log, "\n Cuda synchronization in loop error\n");  return RESULT_FAIL; }



				// Synchronisation  
				// wait if the process is too fast
				// skip image is the process is too slow
				Mem.DisplayFps=Synchronisation();
				
				//write_image("c:\\video\\dcp16b", "image", output_image, image_info.image_width,image_info.image_height, image_info.num_components, image_comp_info[0].precision, true);
				
				// copy decoded image from GPU yo Host memory
				// and start new rendering threads
				unsigned short* chanR = (unsigned short*)vchanR.data();
				unsigned short* chanG = (unsigned short*)vchanG.data();
				unsigned short* chanB = (unsigned short*)vchanB.data();
				er = cudaMemcpy2D(chanR, (size_t)width * sizeof(unsigned short), output_image.pixel_data[0], pitch_in_bytes[0], width * sizeof(unsigned short), height, cudaMemcpyDeviceToHost);
				if (er != cudaSuccess) { if (Options.verbose_flag) fprintf(fp_log, "\n cudaMemcpy2D Red failed\n");  return RESULT_FAIL; }
				er = cudaMemcpy2D(chanG, (size_t)width * sizeof(unsigned short), output_image.pixel_data[1], pitch_in_bytes[1], width * sizeof(unsigned short), height, cudaMemcpyDeviceToHost);
				if (er != cudaSuccess) { if (Options.verbose_flag) fprintf(fp_log, "\n cudaMemcpy2D Green failed\n");  return RESULT_FAIL; }
				er = cudaMemcpy2D(chanB, (size_t)width * sizeof(unsigned short), output_image.pixel_data[2], pitch_in_bytes[2], width * sizeof(unsigned short), height, cudaMemcpyDeviceToHost);
				if (er != cudaSuccess) { if (Options.verbose_flag) fprintf(fp_log, "\n cudaMemcpy2D Blue failed\n");  return RESULT_FAIL; }
				Mem.chanB = chanB;
				Mem.chanR = chanR;
				Mem.chanG = chanG;
				Swap(out, out_swap);
				Mem.scr = out;
				Af1 = new thread(ThreadQuarter1, &Mem);
				Af2 = new thread(ThreadQuarter2, &Mem);
				Af3 = new thread(ThreadQuarter3, &Mem);
				Af4 = new thread(ThreadQuarter4, &Mem);

				CurrentFrameNumber++;

			} // if nextstate = play

		} // end of while frames

		// wait for the last frame rendering
		if (Af1 && Af2 && Af3 && Af4)
		{
			Af1->join(); delete Af1; Af1 = NULL;
			Af2->join(); delete Af2; Af2 = NULL;
			Af3->join(); delete Af3; Af3 = NULL;
			Af4->join(); delete Af4; Af4 = NULL;
		}
	
		RenderImageWithSub(Renderer, Font, ref(MySubTitles), width, height, ref(IndiceSub), CurrentFrameNumber, ref(Mem));
	} 
	while (CurrentFrameNumber < frame_count && !OutEscape);
	return RESULT_OK;
}

float CPlayer::Synchronisation()
{
	float DisplayFps = 0.0;
	std::chrono::time_point<std::chrono::system_clock> ATimerLoop, ATimerLocalInfo,AtimerLocal;

	// synchronisation process
	double ProcessingTime, ProcessingTimeWithDelay, WaitingTime,tempsecoule, ProcessingTimeWithDelayGlobal;
	double TimerLoop,ff ;

	ATimerLocalInfo = MyGetCurrentTime();
	ProcessingTime = Duration(ATimerLocalInfo, AtimePerimage);
	double Pe = tpsframe * double(NbProcFrame); // wait for 40 ms for 25 fps
	do
	{
		ATimerLoop = MyGetCurrentTime();
		ProcessingTimeWithDelayGlobal = Duration(ATimerLoop, AtimerinitialGlobal);

		WaitingTime = Duration(ATimerLoop, ATimerLocalInfo);
	} while (ProcessingTimeWithDelayGlobal < Pe && WaitingTime < 1.0);

	if (WaitingTime >= 1.0)
		if (Options.verbose_flag) fprintf(fp_log, "Synchronisation out of delay");

	ProcessingTimeWithDelay = Duration(ATimerLoop, AtimePerimage);
	//if (Options.verbose_flag) fprintf(fp_log, "\nwaiting time %lf\n", WaitingTime * 1000);

	//for next frame
	AtimePerimage = ATimerLocalInfo;

	AtimerLocal = MyGetCurrentTime();
	tempsecoule = Duration(AtimerLocal, AtimerinitialGlobal);
	ff = (tempsecoule * 1000.0) / (tpsframe * 1000.0);
	Derive = (tempsecoule-Pe)*1000;
	TheoreticalFrame = Uint32(round(ff));
//	TheoreticalFrame = Uint32(floor(ff));
	TheoreticalFrame+=	start_frame;
	DisplayFps = (1.0 / ProcessingTime);
	if (Options.verbose_flag) fprintf(fp_log, "Frame=%d   Theortical frame = %d   Global time = %lf frame proc. time=%lf ms    de=%lf  - proc time with delay = %lf , ff=%lf\n",
		CurrentFrameNumber, TheoreticalFrame, tempsecoule*1000, ProcessingTime * 1000, Derive, ProcessingTimeWithDelay*1000,ff);
//	if (Options.verbose_flag) fprintf(fp_log, "Frame=%d   Theortical frame = %d   Global time = %lf et ff=%lf \n",CurrentFrameNumber, TheoreticalFrame, tempsecoule * 1000,ff);

	if (TheoreticalFrame > CurrentFrameNumber)
	{
		if (Options.verbose_flag) fprintf(fp_log, "image skipped=%d\n", TheoreticalFrame - CurrentFrameNumber);
		CurrentFrameNumber = TheoreticalFrame;
		Derive = 0;
		if (CurrentFrameNumber >= last_frame - 1) CurrentFrameNumber = last_frame - 1;
		NbProcFrame = TheoreticalFrame + 1 - start_frame;
		int lenaudio = 0;
		// refill the audio buffer
		for (ui32_t au = NumFrameAudio + BlockOffset; AudioSuccess && au < min(NumFrameAudio + NbBlock + BlockOffset, frame_count); au++)
		{
			// read one audio frame and send it to the device
			Result_t resultAudio = pReaderPCM->ReadFrame(au, *pFrameBufferPCM, ContextPCM, HMACPCM);
			if (ASDCP_SUCCESS(resultAudio))
			{
				AudioSuccess = true;
				const byte_t* p = pFrameBufferPCM->RoData();
				memcpy(GlobalBufferOneFrame + (NumBlock * NbSampleperImage) * SizeAudioStruct, p, pFrameBufferPCM->Size());
				lenaudio++;
			}
			else
				AudioSuccess = false;
			NumBlock++;
			if (NumBlock >= NbBlock) NumBlock = 0;
		}
		if (lenaudio != 0)
		{
			if (ADesc.ChannelCount == 6 && Options.Output51==false) From51toStereo((SFiveDotOne*)GlobalBufferOneFrame, (SStereo*)AudioForDevice, NbSampleperImage * lenaudio);
			if (ADesc.ChannelCount == 6 && Options.Output51 == true) From51to51_16B((SFiveDotOne*)GlobalBufferOneFrame, (SFiveDotOne16B*)AudioForDevice, NbSampleperImage * lenaudio);
			if (ADesc.ChannelCount == 2)FromStereotoStereo((SStereo24b*)GlobalBufferOneFrame, (SStereo*)AudioForDevice, NbSampleperImage * lenaudio);
			int r = SDL_QueueAudio(Audiodev, (const void*)AudioForDevice, NbSampleperImage * SizeAudioDeviceStruct * lenaudio);
		}
		NumFrameAudio += NbBlock;
	}
	else
		NbProcFrame++;
	return DisplayFps;
}

void CPlayer::EndAndClear(bool LastTime) 
{

	if (output_image.pixel_data != NULL)
	{
		for (uint32_t c = 0; c < output_image.num_components; c++)
		{
			cudaFree(output_image.pixel_data[c]);
		}
	}
	if (nvjpeg2k_handle)
	{
 		nvjpeg2kStreamDestroy(nvjpeg2k_stream);
		nvjpeg2kDecodeParamsDestroy(decode_params);
		nvjpeg2kDecodeStateDestroy(decode_state);
		nvjpeg2kDestroy(nvjpeg2k_handle);
	}
	//printf("\nPlayer end - Press any Key\n\n");
	if (MyAudioDevice && LastTime) SDL_PauseAudioDevice(Audiodev, SDL_TRUE);

	// if (mywin != NULL && LastTime)
	// {   
	// 	bool loop = true;
	// 	event.type = 0;
	// 	do
	// 	{ 
	// 		SDL_PollEvent(&event);
	// 		if (event.type == SDL_KEYDOWN)
	// 		{
	// 			loop = false;
	// 		}
	// 	} while (loop);
	// }
	SDL_FlushEvents(0, SDL_LASTEVENT);

	if (out) SDL_FreeSurface(out);
	if (out_swap) SDL_FreeSurface(out_swap);
	out = out_swap = NULL;
	if (Font32) TTF_CloseFont(Font32); 
	if (Font64) TTF_CloseFont(Font64);
	if (LastTime)
	{
		if (MyAudioDevice)
		{
			SDL_ClearQueuedAudio(Audiodev);
			SDL_CloseAudioDevice(Audiodev);
		}
		if (mywin) SDL_DestroyWindow(mywin);
		SDL_Quit();
		if (Lut22 != NULL) free(Lut22);
		if (Lut26 != NULL) free(Lut26);
		TTF_Quit();	
		if (fp_log) fclose(fp_log);
	}

	if (GlobalBufferOneFrame!=NULL) free(GlobalBufferOneFrame) ;
	if (AudioForDevice!=NULL ) free(AudioForDevice);

	if ( pReaderPCM!=NULL) delete pReaderPCM;
	if( pFrameBufferPCM!=NULL) delete pFrameBufferPCM;
	if (pReader != NULL) delete pReader;
	if (pFrameBuffer != NULL) delete pFrameBuffer;
}

void CPlayer::RenderImageWithSub(SDL_Renderer* Renderer, TTF_Font* Font, vector<SubTitle>& MySubTitles, int width, int height, vector<int>& IndiceSub, Uint32 i, SMemoire& Mem)
{
	SDL_Rect MessageRect; //create a rect
	SDL_Texture* TextTexture;
 	double ScaleFont = (Mem.win_h) / 2160.0; // 1.0 for 2160 and 0.5 for 1080

	Mem.Background_Tx = SDL_CreateTextureFromSurface(Renderer, Mem.scr);
	SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
	SDL_RenderClear(Renderer);
	SDL_RenderCopy(Renderer, Mem.Background_Tx, NULL, &Mem.dstRect);
	SDL_DestroyTexture(Mem.Background_Tx); 

	if (Mem.IncrustPosition)
	{
		int PosFen = ((i * width) / Mem.FrameCount) / Mem.Scalef;
		SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 127);
		SDL_Rect R1{ 0, Mem.win_h - 30, PosFen, 5 };
		SDL_RenderFillRect(Renderer, &R1);
		SDL_SetRenderDrawColor(Renderer, 255, 255, 0, 127);
		SDL_Rect R2{ PosFen, Mem.win_h - 30,  Mem.win_w-PosFen, 5 };
		SDL_RenderFillRect(Renderer, &R2);
		char buf[512];
		sprintf(buf,"Frame %d", i);
		bool bget = get_text_and_rect(Renderer, 0, 0, buf, Font, &TextTexture, &MessageRect);
		if (bget)
		{
			MessageRect.x = 10;
			MessageRect.y = 10;

			SDL_RenderCopy(Renderer, TextTexture, NULL, &MessageRect);
			SDL_DestroyTexture(TextTexture);
		}
		//else
		//	if (Options.verbose_flag) fprintf(fp_log, "Error in frame information printing\n");
	}
	if (Mem.IncrustFps)
	{
		char buf[512];
		sprintf(buf, "fps %3.2f", Mem.DisplayFps);
		bool bget = get_text_and_rect(Renderer, 0, 0, buf, Font, &TextTexture, &MessageRect);
		if (bget)
		{
			MessageRect.x = (width / Mem.Scalef) - MessageRect.w-10;
			MessageRect.y = 10;
			SDL_RenderCopy(Renderer, TextTexture, NULL, &MessageRect);
			SDL_DestroyTexture(TextTexture);
		}
		//else
		//	if (Options.verbose_flag) fprintf(fp_log, "Error in frame information printing\n");

	}

	if (&MySubTitles != NULL)
		if (MySubTitles.size() > 0)
			if (IndiceSub[i] != 0 && MySubTitles.size() > IndiceSub[i])
			{
				Uint32 nbligne = MySubTitles[IndiceSub[i]].Line.size();
				for (ui32_t line = 0; line < nbligne; line++)
				{
					//cout << MySubTitles[IndiceSub[i]].Line[line].Text << "    ";
  					bool bget=get_text_and_rect(Renderer, 0, 0, MySubTitles[IndiceSub[i]].Line[line].Text.c_str(), Font, &TextTexture, &MessageRect);
					if (bget)
					{
						MessageRect.x = (width / 2) / Mem.Scalef - MessageRect.w / 2;
						MessageRect.y = Mem.base - MessageRect.h - ((nbligne - line) * MessageRect.h * 1.5);// -MessageRect.h;
						SDL_RenderCopy(Renderer, TextTexture, NULL, &MessageRect);
						SDL_DestroyTexture(TextTexture);
					}
				}
			}

	SDL_RenderPresent(Renderer);
	//SDL_UpdateWindowSurface(Mem.mywin);

}


void CPlayer::StateMachine()
{
	
	SDL_Event event;
  	event.type = 0;
	int Incrementation = frame_count / 10;

//	SDL_FlushEvents(0, SDL_LASTEVENT);
	SDL_PollEvent(&event);
	bool loop = true;
	if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONUP || (NextState ==PAUSE))
	{
		if (event.key.keysym.sym == SPACEBAR || NextState == PAUSE)
		{
			// pause loop
			while (loop && NextState == PAUSE)
			{
				SDL_PollEvent(&event);
				switch (event.type) {
				case SDL_KEYDOWN:
					
  					if (event.key.keysym.sym == SPACEBAR)
					{

						loop = false;
						NextState = PLAY;
						if (Options.verbose_flag) fprintf(fp_log, "Paused \n\n");
						NbProcFrame = 1;
						RestartLoop = true;
					}

					if (event.key.keysym.sym == ESCAPE)
					{
						loop = false;
					} 
					if (event.key.keysym.sym == SDLK_RIGHT )
					{
						// do not restart audio since next state = pause
						CurrentFrameNumber = min(CurrentFrameNumber + Incrementation, frame_count);
						RestartLoop = true;
						NextState = PAUSE;	
						loop = false;
						if (Options.verbose_flag)  fprintf(fp_log, "Current frame=%d\n\n", CurrentFrameNumber);
					}
					if (event.key.keysym.sym == SDLK_LEFT)
					{
						// do not restart audio since next state = pause
						if (CurrentFrameNumber > Incrementation) CurrentFrameNumber = CurrentFrameNumber - Incrementation; else CurrentFrameNumber = 0;
						RestartLoop = true;
						NextState = PAUSE;
						loop = false;
						if (Options.verbose_flag) fprintf(fp_log, "Current frame=%d\n\n", CurrentFrameNumber);
					}

					if (event.key.keysym.sym == SDLK_UP)
					{
						// do not restart audio since next state = pause
						//CurrentFrameNumber = min(CurrentFrameNumber + 1, frame_count);
						// CurrentFrameNumber will be incremented in the main loop
						RestartLoop = true;
						NextState = PAUSE;
						loop = false;
						if (Options.verbose_flag) fprintf(fp_log, "Current frame=%d\n\n", CurrentFrameNumber);
					}

					if (event.key.keysym.sym == SDLK_DOWN)
					{
						// do not restart audio since next state = pause
						if (CurrentFrameNumber > 1) CurrentFrameNumber = CurrentFrameNumber - 2; else CurrentFrameNumber = 0;
						RestartLoop = true;
						NextState = PAUSE;
						loop = false;
						if (Options.verbose_flag)  fprintf(fp_log, "Current frame=%d\n\n", CurrentFrameNumber);
					}


					break;
				case SDL_MOUSEBUTTONUP:
					if (event.type == SDL_MOUSEBUTTONUP && event.button.clicks == 2 && event.button.button == SDL_BUTTON_LEFT)
					{
						loop = false;
					}
					break;
				default:
					break;
				}
			} // end pause loop
		} // if space bar or pause

		if (event.key.keysym.sym == ESCAPE)
		{
			CurrentFrameNumber = last_frame - 1;
			OutEscape = true;
		}
		if (event.key.keysym.sym == SDLK_i)
		{
 			Mem.IncrustPosition = !Mem.IncrustPosition;
			Options.IncrustPosition = !Options.IncrustPosition;
		}
		if (event.key.keysym.sym == SDLK_j)
		{
			Mem.IncrustFps= !Mem.IncrustFps;
			Options.IncrustFps = !Options.IncrustFps;
		}

		if (loop) // not paused state
		{
			if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_LEFT ||
				event.type == SDL_MOUSEBUTTONUP || event.key.keysym.sym == SDLK_UP ||
				event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SPACEBAR)
			{
				bool Refresh = false;

				if (event.key.keysym.sym == SDLK_RIGHT)
				{
					CurrentFrameNumber = min(CurrentFrameNumber + Incrementation, frame_count);
					Refresh = true;
					NextState = PLAY;
					if (Options.verbose_flag)  {fprintf(fp_log, "Current frame = % d\n\n", CurrentFrameNumber); fprintf(fp_log, "inc % d\n", Incrementation);}
				}
				if (event.key.keysym.sym == SPACEBAR)
				{
					if (loop == true)  NextState = PAUSE;
					SDL_PauseAudioDevice(Audiodev, SDL_TRUE);
					if (Options.verbose_flag)  fprintf(fp_log, "Paused - Current frame=%d\n\n", CurrentFrameNumber); 
				}

				if (event.key.keysym.sym == SDLK_LEFT)
				{
					if (CurrentFrameNumber > Incrementation) CurrentFrameNumber = CurrentFrameNumber - Incrementation; else CurrentFrameNumber = 0;
					Refresh = true;
					NextState = PLAY;
					if (Options.verbose_flag)  {fprintf(fp_log, "Current frame=%d\n\n", CurrentFrameNumber); fprintf(fp_log, "dec %d\n", Incrementation);}

				}
				if (event.type == SDL_MOUSEBUTTONUP)
				{
					if (event.button.clicks == 2 && event.button.button == SDL_BUTTON_LEFT)
					{
						CurrentFrameNumber = (event.button.x * frame_count) / Mem.win_w;
						Refresh = true;
						event.button.clicks = 0;
						NextState = PLAY;
					}
				}
				if (Refresh)
				{
					RefreshAndContinue();
				}
			}
		} // if loop

	} // if keyboard or mouse event or pause
} // end State machine

void CPlayer::RefreshAndContinue()
{

	NumFrameAudio++;
	NbProcFrame = 1;
	RestartLoop = true;

}

int CPlayer::From51toStereo(const SFiveDotOne* GlobalBufferOneFrame, SStereo* AudioDeviceStereo, int NbSamples)
{
	if (GlobalBufferOneFrame == NULL) return -1;
	if (AudioDeviceStereo == NULL) return -2;
	if (NbSamples < 0) return -3;

	SFiveDotOne Sample51;
	float fechR, fechC, fechL, fechBR, fechBL; // discard LFE
	int echR, echC, echL, echBR, echBL;
	float fech;

	for (int j = 0; j < NbSamples; j++)
	{
		//int i = (j * BytePerSampleOutput * NbChannelOut); //  stereo output
		Sample51 = GlobalBufferOneFrame[j];
		echR = ((Sample51.R[2] << 16 | Sample51.R[1] << 8 | Sample51.R[0]) << 8) >> 8;
		fechR = (float)(echR) / float(16777216.0);
		fechR = (fechR * float(0x7fff));

		echC = ((Sample51.C[2] << 16 | Sample51.C[1] << 8 | Sample51.C[0]) << 8) >> 8;
		fechC = (float)(echC) / float(16777216.0);
		fechC = (fechC * float(0x7fff)) * 0.5F;

		echBR = ((Sample51.BR[2] << 16 | Sample51.BR[1] << 8 | Sample51.BR[0]) << 8) >> 8;
		fechBR = (float)(echBR) / float(16777216.0);
		fechBR = (fechBR * float(0x7fff));

		fech = (fechR + fechC + fechBR) / 2.5f;

		AudioDeviceStereo[j].R = short(fech);
		echL = ((Sample51.L[2] << 16 | Sample51.L[1] << 8 | Sample51.L[0]) << 8) >> 8;
		fechL = (float)(echL) / float(16777216.0);
		fechL = (fechL * float(0x7fff));

		echBL = ((Sample51.BL[2] << 16 | Sample51.BL[1] << 8 | Sample51.BL[0]) << 8) >> 8;
		fechBL = (float)(echBL) / float(16777216.0);
		fechBL = (fechBL * float(0x7fff));

		fech = (fechL + fechC + fechBL) / 2.5f;

		AudioDeviceStereo[j].L = short(fech);
	}
	return 0;
}

int CPlayer::From51to51_16B(const SFiveDotOne* GlobalBufferOneFrame, SFiveDotOne16B* AudioDevice, int NbSamples)
{
	if (GlobalBufferOneFrame == NULL) return -1;
	if (AudioDevice == NULL) return -2;
	if (NbSamples < 0) return -3;

	SFiveDotOne Sample51;
	float fechR, fechC, fechL, fechBR, fechBL,fechLFE; // discard LFE
	int echR, echC, echL, echBR, echBL,echLFE;

	for (int j = 0; j < NbSamples; j++)
	{
		Sample51 = GlobalBufferOneFrame[j];
		echL = ((Sample51.L[2] << 16 | Sample51.L[1] << 8 | Sample51.L[0]) << 8) >> 8;
		fechL = (float)(echL) / float(16777216.0);
		fechL = (fechL * float(0x7fff));
		AudioDevice[j].L = short(fechL);

		echR = ((Sample51.R[2] << 16 | Sample51.R[1] << 8 | Sample51.R[0]) << 8) >> 8;
		fechR = (float)(echR) / float(16777216.0);
		fechR = (fechR * float(0x7fff));
		AudioDevice[j].R = short(fechR);

		echC = ((Sample51.C[2] << 16 | Sample51.C[1] << 8 | Sample51.C[0]) << 8) >> 8;
		fechC = (float)(echC) / float(16777216.0);
		fechC = (fechC * float(0x7fff));
		AudioDevice[j].C = short(fechC);

		echLFE = ((Sample51.LFE[2] << 16 | Sample51.LFE[1] << 8 | Sample51.LFE[0]) << 8) >> 8;
		fechLFE = (float)(echLFE) / float(16777216.0);
		fechLFE = (fechLFE * float(0x7fff));
		AudioDevice[j].LFE = short(fechLFE);

		echBR = ((Sample51.BR[2] << 16 | Sample51.BR[1] << 8 | Sample51.BR[0]) << 8) >> 8;
		fechBR = (float)(echBR) / float(16777216.0);
		fechBR = (fechBR * float(0x7fff));
		AudioDevice[j].BR = short(fechBR);

		echBL = ((Sample51.BL[2] << 16 | Sample51.BL[1] << 8 | Sample51.BL[0]) << 8) >> 8;
		fechBL = (float)(echBL) / float(16777216.0);
		fechBL = (fechBL * float(0x7fff));
		AudioDevice[j].BL = short(fechBL);

	}
	return 0;
}


int CPlayer::FromStereotoStereo(const SStereo24b* GlobalBufferOneFrame, SStereo* AudioDeviceStereo, int NbSamples)
 {
	if (GlobalBufferOneFrame == NULL) return -1;
	if (AudioDeviceStereo == NULL) return -2;
	if (NbSamples < 0) return -3;

	SStereo24b Sample;
	float fechR, fechL; // discard LFE
	int echR,  echL ;
	float fech;
	for (int j = 0; j < NbSamples; j++)
	{

		Sample = GlobalBufferOneFrame[j];
		echR = ((Sample.R[2] << 16 | Sample.R[1] << 8 | Sample.R[0]) << 8) >> 8;
		fechR = (float)(echR) / float(16777216.0);
		fechR = (fechR * float(0x7fff));
		fech = (fechR);
		AudioDeviceStereo[j].R = short(fech);
		echL = ((Sample.L[2] << 16 | Sample.L[1] << 8 | Sample.L[0]) << 8) >> 8;
		fechL = (float)(echL) / float(16777216.0);
		fechL = (fechL * float(0x7fff));
		fech = (fechL);
		AudioDeviceStereo[j].L = short(fech);
	}
	return 0;
}


CPlayer::~CPlayer()
{
	//EndAndClear();
}

int CPlayer::getTickCount()
{
	return clock();
}

int CPlayer::getTickFrequency()
{
	return CLOCKS_PER_SEC;
}

double CPlayer::Duration(std::chrono::time_point<std::chrono::system_clock> end, std::chrono::time_point<std::chrono::system_clock> start)
{
	std::chrono::duration<double> diff = end - start;
	return diff.count();
}
std::chrono::time_point<std::chrono::system_clock> CPlayer::MyGetCurrentTime()
{
	return std::chrono::system_clock::now();
}

bool CPlayer::SelectAudioDeviceInitAudio()
{
	SDL_Init(SDL_INIT_AUDIO);
	MyAudioDevice = SDL_GetAudioDeviceName(Options.AudioDevice, 0);
	if (MyAudioDevice == NULL)
	{
		if (Options.verbose_flag) fprintf(fp_log, "Audio device %d not available, please choose another number from following list\n", Options.AudioDevice);
		const char* Proposition;
		int n = 0;
		SDL_Init(SDL_INIT_AUDIO);
		do
		{
			Proposition = SDL_GetAudioDeviceName(n, 0);
			if (Proposition) if (Options.verbose_flag) fprintf(fp_log, "Device %d is %s\n", n++, Proposition);

		} while (Proposition);
		return false;
	}
	return true;
}

void CPlayer::ThreadQuarter1(void* Param)
{
	SMemoire* Mem = (SMemoire*)Param;
	SDL_Surface* scr = Mem->scr;
	if (Mem->mywin == NULL )
	{
		return;
	}
	if ( scr==NULL)
	{
		return;
	}
	int height = Mem->height;
	int width = Mem->width;

	Uint8 bpp = scr->format->BytesPerPixel;
	Uint8 rs = scr->format->Rshift / 8;
	Uint8 rg = scr->format->Gshift / 8;
	Uint8 rb = scr->format->Bshift / 8;
	unsigned short int redbase = 3500;
	unsigned short int greenbase = 4000;
	unsigned short int bluebase = 4000;
	unsigned short int red_co_gamma;
	unsigned short int blue_co_gamma;
	unsigned short int green_co_gamma;
	int red_cor_coul;
	int green_cor_coul;
	int blue_cor_coul;
	unsigned char red_f, green_f, blue_f;
	static unsigned short int maxs = 0XFFFF;

	int h2 = height >> 1;
	int w2 = width >> 1;
	int linit = (scr->h >> 1) - h2;
	int cinit = (scr->w >> 1) - w2;

	for (int li = 0; li < h2; li++)
	{
		for (int col = 0; col < w2; col++)
		{
			int p = width * li + col;
			redbase = Mem->chanR[p];
			greenbase = Mem->chanG[p];
			bluebase = Mem->chanB[p];
			red_co_gamma = Mem->pLut26[redbase << 4];
			green_co_gamma = Mem->pLut26[greenbase << 4];
			blue_co_gamma = Mem->pLut26[bluebase << 4];
			red_cor_coul = int((float)red_co_gamma * MyCoefXYZ[0][0] + (float)green_co_gamma * MyCoefXYZ[0][1] + (float)blue_co_gamma * MyCoefXYZ[0][2]);
			green_cor_coul = int((float)red_co_gamma * MyCoefXYZ[1][0] + (float)green_co_gamma * MyCoefXYZ[1][1] + (float)blue_co_gamma * MyCoefXYZ[1][2]);
			blue_cor_coul = int((float)red_co_gamma * MyCoefXYZ[2][0] + (float)green_co_gamma * MyCoefXYZ[2][1] + (float)blue_co_gamma * MyCoefXYZ[2][2]);
			if (red_cor_coul < 0) red_cor_coul = 0;
			if (green_cor_coul < 0) green_cor_coul = 0;
			if (blue_cor_coul < 0) blue_cor_coul = 0;
			if (red_cor_coul > maxs) red_cor_coul = maxs;
			if (green_cor_coul > maxs) green_cor_coul = maxs;
			if (blue_cor_coul > maxs) blue_cor_coul = maxs;
			red_f = (unsigned char)(Mem->pLut22[red_cor_coul] >> 8);
			green_f = (unsigned char)(Mem->pLut22[green_cor_coul] >> 8);
			blue_f = (unsigned char)(Mem->pLut22[blue_cor_coul] >> 8);
			const int idx = ((li + linit) * scr->w + (col + cinit)) * bpp;
			Uint8* px = (Uint8*)scr->pixels + idx;
			*(px + rs) = (unsigned char)red_f;
			*(px + rg) = (unsigned char)green_f;
			*(px + rb) = (unsigned char)blue_f;
		}
	}
	return;
}

void CPlayer::ThreadQuarter2(void* Param)
{
	SMemoire* Mem = (SMemoire*)Param;
	SDL_Surface* scr = Mem->scr;
	if (Mem->mywin == NULL || scr==NULL)
	{
		return;
	}
	int height = Mem->height;
	int width = Mem->width;
	Uint8 bpp = scr->format->BytesPerPixel;
	Uint8 rs = scr->format->Rshift / 8;
	Uint8 rg = scr->format->Gshift / 8;
	Uint8 rb = scr->format->Bshift / 8;
	unsigned short int redbase = 3500;
	unsigned short int greenbase = 4000;
	unsigned short int bluebase = 4000;
	unsigned short int red_co_gamma;
	unsigned short int blue_co_gamma;
	unsigned short int green_co_gamma;
	int red_cor_coul;
	int green_cor_coul;
	int blue_cor_coul;
	unsigned char red_f, green_f, blue_f;
	static unsigned short int maxs = 0XFFFF;
	int h2 = height >> 1;
	int w2 = width >> 1;
	int linit = (scr->h >> 1);
	int cinit = (scr->w >> 1) - w2;

	for (int li = h2; li < height; li++)
	{
		for (int col = 0; col < w2; col++)
		{
			int p = width * li + col;
			redbase = Mem->chanR[p];
			greenbase = Mem->chanG[p];
			bluebase = Mem->chanB[p];
			red_co_gamma = Mem->pLut26[redbase << 4];
			green_co_gamma = Mem->pLut26[greenbase << 4];
			blue_co_gamma = Mem->pLut26[bluebase << 4];
			red_cor_coul = int((float)red_co_gamma * MyCoefXYZ[0][0] + (float)green_co_gamma * MyCoefXYZ[0][1] + (float)blue_co_gamma * MyCoefXYZ[0][2]);
			green_cor_coul = int((float)red_co_gamma * MyCoefXYZ[1][0] + (float)green_co_gamma * MyCoefXYZ[1][1] + (float)blue_co_gamma * MyCoefXYZ[1][2]);
			blue_cor_coul = int((float)red_co_gamma * MyCoefXYZ[2][0] + (float)green_co_gamma * MyCoefXYZ[2][1] + (float)blue_co_gamma * MyCoefXYZ[2][2]);
			if (red_cor_coul < 0) red_cor_coul = 0;
			if (green_cor_coul < 0) green_cor_coul = 0;
			if (blue_cor_coul < 0) blue_cor_coul = 0;
			if (red_cor_coul > maxs) red_cor_coul = maxs;
			if (green_cor_coul > maxs) green_cor_coul = maxs;
			if (blue_cor_coul > maxs) blue_cor_coul = maxs;
			red_f = (unsigned char)(Mem->pLut22[red_cor_coul] >> 8);
			green_f = (unsigned char)(Mem->pLut22[green_cor_coul] >> 8);
			blue_f = (unsigned char)(Mem->pLut22[blue_cor_coul] >> 8);
			const int idx = ((li + linit - h2) * scr->w + (col + cinit)) * bpp;
			Uint8* px = (Uint8*)scr->pixels + idx;
			*(px + rs) = (unsigned char)red_f;
			*(px + rg) = (unsigned char)green_f;
			*(px + rb) = (unsigned char)blue_f;
		}
	}
	return;
}

void CPlayer::ThreadQuarter3(void* Param)
{
	SMemoire* Mem = (SMemoire*)Param;
	SDL_Surface* scr = Mem->scr;
	if (Mem->mywin == NULL || scr==NULL)
	{
		return;
	}
	int height = Mem->height;
	int width = Mem->width;
	Uint8 bpp = scr->format->BytesPerPixel;
	Uint8 rs = scr->format->Rshift / 8;
	Uint8 rg = scr->format->Gshift / 8;
	Uint8 rb = scr->format->Bshift / 8;
	unsigned short int redbase = 3500;
	unsigned short int greenbase = 4000;
	unsigned short int bluebase = 4000;
	unsigned short int red_co_gamma;
	unsigned short int blue_co_gamma;
	unsigned short int green_co_gamma;
	int red_cor_coul;
	int green_cor_coul;
	int blue_cor_coul;
	unsigned char red_f, green_f, blue_f;
	static unsigned short int maxs = 0XFFFF;
	int h2 = height >> 1;
	int w2 = width >> 1;
	int linit = (scr->h >> 1) - h2;
	int cinit = (scr->w >> 1);

	for (int li = 0; li < h2; li++)
	{
		for (int col = 0; col < w2; col++)
		{
			int p = width * li + (col + w2);
			redbase = Mem->chanR[p];
			greenbase = Mem->chanG[p];
			bluebase = Mem->chanB[p];
			red_co_gamma = Mem->pLut26[redbase << 4];
			green_co_gamma = Mem->pLut26[greenbase << 4];
			blue_co_gamma = Mem->pLut26[bluebase << 4];
			red_cor_coul = int((float)red_co_gamma * MyCoefXYZ[0][0] + (float)green_co_gamma * MyCoefXYZ[0][1] + (float)blue_co_gamma * MyCoefXYZ[0][2]);
			green_cor_coul = int((float)red_co_gamma * MyCoefXYZ[1][0] + (float)green_co_gamma * MyCoefXYZ[1][1] + (float)blue_co_gamma * MyCoefXYZ[1][2]);
			blue_cor_coul = int((float)red_co_gamma * MyCoefXYZ[2][0] + (float)green_co_gamma * MyCoefXYZ[2][1] + (float)blue_co_gamma * MyCoefXYZ[2][2]);
			if (red_cor_coul < 0) red_cor_coul = 0;
			if (green_cor_coul < 0) green_cor_coul = 0;
			if (blue_cor_coul < 0) blue_cor_coul = 0;
			if (red_cor_coul > maxs) red_cor_coul = maxs;
			if (green_cor_coul > maxs) green_cor_coul = maxs;
			if (blue_cor_coul > maxs) blue_cor_coul = maxs;
			red_f = (unsigned char)(Mem->pLut22[red_cor_coul] >> 8);
			green_f = (unsigned char)(Mem->pLut22[green_cor_coul] >> 8);
			blue_f = (unsigned char)(Mem->pLut22[blue_cor_coul] >> 8);
			const int idx = ((li + linit) * scr->w + (col + cinit)) * bpp;
			Uint8* px = (Uint8*)scr->pixels + idx;
			*(px + rs) = (unsigned char)red_f;
			*(px + rg) = (unsigned char)green_f;
			*(px + rb) = (unsigned char)blue_f;
		}
	}
	return;
}

void CPlayer::ThreadQuarter4(void* Param)
{
	SMemoire* Mem = (SMemoire*)Param;
	SDL_Surface* scr = Mem->scr;
	if (Mem->mywin == NULL || scr==NULL)
	{
		return;
	}
	int height = Mem->height;
	int width = Mem->width;
	Uint8 bpp = scr->format->BytesPerPixel;
	Uint8 rs = scr->format->Rshift / 8;
	Uint8 rg = scr->format->Gshift / 8;
	Uint8 rb = scr->format->Bshift / 8;
	unsigned short int redbase = 3500;
	unsigned short int greenbase = 4000;
	unsigned short int bluebase = 4000;
	unsigned short int red_co_gamma;
	unsigned short int blue_co_gamma;
	unsigned short int green_co_gamma;
	int red_cor_coul;
	int green_cor_coul;
	int blue_cor_coul;
	unsigned char red_f, green_f, blue_f;
	static unsigned short int maxs = 0XFFFF;
	int h2 = height >> 1;
	int w2 = width >> 1;
	int linit = (scr->h >> 1);
	int cinit = (scr->w >> 1);

	for (int li = 0; li < h2; li++)
	{
		for (int col = 0; col < w2; col++)
		{
			int p = width * (li + h2) + (col + w2);
			redbase = Mem->chanR[p];
			greenbase = Mem->chanG[p];
			bluebase = Mem->chanB[p];
			red_co_gamma = Mem->pLut26[redbase << 4];
			green_co_gamma = Mem->pLut26[greenbase << 4];
			blue_co_gamma = Mem->pLut26[bluebase << 4];
			red_cor_coul = int((float)red_co_gamma * MyCoefXYZ[0][0] + (float)green_co_gamma * MyCoefXYZ[0][1] + (float)blue_co_gamma * MyCoefXYZ[0][2]);
			green_cor_coul = int((float)red_co_gamma * MyCoefXYZ[1][0] + (float)green_co_gamma * MyCoefXYZ[1][1] + (float)blue_co_gamma * MyCoefXYZ[1][2]);
			blue_cor_coul = int((float)red_co_gamma * MyCoefXYZ[2][0] + (float)green_co_gamma * MyCoefXYZ[2][1] + (float)blue_co_gamma * MyCoefXYZ[2][2]);
			if (red_cor_coul < 0) red_cor_coul = 0;
			if (green_cor_coul < 0) green_cor_coul = 0;
			if (blue_cor_coul < 0) blue_cor_coul = 0;
			if (red_cor_coul > maxs) red_cor_coul = maxs;
			if (green_cor_coul > maxs) green_cor_coul = maxs;
			if (blue_cor_coul > maxs) blue_cor_coul = maxs;
			red_f = (unsigned char)(Mem->pLut22[red_cor_coul] >> 8);
			green_f = (unsigned char)(Mem->pLut22[green_cor_coul] >> 8);
			blue_f = (unsigned char)(Mem->pLut22[blue_cor_coul] >> 8);
			const int idx = ((li + linit) * scr->w + (col + cinit)) * bpp;
			Uint8* px = (Uint8*)scr->pixels + idx;
			*(px + rs) = (unsigned char)red_f;
			*(px + rg) = (unsigned char)green_f;
			*(px + rb) = (unsigned char)blue_f;
		}
	}
	return;
}


bool CPlayer::get_text_and_rect(SDL_Renderer* renderer, int x, int y, const char* text, TTF_Font* font, SDL_Texture** texture, SDL_Rect* rect)
{
	int text_width=0;
	int text_height=0;
	SDL_Surface* surface=NULL;
	SDL_Color textColor = { 255, 255, 255, 0 };
	if (string(text) == "") return false;
	if (font == NULL) return false;
	//surface = TTF_RenderText_Solid(font, text, textColor);
	surface = TTF_RenderUTF8_Solid(font, text, textColor);
	if (surface)
	{
		*texture = SDL_CreateTextureFromSurface(renderer, surface);
		text_width = surface->w;
		text_height = surface->h;
		SDL_FreeSurface(surface);
		rect->x = x;
		rect->y = y;
		rect->w = text_width;
		rect->h = text_height;
		return true;
	}
	else return false;

}

Uint32 CPlayer::DecodeTime(string chaineTps, double frame_rate, bool TypeCs=true)
{
	int secs = 0;
	int FrameNumber;
	int h, m, s, ee,cs;
	if (TypeCs)
	{
		if (sscanf(chaineTps.c_str(), "%d:%d:%d:%d", &h, &m, &s, &ee) >= 3)
		{
			secs = h * 3600 + m * 60 + s;
		}
		FrameNumber = ee+(secs * frame_rate);
	}
	else
	{
		if (sscanf(chaineTps.c_str(), "%d:%d:%d:%d", &h, &m, &s, &cs) >= 3)
		{
			secs = ((h * 3600 + m * 60 + s) * 1000 +cs*10);
		}
		FrameNumber = (secs * frame_rate) / 1000;
	}

	return FrameNumber;
}

SDL_Window* CPlayer::win_init_render(int w, int h, SDL_Renderer** Renderer, bool BlackBackground = false, int NumDisplay = 0, bool FullScreen = false)
{
	int r;
	SDL_Window* win = NULL;
	r = SDL_Init(SDL_INIT_VIDEO);
	int displays = SDL_GetNumVideoDisplays();
	if (NumDisplay > displays || NumDisplay < 0) NumDisplay = 0;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

	// SDL_DisplayMode target, closest;
	// target.w = 1920;
	// target.h = 1080;
	// target.format = 0;  // don't care
	// target.refresh_rate = 0; // don't care
	// target.driverdata = 0; // initialize to 0
	// SDL_GetClosestDisplayMode(0, &target, &closest);


	//if (BlackBackground)
	//{
	//	SDL_Window* mainWindow = SDL_CreateWindow("Main Window", SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), 1000, 1000, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN);
	//	SDL_Renderer* mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
	//	SDL_SetRenderDrawColor(mainRenderer, 10, 10, 10, 255);
	//	SDL_RenderClear(mainRenderer);
	//	SDL_RenderPresent(mainRenderer);
	//}
	if (FullScreen)
	{
		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(NumDisplay, &DM);
		win = SDL_CreateWindow("Dcp", SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), DM.w, DM.h,  SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
		//mywinAccel = SDL_CreateWindow("DCP", SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), DM.w, DM.h,  /*SDL_WINDOW_ALWAYS_ON_TOP |*/SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
		// win for surfaces

		//SDL_SetWindowDisplayMode(win, &closest);
		//SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);

	}
	else
	{
		win = SDL_CreateWindow("Dcp", SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), w, h,  SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
		//mywinAccel = SDL_CreateWindow("DCP", SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), SDL_WINDOWPOS_CENTERED_DISPLAY(NumDisplay), w, h,  SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);

	}

	*Renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	SDL_GetWindowSize(win, &win_w,&win_h);
	assert(win != NULL);

	return win;
}


void CPlayer::Swap(SDL_Surface* &out1, SDL_Surface* &out2)
{
	SDL_Surface* temp;
	temp = out1;
	out1 = out2;
	out2 = temp;
}

 