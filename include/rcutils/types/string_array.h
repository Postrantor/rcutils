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

#ifndef RCUTILS__TYPES__STRING_ARRAY_H_
#define RCUTILS__TYPES__STRING_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/macros.h"
#include "rcutils/qsort.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

/// 结构体，存储字符串数组的元数据。
/// The structure holding the metadata for a string array.
typedef struct RCUTILS_PUBLIC_TYPE rcutils_string_array_s
{
  /// 字符串数组中可以存储的字符串数量。
  /// The number of strings that can be stored in the string array.
  size_t size;

  /// 为字符串数组分配的内存。
  /// The allocated memory for the string array.
  char ** data;

  /// 用于为字符串数组分配和释放内存的分配器。
  /// The allocator used to allocate and free memory for the string array.
  rcutils_allocator_t allocator;
} rcutils_string_array_t;

/// 返回一个空的字符串数组结构体。
/// Return an empty string array struct.
/**
 * 此函数返回一个空的并且初始化为零的字符串数组结构体。
 * 在任何未初始化的实例上调用 rcutils_string_array_fini() 将导致未定义的行为。
 * 每个 string_array_t 实例必须使用此函数进行零初始化或手动分配。
 * This function returns an empty and zero initialized string array struct.
 * Calling rcutils_string_array_fini() on any non-initialized instance leads
 * to undefined behavior.
 * Every instance of string_array_t has to either be zero_initialized with this
 * function or manually allocated.
 *
 * 示例：
 * Example:
 *
 * ```c
 * rcutils_string_array_t foo;
 * rcutils_string_array_fini(&foo); // 未定义行为！
 * rcutils_string_array_fini(&foo); // undefined behavior!
 *
 * rcutils_string_array_t bar = rcutils_get_zero_initialized_string_array();
 * rcutils_string_array_fini(&bar); // 正常
 * rcutils_string_array_fini(&bar); // ok
 * ```
 */
RCUTILS_PUBLIC
rcutils_string_array_t rcutils_get_zero_initialized_string_array(void);

/// 用给定大小初始化字符串数组。
/// Initialize a string array with a given size.
/**
 * 此函数将使用给定的大小初始化给定的、零初始化的字符串数组。
 * 注意，将字符串放入数组会将所有权交给数组。
 * This function will initialize a given, zero initialized, string array to
 * a given size.
 *
 * Note that putting a string into the array gives ownership to the array.
 *
 * 示例：
 * Example:
 *
 * ```c
 * rcutils_allocator_t allocator = rcutils_get_default_allocator();
 * rcutils_string_array_t string_array = rcutils_get_zero_initialized_string_array();
 * rcutils_ret_t ret = rcutils_string_array_init(&string_array, 2, &allocator);
 * if (ret != RCUTILS_RET_OK) {
 *   // ... 错误处理
 *   // ... error handling
 * }
 * string_array.data[0] = rcutils_strdup("Hello", &allocator);
 * string_array.data[1] = rcutils_strdup("World", &allocator);
 * ret = rcutils_string_array_fini(&string_array);
 * ```
 *
 * \param[inout] string_array 要初始化的对象
 * \param[inout] string_array object to be initialized
 * \param[in] size 数组应该具有的大小
 * \param[in] size the size the array should be
 * \param[in] allocator 用于分配和释放内存的分配器
 * \param[in] allocator to be used to allocate and deallocate memory
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效的参数，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation fails, or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_array_init(
  rcutils_string_array_t * string_array, size_t size, const rcutils_allocator_t * allocator);

/// 释放字符串数组，回收所有资源。(Finalize a string array, reclaiming all resources.)
/**
 * 此函数回收字符串数组拥有的任何内存，包括它引用的字符串。(This function reclaims any memory owned by the string array, including the strings it references.)
 *
 * 用于初始化字符串数组的分配器用于释放数组中的每个字符串和字符串数组本身。(The allocator used to initialize the string array is used to deallocate each string in the array and the array of strings itself.)
 *
 * \param[inout] string_array 要完成的对象 (object to be finalized)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数 (for invalid arguments), or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误 (if an unknown error occurs).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_array_fini(rcutils_string_array_t * string_array);

/// 比较两个字符串数组。(Compare two string arrays.)
/**
 * 根据字典顺序比较两个字符串数组。(The two string arrays are compared according to lexicographical order.)
 *
 * \param[in] lhs 第一个字符串数组 (The first string array.)
 * \param[in] rhs 第二个字符串数组 (The second string array.)
 * \param[out] res 如果`lhs`在字典顺序上出现在`rhs`之前，则为负值。 (Negative value if `lhs` appears before `rhs` in lexicographical order.)
 *   如果`lhs`和`rhs`相等，则为零。 (Zero if `lhs` and `rhs` are equal.)
 *   如果`lhs`在字典顺序上出现在`rhs`之后，则为正值。 (Positive value if `lhs` appears after `rhs` in lexicographical order.)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果任何参数为`NULL`, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果`lhs->data`或`rhs->data`为`NULL`, or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误 (if an unknown error occurs).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_array_cmp(
  const rcutils_string_array_t * lhs, const rcutils_string_array_t * rhs, int * res);

/// 调整字符串数组的大小，回收移除的资源。(Resize a string array, reclaiming removed resources.)
/**
 * 此函数更改现有字符串数组的大小。(This function changes the size of an existing string array.)
 * 如果新大小更大，则将新条目添加到数组的末尾，并将其初始化为零。(If the new size is larger, new entries are added to the end of the array and are zero-initialized.)
 * 如果新大小较小，则从数组的末尾删除条目并回收其资源。(If the new size is smaller, entries are removed from the end of the array and their resources reclaimed.)
 *
 * \par 注意：(Note:)
 * 将大小调整为0不是调用::rcutils_string_array_fini的替代方法。(Resizing to 0 is not a substitute for calling ::rcutils_string_array_fini.)
 *
 * \par 注意：(Note:)
 * 如果此函数失败，则\p string_array保持不变，仍应使用::rcutils_string_array_fini回收。(If this function fails, \p string_array remains unchanged and should still be reclaimed with ::rcutils_string_array_fini.)
 *
 * \param[inout] string_array 要调整大小的对象 (object to be resized.)
 * \param[in] new_size 数组应更改为的大小 (the size the array should be changed to.)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数 (for invalid arguments), or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败 (if memory allocation fails), or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误 (if an unknown error occurs).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_array_resize(rcutils_string_array_t * string_array, size_t new_size);

/// 字符串指针的字典序比较器 (Lexicographic comparer for pointers to string pointers).
/**
 * 这个函数按照字典序升序比较字符串指针的指针
 * (This function compares pointers to string pointers lexicographically ascending).
 *
 * \param[in] lhs 指向第一个字符串指针的指针 (pointer to the first string pointer).
 * \param[in] rhs 指向第二个字符串指针的指针 (pointer to the second string pointer).
 * \return 如果 lhs 在字典序上较小，返回 <0 (<0 if lhs is lexicographically lower, or)
 * \return 如果字符串相同，返回 0 (0 if the strings are the same, or)
 * \return 如果 lhs 在字典序上较大，返回 >0 (>0 if lhs is lexicographically higher).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_string_array_sort_compare(const void * lhs, const void * rhs);

/// 根据字典序对字符串数组进行排序 (Sort a string array according to lexicographical order).
/**
 * 这个函数会改变字符串数组中条目的顺序，使它们按照字典序升序排列
 * (This function changes the order of the entries in a string array so that they are in lexicographically ascending order).
 * 空条目将被放置在数组的末尾 (Empty entries are placed at the end of the array).
 *
 * \param[inout] string_array 需要排序的对象 (object whose elements should be sorted).
 * \return 如果成功，返回 #RCUTILS_RET_OK (#RCUTILS_RET_OK if successful, or)
 * \return 对于无效的参数，返回 #RCUTILS_RET_INVALID_ARGUMENT (#RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or)
 * \return 如果发生未知错误，返回 #RCUTILS_RET_ERROR (#RCUTILS_RET_ERROR if an unknown error occurs).
 */
inline RCUTILS_WARN_UNUSED rcutils_ret_t
rcutils_string_array_sort(rcutils_string_array_t * string_array)
{
  // 检查 string_array 是否为空，如果为空则返回 RCUTILS_RET_INVALID_ARGUMENT 错误信息
  // (Check if string_array is null, if it is, return RCUTILS_RET_INVALID_ARGUMENT error message)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_array, "string_array is null", return RCUTILS_RET_INVALID_ARGUMENT);

  // 调用 rcutils_qsort 函数对字符串数组进行排序，并返回排序结果
  // (Call the rcutils_qsort function to sort the string array and return the sorting result)
  return rcutils_qsort(
    string_array->data, string_array->size, sizeof(string_array->data[0]),
    rcutils_string_array_sort_compare);
}

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__TYPES__STRING_ARRAY_H_
