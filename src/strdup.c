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

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/strdup.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "./common.h"
#include "rcutils/macros.h"

/**
 * @brief 使用指定的分配器复制一个字符串。
 * @param str 要复制的源字符串。
 * @param allocator 用于分配内存的分配器。
 * @return 返回新分配的字符串，如果失败则返回NULL。
 *
 * @brief Duplicates a string using the specified allocator.
 * @param str The source string to be duplicated.
 * @param allocator The allocator to use for memory allocation.
 * @return Returns the newly allocated string, or NULL on failure.
 */
char * rcutils_strdup(const char * str, rcutils_allocator_t allocator)
{
  // 检查错误并返回NULL。
  // Check for errors and return NULL.
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(NULL);

  // 调用 rcutils_strndup 函数完成字符串复制操作。
  // Call the rcutils_strndup function to perform the string duplication operation.
  return rcutils_strndup(str, SIZE_MAX, allocator);
}

/**
 * @brief 使用指定的分配器复制一个字符串，最多复制 max_length 个字符。
 * @param str 要复制的源字符串。
 * @param max_length 最大复制长度。
 * @param allocator 用于分配内存的分配器。
 * @return 返回新分配的字符串，如果失败则返回NULL。
 *
 * @brief Duplicates a string using the specified allocator, copying at most max_length characters.
 * @param str The source string to be duplicated.
 * @param max_length The maximum length to copy.
 * @param allocator The allocator to use for memory allocation.
 * @return Returns the newly allocated string, or NULL on failure.
 */
char * rcutils_strndup(const char * str, size_t max_length, rcutils_allocator_t allocator)
{
  // 检查错误并返回NULL。
  // Check for errors and return NULL.
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(NULL);

  // 如果传入的字符串为NULL，直接返回NULL。
  // If the input string is NULL, return NULL directly.
  if (NULL == str) {
    return NULL;
  }

  // 查找字符串中的空字符，并确定字符串长度。
  // Find the null character in the string and determine the string length.
  char * p = memchr(str, '\0', max_length);
  size_t string_length = p == NULL ? max_length : (size_t)(p - str);

  // 使用分配器为新字符串分配内存。
  // Use the allocator to allocate memory for the new string.
  char * new_string = allocator.allocate(string_length + 1, allocator.state);

  // 如果内存分配失败，返回NULL。
  // If memory allocation fails, return NULL.
  if (NULL == new_string) {
    return NULL;
  }

  // 将源字符串复制到新分配的内存中。
  // Copy the source string into the newly allocated memory.
  memcpy(new_string, str, string_length);

  // 在新字符串的末尾添加空字符。
  // Add a null character at the end of the new string.
  new_string[string_length] = '\0';

  // 返回新分配的字符串。
  // Return the newly allocated string.
  return new_string;
}

#ifdef __cplusplus
}
#endif
