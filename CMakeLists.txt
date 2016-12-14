CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Example)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

#FFMPEG is assumed to be in /usr/local/*
INCLUDE_DIRECTORIES(/usr/local/include/)
link_directories(/usr/local/lib/)

#set(my_SOURCES ${PROJECT_SOURCE_DIR}/libao_ffmpeg.c
#)

add_compile_options(-D__STDC_CONSTANT_MACROS)

add_executable(Decode_Audio2 
	       ring_buffer.cpp
               demuxing_decoding_filtering.cpp)


target_link_libraries(Decode_Audio2
		      avutil
		      avformat
		      avcodec
		      swresample
		      swscale
		      ao
		      m
		      pthread
)

   



