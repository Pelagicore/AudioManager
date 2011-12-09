cmake_minimum_required(VERSION 2.6)

PROJECT(PluginControlInterface)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xlinker -export-dynamic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "-g -pipe -g -Wall -W -D_REENTRANT")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(LIBRARY_OUTPUT_PATH ../plugins/control)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/PluginControl)
set(AUDIO_INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/../includes)
set(DOCUMENT $ENV{gendoc})
FIND_PACKAGE(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
 	${CMAKE_CURRENT_BINARY_DIR}
 	${AUDIO_INCLUDES_FOLDER}
)

# all source files go here
SET(PLUGIN_STANDARD_HOOKS_SRCS_CXX
    ControlSender.cpp
)

add_library(PluginControlInterface SHARED ${PLUGIN_STANDARD_HOOKS_SRCS_CXX})

TARGET_LINK_LIBRARIES(PluginControlInterface ${DLT_LIBRARIES} )

#add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
if(DOCUMENT)
  	configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (Docs ALL 
  	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  	SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif(DOCUMENT)
endif(DOXYGEN_FOUND)
