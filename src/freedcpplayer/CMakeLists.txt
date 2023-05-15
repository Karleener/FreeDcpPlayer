cmake_minimum_required(VERSION 3.7.0)

project(Free_DCP_Player VERSION 0.6.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)

# libsdl2-ttf-dev libsdl2-ttf-2.0-0
find_package(SDL_ttf REQUIRED)

# libwxgtk3.0-gtk3-dev
find_package(wxWidgets REQUIRED gl core base COMPONENTS net)

include(${wxWidgets_USE_FILE})

# libnvjpeg-dev-12-0
find_package(CUDAToolkit REQUIRED)

# libnvjpeg2k0-dev-cuda-12
find_library(NVJPEG2K_LIB
     NAMES nvjpeg2k
     PATHS ${NVJPEG2K_PATH}/lib64/${CUDAToolkit_VERSION_MAJOR}
     ${NVJPEG2K_PATH}/lib/${CUDAToolkit_VERSION_MAJOR})

   
include_directories(
  ${NVJPEG2K_PATH}/include
  SYSTEM ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
  ${SDL2_INCLUDE_DIRS}
  ${SDL_TTF_INCLUDE_DIRS}
  ${wxWidgets_INCLUDE_DIRS}
)

add_executable(freedcpplayer
  Cudaj2k/CDcpParse.cpp
  Cudaj2k/freedcpplayer.cpp
  Cudaj2k/Run.cpp
  Cudaj2k/RunDcpPlayer_wx.cpp
  Cudaj2k/CPlayer.cpp
  Cudaj2k/pugixml.cpp
  Cudaj2k/RunDcpDlg.cpp
)

if(UNIX)
  set(FILESYS -lstdc++fs)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# https://github.com/cinecert/asdcplib

target_link_libraries(freedcpplayer
  ${wxWidgets_LIBRARIES}
  nvjpeg2k
  crypto
  ssl
  pthread
  SDL2
  SDL2_ttf
  kumu
  asdcp
  ${NVJPEG2K_LIB} CUDA::cudart ${FILESYS}
  )

install(TARGETS freedcpplayer)
