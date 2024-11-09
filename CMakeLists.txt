cmake_minimum_required(VERSION 3.15)
project(3DRasterizer)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# SDL2 Paths
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/thirdparty/SDL2")
set(SDL2_INCLUDE_DIR "${SDL2_DIR}/include")
set(SDL2_LIB_DIR "${SDL2_DIR}/lib/x64")

# Add SDL2 include directory
include_directories(${SDL2_INCLUDE_DIR})

# Define the SDL2 library, SDL2main library, and DLL paths
if (WIN32)
    set(SDL2_LIBRARY "${SDL2_LIB_DIR}/SDL2.lib")        # SDL2 library
    set(SDL2MAIN_LIBRARY "${SDL2_LIB_DIR}/SDL2main.lib") # SDL2main library
    set(SDL2_DLL "${SDL2_LIB_DIR}/SDL2.dll")            # Path to SDL2.dll file
endif()

# Add executable and source files
# add_executable(3DRasterizer src/main.cpp src/app.cpp)

# Add the 'include' folder to the include search path
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all .cpp files in the 'src' directory
file(GLOB SOURCES "src/*.cpp")

# Ensure main.cpp is first in the list of sources
list(FIND SOURCES "src/main.cpp" main_index)
if(NOT main_index EQUAL -1)
    list(REPLACE SOURCES ${main_index} 0 "src/main.cpp")
endif()

# Use the collected files in add_executable
add_executable(3DRasterizer ${SOURCES})

# Link SDL2 and SDL2main to your project
target_link_libraries(3DRasterizer PRIVATE ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY})

# Specify that we want to build a console application
if (WIN32)
    set_target_properties(3DRasterizer PROPERTIES 
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

# Post-build command to copy SDL2.dll to the output directory
if (WIN32)
    add_custom_command(TARGET 3DRasterizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DLL}"            # Source file
        $<TARGET_FILE_DIR:3DRasterizer> # Destination directory (where the executable is)
    )
endif()

# Copy the 'res' folder to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/res/ DESTINATION ${CMAKE_BINARY_DIR}/res)