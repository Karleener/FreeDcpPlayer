/*****************************************************************************
 * Copyright (C) 2022 Karleener
 *
 * Author:  Karleener
 * Inspired by VLC dcp parser
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
  * @file CDcpParse.h
  * @Basic DCP XML parsing
  */


#pragma once

//#include <windows.h>
//#include <filesystem>
#include<vector>
#include<string>
#include<iostream>
#include <list>



#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <sys/time.h> // timings
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;
#endif
using namespace std;
struct SRational
{
    uint32_t Numerator;
    uint32_t Denominator;
};

class CAsset
{
public:

    CAsset() :
        bPackingList(false), uiSize(0),
        iDurationIntrisic(0), iEntryPoint(0), iDuration(0),uiLength(0),uiOffset(0),sID(""),sPath(""),
        sAnnotation(""),sTypeAsset(""),sFileName(""),sEditRate(""),iVol_Index(0),sKeyID(""),sFrameRate(""),sScreenAspectRatio(""),sFont("") {}
    ~CAsset() {};
    string      sID;
    string      sPath;
    string      sAnnotation;
    string      sHash;
    string      sKeyID;
    string      sFrameRate;
    string      sScreenAspectRatio;
    string      sLanguage;
    string      sFont;
    string      sTypeAsset;
    string      sFileName;
    string      sEditRate;
    int         iDurationIntrisic;
    int         iEntryPoint;
    int         iDuration;
    int         iVol_Index;
    uint32_t    uiOffset;
    uint32_t    uiLength;
    uint32_t    uiSize;
    bool        bPackingList;
    double       dFrameRate;
    void        LogAll();
};

class CAssetVector : public std::vector<CAsset*> {};


class CReel
{
public:
    CReel()
        :ptrMainPicture(NULL), ptrMainSound(NULL), ptrSubtitle(NULL)
    {};

    string sID;
    string sAnnotation;
    CAsset* ptrMainPicture;
    CAsset* ptrMainSound;
    CAsset* ptrSubtitle;
    
};

class CPL 
{
public:
    CPL() {};
    string sID;
    string sAnnotation;
    string sIcon_Id;
    string sIssueDate;
    string sIssuer;
    string sCreator;
    string sContentTitle;
    string sContentKind;
    std::vector<CReel*>   VecReel;
};

class CCplVector : public std::vector<CPL*> {};

class CDcpParse
{
public:
	void ParseDCP(vector<string>& MxfFiles, string MyPath);
    void ParseCPL(CAsset* OneAsset,  string MyPath);
    void ParsePKL(CAsset* OneAsset, string MyPath);
    bool CheckEditRate();
	CDcpParse(bool verbose_i);
    ~CDcpParse();
    CAssetVector AssetVector;
    CCplVector CplVector;
    size_t FindCaseInsensitive(std::string data, std::string toSearch, size_t pos);
    bool DecodeRational(const char* str_rational, SRational& rational);
    bool CplOk;
    bool VideoOk;
    bool SoundOk;
    bool SubOk;
    bool AssetMapOk;
    bool EditRateOk;
    bool SMPTE;
    bool Verbose;
    string DcpPath;

};

