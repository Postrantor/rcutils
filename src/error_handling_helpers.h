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

// Note: migrated from rmw/error_handling.h in 2017-04

#ifndef ERROR_HANDLING_HELPERS_H_
#define ERROR_HANDLING_HELPERS_H_

/**
 * @file
 * @brief 定义错误处理和截断警告的宏 (Macros for error handling and truncation warnings)
 */

// 当此定义为 true 时（默认情况下），如果在设置错误状态时遇到错误，
// 则将向 stderr 打印消息。
// 例如，当无法分配内存或覆盖之前的错误状态时。
// When this define evaluates to true (default), then messages will be printed to
// stderr when an error is encountered while setting the error state.
// For example, when memory cannot be allocated or a previous error state is
// being overwritten.
#ifndef RCUTILS_REPORT_ERROR_HANDLING_ERRORS
#define RCUTILS_REPORT_ERROR_HANDLING_ERRORS 1
#endif

// 当此定义为 true 时（默认情况下），将使用 RCUTILS_SAFE_FWRITE_TO_STDERR 向
// stderr 写入警告。
// When this define evaluates to true (default), then a warning will be written
// to stderr using RCUTILS_SAFE_FWRITE_TO_STDERR.
#ifndef RCUTILS_WARN_ON_TRUNCATION
#define RCUTILS_WARN_ON_TRUNCATION 1
#endif

#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1 // indicate we would like memmove_s if available
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <rcutils/error_handling.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 不要在外部使用，这是一个仅供 error_handling.c 使用的内部函数 (Do not use externally, internal function which is only to be used by error_handling.c)
 * 
 * @param dst 目标字符串指针 (Pointer to the destination string)
 * @param dst_size 目标字符串的大小 (Size of the destination string)
 * @param src 源字符串指针 (Pointer to the source string)
 * @return size_t 拷贝的字符数 (Number of characters copied)
 */
static size_t __rcutils_copy_string(char * dst, size_t dst_size, const char * src)
{
  // 断言目标字符串非空 (Assert that the destination string is not NULL)
  assert(dst != NULL);
  // 断言目标字符串大小大于0 (Assert that the destination string size is greater than 0)
  assert(dst_size > 0);
  // 断言源字符串非空 (Assert that the source string is not NULL)
  assert(src != NULL);

  // 如果源字符串比目标字符串长，则限制其长度为目标字符串长度 + 1（Limit the length of the source string to the destination string length + 1 if it's longer）
  size_t src_length = strlen(src);
  size_t size_to_copy = src_length;

  // 目标字符串需要额外一个字节来存储空终止符 (The destination string needs an extra byte to store the NULL terminating character)
  if (src_length >= dst_size) {
    size_to_copy = dst_size - 1;
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS && RCUTILS_WARN_ON_TRUNCATION
    // 需要截断 (Truncation will be required)
    RCUTILS_SAFE_FWRITE_TO_STDERR("[rcutils|error_handling.c:" RCUTILS_STRINGIFY(
      __LINE__) "] an error string (message, file name, or formatted message) will be truncated\n");
#endif
  }

#ifdef __STDC_LIB_EXT1__
  // 这将始终确保截断 (This will always ensure truncation)
  errno_t ret = memmove_s(dst, dst_size, src, size_to_copy);
  if (0 != ret) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    // memmove_s 失败，错误字符串可能是格式错误的 (memmove_s failed, the error string may be malformed)
    RCUTILS_SAFE_FWRITE_TO_STDERR("[rcutils|error_handling.c:" RCUTILS_STRINGIFY(
      __LINE__) "] memmove_s failed, the error string may be malformed\n");
#endif
  }
#else
  // 拷贝源字符串到目标字符串 (Copy source string to destination string)
  (void)memmove(dst, src, size_to_copy);
#endif

  // 添加空终止符 (Add NULL terminating character)
  dst[size_to_copy] = '\0';

  // 返回拷贝的字符数 (Return the number of characters copied)
  return size_to_copy;
}

/**
 * @brief 反转字符串（Reverse a string）
 * 
 * @param[in,out] string_in 输入字符串（Input string）
 * @param[in] string_len 字符串长度（Length of the string）
 * 
 * @note 此函数仅供 error_handling.c 内部使用，请勿在外部调用。 (Do not use externally, internal function which is only to be used by error_handling.c)
 */
static void __rcutils_reverse_str(char * string_in, size_t string_len)
{
  // 断言，确保输入字符串不为空（Assert that the input string is not NULL）
  assert(string_in != NULL);
  // 如果字符串长度为0，直接返回（If the string length is 0, return immediately）
  if (0 == string_len) {
    return;
  }
  // 初始化两个下标 i 和 j（Initialize two indices i and j）
  size_t i = 0;
  size_t j = string_len - 1;
  // 交换 i 和 j 处的字符，然后向中间靠拢（Swap characters at positions i and j, then move towards the center）
  for (; i < j; i++, j--) {
    char c = string_in[i];
    string_in[i] = string_in[j];
    string_in[j] = c;
  }
}

/**
 * @brief 将 uint64_t 转换为 C 风格字符串（Convert a uint64_t number into a C-style string）
 * 
 * @param[in] number 输入数字（Input number）
 * @param[out] buffer 输出缓冲区（Output buffer）
 * @param[in] buffer_size 缓冲区大小（Size of the buffer）
 * 
 * @note 此函数仅供 error_handling.c 内部使用，请勿在外部调用。 (Do not use externally, internal function which is only to be used by error_handling.c)
 */
static void
__rcutils_convert_uint64_t_into_c_str(uint64_t number, char * buffer, size_t buffer_size)
{
  // 断言，确保输出缓冲区不为空（Assert that the output buffer is not NULL）
  assert(buffer != NULL);
  // 断言，确保缓冲区大小至少为 21（Assert that the buffer size is at least 21）
  assert(buffer_size >= 21);
  // 在 release 构建中阻止警告，因为没有断言(...)（Prevent warning in release builds where there is no assert(...)）
  (void)buffer_size;
  // 初始化下标 i（Initialize index i）
  size_t i = 0;

  // 如果数字为0，直接写入字符 '0' 并添加空终止符（If the number is 0, write the character '0' and add a null terminator）
  if (number == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return;
  }

  // 将数字的每一位添加到字符串中，然后整数除以10，直到数字变为0（Add each digit of the number to the string, then integer divide by 10 until the number becomes 0）
  while (number != 0) {
    buffer[i++] = (char)(number % 10 + '0');
    number = number / 10;
  }

  // null terminate
  buffer[i] = '\0';

  // reverse the string in place
  __rcutils_reverse_str(buffer, strnlen(buffer, 21));
}

/**
 * @brief 格式化错误字符串的内部函数，仅供 error_handling.c 使用 (Internal function for formatting error strings, to be used only by error_handling.c)
 *
 * @param[out] error_string 用于存储格式化后的错误字符串的指针 (Pointer to store the formatted error string)
 * @param[in] error_state 包含错误信息的 rcutils_error_state_t 结构体指针 (Pointer to the rcutils_error_state_t structure containing error information)
 */
static void __rcutils_format_error_string(
  rcutils_error_string_t * error_string, const rcutils_error_state_t * error_state)
{
  // 断言：确保 error_string 和 error_state 不为 NULL (Assert: Ensure that error_string and error_state are not NULL)
  assert(error_string != NULL);
  assert(error_state != NULL);

  // 定义错误字符串的格式 (Define the format of the error string)
  static const char format_1[] = ", at ";
  static const char format_2[] = ":";

  // 存储行号的缓冲区 (Buffer for storing line numbers)
  char line_number_buffer[21];

  // 静态断言：检查 error_string->str 的大小是否足够容纳所有字符串组成 (Static assert: Check if the size of error_string->str is enough to accommodate all the string components)
  static_assert(
    sizeof(error_string->str) ==
      (sizeof(error_state->message) + sizeof(format_1) - 1 /* minus the null-term */ +
       sizeof(error_state->file) + sizeof(format_2) - 1 /* minus the null-term */ +
       sizeof(line_number_buffer) - 1 /* minus the null-term */ + 1 // null terminator
       ),
    "math error in static string formatting");

  // 初始化字符串偏移量和剩余字节数 (Initialize string offset and remaining bytes)
  char * offset = error_string->str;
  size_t bytes_left = sizeof(error_string->str);

  // 将 error_state 的 message 复制到 error_string 中 (Copy the message from error_state to error_string)
  size_t written = __rcutils_copy_string(offset, bytes_left, error_state->message);
  offset += written;
  bytes_left -= written;

  // 添加格式化字符串 format_1 (Add the formatted string format_1)
  written = __rcutils_copy_string(offset, bytes_left, format_1);
  offset += written;
  bytes_left -= written;

  // 将 error_state 的 file 复制到 error_string 中 (Copy the file from error_state to error_string)
  written = __rcutils_copy_string(offset, bytes_left, error_state->file);
  offset += written;
  bytes_left -= written;

  // 添加格式化字符串 format_2 (Add the formatted string format_2)
  written = __rcutils_copy_string(offset, bytes_left, format_2);
  offset += written;
  bytes_left -= written;

  // 将行号从 uint64_t 转换为字符串 (Convert line number from uint64_t to string)
  __rcutils_convert_uint64_t_into_c_str(
    error_state->line_number, line_number_buffer, sizeof(line_number_buffer));

  // 将行号字符串复制到 error_string 中 (Copy the line number string to error_string)
  written = __rcutils_copy_string(offset, bytes_left, line_number_buffer);
  offset += written;

  // 添加空字符终止符 (Add null character terminator)
  offset[0] = '\0';
}

#ifdef __cplusplus
}
#endif

#endif // ERROR_HANDLING_HELPERS_H_
