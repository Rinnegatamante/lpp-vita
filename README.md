<p align="center">
	<img src="https://github.com/gnmmarechal/lpp-vita/raw/master/banner.png?raw=true"/>
</p>

# Description

**Lua Player Plus Vita** is the first lua interpreter made for the Sony PlayStation Vita.

The interpreter currently runs under Lua 5.3.1. 
It has also a debug FTP server to correct your errors in runtime.

Official documentation: http://rinnegatamante.it/lpp-vita_doc.html<br>
Official support board: http://rinnegatamante.it/board/forumdisplay.php?fid=12

# Features

##### Graphics

* Advanced 2D Graphics Rendering (GPU)
* Native support to TTF fonts
* Native support to BMP/PNG/JPG images

##### Sounds

* Sound system with support for WAV/MP3/MIDI/OGG audio files

##### Controls

* Controls system with full console support
* Support to dualshock rumble feature for PSTV.

##### Network
* Built-in debug FTP server
* Complete sockets system support

##### System
* Access to a lot of Sony syscalls
* I/O access for any filesystem
* Native support to ZIP archives

##### Utilities

* Timer system
* Advanced arithmetical functions support (math library)

# Nightly Builds

We offer an automated builder service for our nightly builds. Nightly builds are automatically updated every 4 hours by a server we own. To take advantage of this service, you should visit [this page](http://rinnegatamante.it/lpp-nightly.php).

# Showcase

Here are some PSVITA homebrews made using Lua Player Plus Vita:

##### Games
* [VitaSudoku](http://wololo.net/talk/viewtopic.php?f=116&t=46423)
* [4Rows](http://wololo.net/talk/viewtopic.php?f=116&t=46253)
* [Flppy Bird](http://wololo.net/talk/viewtopic.php?f=116&t=46593)
* [Deathrun to Vita](http://wololo.net/talk/viewtopic.php?f=116&t=46567)
* [Super Hero Chronicles](http://wololo.net/talk/viewtopic.php?f=116&t=46677)
* [vitaWanted](http://wololo.net/talk/viewtopic.php?f=116&t=46726)

##### Utilities
* [Moon Whitelister](https://github.com/gnmmarechal/moon-whitelister)

##### Engines & Interpreters
* [RayCast3D Engine](http://wololo.net/talk/viewtopic.php?f=116&t=46379)

# Compiling the source

To correctly compile lpp-vita you'll need **vitasdk** and **7z** installed and correctly set in your PATH environment variable. You'll also need the following libraries: **zlib**, **libmpg123**, **libogg**, **libvorbis**, **libsndfile**, **vita2dlib**, 
**libftpvita**, **libpng**, **libjpeg**, **freetype**, **libspeexdsp**. You can install most of them with **vita-portlibs** and [EasyRPG Vita toolchain](https://ci.easyrpg.org/view/Toolchains/job/toolchain-vita/)<br><br>
Lua Player Plus Vita supports different flags to enables some features:<br>
**-DSKIP_ERROR_HANDLING** disables error handling for faster code execution.<br>

# Credits

* Everyone involved in vitasdk creation
* xerpi for vita2dlib and debug FTP server code
* gnmmarechal for testing the interpreter
* Misledz for the Lua Player Plus logo
* EasyRPG Team for the Audio Decoder used for Sound module
