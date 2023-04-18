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

#ifndef RCUTILS__SPLIT_H_
#define RCUTILS__SPLIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/allocator.h"
#include "rcutils/types.h"
#include "rcutils/visibility_control.h"

/// 使用指定的分隔符拆分给定字符串 (Split a given string with the specified delimiter)
/**
 * \param[in] str 要拆分的字符串 (string to split)
 * \param[in] delimiter 用于拆分的分隔符 (delimiter on where to split)
 * \param[in] allocator 为输出数组分配新内存的分配器 (allocator for allocating new memory for the output array)
 * \param[out] string_array 包含拆分标记的字符串数组 (string_array with the split tokens)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数 (for invalid arguments), or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败 (if memory allocation fails), or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误 (if an unknown error occurs)
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_split(
  const char * str,
  char delimiter,
  rcutils_allocator_t allocator,
  rcutils_string_array_t * string_array);

/// 在指定分隔符的最后一次出现处拆分给定字符串 (Split a given string on the last occurrence of the specified delimiter)
/**
 * \param[in] str 要拆分的字符串 (string to split)
 * \param[in] delimiter 用于拆分的分隔符 (delimiter on where to split)
 * \param[in] allocator 为输出数组分配新内存的分配器 (allocator for allocating new memory for the output array)
 * \param[out] string_array 包含拆分标记的字符串数组 (string_array with the split tokens)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误 (if an unknown error occurs)
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_split_last(
  const char * str,
  char delimiter,
  rcutils_allocator_t allocator,
  rcutils_string_array_t * string_array);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__SPLIT_H_
