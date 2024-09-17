# Add a cmake binary taget (in this case, a library)
add_library(IState INTERFACE)
target_sources(IState INTERFACE IState.hpp)

# Make the directory known
target_include_directories(IState INTERFACE ${CMAKE_SOURCE_DIR}/lib/Infrastructure)

# Link library to a binary target
target_link_libraries(IState INTERFACE IEvent)