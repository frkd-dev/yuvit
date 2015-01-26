cmake_minimum_required(VERSION 3.0)
project(yuvit VERSION 0.1.1)

set(PROJECT_URL "https://github.com/stunpix/yuvit")
set(PROJECT_AUTHOR "Alexander Shashkevich <stunpix@gmail.com>")
set(PROJECT_LICENSE "LGPLv3")


if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

find_path(FreeImageInclude NAMES FreeImage.h)

if(FreeImageInclude MATCHES "NOTFOUND")
	message(FATAL_ERROR "Missing FreeImage. Perhaps not installed? Try install 'libfreeimage-dev'.")
else()
	message(STATUS "FOUND: FreeImage")
endif()

include_directories(${PROJECT_BINARY_DIR})
configure_file(config.h.in config.h)

add_subdirectory(src)

include(CPackOptions.txt)