#ifndef CPPCORO_LOGGING_HPP_INCLUDED
#define CPPCORO_LOGGING_HPP_INCLUDED

#include <iostream>
#include <sstream>


#define DEBUG       0
#define INFO        1
#define WARNING     2
#define ERROR       3
#define FATAL       4


static int g_log_level = DEBUG;

void set_global_log_level(int level) noexcept
{
    g_log_level = level;
}

class LogStream: public std::stringstream
{
public:
    LogStream(int level)
        : m_level(level) {}
    
    virtual ~LogStream()
    {
        if(m_level >= g_log_level)
        {   std::string s = this->str();
            std::cout << s << std::endl;
        }
    }

private:
    int m_level;
};


#define LOG(x) LogStream(x)
#define DLOG LOG(DEBUG)

#endif
