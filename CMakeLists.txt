CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/DEBIAN/usr/bin)
PROJECT(Hellqualizer)
find_package(OpenGL REQUIRED)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/core/inc)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gui/inc)

#FFMPEG is assumed to be in /usr/local/*
INCLUDE_DIRECTORIES(/usr/local/include/)
link_directories(/usr/local/lib/)
INCLUDE_DIRECTORIES(/usr/include/)
link_directories(/usr/lib/)

set (CMAKE_CXX_FLAGS "-Wall -D__STDC_CONSTANT_MACROS ")


add_executable(Hellqualizer
    main.cpp
    ${PROJECT_SOURCE_DIR}/core/src/rendering.cpp
    ${PROJECT_SOURCE_DIR}/core/src/ring_buffer.cpp
    ${PROJECT_SOURCE_DIR}/core/src/processing.cpp
    ${PROJECT_SOURCE_DIR}/core/src/demuxing_decoding.cpp
    ${PROJECT_SOURCE_DIR}/core/src/controler.cpp
    ${PROJECT_SOURCE_DIR}/gui/src/gui.cpp
)


target_link_libraries(Hellqualizer
		      avutil
		      avformat
		      avcodec
		      swresample
		      swscale
		      ao
		      m
		      pthread
		      ${OPENGL_LIBRARY}
		      glfw
		      GLEW
)

install (TARGETS Hellqualizer DESTINATION ${PROJECT_SOURCE_DIR}/usr/bin/)

   



