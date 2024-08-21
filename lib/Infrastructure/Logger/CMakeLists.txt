# Add a cmake binary taget (in this case, a library)
add_library(Logger INTERFACE)
target_sources(Logger INTERFACE Logger.hpp)