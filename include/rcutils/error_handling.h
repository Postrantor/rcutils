// Copyright 2014 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__ERROR_HANDLING_H_
#define RCUTILS__ERROR_HANDLING_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1 // indicate we would like strnlen_s if available
#endif
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/snprintf.h"
#include "rcutils/testing/fault_injection.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

#ifdef __STDC_LIB_EXT1__
/// 将给定的 msg 写入 stderr，限制 `fwrite` 中的缓冲区大小。
/// Write the given msg out to stderr, limiting the buffer size in the `fwrite`.
/**
 * 如果 `msg` 没有以空字符结尾，这样可以确保缓冲区溢出的上限。
 * This ensures that there is an upper bound to a buffer overrun if `msg` is
 * non-null terminated.
 */
#define RCUTILS_SAFE_FWRITE_TO_STDERR(msg)                                                         \
  do {                                                                                             \
    fwrite(msg, sizeof(char), strnlen_s(msg, 4096), stderr);                                       \
  } while (0)
#else
/// 将给定的 msg 写入 stderr。
/// Write the given msg out to stderr.
#define RCUTILS_SAFE_FWRITE_TO_STDERR(msg)                                                         \
  do {                                                                                             \
    fwrite(msg, sizeof(char), strlen(msg), stderr);                                                \
  } while (0)
#endif

/// 使用格式字符串和格式参数将错误消息设置为 stderr。
/// Set the error message to stderr using a format string and format arguments.
/**
 * 此函数使用给定的格式字符串将错误消息设置为 stderr。
 * The resulting formatted string is silently truncated at
 * RCUTILS_ERROR_MESSAGE_MAX_LENGTH.
 *
 * \param[in] format_string 用作错误消息格式的字符串。
 * \param[in] format_string The string to be used as the format of the error message.
 * \param[in] ... 格式字符串的参数。
 * \param[in] ... Arguments for the format string.
 */
#define RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(format_string, ...)                       \
  do {                                                                                             \
    char output_msg[RCUTILS_ERROR_MESSAGE_MAX_LENGTH];                                             \
    int ret = rcutils_snprintf(output_msg, sizeof(output_msg), format_string, __VA_ARGS__);        \
    if (ret < 0) {                                                                                 \
      RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to call snprintf for error message formatting\n");     \
    } else {                                                                                       \
      RCUTILS_SAFE_FWRITE_TO_STDERR(output_msg);                                                   \
    }                                                                                              \
  } while (0)

/// 允许格式化数字的最大长度。
/// The maximum length a formatted number is allowed to have.
#define RCUTILS_ERROR_STATE_LINE_NUMBER_STR_MAX_LENGTH 20 // "18446744073709551615"

/// 允许的最大格式字符数。
/// The maximum number of formatting characters allowed.
#define RCUTILS_ERROR_FORMATTING_CHARACTERS 6 // ', at ' + ':'

/// 最大格式化字符串长度。
/// The maximum formatted string length.
#define RCUTILS_ERROR_MESSAGE_MAX_LENGTH 1024

/// 用户定义错误消息的最大长度
/// The maximum length for user defined error message
/**
 * 请记住，"链接" 错误将包括先前指定的文件路径
 * 例如 "some error, at /path/to/a.c:42, at /path/to/b.c:42"
 * Remember that "chained" errors will include previously specified file paths
 * e.g. "some error, at /path/to/a.c:42, at /path/to/b.c:42"
 */
#define RCUTILS_ERROR_STATE_MESSAGE_MAX_LENGTH 768

/// 计算的文件名的最大长度。
/// The calculated maximum length for the filename.
/**
 * 使用 RCUTILS_ERROR_STATE_MESSAGE_MAX_LENGTH = 768，RCUTILS_ERROR_STATE_FILE_MAX_LENGTH == 229
 * With RCUTILS_ERROR_STATE_MESSAGE_MAX_LENGTH = 768, RCUTILS_ERROR_STATE_FILE_MAX_LENGTH == 229
 */
#define RCUTILS_ERROR_STATE_FILE_MAX_LENGTH                                                        \
  (RCUTILS_ERROR_MESSAGE_MAX_LENGTH - RCUTILS_ERROR_STATE_MESSAGE_MAX_LENGTH -                     \
   RCUTILS_ERROR_STATE_LINE_NUMBER_STR_MAX_LENGTH - RCUTILS_ERROR_FORMATTING_CHARACTERS - 1)

/// \brief 结构体包装一个固定大小的c字符串，用于返回格式化的错误字符串。
/// Struct wrapping a fixed-size c string used for returning the formatted error string.
typedef struct rcutils_error_string_s
{
  /// \brief 用于返回格式化错误字符串的固定大小C字符串。
  /// The fixed-size C string used for returning the formatted error string.
  char str[RCUTILS_ERROR_MESSAGE_MAX_LENGTH];
} rcutils_error_string_t;

/// \brief 封装了通过 RCUTILS_SET_ERROR_MSG() 设置的错误状态的结构体。
/// Struct which encapsulates the error state set by RCUTILS_SET_ERROR_MSG().
typedef struct rcutils_error_state_s
{
  /// \brief 用户消息存储，限制为 RCUTILS_ERROR_STATE_MESSAGE_MAX_LENGTH 个字符。
  /// User message storage, limited to RCUTILS_ERROR_STATE_MESSAGE_MAX_LENGTH characters.
  char message[RCUTILS_ERROR_STATE_MESSAGE_MAX_LENGTH];

  /// \brief 文件名，限制为从 RCUTILS_ERROR_STATE_MAX_SIZE 字符中减去其他存储后剩余的长度。
  /// File name, limited to what's left from RCUTILS_ERROR_STATE_MAX_SIZE characters
  /// after subtracting storage for others.
  char file[RCUTILS_ERROR_STATE_FILE_MAX_LENGTH];

  /// \brief 错误所在行号。
  /// Line number of error.
  uint64_t line_number;
} rcutils_error_state_t;

// 确保我们的计算是正确的...
// make sure our math is right...
#if __STDC_VERSION__ >= 201112L
static_assert(
  sizeof(rcutils_error_string_t) ==
    (RCUTILS_ERROR_STATE_MESSAGE_MAX_LENGTH + RCUTILS_ERROR_STATE_FILE_MAX_LENGTH +
     RCUTILS_ERROR_STATE_LINE_NUMBER_STR_MAX_LENGTH + RCUTILS_ERROR_FORMATTING_CHARACTERS +
     1 /* null terminating character */),
  "Maximum length calculations incorrect");
#endif

/// 强制初始化线程局部存储，如果在新创建的线程中调用。
/// Forces initialization of thread-local storage if called in a newly created thread.
/**
 * 如果没有事先调用此函数，那么在第一次设置错误状态或者第一次检索错误消息时，
 * 将使用默认分配器分配线程局部存储。
 * If this function is not called beforehand, then the first time the error
 * state is set or the first time the error message is retrieved, the default
 * allocator will be used to allocate thread-local storage.
 *
 * 此函数可能会也可能不会分配内存。
 * 系统的线程局部存储实现可能需要分配内存，因为它通常无法知道在不知道将创建多少个线程的情况下需要多少存储空间。
 * This function may or may not allocate memory.
 * The system's thread-local storage implementation may need to allocate
 * memory, since it usually has no way of knowing how much storage is needed
 * without knowing how many threads will be created.
 * 大多数实现（例如 C11、C++11 和 pthread）没有办法指定如何分配这些内存，
 * 但是如果实现允许，给定的分配器将被用于此功能，否则不使用。
 * Most implementations (e.g. C11, C++11, and pthread) do not have ways to
 * specify how this memory is allocated, but if the implementation allows, the
 * given allocator to this function will be used, but is otherwise unused.
 * 这只发生在创建和销毁线程时，可以通过重用线程池来避免在“稳定”状态下发生。
 * This only occurs when creating and destroying threads, which can be avoided
 * in the "steady" state by reusing pools of threads.
 *
 * 值得考虑的是，重复创建和销毁线程将导致重复分配内存，并可能导致内存碎片化。
 * 通常通过使用线程池来避免这种情况。
 * It is worth considering that repeated thread creation and destruction will
 * result in repeated memory allocations and could result in memory
 * fragmentation.
 * This is typically avoided anyways by using pools of threads.
 *
 * 如果返回代码指示错误，则不会设置错误消息。
 * In case an error is indicated by the return code, no error message will have
 * been set.
 *
 * 如果在一个线程中多次调用，或者在隐式初始化后设置错误状态，
 * 即使给定的分配器无效，也会返回 `RCUTILS_RET_OK`。
 * If called more than once in a thread, or after implicitly initialized by
 * setting the error state, it will still return `RCUTILS_RET_OK`, even
 * if the given allocator is invalid.
 * 本质上，如果已经调用了线程局部存储，此函数不起作用。
 * Essentially this function does nothing if thread-local storage has already
 * been called.
 * 如果已经初始化，即使给定的分配器与最初用于初始化线程局部存储的分配器不匹配，也会忽略它。
 * If already initialized, the given allocator is ignored, even if it does not
 * match the allocator used originally to initialize the thread-local storage.
 *
 * \param[in] allocator 要用于分配和释放内存的分配器
 * \param[in] allocator to be used to allocate and deallocate memory
 * \return #RCUTILS_RET_OK 如果成功，或者
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果分配器无效，或者
 * \return #RCUTILS_RET_INVALID_ARGUMENT if the allocator is invalid, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果分配内存失败，或者
 * \return #RCUTILS_RET_BAD_ALLOC if allocating memory fails, or
 * \return #RCUTILS_RET_ERROR 如果发生未指定的错误。
 * \return #RCUTILS_RET_ERROR if an unspecified error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_initialize_error_handling_thread_local_storage(rcutils_allocator_t allocator);

/// 设置错误消息以及发生错误的文件和行。
/// Set the error message, as well as the file and line on which it occurred.
/**
 * 这不是用来直接使用的，而是通过 RCUTILS_SET_ERROR_MSG(msg) 宏来使用。
 * This is not meant to be used directly, but instead via the
 * RCUTILS_SET_ERROR_MSG(msg) macro.
 *
 * error_msg 参数被复制到内部错误存储中，必须以空字符结尾。
 * file 参数被复制到内部错误存储中，必须以空字符结尾。
 * The error_msg parameter is copied into the internal error storage and must
 * be null terminated.
 * The file parameter is copied into the internal error storage and must
 * be null terminated.
 *
 * \param[in] error_string 要设置的错误消息。
 * \param[in] error_string The error message to set.
 * \param[in] file 发生错误的文件路径。
 * \param[in] file The path to the file in which the error occurred.
 * \param[in] line_number 发生错误的行号。
 * \param[in] line_number The line number on which the error occurred.
 */
RCUTILS_PUBLIC
void rcutils_set_error_state(const char * error_string, const char * file, size_t line_number);

/// 检查参数是否为空值。
/// Check an argument for a null value.
/**
 * 如果参数值为 `NULL`，则设置错误消息并返回 `error_return_type`。
 * If the argument's value is `NULL`, set the error message saying so and
 * return the `error_return_type`.
 *
 * \param[in] argument 要测试的参数。
 * \param[in] argument The argument to test.
 * \param[in] error_return_type 如果参数为 `NULL`，返回的类型。
 * \param[in] error_return_type The type to return if the argument is `NULL`.
 */
#define RCUTILS_CHECK_ARGUMENT_FOR_NULL(argument, error_return_type)                               \
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(argument, #argument " argument is null", return error_return_type)

/// 检查值是否为空，附带错误消息和错误语句。
/// Check a value for null, with an error message and error statement.
/**
 * 如果 `value` 为 `NULL`，在设置错误消息后将评估错误语句。
 * If `value` is `NULL`, the error statement will be evaluated after
 * setting the error message.
 *
 * \param[in] value 要测试的值。
 * \param[in] value The value to test.
 * \param[in] msg 如果 `value` 为 `NULL`，则为错误消息。
 * \param[in] msg The error message if `value` is `NULL`.
 * \param[in] error_statement 如果 `value` 为 `NULL`，要评估的语句。
 * \param[in] error_statement The statement to evaluate if `value` is `NULL`.
 */
#define RCUTILS_CHECK_FOR_NULL_WITH_MSG(value, msg, error_statement)                               \
  do {                                                                                             \
    if (NULL == value) {                                                                           \
// 设置错误消息                                                                              \
// Set the error message
RCUTILS_SET_ERROR_MSG(msg); // 执行错误语句
// Execute the error statement
error_statement;
}
}
while (0)

/// 设置错误消息，并附加当前文件和行号。
/// Set the error message, as well as append the current file and line number.
/**
 * 如果之前设置了错误消息，且此后未调用 rcutils_reset_error()，并且使用 RCUTILS_REPORT_ERROR_HANDLING_ERRORS
 * 构建了此库，则之前设置的错误消息将打印到 stderr。错误状态存储是线程局部的，因此所有与错误相关的函数也是线程局部的。
 * If an error message was previously set, and rcutils_reset_error() was not called
 * afterwards, and this library was built with RCUTILS_REPORT_ERROR_HANDLING_ERRORS
 * turned on, then the previously set error message will be printed to stderr.
 * Error state storage is thread local and so all error related functions are
 * also thread local.
 *
 * \param[in] msg 要设置的错误消息。
 * \param[in] msg The error message to be set.
 */
#define RCUTILS_SET_ERROR_MSG(msg)                                                                 \
  do {                                                                                             \
  // 设置错误状态，包括消息、文件名和行号                                        \
  // Set the error state, including message, filename, and line number
  rcutils_set_error_state(msg, __FILE__, __LINE__);
}
while (0)

/// 设置错误消息，使用格式字符串和格式参数。
/**
 * 此函数使用给定的格式字符串设置错误消息。
 * 结果格式化字符串在 RCUTILS_ERROR_MESSAGE_MAX_LENGTH 处静默截断。
 *
 * Set the error message using the given format string.
 * The resulting formatted string is silently truncated at
 * RCUTILS_ERROR_MESSAGE_MAX_LENGTH.
 *
 * \param[in] format_string 用作错误消息格式的字符串。
 * \param[in] ... 格式字符串的参数。
 */
#define RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(format_string, ...)                               \
  do {                                                                                             \
    char output_msg[RCUTILS_ERROR_MESSAGE_MAX_LENGTH];                                             \
  // 调用 rcutils_snprintf 函数将格式化的字符串存储到 output_msg 中                \
  // Call rcutils_snprintf function to store the formatted string in output_msg
  int ret = rcutils_snprintf(output_msg, sizeof(output_msg), format_string, __VA_ARGS__);
if (ret < 0) { // 如果调用失败，则输出错误信息到标准错误流
  // If the call fails, output the error message to the standard error stream
  RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to call snprintf for error message formatting\n");
} else { // 设置错误消息为 output_msg
  // Set the error message to output_msg
  RCUTILS_SET_ERROR_MSG(output_msg);
}
}
while (0)

/// 表示该函数打算设置错误消息并返回错误值。
/**
 * \def RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF
 * 类似于 RCUTILS_CAN_RETURN_WITH_ERROR_OF 的指示宏，还设置错误消息。
 *
 * Indicating macro similar to RCUTILS_CAN_RETURN_WITH_ERROR_OF, that also sets an error
 * message.
 *
 * 目前，此宏简单地依赖 `RCUTILS_CAN_FAIL_WITH` 设置通用错误消息，并在启用故障注入时返回给定的 `error_return_value`。
 * For now, this macro simply relies on `RCUTILS_CAN_FAIL_WITH` to set a generic error
 * message and return the given `error_return_value` if fault injection is enabled.
 *
 * \param error_return_value 由于给定错误而返回的值。
 */
#define RCUTILS_CAN_SET_MSG_AND_RETURN_WITH_ERROR_OF(error_return_value)                           \
  RCUTILS_CAN_FAIL_WITH({                                                                          \
    // 设置错误消息并注入 error_return_value
    // Set the error message and inject error_return_value
  RCUTILS_SET_ERROR_MSG("Injecting " RCUTILS_STRINGIFY(error_return_value));
return error_return_value;
})

/// 如果错误已设置，则返回 `true`，否则返回 `false`。
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool rcutils_error_is_set(void);

/// 返回通过 rcutils_set_error_state() 设置的 rcutils_error_state_t。
/**
 * 如果在此线程中没有设置错误，则返回的指针将为 NULL。
 * The returned pointer will be NULL if no error has been set in this thread.
 *
 * 在同一线程中调用 RCUTILS_SET_ERROR_MSG、rcutils_set_error_state 或 rcutils_reset_error 之前，返回的指针都是有效的。
 * The returned pointer is valid until RCUTILS_SET_ERROR_MSG, rcutils_set_error_state,
 * or rcutils_reset_error are called in the same thread.
 *
 * \return 指向当前错误状态结构的指针。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
const rcutils_error_state_t * rcutils_get_error_state(void);

/// 返回错误消息，如果设置了 `<file>:<line>`，则在后面添加 `, at <file>:<line>`，否则返回 "error not set"。
/// Return the error message followed by `, at <file>:<line>` if set, else "error not set".
/**
 * 该函数是"安全的"，因为它返回当前错误字符串的副本，或者如果没有设置错误，则返回包含字符串 "error not set" 的副本。
 * This function is "safe" because it returns a copy of the current error
 * string or one containing the string "error not set" if no error was set.
 * 这确保副本由调用线程拥有，因此不会被其他错误处理调用无效，并且其中的 C 字符串始终有效且以空字符结尾。
 * This ensures that the copy is owned by the calling thread and is therefore
 * never invalidated by other error handling calls, and that the C string
 * inside is always valid and null terminated.
 *
 * \return 带有文件和行号的当前错误字符串，如果未设置，则返回 "error not set"。
 * \return The current error string, with file and line number, or "error not set" if not set.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_error_string_t rcutils_get_error_string(void);

/// 通过清除之前设置的任何错误状态来重置错误状态。
/// Reset the error state by clearing any previously set error state.
RCUTILS_PUBLIC
void rcutils_reset_error(void);

/// 使用 RCUTILS_SET_ERROR_MSG 设置错误消息并附加上一个错误。
/// Set the error message using RCUTILS_SET_ERROR_MSG and append the previous error.
/**
 * 如果没有之前的错误，与 RCUTILS_SET_ERROR_MSG 的行为相同。
 * If there is no previous error, has same behavior as RCUTILS_SET_ERROR_MSG.
 * \param[in] msg 要设置的错误消息。
 * \param[in] msg The error message to be set.
 */
#define RCUTILS_SET_ERROR_MSG_AND_APPEND_PREV_ERROR(msg)                                           \
  do {                                                                                             \
    rcutils_error_string_t error_string = rcutils_get_error_string();                              \
    rcutils_reset_error();                                                                         \
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(RCUTILS_EXPAND(msg ": %s"), error_string.str);        \
  } while (0)

/// 使用 RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING 设置错误消息并附加上一个错误。
/// Set the error message with RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING and append the previous
/// error.
/**
 * 该函数使用给定的格式字符串设置错误消息，并附加并重置最新的错误字符串。
 * This function sets the error message using the given format string, and appends and resets the
 * latest error string.
 * 结果格式化字符串在 RCUTILS_ERROR_MESSAGE_MAX_LENGTH 处静默截断。
 * The resulting formatted string is silently truncated at RCUTILS_ERROR_MESSAGE_MAX_LENGTH.
 *
 * 如果没有之前的错误，与 RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING 的行为相同。
 * If there is no previous error, has same behavior as RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING.
 *
 * \param[in] format_string 用作错误消息格式的字符串。
 * \param[in] format_string The string to be used as the format of the error message.
 * \param[in] ... 格式字符串的参数。
 * \param[in] ... Arguments for the format string.
 */
#define RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING_AND_APPEND_PREV_ERROR(format_string, ...)         \
  do {                                                                                             \
    rcutils_error_string_t error_string = rcutils_get_error_string();                              \
    rcutils_reset_error();                                                                         \
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(                                                      \
      RCUTILS_EXPAND(format_string ": %s"), __VA_ARGS__, error_string.str);                        \
  } while (0)

/// 将给定的 msg 写入 stderr，限制 `fwrite` 中的缓冲区大小，并附加之前的错误。
/// Write the given msg out to stderr, limiting the buffer size in the `fwrite`, appending the
/// previous error.
/**
 * 如果存在，此操作将重置先前的错误。
 * This will reset the previous error, if it exists.
 * 如果没有先前的错误，行为与 RCUTILS_SAFE_FWRITE_TO_STDERR 相同。
 * If there is no previous error, has same behavior as RCUTILS_SAFE_FWRITE_TO_STDERR.
 */
#define RCUTILS_SAFE_FWRITE_TO_STDERR_AND_APPEND_PREV_ERROR(msg)                                   \
  do {                                                                                             \
// 获取当前错误字符串                                                                     \
// Get the current error string
rcutils_error_string_t error_string = rcutils_get_error_string(); // 重置错误
// Reset the error
rcutils_reset_error(); // 将 msg 安全地写入到 stderr
// Safely write msg to stderr
RCUTILS_SAFE_FWRITE_TO_STDERR(msg); // 使用格式化字符串将错误字符串写入到 stderr
// Write the error string to stderr with a format string
RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(": %s", error_string.str);
}
while (0)

/// 使用格式字符串和格式参数将错误消息设置为 stderr，并附加先前的错误。
/// Set the error message to stderr using a format string and format arguments, appending the
/// previous error.
/**
 * 此函数使用给定的格式字符串将错误消息设置为 stderr，附加并重置先前的错误。
 * This function sets the error message to stderr using the given format string, appending and
 * resetting the previous error.
 * 结果格式化字符串在 RCUTILS_ERROR_MESSAGE_MAX_LENGTH 处静默截断。
 * The resulting formatted string is silently truncated at RCUTILS_ERROR_MESSAGE_MAX_LENGTH.
 *
 * 如果存在，此操作将重置先前的错误。
 * This will reset the previous error, if it exists.
 * 如果没有先前的错误，行为与 RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING 相同。
 * If there is no previous error, has same behavior as
 * RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING.
 *
 * \param[in] format_string 用作错误消息格式的字符串。
 * \param[in] format_string The string to be used as the format of the error message.
 * \param[in] ... 格式字符串的参数。
 * \param[in] ... Arguments for the format string.
 */
#define RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING_AND_APPEND_PREV_ERROR(format_string, ...) \
  do {                                                                                             \
  // 获取当前错误字符串                                                                   \
  // Get the current error string
  rcutils_error_string_t error_string = rcutils_get_error_string(); // 重置错误
// Reset the error
rcutils_reset_error(); // 使用格式字符串和参数将消息安全地写入到 stderr
// Safely write the message to stderr with a format string and arguments
RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
  format_string, __VA_ARGS__); // 使用格式化字符串将错误字符串写入到 stderr
// Write the error string to stderr with a format string
RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(": %s", error_string.str);
}
while (0)

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__ERROR_HANDLING_H_
