// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__TYPES__UINT8_ARRAY_H_
#define RCUTILS__TYPES__UINT8_ARRAY_H_

#if __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "rcutils/allocator.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

/// 用于保存 uint8 数组元数据的结构体。
/// The structure holding the metadata for a uint8 array.
typedef struct RCUTILS_PUBLIC_TYPE rcutils_uint8_array_s
{
  /// 分配给 uint8 数组的内存。
  /// The allocated memory for the uint8 array.
  uint8_t * buffer;

  /// uint8 数组中有效元素的数量。
  /// The number of valid elements in the uint8 array.
  size_t buffer_length;

  /// uint8 数组的最大容量。
  /// The maximum capacity of the uint8 array.
  size_t buffer_capacity;

  /// 用于为 uint8 数组分配和释放内存的分配器。
  /// The allocator used to allocate and free memory for the uint8 array.
  rcutils_allocator_t allocator;
} rcutils_uint8_array_t;

/// 返回一个零初始化的 uint8 数组结构体。
/// Return a zero initialized uint8 array struct.
/**
 * \return rcutils_uint8_array_t 一个零初始化的 uint8 数组结构体
 * \return rcutils_uint8_array_t a zero initialized uint8 array struct
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_uint8_array_t rcutils_get_zero_initialized_uint8_array(void);

/// 初始化一个零初始化的 uint8 数组结构体。
/// Initialize a zero initialized uint8 array struct.
/**
 * 如果 uint8 数组结构体已经初始化，此函数可能会泄漏。
 * This function may leak if the uint8 array struct is already initialized.
 * 如果容量设置为 0，则不分配内存，内部缓冲区仍然为空。
 * If the capacity is set to 0, no memory is allocated and the internal buffer
 * is still NULL.
 *
 * \param[inout] uint8_array 指向要初始化的 uint8 数组结构体的指针
 * \param[inout] uint8_array a pointer to the to be initialized uint8 array struct
 * \param[in] buffer_capacity 为字节流分配内存的大小
 * \param[in] buffer_capacity the size of the memory to allocate for the byte stream
 * \param[in] allocator 用于内存分配的分配器
 * \param[in] allocator the allocator to use for the memory allocation
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果任何参数无效，或者
 * \return #RCUTILS_RET_INVALID_ARGUMENT if any arguments are invalid, or
 * \return 'RCUTILS_RET_BAD_ALLOC` 如果无法正确分配内存
 * \return 'RCUTILS_RET_BAD_ALLOC` if no memory could be allocated correctly
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 * \return #RCUTILS_RET_ERROR if an unexpected error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_uint8_array_init(
  rcutils_uint8_array_t * uint8_array,
  size_t buffer_capacity,
  const rcutils_allocator_t * allocator);

/// 结束一个 uint8 数组结构体。
/// Finalize a uint8 array struct.
/**
 * 清理并释放 rcutils_uint8_array_t 中使用的任何资源。
 * Cleans up and deallocates any resources used in a rcutils_uint8_array_t.
 * 传递给此函数的数组需要由 rcutils_uint8_array_init() 初始化。
 * The array passed to this function needs to have been initialized by
 * rcutils_uint8_array_init().
 * 将未初始化的实例传递给此函数会导致未定义的行为。
 * Passing an uninitialized instance to this function leads to undefined
 * behavior.
 *
 * \param[in] uint8_array 指向要清理的 rcutils_uint8_array_t 的指针
 * \param[in] uint8_array pointer to the rcutils_uint8_array_t to be cleaned up
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果 uint8_array 参数无效
 * \return #RCUTILS_RET_INVALID_ARGUMENT if the uint8_array argument is invalid
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 * \return #RCUTILS_RET_ERROR if an unexpected error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_uint8_array_fini(rcutils_uint8_array_t * uint8_array);

/// 调整 uint8 数组的内部缓冲区大小。
/// Resize the internal buffer of the uint8 array.
/**
 * 如有必要，可以动态调整 uint8 数组的内部缓冲区大小。
 * The internal buffer of the uint8 array can be resized dynamically if needed.
 * 如果新大小小于当前容量，则内存被截断。
 * If the new size is smaller than the current capacity, then the memory is
 * truncated.
 * 请注意，这可能会释放内存，因此会使指向该存储的任何指针无效。
 * Be aware, that this might deallocate the memory and therefore invalidates any
 * pointers to this storage.
 *
 * \param[inout] uint8_array 指向正在调整大小的 rcutils_uint8_array_t 实例的指针
 * \param[inout] uint8_array pointer to the instance of rcutils_uint8_array_t which is
 * being resized
 * \param[in] new_size 内部缓冲区的新大小
 * \param[in] new_size the new size of the internal buffer
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果 new_size 设置为零
 * \return #RCUTILS_RET_INVALID_ARGUMENT if new_size is set to zero
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或者
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation failed, or
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 * \return #RCUTILS_RET_ERROR if an unexpected error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_uint8_array_resize(rcutils_uint8_array_t * uint8_array, size_t new_size);

#if __cplusplus
}
#endif

#endif // RCUTILS__TYPES__UINT8_ARRAY_H_
