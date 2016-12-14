#pragma once
//////////////////////////////////////////////////////////////////////////////
// Copyright (c) Joseph Mariadassou
// theSundayProgrammer@gmail.com
// Distributed under the Boost Software License, Version 1.0.
// 
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__
typedef int errno_t;
errno_t fopen_s(FILE** fp, const char* name, const char* mode);
#endif
#include <spdlog/spdlog.h>
extern std::shared_ptr<spdlog::logger> console;
