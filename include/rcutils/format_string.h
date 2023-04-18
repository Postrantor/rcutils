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

#ifndef RCUTILS__FORMAT_STRING_H_
#define RCUTILS__FORMAT_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// 返回一个新分配的字符串，使用格式字符串创建。
/// Return a newly allocated string, created with a format string.
/**
 * 该函数与 rcutils_format_string_limit() 相同，只是它具有隐式限制 2048。
 * 对于更长的格式字符串，请参阅 rcutils_format_string_limit()。
 * This function is identical to rcutils_format_string_limit() except it has an
 * implicit limit of 2048.
 * For longer format strings, see rcutils_format_string_limit().
 */
#define rcutils_format_string(allocator, format_string, ...)                                       \
  rcutils_format_string_limit(allocator, 2048, format_string, __VA_ARGS__)

/// 返回一个新分配的字符串，使用格式字符串创建，最多可达到限制长度。
/// Return a newly allocated string, created with a format string up to a limit.
/**
 * 此函数使用 snprintf_s 确定结果字符串的长度，并为结果字符串分配存储空间，
 * 对其进行格式化，然后返回结果。
 * This function uses snprintf_s to determine the length of the resulting
 * string and allocates storage for the resulting string, formats it, and
 * then returns the result.
 *
 * 此函数可能会失败，因此如果 format_string 为空或内存分配失败或 snprintf_s 失败，
 * 则返回 null。在任何情况下都不会设置错误消息。
 * This function can fail and therefore return null if the format_string is
 * null or if memory allocation fails or if snprintf_s fails.
 * An error message is not set in any case.
 *
 * 超过给定限制的输出字符串将被截断。
 * Output strings that would be longer than the given limit are truncated.
 *
 * 所有返回的字符串都以空字符结尾。
 * All returned strings are null terminated.
 *
 * 格式字符串传递给 snprintf_s()，请参阅其文档了解如何使用格式字符串。
 * The format string is passed to snprintf_s(), see its documentation for
 * how to use the format string.
 *
 * 返回的字符串在不再需要时必须使用相同的分配器进行处理。
 * The returned string must be deallocated using the same allocator given once
 * it is no longer needed.
 *
 * \see rcutils_snprintf()
 *
 * \param[in] allocator 用于分配的分配器
 * \param[in] limit 输出字符串的最大长度
 * \param[in] format_string 输出的格式，必须以空字符结尾
 * \param[in] allocator the allocator to use for allocation
 * \param[in] limit maximum length of the output string
 * \param[in] format_string format of the output, must be null terminated
 * \return 新分配且格式化的输出字符串，或
 * \return 如果发生错误，则返回 `NULL`。
 * \return The newly allocated and format output string, or
 * \return `NULL` if there was an error.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
char * rcutils_format_string_limit(
  rcutils_allocator_t allocator, size_t limit, const char * format_string, ...)
  /// @cond Doxygen_Suppress
  RCUTILS_ATTRIBUTE_PRINTF_FORMAT(3, 4)
  /// @endcond
  ;

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__FORMAT_STRING_H_
