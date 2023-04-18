// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#include "rcutils/snprintf.h"

#include <errno.h>
#ifdef _WIN32
#include <string.h>
#endif
#include <stdarg.h>
#include <stdio.h>

/**
 * @brief 使用格式字符串和参数列表格式化输出到缓冲区 (Format output to buffer using format string and argument list)
 *
 * @param[in] buffer 缓冲区指针 (Pointer to the buffer)
 * @param[in] buffer_size 缓冲区大小 (Size of the buffer)
 * @param[in] format 格式字符串 (Format string)
 * @param[in] ... 可变参数列表 (Variable argument list)
 * @return 格式化后的字符串长度 (Length of the formatted string)
 */
int rcutils_snprintf(char * buffer, size_t buffer_size, const char * format, ...)
{
  va_list args;
  // 初始化可变参数列表 (Initialize the variable argument list)
  va_start(args, format);
  // 调用 rcutils_vsnprintf 处理可变参数列表 (Call rcutils_vsnprintf to handle the variable argument list)
  int ret = rcutils_vsnprintf(buffer, buffer_size, format, args);
  // 清除可变参数列表 (Clean up the variable argument list)
  va_end(args);
  // 返回格式化后的字符串长度 (Return the length of the formatted string)
  return ret;
}

/**
 * @brief 使用格式字符串和 va_list 类型的参数列表格式化输出到缓冲区 (Format output to buffer using format string and va_list type argument list)
 *
 * @param[in] buffer 缓冲区指针 (Pointer to the buffer)
 * @param[in] buffer_size 缓冲区大小 (Size of the buffer)
 * @param[in] format 格式字符串 (Format string)
 * @param[in] args va_list 类型的参数列表 (Argument list of type va_list)
 * @return 格式化后的字符串长度 (Length of the formatted string)
 */
int rcutils_vsnprintf(char * buffer, size_t buffer_size, const char * format, va_list args)
{
  // 检查错误并设置 errno (Check for errors and set errno)
  RCUTILS_CAN_FAIL_WITH({
    errno = EINVAL;
    return -1;
  });

  // 检查格式字符串是否为空 (Check if the format string is NULL)
  if (NULL == format) {
    errno = EINVAL;
    return -1;
  }
  // 检查缓冲区指针和大小 (Check buffer pointer and size)
  if (NULL == buffer && 0 == buffer_size) {
#ifndef _WIN32
    // 使用 vsnprintf 获取格式化后的字符串长度 (Use vsnprintf to get the length of the formatted string)
    return vsnprintf(NULL, 0, format, args);
#else
    // 使用 _vscprintf 获取格式化后的字符串长度 (Use _vscprintf to get the length of the formatted string)
    return _vscprintf(format, args);
#endif
  }
  // 检查缓冲区指针或大小是否无效 (Check if buffer pointer or size is invalid)
  if (NULL == buffer || 0 == buffer_size) {
    errno = EINVAL;
    return -1;
  }

  int ret;
#ifndef _WIN32
  // 使用 vsnprintf 格式化输出到缓冲区 (Format output to buffer using vsnprintf)
  ret = vsnprintf(buffer, buffer_size, format, args);
#else
  // 在截断发生时，errno 没有明确设置为 0 (errno isn't explicitly set to 0 when truncation occurs)
  errno = 0;
  // 使用 _vsnprintf_s 格式化输出到缓冲区 (Format output to buffer using _vsnprintf_s)
  ret = _vsnprintf_s(buffer, buffer_size, _TRUNCATE, format, args);
  // 检查截断是否发生 (Check if truncation has occurred)
  if (-1 == ret && 0 == errno) {
    // 截断发生时，返回本应具有的长度 (Return the length it should have been when truncation occurs)
    return _vscprintf(format, args);
  }
#endif
  // 返回格式化后的字符串长度 (Return the length of the formatted string)
  return ret;
}

#ifdef __cplusplus
}
#endif
