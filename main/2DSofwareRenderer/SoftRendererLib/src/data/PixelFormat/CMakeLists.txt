# Add source files from PixelFormat folder
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/PixelConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PixelFormatInfo.cpp

)


if(USE_NEON)
message("PixelConverter Neon used")
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform/arm_neon/PixelConverter.cpp

)
else()
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform/generic/PixelConverter.cpp
)
endif()


set(SOURCES ${SOURCES} PARENT_SCOPE)
