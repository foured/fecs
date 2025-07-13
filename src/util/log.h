#pragma once

#include <iostream>
#include <cassert>

#define FECS_LOG std::cout << "[INFO] "
#define FECS_LOG_WARN std::cout << "[WARN] "
#define FECS_LOG_ERR std::cout << "[ERR] "
#define FECS_NL '\n'

#define FECS_ASSERT(expr) assert(expr)

#define FECS_ASSERT_M(expr, msg) if(!expr) { FECS_LOG_ERR << msg << FECS_NL; FECS_ASSERT(false); }