# Top-level CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(OpenGLWrapper)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


if(WIN32)
    # Add subdirectories for external libraries
    add_subdirectory(external/glfw)
    add_definitions(-DGLEW_STATIC)
    add_subdirectory(external/glew)
    find_package(OpenGL REQUIRED)
        if(OpenGL_FOUND)
            message(STATUS "Found OpenGL: ${OPENGL_gl_LIBRARY}")
        else()
            message(FATAL_ERROR "OpenGL not found")
        endif()
endif(WIN32)

# Add subdirectory for your SoftRendererLib
add_subdirectory(SoftRendererLib)

if(WIN32)

# Main OpenGL wrapper executable
add_executable(OpenGLWrapper OpenGLWrapper/main.cpp)

# Include directories for GLFW and SoftRendererLib
target_include_directories(OpenGLWrapper PRIVATE
    external/glfw/include
    ${CMAKE_SOURCE_DIR}/SoftRendererLib
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)



# Link libraries for OpenGL wrapper
target_link_libraries(OpenGLWrapper SoftRendererLib libglew_static glfw ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS})

# Copy data for the OpenGLWrapper target
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLWrapper/data
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
endif(WIN32)


if(UNIX)
set(USE_NEON ON CACHE BOOL "use neon")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flax-vector-conversions")

if(USE_NEON)
message("arm NEON is used")
endif()
# Second target for testing SoftRendererLib without OpenGL
add_executable(SoftRendererLinuxDemo SoftRendererLinuxDemo/main.cpp)

# Include directories for the SoftRendererTest target
target_include_directories(SoftRendererLinuxDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/SoftRendererLib
    /opt/st/stm32mp1/5.0.3-openstlinux-6.6-yocto-scarthgap-mpu-v24.11.06/sysroots/cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi/usr/include/drm
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Link libraries for SoftRendererTest (no OpenGL)
target_link_libraries(SoftRendererLinuxDemo SoftRendererLib         ${CMAKE_SYSROOT}/usr/lib/libdrm.so
)

endif(UNIX)


# Set build-specific options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for Debug build")
    # Add Debug specific options
    if(WIN32)
    target_compile_definitions(OpenGLWrapper PRIVATE DEBUG)
    endif(WIN32)
    if(UNIX)
    target_compile_definitions(SoftRendererLinuxDemo PRIVATE DEBUG)
    target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SYSROOT}/usr/lib/libdrm.so
)
    endif(UNIX)
else()
    message(STATUS "Configuring for Release build")
    # Add Release specific options
    if(WIN32)
    target_compile_definitions(OpenGLWrapper PRIVATE NDEBUG)
    endif(WIN32)
    if(UNIX)
    target_compile_definitions(SoftRendererLinuxDemo PRIVATE NDEBUG)
    endif(UNIX)
endif()

# Optionally, set default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "Build type" FORCE)
endif()


set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
