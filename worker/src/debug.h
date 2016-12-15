#pragma once

#define DEBUG

#ifdef DEBUG
#define DBG_PRINT(str) std::cout << str << std::endl;
#else
#define DBG_PRINT(str)
#endif
