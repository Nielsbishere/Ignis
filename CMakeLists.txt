cmake_minimum_required(VERSION 3.13.0)
project(ignis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)

include_directories(include)
include_directories(core2/include)

file(GLOB_RECURSE ignisHpp
	"include/*.hpp"
)

file(GLOB_RECURSE ignisCpp
	"src/*.cpp"
)

add_subdirectory(core2)

set(graphicApi opengl CACHE STRING "Graphics API")

set(graphicsApis vulkan opengl directx)
set_property(CACHE graphicApi PROPERTY STRINGS ${graphicsApis})

# Setup graphics api

include_directories(api/${graphicsApi}/include)
	
file(GLOB_RECURSE apiHpp "api/${graphicsApi}/include/*.hpp")
file(GLOB_RECURSE apiCpp "api/${graphicsApi}/src/*.cpp")

# Setup platforms

include_directories(api/${graphicsApi}/platform/${platform}/include)
	
file(GLOB_RECURSE platformHpp "api/${graphicsApi}/platform/${platform}/include/*.hpp")
file(GLOB_RECURSE platformCpp "api/${graphicsApi}/platform/${platform}/src/*.cpp")

# Link library

add_library(
	ignis
	STATIC
	${ignisHpp}
	${ignisCpp}
	${platformHpp}
	${platformCpp}
	${apiHpp}
	${apiCpp}
	CMakeLists.txt
)

target_link_libraries(ignis ocore)

source_group("Header Files" FILES ${ignisHpp})
source_group("Source Files" FILES ${ignisCpp})
source_group("Platform Header Files" FILES ${platformHpp})
source_group("Platform Source Files" FILES ${platformCpp})
source_group("Graphics API Header Files" FILES ${apiHpp})
source_group("Graphics API Source Files" FILES ${apiCpp})

if(MSVC)
    target_compile_options(ignis PRIVATE /W4 /WX /MD /MP)
else()
    target_compile_options(ignis PRIVATE -Wall -Wextra -pedantic -Werror)
endif()