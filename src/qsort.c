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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "rcutils/error_handling.h"
#include "rcutils/qsort.h"

/**
 * @brief 对给定数组进行排序 (Sort the given array)
 *
 * @param[in] ptr 指向要排序的数组的指针 (Pointer to the array to be sorted)
 * @param[in] count 数组中元素的数量 (Number of elements in the array)
 * @param[in] size 每个元素的大小 (Size of each element)
 * @param[in] comp 用于比较两个元素的函数指针 (Function pointer for comparing two elements)
 * @return rcutils_ret_t 返回排序操作的结果 (Return the result of the sorting operation)
 */
rcutils_ret_t
rcutils_qsort(void * ptr, size_t count, size_t size, int (*comp)(const void *, const void *))
{
  // 检查 comp 是否为空，如果为空，则返回 RCUTILS_RET_INVALID_ARGUMENT (Check if comp is null, if it is, return RCUTILS_RET_INVALID_ARGUMENT)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(comp, "comp is null", return RCUTILS_RET_INVALID_ARGUMENT);

  // 如果数组中的元素数量小于等于1，则不需要排序，直接返回 RCUTILS_RET_OK (If the number of elements in the array is less than or equal to 1, no need to sort, return RCUTILS_RET_OK directly)
  if (1 >= count) {
    return RCUTILS_RET_OK;
  }

  // 检查 ptr 是否为空，如果为空，则返回 RCUTILS_RET_INVALID_ARGUMENT (Check if ptr is null, if it is, return RCUTILS_RET_INVALID_ARGUMENT)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(ptr, "ptr is null", return RCUTILS_RET_INVALID_ARGUMENT);

  // 调用 C 标准库的 qsort 函数对数组进行排序 (Call the qsort function from the C standard library to sort the array)
  qsort(ptr, count, size, comp);

  // 排序完成后，返回 RCUTILS_RET_OK (After sorting is done, return RCUTILS_RET_OK)
  return RCUTILS_RET_OK;
}

#ifdef __cplusplus
}
#endif
