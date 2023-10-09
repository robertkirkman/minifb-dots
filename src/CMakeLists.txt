add_executable(minifbproject main.c)

# Link MiniFB
target_link_libraries(${PROJECT_NAME}
    minifb
)