[![Build Status](https://travis-ci.org/DerouineauNicolas/Hellqualizer.svg?branch=master)](https://travis-ci.org/DerouineauNicolas/Hellqualizer)

Intro
-------------------

This program is decoding, filtering and playing encoded audio streams in real time.

Compilation
-------------------

To compile it, you need the following depandancies:

ffmpeg:

	git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg

	git checkout remotes/origin/release/2.0

	./configure --enable-shared --disable-static

	make && sudo make install

alsa:

	sudo apt-get install libasound2-dev

Clone the Hellqualizer project in $PROJECT_DIR, then:

	mkdir $PROJECT_DIR/build
	cd $PROJECT_DIR/build
	cmake ..
	make

Execution
-------------------
For encoded streams (If a video is provided, only the audio track will be processed)
 
	./Hellqualizer ~/musique/some_music.mp4

EQ can be controlled through the keyboard at runtime:

a - q : Increase/Decrease gain between 0 - 2000 Hz

z - s : Increase/Decrease gain between 2000 - 4000 Hz

e - d : Increase/Decrease gain between 4000 - 6000 Hz

r - f : Increase/Decrease gain between 6000 - 10000 Hz

t - g : Increase/Decrease gain between 10000 - 22000 Hz

space: Play/Pause 

x : Enable/Disable EQ

Package
-------------------
Debian packages are available [here](http://the.ndero.ovh/build/Hellqualizer/)









