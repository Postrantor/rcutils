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

#include "rcutils/types/char_array.h"
#include "rcutils/error_handling.h"
#include <stdarg.h>

/**
 * @brief 获取两个值中的最小值 (Get the minimum of two values)
 *
 * @param a 第一个值 (The first value)
 * @param b 第二个值 (The second value)
 * @return 返回最小值 (Return the minimum value)
 */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief 初始化一个字符数组并将其设置为零 (Initialize a character array and set it to zero)
 *
 * @return 返回一个零初始化的字符数组 (Return a zero-initialized character array)
 */
rcutils_char_array_t rcutils_get_zero_initialized_char_array(void)
{
  // 静态变量，用于存储零初始化的字符数组 (Static variable for storing the zero-initialized character array)
  static rcutils_char_array_t char_array = {
    .buffer = NULL, .owns_buffer = true, .buffer_length = 0u, .buffer_capacity = 0u};

  // 设置分配器为零初始化的分配器 (Set the allocator to a zero-initialized allocator)
  char_array.allocator = rcutils_get_zero_initialized_allocator();

  // 返回零初始化的字符数组 (Return the zero-initialized character array)
  return char_array;
}

/**
 * @brief 初始化字符数组 (Initialize a character array)
 *
 * @param[out] char_array 指向要初始化的字符数组的指针 (Pointer to the character array to be initialized)
 * @param[in] buffer_capacity 字符数组的缓冲区容量 (Buffer capacity of the character array)
 * @param[in] allocator 分配器，用于分配内存 (Allocator for memory allocation)
 * @return 返回操作结果 (Return the operation result)
 */
rcutils_ret_t rcutils_char_array_init(
  rcutils_char_array_t * char_array, size_t buffer_capacity, const rcutils_allocator_t * allocator)
{
  // 检查输入参数是否为空 (Check if the input argument is null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(char_array, RCUTILS_RET_ERROR);

  // 检查分配器是否有效，并返回错误消息 (Check if the allocator is valid and return an error message)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    allocator, "char array has no valid allocator", return RCUTILS_RET_ERROR);

  // 设置字符数组属性 (Set character array properties)
  char_array->owns_buffer = true;
  char_array->buffer_length = 0lu;
  char_array->buffer_capacity = buffer_capacity;
  char_array->allocator = *allocator;

  // 如果缓冲区容量大于0，则分配内存 (If the buffer capacity is greater than 0, allocate memory)
  if (buffer_capacity > 0lu) {
    char_array->buffer =
      (char *)allocator->allocate(buffer_capacity * sizeof(char), allocator->state);

    // 检查分配的内存是否为空，如果为空则返回错误消息 (Check if the allocated memory is null, return an error message if it is)
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(char_array->buffer, "failed to allocate memory for char array",
                                    char_array->buffer_capacity = 0lu;
                                    char_array->buffer_length = 0lu; return RCUTILS_RET_BAD_ALLOC);

    // 将第一个字符设置为空字符 (Set the first character to a null character)
    char_array->buffer[0] = '\0';
  }

  // 返回操作成功 (Return operation successful)
  return RCUTILS_RET_OK;
}

/**
 * \brief 释放字符数组的资源 (Free the resources of a character array)
 * \param[in,out] char_array 要释放资源的字符数组指针 (Pointer to the character array to free resources)
 * \return 返回操作状态 (Return the operation status)
 * - RCUTILS_RET_OK 操作成功 (Operation succeeded)
 * - RCUTILS_RET_ERROR 参数无效或分配器错误 (Invalid argument or allocator error)
 */
rcutils_ret_t rcutils_char_array_fini(rcutils_char_array_t * char_array)
{
  // 检查参数是否为空 (Check if the argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(char_array, RCUTILS_RET_ERROR);

  // 如果字符数组拥有缓冲区 (If the character array owns the buffer)
  if (char_array->owns_buffer) {
    // 获取分配器 (Get the allocator)
    rcutils_allocator_t * allocator = &char_array->allocator;
    // 检查分配器是否有效 (Check if the allocator is valid)
    RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
      allocator, "char array has no valid allocator", return RCUTILS_RET_ERROR);

    // 使用分配器释放缓冲区 (Deallocate the buffer using the allocator)
    allocator->deallocate(char_array->buffer, allocator->state);
  }

  // 将缓冲区指针设置为 NULL (Set the buffer pointer to NULL)
  char_array->buffer = NULL;
  // 将缓冲区长度设置为 0 (Set the buffer length to 0)
  char_array->buffer_length = 0lu;
  // 将缓冲区容量设置为 0 (Set the buffer capacity to 0)
  char_array->buffer_capacity = 0lu;

  // 返回操作成功状态 (Return the operation success status)
  return RCUTILS_RET_OK;
}

/**
 * \brief 调整字符数组大小 (Resize a character array)
 * \param[in,out] char_array 要调整大小的字符数组指针 (Pointer to the character array to resize)
 * \param[in] new_size 新的字符数组大小 (New size of the character array)
 * \return 返回操作状态 (Return the operation status)
 * - RCUTILS_RET_OK 操作成功 (Operation succeeded)
 * - RCUTILS_RET_INVALID_ARGUMENT 新大小参数无效 (Invalid new size argument)
 * - RCUTILS_RET_ERROR 参数无效或分配器错误 (Invalid argument or allocator error)
 * - RCUTILS_RET_BAD_ALLOC 重新分配内存失败 (Failed to reallocate memory)
 */
rcutils_ret_t rcutils_char_array_resize(rcutils_char_array_t * char_array, size_t new_size)
{
  // 检查参数是否为空 (Check if the argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(char_array, RCUTILS_RET_ERROR);

  // 检查新大小是否为零 (Check if the new size is zero)
  if (0lu == new_size) {
    RCUTILS_SET_ERROR_MSG("new size of char_array has to be greater than zero");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 获取分配器 (Get the allocator)
  rcutils_allocator_t * allocator = &char_array->allocator;
  // 检查分配器是否有效 (Check if the allocator is valid)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    allocator, "char array has no valid allocator", return RCUTILS_RET_ERROR);

  // 如果新大小等于缓冲区容量 (If the new size is equal to the buffer capacity)
  if (new_size == char_array->buffer_capacity) {
    // 无需执行操作 (Nothing to do here)
    return RCUTILS_RET_OK;
  }

  // 保存旧的缓冲区、容量和长度 (Save the old buffer, capacity, and length)
  char * old_buf = char_array->buffer;
  size_t old_size = char_array->buffer_capacity;
  size_t old_length = char_array->buffer_length;

  // 如果字符数组拥有缓冲区 (If the character array owns the buffer)
  if (char_array->owns_buffer) {
    // 使用分配器重新分配内存 (Reallocate memory using the allocator)
    char * new_buf = rcutils_reallocf(char_array->buffer, new_size * sizeof(char), allocator);
    // 检查重新分配的内存是否为空 (Check if the reallocated memory is NULL)
    RCUTILS_CHECK_FOR_NULL_WITH_MSG(
      new_buf, "failed to reallocate memory for char array", return RCUTILS_RET_BAD_ALLOC);
    // 更新缓冲区指针 (Update the buffer pointer)
    char_array->buffer = new_buf;
  } else {
    // 初始化一个新的字符数组 (Initialize a new character array)
    rcutils_ret_t ret = rcutils_char_array_init(char_array, new_size, allocator);
    // 检查操作是否成功 (Check if the operation succeeded)
    if (ret != RCUTILS_RET_OK) {
      return ret;
    }
    // 计算需要复制的字节数 (Calculate the number of bytes to copy)
    size_t n = MIN(new_size, old_size);
    // 复制旧缓冲区到新缓冲区 (Copy the old buffer to the new buffer)
    memcpy(char_array->buffer, old_buf, n);
    // 总是添加结束符 (Always add an ending character)
    char_array->buffer[n - 1] = '\0';
  }

  // 更新缓冲区容量和长度 (Update the buffer capacity and length)
  char_array->buffer_capacity = new_size;
  char_array->buffer_length = MIN(new_size, old_length);

  // 返回操作成功状态 (Return the operation success status)
  return RCUTILS_RET_OK;
}

/**
 * @brief 将字符数组扩展到指定大小 (Expand the character array to a specified size)
 *
 * @param[in,out] char_array 输入的字符数组指针 (Pointer to the input character array)
 * @param[in] new_size 新的缓冲区大小 (New buffer size)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t
rcutils_char_array_expand_as_needed(rcutils_char_array_t * char_array, size_t new_size)
{
  // 检查输入参数是否为空 (Check if the input parameter is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(char_array, RCUTILS_RET_ERROR);

  // 如果新大小小于等于当前缓冲区容量，则不需要扩展 (If the new size is less than or equal to the current buffer capacity, expansion is not needed)
  if (new_size <= char_array->buffer_capacity) {
    return RCUTILS_RET_OK;
  }

  // 确保扩展至少是旧容量的1.5倍 (Make sure to expand at least 1.5 times the old capacity)
  size_t minimum_size = char_array->buffer_capacity + (char_array->buffer_capacity >> 1);
  if (new_size < minimum_size) {
    new_size = minimum_size;
  }

  // 调整字符数组大小并返回结果 (Resize the character array and return the result)
  return rcutils_char_array_resize(char_array, new_size);
}

/**
 * @brief 使用格式化字符串和参数列表向字符数组写入数据 (Write data to the character array using a formatted string and argument list)
 *
 * @param[out] char_array 输出的字符数组指针 (Pointer to the output character array)
 * @param[in] format 格式化字符串 (Formatted string)
 * @param[in] args 可变参数列表 (Variable argument list)
 * @return int 返回写入的字符数，不包括终止空字符 (Return the number of characters written, excluding the terminating null character)
 */
static int
_rcutils_char_array_vsprintf(rcutils_char_array_t * char_array, const char * format, va_list args)
{
  // 复制可变参数列表 (Copy the variable argument list)
  va_list args_clone;
  va_copy(args_clone, args);

  // 计算大小时，记住vsnprintf的返回值排除了终止空字节 (When calculating the size, remember that the return value of vsnprintf excludes the terminating null byte)
  int size = vsnprintf(char_array->buffer, char_array->buffer_capacity, format, args_clone);

  // 结束复制的参数列表 (End the copied argument list)
  va_end(args_clone);

  // 返回写入的字符数 (Return the number of characters written)
  return size;
}

/**
 * @brief 使用给定的格式字符串和参数列表向字符数组进行格式化输出（Format a character array with a given format string and argument list.）
 *
 * @param[out] char_array 输出的字符数组指针（Pointer to the output character array.）
 * @param[in] format 格式字符串（Format string.）
 * @param[in] args 可变参数列表（Variable argument list.）
 * @return 返回 rcutils_ret_t 类型的结果（Returns a result of type rcutils_ret_t.）
 */
rcutils_ret_t
rcutils_char_array_vsprintf(rcutils_char_array_t * char_array, const char * format, va_list args)
{
  // 调用 _rcutils_char_array_vsprintf 函数，并将返回值存储在 size 变量中
  // (Call the _rcutils_char_array_vsprintf function and store the return value in the size variable.)
  int size = _rcutils_char_array_vsprintf(char_array, format, args);

  // 判断 size 是否小于 0，如果是，则设置错误信息并返回 RCUTILS_RET_ERROR
  // (Check if size is less than 0; if so, set an error message and return RCUTILS_RET_ERROR.)
  if (size < 0) {
    RCUTILS_SET_ERROR_MSG("vsprintf on char array failed");
    return RCUTILS_RET_ERROR;
  }

  // 将 size 加上 1，包含终止空字节 (Add 1 to size, including the terminating null byte.)
  size_t new_size = (size_t)size + 1;

  // 判断 new_size 是否大于 char_array 的 buffer_capacity，如果是，则执行以下操作
  // (Check if new_size is greater than the buffer_capacity of char_array; if so, perform the following operations.)
  if (new_size > char_array->buffer_capacity) {
    // 尝试扩展 char_array 的容量到 new_size，将返回值存储在 ret 变量中
    // (Try to expand the capacity of char_array to new_size and store the return value in the ret variable.)
    rcutils_ret_t ret = rcutils_char_array_expand_as_needed(char_array, new_size);

    // 如果 ret 不等于 RCUTILS_RET_OK，则设置错误信息并返回 ret
    // (If ret is not equal to RCUTILS_RET_OK, set an error message and return ret.)
    if (ret != RCUTILS_RET_OK) {
      RCUTILS_SET_ERROR_MSG("char array failed to expand");
      return ret;
    }

    // 再次调用 _rcutils_char_array_vsprintf 函数，如果返回值不等于 size，则尝试清理 char_array 并设置错误信息
    // (Call the _rcutils_char_array_vsprintf function again; if the return value is not equal to size, try to clean up char_array and set an error message.)
    if (_rcutils_char_array_vsprintf(char_array, format, args) != size) {
      if (rcutils_char_array_fini(char_array) == RCUTILS_RET_OK) {
        RCUTILS_SET_ERROR_MSG("vsprintf on resized char array failed");
      } else {
        RCUTILS_SET_ERROR_MSG("vsprintf on resized char array failed; clean up failed too");
      }
      return RCUTILS_RET_ERROR;
    }
  }

  // 设置 char_array 的 buffer_length 为 new_size
  // (Set the buffer_length of char_array to new_size.)
  char_array->buffer_length = new_size;

  // 返回 RCUTILS_RET_OK
  // (Return RCUTILS_RET_OK.)
  return RCUTILS_RET_OK;
}

/**
 * @brief 将源字符串的前 n 个字符复制到目标字符数组中（Copy the first n characters of the source string to the destination character array.）
 *
 * @param[out] char_array 目标字符数组指针（Pointer to the destination character array.）
 * @param[in] src 源字符串指针（Pointer to the source string.）
 * @param[in] n 要复制的字符数（Number of characters to copy.）
 * @return 返回 rcutils_ret_t 类型的结果（Returns a result of type rcutils_ret_t.）
 */
rcutils_ret_t
rcutils_char_array_memcpy(rcutils_char_array_t * char_array, const char * src, size_t n)
{
  // 尝试扩展 char_array 的容量到 n，将返回值存储在 ret 变量中
  // (Try to expand the capacity of char_array to n and store the return value in the ret variable.)
  rcutils_ret_t ret = rcutils_char_array_expand_as_needed(char_array, n);

  // 如果 ret 不等于 RCUTILS_RET_OK，则设置错误信息并返回 ret
  // (If ret is not equal to RCUTILS_RET_OK, set an error message and return ret.)
  if (ret != RCUTILS_RET_OK) {
    RCUTILS_SET_ERROR_MSG("char array failed to expand");
    return ret;
  }

  // 使用 memcpy 函数将 src 中的前 n 个字符复制到 char_array 的 buffer 中
  // (Use the memcpy function to copy the first n characters from src to the buffer of char_array.)
  memcpy(char_array->buffer, src, n);

  // 设置 char_array 的 buffer_length 为 n
  // (Set the buffer_length of char_array to n.)
  char_array->buffer_length = n;

  // 返回 RCUTILS_RET_OK
  // (Return RCUTILS_RET_OK.)
  return RCUTILS_RET_OK;
}

/**
 * @brief 将源字符串复制到字符数组中 (Copy the source string to the character array)
 *
 * @param[out] char_array 目标字符数组 (Target character array)
 * @param[in] src 源字符串 (Source string)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_char_array_strcpy(rcutils_char_array_t * char_array, const char * src)
{
  // 调用 rcutils_char_array_memcpy 函数进行内存拷贝，并返回结果 (Call the rcutils_char_array_memcpy function for memory copy and return the result)
  return rcutils_char_array_memcpy(char_array, src, strlen(src) + 1);
}

/**
 * @brief 将源字符串的前 n 个字符连接到字符数组中 (Concatenate the first n characters of the source string to the character array)
 *
 * @param[out] char_array 目标字符数组 (Target character array)
 * @param[in] src 源字符串 (Source string)
 * @param[in] n 要连接的字符数 (Number of characters to concatenate)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t
rcutils_char_array_strncat(rcutils_char_array_t * char_array, const char * src, size_t n)
{
  size_t current_strlen;
  // 判断字符数组的缓冲区长度是否为0 (Determine whether the buffer length of the character array is 0)
  if (char_array->buffer_length == 0) {
    current_strlen = 0;
  } else {
    // 缓冲区长度总是包含尾部的 \0，所以实际字符串长度比缓冲区长度少1 (The buffer length always contains the trailing \0, so the actual string length is one less than the buffer length)
    current_strlen = char_array->buffer_length - 1;
  }
  // 计算新的长度 (Calculate the new length)
  size_t new_length = current_strlen + n + 1;
  // 调用 rcutils_char_array_expand_as_needed 函数，根据需要扩展字符数组 (Call the rcutils_char_array_expand_as_needed function to expand the character array as needed)
  rcutils_ret_t ret = rcutils_char_array_expand_as_needed(char_array, new_length);
  // 判断操作是否成功 (Determine whether the operation is successful)
  if (ret != RCUTILS_RET_OK) {
    RCUTILS_SET_ERROR_MSG("char array failed to expand");
    return ret;
  }

  // 使用 memcpy 将源字符串的前 n 个字符拷贝到目标字符数组中 (Use memcpy to copy the first n characters of the source string to the target character array)
  memcpy(char_array->buffer + current_strlen, src, n);
  // 设置新长度处的字符为 '\0' (Set the character at the new length to '\0')
  char_array->buffer[new_length - 1] = '\0';

  // 更新字符数组的缓冲区长度 (Update the buffer length of the character array)
  char_array->buffer_length = new_length;
  // 返回操作结果 (Return operation result)
  return RCUTILS_RET_OK;
}

/**
 * @brief 将源字符串连接到字符数组中 (Concatenate the source string to the character array)
 *
 * @param[out] char_array 目标字符数组 (Target character array)
 * @param[in] src 源字符串 (Source string)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_char_array_strcat(rcutils_char_array_t * char_array, const char * src)
{
  // 调用 rcutils_char_array_strncat 函数进行字符串连接，并返回结果 (Call the rcutils_char_array_strncat function for string concatenation and return the result)
  return rcutils_char_array_strncat(char_array, src, strlen(src));
}
