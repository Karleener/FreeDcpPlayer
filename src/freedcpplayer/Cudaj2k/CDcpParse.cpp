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
  * @file CDcpParse.cpp
  * @Basic DCP XML parsing
  */


//#include "pch.h"
#include "CDcpParse.h"
#include "pugixml.hpp"
#include <cstring>
#include <algorithm>


void Logdcp(const char *aff, auto  Name)
{
	FILE * fp=fopen("dcplog.log", "a+");
	fprintf(fp, aff, Name);
	fclose(fp);
}

size_t CDcpParse::FindCaseInsensitive(std::string data, std::string toSearch, size_t pos = 0)
{
	transform(data.begin(), data.end(), data.begin(), ::tolower);
	transform(toSearch.begin(), toSearch.end(), toSearch.begin(), ::tolower);
	return data.find(toSearch, pos);
}


void CAsset::LogAll()
{

	Logdcp( "Id              = %s\n", sID.c_str());
	Logdcp( "Path            = %s\n", sPath.c_str());
	Logdcp( "AnnotationText  = %s\n", sAnnotation.c_str());
	Logdcp( "PKL          = %s\n", bPackingList ? "True" : "False");
	Logdcp( "Hash            = %s\n", sHash.c_str());
	Logdcp( "Size            = %i\n", uiSize);
	Logdcp( "Type            = %s\n", sTypeAsset.c_str());
	Logdcp( "OrignalFileName = %s\n", sFileName.c_str());
	Logdcp("Volume Index  = %i\n", iVol_Index);
	Logdcp("offset  = %i\n", uiOffset);
	Logdcp("Length  = %i\n", uiLength);
	Logdcp("Duration  = %i\n", iDuration);
	Logdcp("Frame rate  = %s\n", sFrameRate.c_str());
	Logdcp("Edit rate  = %s\n", sEditRate.c_str());
	Logdcp("Entry point  = %i\n\n\n", iEntryPoint);
}

CDcpParse::CDcpParse(bool verbose_i=false):Verbose(verbose_i)
{
	 CplOk= VideoOk= SoundOk = 	SubOk = AssetMapOk= EditRateOk =false;
	 DcpPath = "";
	 SMPTE = true;
}

CDcpParse::~CDcpParse()
{
	int scpl = CplVector.size();
	for (int i = 0; i < scpl; i++)
	{
		int s = CplVector[i]->VecReel.size();
		for (int k = 0; k <s; k++) delete   CplVector[i]->VecReel[k];
		delete CplVector[i];
	}
	int sasset = AssetVector.size();
	for (int i = 0; i < sasset; i++)
	{
		delete AssetVector[i];
	}


}

// read ASSETMAP
//
void CDcpParse::ParseDCP(vector<string>& MxfFiles, string MyPath)
{
	pugi::xml_document doc;
	string source = MyPath + "/ASSETMAP.xml";
	fs::path fspath(source);
	if (!exists(fspath))
	{
		source = MyPath + "/ASSETMAP"; //try interop
		fs::path fspath(source);
		if (!exists(fspath)) { fprintf(stderr, "Assetmap file not found"); return; }
		SMPTE = false;
	}

	DcpPath = MyPath;
	pugi::xml_parse_result res = doc.load_file(source.c_str());
	vector <string> NomFichiermxf;
	MxfFiles.resize(4);
	MxfFiles[0] = ""; // video file name
	MxfFiles[1] = ""; // audio file name
	MxfFiles[2] = ""; // subtitle file name
	MxfFiles[3] = ""; // font file name

	pugi::xml_node Asset = doc.child("AssetMap").child("AssetList").child("Asset");

	if (res)
	{
		pugi::xml_node assets = doc.child("AssetMap").child("AssetList");
		for (pugi::xml_node asset = assets.first_child(); asset; asset = asset.next_sibling())
		{
			CAsset* OneAsset = new CAsset();
			for (pugi::xml_node attr = asset.first_child(); attr; attr = attr.next_sibling())
			{
				//std::cout << " " << attr.name() << "=" << attr.value()<<endl;
				string AssetNodeName = attr.name();
				if (AssetNodeName == "Id")
				{
					OneAsset->sID = attr.first_child().value();
				}
				if (AssetNodeName == "PackingList")
				{
					string v = attr.first_child().value();
					if (v == "true") OneAsset->bPackingList; else OneAsset->bPackingList = false;
				}
				if (AssetNodeName == "ChunkList")
				{
					for (pugi::xml_node chunk = attr.first_child(); chunk; chunk = chunk.next_sibling())
					{

						for (pugi::xml_node ChunkNode = chunk.first_child(); ChunkNode; ChunkNode = ChunkNode.next_sibling())
						{
							string cattr = ChunkNode.first_child().value();
							string ChunkNodeName(ChunkNode.name());
							if (ChunkNodeName == "Path")	OneAsset->sPath = cattr;
							if (ChunkNodeName == "VolumeIndex")	OneAsset->iVol_Index = atoi(cattr.c_str());
							if (ChunkNodeName == "Offset")	OneAsset->uiOffset = atoi(cattr.c_str());
							if (ChunkNodeName == "Length")	OneAsset->uiLength = atoi(cattr.c_str());
							//if ()

						}
					}
				}
			}
			AssetVector.push_back(OneAsset);
		}
	}
	else { AssetMapOk = false; return; }
	AssetMapOk = true;
	size_t found;
	for (int i = 0; i < AssetVector.size(); i++)
	{
		found = FindCaseInsensitive(AssetVector[i]->sPath, "cpl");
		if (found != string::npos)
		{
			CplOk = true;
			ParseCPL(AssetVector[i],MyPath);
		}
	}
	if (CplOk)
	for (int i = 0; i < AssetVector.size(); i++)
	{
		found = FindCaseInsensitive(AssetVector[i]->sPath, "pkl");
		if (found != string::npos)
		{
			ParsePKL(AssetVector[i], MyPath);
		}
	}


	if (CplVector.size() == 0) { CplOk = false; return; }
	if (Verbose)
	{
		fs::path fslog("dcplog.log");
		if (exists(fslog)) fs::remove(fslog);
		for (int i = 0; i < CplVector.size(); i++)
		{
			for (int k = 0; k < CplVector[i]->VecReel.size(); k++)
			{
				if (CplVector[i]->VecReel[k]->ptrMainPicture) CplVector[i]->VecReel[k]->ptrMainPicture->LogAll();
				if (CplVector[i]->VecReel[k]->ptrMainSound) CplVector[i]->VecReel[k]->ptrMainSound->LogAll();
				if (CplVector[i]->VecReel[k]->ptrSubtitle) CplVector[i]->VecReel[k]->ptrSubtitle->LogAll();
			}
		}
		if (CheckEditRate()) Logdcp("Check Frame rate              = %s\n", "ok"); else Logdcp("Check Frame rate              = %s\n", "Failed");
	}

	//for (int i = 0; i < AssetVector.size(); i++)
	//{
	//	AssetVector[i]->LogAll();
	//}
	

} // end dcp parse


void CDcpParse::ParseCPL(CAsset* OneAsset,  string MyPath)
{
	int ki=0;
	CPL* OneCpl = new CPL();
	pugi::xml_document doc;
	string source = MyPath+"/" + OneAsset->sPath;
	pugi::xml_parse_result res = doc.load_file(source.c_str());

	pugi::xml_node NodeId = doc.child("CompositionPlaylist").child("Id");
	OneCpl->sID = NodeId.first_child().value();
	pugi::xml_node NodeCreator = doc.child("CompositionPlaylist").child("Creator");
	OneCpl->sCreator = NodeCreator.first_child().value();
	pugi::xml_node NodeAnnotation = doc.child("CompositionPlaylist").child("AnnotationText");
	OneCpl->sAnnotation = NodeAnnotation.first_child().value();

	pugi::xml_node nodereel = doc.child("CompositionPlaylist").child("ReelList");
	// for all reels
	for (pugi::xml_node reel = nodereel.first_child(); reel; reel = reel.next_sibling())
	{
		CReel* OneReel = new CReel();
		//MainPicture
		pugi::xml_node MainPicture = reel.child("AssetList").child("MainPicture");
		if (MainPicture)
		{
			string mpid = MainPicture.child("Id").first_child().value();
			ki = -1;
			for (int i = 0; i < AssetVector.size(); i++)
			{
				if (mpid == AssetVector[i]->sID) {
					ki = i; break;
				}
			}
			if (ki != -1) 
			{
				AssetVector[ki]->sEditRate = MainPicture.child("EditRate").first_child().value();
				AssetVector[ki]->iDurationIntrisic = atoi(MainPicture.child("IntrinsicDuration").first_child().value());
				AssetVector[ki]->iEntryPoint = atoi(MainPicture.child("EntryPoint").first_child().value());
				AssetVector[ki]->iDuration = atoi(MainPicture.child("Duration").first_child().value());
				AssetVector[ki]->sFrameRate = MainPicture.child("FrameRate").first_child().value();
				SRational Fr;
				if( DecodeRational(AssetVector[ki]->sFrameRate.c_str(), Fr)) AssetVector[ki]->dFrameRate=double(Fr.Numerator)/ double(Fr.Denominator);
				AssetVector[ki]->sScreenAspectRatio = MainPicture.child("ScreenAspectRatio").first_child().value();
				string Annotation = MainPicture.child("AnnotationText").first_child().value();
				if (Annotation != "") AssetVector[ki]->sAnnotation = Annotation;

				OneReel->ptrMainPicture = AssetVector[ki];
				VideoOk = true;
			}

		}
		// MainSound

		pugi::xml_node MainSound = reel.child("AssetList").child("MainSound");
		if (MainSound)
		{
			string mpid = MainSound.child("Id").first_child().value();
			ki = -1;
			for (int i = 0; i < AssetVector.size(); i++)
			{
				if (mpid == AssetVector[i]->sID) {
					ki = i; break;
				}
			}
			if (ki != -1)
			{
				AssetVector[ki]->sEditRate = MainSound.child("EditRate").first_child().value();
				AssetVector[ki]->iDurationIntrisic = atoi(MainSound.child("IntrinsicDuration").first_child().value());
				AssetVector[ki]->iEntryPoint = atoi(MainSound.child("EntryPoint").first_child().value());
				AssetVector[ki]->iDuration = atoi(MainSound.child("Duration").first_child().value());
				AssetVector[ki]->sFrameRate = MainSound.child("FrameRate").first_child().value();
				SRational Fr;
				if (DecodeRational(AssetVector[ki]->sFrameRate.c_str(), Fr)) AssetVector[ki]->dFrameRate = double(Fr.Numerator) / double(Fr.Denominator);
				AssetVector[ki]->sScreenAspectRatio = MainSound.child("ScreenAspectRatio").first_child().value();
				string Annotation = MainSound.child("AnnotationText").first_child().value();
				if (Annotation != "") AssetVector[ki]->sAnnotation = Annotation;
				OneReel->ptrMainSound = AssetVector[ki];
				SoundOk = true;
			}
		}

		// MainSubTitle

		pugi::xml_node MainSubtitle = reel.child("AssetList").child("MainSubtitle");
		if (MainSubtitle)
		{
			string mpid = MainSubtitle.child("Id").first_child().value();
			ki = -1;
			for (int i = 0; i < AssetVector.size(); i++)
			{
				if (mpid == AssetVector[i]->sID) {
					ki = i; break;
				}
			}
			if (ki != -1)
			{
				AssetVector[ki]->sEditRate = MainSubtitle.child("EditRate").first_child().value();
				AssetVector[ki]->iDurationIntrisic = atoi(MainSubtitle.child("IntrinsicDuration").first_child().value());
				AssetVector[ki]->iEntryPoint = atoi(MainSubtitle.child("EntryPoint").first_child().value());
				AssetVector[ki]->iDuration = atoi(MainSubtitle.child("Duration").first_child().value());
				AssetVector[ki]->sFrameRate = MainSubtitle.child("FrameRate").first_child().value();
				AssetVector[ki]->sScreenAspectRatio = MainSubtitle.child("ScreenAspectRatio").first_child().value();
				string Annotation = MainSubtitle.child("AnnotationText").first_child().value();
				if (Annotation != "") AssetVector[ki]->sAnnotation = Annotation;
				OneReel->ptrSubtitle = AssetVector[ki];
				SubOk = true;
			}

		}
		OneCpl->VecReel.push_back(OneReel);
	} // for reels
	CplVector.push_back(OneCpl);
}

bool CDcpParse::CheckEditRate()
{
	int Error = 0;
	for (int i = 0; i < CplVector.size(); i++)
	{
		for (int k = 0; k < CplVector[i]->VecReel.size(); k++)
		{
			if (CplVector[i]->VecReel[k]->ptrMainPicture && CplVector[i]->VecReel[k]->ptrMainSound)
				if (CplVector[i]->VecReel[k]->ptrMainPicture->sEditRate != CplVector[i]->VecReel[k]->ptrMainSound->sEditRate) Error++;
			if (CplVector[i]->VecReel[k]->ptrMainPicture && CplVector[i]->VecReel[k]->ptrSubtitle)
				if (CplVector[i]->VecReel[k]->ptrMainPicture->sEditRate != CplVector[i]->VecReel[k]->ptrSubtitle->sEditRate) Error++;
			if (CplVector[i]->VecReel[k]->ptrSubtitle && CplVector[i]->VecReel[k]->ptrMainSound)
				if (CplVector[i]->VecReel[k]->ptrSubtitle->sEditRate != CplVector[i]->VecReel[k]->ptrMainSound->sEditRate) Error++;
		}
	}
	if (Error > 0) EditRateOk = false; else EditRateOk = true;
	return EditRateOk;

}

void CDcpParse::ParsePKL(CAsset* OneAsset, string MyPath)
{
	int ki = 0;
	pugi::xml_document doc;
	string source = MyPath + "/" + OneAsset->sPath;
	pugi::xml_parse_result res = doc.load_file(source.c_str());
//	pugi::xml_node Node = doc.child("PackingList").child("Creator");
	pugi::xml_node nodeAsset = doc.child("PackingList").child("AssetList");
	// for all assets
	for (pugi::xml_node Asset = nodeAsset.first_child(); Asset; Asset = Asset.next_sibling())
	{
		if (Asset)
		{
			string mpid = Asset.child("Id").first_child().value();
			ki = -1;
			for (int i = 0; i < AssetVector.size(); i++)
			{
				if (mpid == AssetVector[i]->sID) {
					ki = i; break;
				}
			}
			if (ki != -1)
			{
				string Annotation= Asset.child("AnnotationText").first_child().value();
				if (Annotation!="" && AssetVector[ki]->sAnnotation=="") 	AssetVector[ki]->sAnnotation = Annotation;
				AssetVector[ki]->sTypeAsset = Asset.child("Type").first_child().value();
				if (AssetVector[ki]->sTypeAsset == "application/ttf")
				{
					string subpath = AssetVector[ki]->sPath.substr(0, 36);
					
					for (int j = 0; j < CplVector.size(); j++)
					{
						for (int u = 0; u < CplVector[j]->VecReel.size(); u++)
						{
							if (CplVector[j]->VecReel[u]->ptrSubtitle)
							{
							
							string uuid = CplVector[j]->VecReel[u]->ptrSubtitle->sID.substr(9, CplVector[j]->VecReel[u]->ptrSubtitle->sID.size() - 9);
							if (subpath == uuid)
								CplVector[j]->VecReel[u]->ptrSubtitle->sFont = AssetVector[ki]->sPath;
							}
						}
					}
				}

			}
		}
	}
	int k = 0;
}


bool CDcpParse::DecodeRational(const char* str_rational, SRational& rational)
{
	if (!str_rational) return false;
	if (strlen(str_rational) == 0) return false;
	rational.Numerator = strtol(str_rational, 0, 10);
	const char* p = str_rational;
	while (*p && isdigit(*p))
	{
		++p;
	}
	if (p[0] == 0 || p[1] == 0)
	{
		return false;
	}
	++p;
	rational.Denominator = strtol(p, 0, 10);
	return  true;
}
