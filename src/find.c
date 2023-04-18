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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/find.h"
#include "rcutils/types.h"

/**
 * @brief 在字符串中查找指定的分隔符 (Find the specified delimiter in the string)
 * @param str 输入字符串 (The input string)
 * @param delimiter 要查找的分隔符 (The delimiter to be found)
 * @return 如果找到分隔符，则返回其位置；否则返回 SIZE_MAX (If the delimiter is found, return its position; otherwise return SIZE_MAX)
 */
size_t rcutils_find(const char * str, char delimiter)
{
  // 检查输入字符串是否为空或长度为0 (Check if the input string is NULL or has a length of 0)
  if (NULL == str || 0 == strlen(str)) {
    return SIZE_MAX;
  }
  // 调用 rcutils_findn 函数进行查找 (Call the rcutils_findn function to perform the search)
  return rcutils_findn(str, delimiter, strlen(str));
}

/**
 * @brief 在给定长度的字符串中查找指定的分隔符 (Find the specified delimiter in the string with given length)
 * @param str 输入字符串 (The input string)
 * @param delimiter 要查找的分隔符 (The delimiter to be found)
 * @param string_length 字符串的长度 (The length of the string)
 * @return 如果找到分隔符，则返回其位置；否则返回 SIZE_MAX (If the delimiter is found, return its position; otherwise return SIZE_MAX)
 */
size_t rcutils_findn(const char * str, char delimiter, size_t string_length)
{
  // 检查输入字符串是否为空或长度为0 (Check if the input string is NULL or has a length of 0)
  if (NULL == str || 0 == string_length) {
    return SIZE_MAX;
  }

  // 遍历字符串，查找指定的分隔符 (Traverse the string to find the specified delimiter)
  for (size_t i = 0; i < string_length; ++i) {
    if (str[i] == delimiter) {
      return i;
    }
  }
  return SIZE_MAX;
}

/**
 * @brief 在字符串中查找指定的分隔符的最后一个位置 (Find the last position of the specified delimiter in the string)
 * @param str 输入字符串 (The input string)
 * @param delimiter 要查找的分隔符 (The delimiter to be found)
 * @return 如果找到分隔符，则返回其位置；否则返回 SIZE_MAX (If the delimiter is found, return its position; otherwise return SIZE_MAX)
 */
size_t rcutils_find_last(const char * str, char delimiter)
{
  // 检查输入字符串是否为空或长度为0 (Check if the input string is NULL or has a length of 0)
  if (NULL == str || 0 == strlen(str)) {
    return SIZE_MAX;
  }
  // 调用 rcutils_find_lastn 函数进行查找 (Call the rcutils_find_lastn function to perform the search)
  return rcutils_find_lastn(str, delimiter, strlen(str));
}

/**
 * @brief 在给定长度的字符串中查找指定的分隔符的最后一个位置 (Find the last position of the specified delimiter in the string with given length)
 * @param str 输入字符串 (The input string)
 * @param delimiter 要查找的分隔符 (The delimiter to be found)
 * @param string_length 字符串的长度 (The length of the string)
 * @return 如果找到分隔符，则返回其位置；否则返回 SIZE_MAX (If the delimiter is found, return its position; otherwise return SIZE_MAX)
 */
size_t rcutils_find_lastn(const char * str, char delimiter, size_t string_length)
{
  // 检查输入字符串是否为空或长度为0 (Check if the input string is NULL or has a length of 0)
  if (NULL == str || 0 == string_length) {
    return SIZE_MAX;
  }

#if defined(_GNU_SOURCE)
  // 使用 memrchr 函数进行反向查找 (Use the memrchr function for reverse search)
  const char * ptr = memrchr(str, delimiter, string_length);
  if (ptr == NULL) {
    return SIZE_MAX;
  }

  // 计算找到的分隔符在字符串中的位置 (Calculate the position of the found delimiter in the string)
  return ptr - str;
#else
  // 遍历字符串，查找指定的分隔符的最后一个位置 (Traverse the string to find the last position of the specified delimiter)
  for (size_t i = string_length - 1; i > 0; --i) {
    if (str[i] == delimiter) {
      return i;
    }
  }
  // 如果第一个字符是分隔符，则返回0；否则返回 SIZE_MAX (If the first character is the delimiter, return 0; otherwise return SIZE_MAX)
  return str[0] == delimiter ? 0 : SIZE_MAX;
#endif
}
