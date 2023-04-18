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

#ifndef RCUTILS__FIND_H_
#define RCUTILS__FIND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/types.h"
#include "rcutils/visibility_control.h"

/// 返回字符串中某个字符首次出现的位置（Return the first index of a character in a string.）
/**
 * 在一个字符串中搜索某个分隔符的第一次出现。
 * (Search in a string for the first occurrence of a delimiter.)
 *
 * \param[in] str 要搜索的以空字符结尾的C字符串（null terminated c string to search）
 * \param[in] delimiter 要搜索的字符（the character to search for）
 * \return 如果找到分隔符，返回其第一次出现的索引，或者（the index of the first occurrence of the delimiter if found, or）
 * \return 对于无效参数，返回 `SIZE_MAX`，或者（`SIZE_MAX` for invalid arguments, or）
 * \return 如果未找到分隔符，返回 `SIZE_MAX`（`SIZE_MAX` if the delimiter is not found.）
 */
RCUTILS_PUBLIC
size_t rcutils_find(const char * str, char delimiter);

/// 在指定长度的字符串中返回某个字符首次出现的位置（Return the first index of a character in a string of specified length.）
/**
 * 与 rcutils_find_first() 相同，但不依赖于字符串是以空字符结尾的C字符串。
 * (Identical to rcutils_find_first() but without relying on the string to be a
 * null terminated c string.)
 *
 * \param[in] str 要搜索的字符串（string to search）
 * \param[in] delimiter 要搜索的字符（the character to search for）
 * \param[in] string_length 要搜索的字符串长度（length of the string to search）
 * \return 如果找到分隔符，返回其第一次出现的索引，或者（the index of the first occurrence of the delimiter if found, or）
 * \return 对于无效参数，返回 `SIZE_MAX`，或者（`SIZE_MAX` for invalid arguments, or）
 * \return 如果未找到分隔符，返回 `SIZE_MAX`（`SIZE_MAX` if the delimiter is not found.）
 */
RCUTILS_PUBLIC
size_t rcutils_findn(const char * str, char delimiter, size_t string_length);

/// 返回字符串中某个字符最后一次出现的位置（Return the last index of a character in a string.）
/**
 * 在一个字符串中搜索某个分隔符的最后一次出现。
 * (Search in a string for the last occurrence of a delimiter.)
 *
 * \param[in] str 要搜索的以空字符结尾的C字符串（null terminated c string to search）
 * \param[in] delimiter 要搜索的字符（the character to search for）
 * \return 如果找到分隔符，返回其最后一次出现的索引，或者（the index of the last occurrence of the delimiter if found, or）
 * \return 对于无效参数，返回 `SIZE_MAX`，或者（`SIZE_MAX` for invalid arguments, or）
 * \return 如果未找到分隔符，返回 `SIZE_MAX`（`SIZE_MAX` if the delimiter is not found.）
 */
RCUTILS_PUBLIC
size_t rcutils_find_last(const char * str, char delimiter);

/// 在指定长度的字符串中返回某个字符最后一次出现的位置（Return the last index of a character in a string of specified length.）
/**
 * 与 rcutils_find_last() 相同，但不依赖于字符串是以空字符结尾的C字符串。
 * (Identical to rcutils_find_last() but without relying on the string to be a
 * null terminated c string.)
 *
 * \param[in] str 要搜索的字符串（string to search）
 * \param[in] delimiter 要搜索的字符（the character to search for）
 * \param[in] string_length 要搜索的字符串长度（length of the string to search）
 * \return 如果找到分隔符，返回其最后一次出现的索引，或者（the index of the last occurrence of the delimiter if found, or）
 * \return 对于无效参数，返回 `SIZE_MAX`，或者（`SIZE_MAX` for invalid arguments, or）
 * \return 如果未找到分隔符，返回 `SIZE_MAX`（`SIZE_MAX` if the delimiter is not found.）
 */
RCUTILS_PUBLIC
size_t rcutils_find_lastn(const char * str, char delimiter, size_t string_length);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__FIND_H_
