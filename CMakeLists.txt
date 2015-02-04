cmake_minimum_required(VERSION 2.8.0)
project(yuvit)

set(PROJECT_VERSION 0.1.1)
set(PROJECT_URL "https://github.com/stunpix/yuvit")
set(PROJECT_AUTHOR "Alexander Shashkevich <stunpix@gmail.com>")
set(PROJECT_LICENSE "LGPLv3")


if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

find_path(FreeImageIncludeDir NAMES FreeImage.h)
find_library(FreeImageLib freeimage)

if(FreeImageIncludeDir MATCHES "NOTFOUND" OR FreeImageLib MATCHES "NOTFOUND")
	message(FATAL_ERROR "NOT FOUND: FreeImage.
	Not installed? Try install:
	Debian: sudo apt-get install libfreeimage-dev
	RedHat: sudo yum install freeimage-devel
	OS X: brew install freeimage")
else()
	if(CMAKE_HOST_APPLE)
		# On OS X freeimage installed under /usr/local and
		# clang doesn't see it, so we must set paths explicitly
		get_filename_component(FreeImageLibDir ${FreeImageLib} DIRECTORY)
		include_directories(${FreeImageIncludeDir})
		link_directories(${FreeImageLibDir})
	endif()
	message(STATUS "FreeImage: FOUND")
endif()

include_directories(${PROJECT_BINARY_DIR})
configure_file(config.h.in config.h)

add_subdirectory(src)

include(CPackOptions.txt)