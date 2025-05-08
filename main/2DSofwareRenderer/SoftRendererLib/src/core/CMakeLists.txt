# Add source files from core folder
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderContext2D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RendererBase.cpp

)

add_subdirectory(Renderers)

set(SOURCES ${SOURCES} PARENT_SCOPE)

