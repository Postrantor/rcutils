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

#ifndef RCUTILS__TYPES__CHAR_ARRAY_H_
#define RCUTILS__TYPES__CHAR_ARRAY_H_

#if __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#include "rcutils/allocator.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

/// 保存字符数组元数据的结构。 (The structure holding the metadata for a char array)
typedef struct RCUTILS_PUBLIC_TYPE rcutils_char_array_s
{
  /// 分配给此字符数组的内存指针。 (A pointer to the allocated memory for this char array)
  char * buffer;

  /**
   * 如果为true，我们可以根据需要安全地释放/重新分配缓冲区;
   * 否则，我们将保留缓冲区并在需要更多空间时分配新内存。
   * (If this is true, we may safely free/realloc the buffer as needed;
   * otherwise we will leave the buffer alone and alloc new memory if
   * more space is needed)
   */
  bool owns_buffer;

  /// 存储在缓冲区指针中的数据长度。 (The length of the data stored in the buffer pointer)
  size_t buffer_length;

  /// 缓冲区指针的最大容量。 (The maximum capacity of the buffer pointer)
  size_t buffer_capacity;

  /// 用于分配和释放指针中数据的分配器。 (The allocator used to allocate and free the data in the pointer)
  rcutils_allocator_t allocator;
} rcutils_char_array_t;

/// 返回一个零初始化的字符数组结构。 (Return a zero initialized char array struct)
/**
 * \return rcutils_char_array_t 零初始化的字符数组结构 (a zero initialized char array struct)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_char_array_t rcutils_get_zero_initialized_char_array(void);

/// 初始化一个零初始化的字符数组结构。 (Initialize a zero initialized char array struct)
/**
 * 如果字符数组结构已经预先初始化，此函数可能会泄漏。
 * 如果容量设置为0，则不分配内存，内部缓冲区仍然为空。
 * (This function may leak if the char array struct is already
 * pre-initialized.
 * If the capacity is set to 0, no memory is allocated and the internal buffer
 * is still NULL.)
 *
 * \param[in] char_array 指向要初始化的字符数组结构的指针 (a pointer to the to be initialized char array struct)
 * \param[in] buffer_capacity 分配给字节流的内存大小 (the size of the memory to allocate for the byte stream)
 * \param[in] allocator 用于内存分配的分配器 (the allocator to use for the memory allocation)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果任何参数无效 (if any arguments are invalid), or
 * \return #RCUTILS_RET_BAD_ALLOC 如果无法正确分配内存 (if no memory could be allocated correctly)
 * \return #RCUTILS_RET_ERROR 如果发生意外错误 (if an unexpected error occurs)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_char_array_init(
  rcutils_char_array_t * char_array, size_t buffer_capacity, const rcutils_allocator_t * allocator);

/// 结束一个字符数组结构。 (Finalize a char array struct.)
/**
 * 清理并释放由 rcutils_char_array_t 拥有的任何资源。
 * 传递给此函数的数组需要通过 rcutils_char_array_init() 初始化。
 * 如果 .owns_buffer 为 false，则此函数无效，因为这意味着 char_array 不拥有内部缓冲区。
 * 将未初始化的实例传递给此函数会导致未定义的行为。
 *
 * \param[in] char_array 指向要清理的 rcutils_char_array_t 的指针
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果 char_array 参数无效
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_char_array_fini(rcutils_char_array_t * char_array);

/// 调整字符数组的内部缓冲区大小。 (Resize the internal buffer of the char array.)
/**
 * 如果需要，可以动态调整字符数组的内部缓冲区大小。
 * 如果新大小小于当前容量，则内存将被截断。
 * 请注意，这将释放内存，因此使指向该存储的任何指针无效。
 * 如果新大小较大，则分配新内存并复制现有内容。
 * 请注意，如果数组不拥有当前缓冲区，该函数只分配一个新的内存块并复制旧缓冲区的内容，而不是调整现有缓冲区的大小。
 *
 * \param[in] char_array 指向要调整大小的 rcutils_char_array_t 实例的指针
 * \param[in] new_size 内部缓冲区的新大小
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果 new_size 设置为零
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_char_array_resize(rcutils_char_array_t * char_array, size_t new_size);

/// 扩展字符数组的内部缓冲区。 (Expand the internal buffer of the char array.)
/**
 * 该功能等同于 `rcutils_char_array_resize`，只是在内部缓冲区不够大时才调整大小。
 * 如果缓冲区已经足够容纳 `new_size`，则返回 `RCUTILS_RET_OK` 而无需执行任何操作。
 *
 * \param[inout] char_array 指向要调整大小的 rcutils_char_array_t 实例的指针
 * \param[in] new_size 内部缓冲区的新大小
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_char_array_expand_as_needed(rcutils_char_array_t * char_array, size_t new_size);

/// 根据格式和参数生成输出。 (Produce output according to format and args.)
/**
 * 该函数等同于 `vsprintf(char_array->buffer, format, args)`，只是缓冲区根据需要增长，因此用户无需处理内存管理。
 * 在使用之前，将克隆 `va_list args`，因此用户可以在调用此函数后安全地再次使用它。
 *
 * \param[inout] char_array 指向要写入的 rcutils_char_array_t 实例的指针
 * \param[in] format 底层 `vsnprintf` 使用的格式字符串
 * \param[in] args 底层 `vsnprintf` 使用的 `va_list`
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_char_array_vsprintf(rcutils_char_array_t * char_array, const char * format, va_list args);

/// 将一个字符串（或其部分）添加到缓冲区中的字符串。
/// Append a string (or part of it) to the string in buffer.
/**
 * 该函数将内部缓冲区视为字符串，并将src字符串附加到其中。
 * This function treats the internal buffer as a string and appends the src string to it.
 * 如果src长度大于n，则使用n个字节，然后追加一个额外的空字节。
 * If src is longer than n, n bytes will be used and an extra null byte will be appended.
 * 它实际上等同于`strncat(char_array->buffer, src, n)`，只是缓冲区会根据需要增长，因此用户无需处理内存管理。
 * It is virtually equivalent to `strncat(char_array->buffer, src, n)` except that the buffer
 * grows as needed so a user doesn't have to deal with memory management.
 *
 * \param[inout] char_array 指向要附加到的rcutils_char_array_t实例的指针
 * \param[inout] char_array pointer to the instance of rcutils_char_array_t which is being appended to
 * \param[in] src 要附加到缓冲区中字符串末尾的字符串
 * \param[in] src the string to be appended to the end of the string in buffer
 * \param[in] n 使用src字符串的至多n个字节
 * \param[in] n it uses at most n bytes from the src string
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或者
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation failed, or
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 * \return #RCUTILS_RET_ERROR if an unexpected error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_char_array_strncat(rcutils_char_array_t * char_array, const char * src, size_t n);

/// 将一个字符串添加到缓冲区中的字符串。
/// Append a string to the string in buffer.
/**
 * 该函数将内部缓冲区视为字符串，并将src字符串附加到其中。
 * This function treats the internal buffer as a string and appends the src string to it.
 * 它实际上等同于`strcat(char_array->buffer, src)`，只是缓冲区会根据需要增长。也就是说，用户可以安全地使用它，无需计算或检查src和缓冲区的大小。
 * It is virtually equivalent to `strcat(char_array->buffer, src)` except that the buffer
 * grows as needed. That is to say, a user can safely use it without doing calculation or
 * checks on the sizes of the src and buffer.
 *
 * \param[inout] char_array 指向要附加到的rcutils_char_array_t实例的指针
 * \param[inout] char_array pointer to the instance of rcutils_char_array_t which is being
 * appended to
 * \param[in] src 要附加到缓冲区中字符串末尾的字符串
 * \param[in] src the string to be appended to the end of the string in buffer
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或者
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation failed, or
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 * \return #RCUTILS_RET_ERROR if an unexpected error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_char_array_strcat(rcutils_char_array_t * char_array, const char * src);

/// 将内存复制到缓冲区。
/// Copy memory to buffer.
/**
 * 该函数等同于`memcpy(char_array->buffer, src, n)`，只是缓冲区会根据需要增长，因此用户无需担心溢出。
 * This function is equivalent to `memcpy(char_array->buffer, src, n)` except that the buffer
 * grows as needed so a user doesn't have to worry about overflow.
 *
 * \param[inout] char_array 指向要调整大小的rcutils_char_array_t实例的指针
 * \param[inout] char_array pointer to the instance of rcutils_char_array_t which is being resized
 * \param[in] src 要从中复制的内存
 * \param[in] src the memory to be copied from
 * \param[in] n 将复制n个字节
 * \param[in] n a total of n bytes will be copied
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或者
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation failed, or
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 * \return #RCUTILS_RET_ERROR if an unexpected error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_char_array_memcpy(rcutils_char_array_t * char_array, const char * src, size_t n);

/// 将一个字符串复制到缓冲区。
/// Copy a string to buffer.
/**
 * 该函数等同于`strcpy(char_array->buffer, src)`，只是缓冲区会根据需要增长，以便`src`适应而不会溢出。
 * This function is equivalent to `strcpy(char_array->buffer, src)` except that the buffer
 * grows as needed so that `src` will fit without overflow.
 *
 * \param[inout] char_array 指向要复制到的rcutils_char_array_t实例的指针
 * \param[inout] char_array pointer to the instance of rcutils_char_array_t which is being
 * copied to
 * \param[in] src 要从中复制的字符串
 * \param[in] src the string to be copied from
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或者
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation failed, or
 * \return #RCUTILS_RET_ERROR 如果发生意外错误。
 * \return #RCUTILS_RET_ERROR if an unexpected error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_char_array_strcpy(rcutils_char_array_t * char_array, const char * src);

#if __cplusplus
}
#endif

#endif // RCUTILS__TYPES__CHAR_ARRAY_H_
