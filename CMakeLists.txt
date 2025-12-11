# CMakeLists.txt file for JsonEditor

cmake_minimum_required(VERSION 3.27)

project(JsonEditor)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(ThirdParty)

set(ACHIEVEMENTS_TOOL_SOURCES
	Source/Application.cpp
	Source/Application.h
	Source/Frame.cpp
	Source/Frame.h
	Source/JsonDataViewModel.cpp
	Source/JsonDataViewModel.h
)

source_group("Source" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ACHIEVEMENTS_TOOL_SOURCES})

add_executable(JsonEditor WIN32
	${ACHIEVEMENTS_TOOL_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/wx_dpi_aware_pmv2.manifest
)

target_compile_definitions(JsonEditor PUBLIC
	__WXMSW__
	_UNICODE
)

target_compile_definitions(JsonEditor PRIVATE
	WIN32_LEAN_AND_MEAN
	NOMINMAX
	_USE_MATH_DEFINES
)

target_link_libraries(JsonEditor
	ParseParty
	wx::core
	wx::base
	wx::aui
)

target_include_directories(JsonEditor PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ParseParty/ParseLibrary/Source
	${CMAKE_CURRENT_SOURCE_DIR}/Source
)