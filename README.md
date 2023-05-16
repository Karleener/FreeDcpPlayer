# Free Dcp Player

## Intro

Free Dcp Player Nvidia GPU based for independent filmmakers

Version 0.6.2 
support multichannel audio (i.e. 7.1, with 5.1 or stereo downsampling)
support non zero entrypoint
Note that the progress bar show the position in the current reel or mxf, not the position in the full movie. To be modified later.

Support 4K DCP !!

FreeDcpPlayer is a “Digital Cinema Package”  player ,  a project mainly based on
- Nvidia jpeg2000 GPU decoder (Pascal and more recent architecture only i.e. from series 10, such as GTX 1060) - https://developer.nvidia.com/blog/accelerating-jpeg-2000-decoding-for-digital-pathology-and-satellite-images-using-the-nvjpeg2000-library/
- Asdcplib (https://github.com/cinecert/asdcplib)
- SDL2 (https://www.libsdl.org/download-2.0.php)
- SDL_TTF (https://github.com/libsdl-org/SDL_ttf)
- Wxwidgets (https://www.wxwidgets.org/downloads/)

Some part of the code is partially inspired from VLC DCP project from Nicolas Bertrand.

The current version of FreeDcpPlayer,  is in a beta state, for testing purpose only, limited to simple DCP :
SMPTE or Interop, 2k and 4k (version 0.6.0), with stereo or 5.1 soundtrack, and optionally subtitles, not encrypted.

The wxWidgets based interface allows to launch the main program which is Windows and Ubuntu compatible.

## Keyboard shortcuts

 - Stop = Esc
 - Play = space bar
 - Fast rewind or play = left right arrow keys
 - Frame by frame = up and down arrows during stop
 - toggle progress bar = i
 - Press "j" to enable or disable current fps information (from the image processing time).
 - User can also double-click on the picture  : the horizontal position of the click give the start position of the lecture.

## How to use

 - Choose the DCP (folder containing ASSETMAP.xml or ASSETMAP file)
 - Choose audio and display device
 - Choose 5.1 output if available.
 - Clic on "Run FreeDcpPlayer"
 - Press space bar to start playing

This version has been tested sucessfully with a Geforce 1060, 1070, 2080, 3050, Quadro M2000 under Windows 10 , Windows 11, and Ubuntu 20.04. 

## Ubuntu installation

### Install AS-DCP Lib

```
git clone https://github.com/cinecert/asdcplib
cd asdcplib
cmake .
make
sudo make install
```

### Add the nvidia jpeg2000 Ubuntu repository

Follow the instructions on the nvidia page:

https://developer.nvidia.com/nvjpeg

Choose the .deb option. This will add the nvjpeg repository to your system.

### Install the required Ubuntu packages

```
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-ttf-2.0-0 libwxgtk3.0-gtk3-dev libnvjpeg-dev-12-0
```

### Compilation and installation

```
cd src/freedcpplayer
cmake .
make
sudo make install
```

### How to run

Just type in:

```
freedcpplayer
```

## Troubeshooting & Tips

The program won't work on Maxwell architecture or older (GTX 9xx series)

If you disable "Full resolution" , the images are displayed without any scaling (i.e. one screen pixel = one image pixel).
Real time 4k (or even 2K) decoding/rendering can be slow. You can "Enable Half resolution decoding" in order to accelerate.

The program will generate a freedcpplayer.log file if the box “Log” is checked.

You can test the player with the trailer of my short film "Croquis d'audience". 
This DCP Format is DCI scope, in french, with english subtitle.
It has been encoded using Da Vinci Resolve, and Subtitle are added using Dcp-o-matic.

https://drive.google.com/file/d/1o6mZ97XvE4VkKvm5fVJQQruFNIWV9dA0/view?usp=sharing

Tutorial on Youtube
https://www.youtube.com/watch?v=Cni9rMPJAGU

Another link for testing DCP
https://dpel.aswf.io/asc-stem2/


This program is not intended to replace professional software but can help independent filmmakers 
to check their DCP on their PC after generating their own DCP with Da Vinci Resolve or Dcp-o-matic.

For beta tester : in case of problem, you can send me the file freedcpplayer.log to the email adresse  : karleener at orange.fr

# License (MIT)

If not specified otherwise (see individual files):

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Changelog

Minor modifications 03/11/2022 - version 0.6.1
- small speed improvement, allowing full resolution 4K (at least for DCI Scope) decoding and screening
- add command line support. 
For example 
freedcpplayer "c:\mydcpdir\Myshortfilm_SHR-1-25_S_Fr-EN_FR-NR_51_2K_karleener_20171009_SMPTE_OV" -a 3 -d 1 -i -j
type freedcpplayer -h for help. If you run freedcpplayer without option, the GUI will be used.
The command option allows screening DCPs in a batch file, optionnally scheduled by the OS.

Major modifications 15/10/22 - version 0.6.0
- Add 4k decoding thanks to the 0.6.0 version of nvjpeg2000 by Nvidia
- Half resolution decoding option (full resolution by default)
- direct play without pause

Note that if you change the hardware (adding/removing a screen or audio interface), it is necessary to close FreeDcpPlayer, delete the config.txt file, then restart FreeDcpPlayer


Minor modifications 22/03/22 - version 0.4.3
- added fast navigation with page up and page down keys (10% of reel duration)
- navigation with left and right keys are now 5% of reel duration
- disable the possibility to quit the interface while player is running: user have to end the player first using 'esc'.
- added openjpeg licence which is used by Nvidia nvjpeg2000 library
