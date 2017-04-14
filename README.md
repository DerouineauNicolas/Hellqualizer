Intro
-------------------

This program is decoding, filtering and playing encoded audio streams in real time.
The current EQ is based on 5 FIR filters. Inband individual gain can be selected.
The gain for each frequency range is the following

G0: Gain for 0-2000 Hz (Should be between 0 and 1.0)

G1: Gain for 2000-4000 Hz (Should be between 0 and 1.0)

G2: Gain for 4000-6000 Hz (Should be between 0 and 1.0)

G3: Gain for 6000-10000 Hz (Should be between 0 and 1.0)

G4: Gain for 10000-22000 Hz (Should be between 0 and 1.0)

The filter coefficients are generated with scilab.
The script to generate filter is available in the script directory. Only floating point is used at the moment, but fixed point is already implemented.

Execution
-------------------
 
	./Hellqualizer ~/musique/some_music.mp4
This will decode and play the bitstream

	./Hellqualizer ~/musique/some_music.mp4 -f G0:G1:G2:G3:G4

Gain can be controlled through the keyboard at runtime:

a - q : Increase/Decrease G0

z - s : Increase/Decrease G1

e - d : Increase/Decrease G2

r - f : Increase/Decrease G3

t - g : Increase/Decrease G4

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






