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

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/macros.h"
#include "rcutils/types/array_list.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

/**
 * @brief 检查数组列表中的索引是否越界 (Check if the index is out of bounds in the array list)
 * 
 * @param[in] array_list 要检查的数组列表 (The array list to check)
 * @param[in] index 要检查的索引 (The index to check)
 * 
 * 如果索引超出数组列表的范围，将设置错误消息并返回 RCUTILS_RET_INVALID_ARGUMENT。
 * (If the index is out of the range of the array list, it sets an error message and returns RCUTILS_RET_INVALID_ARGUMENT.)
 */
#define ARRAY_LIST_VALIDATE_INDEX_IN_BOUNDS(array_list, index)                                     \
  if (array_list->impl->size <= index) {                                                           \
    /* 索引超出列表范围的错误消息 (Error message when index is out of list bounds) */ \
    RCUTILS_SET_ERROR_MSG("index is out of bounds of the list");                                   \
    /* 返回无效参数错误代码 (Return invalid argument error code) */                      \
    return RCUTILS_RET_INVALID_ARGUMENT;                                                           \
  }

/**
 * @brief rcutils_array_list_impl_s 结构体定义 (Definition of the rcutils_array_list_impl_s structure)
 */
typedef struct rcutils_array_list_impl_s
{
  size_t size;     /*!< 数组列表的当前大小 (Current size of the array list) */
  size_t capacity; /*!< 数组列表的容量 (Capacity of the array list) */
  void * list; /*!< 存储数组列表元素的指针 (Pointer to store the array list elements) */
  size_t data_size; /*!< 单个数据元素的大小 (Size of a single data element) */
  rcutils_allocator_t
    allocator; /*!< 分配器，用于分配和释放内存 (Allocator for allocating and deallocating memory) */
} rcutils_array_list_impl_t;

/**
 * @brief 获取一个初始化为零的数组列表 (Get an array list initialized to zero)
 * 
 * @return rcutils_array_list_t 返回初始化为零的数组列表结构体 (Return an array list structure initialized to zero)
 */
rcutils_array_list_t rcutils_get_zero_initialized_array_list(void)
{
  /* 定义并初始化一个静态的空数组列表 (Define and initialize a static empty array list) */
  static rcutils_array_list_t zero_initialized_array_list = {NULL};
  /* 返回已初始化的空数组列表 (Return the initialized empty array list) */
  return zero_initialized_array_list;
}

/**
 * @brief 初始化数组列表
 * @param array_list 指向要初始化的数组列表的指针
 * @param initial_capacity 数组列表的初始容量
 * @param data_size 数据元素的大小（以字节为单位）
 * @param allocator 用于分配和释放内存的分配器
 * @return 返回 rcutils_ret_t 的状态代码
 *
 * @brief Initialize an array list
 * @param array_list Pointer to the array list to be initialized
 * @param initial_capacity Initial capacity of the array list
 * @param data_size Size of the data elements in bytes
 * @param allocator Allocator for allocating and freeing memory
 * @return Returns a status code of type rcutils_ret_t
 */
rcutils_ret_t rcutils_array_list_init(
  rcutils_array_list_t * array_list,
  size_t initial_capacity,
  size_t data_size,
  const rcutils_allocator_t * allocator)
{
  // 检查 array_list 是否为空，如果为空，则返回无效参数错误
  // Check if array_list is NULL, if it is, return an invalid argument error
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(array_list, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查分配器是否有效，如果无效，则返回无效参数错误
  // Check if the allocator is valid, if not, return an invalid argument error
  RCUTILS_CHECK_ALLOCATOR(allocator, return RCUTILS_RET_INVALID_ARGUMENT);

  // 如果 array_list 已初始化，则返回无效参数错误
  // If array_list is already initialized, return an invalid argument error
  if (NULL != array_list->impl) {
    RCUTILS_SET_ERROR_MSG("array_list is already initialized");
    return RCUTILS_RET_INVALID_ARGUMENT;
  } else if (1 > initial_capacity) {
    // 如果初始容量小于1，则返回无效参数错误
    // If the initial capacity is less than 1, return an invalid argument error
    RCUTILS_SET_ERROR_MSG("initial_capacity cannot be less than 1");
    return RCUTILS_RET_INVALID_ARGUMENT;
  } else if (1 > data_size) {
    // 如果数据大小小于1，则返回无效参数错误
    // If the data size is less than 1, return an invalid argument error
    RCUTILS_SET_ERROR_MSG("data_size cannot be less than 1");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 为 array_list->impl 分配内存
  // Allocate memory for array_list->impl
  array_list->impl = allocator->allocate(sizeof(rcutils_array_list_impl_t), allocator->state);
  if (NULL == array_list->impl) {
    // 如果分配失败，设置错误消息并返回 BAD_ALLOC 错误
    // If allocation fails, set error message and return BAD_ALLOC error
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for array list impl");
    return RCUTILS_RET_BAD_ALLOC;
  }

  // 设置数组列表的初始容量、大小、数据大小和分配器
  // Set the initial capacity, size, data size, and allocator for the array list
  array_list->impl->capacity = initial_capacity;
  array_list->impl->size = 0;
  array_list->impl->data_size = data_size;
  array_list->impl->list = allocator->allocate(initial_capacity * data_size, allocator->state);
  if (NULL == array_list->impl->list) {
    // 如果分配失败，释放 array_list->impl 并设置错误消息，然后返回 BAD_ALLOC 错误
    // If allocation fails, free array_list->impl, set error message and return BAD_ALLOC error
    allocator->deallocate(array_list->impl, allocator->state);
    array_list->impl = NULL;
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for array list data");
    return RCUTILS_RET_BAD_ALLOC;
  }
  array_list->impl->allocator = *allocator;

  // 返回成功状态
  // Return success status
  return RCUTILS_RET_OK;
}

/**
 * @brief 销毁数组列表
 * @param array_list 指向要销毁的数组列表的指针
 * @return 返回 rcutils_ret_t 的状态代码
 *
 * @brief Finalize an array list
 * @param array_list Pointer to the array list to be destroyed
 * @return Returns a status code of type rcutils_ret_t
 */
rcutils_ret_t rcutils_array_list_fini(rcutils_array_list_t * array_list)
{
  // 验证 array_list 是否有效
  // Validate if array_list is valid
  ARRAY_LIST_VALIDATE_ARRAY_LIST(array_list);

  // 使用分配器释放列表数据内存和实现内存
  // Use the allocator to free the memory for the list data and implementation
  array_list->impl->allocator.deallocate(array_list->impl->list, array_list->impl->allocator.state);
  array_list->impl->allocator.deallocate(array_list->impl, array_list->impl->allocator.state);

  // 将 array_list->impl 设置为 NULL
  // Set array_list->impl to NULL
  array_list->impl = NULL;

  // 返回成功状态
  // Return success status
  return RCUTILS_RET_OK;
}

/**
 * @brief 增加数组列表的容量。
 * @param[in,out] array_list 指向要操作的数组列表的指针。
 * @return 返回 rcutils_ret_t 类型的结果，表示操作是否成功。
 *
 * Increase the capacity of an array list.
 * @param[in,out] array_list Pointer to the array list to operate on.
 * @return Returns a result of type rcutils_ret_t, indicating whether the operation was successful.
 */
static rcutils_ret_t rcutils_array_list_increase_capacity(rcutils_array_list_t * array_list)
{
  // 计算新的容量，是当前容量的两倍
  // Calculate the new capacity, which is twice the current capacity
  size_t new_capacity = 2 * array_list->impl->capacity;

  // 计算新容量对应的大小
  // Calculate the new size corresponding to the new capacity
  size_t new_size = array_list->impl->data_size * new_capacity;

  // 使用分配器重新分配内存，并将新列表指针赋值给 new_list
  // Use the allocator to reallocate memory and assign the new list pointer to new_list
  void * new_list = array_list->impl->allocator.reallocate(
    array_list->impl->list, new_size, array_list->impl->allocator.state);

  // 如果新列表指针为 NULL，则分配失败，返回错误代码
  // If the new list pointer is NULL, allocation failed, return error code
  if (NULL == new_list) {
    return RCUTILS_RET_BAD_ALLOC;
  }

  // 更新实现结构体中的列表指针和容量
  // Update the list pointer and capacity in the implementation structure
  array_list->impl->list = new_list;
  array_list->impl->capacity = new_capacity;

  // 返回成功状态
  // Return success status
  return RCUTILS_RET_OK;
}

/**
 * @brief 获取数组列表中指定索引的元素指针。
 * @param[in] array_list 指向要操作的数组列表的指针。
 * @param[in] index 要获取的元素的索引。
 * @return 返回指向指定索引元素的 uint8_t 类型指针。
 *
 * Get the pointer to the element at the specified index in the array list.
 * @param[in] array_list Pointer to the array list to operate on.
 * @param[in] index Index of the element to get.
 * @return Returns a uint8_t type pointer pointing to the specified index element.
 */
static uint8_t *
rcutils_array_list_get_pointer_for_index(const rcutils_array_list_t * array_list, size_t index)
{
  // 获取列表开始位置的指针
  // Get the pointer to the start position of the list
  uint8_t * list_start = array_list->impl->list;

  // 计算指定索引元素的指针并返回
  // Calculate the pointer to the specified index element and return
  uint8_t * index_ptr = list_start + (array_list->impl->data_size * index);
  return index_ptr;
}

/**
 * @brief 向数组列表中添加一个元素。
 * @param[in,out] array_list 指向要操作的数组列表的指针。
 * @param[in] data 要添加的元素的指针。
 * @return 返回 rcutils_ret_t 类型的结果，表示操作是否成功。
 *
 * Add an element to the array list.
 * @param[in,out] array_list Pointer to the array list to operate on.
 * @param[in] data Pointer to the element to add.
 * @return Returns a result of type rcutils_ret_t, indicating whether the operation was successful.
 */
rcutils_ret_t rcutils_array_list_add(rcutils_array_list_t * array_list, const void * data)
{
  // 验证数组列表是否有效
  // Verify that the array list is valid
  ARRAY_LIST_VALIDATE_ARRAY_LIST(array_list);

  // 检查输入的数据指针是否为空，如果为空，则返回错误代码
  // Check if the input data pointer is NULL, if it is, return error code
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(data, RCUTILS_RET_INVALID_ARGUMENT);

  // 初始化返回值为成功状态
  // Initialize the return value as a success status
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // 如果添加元素后大小超过容量，尝试增加容量
  // If the size exceeds capacity after adding an element, try to increase the capacity
  if (array_list->impl->size + 1 > array_list->impl->capacity) {
    ret = rcutils_array_list_increase_capacity(array_list);
    if (RCUTILS_RET_OK != ret) {
      return ret;
    }
  }

  // 获取要添加元素的索引位置指针
  // Get the index position pointer for the element to be added
  uint8_t * index_ptr =
    rcutils_array_list_get_pointer_for_index(array_list, array_list->impl->size);

  // 将数据拷贝到索引位置
  // Copy the data to the index position
  memcpy(index_ptr, data, array_list->impl->data_size);

  // 增加数组列表的大小
  // Increase the size of the array list
  array_list->impl->size++;

  // 返回操作结果
  // Return the operation result
  return ret;
}

/**
 * @brief 用给定的数据替换指定索引处的元素 (Replace the element at the specified index with the given data)
 *
 * @param[in] array_list 指向要操作的数组列表的指针 (Pointer to the array list to operate on)
 * @param[in] index 要设置的元素的索引 (Index of the element to set)
 * @param[in] data 要存储在指定索引处的数据 (Data to store at the specified index)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t
rcutils_array_list_set(rcutils_array_list_t * array_list, size_t index, const void * data)
{
  // 验证数组列表是否有效 (Verify if the array list is valid)
  ARRAY_LIST_VALIDATE_ARRAY_LIST(array_list);

  // 检查传入的数据参数是否为空 (Check if the passed data argument is null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(data, RCUTILS_RET_INVALID_ARGUMENT);

  // 验证索引是否在数组列表的范围内 (Validate if the index is within the bounds of the array list)
  ARRAY_LIST_VALIDATE_INDEX_IN_BOUNDS(array_list, index);

  // 获取指定索引的指针 (Get the pointer for the specified index)
  uint8_t * index_ptr = rcutils_array_list_get_pointer_for_index(array_list, index);

  // 将数据复制到指定索引处 (Copy the data to the specified index)
  memcpy(index_ptr, data, array_list->impl->data_size);

  return RCUTILS_RET_OK;
}

/**
 * @brief 从数组列表中删除指定索引处的元素 (Remove the element at the specified index from the array list)
 *
 * @param[in] array_list 指向要操作的数组列表的指针 (Pointer to the array list to operate on)
 * @param[in] index 要删除的元素的索引 (Index of the element to remove)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t rcutils_array_list_remove(rcutils_array_list_t * array_list, size_t index)
{
  // 验证数组列表是否有效 (Verify if the array list is valid)
  ARRAY_LIST_VALIDATE_ARRAY_LIST(array_list);

  // 验证索引是否在数组列表的范围内 (Validate if the index is within the bounds of the array list)
  ARRAY_LIST_VALIDATE_INDEX_IN_BOUNDS(array_list, index);

  // 计算需要复制的数据数量，以替换缺失的数据 (Calculate the number of data to copy to replace the missing data)
  size_t copy_count = array_list->impl->size - (index + 1);
  if (copy_count > 0) {
    // 获取目标指针和源指针 (Get the destination and source pointers)
    uint8_t * dst_ptr = rcutils_array_list_get_pointer_for_index(array_list, index);
    uint8_t * src_ptr = rcutils_array_list_get_pointer_for_index(array_list, index + 1);

    // 将数据从源指针复制到目标指针 (Copy the data from the source pointer to the destination pointer)
    memcpy(dst_ptr, src_ptr, array_list->impl->data_size * copy_count);
  }

  // 减小数组列表的大小 (Decrease the size of the array list)
  array_list->impl->size--;
  return RCUTILS_RET_OK;
}

/**
 * @brief 获取指定索引处的元素 (Get the element at the specified index)
 *
 * @param[in] array_list 指向要操作的数组列表的指针 (Pointer to the array list to operate on)
 * @param[in] index 要获取的元素的索引 (Index of the element to get)
 * @param[out] data 存储从指定索引处获取到的数据的指针 (Pointer to store the data retrieved from the specified index)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t
rcutils_array_list_get(const rcutils_array_list_t * array_list, size_t index, void * data)
{
  // 验证数组列表是否有效 (Verify if the array list is valid)
  ARRAY_LIST_VALIDATE_ARRAY_LIST(array_list);

  // 检查传入的数据参数是否为空 (Check if the passed data argument is null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(data, RCUTILS_RET_INVALID_ARGUMENT);

  // 验证索引是否在数组列表的范围内 (Validate if the index is within the bounds of the array list)
  ARRAY_LIST_VALIDATE_INDEX_IN_BOUNDS(array_list, index);

  // 获取指定索引的指针 (Get the pointer for the specified index)
  uint8_t * index_ptr = rcutils_array_list_get_pointer_for_index(array_list, index);

  // 将数据从指定索引复制到输出参数 (Copy the data from the specified index to the output parameter)
  memcpy(data, index_ptr, array_list->impl->data_size);

  return RCUTILS_RET_OK;
}

/**
 * @brief 获取数组列表的大小 (Get the size of the array list)
 *
 * @param[in] array_list 指向要操作的数组列表的指针 (Pointer to the array list to operate on)
 * @param[out] size 存储数组列表大小的指针 (Pointer to store the size of the array list)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t rcutils_array_list_get_size(const rcutils_array_list_t * array_list, size_t * size)
{
  // 验证数组列表是否有效 (Verify if the array list is valid)
  ARRAY_LIST_VALIDATE_ARRAY_LIST(array_list);

  // 检查传入的大小参数是否为空 (Check if the passed size argument is null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(size, RCUTILS_RET_INVALID_ARGUMENT);

  // 将数组列表的大小存储到输出参数中 (Store the size of the array list in the output parameter)
  *size = array_list->impl->size;

  return RCUTILS_RET_OK;
}

#ifdef __cplusplus
}
#endif
