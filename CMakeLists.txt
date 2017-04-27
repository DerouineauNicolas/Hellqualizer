CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/DEBIAN/usr/bin)
PROJECT(Hellqualizer)

set(GUI OFF)
set(PROFILING OFF)

if(GUI)
    message("Build with gui")
    find_package(OpenGL REQUIRED)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -DHQ_GUI=1")
endif()

if(PROFILING)
    message("Build with profiling")
    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utils/)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -DHQ_PROFILING=1")
endif()



INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/core/inc)


#FFMPEG is assumed to be in /usr/local/*
INCLUDE_DIRECTORIES(/usr/local/include/)
link_directories(/usr/local/lib/)
INCLUDE_DIRECTORIES(/usr/include/)
link_directories(/usr/lib/)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -D__STDC_CONSTANT_MACROS ")

set(my_SOURCES     ${PROJECT_SOURCE_DIR}/core/src/rendering.cpp
    ${PROJECT_SOURCE_DIR}/core/src/ring_buffer.cpp
    ${PROJECT_SOURCE_DIR}/core/src/processing.cpp
    ${PROJECT_SOURCE_DIR}/core/src/demuxing_decoding.cpp
)

if(GUI)

  set(my_SOURCES ${my_SOURCES}
      ${PROJECT_SOURCE_DIR}/gui/src/gui.cpp)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gui/inc)

else()

  set(my_SOURCES ${my_SOURCES}
      ${PROJECT_SOURCE_DIR}/core/src/controler.cpp)

endif()

add_executable(Hellqualizer
    main.cpp
    ${my_SOURCES}
)

set(dependancies   		      
    avutil
    avformat
    avcodec
    swresample
    swscale
    ao
    m
    pthread
)

if(GUI)
    set(dependancies ${dependancies}
	${OPENGL_LIBRARY}
	glfw
	GLEW)
endif()



target_link_libraries(Hellqualizer
    ${dependancies}
)

install (TARGETS Hellqualizer DESTINATION ${PROJECT_SOURCE_DIR}/usr/bin/)

   



