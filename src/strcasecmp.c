// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <string.h>

#include "rcutils/strcasecmp.h"

/**
 * @brief 比较两个字符串，忽略大小写 (Compare two strings, case-insensitive)
 *
 * @param[in] s1 第一个字符串 (The first string)
 * @param[in] s2 第二个字符串 (The second string)
 * @param[out] value 指向比较结果的指针，0 表示相等，非 0 表示不相等 (Pointer to the comparison result, 0 means equal, non-zero means unequal)
 * @return 成功返回 0，如果有任何参数为 NULL 返回 -1 (Returns 0 on success, -1 if any of the arguments are NULL)
 */
int rcutils_strcasecmp(const char * s1, const char * s2, int * value)
{
  // 检查输入参数是否为 NULL (Check if input arguments are NULL)
  if (s1 == NULL || s2 == NULL || value == NULL) {
    return -1;
  }

#ifndef _WIN32
  // 使用 POSIX 函数 strcasecmp 比较字符串 (Use the POSIX function strcasecmp to compare strings)
  *value = strcasecmp(s1, s2);
#else
  // 在 Windows 上使用 _stricmp 函数比较字符串 (On Windows, use the _stricmp function to compare strings)
  *value = _stricmp(s1, s2);
#endif

  return 0;
}

/**
 * @brief 比较两个字符串的前 n 个字符，忽略大小写 (Compare the first n characters of two strings, case-insensitive)
 *
 * @param[in] s1 第一个字符串 (The first string)
 * @param[in] s2 第二个字符串 (The second string)
 * @param[in] n 要比较的字符数 (The number of characters to compare)
 * @param[out] value 指向比较结果的指针，0 表示相等，非 0 表示不相等 (Pointer to the comparison result, 0 means equal, non-zero means unequal)
 * @return 成功返回 0，如果有任何参数为 NULL 返回 -1 (Returns 0 on success, -1 if any of the arguments are NULL)
 */
int rcutils_strncasecmp(const char * s1, const char * s2, size_t n, int * value)
{
  // 检查输入参数是否为 NULL (Check if input arguments are NULL)
  if (s1 == NULL || s2 == NULL || value == NULL) {
    return -1;
  }

#ifndef _WIN32
  // 使用 POSIX 函数 strncasecmp 比较字符串的前 n 个字符 (Use the POSIX function strncasecmp to compare the first n characters of strings)
  *value = strncasecmp(s1, s2, n);
#else
  // 在 Windows 上使用 _strnicmp 函数比较字符串的前 n 个字符 (On Windows, use the _strnicmp function to compare the first n characters of strings)
  *value = _strnicmp(s1, s2, n);
#endif

  return 0;
}

#ifdef __cplusplus
}
#endif
