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

/// \file

#ifndef RCUTILS__ALLOCATOR_H_
#define RCUTILS__ALLOCATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

/// 内存分配器的封装。
/**
 * 默认分配器使用 malloc(), free(), calloc() 和 realloc()。
 * 可以通过 rcutils_get_default_allocator() 获取。
 *
 * 分配器应该是平凡可复制的。
 * 意味着在被赋值复制到新结构后，该结构应继续工作。
 * 特别地，指向 state 指针的对象在所有使用分配器之前应保持有效。
 * 当将分配器提供给像 rcutils_*_init() 这样的函数时，应特别注意，
 * 在其中它被存储在另一个对象中并稍后使用。
 * 开发人员应注意，虽然 const-qualified 分配器结构的字段不能修改，
 * 但可以修改分配器的状态。
 */
typedef struct rcutils_allocator_s
{
  /// 给定大小和 `state` 指针，分配内存。
  /** 通过返回 `NULL` 来指示错误。 */
  void * (*allocate)(size_t size, void * state);
  /// 释放先前分配的内存，模拟 free()。
  /** 也需要 `state` 指针。 */
  void (*deallocate)(void * pointer, void * state);
  /// 如果可能的话重新分配，否则先释放再分配。
  /**
   * 也需要 `state` 指针。
   *
   * 如果不支持，则执行 deallocate 然后 allocate。
   * 这应该表现为 realloc()，而不是 posix 的
   * [reallocf](https://linux.die.net/man/3/reallocf)，即如果 realloc() 失败，
   * pointer 给出的内存将不会自动释放。
   * 对于类似 reallocf 的行为，请使用 rcutils_reallocf()。
   * 此函数必须能够接受 `NULL` 的输入指针并成功。
   */
  void * (*reallocate)(void * pointer, size_t size, void * state);
  /// 给定元素数量和大小，分配所有元素都设置为零的内存。
  /** 通过返回 `NULL` 来指示错误。 */
  void * (*zero_allocate)(size_t number_of_elements, size_t size_of_element, void * state);
  /// 实现定义的状态存储。
  /**
   * 这作为其他分配器函数的最后一个参数传递。
   * 请注意，即使在 const-qualified 分配器对象中，也可以修改 state 的内容。
   */
  void * state;
} rcutils_allocator_t;

/// 返回一个零初始化的分配器（Return a zero initialized allocator）。
/**
 * 请注意，这是一个无效的分配器，仅应用作占位符（Note that this is an invalid allocator and should only be used as a placeholder）。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_allocator_t rcutils_get_zero_initialized_allocator(void);

/// 使用默认值返回正确初始化的 rcutils_allocator_t（Return a properly initialized rcutils_allocator_t with default values）。
/**
 * 默认为（This defaults to）:
 *
 * - allocate = 包装 malloc()（wraps malloc()）
 * - deallocate = 包装 free()（wraps free()）
 * - reallocate = 包装 realloc()（wraps realloc()）
 * - zero_allocate = 包装 calloc()（wraps calloc()）
 * - state = `NULL`
 *
 * <hr>
 * 属性（Attribute）          | 遵循（Adherence）
 * ------------------ | -------------
 * 分配内存（Allocates Memory）   | 否（No）
 * 线程安全（Thread-Safe）        | 是（Yes）
 * 使用原子操作（Uses Atomics）   | 否（No）
 * 无锁（Lock-Free）          | 是（Yes）
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_allocator_t rcutils_get_default_allocator(void);

/// 如果给定的分配器具有非空函数指针，则返回 true（Return true if the given allocator has non-null function pointers）。
/**
 * \param[in] 分配器将被函数检查（allocator to be checked by the function）
 * \return 如果分配器有效，则为`true`，否则为`false`（`true` if the allocator is valid, `false` otherwise）。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool rcutils_allocator_is_valid(const rcutils_allocator_t * allocator);

/// 检查给定的分配器，如果它无效，则运行 fail_statement（Check the given allocator and run fail_statement if it is not valid）。
#define RCUTILS_CHECK_ALLOCATOR(allocator, fail_statement)                                         \
  if (!rcutils_allocator_is_valid(allocator)) {                                                    \
    fail_statement;                                                                                \
  }

/// 检查给定的分配器，并在 msg 中设置消息，如果它无效，则运行 fail_statement。
/// Check the given allocator, and set the message in msg and run fail_statement if it is not valid.
#define RCUTILS_CHECK_ALLOCATOR_WITH_MSG(allocator, msg, fail_statement)                           \
  if (!rcutils_allocator_is_valid(allocator)) {                                                    \
/// 如果分配器无效，设置错误消息                                                     \
/// If the allocator is invalid, set the error message
RCUTILS_SET_ERROR_MSG(msg); /// 运行失败语句
/// Execute the fail statement
fail_statement;
}

/// 模拟 [reallocf](https://linux.die.net/man/3/reallocf) 的行为。
/// Emulate the behavior of [reallocf](https://linux.die.net/man/3/reallocf).
/**
 * 如果分配器是 `NULL` 或者函数指针字段为 `NULL`，此函数将返回 `NULL`。
 * This function will return `NULL` if the allocator is `NULL` or has `NULL` for
 * function pointer fields.
 * \param[inout] 指向将被重新分配的内存的指针
 * \param[inout] pointer to the memory which will be reallocated
 * \param[in] 字节数大小
 * \param[in] size in bytes
 * \param[in] 用于分配和释放内存的分配器
 * \param[in] allocator to be used to allocate and deallocate memory
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
void * rcutils_reallocf(void * pointer, size_t size, rcutils_allocator_t * allocator);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__ALLOCATOR_H_
