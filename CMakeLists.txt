CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Example)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

#FFMPEG is assumed to be in /usr/local/*
INCLUDE_DIRECTORIES(/usr/local/include/)
link_directories(/usr/local/lib/)

#set(my_SOURCES ${PROJECT_SOURCE_DIR}/libao_ffmpeg.c
#)

add_executable(Decode_Audio2 demuxing_decoding_filtering.c)
#add_executable(Sample ao_example.c)



target_link_libraries(Decode_Audio2
		      avutil
		      avformat
		      avcodec
		      swresample
		      swscale
		      ao
		      m
)


if (BZIP2_FOUND)
  include_directories(${BZIP_INCLUDE_DIRS})
  target_link_libraries (Decode_Video ${BZIP2_LIBRARIES})
endif (BZIP2_FOUND)
   



