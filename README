Intro
_________

This program is decoding, filtering and playing encoded audio streams in real time

Compilation
___________

To compile it, you need cmake , ffmpeg-dev (>=V3.0.0) and libao-dev installed in /usr/local/

First, clone the project in $PROJECT_DIR, then:

 	mkdir $PROJECT_DIR/build
	cd $PROJECT_DIR/build
	cmake ..
	make

Execution
___________
 
	./Decode_Audio2 ~/musique/some_music.mp4
This will decode and play the bitstream

	./Decode_Audio2 ~/musique/some_music.mp4 -f 1
This will decode, filter the sound with a LP filter 0-5000 Hz

	./Decode_Audio2 ~/musique/some_music.mp4 -f 2
This will decode, filter the sound with a BP filter 5000-10000 Hz

Please note that only stereo aac streams 44100 kHz/s are currently supported.

The filter coefficients are generated with scilab using the equiribble constrain.
Only floating point is used at the moment, but fixed point is already implemented.

