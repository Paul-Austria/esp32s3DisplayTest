add_subdirectory(PixelFormat)
add_subdirectory(BlendMode)

set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Color.cpp

)
set(SOURCES ${SOURCES} PARENT_SCOPE)
