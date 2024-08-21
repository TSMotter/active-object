find_package(Boost 1.74.0 REQUIRED)

# Add a cmake binary taget (in this case, a library)
add_library(IEvent INTERFACE)
target_sources(IEvent INTERFACE IEvent.hpp)

# Make the directory known
target_include_directories(IEvent INTERFACE ${Boost_INCLUDE_DIR})
# Link library to a binary target
target_link_libraries(IEvent INTERFACE ${Boost_LIBRARIES})