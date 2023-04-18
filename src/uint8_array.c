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

#include "rcutils/types/uint8_array.h"
#include "rcutils/error_handling.h"

/**
 * @brief 获取一个零初始化的 uint8 数组（Get a zero-initialized uint8 array）
 *
 * @return 返回一个零初始化的 rcutils_uint8_array_t 结构体（Return a zero-initialized rcutils_uint8_array_t structure）
 */
rcutils_uint8_array_t rcutils_get_zero_initialized_uint8_array(void)
{
  // 静态变量，用于存储零初始化的数组（Static variable to store the zero-initialized array）
  static rcutils_uint8_array_t uint8_array = {
    .buffer = NULL, .buffer_length = 0lu, .buffer_capacity = 0lu};

  // 设置分配器为零初始化的分配器（Set the allocator to a zero-initialized allocator）
  uint8_array.allocator = rcutils_get_zero_initialized_allocator();

  // 返回零初始化的数组（Return the zero-initialized array）
  return uint8_array;
}

/**
 * @brief 初始化一个 uint8 数组（Initialize a uint8 array）
 *
 * @param uint8_array 指向 rcutils_uint8_array_t 结构体的指针（Pointer to a rcutils_uint8_array_t structure）
 * @param buffer_capacity 缓冲区容量（Buffer capacity）
 * @param allocator 指向 rcutils_allocator_t 结构体的指针（Pointer to a rcutils_allocator_t structure）
 * @return 返回 rcutils_ret_t 类型的状态（Return a status of type rcutils_ret_t）
 */
rcutils_ret_t rcutils_uint8_array_init(
  rcutils_uint8_array_t * uint8_array,
  size_t buffer_capacity,
  const rcutils_allocator_t * allocator)
{
  // 检查参数是否为空（Check if the argument is null）
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(uint8_array, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查分配器是否有效（Check if the allocator is valid）
  RCUTILS_CHECK_ALLOCATOR(allocator, return RCUTILS_RET_INVALID_ARGUMENT);

  // 设置缓冲区长度为 0（Set buffer length to 0）
  uint8_array->buffer_length = 0lu;

  // 设置缓冲区容量（Set buffer capacity）
  uint8_array->buffer_capacity = buffer_capacity;

  // 设置分配器（Set the allocator）
  uint8_array->allocator = *allocator;

  // 如果缓冲区容量大于 0（If buffer capacity is greater than 0）
  if (buffer_capacity > 0lu) {
    // 分配内存（Allocate memory）
    uint8_array->buffer =
      (uint8_t *)allocator->allocate(buffer_capacity * sizeof(uint8_t), allocator->state);

    // 检查分配的内存是否为空（Check if the allocated memory is null）
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(uint8_array->buffer,
                                    "failed to allocate memory for uint8 array",
                                    uint8_array->buffer_capacity = 0lu;
                                    uint8_array->buffer_length = 0lu; return RCUTILS_RET_BAD_ALLOC);
  }

  // 返回成功状态（Return success status）
  return RCUTILS_RET_OK;
}

/**
 * @brief 释放一个 uint8 数组的内存 (Free the memory of a uint8 array)
 *
 * @param uint8_array 指向要释放的 uint8 数组的指针 (Pointer to the uint8 array to be freed)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_uint8_array_fini(rcutils_uint8_array_t * uint8_array)
{
  // 检查输入参数是否为空 (Check if the input argument is null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(uint8_array, RCUTILS_RET_INVALID_ARGUMENT);

  // 获取分配器指针 (Get allocator pointer)
  rcutils_allocator_t * allocator = &uint8_array->allocator;
  // 检查分配器是否有效 (Check if the allocator is valid)
  RCUTILS_CHECK_ALLOCATOR(allocator, return RCUTILS_RET_INVALID_ARGUMENT);

  // 使用分配器释放缓冲区内存 (Deallocate buffer memory using allocator)
  allocator->deallocate(uint8_array->buffer, allocator->state);
  // 将缓冲区指针设置为 NULL (Set buffer pointer to NULL)
  uint8_array->buffer = NULL;
  // 将缓冲区长度设置为 0 (Set buffer length to 0)
  uint8_array->buffer_length = 0lu;
  // 将缓冲区容量设置为 0 (Set buffer capacity to 0)
  uint8_array->buffer_capacity = 0lu;

  // 返回操作成功 (Return operation success)
  return RCUTILS_RET_OK;
}

/**
 * @brief 调整 uint8 数组的大小 (Resize a uint8 array)
 *
 * @param uint8_array 指向要调整大小的 uint8 数组的指针 (Pointer to the uint8 array to be resized)
 * @param new_size 新的缓冲区大小 (New buffer size)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_uint8_array_resize(rcutils_uint8_array_t * uint8_array, size_t new_size)
{
  // 检查输入参数是否为空 (Check if the input argument is null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(uint8_array, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查新大小是否大于零 (Check if the new size is greater than zero)
  if (0lu == new_size) {
    RCUTILS_SET_ERROR_MSG("new size of uint8_array has to be greater than zero");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 获取分配器指针 (Get allocator pointer)
  rcutils_allocator_t * allocator = &uint8_array->allocator;
  // 检查分配器是否有效 (Check if the allocator is valid)
  RCUTILS_CHECK_ALLOCATOR(allocator, return RCUTILS_RET_INVALID_ARGUMENT);

  // 如果新大小等于当前容量，则无需执行任何操作 (If the new size is equal to the current capacity, there's nothing to do)
  if (new_size == uint8_array->buffer_capacity) {
    // nothing to do here
    return RCUTILS_RET_OK;
  }

  // 使用分配器重新分配内存 (Reallocate memory using allocator)
  uint8_array->buffer =
    rcutils_reallocf(uint8_array->buffer, new_size * sizeof(uint8_t), allocator);
  // 检查重新分配的内存是否为 NULL (Check if the reallocated memory is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(uint8_array->buffer,
                                  "failed to reallocate memory for uint8 array",
                                  uint8_array->buffer_capacity = 0lu;
                                  uint8_array->buffer_length = 0lu; return RCUTILS_RET_BAD_ALLOC);

  // 更新缓冲区容量 (Update buffer capacity)
  uint8_array->buffer_capacity = new_size;
  // 如果新大小小于当前长度，则更新缓冲区长度 (If the new size is less than the current length, update buffer length)
  if (new_size < uint8_array->buffer_length) {
    uint8_array->buffer_length = new_size;
  }

  // 返回操作成功 (Return operation success)
  return RCUTILS_RET_OK;
}
