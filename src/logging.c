// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
// When building with MSVC 19.28.29333.0 on Windows 10 (as of 2020-11-11),
// there appears to be a problem with winbase.h (which is included by
// Windows.h).  In particular, warnings of the form:
//
// warning C5105: macro expansion producing 'defined' has undefined behavior
//
// See https://developercommunity.visualstudio.com/content/problem/695656/wdk-and-sdk-are-not-compatible-with-experimentalpr.html
// for more information.  For now disable that warning when including windows.h
#pragma warning(push)
#pragma warning(disable : 5105)
#include <windows.h>
#pragma warning(pop)
#else
#include <unistd.h>
#endif

#include "rcutils/allocator.h"
#include "rcutils/env.h"
#include "rcutils/error_handling.h"
#include "rcutils/find.h"
#include "rcutils/format_string.h"
#include "rcutils/logging.h"
#include "rcutils/snprintf.h"
#include "rcutils/strdup.h"
#include "rcutils/strerror.h"
#include "rcutils/time.h"
#include "rcutils/types/hash_map.h"

/**
 * @brief 定义日志分隔符字符
 * @details 用于在日志输出中分隔不同部分的字符。
 *
 * Define the log separator character.
 * This character is used to separate different parts in the log output.
 */
#define RCUTILS_LOGGING_SEPARATOR_CHAR '.'

/**
 * @brief 定义日志最大输出格式长度
 * @details 日志输出格式字符串的最大允许长度。
 *
 * Define the maximum length for log output format.
 * The maximum allowed length for the log output format string.
 */
#define RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN (2048)

#if defined(_WIN32)
/**
 * @brief 定义日志流缓冲区大小（Windows）
 * @details 在 Windows 系统上，使用 setvbuf 设置缓冲区大小。大小必须为 2 <= size <= INT_MAX。
 *          更多信息，请参阅：https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setvbuf
 *
 * Define the log stream buffer size (Windows).
 * On Windows systems, the buffer size is set using setvbuf, and its size must be 2 <= size <= INT_MAX.
 * For more information, see: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setvbuf
 */
#define RCUTILS_LOGGING_STREAM_BUFFER_SIZE (BUFSIZ)
#else
/**
 * @brief 定义日志流缓冲区大小（非 Windows）
 * @details 在非 Windows 系统上，让系统选择合适的大小。
 *          glibc 参考：https://sourceware.org/git/?p=glibc.git;a=blob;f=libio/iosetvbuf.c;hb=HEAD
 *          OSX 参考：https://opensource.apple.com/source/Libc/Libc-166/stdio.subproj/setvbuf.c.auto.html
 *
 * Define the log stream buffer size (non-Windows).
 * On non-Windows systems, let the system choose the appropriate size.
 * glibc reference: https://sourceware.org/git/?p=glibc.git;a=blob;f=libio/iosetvbuf.c;hb=HEAD
 * OSX reference: https://opensource.apple.com/source/Libc/Libc-166/stdio.subproj/setvbuf.c.auto.html
 */
#define RCUTILS_LOGGING_STREAM_BUFFER_SIZE (0)
#endif

/**
 * @brief 日志严重性名称数组
 * @details 用于将日志严重性枚举值转换为对应的字符串表示。
 *
 * Log severity names array.
 * Used for converting log severity enum values to their corresponding string representations.
 */
const char * const g_rcutils_log_severity_names[] = {
  [RCUTILS_LOG_SEVERITY_UNSET] = "UNSET", ///< 未设置的日志严重性 / Unset log severity
  [RCUTILS_LOG_SEVERITY_DEBUG] = "DEBUG", ///< 调试级别日志 / Debug level log
  [RCUTILS_LOG_SEVERITY_INFO] = "INFO",   ///< 信息级别日志 / Info level log
  [RCUTILS_LOG_SEVERITY_WARN] = "WARN",   ///< 警告级别日志 / Warning level log
  [RCUTILS_LOG_SEVERITY_ERROR] = "ERROR", ///< 错误级别日志 / Error level log
  [RCUTILS_LOG_SEVERITY_FATAL] = "FATAL", ///< 致命级别日志 / Fatal level log
};

/**
 * @brief 彩色输出枚举
 * @details 用于控制日志输出是否使用彩色文本。
 *
 * Colorized output enumeration.
 * Used for controlling whether the log output uses colorized text or not.
 */
enum rcutils_colorized_output {
  RCUTILS_COLORIZED_OUTPUT_FORCE_DISABLE = 0, ///< 强制禁用彩色输出 / Force disable colorized output
  RCUTILS_COLORIZED_OUTPUT_FORCE_ENABLE = 1, ///< 强制启用彩色输出 / Force enable colorized output
  RCUTILS_COLORIZED_OUTPUT_AUTO =
    2, ///< 自动选择是否使用彩色输出 / Automatically decide whether to use colorized output or not
};

/**
 * @file logging.c
 *
 * @brief ROS2 RCLCPP logging module.
 */

// 声明一个全局变量，表示日志系统是否已经初始化（Declare a global variable indicating whether the logging system has been initialized）
bool g_rcutils_logging_initialized = false;

// 定义一个字符数组，用于存储日志输出格式字符串（Define a character array to store the log output format string）
static char g_rcutils_logging_output_format_string[RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN];

// 默认的日志输出格式（Default log output format）
static const char * g_rcutils_logging_default_output_format =
  "[{severity}] [{time}] [{name}]: {message}";

// 日志模块使用的内存分配器（Memory allocator used by the logging module）
static rcutils_allocator_t g_rcutils_logging_allocator;

// 自定义日志输出处理函数（Custom log output handler function）
static rcutils_logging_output_handler_t g_rcutils_logging_output_handler = NULL;

// 存储日志级别的哈希映射（Hash map for storing log levels）
static rcutils_hash_map_t g_rcutils_logging_severities_map;

// 如果此值为 false，则跳过对 severities 映射的使用。如果在初始化时映射分配失败，则可能发生这种情况。
// (If this value is false, attempts to use the severities map will be skipped. This can happen if the allocation of the map fails during initialization.)
static bool g_rcutils_logging_severities_map_valid = false;

// 默认的日志记录器级别（Default logger level）
static int g_rcutils_logging_default_logger_level = 0;

// 输出流文件指针（Output stream file pointer）
static FILE * g_output_stream = NULL;

// 彩色输出设置（Colorized output setting）
static enum rcutils_colorized_output g_colorized_output = RCUTILS_COLORIZED_OUTPUT_AUTO;

/**
 * @struct logging_input_s
 * @brief 日志输入结构体 (Logging input structure)
 * 
 * @param name 名称 (Name)
 * @param location 日志位置 (Log location)
 * @param msg 日志消息 (Log message)
 * @param severity 日志严重性级别 (Log severity level)
 * @param timestamp 时间戳 (Timestamp)
 */
typedef struct logging_input_s
{
  const char * name;                       ///< 名称 (Name)
  const rcutils_log_location_t * location; ///< 日志位置 (Log location)
  const char * msg;                        ///< 日志消息 (Log message)
  int severity;                            ///< 日志严重性级别 (Log severity level)
  rcutils_time_point_value_t timestamp;    ///< 时间戳 (Timestamp)
} logging_input_t;

/**
 * @typedef token_handler
 * @brief 处理日志输入的回调函数类型 (Callback function type for handling log inputs)
 *
 * @param logging_input 日志输入结构体指针 (Pointer to the logging input structure)
 * @param logging_output 日志输出字符数组 (Log output character array)
 * @param start_offset 开始偏移量 (Start offset)
 * @param end_offset 结束偏移量 (End offset)
 * @return 返回处理后的字符串 (Return processed string)
 */
typedef const char * (*token_handler)(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset);

/**
 * @struct log_msg_part_s
 * @brief 日志消息部分结构体 (Log message part structure)
 *
 * @param handler 回调函数处理器 (Callback function handler)
 * @param start_offset 开始偏移量 (Start offset)
 * @param end_offset 结束偏移量 (End offset)
 */
typedef struct log_msg_part_s
{
  token_handler handler; ///< 回调函数处理器 (Callback function handler)
  size_t start_offset;   ///< 开始偏移量 (Start offset)
  size_t end_offset;     ///< 结束偏移量 (End offset)
} log_msg_part_t;

static size_t g_num_log_msg_handlers = 0; ///< 日志消息处理器的数量 (Number of log message handlers)
static log_msg_part_t g_handlers
  [1024]; ///< 处理器数组，最多可存储1024个处理器 (Handler array, can store up to 1024 handlers)

/**
 * @brief 初始化日志系统 (Initialize the logging system)
 *
 * @return 返回初始化结果 (Return the initialization result)
 */
rcutils_ret_t rcutils_logging_initialize(void)
{
  return rcutils_logging_initialize_with_allocator(rcutils_get_default_allocator());
}

/**
 * @enum rcutils_get_env_retval
 * @brief 获取环境变量返回值枚举类型 (Enumeration type for return values of getting environment variables)
 */
enum rcutils_get_env_retval {
  RCUTILS_GET_ENV_ERROR = -1, ///< 错误 (Error)
  RCUTILS_GET_ENV_ZERO = 0,   ///< 零值 (Zero value)
  RCUTILS_GET_ENV_ONE = 1,    ///< 一值 (One value)
  RCUTILS_GET_ENV_EMPTY = 2,  ///< 空值 (Empty value)
};

/**
 * @brief 获取环境变量的值为零或一的实用函数 (A utility function to get zero or one from an environment variable)
 * @param[in] name 环境变量名称 (Name of the environment variable)
 * @param[in] zero_semantic 0 的语义 (Semantic meaning of 0)
 * @param[in] one_semantic 1 的语义 (Semantic meaning of 1)
 * @return rcutils_get_env_retval 返回枚举值，表示结果状态 (Return enumeration value indicating result status)
 * - RCUTILS_GET_ENV_ERROR: 获取环境变量失败或不可理解的值 (Failed to get the environment variable or an incomprehensible value)
 * - RCUTILS_GET_ENV_ZERO: 环境变量值为 "0" (The value in the environment variable is "0")
 * - RCUTILS_GET_ENV_ONE: 环境变量值为 "1" (The value in the environment variable is "1")
 * - RCUTILS_GET_ENV_EMPTY: 环境变量为空 (The environment variables is empty)
 */
static enum rcutils_get_env_retval rcutils_get_env_var_zero_or_one(
  const char * name, const char * zero_semantic, const char * one_semantic)
{
  // 声明一个指向环境变量值的指针 (Declare a pointer to the value of the environment variable)
  const char * env_var_value = NULL;
  // 调用 rcutils_get_env 函数获取环境变量值 (Call the rcutils_get_env function to get the value of the environment variable)
  const char * ret_str = rcutils_get_env(name, &env_var_value);
  // 如果获取环境变量值失败，设置错误信息并返回 RCUTILS_GET_ENV_ERROR (If failed to get the value of the environment variable, set the error message and return RCUTILS_GET_ENV_ERROR)
  if (NULL != ret_str) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting environment variable %s: %s", name, ret_str);
    return RCUTILS_GET_ENV_ERROR;
  }

  // 如果环境变量值为空字符串，返回 RCUTILS_GET_ENV_EMPTY (If the value of the environment variable is an empty string, return RCUTILS_GET_ENV_EMPTY)
  if (strcmp(env_var_value, "") == 0) {
    return RCUTILS_GET_ENV_EMPTY;
  }
  // 如果环境变量值为 "0"，返回 RCUTILS_GET_ENV_ZERO (If the value of the environment variable is "0", return RCUTILS_GET_ENV_ZERO)
  if (strcmp(env_var_value, "0") == 0) {
    return RCUTILS_GET_ENV_ZERO;
  }
  // 如果环境变量值为 "1"，返回 RCUTILS_GET_ENV_ONE (If the value of the environment variable is "1", return RCUTILS_GET_ENV_ONE)
  if (strcmp(env_var_value, "1") == 0) {
    return RCUTILS_GET_ENV_ONE;
  }

  // 如果环境变量值不是预期的 "0" 或 "1"，设置警告信息并返回 RCUTILS_GET_ENV_ERROR (If the value of the environment variable is not the expected "0" or "1", set a warning message and return RCUTILS_GET_ENV_ERROR)
  RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
    "Warning: unexpected value [%s] specified for %s. "
    "Valid values are 0 (%s) or 1 (%s).",
    env_var_value, name, zero_semantic, one_semantic);

  return RCUTILS_GET_ENV_ERROR;
}

/**
 * @brief Expand the timestamp in logging_input into a string and append it to logging_output.
 *
 * @param[in] logging_input 输入参数，包含日志信息和时间戳 (Input parameter containing log information and timestamp)
 * @param[out] logging_output 输出参数，将转换后的时间字符串追加到这个字符数组中 (Output parameter where the converted time string will be appended)
 * @param[in] time_func 时间转换函数，用于将时间戳转换为字符串 (Time conversion function used to convert the timestamp into a string)
 * @return 返回追加了时间字符串的 logging_output 的 buffer，如果失败则返回 NULL (Returns the buffer of logging_output with the time string appended, or NULL on failure)
 */
static const char * expand_time(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  rcutils_ret_t (*time_func)(const rcutils_time_point_value_t *, char *, size_t))
{
  // 临时本地存储，用于整数/浮点数转换为字符串 (Temporary, local storage for integer/float conversion to string)
  // 注意：32个字符足够了，因为最多可能有20个字符（64位有符号数字中可能的19位）加上浮点秒版本中的可选小数点 (Note: 32 characters enough, because the most it can be is 20 characters for the 19 possible digits in a signed 64-bit number plus the optional decimal point in the floating point seconds version)
  char numeric_storage[32];

  // 如果时间转换函数失败，则输出错误并返回 NULL (If the time conversion function fails, output the error and return NULL)
  if (
    time_func(&logging_input->timestamp, numeric_storage, sizeof(numeric_storage)) !=
    RCUTILS_RET_OK) {
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
    rcutils_reset_error();
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    return NULL;
  }

  // 将转换后的时间字符串追加到 logging_output，如果失败则输出错误并返回 NULL (Append the converted time string to logging_output, if it fails output the error and return NULL)
  if (rcutils_char_array_strcat(logging_output, numeric_storage) != RCUTILS_RET_OK) {
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
    rcutils_reset_error();
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    return NULL;
  }

  // 返回追加了时间字符串的 logging_output 的 buffer (Return the buffer of logging_output with the time string appended)
  return logging_output->buffer;
}

/**
 * @brief 使用 expand_time 函数将日志输入中的时间戳以秒为单位扩展为字符串，并追加到日志输出中。
 *
 * @param[in] logging_input 输入参数，包含日志信息和时间戳 (Input parameter containing log information and timestamp)
 * @param[out] logging_output 输出参数，将转换后的时间字符串追加到这个字符数组中 (Output parameter where the converted time string will be appended)
 * @param[in] start_offset 起始偏移量，未使用 (Start offset, not used)
 * @param[in] end_offset 结束偏移量，未使用 (End offset, not used)
 * @return 返回追加了时间字符串的 logging_output 的 buffer，如果失败则返回 NULL (Returns the buffer of logging_output with the time string appended, or NULL on failure)
 */
static const char * expand_time_as_seconds(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  // 忽略未使用的参数 (Ignore unused parameters)
  (void)start_offset;
  (void)end_offset;

  // 使用 rcutils_time_point_value_as_seconds_string 函数将时间戳转换为秒，并追加到 logging_output 中 (Use the rcutils_time_point_value_as_seconds_string function to convert the timestamp into seconds and append it to logging_output)
  return expand_time(logging_input, logging_output, rcutils_time_point_value_as_seconds_string);
}

/**
 * @brief 将时间以纳秒格式展开 (Expands the time as nanoseconds)
 *
 * @param[in] logging_input 日志输入结构体指针 (Pointer to the logging input structure)
 * @param[out] logging_output 日志输出字符数组指针 (Pointer to the logging output character array)
 * @param[in] start_offset 起始偏移量 (Start offset)
 * @param[in] end_offset 结束偏移量 (End offset)
 * @return 返回展开后的时间字符串 (Returns the expanded time string)
 */
static const char * expand_time_as_nanoseconds(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  // 忽略未使用的参数 (Ignore unused parameters)
  (void)start_offset;
  (void)end_offset;

  // 调用 expand_time 函数，传入 rcutils_time_point_value_as_nanoseconds_string 函数作为参数
  // (Call the expand_time function, passing in the rcutils_time_point_value_as_nanoseconds_string function as a parameter)
  return expand_time(logging_input, logging_output, rcutils_time_point_value_as_nanoseconds_string);
}

/**
 * @brief 展开行号 (Expands the line number)
 *
 * @param[in] logging_input 日志输入结构体指针 (Pointer to the logging input structure)
 * @param[out] logging_output 日志输出字符数组指针 (Pointer to the logging output character array)
 * @param[in] start_offset 起始偏移量 (Start offset)
 * @param[in] end_offset 结束偏移量 (End offset)
 * @return 返回展开后的行号字符串 (Returns the expanded line number string)
 */
static const char * expand_line_number(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  // 忽略未使用的参数 (Ignore unused parameters)
  (void)start_offset;
  (void)end_offset;

  // 检查日志输入位置是否存在 (Check if the logging input location exists)
  if (logging_input->location) {
    // 允许行号展开为9位数字（否则，截断）(Allow 9 digits for the expansion of the line number (otherwise, truncate))
    char line_number_expansion[10];

    // 即使在截断情况下，结果仍将以空字符结尾 (Even in the case of truncation the result will still be null-terminated)
    int written = rcutils_snprintf(
      line_number_expansion, sizeof(line_number_expansion), "%zu",
      logging_input->location->line_number);
    if (written < 0) {
      RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
        "failed to format line number: '%zu'\n", logging_input->location->line_number);
      return NULL;
    }

    // 将展开的行号附加到日志输出字符数组中 (Append the expanded line number to the logging output character array)
    if (rcutils_char_array_strcat(logging_output, line_number_expansion) != RCUTILS_RET_OK) {
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
      rcutils_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
      return NULL;
    }
  }

  // 返回日志输出缓冲区 (Return the logging output buffer)
  return logging_output->buffer;
}

/**
 * @brief 扩展日志严重性级别的字符串表示形式 (Expand the string representation of the log severity level)
 *
 * @param[in] logging_input 包含日志信息的结构体指针 (Pointer to a structure containing log information)
 * @param[out] logging_output 存储扩展后的严重性级别字符串的字符数组 (Character array to store the expanded severity level string)
 * @param[in] start_offset 未使用 (Not used)
 * @param[in] end_offset 未使用 (Not used)
 * @return 返回扩展后的严重性级别字符串，如果出错则返回 NULL (Returns the expanded severity level string, or NULL if an error occurs)
 */
static const char * expand_severity(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  (void)start_offset; // 忽略未使用的参数 (Ignore unused parameter)
  (void)end_offset;   // 忽略未使用的参数 (Ignore unused parameter)

  // 从全局数组中获取严重性级别的字符串表示 (Get the string representation of the severity level from the global array)
  const char * severity_string = g_rcutils_log_severity_names[logging_input->severity];
  // 将严重性字符串附加到 logging_output (Append the severity string to logging_output)
  if (rcutils_char_array_strcat(logging_output, severity_string) != RCUTILS_RET_OK) {
    // 如果出错，则向标准错误输出流写入错误消息并重置错误状态 (If an error occurs, write the error message to the standard error output stream and reset the error state)
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
    rcutils_reset_error();
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    return NULL;
  }

  return logging_output->buffer;
}

/**
 * @brief 扩展日志记录器名称字符串 (Expand the logger name string)
 *
 * @param[in] logging_input 包含日志信息的结构体指针 (Pointer to a structure containing log information)
 * @param[out] logging_output 存储扩展后的日志记录器名称字符串的字符数组 (Character array to store the expanded logger name string)
 * @param[in] start_offset 未使用 (Not used)
 * @param[in] end_offset 未使用 (Not used)
 * @return 返回扩展后的日志记录器名称字符串，如果出错则返回 NULL (Returns the expanded logger name string, or NULL if an error occurs)
 */
static const char * expand_name(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  (void)start_offset; // 忽略未使用的参数 (Ignore unused parameter)
  (void)end_offset;   // 忽略未使用的参数 (Ignore unused parameter)

  // 如果日志记录器名称不为空 (If the logger name is not NULL)
  if (NULL != logging_input->name) {
    // 将日志记录器名称附加到 logging_output (Append the logger name to logging_output)
    if (rcutils_char_array_strcat(logging_output, logging_input->name) != RCUTILS_RET_OK) {
      // 如果出错，则向标准错误输出流写入错误消息并重置错误状态 (If an error occurs, write the error message to the standard error output stream and reset the error state)
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
      rcutils_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
      return NULL;
    }
  }

  return logging_output->buffer;
}

/**
 * @brief 扩展日志消息字符串 (Expand the log message string)
 *
 * @param[in] logging_input 包含日志信息的结构体指针 (Pointer to a structure containing log information)
 * @param[out] logging_output 存储扩展后的日志消息字符串的字符数组 (Character array to store the expanded log message string)
 * @param[in] start_offset 未使用 (Not used)
 * @param[in] end_offset 未使用 (Not used)
 * @return 返回扩展后的日志消息字符串，如果出错则返回 NULL (Returns the expanded log message string, or NULL if an error occurs)
 */
static const char * expand_message(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  (void)start_offset; // 忽略未使用的参数 (Ignore unused parameter)
  (void)end_offset;   // 忽略未使用的参数 (Ignore unused parameter)

  // 将日志消息附加到 logging_output (Append the log message to logging_output)
  if (rcutils_char_array_strcat(logging_output, logging_input->msg) != RCUTILS_RET_OK) {
    // 如果出错，则向标准错误输出流写入错误消息并重置错误状态 (If an error occurs, write the error message to the standard error output stream and reset the error state)
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
    rcutils_reset_error();
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    return NULL;
  }

  return logging_output->buffer;
}

/**
 * @brief 扩展函数名
 * @param[in] logging_input 日志输入结构体指针
 * @param[out] logging_output 日志输出字符数组指针
 * @param[in] start_offset 起始偏移量（未使用）
 * @param[in] end_offset 结束偏移量（未使用）
 * @return 函数成功执行返回日志输出缓冲区的指针，否则返回 NULL
 *
 * @brief Expand function name
 * @param[in] logging_input Pointer to the logging input structure
 * @param[out] logging_output Pointer to the logging output character array
 * @param[in] start_offset Start offset (unused)
 * @param[in] end_offset End offset (unused)
 * @return On successful execution, returns a pointer to the logging output buffer, otherwise NULL
 */
static const char * expand_function_name(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  // 忽略未使用的参数
  // Ignore unused parameters
  (void)start_offset;
  (void)end_offset;

  // 检查 location 是否存在
  // Check if location exists
  if (logging_input->location) {
    // 将函数名添加到 logging_output
    // Append function name to logging_output
    if (
      rcutils_char_array_strcat(logging_output, logging_input->location->function_name) !=
      RCUTILS_RET_OK) {
      // 如果发生错误，将错误字符串写入标准错误流并重置错误
      // If an error occurs, write the error string to stderr and reset the error
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
      rcutils_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
      return NULL;
    }
  }

  // 返回日志输出缓冲区的指针
  // Return pointer to the logging output buffer
  return logging_output->buffer;
}

/**
 * @brief 扩展文件名
 * @param[in] logging_input 日志输入结构体指针
 * @param[out] logging_output 日志输出字符数组指针
 * @param[in] start_offset 起始偏移量（未使用）
 * @param[in] end_offset 结束偏移量（未使用）
 * @return 函数成功执行返回日志输出缓冲区的指针，否则返回 NULL
 *
 * @brief Expand file name
 * @param[in] logging_input Pointer to the logging input structure
 * @param[out] logging_output Pointer to the logging output character array
 * @param[in] start_offset Start offset (unused)
 * @param[in] end_offset End offset (unused)
 * @return On successful execution, returns a pointer to the logging output buffer, otherwise NULL
 */
static const char * expand_file_name(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  // 忽略未使用的参数
  // Ignore unused parameters
  (void)start_offset;
  (void)end_offset;

  // 检查 location 是否存在
  // Check if location exists
  if (logging_input->location) {
    // 将文件名添加到 logging_output
    // Append file name to logging_output
    if (
      rcutils_char_array_strcat(logging_output, logging_input->location->file_name) !=
      RCUTILS_RET_OK) {
      // 如果发生错误，将错误字符串写入标准错误流并重置错误
      // If an error occurs, write the error string to stderr and reset the error
      RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
      rcutils_reset_error();
      RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
      return NULL;
    }
  }

  // 返回日志输出缓冲区的指针
  // Return pointer to the logging output buffer
  return logging_output->buffer;
}

// 定义令牌映射条目结构体
// Define token map entry structure
typedef struct token_map_entry_s
{
  const char * token;    // 令牌字符串
  token_handler handler; // 令牌处理函数
} token_map_entry_t;

// 定义令牌映射数组
// Define token map array
static const token_map_entry_t tokens[] = {
  {.token = "severity", .handler = expand_severity},
  {.token = "name", .handler = expand_name},
  {.token = "message", .handler = expand_message},
  {.token = "function_name", .handler = expand_function_name},
  {.token = "file_name", .handler = expand_file_name},
  {.token = "time", .handler = expand_time_as_seconds},
  {.token = "time_as_nanoseconds", .handler = expand_time_as_nanoseconds},
  {.token = "line_number", .handler = expand_line_number},
};

/**
 * @brief 查找与给定token对应的处理函数 (Find the corresponding handler function for the given token)
 *
 * @param[in] token 要查找的token字符串 (The token string to be searched)
 * @return 如果找到匹配的处理函数，则返回该函数；否则，返回NULL (If a matching handler function is found, return it; otherwise, return NULL)
 */
static token_handler find_token_handler(const char * token)
{
  // 计算tokens数组中的元素个数 (Calculate the number of elements in the tokens array)
  int token_number = sizeof(tokens) / sizeof(tokens[0]);

  // 遍历tokens数组，查找与给定token匹配的处理函数 (Traverse the tokens array and find the handler function that matches the given token)
  for (int token_index = 0; token_index < token_number; token_index++) {
    if (strcmp(token, tokens[token_index].token) == 0) {
      return tokens[token_index].handler;
    }
  }

  // 如果没有找到匹配的处理函数，则返回NULL (If no matching handler function is found, return NULL)
  return NULL;
}

/**
 * @brief 从原始日志输入中复制一段文本 (Copy a segment of text from the original log input)
 *
 * @param[in] logging_input 指向原始日志输入的指针 (Pointer to the original log input)
 * @param[out] logging_output 用于存储复制后的文本的字符数组 (Character array for storing the copied text)
 * @param[in] start_offset 复制开始的偏移量 (Offset where copying starts)
 * @param[in] end_offset 复制结束的偏移量 (Offset where copying ends)
 * @return 如果复制成功，则返回指向logging_output->buffer的指针；否则，返回NULL (If the copy is successful, return a pointer to logging_output->buffer; otherwise, return NULL)
 */
static const char * copy_from_orig(
  const logging_input_t * logging_input,
  rcutils_char_array_t * logging_output,
  size_t start_offset,
  size_t end_offset)
{
  // 忽略未使用的参数 (Ignore unused parameter)
  (void)logging_input;

  // 将原始日志输入中指定范围内的文本追加到logging_output (Append the text within the specified range of the original log input to logging_output)
  if (
    rcutils_char_array_strncat(
      logging_output, g_rcutils_logging_output_format_string + start_offset,
      end_offset - start_offset) != RCUTILS_RET_OK) {
    // 如果追加失败，将错误信息输出到stderr (If the append fails, output the error information to stderr)
    RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);
    rcutils_reset_error();
    RCUTILS_SAFE_FWRITE_TO_STDERR("\n");
    return NULL;
  }

  // 返回指向logging_output->buffer的指针 (Return a pointer to logging_output->buffer)
  return logging_output->buffer;
}

/**
 * @brief 向处理函数数组中添加一个处理函数 (Add a handler function to the handler array)
 *
 * @param[in] handler 要添加的处理函数 (The handler function to be added)
 * @param[in] start_offset 处理函数处理的文本范围的起始偏移量 (The starting offset of the text range handled by the handler function)
 * @param[in] end_offset 处理函数处理的文本范围的结束偏移量 (The ending offset of the text range handled by the handler function)
 * @return 如果添加成功，则返回true；否则，返回false (If the addition is successful, return true; otherwise, return false)
 */
static bool add_handler(token_handler handler, size_t start_offset, size_t end_offset)
{
  // 检查处理函数数组是否已满 (Check if the handler array is full)
  if (g_num_log_msg_handlers >= (sizeof(g_handlers) / sizeof(g_handlers[0]))) {
    // 如果处理函数数组已满，设置错误消息并返回false (If the handler array is full, set an error message and return false)
    RCUTILS_SET_ERROR_MSG("Too many substitutions in the logging output format string; truncating");
    return false;
  }

  // 将处理函数及其处理范围添加到处理函数数组中 (Add the handler function and its processing range to the handler array)
  g_handlers[g_num_log_msg_handlers].handler = handler;
  g_handlers[g_num_log_msg_handlers].start_offset = start_offset;
  g_handlers[g_num_log_msg_handlers].end_offset = end_offset;

  // 更新处理函数数组中的元素数量 (Update the number of elements in the handler array)
  g_num_log_msg_handlers++;

  // 返回true表示添加成功 (Return true to indicate a successful addition)
  return true;
}

/**
 * @brief 解析并创建处理程序列表 (Parse and create handlers list)
 *
 * 该函数用于解析日志输出格式字符串并创建相应的回调处理程序。 (This function is used to parse the log output format string and create corresponding callback handlers.)
 */
static void parse_and_create_handlers_list(void)
{
  // 处理格式字符串，寻找已知的标记 (Process the format string looking for known tokens)
  const char token_start_delimiter = '{';
  const char token_end_delimiter = '}';

  const char * str = g_rcutils_logging_output_format_string;
  size_t size = strlen(g_rcutils_logging_output_format_string);

  g_num_log_msg_handlers = 0;

  // 遍历格式字符串，在遇到回调时创建它们 (Walk through the format string and create callbacks when they're encountered)
  size_t i = 0;
  while (i < size) {
    // 打印下一个标记开始分隔符之前的所有内容 (Print everything up to the next token start delimiter)
    size_t chars_to_start_delim = rcutils_find(str + i, token_start_delimiter);
    size_t remaining_chars = size - i;

    if (
      chars_to_start_delim >
      0) { // 在标记开始分隔符之前有内容 (there is stuff before a token start delimiter)
      size_t chars_to_copy =
        chars_to_start_delim > remaining_chars ? remaining_chars : chars_to_start_delim;
      if (!add_handler(copy_from_orig, i, i + chars_to_copy)) {
        // 错误已由 add_handler 设置 (The error was already set by add_handler)
        return;
      }

      i += chars_to_copy;
      if (i >= size) { // 也许没有找到开始分隔符 (perhaps no start delimiter was found)
        break;
      }

      continue;
    }

    // 我们在一个标记开始分隔符处：确定是否有已知的标记 (We are at a token start delimiter: determine if there's a known token or not)
    // 潜在的标记不可能比格式字符串本身更长 (Potential tokens can't possibly be longer than the format string itself)
    char token[RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN];

    // 寻找一个标记结束分隔符 (Look for a token end delimiter)
    size_t chars_to_end_delim = rcutils_find(str + i, token_end_delimiter);
    remaining_chars = size - i;

    if (chars_to_end_delim > remaining_chars) {
      // 在格式字符串的剩余部分中未找到结束分隔符；(No end delimiters found in the remainder of the format string)
      // 不会有更多的标记，所以可以快速检查其余的部分 (there won't be any more tokens so shortcut the rest of the checking)
      if (!add_handler(copy_from_orig, i, i + remaining_chars)) {
        // 错误已由 add_handler 设置 (The error was already set by add_handler)
        return;
      }
      break;
    }

    // 找到类似于标记的东西；确定它是否被识别 (Found what looks like a token; determine if it's recognized)
    size_t token_len = chars_to_end_delim - 1; // 不包括分隔符 (Not including delimiters)
    memcpy(token, str + i + 1, token_len);     // 跳过开始分隔符 (Skip the start delimiter)
    token[token_len] = '\0';

    token_handler expand_token = find_token_handler(token);

    if (!expand_token) {
      // 这不是一个标记；打印开始分隔符，然后像往常一样继续搜索 (This wasn't a token; print the start delimiter and continue the search as usual)
      // （子字符串可能包含更多的开始分隔符）(the substring might contain more start delimiters)
      if (!add_handler(copy_from_orig, i, i + 1)) {
        // 错误已由 add_handler 设置 (The error was already set by add_handler)
        return;
      }
      i++;
      continue;
    }

    if (!add_handler(expand_token, 0, 0)) {
      // 错误已由 add_handler 设置 (The error was already set by add_handler)
      return;
    }

    // 向前跳过，以避免重新处理标记字符（包括两个分隔符）(Skip ahead to avoid re-processing the token characters (including the 2 delimiters))
    i += token_len + 2;
  }
}

/**
 * @brief 初始化 rcutils 日志系统，使用给定的内存分配器。 (Initialize the rcutils logging system using the given allocator.)
 *
 * @param[in] allocator 用于日志系统的内存分配器。 (The memory allocator to be used for the logging system.)
 * @return 返回 RCUTILS_RET_OK 表示成功，其他值表示失败。 (Returns RCUTILS_RET_OK if successful, other values indicate failure.)
 */
rcutils_ret_t rcutils_logging_initialize_with_allocator(rcutils_allocator_t allocator)
{
  // 如果已经初始化，直接返回 RCUTILS_RET_OK。(If already initialized, return RCUTILS_RET_OK directly.)
  if (g_rcutils_logging_initialized) {
    return RCUTILS_RET_OK;
  }

  // 检查提供的分配器是否有效。 (Check if the provided allocator is valid.)
  if (!rcutils_allocator_is_valid(&allocator)) {
    RCUTILS_SET_ERROR_MSG("Provided allocator is invalid.");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  g_rcutils_logging_allocator = allocator;

  // 设置默认日志输出处理函数为控制台输出。 (Set the default log output handler to console output.)
  g_rcutils_logging_output_handler = &rcutils_logging_console_output_handler;
  // 设置默认日志记录级别。 (Set the default logger level.)
  g_rcutils_logging_default_logger_level = RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL;

  // 获取环境变量 "RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED" 的值。 (Get the value of the environment variable "RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED".)
  const char * line_buffered = NULL;
  const char * ret_str = rcutils_get_env("RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED", &line_buffered);
  if (NULL != ret_str) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting environment variable RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED: %s", ret_str);
    return RCUTILS_RET_ERROR;
  }

  // 检查环境变量是否为空，如果不为空，则输出警告信息。 (Check if the environment variable is empty, and output a warning message if it is not.)
  if (strcmp(line_buffered, "") != 0) {
    RCUTILS_SAFE_FWRITE_TO_STDERR(
      "RCUTILS_CONSOLE_STDOUT_LINE_BUFFERED is now ignored. "
      "Please set RCUTILS_LOGGING_USE_STDOUT and RCUTILS_LOGGING_BUFFERED_STREAM "
      "to control the stream and the buffering of log messages.\n");
  }

  // 设置所有严重性级别的默认输出流为 stderr，以便立即传播错误。用户可以通过设置 RCUTILS_LOGGING_USE_STDOUT 环境变量为 1 来选择将输出流设置为 stdout。
  // (Set the default output stream for all severities to stderr so that errors are propagated immediately. The user can choose to set the output stream to stdout by setting the RCUTILS_LOGGING_USE_STDOUT environment variable to 1.)
  enum rcutils_get_env_retval retval =
    rcutils_get_env_var_zero_or_one("RCUTILS_LOGGING_USE_STDOUT", "use stderr", "use stdout");
  switch (retval) {
  case RCUTILS_GET_ENV_ERROR:
    return RCUTILS_RET_INVALID_ARGUMENT;
  case RCUTILS_GET_ENV_EMPTY:
  case RCUTILS_GET_ENV_ZERO:
    g_output_stream = stderr;
    break;
  case RCUTILS_GET_ENV_ONE:
    g_output_stream = stdout;
    break;
  default:
    RCUTILS_SET_ERROR_MSG("Invalid return from environment fetch");
    return RCUTILS_RET_ERROR;
  }

  // 允许用户通过设置 RCUTILS_LOGGING_BUFFERED_STREAM 来选择流的缓冲方式。空环境变量使用流的默认值。值为 0 时，强制使流不缓冲。值为 1 时，强制使流行缓冲。
  // (Allow the user to choose how buffering on the stream works by setting RCUTILS_LOGGING_BUFFERED_STREAM. With an empty environment variable, use the default of the stream. With a value of 0, force the stream to be unbuffered. With a value of 1, force the stream to be line buffered.)
  retval =
    rcutils_get_env_var_zero_or_one("RCUTILS_LOGGING_BUFFERED_STREAM", "not buffered", "buffered");
  if (RCUTILS_GET_ENV_ERROR == retval) {
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  if (RCUTILS_GET_ENV_ZERO == retval || RCUTILS_GET_ENV_ONE == retval) {
    int mode = retval == RCUTILS_GET_ENV_ZERO ? _IONBF : _IOLBF;
    size_t buffer_size = (mode == _IOLBF) ? RCUTILS_LOGGING_STREAM_BUFFER_SIZE : 0;

    // 在 Windows 上，当 IOLBF 时，buffer_size 不能为 0，请参阅上面的 #define 注释。(buffer_size cannot be 0 on Windows with IOLBF, see comments above where it's #define'd)
    if (setvbuf(g_output_stream, NULL, mode, buffer_size) != 0) {
      char error_string[1024];
      rcutils_strerror(error_string, sizeof(error_string));
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Error setting stream buffering mode: %s", error_string);
      return RCUTILS_RET_ERROR;
    }
  } else if (RCUTILS_GET_ENV_EMPTY != retval) {
    RCUTILS_SET_ERROR_MSG("Invalid return from environment fetch");
    return RCUTILS_RET_ERROR;
  }

  // 获取环境变量 "RCUTILS_COLORIZED_OUTPUT" 的值以设置彩色输出。(Get the value of the environment variable "RCUTILS_COLORIZED_OUTPUT" to set colorized output.)
  retval =
    rcutils_get_env_var_zero_or_one("RCUTILS_COLORIZED_OUTPUT", "force color", "force no color");
  switch (retval) {
  case RCUTILS_GET_ENV_ERROR:
    return RCUTILS_RET_INVALID_ARGUMENT;
  case RCUTILS_GET_ENV_EMPTY:
    g_colorized_output = RCUTILS_COLORIZED_OUTPUT_AUTO;
    break;
  case RCUTILS_GET_ENV_ZERO:
    g_colorized_output = RCUTILS_COLORIZED_OUTPUT_FORCE_DISABLE;
    break;
  case RCUTILS_GET_ENV_ONE:
    g_colorized_output = RCUTILS_COLORIZED_OUTPUT_FORCE_ENABLE;
    break;
  default:
    RCUTILS_SET_ERROR_MSG("Invalid return from environment fetch");
    return RCUTILS_RET_ERROR;
  }

  // 检查自定义输出格式的环境变量 (Check for the environment variable for custom output formatting)
  const char * output_format;
  ret_str = rcutils_get_env("RCUTILS_CONSOLE_OUTPUT_FORMAT", &output_format);
  if (NULL != ret_str) {
    // 获取环境变量失败，使用默认输出格式 (Failed to get environment variable, using default output format)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Failed to get output format from env. variable [%s]. Using default output format.", ret_str);
    output_format = g_rcutils_logging_default_output_format;
  } else {
    if (strcmp(output_format, "") == 0) {
      // 环境变量为空，使用默认输出格式 (Environment variable is empty, using default output format)
      output_format = g_rcutils_logging_default_output_format;
    }
  }

  // 复制输出格式字符串 (Copy the output format string)
  size_t chars_to_copy = strlen(output_format);
  if (chars_to_copy > RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN - 1) {
    chars_to_copy = RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN - 1;
  }
  memcpy(g_rcutils_logging_output_format_string, output_format, chars_to_copy);
  g_rcutils_logging_output_format_string[chars_to_copy] = '\0';

  // 初始化日志严重性映射哈希表 (Initialize the log severity mapping hash table)
  g_rcutils_logging_severities_map = rcutils_get_zero_initialized_hash_map();
  rcutils_ret_t hash_map_ret = rcutils_hash_map_init(
    &g_rcutils_logging_severities_map, 2, sizeof(const char *), sizeof(int),
    rcutils_hash_map_string_hash_func, rcutils_hash_map_string_cmp_func, &allocator);
  if (hash_map_ret != RCUTILS_RET_OK) {
    // 初始化失败，严重性将不可配置 (Initialization failed, severities will not be configurable)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Failed to initialize map for logger severities [%s]. Severities will not be configurable.",
      rcutils_get_error_string().str);
    g_rcutils_logging_severities_map_valid = false;
    return RCUTILS_RET_ERROR;
  }

  // 解析并创建处理程序列表 (Parse and create handlers list)
  parse_and_create_handlers_list();

  // 设置日志严重性映射哈希表为有效 (Set log severity mapping hash table as valid)
  g_rcutils_logging_severities_map_valid = true;

  // 设置日志系统为已初始化 (Set logging system as initialized)
  g_rcutils_logging_initialized = true;

  return RCUTILS_RET_OK;
}

/**
 * @brief 关闭 rcutils 日志系统 (Shutdown the rcutils logging system)
 *
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t rcutils_logging_shutdown(void)
{
  // 检查日志系统是否已初始化，如果未初始化则返回 RCUTILS_RET_OK
  // (Check if the logging system is initialized, return RCUTILS_RET_OK if not)
  if (!g_rcutils_logging_initialized) {
    return RCUTILS_RET_OK;
  }

  // 定义一个返回值变量 ret，并设置其初始值为 RCUTILS_RET_OK
  // (Define a return value variable `ret` and set its initial value to RCUTILS_RET_OK)
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // 检查日志严重性映射是否有效
  // (Check if the logging severities map is valid)
  if (g_rcutils_logging_severities_map_valid) {
    // 遍历映射表，获取每个键以释放它们
    // (Iterate over the map, getting every key so we can free them)
    char * key = NULL;
    int level;
    rcutils_ret_t hash_map_ret =
      rcutils_hash_map_get_next_key_and_data(&g_rcutils_logging_severities_map, NULL, &key, &level);

    // 当映射表中仍有数据时继续循环
    // (Continue looping while there is still data in the map)
    while (RCUTILS_RET_OK == hash_map_ret) {
      // 移除当前键值对
      // (Remove the current key-value pair)
      hash_map_ret = rcutils_hash_map_unset(&g_rcutils_logging_severities_map, &key);
      if (hash_map_ret != RCUTILS_RET_OK) {
        // 如果移除失败，设置错误信息并跳出循环
        // (If the removal fails, set an error message and break the loop)
        RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Failed to clear out logger severities [%s] during shutdown; memory will be leaked.",
          rcutils_get_error_string().str);
        break;
      }
      // 使用分配器释放键内存
      // (Free the key memory using the allocator)
      g_rcutils_logging_allocator.deallocate(key, g_rcutils_logging_allocator.state);

      // 获取下一个键值对
      // (Get the next key-value pair)
      hash_map_ret = rcutils_hash_map_get_next_key_and_data(
        &g_rcutils_logging_severities_map, NULL, &key, &level);
    }

    // 销毁映射表
    // (Destroy the map)
    hash_map_ret = rcutils_hash_map_fini(&g_rcutils_logging_severities_map);
    if (hash_map_ret != RCUTILS_RET_OK) {
      // 如果销毁失败，设置错误信息并更新返回值
      // (If destruction fails, set an error message and update the return value)
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Failed to finalize map for logger severities: %s", rcutils_get_error_string().str);
      ret = RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID;
    }
    // 设置日志严重性映射表为无效
    // (Set the logging severities map as invalid)
    g_rcutils_logging_severities_map_valid = false;
  }
  // 将日志消息处理器数量重置为零
  // (Reset the number of log message handlers to zero)
  g_num_log_msg_handlers = 0;

  // 设置日志系统未初始化状态
  // (Set the logging system as uninitialized)
  g_rcutils_logging_initialized = false;

  // 返回操作结果
  // (Return the operation result)
  return ret;
}

/**
 * @brief 将字符串解析为日志级别枚举值
 * @param severity_string 输入的日志级别字符串
 * @param allocator 分配器用于分配内存
 * @param[out] severity 输出的日志级别枚举值
 * @return rcutils_ret_t 返回状态码
 *
 * Parses a string into a logging severity level enumeration value.
 *
 * @param severity_string The input logging severity level string.
 * @param allocator The allocator used for allocating memory.
 * @param[out] severity The output logging severity level enumeration value.
 * @return rcutils_ret_t The return status code.
 */
rcutils_ret_t rcutils_logging_severity_level_from_string(
  const char * severity_string, rcutils_allocator_t allocator, int * severity)
{
  // 检查分配器是否有效
  // Check if the allocator is valid
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT);
  // 检查输入的日志级别字符串是否为空
  // Check if the input logging severity level string is NULL
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(severity_string, RCUTILS_RET_INVALID_ARGUMENT);
  // 检查输出的日志级别枚举值指针是否为空
  // Check if the output logging severity level enumeration value pointer is NULL
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(severity, RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = RCUTILS_RET_LOGGING_SEVERITY_STRING_INVALID;

  // 将输入字符串转换为大写（忽略大小写）
  // Convert the input string to upper case (for case insensitivity)
  char * severity_string_upper = rcutils_strdup(severity_string, allocator);
  if (NULL == severity_string_upper) {
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for uppercase string");
    return RCUTILS_RET_BAD_ALLOC;
  }
  for (int i = 0; severity_string_upper[i]; ++i) {
    severity_string_upper[i] = (char)toupper(severity_string_upper[i]);
  }

  // 确定与严重性名称匹配的严重性值
  // Determine the severity value matching the severity name
  for (size_t i = 0;
       i < sizeof(g_rcutils_log_severity_names) / sizeof(g_rcutils_log_severity_names[0]); ++i) {
    const char * severity_string_i = g_rcutils_log_severity_names[i];
    if (severity_string_i && strcmp(severity_string_i, severity_string_upper) == 0) {
      *severity = (enum RCUTILS_LOG_SEVERITY)i;
      ret = RCUTILS_RET_OK;
      break;
    }
  }
  allocator.deallocate(severity_string_upper, allocator.state);
  return ret;
}

/**
 * @brief 获取当前日志输出处理函数
 * @return rcutils_logging_output_handler_t 日志输出处理函数指针
 *
 * Gets the current logging output handler function.
 *
 * @return rcutils_logging_output_handler_t The logging output handler function pointer.
 */
rcutils_logging_output_handler_t rcutils_logging_get_output_handler(void)
{
  RCUTILS_LOGGING_AUTOINIT;
  return g_rcutils_logging_output_handler;
}

/**
 * @brief 设置日志输出处理函数
 * @param function 日志输出处理函数指针
 *
 * Sets the logging output handler function.
 *
 * @param function The logging output handler function pointer.
 */
void rcutils_logging_set_output_handler(rcutils_logging_output_handler_t function)
{
  RCUTILS_LOGGING_AUTOINIT;
  g_rcutils_logging_output_handler = function;
}

/**
 * @brief 获取默认日志记录器的日志级别
 * @return int 默认日志记录器的日志级别
 *
 * Gets the logging level of the default logger.
 *
 * @return int The logging level of the default logger.
 */
int rcutils_logging_get_default_logger_level(void)
{
  RCUTILS_LOGGING_AUTOINIT;
  return g_rcutils_logging_default_logger_level;
}

/**
 * @brief 设置默认日志记录器的级别 (Set the default logger level)
 * 
 * @param[in] level 日志级别 (The logging level)
 */
void rcutils_logging_set_default_logger_level(int level)
{
  // 自动初始化日志系统 (Automatically initialize the logging system)
  RCUTILS_LOGGING_AUTOINIT;

  // 如果传入的级别为未设置，则恢复默认值 (If the input level is unset, restore the default value)
  if (RCUTILS_LOG_SEVERITY_UNSET == level) {
    // 恢复默认级别 (Restore the default level)
    level = RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL;
  }

  // 更新全局变量中的默认日志记录器级别 (Update the default logger level in the global variable)
  g_rcutils_logging_default_logger_level = level;
}

/**
 * @brief 获取指定名称的日志记录器的级别 (Get the logger level of a specified name)
 *
 * @param[in] name 日志记录器名称 (The logger name)
 * @return int 日志级别 (The logging level)
 */
int rcutils_logging_get_logger_level(const char * name)
{
  // 自动初始化日志系统 (Automatically initialize the logging system)
  RCUTILS_LOGGING_AUTOINIT;

  // 如果名称为空，返回错误 (-1) (If the name is null, return error (-1))
  if (NULL == name) {
    return -1;
  }

  // 调用 rcutils_logging_get_logger_leveln 函数获取日志级别 (Call the rcutils_logging_get_logger_leveln function to get the logger level)
  return rcutils_logging_get_logger_leveln(name, strlen(name));
}

/**
 * @brief 将键添加到哈希映射中 (Add the key to the hash map)
 *
 * @param[in] name 日志记录器的名称 (The logger name)
 * @param[in] level 日志级别 (The logging level)
 * @param[in] set_by_user 是否由用户设置 (Whether it is set by the user)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
static rcutils_ret_t add_key_to_hash_map(const char * name, int level, bool set_by_user)
{
  const char * copy_name = name;

  // 检查键是否已经存在，以避免额外的内存分配 (Check if the key already exists to avoid extra memory allocation)
  bool already_exists = rcutils_hash_map_key_exists(&g_rcutils_logging_severities_map, &copy_name);

  if (!already_exists) {
    // 复制要存储的名称，因为不能保证调用者会一直保留它 (Copy the name to be stored, as there is no guarantee that the caller will keep it around)
    copy_name = rcutils_strdup(name, g_rcutils_logging_allocator);
    if (copy_name == NULL) {
      // 不要向错误处理机制报告错误；此函数的某些用途是用于缓存，所以这不一定是致命的 (Don't report an error to the error handling machinery; some uses of this function are for caching so this is not necessarily fatal)
      return RCUTILS_RET_ERROR;
    }
  }

  if (set_by_user) {
    // 如果级别是由用户设置的（而不是由实现乐观地缓存），在此处标记它。当我们清除缓存时，我们将确保不要清除这些。我们使用的标记是设置底位；由于我们的级别是 0、10、20、30、40 和 50，所以这是可行的 (If the level was set by the user (rather than optimistically cached by the implementation), mark it here. When we purge the cache, we'll make sure not to purge these. The mark we use is setting the bottom bit; since our levels are 0, 10, 20, 30, 40, and 50, this works)
    level |= 0x1;
  }

  // 将键值对添加到哈希映射中 (Add the key-value pair to the hash map)
  rcutils_ret_t hash_map_ret =
    rcutils_hash_map_set(&g_rcutils_logging_severities_map, &copy_name, &level);
  if (hash_map_ret != RCUTILS_RET_OK) {
    // 设置错误消息 (Set error message)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error setting severity level for logger named '%s': %s", name,
      rcutils_get_error_string().str);
    return RCUTILS_RET_ERROR;
  }

  // 返回操作成功 (Return operation success)
  return RCUTILS_RET_OK;
}

/**
 * @brief 获取给定名称的日志严重性级别（Get the severity level for the given name）
 *
 * @param[in] name 日志名（Logger name）
 * @param[out] severity 返回的严重性级别（Returned severity level）
 * @return rcutils_ret_t 操作结果状态（Operation result status）
 */
static rcutils_ret_t get_severity_level(const char * name, int * severity)
{
  // 从哈希映射中获取与给定名称对应的严重性级别（Get the severity level corresponding to the given name from the hash map）
  rcutils_ret_t ret = rcutils_hash_map_get(&g_rcutils_logging_severities_map, &name, severity);
  if (ret != RCUTILS_RET_OK) {
    // 一个可能的响应是RCUTILS_RET_NOT_FOUND，但更高层次可能对此没有问题（One possible response is RCUTILS_RET_NOT_FOUND, but the higher layers may be OK with that）
    return ret;
  }

  // 参见add_key_to_hash_map()中关于为什么要删除最低位的注释（See the comment in add_key_to_hash_map() on why we remove the bottom bit）
  (*severity) &= ~(0x1);

  return RCUTILS_RET_OK;
}

/**
 * @brief 根据给定的名称获取日志记录器级别（Get the logger level for the given name）
 *
 * @param[in] name 日志记录器名称（Logger name）
 * @param[in] name_length 名称长度（Name length）
 * @return int 日志记录器严重性级别（Logger severity level）
 */
int rcutils_logging_get_logger_leveln(const char * name, size_t name_length)
{
  // 自动初始化日志记录（Automatically initialize logging）
  RCUTILS_LOGGING_AUTOINIT;
  if (NULL == name) {
    return -1;
  }

  // 如果请求默认值，则跳过映射查找，因为即使严重性映射无效，仍可以使用它（Skip the map lookup if the default was requested, as it can still be used even if the severity map is invalid）
  if (0 == name_length) {
    return g_rcutils_logging_default_logger_level;
  }
  if (!g_rcutils_logging_severities_map_valid) {
    return RCUTILS_LOG_SEVERITY_UNSET;
  }

  // 分配内存以获取短名称（Allocate memory to get the short name）
  char * short_name = rcutils_strndup(name, name_length, g_rcutils_logging_allocator);
  if (short_name == NULL) {
    // 分配内存失败时设置错误消息（Set error message when memory allocation fails）
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Failed to allocate memory when looking up logger level for '%s'", name);
    return -1;
  }

  // 获取给定短名称的严重性级别（Get the severity level for the given short name）
  int severity;
  rcutils_ret_t ret = get_severity_level(short_name, &severity);
  // 释放短名称所占用的内存（Free the memory occupied by the short name）
  g_rcutils_logging_allocator.deallocate(short_name, g_rcutils_logging_allocator.state);
  if (ret != RCUTILS_RET_OK) {
    // 错误消息已由get_severity_level设置（The error message was already set by get_severity_level）
    return RCUTILS_LOG_SEVERITY_UNSET;
  }

  return severity;
}

/**
 * @brief 获取指定名称的日志记录器的有效级别 (Get the effective level of a logger with the specified name)
 *
 * @param[in] name 日志记录器的名称 (The name of the logger)
 * @return 返回有效日志级别，如果发生错误则返回-1 (Return the effective logging level, or -1 if an error occurs)
 */
int rcutils_logging_get_logger_effective_level(const char * name)
{
  // 自动初始化日志系统 (Automatically initialize the logging system)
  RCUTILS_LOGGING_AUTOINIT;
  // 如果名称为空，则返回-1 (If the name is NULL, return -1)
  if (NULL == name) {
    return -1;
  }

  size_t hash_map_size;
  // 获取哈希映射中的日志严重性等级数量 (Get the number of severity levels in the hash map)
  rcutils_ret_t hash_map_ret =
    rcutils_hash_map_get_size(&g_rcutils_logging_severities_map, &hash_map_size);
  if (hash_map_ret != RCUTILS_RET_OK) {
    // 设置错误消息 (Set the error message)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting severity level for logger named '%s': %s", name,
      rcutils_get_error_string().str);
    return -1;
  }

  if (hash_map_size == 0) {
    // 如果哈希映射中没有条目，返回默认日志记录器级别 (If there are no entries in the hash map, return the default logger level)
    return g_rcutils_logging_default_logger_level;
  }

  // 首先尝试查找完全匹配的名称 (Start by trying to find the exact name)
  int severity;
  rcutils_ret_t ret = get_severity_level(name, &severity);
  if (ret == RCUTILS_RET_OK) {
    if (severity != RCUTILS_LOG_SEVERITY_UNSET) {
      // 如果严重性已设置，返回严重性等级 (If the severity is set, return the severity level)
      return severity;
    }
    // 如果严重性未设置，则通过慢速路径查找已设置的父记录器，如果失败，则回退到默认日志记录器级别
    // (If the severity is UNSET, then go through the slow path and try to find a parent logger that is set. Failing that, fall back to the default logger level)
  } else if (ret != RCUTILS_RET_NOT_FOUND) {
    // 错误消息已由get_severity_level设置 (The error message was already set by get_severity_level)
    return -1;
  }

  // 由于在快速路径中未找到名称，请回退到将字符串拆分为基于点的子字符串并查找任何匹配部分的慢速路径
  // (Since we didn't find the name in the fast path, fall back to the slow path where we break the string into substrings based on dots and look for any part that matches)

  size_t substring_length = strlen(name);
  char * tmp_name = rcutils_strdup(name, g_rcutils_logging_allocator);
  if (tmp_name == NULL) {
    // 复制字符串时出错 (Error copying string)
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING("Error copying string '%s'\n", name);
    return -1;
  }

  severity = RCUTILS_LOG_SEVERITY_UNSET;

  while (true) {
    // 通过删除子名称来确定下一个祖先的FQN (Determine the next ancestor's FQN by removing the child's name)
    size_t index_last_separator =
      rcutils_find_lastn(name, RCUTILS_LOGGING_SEPARATOR_CHAR, substring_length);
    if (SIZE_MAX == index_last_separator) {
      // 子字符串中没有更多的分隔符，因此这是我们需要检查的最后一个名称
      // (There are no more separators in the substring, so this is the last name we needed to check)
      break;
    }

    substring_length = index_last_separator;

    // 缩短子字符串以使其成为祖先的名称（不包括分隔符）(Shorten the substring to be the name of the ancestor (excluding the separator))
    tmp_name[index_last_separator] = '\0';

    rcutils_ret_t ret = get_severity_level(tmp_name, &severity);
    if (ret == RCUTILS_RET_OK) {
      if (severity != RCUTILS_LOG_SEVERITY_UNSET) {
        break;
      }
    } else if (ret != RCUTILS_RET_NOT_FOUND) {
      // 错误消息已由get_severity_level设置 (The error message was already set by get_severity_level)
      g_rcutils_logging_allocator.deallocate(tmp_name, g_rcutils_logging_allocator.state);
      return -1;
    }
  }

  g_rcutils_logging_allocator.deallocate(tmp_name, g_rcutils_logging_allocator.state);

  if (severity == RCUTILS_LOG_SEVERITY_UNSET) {
    // 日志记录器及其祖先都未指定级别 (Neither the logger nor its ancestors have had their level specified)
    severity = g_rcutils_logging_default_logger_level;
  }

  // 当线程安全问题得到解决时，恢复或替换此优化
  // (Restore or replace this optimization when thread-safety is addressed)
  // TODO(wjwwood): restore or replace this optimization when thread-safety is addressed
  //   see: https://github.com/ros2/rcutils/pull/393

  return severity;
}

/**
 * @brief 设置给定名称的记录器的日志级别。
 * @param[in] name 日志记录器的名称。
 * @param[in] level 要设置的日志级别。
 * @return 返回 rcutils_ret_t，表示操作成功或失败。
 *
 * @brief Set the logging level for a logger with the given name.
 * @param[in] name The name of the logger.
 * @param[in] level The logging level to set.
 * @return Returns an rcutils_ret_t indicating success or failure.
 */
rcutils_ret_t rcutils_logging_set_logger_level(const char * name, int level)
{
  // 自动初始化日志记录功能
  // Automatically initialize the logging functionality
  RCUTILS_LOGGING_AUTOINIT;

  // 检查输入的记录器名称是否为空
  // Check if the input logger name is NULL
  if (NULL == name) {
    RCUTILS_SET_ERROR_MSG("Invalid logger name");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 检查日志严重性映射是否有效
  // Check if the logging severity map is valid
  if (!g_rcutils_logging_severities_map_valid) {
    RCUTILS_SET_ERROR_MSG("Logger severity level map is invalid");
    return RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID;
  }

  // 将严重性值转换为字符串以便存储。
  // Convert the severity value into a string for storage.
  if (
    level < 0 || level >= (int)(sizeof(g_rcutils_log_severity_names) /
                                sizeof(g_rcutils_log_severity_names[0]))) {
    RCUTILS_SET_ERROR_MSG("Invalid severity level specified for logger");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }
  const char * severity_string = g_rcutils_log_severity_names[level];
  if (NULL == severity_string) {
    RCUTILS_SET_ERROR_MSG("Unable to determine severity_string for severity");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  size_t name_length = strlen(name);

  if (rcutils_hash_map_key_exists(&g_rcutils_logging_severities_map, &name)) {
    // 遍历严重性映射的所有内容，查找以此键名开头的条目。
    // 对于匹配的任何条目，检查用户是否明确设置了它们。如果用户没有设置，
    // 那么我们将其缓存，因此可以将其丢弃。
    //
    // Iterate over the entire contents of the severities map, looking for entries that start with
    // this key name. For any ones that match, check whether the user explicitly set them. If the
    // user did not set it, then we cached it and so we can throw it away.
    char * key = NULL;
    int tmp_level;

    rcutils_ret_t hash_map_ret = rcutils_hash_map_get_next_key_and_data(
      &g_rcutils_logging_severities_map, NULL, &key, &tmp_level);
    while (RCUTILS_RET_OK == hash_map_ret) {
      // 保留对指针的引用；稍后我们需要它
      // Hold onto a reference to the pointer; we'll need it later
      char * previous_key = key;
      bool free_current_key = false;
      if (key != NULL && strncmp(name, key, name_length) == 0) {
        // 如果这是我们要替换的键，请从哈希映射中无条件删除它；
        // 无论如何，我们都会将其添加回作为用户设置的级别
        //
        // If this is the key we are replacing, unconditionally remove it from the hash map;
        // we'll be adding it back as a user-set level anyway
        if (key[name_length] == '\0') {
          free_current_key = true;
        } else {
          // 否则，这是一个后代；仅在我们缓存它时从哈希映射中删除它
          // （用户没有明确设置它）。
          //
          // Otherwise, this is a descendant; only remove it from the hash map
          // if we cached it (the user didn't explicitly set it).
          if (!(tmp_level & 0x1)) {
            free_current_key = true;
          }
        }
      }

      // 注意，我们需要在释放当前的前获取下一个键
      // 这样我们才能继续遍历hash_map
      //
      // Note that we need to get the next key before we free the current
      // key so that we can continue iterating over the hash_map
      hash_map_ret = rcutils_hash_map_get_next_key_and_data(
        &g_rcutils_logging_severities_map, &previous_key, &key, &tmp_level);
      if (
        hash_map_ret != RCUTILS_RET_OK && hash_map_ret != RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES &&
        hash_map_ret != RCUTILS_RET_NOT_FOUND) {
        RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Error accessing hash map when setting logger level for '%s': %s", name,
          rcutils_get_error_string().str);
        return hash_map_ret;
      }

      if (free_current_key) {
        rcutils_ret_t unset_ret =
          rcutils_hash_map_unset(&g_rcutils_logging_severities_map, &previous_key);
        if (unset_ret != RCUTILS_RET_OK) {
          // hash_map_unset 只有在哈希映射结构出错或键为 NULL 时才会失败。
          // 由于我们不希望这两种情况发生，因此将其报告为错误并返回。
          //
          // The only way that hash_map_unset can fail is if there is something wrong with the
          // hashmap structure or the key is NULL. Since we don't expect either, report it as an
          // error and return.
          RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Error clearing old severity level for logger named '%s': %s", name,
            rcutils_get_error_string().str);
          return unset_ret;
        }
        g_rcutils_logging_allocator.deallocate(previous_key, g_rcutils_logging_allocator.state);
      }
    }
  }

  rcutils_ret_t add_key_ret = add_key_to_hash_map(name, level, true);
  if (add_key_ret != RCUTILS_RET_OK) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error setting severity level for logger named '%s': %s", name,
      rcutils_get_error_string().str);
  }

  if (name_length == 0) {
    // 如果名称为空，这也意味着我们应该更新默认记录器级别
    // If the name was empty, this also means we should update the default logger level
    g_rcutils_logging_default_logger_level = level;
  }

  return add_key_ret;
}

/**
 * @brief 判断指定日志记录器是否启用了给定的日志级别 (Determine if the specified logger is enabled for the given severity level)
 * @param[in] name 日志记录器名称 (Logger name)
 * @param[in] severity 要检查的日志级别 (Severity level to check)
 * @return 如果启用了给定的日志级别，则返回true，否则返回false (Returns true if the given severity level is enabled, false otherwise)
 */
bool rcutils_logging_logger_is_enabled_for(const char * name, int severity)
{
  // 自动初始化日志系统 (Automatically initialize the logging system)
  RCUTILS_LOGGING_AUTOINIT;
  // 获取默认日志记录器的日志级别 (Get the log level of the default logger)
  int logger_level = g_rcutils_logging_default_logger_level;
  // 如果提供了日志记录器名称 (If a logger name is provided)
  if (name) {
    // 获取有效的日志级别 (Get the effective log level)
    logger_level = rcutils_logging_get_logger_effective_level(name);
    // 如果获取日志级别失败 (If getting the log level fails)
    if (-1 == logger_level) {
      // 输出错误信息到标准错误流 (Output error message to standard error stream)
      RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
        "Error determining if logger '%s' is enabled for severity '%d'\n", name, severity);
      return false;
    }
  }
  // 返回给定的日志级别是否启用 (Return whether the given severity level is enabled)
  return severity >= logger_level;
}

/**
 * @brief 内部日志函数，处理可变参数列表 (Internal logging function, handling variable argument list)
 * @param[in] location 日志消息的源代码位置 (Source code location of the log message)
 * @param[in] severity 日志消息的严重性级别 (Severity level of the log message)
 * @param[in] name 日志记录器名称 (Logger name)
 * @param[in] format 格式化字符串 (Format string)
 * @param[in] args 可变参数列表指针 (Pointer to the variable argument list)
 */
static void vrcutils_log_internal(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  const char * format,
  va_list * args)
{
  // 获取当前时间戳 (Get the current timestamp)
  rcutils_time_point_value_t now;
  rcutils_ret_t ret = rcutils_system_time_now(&now);
  // 如果获取时间戳失败 (If getting the timestamp fails)
  if (ret != RCUTILS_RET_OK) {
    // 输出错误信息到标准错误流 (Output error message to standard error stream)
    RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to get timestamp while doing a console logging.\n");
    return;
  }
  // 获取日志输出处理函数 (Get the log output handler function)
  rcutils_logging_output_handler_t output_handler = g_rcutils_logging_output_handler;
  // 如果提供了输出处理函数 (If an output handler is provided)
  if (output_handler != NULL) {
    // 调用输出处理函数，传递日志信息 (Call the output handler function, passing the log information)
    (*output_handler)(location, severity, name ? name : "", now, format, args);
  }
}

/**
 * @brief 记录日志消息 (Log a message)
 * @param[in] location 日志消息的源代码位置 (Source code location of the log message)
 * @param[in] severity 日志消息的严重性级别 (Severity level of the log message)
 * @param[in] name 日志记录器名称 (Logger name)
 * @param[in] format 格式化字符串 (Format string)
 * @param[in] ... 可变参数列表 (Variable argument list)
 */
void rcutils_log(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  const char * format,
  ...)
{
  // 如果指定的日志记录器未启用给定的日志级别 (If the specified logger is not enabled for the given severity level)
  if (!rcutils_logging_logger_is_enabled_for(name, severity)) {
    return;
  }

  // 初始化可变参数列表 (Initialize the variable argument list)
  va_list args;
  va_start(args, format);
  // 调用内部日志函数处理可变参数列表 (Call the internal logging function to handle the variable argument list)
  vrcutils_log_internal(location, severity, name, format, &args);
  // 清理可变参数列表 (Clean up the variable argument list)
  va_end(args);
}

/**
 * @brief 内部日志函数，处理可变参数列表 (Internal logging function, handling variable argument list)
 * @param[in] location 日志消息的源代码位置 (Source code location of the log message)
 * @param[in] severity 日志消息的严重性级别 (Severity level of the log message)
 * @param[in] name 日志记录器名称 (Logger name)
 * @param[in] format 格式化字符串 (Format string)
 * @param[in] ... 可变参数列表 (Variable argument list)
 */
void rcutils_log_internal(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  const char * format,
  ...)
{
  // 初始化可变参数列表 (Initialize the variable argument list)
  va_list args;
  va_start(args, format);
  // 调用内部日志函数处理可变参数列表 (Call the internal logging function to handle the variable argument list)
  vrcutils_log_internal(location, severity, name, format, &args);
  // 清理可变参数列表 (Clean up the variable argument list)
  va_end(args);
}

/**
 * @brief 格式化日志消息 (Format a log message)
 * @param[in] location 日志消息的源代码位置 (Source code location of the log message)
 * @param[in] severity 日志消息的严重性级别 (Severity level of the log message)
 * @param[in] name 日志记录器名称 (Logger name)
 * @param[in] timestamp 日志消息的时间戳 (Timestamp of the log message)
 * @param[in] msg 日志消息文本 (Log message text)
 * @param[out] logging_output 格式化后的日志输出 (Formatted log output)
 * @return 成功返回RCUTILS_RET_OK，失败返回RCUTILS_RET_ERROR (Returns RCUTILS_RET_OK on success, RCUTILS_RET_ERROR on failure)
 */
rcutils_ret_t rcutils_logging_format_message(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  rcutils_time_point_value_t timestamp,
  const char * msg,
  rcutils_char_array_t * logging_output)
{
  // 初始化日志输入结构体 (Initialize the logging input structure)
  const logging_input_t logging_input = {
    .location = location, .severity = severity, .name = name, .timestamp = timestamp, .msg = msg};

  // 遍历所有日志消息处理函数 (Iterate through all log message handlers)
  for (size_t i = 0; i < g_num_log_msg_handlers; ++i) {
    // 调用处理函数，格式化日志输出 (Call the handler function to format the log output)
    if (
      g_handlers[i].handler(
        &logging_input, logging_output, g_handlers[i].start_offset, g_handlers[i].end_offset) ==
      NULL) {
      return RCUTILS_RET_ERROR;
    }
  }

  return RCUTILS_RET_OK;
}

#ifdef _WIN32
#define COLOR_NORMAL 7
#define COLOR_RED 4
#define COLOR_GREEN 2
#define COLOR_YELLOW 6
#define IS_STREAM_A_TTY(stream) (_isatty(_fileno(stream)) != 0)
#else
#define COLOR_NORMAL "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define IS_STREAM_A_TTY(stream) (isatty(fileno(stream)) != 0)
#endif

/** \brief 设置日志输出颜色 (Set log output color)
 *
 * 该宏根据给定的严重程度设置日志输出的颜色。 (This macro sets the log output color according to the given severity.)
 *
 * \param[out] status 返回状态 (Return status)
 * \param[in] severity 日志严重程度 (Log severity level)
 * \param[out] color 颜色代码 (Color code)
 */
#define SET_COLOR_WITH_SEVERITY(status, severity, color)                                                                                             \
  {                                                                                                                                                  \
    /* 根据严重程度选择颜色 (Select color based on severity) */                                                                            \
    switch (severity) {                                                                                                                              \
    /* 调试级别，绿色 (Debug level, green) */                                                                                                 \
    case RCUTILS_LOG_SEVERITY_DEBUG:                                                                                                                 \
      color = COLOR_GREEN;                                                                                                                           \
      break;                                                                                                                                         \
    /* 信息级别，正常颜色 (Info level, normal color) */                                                                                     \
    case RCUTILS_LOG_SEVERITY_INFO:                                                                                                                  \
      color = COLOR_NORMAL;                                                                                                                          \
      break;                                                                                                                                         \
    /* 警告级别，黄色 (Warning level, yellow) */                                                                                              \
    case RCUTILS_LOG_SEVERITY_WARN:                                                                                                                  \
      color = COLOR_YELLOW;                                                                                                                          \
      break;                                                                                                                                         \
    /* 错误和致命级别，红色 (Error and fatal levels, red) */                                                                               \
    case RCUTILS_LOG_SEVERITY_ERROR:                                                                                                                 \
    case RCUTILS_LOG_SEVERITY_FATAL:                                                                                                                 \
      color = COLOR_RED;                                                                                                                             \
      break;                                                                                                                                         \
    /* 默认情况下，打印未知严重程度并设置无效参数状态 (By default, print unknown severity and set invalid argument status) */ \
    default:                                                                                                                                         \
      RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING("unknown severity level: %d\n", severity);                                                    \
      status = RCUTILS_RET_INVALID_ARGUMENT;                                                                                                         \
    }                                                                                                                                                \
  }

#ifdef _WIN32
/** \brief 使用给定的颜色设置控制台输出颜色 (Set console output color with the given color)
 *
 * 该宏用于在Windows系统上设置控制台输出颜色。 (This macro is used to set console output color on Windows systems.)
 *
 * \param[out] status 返回状态 (Return status)
 * \param[in] color 颜色代码 (Color code)
 * \param[in] handle 控制台句柄 (Console handle)
 */
#define SET_OUTPUT_COLOR_WITH_COLOR(status, color, handle)                                                                                 \
  {                                                                                                                                        \
    /* 如果状态为RCUTILS_RET_OK，则设置控制台文本属性 (If the status is RCUTILS_RET_OK, set the console text attribute) */ \
    if (RCUTILS_RET_OK == status) {                                                                                                        \
      if (!SetConsoleTextAttribute(handle, color)) {                                                                                       \
        /* 获取错误代码 (Get error code) */                                                                                          \
        DWORD error = GetLastError();                                                                                                      \
        /* 将错误信息写入标准错误流 (Write error message to standard error stream) */                                          \
        RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(                                                                                  \
          "SetConsoleTextAttribute failed with error code %lu.\n", error);                                                                 \
        /* 设置返回状态为错误 (Set return status to error) */                                                                     \
        status = RCUTILS_RET_ERROR;                                                                                                        \
      }                                                                                                                                    \
    }                                                                                                                                      \
  }

/** \brief 从输出流获取控制台句柄 (Get console handle from output stream)
 *
 * 该宏用于在Windows系统上获取与输出流关联的控制台句柄。 (This macro is used to get the console handle associated with the output stream on Windows systems.)
 *
 * \param[out] status 返回状态 (Return status)
 * \param[out] handle 控制台句柄 (Console handle)
 */
#define GET_HANDLE_FROM_STREAM(status, handle)                                                                                                                      \
  {                                                                                                                                                                 \
    /* 如果状态为RCUTILS_RET_OK，则获取控制台句柄 (If the status is RCUTILS_RET_OK, get the console handle) */                                        \
    if (RCUTILS_RET_OK == status) {                                                                                                                                 \
      /* 如果输出流为stdout，获取标准输出句柄 (If the output stream is stdout, get the standard output handle) */                                    \
      if (g_output_stream == stdout) {                                                                                                                              \
        handle = GetStdHandle(STD_OUTPUT_HANDLE);                                                                                                                   \
      } else {                                                                                                                                                      \
        /* 否则，获取标准错误句柄 (Otherwise, get the standard error handle) */                                                                          \
        handle = GetStdHandle(STD_ERROR_HANDLE);                                                                                                                    \
      }                                                                                                                                                             \
      /* 如果句柄无效，获取错误代码并设置返回状态为错误 (If the handle is invalid, get the error code and set the return status to error) */ \
      if (INVALID_HANDLE_VALUE == handle) {                                                                                                                         \
        DWORD error = GetLastError();                                                                                                                               \
        RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(                                                                                                           \
          "GetStdHandle failed with error code %lu.\n", error);                                                                                                     \
        status = RCUTILS_RET_ERROR;                                                                                                                                 \
      }                                                                                                                                                             \
    }                                                                                                                                                               \
  }
#define SET_OUTPUT_COLOR_WITH_SEVERITY(status, severity, output_array)                             \
  {                                                                                                \
    WORD color = COLOR_NORMAL;                                                                     \
    HANDLE handle = INVALID_HANDLE_VALUE;                                                          \
    SET_COLOR_WITH_SEVERITY(status, severity, color)                                               \
    GET_HANDLE_FROM_STREAM(status, handle)                                                         \
    SET_OUTPUT_COLOR_WITH_COLOR(status, color, handle)                                             \
  }
#define SET_STANDARD_COLOR_IN_STREAM(is_colorized, status)                                         \
  {                                                                                                \
    if (is_colorized) {                                                                            \
      HANDLE handle = INVALID_HANDLE_VALUE;                                                        \
      GET_HANDLE_FROM_STREAM(status, handle)                                                       \
      SET_OUTPUT_COLOR_WITH_COLOR(status, COLOR_NORMAL, handle)                                    \
    }                                                                                              \
  }
#define SET_STANDARD_COLOR_IN_BUFFER(is_colorized, status, output_array)
#else
#define SET_OUTPUT_COLOR_WITH_COLOR(status, color, output_array)                                   \
  {                                                                                                \
    if (RCUTILS_RET_OK == status) {                                                                \
      status = rcutils_char_array_strncat(&output_array, color, strlen(color));                    \
      if (RCUTILS_RET_OK != status) {                                                              \
        RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(                                          \
          "Error: rcutils_char_array_strncat failed with: %d\n", status);                          \
      }                                                                                            \
    }                                                                                              \
  }
#define SET_OUTPUT_COLOR_WITH_SEVERITY(status, severity, output_array)                             \
  {                                                                                                \
    const char * color = NULL;                                                                     \
    SET_COLOR_WITH_SEVERITY(status, severity, color)                                               \
    SET_OUTPUT_COLOR_WITH_COLOR(status, color, output_array)                                       \
  }
#define SET_STANDARD_COLOR_IN_BUFFER(is_colorized, status, output_array)                           \
  {                                                                                                \
    if (is_colorized) {                                                                            \
      SET_OUTPUT_COLOR_WITH_COLOR(status, COLOR_NORMAL, output_array)                              \
    }                                                                                              \
  }
#define SET_STANDARD_COLOR_IN_STREAM(is_colorized, status)
#endif

/**
 * @brief 控制台日志输出处理器 (Console log output handler)
 *
 * @param[in] location 日志位置信息 (Log location information)
 * @param[in] severity 日志严重性级别 (Log severity level)
 * @param[in] name 日志记录名称 (Logger name)
 * @param[in] timestamp 时间戳 (Timestamp)
 * @param[in] format 格式化字符串 (Format string)
 * @param[in] args 可变参数列表 (Variable argument list)
 */
void rcutils_logging_console_output_handler(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  rcutils_time_point_value_t timestamp,
  const char * format,
  va_list * args)
{
  // 初始化状态变量 (Initialize the status variable)
  rcutils_ret_t status = RCUTILS_RET_OK;
  // 是否使用彩色输出 (Whether to use colorized output)
  bool is_colorized = false;

  // 如果日志系统未初始化，则输出错误信息并返回 (If the logging system is not initialized, output an error message and return)
  if (!g_rcutils_logging_initialized) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("logging system isn't initialized: "
                                  "call to rcutils_logging_console_output_handler failed.\n");
    return;
  }

  // 检查严重性级别是否有效 (Check if the severity level is valid)
  switch (severity) {
  case RCUTILS_LOG_SEVERITY_DEBUG:
  case RCUTILS_LOG_SEVERITY_INFO:
  case RCUTILS_LOG_SEVERITY_WARN:
  case RCUTILS_LOG_SEVERITY_ERROR:
  case RCUTILS_LOG_SEVERITY_FATAL:
    break;
  default:
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING("unknown severity level: %d\n", severity);
    return;
  }

  // 根据全局设置确定是否使用彩色输出 (Determine whether to use colorized output based on
  if (g_colorized_output == RCUTILS_COLORIZED_OUTPUT_FORCE_ENABLE) {
    is_colorized = true;
  } else if (g_colorized_output == RCUTILS_COLORIZED_OUTPUT_FORCE_DISABLE) {
    is_colorized = false;
  } else {
    is_colorized = IS_STREAM_A_TTY(g_output_stream);
  }

/**
 * @brief 该功能示例展示了如何在ROS2项目中使用rmw相关的代码。
 *        This example demonstrates how to use rmw-related code in a ROS2 project.
 *
 * @param[out] msg_buf 用于存储消息字符串的字符数组。Character array for holding the message string.
 * @param[out] output_buf 用于存储输出字符串的字符数组。Character array for holding the output string.
 * @param[in] is_colorized 是否为输出文本添加颜色。Whether to colorize the output text.
 * @param[out] status 用于存储操作状态的变量。Variable for storing operation status.
 */

// 初始化msg_array，用于存储消息字符串
// Initialize msg_array for storing message string
char msg_buf[1024] = "";
rcutils_char_array_t msg_array = {
  .buffer = msg_buf,
  .owns_buffer = false,
  .buffer_length = 0u,
  .buffer_capacity = sizeof(msg_buf),
  .allocator = g_rcutils_logging_allocator};

// 初始化output_array，用于存储输出字符串
// Initialize output_array for storing output string
char output_buf[1024] = "";
rcutils_char_array_t output_array = {
  .buffer = output_buf,
  .owns_buffer = false,
  .buffer_length = 0u,
  .buffer_capacity = sizeof(output_buf),
  .allocator = g_rcutils_logging_allocator};

// 如果需要添加颜色，则根据严重性设置输出颜色
// If colorization is needed, set output color based on severity
if (is_colorized) {
  SET_OUTPUT_COLOR_WITH_SEVERITY(status, severity, output_array)
}

// 如果状态正常，则使用给定的格式和参数将消息写入msg_array
// If the status is OK, write the message into msg_array using the given format and arguments
if (RCUTILS_RET_OK == status) {
  status = rcutils_char_array_vsprintf(&msg_array, format, *args);
  if (RCUTILS_RET_OK != status) {
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
      "Error: rcutils_char_array_vsprintf failed with: %d\n", status);
  }
}

// 如果状态正常，则将格式化后的消息写入output_array
// If the status is OK, write the formatted message into output_array
if (RCUTILS_RET_OK == status) {
  status = rcutils_logging_format_message(
    location, severity, name, timestamp, msg_array.buffer, &output_array);
  if (RCUTILS_RET_OK != status) {
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
      "Error: rcutils_logging_format_message failed with: %d\n", status);
  }
}

// 在Windows系统中，此操作无效
// Does nothing in Windows
SET_STANDARD_COLOR_IN_BUFFER(is_colorized, status, output_array)

// 如果状态正常，则将输出数组的内容打印到预定义的输出流
// If the status is OK, print the contents of the output array to the predefined output stream
if (RCUTILS_RET_OK == status) {
  fprintf(g_output_stream, "%s\n", output_array.buffer);
}

// 仅在Windows系统中有效
// Only does something in Windows
// cppcheck-suppress uninitvar  // suppress cppcheck false positive
SET_STANDARD_COLOR_IN_STREAM(is_colorized, status)

// 结束msg_array并检查操作状态
// Finalize msg_array and check operation status
status = rcutils_char_array_fini(&msg_array);
if (RCUTILS_RET_OK != status) {
  RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to fini array.\n");
}

// 结束output_array并检查操作状态
// Finalize output_array and check operation status
status = rcutils_char_array_fini(&output_array);
if (RCUTILS_RET_OK != status) {
  RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to fini array.\n");
}
}
