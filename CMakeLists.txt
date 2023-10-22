cmake_minimum_required(VERSION 3.20)
project(hotline)

project(hotline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" HOTLINE_STANDALONE)
option(HOTLINE_BUILD_EXAMPLES "Build hotline examples" ${HOTLINE_STANDALONE})

#glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/include)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw)

#imgui
# Add ImGui library
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)
add_library(IMGUI STATIC)

target_sources( IMGUI
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
                )

target_include_directories( IMGUI
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            PUBLIC ${SDL3_DIR}/include
                            )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/backends)

add_library(hotline STATIC)
target_sources( hotline
                PRIVATE
				src/IActionFrontend.h
				src/ActionManager.h
				src/ActionManager.cpp
				src/Action.h 
				src/ArgProvider.h
                src/ActionSet.h
                src/ActionSet.cpp
                src/search/FuzzyScorer.h
                src/search/FuzzyScorer.cpp
                src/Hotline.h
                src/Hotline.cpp
				src/ProviderWindow.h
				src/ProviderWindow.cpp
                )

find_package(OpenGL REQUIRED)
    target_link_libraries(IMGUI PUBLIC ${OPENGL_LIBRARIES})

if (HOTLINE_BUILD_EXAMPLES)
    ## Create main executable
	add_executable(hotline_example src/main.cpp)
	
	target_link_libraries(
	  hotline_example
	  PUBLIC
		hotline
		glfw
		IMGUI
	)
endif()


