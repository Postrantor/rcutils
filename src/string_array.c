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

#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/types/string_array.h"

/**
 * @brief 获取一个零初始化的字符串数组 (Get a zero-initialized string array)
 *
 * @return 返回一个零初始化的字符串数组 (Return a zero-initialized string array)
 */
rcutils_string_array_t rcutils_get_zero_initialized_string_array(void)
{
  // 定义一个静态的零初始化的字符串数组结构体 (Define a static zero-initialized string array structure)
  static rcutils_string_array_t array = {
    .size = 0,
    .data = NULL,
  };
  // 使用零初始化的内存分配器初始化数组的内存分配器 (Initialize the array's allocator with a zero-initialized allocator)
  array.allocator = rcutils_get_zero_initialized_allocator();
  // 返回零初始化的字符串数组 (Return the zero-initialized string array)
  return array;
}

/**
 * @brief 初始化字符串数组 (Initialize a string array)
 *
 * @param[out] string_array 要初始化的字符串数组指针 (Pointer to the string array to be initialized)
 * @param[in] size 字符串数组的大小 (Size of the string array)
 * @param[in] allocator 分配器 (Allocator)
 * @return 返回操作结果状态 (Return the operation result status)
 */
rcutils_ret_t rcutils_string_array_init(
  rcutils_string_array_t * string_array, size_t size, const rcutils_allocator_t * allocator)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCUTILS_RET_BAD_ALLOC);

  // 检查分配器是否为空 (Check if allocator is null)
  if (NULL == allocator) {
    RCUTILS_SET_ERROR_MSG("allocator is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  // 检查字符串数组是否为空 (Check if string_array is null)
  if (NULL == string_array) {
    RCUTILS_SET_ERROR_MSG("string_array is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  // 设置字符串数组的大小 (Set the size of the string array)
  string_array->size = size;
  // 使用分配器为字符串数组数据分配内存 (Allocate memory for the string array data using the allocator)
  string_array->data = allocator->zero_allocate(size, sizeof(char *), allocator->state);
  // 检查分配的内存是否为空，且大小不为零 (Check if the allocated memory is null and the size is not zero)
  if (NULL == string_array->data && 0 != size) {
    RCUTILS_SET_ERROR_MSG("failed to allocate string array");
    return RCUTILS_RET_BAD_ALLOC;
  }
  // 设置字符串数组的分配器 (Set the allocator of the string array)
  string_array->allocator = *allocator;
  // 返回操作成功状态 (Return the operation success status)
  return RCUTILS_RET_OK;
}

/**
 * @brief 终止字符串数组，释放资源 (Finalize a string array, release resources)
 *
 * @param[in,out] string_array 要终止的字符串数组指针 (Pointer to the string array to be finalized)
 * @return 返回操作结果状态 (Return the operation result status)
 */
rcutils_ret_t rcutils_string_array_fini(rcutils_string_array_t * string_array)
{
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCUTILS_RET_INVALID_ARGUMENT);

  // 检查字符串数组是否为空 (Check if string_array is null)
  if (NULL == string_array) {
    RCUTILS_SET_ERROR_MSG("string_array is null");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 检查字符串数组数据是否为空 (Check if string_array's data is null)
  if (NULL == string_array->data) {
    return RCUTILS_RET_OK;
  }

  // 获取字符串数组的分配器 (Get the allocator of the string array)
  rcutils_allocator_t * allocator = &string_array->allocator;
  // 检查分配器是否有效 (Check if the allocator is valid)
  if (!rcutils_allocator_is_valid(allocator)) {
    RCUTILS_SET_ERROR_MSG("allocator is invalid");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  // 遍历字符串数组，释放每个字符串占用的内存 (Iterate through the string array, releasing the memory occupied by each string)
  size_t i;
  for (i = 0; i < string_array->size; ++i) {
    allocator->deallocate(string_array->data[i], allocator->state);
    string_array->data[i] = NULL;
  }
  // 释放字符串数组数据占用的内存 (Release the memory occupied by the string array data)
  allocator->deallocate(string_array->data, allocator->state);
  // 将字符串数组数据设置为空 (Set the string array data to null)
  string_array->data = NULL;
  // 将字符串数组大小设置为零 (Set the string array size to zero)
  string_array->size = 0;

  // 返回操作成功状态 (Return the operation success status)
  return RCUTILS_RET_OK;
}

/**
 * @brief 比较两个字符串数组 (Compare two string arrays)
 *
 * @param[in] lhs 第一个字符串数组 (The first string array)
 * @param[in] rhs 第二个字符串数组 (The second string array)
 * @param[out] res 比较结果，负数表示lhs<rhs，正数表示lhs>rhs，0表示相等 (Comparison result, negative if lhs<rhs, positive if lhs>rhs, 0 if equal)
 * @return rcutils_ret_t 返回比较结果的状态 (Return the status of the comparison result)
 */
rcutils_ret_t rcutils_string_array_cmp(
  const rcutils_string_array_t * lhs, const rcutils_string_array_t * rhs, int * res)
{
  // 检查 lhs 是否为空 (Check if lhs is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    lhs, "lhs string array is null", return RCUTILS_RET_INVALID_ARGUMENT);
  // 检查 rhs 是否为空 (Check if rhs is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    rhs, "rhs string array is null", return RCUTILS_RET_INVALID_ARGUMENT);
  // 检查 res 是否为空 (Check if res is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(res, "res argument is null", return RCUTILS_RET_INVALID_ARGUMENT);

  // 获取最小数组大小 (Get the smallest array size)
  size_t smallest_size = lhs->size;
  if (rhs->size < smallest_size) {
    smallest_size = rhs->size;
  }

  // 如果最小数组大小大于0，检查数据是否为空 (If the smallest array size is greater than 0, check if the data is null)
  if (smallest_size > 0) {
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(
      lhs->data, "lhs->data is null", return RCUTILS_RET_INVALID_ARGUMENT);
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(
      rhs->data, "rhs->data is null", return RCUTILS_RET_INVALID_ARGUMENT);
  }

  // 遍历两个数组并比较每个字符串 (Iterate through both arrays and compare each string)
  for (size_t i = 0; i < smallest_size; ++i) {
    // 检查 lhs 数组元素是否为空 (Check if lhs array element is null)
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(
      lhs->data[i], "lhs array element is null", return RCUTILS_RET_ERROR);
    // 检查 rhs 数组元素是否为空 (Check if rhs array element is null)
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(
      rhs->data[i], "rhs array element is null", return RCUTILS_RET_ERROR);
    // 循环直到找到一对不相等的字符串 (Loop until we find a pair of strings that are not equal)
    int strcmp_res = strcmp(lhs->data[i], rhs->data[i]);
    if (0 != strcmp_res) {
      *res = strcmp_res;
      return RCUTILS_RET_OK;
    }
  }

  // 如果所有字符串相等，比较数组大小 (If all strings are equal, compare array sizes)
  *res = 0;
  if (lhs->size < rhs->size) {
    *res = -1;
  } else if (lhs->size > rhs->size) {
    *res = 1;
  }
  return RCUTILS_RET_OK;
}

/**
 * @brief 调整字符串数组的大小 (Resize the size of a string array)
 *
 * @param[in,out] string_array 要调整大小的字符串数组指针 (Pointer to the string array to be resized)
 * @param[in] new_size 新的大小 (New size)
 * @return rcutils_ret_t 返回操作结果，成功返回RCUTILS_RET_OK (Return operation result, success returns RCUTILS_RET_OK)
 */
rcutils_ret_t rcutils_string_array_resize(rcutils_string_array_t * string_array, size_t new_size)
{
  // 检查 string_array 是否为空 (Check if string_array is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_array, "string_array is null", return RCUTILS_RET_INVALID_ARGUMENT);

  // 如果新大小与当前大小相同，则无需调整 (If the new size is the same as the current size, no adjustment is needed)
  if (string_array->size == new_size) {
    return RCUTILS_RET_OK;
  }

  // 获取分配器 (Get the allocator)
  rcutils_allocator_t * allocator = &string_array->allocator;
  // 检查分配器是否有效 (Check if the allocator is valid)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    allocator, "allocator is invalid", return RCUTILS_RET_INVALID_ARGUMENT);

  // 保存要删除的条目 (Stash entries being removed)
  rcutils_string_array_t to_reclaim = rcutils_get_zero_initialized_string_array();
  // 如果新大小小于当前大小 (If the new size is smaller than the current size)
  if (new_size < string_array->size) {
    size_t num_removed = string_array->size - new_size;
    // 初始化要回收的字符串数组 (Initialize the string array to be reclaimed)
    rcutils_ret_t ret = rcutils_string_array_init(&to_reclaim, num_removed, allocator);
    if (RCUTILS_RET_OK != ret) {
      // rcutils_string_array_init 应已设置错误消息 (rcutils_string_array_init should have already set an error message)
      return ret;
    }
    // 复制要删除的数据到待回收数组 (Copy the data to be deleted to the array to be reclaimed)
    memcpy(to_reclaim.data, &string_array->data[new_size], to_reclaim.size * sizeof(char *));
  }

  // 重新分配新大小的内存 (Reallocate memory for the new size)
  char ** new_data =
    allocator->reallocate(string_array->data, new_size * sizeof(char *), allocator->state);
  // 如果分配失败且新大小非零 (If allocation fails and the new size is not zero)
  if (NULL == new_data && 0 != new_size) {
    RCUTILS_SET_ERROR_MSG("failed to allocate string array");
    // 清空待回收数组中的数据 (Empty the data in the array to be reclaimed)
    for (size_t i = 0; i < to_reclaim.size; ++i) {
      to_reclaim.data[i] = NULL;
    }
    // 结束待回收数组 (Finalize the array to be reclaimed)
    rcutils_ret_t ret = rcutils_string_array_fini(&to_reclaim);
    if (RCUTILS_RET_OK != ret) {
      RCUTILS_SET_ERROR_MSG("memory was leaked during error handling");
    }
    return RCUTILS_RET_BAD_ALLOC;
  }
  // 更新 string_array 的 data 指针 (Update the data pointer of string_array)
  string_array->data = new_data;

  // 零初始化新条目 (Zero-initialize new entries)
  for (size_t i = string_array->size; i < new_size; ++i) {
    string_array->data[i] = NULL;
  }

  // 更新字符串数组的大小 (Update the size of the string array)
  string_array->size = new_size;

  // 最后，回收已删除的条目 (Lastly, reclaim removed entries)
  return rcutils_string_array_fini(&to_reclaim);
}

/**
 * @brief 比较两个字符串的大小，用于排序函数。
 *        Compare two strings to determine their order, for use in a sorting function.
 *
 * @param lhs 左侧字符串指针的指针。A pointer to the left-hand side string pointer.
 * @param rhs 右侧字符串指针的指针。A pointer to the right-hand side string pointer.
 * @return 返回比较结果。Return the comparison result.
 */
int rcutils_string_array_sort_compare(const void * lhs, const void * rhs)
{
  // 将左侧传入参数转换为字符串指针。Convert the left-hand side input parameter to a string pointer.
  // 将右侧传入参数转换为字符串指针。Convert the right-hand side input parameter to a string pointer.
  const char * left = *(const char **)lhs;
  const char * right = *(const char **)rhs;

  // 如果左侧字符串为空。If the left-hand side string is null.
  if (NULL == left) {
    // 如果右侧字符串也为空，则它们相等，返回0。If the right-hand side string is also null, they are equal, return 0.
    // 否则，左侧字符串较小，返回1。Otherwise, the left-hand side string is smaller, return 1.
    return NULL == right ? 0 : 1;
  }
  // 如果右侧字符串为空。If the right-hand side string is null.
  else if (NULL == right) {
    // 则左侧字符串较大，返回-1。Then the left-hand side string is larger, return -1.
    return -1;
  }

  // 比较两个字符串，并返回比较结果。Compare the two strings and return the comparison result.
  return strcmp(left, right);
}

#ifdef __cplusplus
}
#endif
