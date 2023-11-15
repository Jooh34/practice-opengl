# Copyright (C) 2018 Tomasz Gałaj

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(OpenGLExample)

# Add .lib files
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/rendering/*.c
	${CMAKE_SOURCE_DIR}/src/rendering/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/rendering/*.h
	${CMAKE_SOURCE_DIR}/src/rendering/*.hpp)

# Configure assets header file
configure_file(src/helpers/RootDir.h.in src/helpers/RootDir.h)
include_directories(${CMAKE_BINARY_DIR}/src)
	
# Define the executable
add_library(COMMON ${HEADER_FILES} ${SOURCE_FILES})

# executables for main entry, and chapter entries
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)
add_executable(ch02 ${CMAKE_SOURCE_DIR}/src/ch02.cpp)
add_executable(ch02_answer ${CMAKE_SOURCE_DIR}/src/ch02_answer.cpp)
add_executable(ch02_01_answer ${CMAKE_SOURCE_DIR}/src/ch02_01_answer.cpp)
add_executable(ch02_02_answer ${CMAKE_SOURCE_DIR}/src/ch02_02_answer.cpp)
add_executable(ch02_03_answer ${CMAKE_SOURCE_DIR}/src/ch02_03_answer.cpp)
add_executable(ch03 ${CMAKE_SOURCE_DIR}/src/ch03.cpp)
add_executable(ch03_01_answer ${CMAKE_SOURCE_DIR}/src/ch03_01_answer.cpp)
add_executable(ch03_02_answer ${CMAKE_SOURCE_DIR}/src/ch03_02_answer.cpp)
add_executable(ch04 ${CMAKE_SOURCE_DIR}/src/ch04.cpp)
add_executable(ch04_01_answer ${CMAKE_SOURCE_DIR}/src/ch04_01_answer.cpp)


# We need a CMAKE_DIR with some code to find external dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

#######################################
# LOOK for the packages that we need! #
#######################################

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# GLM
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# GLFW
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# ASSIMP
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

# STB_IMAGE
add_library(STB_IMAGE "thirdparty/stb_image.cpp")

# GLAD
add_library(GLAD "thirdparty/glad.c")

# Put all libraries into a variable
set(LIBS ${GLFW3_LIBRARY} ${OPENGL_LIBRARY} GLAD ${CMAKE_DL_LIBS} ${ASSIMP_LIBRARY} STB_IMAGE)

# Define the include DIRs
include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/include"
	"${CMAKE_SOURCE_DIR}/include/glad"
	"${CMAKE_SOURCE_DIR}/include/glm"
)

# Define the link libraries
target_link_libraries(${PROJECT_NAME} COMMON ${LIBS})
target_link_libraries(ch02 COMMON ${LIBS})
target_link_libraries(ch02_answer COMMON ${LIBS})
target_link_libraries(ch02_01_answer COMMON ${LIBS})
target_link_libraries(ch02_02_answer COMMON ${LIBS})
target_link_libraries(ch02_03_answer COMMON ${LIBS})
target_link_libraries(ch03 COMMON ${LIBS})
target_link_libraries(ch03_01_answer COMMON ${LIBS})
target_link_libraries(ch03_02_answer COMMON ${LIBS})
target_link_libraries(ch04 COMMON ${LIBS})
target_link_libraries(ch04_01_answer COMMON ${LIBS})

# Create virtual folders to make it look nicer in VS
if(MSVC_IDE)
	# Macro to preserve source files hierarchy in the IDE
	macro(GroupSources curdir)
		file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

		foreach(child ${children})
			if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
				GroupSources(${curdir}/${child})
			else()
				string(REPLACE "/" "\\" groupname ${curdir})
				string(REPLACE "src" "Sources" groupname ${groupname})
				source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			endif()
		endforeach()
	endmacro()

	# Run macro
	GroupSources(src)
endif()

# Copy dlls
if(WIN32)
	add_custom_command(TARGET COMMON POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/dlls"
		$<TARGET_FILE_DIR:COMMON>)
endif()
