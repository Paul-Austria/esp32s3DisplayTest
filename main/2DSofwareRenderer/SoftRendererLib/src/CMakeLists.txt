# Add subdirectories for subfolders
add_subdirectory(util)
add_subdirectory(data)
add_subdirectory(core)

# Add source files from the src directory itself (if there are any)
set(SOURCES
    ${SOURCES}
)
set(SOURCES ${SOURCES} PARENT_SCOPE)
