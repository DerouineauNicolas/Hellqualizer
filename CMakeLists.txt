CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Example)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/core/inc)

#FFMPEG is assumed to be in /usr/local/*
INCLUDE_DIRECTORIES(/usr/local/include/)
link_directories(/usr/local/lib/)

set (CMAKE_CXX_FLAGS "-Wall -D__STDC_CONSTANT_MACROS ")

add_executable(Decode_Audio2
    main.cpp
    ${PROJECT_SOURCE_DIR}/core/src/rendering.cpp
    ${PROJECT_SOURCE_DIR}/core/src/ring_buffer.cpp
    ${PROJECT_SOURCE_DIR}/core/src/processing.cpp
    ${PROJECT_SOURCE_DIR}/core/src/demuxing_decoding.cpp
)


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

   



