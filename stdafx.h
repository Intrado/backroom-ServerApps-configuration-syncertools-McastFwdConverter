// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: reference additional headers your program requires here
#include <sstream>
template <typename T>
std::string ToStr(T Number)
{
  std::ostringstream ss;
  ss << Number;
  return ss.str();
};

#include "windows.h"
#include <vector>
inline std::wstring ToWStr(std::string s)
{
  std::vector<wchar_t> s1(s.size() + 1, 0);
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, s.c_str(), (int)s.size() + 1, &s1[0], (int)s.size() + 1);
  return &s1[0];
}
