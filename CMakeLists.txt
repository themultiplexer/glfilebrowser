cmake_minimum_required(VERSION 3.5)
project(glfilebrowser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
IF (WIN32)
find_package(unofficial-kissfft REQUIRED)
find_package(freetype REQUIRED)
ELSE()
find_package(kissfft REQUIRED)
find_package(Freetype REQUIRED)
ENDIF()
find_package(RtAudio REQUIRED)
find_package(PNG REQUIRED)


add_compile_definitions(_USE_MATH_DEFINES)
add_executable(glfilebrowser main.cpp)

include_directories( ${OpenCV_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} )

IF (WIN32)
target_link_libraries(glfilebrowser ${X11_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${OpenCV_LIBS} ${PNG_LIBRARIES} ${FREETYPE_LIBRARIES} glfw RtAudio::rtaudio unofficial::kissfft::kissfft)
ELSE()
target_link_libraries(glfilebrowser ${X11_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${OpenCV_LIBS} ${PNG_LIBRARIES} ${FREETYPE_LIBRARIES} m glfw RtAudio::rtaudio "/usr/lib/x86_64-linux-gnu/libkissfft-float.so.131.1.0")
ENDIF()
