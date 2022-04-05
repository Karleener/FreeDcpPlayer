# Free Dcp Player

Free Dcp Player Nvidia GPU based for independent filmmakers

FreeDcpPlayer is a “Digital Cinema Package”  player ,  a project mainly based on
- Nvidia jpeg2000 GPU decoder (Pascal and more recent architecture only i.e. from series 10, such as GTX 1060) - https://developer.nvidia.com/blog/accelerating-jpeg-2000-decoding-for-digital-pathology-and-satellite-images-using-the-nvjpeg2000-library/
- Asdcplib (https://github.com/cinecert/asdcplib)
- SDL2 (https://www.libsdl.org/download-2.0.php)
- SDL_TTF (https://github.com/libsdl-org/SDL_ttf)

Some part of the code is partially inspired from VLC DCP project from Nicolas Bertrand.

The current version of FreeDcpPlayer,  is in a beta state, for testing purpose only, limited to simple DCP :
SMPTE or Interop, 2k, with stereo or 5.1 soundtrack, and optionally subtitles, not encrypted.

A new wxWidgets based interface allows to lunch the main program, x64 anx Ubuntu compatible.

Interface is keyboard based (Esc to stop, space to play  and pause, and left right arrows for fast rewind or play). 
image per image mode is possible in paused mode, using up and down arrows.
User can also double-clic on the picture  : the horizontal position of the click give the start position of the lecture.
Press “i” to enable or disable progress bar.
Press "j" to enable or disable current fps information (from the image processing time).

- Choose the DCP (folder containing ASSETMAP.xml or ASSETMAP file)
- Choose audio and display device
- Choose 5.1 output if available.
- Clic on "Run FreeDcpPlayer"

This version has been tested sucessfully with a Geforce 1060, 1070, 2080,3050, under Windows 10 , Windows 11, and Ubuntu 20.04

The program won't work on Maxwell architecture or older (GTX 9xx series)

If you disable "Full resolution" , image is screened with a 100% scale (One screen pixel = one image pixel).

The program will generate a freedcpplayer.log file if the box “Log” is checked.

You can test the player with the trailer of my short film "Croquis d'audience". 
This DCP Format is DCI scope, in french, with english subtitle.
It has been encoded using Da Vinci Resolve, and Subtitle are added using Dcp-o-matic.

https://drive.google.com/file/d/1o6mZ97XvE4VkKvm5fVJQQruFNIWV9dA0/view?usp=sharing

Tutorial on Youtube
https://www.youtube.com/watch?v=Cni9rMPJAGU


This program is not intended to replace professional software but can help independent filmmakers 
to check their DCP on their PC after generating their own DCP with Da Vinci Resolve or Dcp-o-matic.


THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For beta tester : in case of problem, you can send me the file freedcpplayer.log to the email adresse  : karleener at orange.fr

Minor modifications 22/03/22 - version 0.4.3
- added fast navigation with page up and page down keys (10% of reel duration)
- navigation with left and right keys are now 5% of reel duration
- disable the possibility to quit the interface while player is running: use have to end the player first using 'esc'.
- added openjpeg licence which is used by Nvidia nvjpeg2000 library
