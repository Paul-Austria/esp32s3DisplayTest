# Add source files from PixelFormat folder
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/BlendMode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlendFunctions.cpp

)


if(USE_NEON)
message("Blend Neon used")
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform/arm_neon/BlendFunctions.cpp
)
else()

set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform/generic/BlendFunctions.cpp

)

endif()


set(SOURCES ${SOURCES} PARENT_SCOPE)
