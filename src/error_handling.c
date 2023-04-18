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

// Note: migrated from rmw/error_handling.c in 2017-04

#ifdef __cplusplus
extern "C" {
#endif

#include <rcutils/error_handling.h>

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rcutils/allocator.h>
#include <rcutils/macros.h>
#include <rcutils/strdup.h>

#include "./error_handling_helpers.h"

// g_ 是全局变量，gtls_ 是全局线程本地存储变量
// (g_ is to global variable, as gtls_ is to global thread-local storage variable)
RCUTILS_THREAD_LOCAL bool gtls_rcutils_thread_local_initialized = false;
RCUTILS_THREAD_LOCAL rcutils_error_state_t gtls_rcutils_error_state;
RCUTILS_THREAD_LOCAL bool gtls_rcutils_error_string_is_formatted = false;
RCUTILS_THREAD_LOCAL rcutils_error_string_t gtls_rcutils_error_string;
RCUTILS_THREAD_LOCAL bool gtls_rcutils_error_is_set = false;

/**
 * @brief 初始化错误处理线程的本地存储。
 * @param allocator 分配器实例。
 * @return rcutils_ret_t 返回状态。
 *
 * @brief Initialize the error handling thread local storage.
 * @param allocator The allocator instance.
 * @return rcutils_ret_t The return status.
 */
rcutils_ret_t rcutils_initialize_error_handling_thread_local_storage(rcutils_allocator_t allocator)
{
  // 如果线程本地存储已经初始化，直接返回
  // (If the thread local storage has been initialized, return directly)
  if (gtls_rcutils_thread_local_initialized) {
    return RCUTILS_RET_OK;
  }

  // 检查给定的分配器是否有效
  // (Check if the given allocator is valid)
  if (!rcutils_allocator_is_valid(&allocator)) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    RCUTILS_SAFE_FWRITE_TO_STDERR("[rcutils|error_handling.c:" RCUTILS_STRINGIFY(
      __LINE__) "] rcutils_initialize_error_handling_thread_local_storage() given invalid "
                "allocator\n");
#endif
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  // 现在分配器还没有用于任何事情
  // 但是其他未来的实现可能需要使用它
  // 例如只能提供线程本地指针的 pthread 需要分配内存，以便这些指针指向
  // (Right now the allocator is not used for anything,
  // but other future implementations may need to use it,
  // e.g. pthread which could only provide thread-local pointers would need to
  // allocate memory to which those pointers would point)

  // 强制将值恢复到初始状态应该强制线程本地存储初始化并执行所需的内存分配
  // (Forcing the values back to their initial state should force the thread-local storage
  // to initialize and do any required memory allocation)
  gtls_rcutils_thread_local_initialized = true;
  rcutils_reset_error();
  RCUTILS_SET_ERROR_MSG("no error - initializing thread-local storage");
  rcutils_error_string_t throw_away = rcutils_get_error_string();
  (void)throw_away;
  rcutils_reset_error();

  // 此时线程本地分配器、错误状态和错误字符串都已初始化
  // (At this point the thread-local allocator, error state, and error string are all initialized)
  return RCUTILS_RET_OK;
}

/**
 * @brief 比较两个字符串是否相同。
 * @param str1 字符串1。
 * @param str2 字符串2。
 * @param count 要比较的字符数。
 * @return bool 如果相同则返回 true，否则返回 false。
 *
 * @brief Compare whether two strings are the same.
 * @param str1 String 1.
 * @param str2 String 2.
 * @param count The number of characters to compare.
 * @return bool If the same, return true, otherwise return false.
 */
static bool __same_string(const char * str1, const char * str2, size_t count)
{
  // 断言字符串不为空
  // (Assert that the strings are not null)
  assert(NULL != str1);
  assert(NULL != str2);
  // 如果字符串相同或者比较结果为0，则认为是相同的字符串
  // (If the strings are the same or the comparison result is 0, they are considered the same)
  return str1 == str2 || 0 == strncmp(str1, str2, count);
}

/**
 * @brief Formats an overwriting error state message.
 *
 * @param[in] buffer Buffer to store the formatted error message.
 * @param[in] buffer_size Size of the provided buffer.
 * @param[in] new_error_state Pointer to the new error state that is overwriting the previous one.
 */
static void __format_overwriting_error_state_message(
  char * buffer, size_t buffer_size, const rcutils_error_state_t * new_error_state)
{
  // 确保 buffer 不为空 (Ensure buffer is not NULL)
  assert(NULL != buffer);
  // 确保 buffer_size 不为 0 (Ensure buffer_size is not 0)
  assert(0 != buffer_size);
  // 确保 buffer_size 小于 SIZE_MAX (Ensure buffer_size is less than SIZE_MAX)
  assert(SIZE_MAX > buffer_size);
  // 确保 new_error_state 不为空 (Ensure new_error_state is not NULL)
  assert(NULL != new_error_state);

  int64_t bytes_left = (int64_t)buffer_size;
  do {
    char * offset = buffer;
    size_t written = 0;

    // 写入错误消息的第一部分 (Write the first static part of the error message)
    written = __rcutils_copy_string(
      offset, (size_t)bytes_left,
      "\n"
      ">>> [rcutils|error_handling.c:" RCUTILS_STRINGIFY(
        __LINE__) "] rcutils_set_error_state()\n"
                  "This error state is being overwritten:\n"
                  "\n"
                  "  '");
    offset += written;
    bytes_left -= (int64_t)written;
    if (0 >= bytes_left) {
      break;
    }

    // 写入旧错误字符串 (Write the old error string)
    rcutils_error_string_t old_error_string = rcutils_get_error_string();
    written = __rcutils_copy_string(offset, sizeof(old_error_string.str), old_error_string.str);
    offset += written;
    bytes_left -= (int64_t)written;
    if (0 >= bytes_left) {
      break;
    }

    // 写入错误状态消息的中间部分 (Write the middle part of the state error message)
    written = __rcutils_copy_string(
      offset, (size_t)bytes_left,
      "'\n"
      "\n"
      "with this new error message:\n"
      "\n"
      "  '");
    offset += written;
    bytes_left -= (int64_t)written;
    if (0 >= bytes_left) {
      break;
    }

    // 格式化新错误状态的错误字符串并写入 (Format error string for new error state and write it in)
    rcutils_error_string_t new_error_string = {.str = "\0"};
    __rcutils_format_error_string(&new_error_string, new_error_state);
    written = __rcutils_copy_string(offset, sizeof(new_error_string.str), new_error_string.str);
    offset += written;
    bytes_left -= (int64_t)written;
    if (0 >= bytes_left) {
      break;
    }

    // 写入错误状态消息的最后一部分 (Write the last part of the state error message)
    written = __rcutils_copy_string(
      offset, (size_t)bytes_left,
      "'\n"
      "\n"
      "rcutils_reset_error() should be called after error handling to avoid this.\n"
      "<<<\n");
    bytes_left -= (int64_t)written;
  } while (0);

#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
  // 注意，由于上述截断的方式和在 rcutils_set_error_state() 中与此函数一起使用的输出缓冲区的大小，
  // 这应该永远不会评估为 true，但它在这里是防御性的，并尝试更早地捕获此文件中的编程错误。
  // (Note that due to the way truncating is done above, and the size of the
  // output buffer used with this function in rcutils_set_error_state() below,
  // this should never evaluate to true, but it's here to be defensive and try
  // to catch programming mistakes in this file earlier.)
  if (0 >= bytes_left) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("[rcutils|error_handling.c:" RCUTILS_STRINGIFY(
      __LINE__) "] rcutils_set_error_state() following error message was too long and will be "
                "truncated\n");
  }
#else
  (void)
    bytes_left; // 避免在此情况下出现范围可能减少的警告 (avoid scope could be reduced warning if in this case)
#endif
}

/**
 * @brief 设置错误状态信息 (Set error state information)
 *
 * @param[in] error_string 错误信息字符串 (Error message string)
 * @param[in] file 发生错误的文件名 (File name where the error occurred)
 * @param[in] line_number 发生错误的行号 (Line number where the error occurred)
 */
void rcutils_set_error_state(const char * error_string, const char * file, size_t line_number)
{
  rcutils_error_state_t error_state;

  // 如果 error_string 为空，则不设置错误状态 (If error_string is NULL, do not set the error state)
  if (NULL == error_string) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    RCUTILS_SAFE_FWRITE_TO_STDERR("[rcutils|error_handling.c:" RCUTILS_STRINGIFY(
      __LINE__) "] rcutils_set_error_state() given null pointer for error_string, error was not "
                "set\n");
#endif
    return;
  }

  // 如果 file 为空，则不设置错误状态 (If file is NULL, do not set the error state)
  if (NULL == file) {
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
    RCUTILS_SAFE_FWRITE_TO_STDERR("[rcutils|error_handling.c:" RCUTILS_STRINGIFY(
      __LINE__) "] rcutils_set_error_state() given null pointer for file string, error was not "
                "set\n");
#endif
    return;
  }

  // 复制 error_string 到 error_state.message (Copy error_string to error_state.message)
  __rcutils_copy_string(error_state.message, sizeof(error_state.message), error_string);
  // 复制 file 到 error_state.file (Copy file to error_state.file)
  __rcutils_copy_string(error_state.file, sizeof(error_state.file), file);
  // 设置 error_state.line_number (Set error_state.line_number)
  error_state.line_number = line_number;
#if RCUTILS_REPORT_ERROR_HANDLING_ERRORS
  // 如果新错误与旧错误不同，则仅在覆盖时发出警告 (Only warn of overwriting if the new error is different from the old ones)
  size_t characters_to_compare = strnlen(error_string, RCUTILS_ERROR_MESSAGE_MAX_LENGTH);
  // 假设 message 长度小于等于最大错误字符串长度 (Assumption is that message length is <= max error string length)
  static_assert(
    sizeof(gtls_rcutils_error_state.message) <= sizeof(gtls_rcutils_error_string.str),
    "expected error state's max message length to be less than or equal to error string max");
  if (
    gtls_rcutils_error_is_set &&
    !__same_string(error_string, gtls_rcutils_error_string.str, characters_to_compare) &&
    !__same_string(error_string, gtls_rcutils_error_state.message, characters_to_compare)) {
    char output_buffer[4096];
    __format_overwriting_error_state_message(output_buffer, sizeof(output_buffer), &error_state);
    RCUTILS_SAFE_FWRITE_TO_STDERR(output_buffer);
  }
#endif
  // 将 error_state 设置为全局变量 (Set error_state as a global variable)
  gtls_rcutils_error_state = error_state;
  // 设置错误字符串未格式化标志 (Set the error string unformatted flag)
  gtls_rcutils_error_string_is_formatted = false;
  // 初始化错误字符串 (Initialize the error string)
  gtls_rcutils_error_string = (const rcutils_error_string_t){.str = "\0"};
  // 设置错误已设置标志 (Set the error set flag)
  gtls_rcutils_error_is_set = true;
}

/**
 * @brief 检查是否设置了错误 (Check if an error is set)
 *
 * @return 如果错误已设置，则返回 true，否则返回 false (Returns true if an error is set, false otherwise)
 */
bool rcutils_error_is_set(void) { return gtls_rcutils_error_is_set; }

/**
 * @brief 获取当前的错误状态 (Get the current error state)
 *
 * @return 返回当前的错误状态指针 (Returns a pointer to the current error state)
 */
const rcutils_error_state_t * rcutils_get_error_state(void) { return &gtls_rcutils_error_state; }

/**
 * @brief 获取当前的错误字符串 (Get the current error string)
 *
 * @return 返回当前的错误字符串 (Returns the current error string)
 */
rcutils_error_string_t rcutils_get_error_string(void)
{
  // 如果没有设置错误，则返回 "error not set" (If no error is set, return "error not set")
  if (!gtls_rcutils_error_is_set) {
    return (rcutils_error_string_t){"error not set"}; // NOLINT(readability/braces)
  }
  // 如果错误字符串尚未格式化，则格式化它 (If the error string is not yet formatted, format it)
  if (!gtls_rcutils_error_string_is_formatted) {
    __rcutils_format_error_string(&gtls_rcutils_error_string, &gtls_rcutils_error_state);
    gtls_rcutils_error_string_is_formatted = true;
  }
  // 返回格式化后的错误字符串 (Return the formatted error string)
  return gtls_rcutils_error_string;
}

/**
 * @brief 重置当前的错误状态和字符串 (Reset the current error state and string)
 */
void rcutils_reset_error(void)
{
  // 重置错误状态为默认值 (Reset the error state to default values)
  gtls_rcutils_error_state = (const rcutils_error_state_t){
    .message = {0}, .file = {0}, .line_number = 0}; // NOLINT(readability/braces)
  // 将错误字符串标记为未格式化 (Mark the error string as unformatted)
  gtls_rcutils_error_string_is_formatted = false;
  // 重置错误字符串为默认值 (Reset the error string to default value)
  gtls_rcutils_error_string = (const rcutils_error_string_t){.str = "\0"};
  // 设置错误状态为未设置 (Set the error state as unset)
  gtls_rcutils_error_is_set = false;
}

#ifdef __cplusplus
}
#endif
