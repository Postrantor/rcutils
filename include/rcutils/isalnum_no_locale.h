// Copyright 2017 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file

#ifndef RCUTILS__ISALNUM_NO_LOCALE_H_
#define RCUTILS__ISALNUM_NO_LOCALE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 自定义 isalnum() 函数，不受区域设置影响 (Custom isalnum() which is not affected by locale)
 *
 * @param c 输入字符 (Input character)
 * @return true 如果输入字符是字母或数字 (If the input character is a letter or digit)
 * @return false 如果输入字符不是字母或数字 (If the input character is not a letter or digit)
 */
static inline bool rcutils_isalnum_no_locale(char c)
{
  // 如果在 '0', ..., '9' 之间，则返回 true (If in '0', ..., '9', then return true)
  if (c >= 0x30 /*0*/ && c <= 0x39 /*9*/) {
    return true;
  }
  // 如果在 'A', ..., 'Z' 之间，则返回 true (If in 'A', ..., 'Z', then return true)
  if (c >= 0x41 /*A*/ && c <= 0x5a /*Z*/) {
    return true;
  }
  // 如果在 'a', ..., 'z' 之间，则返回 true (If in 'a', ..., 'z', then return true)
  if (c >= 0x61 /*a*/ && c <= 0x7a /*z*/) {
    return true;
  }
  // 其他情况下，返回 false (In other cases, return false)
  return false;
}

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__ISALNUM_NO_LOCALE_H_
