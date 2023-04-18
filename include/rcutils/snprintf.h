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

#ifndef RCUTILS__SNPRINTF_H_
#define RCUTILS__SNPRINTF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>

#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// 格式化字符串 (Format a string).
/**
 * 这个函数只是以一种便携的方式封装了 C11 中定义的 snprintf() 函数。
 * (This function just wraps snprintf() as defined in C11 in a portable way.)
 *
 * 在 Windows 上，默认使用 _snprintf_s() 的 _TRUNCATE 行为，但只在 errno 不为 0 时返回 -1。
 * (On Windows this defaults to the _TRUNCATE behavior of _snprintf_s(), but
 * only returns -1 if errno is not 0.)
 * 与 _snprintf_s() 在截断发生时返回 -1 不同，此函数的行为类似于 snprintf() (http://en.cppreference.com/w/cpp/io/c/fprintf)：
 * (Unlike _snprintf_s() which returns -1 when truncation occurs, this function
 * behaves like snprintf():)
 *
 * > 如果成功，则写入字符的数量；如果发生错误，则为负值。
 * > (Number of characters written if successful or negative value if an error occurred.)
 * > 如果由于 buf_size 限制导致生成的字符串被截断，函数返回不包括终止空字节的字符总数，如果没有限制，则返回该字符总数。
 * > (If the resulting string gets truncated due to buf_size limit, function
 * > returns the total number of characters (not including the terminating
 * > null-byte) which would have been written, if the limit was not imposed.)
 *
 * 如果 buffer 和 buffer_size 分别给定 `NULL` 和 `0`，则返回将要生成的字符串的大小。
 * (If `NULL` and `0` are given for buffer and buffer_size respectively, the
 * size of the string that would be generated is returned.)
 * 使用 snprintf() 或 _vscprintf() 计算此值。
 * (Either snprintf() or _vscprintf() is used to calculate this value.)
 *
 * \see snprintf()
 * \see _snprintf_s()
 * \return 如果有足够的空间，将返回本应写入的字节数，或
 * (\return the number of bytes that would have been written given enough space, or)
 * \return 如果有错误，则返回负数，但与 _snprintf_s() 不同，如果有截断，不会返回 -1。
 * (\return a negative number if there is an error, but unlike _snprintf_s(),
 *   -1 is not returned if there is truncation.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_snprintf(char * buffer, size_t buffer_size, const char * format, ...)
  /// @cond Doxygen_Suppress
  RCUTILS_ATTRIBUTE_PRINTF_FORMAT(3, 4)
  /// @endcond
  ;

/// 使用 va_list 格式化字符串，详见 rcutils_snprintf()。
/// (Format a string with va_list for arguments, see rcutils_snprintf().)
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_vsnprintf(char * buffer, size_t buffer_size, const char * format, va_list args);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__SNPRINTF_H_
