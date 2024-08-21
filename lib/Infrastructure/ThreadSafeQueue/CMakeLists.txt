# Add a cmake binary taget (in this case, a library)
add_library(ThreadSafeQueue INTERFACE)
target_sources(ThreadSafeQueue INTERFACE ThreadSafeQueue.hpp)

# Link library to a binary target
target_link_libraries(ThreadSafeQueue INTERFACE Logger)