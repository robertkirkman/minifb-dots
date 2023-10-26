add_executable(minifbproject main.c)

target_include_directories(minifbproject PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Link MiniFB
target_link_libraries(${PROJECT_NAME}
    minifb
)