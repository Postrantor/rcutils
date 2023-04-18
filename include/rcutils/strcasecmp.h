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

/// \file

#ifndef RCUTILS__STRCASECMP_H_
#define RCUTILS__STRCASECMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// 不区分大小写的字符串比较。
/// Case insensitive string compare.
/**
 * 此函数以可移植的方式比较两个忽略大小写的字符串。
 * This function compares two strings ignoring case in a portable way.
 * 该函数逐字节比较字符串 s1 和 s2，忽略字符的大小写。
 * This performs a byte-by-byte comparison of the strings s1 and s2,
 * ignoring the case of the characters.
 *
 * \param[in] s1 要比较的空终止字符串。
 * \param[in] s1 Null terminated string to compare.
 * \param[in] s2 要比较的空终止字符串。
 * \param[in] s2 Null terminated string to compare.
 * \param[out] value 指向比较结果的指针。
 * \param[out] value Pointer to comparison result.
 *   如果在忽略大小写后，发现 s1 小于、匹配或大于 s2，则返回一个小于、等于或大于零的整数。
 *   An integer less than, equal to, or greater than zero if s1 is, after
 *   ignoring case, found to be less than, to match, or be greater than s2,
 *   respectively.
 * \return 如果方法成功，则返回 0，或者
 * \return 0 if method succeeded, or
 * \return 如果失败，则返回 -1。
 * \return -1 if failed.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_strcasecmp(const char * s1, const char * s2, int * value);

/// 在计数字符内不区分大小写的字符串比较。
/// Case insensitive string compare up to count characters.
/**
 * 此函数以可移植的方式比较两个忽略大小写的字符串。
 * This function compares two strings ignoring case in a portable way.
 * 该函数在 s1 和 s2 的计数字符内逐字节比较字符串，忽略字符的大小写。
 * This performs a byte-by-byte comparison of the strings s1 and s2 up to count
 * characters of s1 and s2, ignoring the case of the characters.
 *
 * \param[in] s1 要比较的第一个字符串。
 * \param[in] s1 First string to compare.
 * \param[in] s2 要比较的第二个字符串。
 * \param[in] s2 Second string to compare.
 * \param[in] n 要比较的字符数。
 * \param[in] n Count of characters to compare.
 * \param[out] value 指向比较结果的指针。
 * \param[out] value Pointer to comparison result.
 *   如果在忽略大小写后，发现 s1 小于、匹配或大于 s2，则返回一个小于、等于或大于零的整数。
 *   An integer less than, equal to, or greater than zero if s1 is, after
 *   ignoring case, found to be less than, to match, or be greater than s2,
 *   respectively.
 * \return 如果方法成功，则返回 0，或者
 * \return 0 if method succeeded, or
 * \return 如果失败，则返回 -1。
 * \return -1 if failed.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_strncasecmp(const char * s1, const char * s2, size_t n, int * value);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__STRCASECMP_H_
