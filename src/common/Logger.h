//
// Created by DELL on 2022/3/4.
//
#pragma once

#include "spdlog/spdlog.h"

// general information
#define LOGI(...) spdlog::info("{}, <{}:{}>" , spdlog::fmt_lib::format(__VA_ARGS__), __FILE__, __LINE__)
// error
#define LOGE(...) spdlog::error("{}, <{}:{}>", spdlog::fmt_lib::format(__VA_ARGS__), __FILE__, __LINE__)
// warning
#define LOGW(...) spdlog::warn("{}, <{}:{}>", spdlog::fmt_lib::format(__VA_ARGS__), __FILE__, __LINE__)
