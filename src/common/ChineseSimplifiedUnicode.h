//
// Created by DELL on 2022/3/29.
//

#pragma once
#include <imgui.h>

namespace ChineseSimplifiedUnicode {

    extern wchar_t commonChars[3500]; // 3500 个常用汉字
    extern const char* specialChars; // 指定一些特殊的汉字

    const ImWchar*  getImguiCharRange();
};

