#ifndef __LOGGER_H_
#define __LOGGER_H_

#include <iostream>
#include <sstream>

// Dummy stream class that discards everything
class NullStream : public std::ostream
{
   public:
    NullStream() : std::ostream(nullptr)
    {
    }
};

NullStream nullStream;

constexpr const unsigned int LEVEL_UNKNOWN = 0;
constexpr const unsigned int LEVEL_DEBUG   = 1;
constexpr const unsigned int LEVEL_INFO    = 2;
constexpr const unsigned int LEVEL_WARNING = 3;
constexpr const unsigned int LEVEL_ERROR   = 4;

#if not defined(SYSTEM_LOG_LEVEL)
#define SYSTEM_LOG_LEVEL LEVEL_INFO
#endif

#define LOG(mod, level) level >= SYSTEM_LOG_LEVEL ? std::cout << "[" << mod << "] " : nullStream

#endif