# FreeDcpPlayer

Free Dcp Player Nvidia GPU based for independent filmmakers

Source code will be shared here soon.

FreeDcpPlayer is a “Digital Cinema Package”  player ,  a project mainly based on
- Nvidia jpeg2000 GPU decoder (Pascal and more recent architecture only)
- Asdcplib
- SDL2

The current version, for Windows 64, is in a beta state, for testing purpose only, limited to simple DCP :
SMPTE, 2k, with 5.1 soundtrack, and optionally subtitles embedded in MXF file, not encrypted.

The command line is for example for audio device 0 and display 0 , assuming c:/mydcp/ contains the file ASSETMAP.xml

FreeDcpPlayer "c:/mydcp" -a 0 -d 0

Interface is keyboard based (Esc to stop, space to play  and pause, and left right arrows for fast rewind or play). 
User can also double-clic on the picture  : the horizontal position of the click give the start position of the lecture.
Press “i” to enable or disable progress bar.

A GUI (also a beta version) is provided to generate the command line : RunMyDcpPlayer.exe
- Choose the DCP (ASSETMAP.xml file)
- Choose audio and display device
- Clic on "Run FreeDcpPlayer"

The program will generate a freedcpplayer.log file if the box “Log” is checked.

This program is not intended to replace professional software but can help independent filmmakers 
to check their DCP on their PC after generating their own DCP with Da Vinci Resolve or Dcp-o-matic.

Coming soon: more elaborated DCP reading, including several CPL and reels will be supported.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
