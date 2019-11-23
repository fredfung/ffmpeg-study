# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("ffmpeg-study")

# Include sub-projects.
include_directories("D:/SDK/include")

# lib directory
link_directories("D:/SDK/lib/win32")
link_libraries(avcodec)


add_subdirectory ("1-extractYUV420")