# FreeDcpPlayer

Free Dcp Player Nvidia GPU based for independent filmmakers

Source code is in a very early state, not cleaned...

FreeDcpPlayer is a “Digital Cinema Package”  player ,  a project mainly based on
- Nvidia jpeg2000 GPU decoder (Pascal and more recent architecture only) - https://developer.nvidia.com/blog/accelerating-jpeg-2000-decoding-for-digital-pathology-and-satellite-images-using-the-nvjpeg2000-library/
- Asdcplib (https://github.com/cinecert/asdcplib)
- SDL2 (https://www.libsdl.org/download-2.0.php)
- SDL_TTF (https://github.com/libsdl-org/SDL_ttf)

Some part of the code is partially inspired from VLC DCP project from Nicolas Bertrand.

The current version of FreeDcpPlayer, for Windows 64, is in a beta state, for testing purpose only, limited to simple DCP :
SMPTE or Interop, 2k, with stereo or 5.1 soundtrack, and optionally subtitles, not encrypted.

The command line is for example for audio device 0 and display 0 , assuming c:/mydcp/ contains the file ASSETMAP.xml

FreeDcpPlayer "c:/mydcp" -a 0 -d 0

Interface is keyboard based (Esc to stop, space to play  and pause, and left right arrows for fast rewind or play). 
image per image mode is possible in paused mode, using up and down arrows.
User can also double-clic on the picture  : the horizontal position of the click give the start position of the lecture.
Press “i” to enable or disable progress bar.
Press "j" to enable or disable current fps information (from the image processing time).

A GUI (also a beta version) is provided to generate the command line : RunMyDcpPlayer.exe
- Choose the DCP (ASSETMAP.xml or ASSETMAP file)
- Choose audio and display device
- Choose 5.1 output if available.
- Clic on "Run FreeDcpPlayer"

In full resolution mode, choose a resolution screen close to 1920x1080 (in the Windows 10 display parameters) in order to avoid skipped frames.
If the resolution of the screen is too high, the number of skipped frames will increase.

If you disable "Full resolution" , image is screened with a 100% scale (One screen pixel = one image pixel).

The program will generate a freedcpplayer.log file if the box “Log” is checked.

This program is not intended to replace professional software but can help independent filmmakers 
to check their DCP on their PC after generating their own DCP with Da Vinci Resolve or Dcp-o-matic.


THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
