#pragma once

#include <iostream>
#include <cassert>

#ifdef FECS_LOGGING
    #define FECS_LOG std::cout << "[FECS] [INFO] "
    #define FECS_LOG_WARN std::cout << "[FECS] [WARN] "
    #define FECS_LOG_ERR std::cout << "[FECS] [ERR] "
    #define FECS_NL '\n'
#else
    // Expand to nothing if logging is disabled
    #define FECS_LOG if (false) std::cout
    #define FECS_LOG_WARN if (false) std::cout
    #define FECS_LOG_ERR if (false) std::cout
    #define FECS_NL '\n'
#endif

#define FECS_ASSERT(expr) assert(expr)

#ifdef FECS_LOGGING
    #define FECS_ASSERT_M(expr, msg) if(!(expr)) { FECS_LOG_ERR << msg << FECS_NL; FECS_ASSERT(false); }
#else
    #define FECS_ASSERT_M(expr, msg) FECS_ASSERT(expr)
#endif