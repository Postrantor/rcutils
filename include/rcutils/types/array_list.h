// Copyright 2018-2019 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__TYPES__ARRAY_LIST_H_
#define RCUTILS__TYPES__ARRAY_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

struct rcutils_array_list_impl_s;

/// 结构体，用于保存数组列表的元数据。
/// The structure holding the metadata for an array list.
typedef struct RCUTILS_PUBLIC_TYPE rcutils_array_list_s
{
  /// 指向PIMPL实现类型的指针。
  /// A pointer to the PIMPL implementation type.
  struct rcutils_array_list_impl_s * impl;
} rcutils_array_list_t;

/**
 * 验证 rcutils_array_list_t* 是否指向一个有效的数组列表。
 * Validates that an rcutils_array_list_t* points to a valid array list.
 * \param[in] array_list 指向 rcutils_array_list_t 的指针
 * \param[in] array_list A pointer to an rcutils_array_list_t
 * \return 若 array_list 为空，则返回 RCUTILS_RET_INVALID_ARGUMENT
 * \return RCUTILS_RET_INVALID_ARGUMENT if array_list is null
 * \return 若 array_list 未初始化，则返回 RCUTILS_RET_NOT_INITIALIZED
 * \return RCUTILS_RET_NOT_INITIALIZED if array_list is not initialized
 */
#define ARRAY_LIST_VALIDATE_ARRAY_LIST(array_list)                                                 \
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(array_list, RCUTILS_RET_INVALID_ARGUMENT);                       \
  if (NULL == array_list->impl) {                                                                  \
    RCUTILS_SET_ERROR_MSG("array_list is not initialized");                                        \
    return RCUTILS_RET_NOT_INITIALIZED;                                                            \
  }

/// 返回一个空的 array_list 结构体。
/// Return an empty array_list struct.
/**
 * 此函数返回一个空的，零初始化的 array_list 结构体。
 * This function returns an empty and zero initialized array_list struct.
 * 在任何未初始化的实例上调用 rcutils_array_list_fini() 将导致未定义的行为。
 * Calling rcutils_array_list_fini() on any non-initialized instance leads
 * to undefined behavior.
 * array_list_t 的每个实例必须使用此函数进行零初始化，或手动分配。
 * Every instance of array_list_t has to either be zero_initialized with this
 * function or manually allocated.
 *
 * <hr>
 * 属性                | 遵循
 * ------------------ | -------------
 * 分配内存            | 否
 * 线程安全            | 是
 * 使用原子操作        | 否
 * 无锁                | 是
 *
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * 示例：
 * Example:
 *
 * ```c
 * rcutils_array_list_t foo;
 * rcutils_array_list_fini(&foo); // 未定义行为！
 * rcutils_array_list_fini(&foo); // undefined behavior!
 *
 * rcutils_array_list_t bar = rcutils_get_zero_initialized_array_list();
 * rcutils_array_list_fini(&bar); // 正常
 * rcutils_array_list_fini(&bar); // ok
 * ```
 */
RCUTILS_PUBLIC
rcutils_array_list_t rcutils_get_zero_initialized_array_list(void);

/// 初始化具有给定初始容量的数组列表。
/**
 * 此函数将给定的、零初始化的 array_list 初始化为给定大小。
 *
 * <hr>
 * 属性              | 遵循
 * ------------------ | -------------
 * 分配内存           | 是
 * 线程安全           | 否
 * 使用原子操作       | 否
 * 无锁               | 是
 *
 * 示例：
 *
 * ```c
 * rcutils_allocator_t allocator = rcutils_get_default_allocator();
 * rcutils_array_list_t array_list = rcutils_get_zero_initialized_array_list();
 * rcutils_ret_t ret = rcutils_array_list_init(&array_list, 2, sizeof(int), &allocator);
 * if (ret != RCUTILS_RET_OK) {
 *   // ... 错误处理
 * }
 * int data = 42;
 * int out_data = 0;
 * ret = rcutils_array_list_add(&array_list, &data);
 * data++;
 * ret = rcutils_array_list_get(&array_list, 0, &out_data);
 * assert(42 == out_data);
 * ret = rcutils_array_list_fini(&array_list);
 * ```
 *
 * \param[inout] array_list 要初始化的对象
 * \param[in] initial_capacity 列表中要分配的初始容量
 * \param[in] data_size 列表中存储的数据对象的大小（以字节为单位）
 * \param[in] allocator 用于分配和释放内存的分配器
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或者
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或者
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_array_list_init(
  rcutils_array_list_t * array_list,
  size_t initial_capacity,
  size_t data_size,
  const rcutils_allocator_t * allocator);

/// Finalize an array list, reclaiming all resources.
/**
 * 此函数回收数组列表拥有的任何内存。
 *
 * 用于初始化数组列表的分配器用于释放列表中的每个条目和列表本身。
 *
 * <hr>
 * 属性              | 遵循
 * ------------------ | -------------
 * 分配内存           | 否
 * 线程安全           | 否
 * 使用原子操作       | 否
 * 无锁               | 是
 *
 * \param[inout] array_list 要完成的对象
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或者
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_array_list_fini(rcutils_array_list_t * array_list);

/// Adds an entry to the list
/**
 * 此函数将提供的数据添加到列表的末尾。在列表中存储所提供数据的浅拷贝，而不是仅存储指向所提供数据的指针。
 *
 * <hr>
 * 属性              | 遵循
 * ------------------ | -------------
 * 分配内存           | 是
 * 线程安全           | 否
 * 使用原子操作       | 否
 * 无锁               | 是
 *
 * \param[in] array_list 要添加数据的列表
 * \param[in] data 要添加到列表中的数据的指针
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或者
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或者
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_array_list_add(rcutils_array_list_t * array_list, const void * data);

/// 设置列表中的条目为提供的数据 (Sets an entry in the list to the provided data)
/**
 * 此函数在列表中指定的索引处设置提供的数据。 (This function sets the provided data at the specified index in the list.)
 * 为了在列表中存储，会对提供的数据进行浅拷贝，而不是仅存储指向提供数据的指针。 (A shallow copy of the provided data is made to store in the list instead of just storing the pointer to the provided data.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] array_list 要添加数据的数组列表 (to add the data to)
 * \param[in] index 列表中要设置数据的位置 (the position in the list to set the data)
 * \param[in] data 将在列表中设置的数据的指针 (a pointer to the data that will be set in the list)
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT if index out of bounds, or
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_array_list_set(rcutils_array_list_t * array_list, size_t index, const void * data);

/// 删除提供索引处的列表条目 (Removes an entry in the list at the provided index)
/**
 * 此函数从指定索引处的列表中删除数据。删除条目时，列表的容量永远不会减少。 (This function removes data from the list at the specified index. The capacity of the list will never decrease when entries are removed.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] array_list 要添加数据的数组列表 (to add the data to)
 * \param[in] index 要从列表中删除的项目的索引 (the index of the item to remove from the list)
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT if index out of bounds, or
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_array_list_remove(rcutils_array_list_t * array_list, size_t index);

/// 获取提供索引处的列表条目 (Retrieves an entry in the list at the provided index)
/**
 * 此函数检索提供的索引处存储在列表中的数据的副本。 (This function retrieves a copy of the data stored in the list at the provided index.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 *
 * \param[in] array_list 要添加数据的数组列表 (to add the data to)
 * \param[in] index 要获取数据的索引 (the index at which to get the data)
 * \param[out] data 存储在列表中的数据的副本 (a copy of the data stored in the list)
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_array_list_get(const rcutils_array_list_t * array_list, size_t index, void * data);

/// 获取提供的 array_list 的大小 (Retrieves the size of the provided array_list)
/**
 * 此函数检索提供的数组列表中的项目数量。 (This function retrieves the number of items in the provided array list)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 *
 * \param[in] array_list 要获取大小的列表 (list to get the size of)
 * \param[out] size 当前存储在列表中的项目数 (The number of items currently stored in the list)
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_array_list_get_size(const rcutils_array_list_t * array_list, size_t * size);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__TYPES__ARRAY_LIST_H_
