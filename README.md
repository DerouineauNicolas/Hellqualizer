Intro
-------------------

This program is decoding, filtering and playing encoded audio streams in real time.

Execution
-------------------
 
	./Hellqualizer ~/musique/some_music.mp4

EQ can be controlled through the keyboard at runtime:

a - q : Increase/Decrease gain between 0 - 2000 Hz

z - s : Increase/Decrease gain between 2000 - 4000 Hz

e - d : Increase/Decrease gain between 4000 - 6000 Hz

r - f : Increase/Decrease gain between 6000 - 10000 Hz

t - g : Increase/Decrease gain between 10000 - 22000 Hz

Package
-------------------
Debian packages are available [here](http://the.ndero.ovh/build/Hellqualizer/)


Compilation
-------------------

To compile it, you need cmake , ffmpeg-dev (>=V3.0.0) and libao-dev installed in /usr/local/

First, clone the project in $PROJECT_DIR, then:

 	mkdir $PROJECT_DIR/build
	cd $PROJECT_DIR/build
	cmake ..
	make






