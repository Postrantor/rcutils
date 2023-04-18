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

#ifndef RCUTILS__QSORT_H_
#define RCUTILS__QSORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

/// Interface to qsort with rcutils-style argument validation.
/**
 * 这个函数根据给定的比较函数，改变数组中元素的顺序，使它们按升序排列。
 * This function changes the order of the elements in the array so that they
 * are in ascending order according to the given comparison function.
 *
 * 该函数是线程安全的。
 * This function is thread-safe.
 *
 * \param[inout] ptr 需要排序的对象。
 * \param[inout] ptr object whose elements should be sorted.
 * \param[in] count 对象中存在的元素数量。
 * \param[in] count number of elements present in the object.
 * \param[in] size 每个元素的大小，以字节为单位。
 * \param[in] size size of each element, in bytes.
 * \param[in] comp 用于比较两个元素的函数。
 * \param[in] comp function used to compare two elements.
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效的参数，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_qsort(void * ptr, size_t count, size_t size, int (*comp)(const void *, const void *));

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__QSORT_H_
