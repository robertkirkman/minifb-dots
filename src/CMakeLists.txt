add_executable(minifbproject main.c)

# Link MiniFB to your project:
target_link_libraries(${PROJECT_NAME}
    minifb
)