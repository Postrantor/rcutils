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

#ifndef RCUTILS__STRDUP_H_
#define RCUTILS__STRDUP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// 返回一个使用分配器复制的字符串，如果发生错误则返回空。
/// Return a duplicated string with an allocator, or null if an error occurs.
/**
 * 此函数与 rcutils_strndup() 相同，只是不需要给出 c 字符串的长度，
 * 因此 c 字符串必须以空字符结尾。
 * This function is identical to rcutils_strndup() except the length of the
 * c string does not have to be given and therefore the c string must be
 * null terminated.
 *
 * \see rcutils_strndup()
 *
 * \param[in] str 要复制的空终止 c 字符串
 * \param[in] str null terminated c string to be duplicated
 * \param[in] allocator 用于分配的分配器
 * \param[in] allocator the allocator to use for allocation
 * \return 复制的字符串，或
 * \return duplicated string, or
 * \return 如果有错误，则返回 `NULL`。
 * \return `NULL` if there is an error.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
char * rcutils_strdup(const char * str, rcutils_allocator_t allocator);

/// 返回一个使用分配器复制的字符串，如果发生错误则返回空。
/// Return a duplicated string with an allocator, or null if an error occurs.
/**
 * 如果无法分配内存或输入 c 字符串指针为空，此函数可能会失败并返回空。
 * 在这两种情况下都不会设置错误消息。
 * The function can fail and return null if memory cannot be allocated or
 * if the input c string pointer is null.
 * In both cases no error message is set.
 * 不再需要时，应使用给定的分配器释放返回的字符串。
 * The returned string should be deallocated using the given allocator when
 * it is no longer needed.
 *
 * 给定的 max_length 不包括空终止字符。
 * 因此，max_length 为 0 仍将导致复制的字符串，但字符串将为空字符串，其 strlen 为 0，
 * 但仍必须释放它。所有返回的字符串都以空字符结尾。
 * The max_length given does not include the null terminating character.
 * Therefore a max_length of 0 will still result in a duplicated string, but
 * the string will be an empty string of strlen 0, but it still must be
 * deallocated.
 * All returned strings are null terminated.
 *
 * \param[in] str 要复制的空终止 c 字符串
 * \param[in] str null terminated c string to be duplicated
 * \param[in] max_length 要复制的字符串的最大长度
 * \param[in] max_length maximum length of the string to duplicate
 * \param[in] allocator 用于分配的分配器
 * \param[in] allocator the allocator to use for allocation
 * \return 复制的字符串，或
 * \return duplicated string, or
 * \return 如果有错误，则返回 `NULL`。
 * \return `NULL` if there is an error.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
char * rcutils_strndup(const char * str, size_t max_length, rcutils_allocator_t allocator);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__STRDUP_H_
