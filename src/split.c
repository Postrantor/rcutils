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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/error_handling.h"
#include "rcutils/format_string.h"
#include "rcutils/logging_macros.h"
#include "rcutils/split.h"
#include "rcutils/types.h"

/**
 * @brief 分割字符串 (Split a string)
 *
 * @param[in] str 要分割的字符串 (The string to be split)
 * @param[in] delimiter 用于分割字符串的分隔符 (The delimiter used to split the string)
 * @param[in] allocator 分配器 (The allocator)
 * @param[out] string_array 存储分割后的字符串数组 (The string array to store the split strings)
 * @return rcutils_ret_t 返回操作状态 (Return the operation status)
 */
rcutils_ret_t rcutils_split(
  const char * str,
  char delimiter,
  rcutils_allocator_t allocator,
  rcutils_string_array_t * string_array)
{
  // 检查string_array是否为空 (Check if string_array is null)
  if (NULL == string_array) {
    RCUTILS_SET_ERROR_MSG("string_array is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  // 检查str是否为空或空字符串 (Check if str is null or an empty string)
  if (NULL == str || '\0' == *str) {
    *string_array = rcutils_get_zero_initialized_string_array();
    return RCUTILS_RET_OK;
  }
  string_array->allocator = allocator;

  size_t string_size = strlen(str);

  // 字符串是否以分隔符开头？(Does the string start with a delimiter?)
  size_t lhs_offset = 0;
  if (str[0] == delimiter) {
    lhs_offset = 1;
  }

  // 字符串是否以分隔符结尾？(Does the string end with a delimiter?)
  size_t rhs_offset = 0;
  if (str[string_size - 1] == delimiter) {
    rhs_offset = 1;
  }

  // 计算分割后的字符串数量 (Calculate the number of split strings)
  string_array->size = 1;
  for (size_t i = lhs_offset; i < string_size - rhs_offset; ++i) {
    if (str[i] == delimiter) {
      ++string_array->size;
    }
  }
  // 分配内存来存储分割后的字符串数组 (Allocate memory to store the split string array)
  string_array->data = allocator.allocate(string_array->size * sizeof(char *), allocator.state);
  if (NULL == string_array->data) {
    goto fail;
  }

  size_t token_counter = 0;
  size_t lhs = 0 + lhs_offset;
  size_t rhs = 0 + lhs_offset;
  // 遍历字符串，根据分隔符进行分割 (Traverse the string and split it according to the delimiter)
  for (; rhs < string_size - rhs_offset; ++rhs) {
    if (str[rhs] == delimiter) {
      // 如果有两个连续的分隔符，忽略它们并减小数组大小 (If there are two consecutive delimiters, ignore them and reduce the array size)
      if (rhs - lhs < 1) {
        --string_array->size;
        string_array->data[string_array->size] = NULL;
      } else {
        // 分配内存并将分割后的字符串存储到数组中 (Allocate memory and store the split string in the array)
        string_array->data[token_counter] =
          allocator.allocate((rhs - lhs + 2) * sizeof(char), allocator.state);
        if (NULL == string_array->data[token_counter]) {
          string_array->size = token_counter;
          goto fail;
        }
        snprintf(string_array->data[token_counter], (rhs - lhs + 1), "%s", str + lhs);
        ++token_counter;
      }
      lhs = rhs;
      ++lhs;
    }
  }

  // 处理末尾的标记 (Handle the trailing token)
  if (rhs - lhs < 1) {
    --string_array->size;
    string_array->data[string_array->size] = NULL;
  } else {
    string_array->data[token_counter] =
      allocator.allocate((rhs - lhs + 2) * sizeof(char), allocator.state);
    snprintf(string_array->data[token_counter], (rhs - lhs + 1), "%s", str + lhs);
  }

  return RCUTILS_RET_OK;

fail:
  // 如果内存分配失败，清理并返回错误 (If memory allocation fails, clean up and return an error)
  if (rcutils_string_array_fini(string_array) != RCUTILS_RET_OK) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("failed to finalize string array during error handling: ");
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    rcutils_reset_error();
  }

  RCUTILS_SET_ERROR_MSG("unable to allocate memory for string array data");
  return RCUTILS_RET_ERROR;
}

/**
 * @brief 分割字符串中最后出现的分隔符 (Split the string at the last occurrence of the delimiter)
 *
 * @param[in] str 输入字符串 (Input string)
 * @param[in] delimiter 分隔符 (Delimiter)
 * @param[in] allocator 内存分配器 (Memory allocator)
 * @param[out] string_array 存储分割结果的字符串数组 (String array to store the split result)
 * @return rcutils_ret_t 返回操作状态 (Return operation status)
 */
rcutils_ret_t rcutils_split_last(
  const char * str,
  char delimiter,
  rcutils_allocator_t allocator,
  rcutils_string_array_t * string_array)
{
  // 检查输入字符串是否为空或长度为0 (Check if the input string is null or has a length of 0)
  if (NULL == str || strlen(str) == 0) {
    *string_array = rcutils_get_zero_initialized_string_array();
    return RCUTILS_RET_OK;
  }

  size_t string_size = strlen(str);

  // 字符串是否以分隔符开头？(Does the string start with a delimiter?)
  size_t lhs_offset = 0;
  if (str[0] == delimiter) {
    lhs_offset = 1;
  }

  // 字符串是否以分隔符结尾？(Does the string end with a delimiter?)
  size_t rhs_offset = 0;
  if (str[string_size - 1] == delimiter) {
    rhs_offset = 1;
  }

  size_t found_last = string_size;
  // 寻找最后一个分隔符的位置 (Find the position of the last delimiter)
  for (size_t i = lhs_offset; i < string_size - rhs_offset; ++i) {
    if (str[i] == delimiter) {
      found_last = i;
    }
  }

  rcutils_ret_t result_error;
  // 如果找不到分隔符，将整个字符串作为一个元素 (If the delimiter is not found, treat the entire string as one element)
  if (found_last == string_size) {
    // 初始化一个只有一个元素的字符串数组 (Initialize a string array with only one element)
    rcutils_ret_t ret = rcutils_string_array_init(string_array, 1, &allocator);
    if (ret != RCUTILS_RET_OK) {
      result_error = ret;
      goto fail;
    }
    // 为字符串数组的第一个元素分配内存 (Allocate memory for the first element of the string array)
    string_array->data[0] =
      allocator.allocate((found_last - lhs_offset + 2) * sizeof(char), allocator.state);
    if (NULL == string_array->data[0]) {
      result_error = RCUTILS_RET_BAD_ALLOC;
      goto fail;
    }
    // 将输入字符串的一部分复制到字符串数组的第一个元素中 (Copy a part of the input string to the first element of the string array)
    snprintf(string_array->data[0], found_last - lhs_offset + 1, "%s", str + lhs_offset);
  } else {
    // 如果找到分隔符，将字符串分成两部分 (If the delimiter is found, split the string into two parts)
    rcutils_ret_t ret = rcutils_string_array_init(string_array, 2, &allocator);
    if (ret != RCUTILS_RET_OK) {
      result_error = ret;
      goto fail;
    }

    size_t inner_rhs_offset = (str[found_last - 1] == delimiter) ? 1 : 0;
    // 为字符串数组的第一个元素分配内存 (Allocate memory for the first element of the string array)
    string_array->data[0] = allocator.allocate(
      (found_last + 1 - lhs_offset - inner_rhs_offset + 1) * sizeof(char), allocator.state);
    if (NULL == string_array->data[0]) {
      result_error = RCUTILS_RET_BAD_ALLOC;
      goto fail;
    }
    // 将输入字符串的前半部分复制到字符串数组的第一个元素中 (Copy the first half of the input string to the first element of the string array)
    snprintf(
      string_array->data[0], found_last + 1 - lhs_offset - inner_rhs_offset, "%s",
      str + lhs_offset);

    // 为字符串数组的第二个元素分配内存 (Allocate memory for the second element of the string array)
    string_array->data[1] = allocator.allocate(
      (string_size - found_last - rhs_offset + 1) * sizeof(char), allocator.state);
    if (NULL == string_array->data[1]) {
      result_error = RCUTILS_RET_BAD_ALLOC;
      goto fail;
    }
    // 将输入字符串的后半部分复制到字符串数组的第二个元素中 (Copy the second half of the input string to the second element of the string array)
    snprintf(
      string_array->data[1], string_size - found_last - rhs_offset, "%s", str + found_last + 1);
  }

  return RCUTILS_RET_OK;

fail:
  // 如果失败，清理资源 (Clean up resources if failed)
  if (rcutils_string_array_fini(string_array) != RCUTILS_RET_OK) {
    RCUTILS_LOG_ERROR(
      "failed to clean up on error (leaking memory): '%s'", rcutils_get_error_string().str);
  }
  return result_error;
}

#ifdef __cplusplus
}
#endif
