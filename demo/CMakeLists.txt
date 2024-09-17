# Add a cmake binary taget (in this case, an executable)
add_executable(main main.cpp)

# Make the directory known
target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/lib/Infrastructure)

# Link library to a binary target
target_link_libraries(main PUBLIC IState)
target_link_libraries(main PUBLIC StateManager)
target_link_libraries(main PUBLIC ThreadSafeQueue)