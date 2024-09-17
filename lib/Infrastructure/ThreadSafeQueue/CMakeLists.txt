# Add a cmake binary taget (in this case, a library)
add_library(ThreadSafeQueue INTERFACE)
target_sources(ThreadSafeQueue INTERFACE ThreadSafeQueue.hpp)

# Make the directory known
target_include_directories(ThreadSafeQueue INTERFACE ${CMAKE_SOURCE_DIR}/lib/Infrastructure)
# Link library to a binary target
target_link_libraries(ThreadSafeQueue INTERFACE Logger)