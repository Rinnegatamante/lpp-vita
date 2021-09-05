<p align="center">
	<img src="https://github.com/gnmmarechal/lpp-vita/raw/master/banner.png?raw=true"/>
</p>

[![Build Status](https://dev.azure.com/rinnegatamante/lpp-vita/_apis/build/status/Rinnegatamante.lpp-vita?branchName=master)](https://dev.azure.com/rinnegatamante/lpp-vita/_build/latest?definitionId=1&branchName=master)

# Description

**Lua Player Plus Vita** is the first lua interpreter made for the Sony PlayStation Vita.

The interpreter currently runs under LuaJIT 2.0.5 with slight additions from Lua 5.2.4 (for example the bit32 lib for bitwise operations). 
It has also a debug FTP server to correct your errors in runtime.

Official documentation: http://rinnegatamante.github.io/lpp-vita/

# Features

##### Graphics

* Advanced 2D Graphics Rendering (GPU).
* Basic 3D Graphics Rendering (GPU).
* Native support to OBJ models.
* Native support to TTF/PGF/PVF/OTF fonts.
* Native support to BMP/PNG/JPG images.
* Native support to GIF animated images.
* Built-in rescaler to perform portings from [lpp-3ds](https://github.com/Rinnegatamante/lpp-3ds) easily.

##### Multimedia

* Complete sound system with support for:
	* WAVEform audio files (**.wav**) with PCM8/PCM16/ADPCM codecs.
	* Audio Interchange File Format audio files (**.aif/.aiff**) with PCM8/PCM16 codecs.
	* Moving Picture Expert Group-1/2 Audio Layer 3 audio files (**.mp3**) with MPEG-1 Audio Layer III/MPEG-2 Audio Layer III codecs.
	* Musical Instrument Digital Interface audio files (**.midi**).
	* Ogg Media audio files (**.ogg/.opus**) with Vorbis/Opus codecs.
* Native support to MP4 video files with support for **.srt** and **.vtt** subtitles files.
	
##### Controls

* Controls system with full console support.
* Support to dualshock rumble feature for PSTV.
* Support to home button locking/unlocking.
* Support to lightbar color change for PSTV.
* Support to gyroscope and accelerometer reading.
* Support to both touchpads reading.

##### Network

* Built-in debug FTP server.
* Complete sockets system support.
* Native support to HTTP requests.
* Support to SSL (HTTPS) for file downloading.

##### System

* Access to a lot of Sony syscalls.
* I/O access for any filesystem.
* Native support to ZIP archives.
* Native support to SFO file descriptors.
* Native support to SQLite databases.
* Native support to apps installation/uninstallation.

##### Utilities

* Timer system.
* Advanced arithmetical functions support (math library).

# Nightly Builds

We offer an automated builder service for our nightly builds powered by Azure Pipelines CI. Nightly builds are automatically updated at each new commit. To take advantage of this service, you can find a proper release here on GitHub with the Nightly tag.

# Showcase

Here are some PSVITA homebrews made using Lua Player Plus Vita:

##### Games
* [VitaSudoku](http://wololo.net/talk/viewtopic.php?f=116&t=46423)
* [4Rows](http://wololo.net/talk/viewtopic.php?f=116&t=46253)
* [Flppy Bird](http://wololo.net/talk/viewtopic.php?f=116&t=46593)
* [Deathrun to Vita](http://wololo.net/talk/viewtopic.php?f=116&t=46567)
* [Super Hero Chronicles](http://wololo.net/talk/viewtopic.php?f=116&t=46677)
* [vitaWanted](http://wololo.net/talk/viewtopic.php?f=116&t=46726)
* [Labyrinth 3D](http://wololo.net/talk/viewtopic.php?f=116&t=46845)
* [Resident Evil CODE:Vita](http://wololo.net/talk/viewtopic.php?f=52&t=47380)
* [Lua Game of Life](https://github.com/domis4/lua-gameoflife/)
* [Galactic Federation](http://vitadb.rinnegatamante.it/#/info/206)
* [Starfield Vita](http://vitadb.rinnegatamante.it/#/info/128)
* [EUCLIOD](http://vitadb.rinnegatamante.it/#/info/263)
* [Crazy Traffic Jam 3D](http://wololo.net/talk/viewtopic.php?f=116&t=48358)
* [Zombiebound](http://wololo.net/talk/viewtopic.php?f=116&t=48583)
* [vita-tetromino](https://github.com/svennd/vita-tetromino)
* [VitaHangman](https://vitadb.rinnegatamante.it/#/info/306)
* [Tetromino: Touhou Edition](https://vitadb.rinnegatamante.it/#/info/376)
* [Vita Hangman: Touhou Edition](https://vitadb.rinnegatamante.it/#/info/379)
* [Cookie Clicker](https://vitadb.rinnegatamante.it/#/info/351)
* [Vitamon GO](https://vitadb.rinnegatamante.it/#/info/353)
* [vita-chain](https://vitadb.rinnegatamante.it/#/info/339)
* [ViTanks](https://vitadb.rinnegatamante.it/#/info/472)

##### Utilities
* [Moon Whitelister](https://github.com/gnmmarechal/moon-whitelister)
* [luaIrc](http://wololo.net/talk/viewtopic.php?f=116&t=47107)
* [rinCheat SE](https://github.com/Rinnegatamante/rinCheat/tree/master/SE_module)
* [Simple Text Editor](http://wololo.net/talk/viewtopic.php?f=116&t=47018)
* [ePSP Bubble Installer](http://vitadb.rinnegatamante.it/#/info/124)
* [Vita Object Loader](http://wololo.net/talk/viewtopic.php?f=116&t=47982)
* [Voice Recorder Vita](http://wololo.net/talk/viewtopic.php?f=116&t=48444)
* [Pro Camera Vita](http://wololo.net/talk/viewtopic.php?f=116&t=48379)
* [Notepad Vita](http://wololo.net/talk/viewtopic.php?f=116&t=48500)
* [Video Bubbles Creator](http://wololo.net/talk/viewtopic.php?f=116&t=48581)
* [VitaGraphs](https://vitadb.rinnegatamante.it/#/info/363)
* [SwitchView UI](https://vitadb.rinnegatamante.it/#/info/338)

##### Emulators
* [MicroCHIP](http://wololo.net/talk/viewtopic.php?f=116&t=48620)

##### Engines & Interpreters
* [RayCast3D Engine](http://wololo.net/talk/viewtopic.php?f=116&t=46379)
* [March22](http://wololo.net/talk/viewtopic.php?f=116&t=47068)

# Compiling the source

To correctly compile lpp-vita you'll need **vitasdk** installed and correctly set in your PATH environment variable. You'll also need the following libraries: **zlib**, **libmpg123**, **libogg**, **libvorbis**, **libsndfile**, **vita2d**, 
**libftpvita**, **libpng**, **libjpeg-turbo**, **freetype**, **libspeexdsp**, **libopus**, **lua-jit**, **libdl**. You can install most of them with **vita-portlibs** and [EasyRPG Vita toolchain](https://ci.easyrpg.org/view/Toolchains/job/toolchain-vita/).<br><br>
Lua Player Plus Vita supports different flags to enables some features:<br>
**-DSKIP_ERROR_HANDLING** disables error handling for faster code execution.<br>
**-DPARANOID** enables extra internal error handling.<br>

# Credits

* vitasdk contributors.
* **xerpi** for vita2d and debug FTP code.
* **gnmmarechal** for testing the interpreter.
* **hyln9** for vita-luajit.
* **frangarcj** for the help during 3D rendering feature addition.
* **TheFloW** for some snippets i used and some ideas i borrowed.
* **Misledz** for the Lua Player Plus logo.
* **Arkanite** for providing a sample for sceAvPlayer used to implement the Video module.
* **EasyRPG Team** for the Audio Decoder used for Sound module.
* **lecram** for gifdec usd for animated GIFs support.
