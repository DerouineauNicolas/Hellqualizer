Intro
-------------------

This program is decoding, filtering and playing encoded audio streams in real time.
The current EQ is based on 5 FIR filters (63 taps). Inband individual gain can be selected.
This implementation differs from the tradional 2nd order biquad filtering. The complexity and
the audio quality of this eq needs to be profiled.
The current bands are 0:2000:4000:6000:10000:22000


Compilation
-------------------

To compile it, you need cmake , ffmpeg-dev (>=V3.0.0) and libao-dev installed in /usr/local/

First, clone the project in $PROJECT_DIR, then:

 	mkdir $PROJECT_DIR/build
	cd $PROJECT_DIR/build
	cmake ..
	make

Execution
-------------------
 
	./Decode_Audio2 ~/musique/some_music.mp4
This will decode and play the bitstream

	./Decode_Audio2 ~/musique/some_music.mp4 -f 0.1:0.1:0.1:0.1:0.1

where -f G0:G1:G2:G3:G4 with

G0: Gain for 0-2000 Hz
G1: Gain for 2000-4000 Hz
G2: Gain for 4000-6000 Hz
G3: Gain for 6000-10000 Hz
G4: Gain for 10000-22000 Hz

The filter coefficients are generated with scilab using the equiribble constrain.
Only floating point is used at the moment, but fixed point is already implemented.

