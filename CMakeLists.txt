CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/DEBIAN/usr/bin)
PROJECT(Hellqualizer)

set(GUI OFF)
set(RENDER_WITH_ALSA ON)
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

if (UNIX)
#FFMPEG is assumed to be in /usr/local/*
INCLUDE_DIRECTORIES(/usr/local/include/)
link_directories(/usr/local/lib/)
INCLUDE_DIRECTORIES(/usr/include/)
link_directories(/usr/lib/)
elseif (WIN32)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/extra_lib/include/)
link_directories(${PROJECT_SOURCE_DIR}/extra_lib/lib/)
endif ()



set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -D__STDC_CONSTANT_MACROS ")

set(my_SOURCES    
    ${PROJECT_SOURCE_DIR}/core/src/ring_buffer.cpp
    ${PROJECT_SOURCE_DIR}/core/src/processing.cpp
    ${PROJECT_SOURCE_DIR}/core/src/demuxing_decoding.cpp
)

if (UNIX)
if(RENDER_WITH_ALSA)

  set(my_SOURCES ${my_SOURCES}
      ${PROJECT_SOURCE_DIR}/core/src/rendering_alsa.cpp
      ${PROJECT_SOURCE_DIR}/core/src/record.cpp
  )

else()

    set(my_SOURCES ${my_SOURCES}
      ${PROJECT_SOURCE_DIR}/core/src/rendering_ao.cpp)

endif()
elseif (WIN32)
    set(my_SOURCES ${my_SOURCES}
      ${PROJECT_SOURCE_DIR}/core/src/rendering_win32.cpp)
endif()


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
    #swresample
    #swscale
    m
    pthread
)

if(GUI)
    set(dependancies ${dependancies}
	${OPENGL_LIBRARY}
	glfw
	GLEW)
endif()

if (UNIX)
if(RENDER_WITH_ALSA)

    set(dependancies ${dependancies}
                      asound)

else()

    set(dependancies ${dependancies}
                      ao)

endif()
elseif (WIN32) 
    set(dependancies ${dependancies}
                      Winmm)
endif()




target_link_libraries(Hellqualizer
    ${dependancies}
)

install (TARGETS Hellqualizer DESTINATION ${PROJECT_SOURCE_DIR}/usr/bin/)

   



