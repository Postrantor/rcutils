// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>

#include "rcutils/allocator.h"

#include "rcutils/error_handling.h"
#include "rcutils/macros.h"

// When this define evaluates to true (default), then messages will printed to
// stderr when an error is encoutered while setting the error state.
// For example, when memory cannot be allocated or a previous error state is
// being overwritten.
#ifndef RCUTILS_REPORT_ERROR_HANDLING_ERRORS
#define RCUTILS_REPORT_ERROR_HANDLING_ERRORS 1
#endif

/**
 * @brief 默认分配内存的函数 (Default memory allocation function)
 *
 * @param[in] size 要分配的内存大小 (The size of the memory to allocate)
 * @param[in] state 分配器状态指针，此处未使用 (Allocator state pointer, not used here)
 * @return 返回分配的内存指针，如果失败则返回 NULL (Returns the allocated memory pointer, or NULL if failed)
 */
static void * __default_allocate(size_t size, void * state)
{
  // 检查是否可以返回错误值 NULL (Check if it's allowed to return error value NULL)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(NULL);

  // 声明 state 参数未使用 (Declare the state parameter as unused)
  RCUTILS_UNUSED(state);

  // 分配指定大小的内存并返回指针 (Allocate the specified size of memory and return the pointer)
  return malloc(size);
}

/**
 * @brief 默认释放内存的函数 (Default memory deallocation function)
 *
 * @param[in] pointer 要释放的内存指针 (The memory pointer to be freed)
 * @param[in] state 分配器状态指针，此处未使用 (Allocator state pointer, not used here)
 */
static void __default_deallocate(void * pointer, void * state)
{
  // 声明 state 参数未使用 (Declare the state parameter as unused)
  RCUTILS_UNUSED(state);

  // 释放指定的内存指针 (Free the specified memory pointer)
  free(pointer);
}

/**
 * @brief 默认重新分配内存的函数 (Default memory reallocation function)
 *
 * @param[in] pointer 要重新分配的内存指针 (The memory pointer to be reallocated)
 * @param[in] size 新的内存大小 (The new memory size)
 * @param[in] state 分配器状态指针，此处未使用 (Allocator state pointer, not used here)
 * @return 返回重新分配的内存指针，如果失败则返回 NULL (Returns the reallocated memory pointer, or NULL if failed)
 */
static void * __default_reallocate(void * pointer, size_t size, void * state)
{
  // 检查是否可以返回错误值 NULL (Check if it's allowed to return error value NULL)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(NULL);

  // 声明 state 参数未使用 (Declare the state parameter as unused)
  RCUTILS_UNUSED(state);

  // 重新分配指定大小的内存并返回指针 (Reallocate the specified size of memory and return the pointer)
  return realloc(pointer, size);
}

/**
 * @brief 默认分配并初始化为零的内存函数 (Default memory allocation and zero-initialization function)
 *
 * @param[in] number_of_elements 要分配的元素数量 (The number of elements to allocate)
 * @param[in] size_of_element 每个元素的大小 (The size of each element)
 * @param[in] state 分配器状态指针，此处未使用 (Allocator state pointer, not used here)
 * @return 返回分配并初始化为零的内存指针，如果失败则返回 NULL (Returns the allocated and zero-initialized memory pointer, or NULL if failed)
 */
static void *
__default_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state)
{
  // 检查是否可以返回错误值 NULL (Check if it's allowed to return error value NULL)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(NULL);

  // 声明 state 参数未使用 (Declare the state parameter as unused)
  RCUTILS_UNUSED(state);

  // 分配指定数量和大小的元素并初始化为零，返回指针 (Allocate the specified number and size of elements, initialize them to zero, and return the pointer)
  return calloc(number_of_elements, size_of_element);
}

/**
 * @brief 获取一个零初始化的分配器对象 (Get a zero-initialized allocator object)
 *
 * @return 返回一个零初始化的 rcutils_allocator_t 结构体实例 (Returns a zero-initialized instance of rcutils_allocator_t struct)
 */
rcutils_allocator_t rcutils_get_zero_initialized_allocator(void)
{
  // 定义一个静态的零初始化分配器变量 (Define a static zero-initialized allocator variable)
  static rcutils_allocator_t zero_allocator = {
    .allocate = NULL,
    .deallocate = NULL,
    .reallocate = NULL,
    .zero_allocate = NULL,
    .state = NULL,
  };

  // 返回零初始化分配器 (Return the zero-initialized allocator)
  return zero_allocator;
}

/**
 * @brief 获取默认的内存分配器 (Get the default memory allocator)
 *
 * @return 返回默认的内存分配器对象 (Returns the default memory allocator object)
 */
rcutils_allocator_t rcutils_get_default_allocator()
{
  // 定义并初始化一个静态默认分配器变量 (Define and initialize a static default allocator variable)
  static rcutils_allocator_t default_allocator = {
    .allocate = __default_allocate,     // 分配内存函数 (Memory allocation function)
    .deallocate = __default_deallocate, // 释放内存函数 (Memory deallocation function)
    .reallocate = __default_reallocate, // 重新分配内存函数 (Memory reallocation function)
    .zero_allocate =
      __default_zero_allocate, // 分配并清零内存函数 (Allocate and zero memory function)
    .state = NULL,             // 分配器状态 (Allocator state)
  };
  // 返回默认分配器 (Return the default allocator)
  return default_allocator;
}

/**
 * @brief 检查内存分配器是否有效 (Check if the memory allocator is valid)
 *
 * @param[in] allocator 要检查的内存分配器指针 (Pointer to the memory allocator to check)
 * @return 如果内存分配器有效，则返回 true，否则返回 false (Returns true if the memory allocator is valid, false otherwise)
 */
bool rcutils_allocator_is_valid(const rcutils_allocator_t * allocator)
{
  // 检查分配器及其函数指针是否为 NULL (Check if the allocator and its function pointers are NULL)
  if (
    NULL == allocator || NULL == allocator->allocate || NULL == allocator->deallocate ||
    NULL == allocator->zero_allocate || NULL == allocator->reallocate) {
    return false;
  }
  return true;
}

/**
 * @brief 使用给定的分配器重新分配内存，如果分配失败，则释放原始指针 (Reallocate memory with the given allocator, deallocating the original pointer if allocation fails)
 *
 * @param[in] pointer 要重新分配的内存指针 (Pointer to the memory to be reallocated)
 * @param[in] size 新的内存大小 (New memory size)
 * @param[in] allocator 用于重新分配内存的分配器 (Allocator for reallocating the memory)
 * @return 如果分配成功，则返回新的内存指针，否则返回 NULL (Returns the new memory pointer if allocation is successful, NULL otherwise)
 */
void * rcutils_reallocf(void * pointer, size_t size, rcutils_allocator_t * allocator)
{
  // 检查分配器是否有效 (Check if the allocator is valid)
  if (!rcutils_allocator_is_valid(allocator)) {
    // 无法释放指针，因此将消息打印到 stderr 并返回 NULL (Cannot deallocate pointer, so print message to stderr and return NULL)
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    RCUTILS_SAFE_FWRITE_TO_STDERR("[rcutils|allocator.c:" RCUTILS_STRINGIFY(
      __LINE__) "] rcutils_reallocf(): "
                "invalid allocator or allocator function pointers, memory leaked\n");
#endif
    return NULL;
  }
  // 使用给定的分配器重新分配内存 (Reallocate memory using the given allocator)
  void * new_pointer = allocator->reallocate(pointer, size, allocator->state);
  // 如果新指针为 NULL，则释放原始指针 (If the new pointer is NULL, deallocate the original pointer)
  if (NULL == new_pointer) {
    allocator->deallocate(pointer, allocator->state);
  }
  // 返回新的内存指针 (Return the new memory pointer)
  return new_pointer;
}
