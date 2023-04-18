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

#include "rcutils/format_string.h"

#include <stdarg.h>
#include <stddef.h>
#ifdef _WIN32
#include <stdio.h>
#endif
#include <string.h>

#include "rcutils/snprintf.h"

/**
 * @brief 格式化字符串，限制输出长度。Format a string with a length limit.
 *
 * @param[in] allocator 分配器，用于分配内存。Allocator for memory allocation.
 * @param[in] limit 输出字符串的最大长度限制。Maximum length limit of the output string.
 * @param[in] format_string 需要格式化的字符串。String to be formatted.
 * @param[in] ... 可变参数列表，与格式化字符串对应。Variable argument list corresponding to the format string.
 * @return 返回格式化后的字符串，如果失败返回NULL。Returns the formatted string, or NULL on failure.
 */
char * rcutils_format_string_limit(
  rcutils_allocator_t allocator, size_t limit, const char * format_string, ...)
{
  // 检查 format_string 是否为 NULL
  // Check if format_string is NULL
  if (NULL == format_string) {
    return NULL;
  }
  // 检查分配器是否有效
  // Check if the allocator is valid
  RCUTILS_CHECK_ALLOCATOR(&allocator, return NULL);

  // 提取可变参数两次，一次用于计算长度，一次用于格式化
  // Extract variadic arguments twice, once for length calculation and once for formatting
  va_list args1;
  va_start(args1, format_string);
  va_list args2;
  va_copy(args2, args1);

  // 首先计算输出字符串的长度
  // First, calculate the length of the output string
  size_t bytes_to_be_written = (size_t)rcutils_vsnprintf(NULL, 0, format_string, args1);
  va_end(args1);
  if (bytes_to_be_written == (size_t)-1) {
    va_end(args2);
    return NULL;
  }

  // 为返回字符串分配空间
  // Allocate space for the return string
  if (bytes_to_be_written + 1 > limit) {
    bytes_to_be_written = limit - 1;
  }
  char * output_string = allocator.allocate(bytes_to_be_written + 1, allocator.state);
  if (NULL == output_string) {
    va_end(args2);
    return NULL;
  }

  // 格式化字符串
  // Format the string
  int ret = rcutils_vsnprintf(output_string, bytes_to_be_written + 1, format_string, args2);
  if (0 > ret) {
    allocator.deallocate(output_string, allocator.state);
    va_end(args2);
    return NULL;
  }

  // 添加字符串结束符
  // Add string terminator
  output_string[bytes_to_be_written] = '\0';
  va_end(args2);

  // 返回格式化后的字符串
  // Return the formatted string
  return output_string;
}

#ifdef __cplusplus
}
#endif
