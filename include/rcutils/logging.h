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

/// \file

#ifndef RCUTILS__LOGGING_H_
#define RCUTILS__LOGGING_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/macros.h"
#include "rcutils/time.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

#ifdef __cplusplus
extern "C" {
#endif

/// 分隔符，用于记录节点名称时使用。
/// The separator used when logging node names.
#define RCUTILS_LOGGING_SEPARATOR_STRING "."

/**
 * \def RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL
 * \brief 默认日志记录器的默认严重性级别。
 * \brief The default severity level of the default logger.
 */
#define RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL RCUTILS_LOG_SEVERITY_INFO

/// 日志系统是否已初始化的标志。
/// The flag if the logging system has been initialized.
RCUTILS_PUBLIC
extern bool g_rcutils_logging_initialized;

/// 使用指定的分配器初始化日志系统。
/// Initialize the logging system using the specified allocator.
/**
 * 如果日志系统未处于初始化状态，则进行初始化。
 * Initialize the logging system only if it was not in an initialized state.
 *
 * 如果传入了无效的分配器，初始化将失败。
 * If an invalid allocator is passed, the initialization will fail.
 * 否则，即使出现错误，此函数仍会将内部状态设置为已初始化，
 * 以避免自动从日志宏调用此函数时反复尝试失败的初始化。
 * Otherwise, this function will still set the internal state to initialized
 * even if an error occurs, to avoid repeated failing initialization attempts
 * since this function is called automatically from logging macros.
 * 若要重新尝试初始化，请在重新调用此函数之前调用 rcutils_logging_shutdown()。
 * To re-attempt initialization, call rcutils_logging_shutdown() before
 * re-calling this function.
 *
 * 如果发生多个错误，将返回最后一个错误的错误代码。
 * If multiple errors occur, the error code of the last error will be returned.
 *
 * 可以使用 `RCUTILS_CONSOLE_OUTPUT_FORMAT` 环境变量设置记录到控制台的消息的输出格式。
 * The `RCUTILS_CONSOLE_OUTPUT_FORMAT` environment variable can be used to set
 * the output format of messages logged to the console.
 * 可用的标记有：
 * Available tokens are:
 *   - `file_name`，调用者的完整文件名（包括路径）
 *   - `file_name`, the full file name of the caller including the path
 *   - `function_name`，调用者的函数名
 *   - `function_name`, the function name of the caller
 *   - `line_number`，调用者的行号
 *   - `line_number`, the line number of the caller
 *   - `message`，格式化后的消息字符串
 *   - `message`, the message string after it has been formatted
 *   - `name`，完整的日志记录器名称
 *   - `name`, the full logger name
 *   - `severity`，严重性级别的名称，例如 `INFO`
 *   - `severity`, the name of the severity level, e.g. `INFO`
 *   - `time`，以浮点秒为单位的日志消息的时间戳
 *   - `time`, the timestamp of log message in floating point seconds
 *   - `time_as_nanoseconds`，以整数纳秒为单位的日志消息的时间戳
 *   - `time_as_nanoseconds`, the timestamp of log message in integer nanoseconds
 *
 * `RCUTILS_COLORIZED_OUTPUT` 环境变量允许配置是否使用颜色。
 * The `RCUTILS_COLORIZED_OUTPUT` environment variable allows configuring if colours
 * are used or not. 可用的值有：
 *  - `1`: 强制使用颜色。
 *  - `0`: 不使用颜色。
 * 如果未设置，根据目标流是否为终端来决定是否使用颜色。
 * See `isatty` documentation.
 *
 * 格式字符串可以通过在花括号中引用这些标记来使用它们，
 * 例如 `"[{severity}] [{name}]: {message} ({function_name}() at {file_name}:{line_number})"`。
 * 可以使用任意数量的标记。
 * 格式字符串的限制是2048个字符。
 * The limit of the format string is 2048 characters.
 *
 * <hr>
 * 属性                 | 遵循性
 * Attribute          | Adherence
 * ------------------ | -------------
 * 分配内存             | 是
 * Allocates Memory   | Yes
 * 线程安全             | 否
 * Thread-Safe        | No
 * 使用原子操作         | 否
 * Uses Atomics       | No
 * 无锁                 | 是
 * Lock-Free          | Yes
 *
 * \param[in] allocator 要使用的 rcutils_allocator_t。
 * \return #RCUTILS_RET_OK，如果成功，或者
 * \return #RCUTILS_RET_INVALID_ARGUMENT，如果分配器无效，
 *   这种情况下初始化将失败，或者
 * \return #RCUTILS_RET_INVALID_ARGUMENT，如果从 `RCUTILS_CONSOLE_OUTPUT_FORMAT` 环境变量
 *   读取输出格式时发生错误，在这种情况下将使用默认格式，或者
 * \return #RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID，如果无法初始化内部日志记录器严重性级别映射，
 *   在这种情况下，日志记录器严重性级别将不可配置。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_logging_initialize_with_allocator(rcutils_allocator_t allocator);

/// 初始化日志系统。 (Initialize the logging system.)
/**
 * 使用默认分配器调用 rcutils_logging_initialize_with_allocator()。
 * 当使用日志宏时，此函数会自动调用。
 * (Call rcutils_logging_initialize_with_allocator() using the default allocator.
 * This function is called automatically when using the logging macros.)
 *
 * <hr>
 * 属性 (Attribute)          | 遵循 (Adherence)
 * ------------------ | -------------
 * 分配内存 (Allocates Memory)   | 是 (Yes)
 * 线程安全 (Thread-Safe)        | 否 (No)
 * 使用原子操作 (Uses Atomics)       | 否 (No)
 * 无锁 (Lock-Free)          | 是 (Yes)
 *
 * \return 如果成功，则返回 #RCUTILS_RET_OK，或者
 * (\return #RCUTILS_RET_OK if successful, or)
 * \return 如果在读取 `RCUTILS_CONSOLE_OUTPUT_FORMAT` 环境变量的输出格式时发生错误，
 * 则返回 #RCUTILS_RET_INVALID_ARGUMENT，在这种情况下将使用默认格式，或者
 * (\return #RCUTILS_RET_INVALID_ARGUMENT if an error occurs reading the output
 *   format from the `RCUTILS_CONSOLE_OUTPUT_FORMAT` environment variable, in
 *   which case the default format will be used, or)
 * \return 如果无法初始化内部记录器严重性级别映射，
 * 则返回 #RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID，在这种情况下记录器级别将不可配置。
 * (\return #RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID if the internal logger
 *   severity level map cannot be initialized, in which case logger levels
 *   will not be configurable.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_logging_initialize(void);

/// 关闭日志系统。 (Shutdown the logging system.)
/**
 * 释放为日志系统分配的资源。
 * 这将使系统处于等同于未初始化的状态。
 * (Free the resources allocated for the logging system.
 * This puts the system into a state equivalent to being uninitialized.)
 *
 * <hr>
 * 属性 (Attribute)          | 遵循 (Adherence)
 * ------------------ | -------------
 * 分配内存 (Allocates Memory)   | 否 (No)
 * 线程安全 (Thread-Safe)        | 否 (No)
 * 使用原子操作 (Uses Atomics)       | 否 (No)
 * 无锁 (Lock-Free)          | 是 (Yes)
 *
 * \return 如果成功，则返回 #RCUTILS_RET_OK，或者
 * (\return #RCUTILS_RET_OK if successful, or)
 * \return 如果无法完成内部记录器严重性级别映射，
 * 则返回 #RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID。
 * (\return #RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID if the internal logger
 *   severity level map cannot be finalized.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_logging_shutdown(void);

/// 在源代码中标识调用者位置的结构。 (The structure identifying the caller location in the source code.)
typedef struct rcutils_log_location_s
{
  /// 包含日志调用的函数名称。 (The name of the function containing the log call.)
  const char * function_name;
  /// 包含日志调用的源文件名称。 (The name of the source file containing the log call.)
  const char * file_name;
  /// 包含日志调用的行号。 (The line number containing the log call.)
  size_t line_number;
} rcutils_log_location_t;

/// 日志消息/记录器的严重性级别。 (The severity levels of log messages / loggers.)
/**
 * 注意：所有日志级别的最低有效位都为0，这是一种优化。
 * 如果添加新的日志级别，请确保新级别保持此属性。
 * (Note: all logging levels have their Least Significant Bit as 0, which is used as an
 * optimization.  If adding new logging levels, ensure that the new levels keep this property.)
 */
enum RCUTILS_LOG_SEVERITY {
  RCUTILS_LOG_SEVERITY_UNSET = 0,  ///< 未设置的日志级别 (The unset log level)
  RCUTILS_LOG_SEVERITY_DEBUG = 10, ///< 调试日志级别 (The debug log level)
  RCUTILS_LOG_SEVERITY_INFO = 20,  ///< 信息日志级别 (The info log level)
  RCUTILS_LOG_SEVERITY_WARN = 30,  ///< 警告日志级别 (The warn log level)
  RCUTILS_LOG_SEVERITY_ERROR = 40, ///< 错误日志级别 (The error log level)
  RCUTILS_LOG_SEVERITY_FATAL = 50, ///< 致命日志级别 (The fatal log level)
};

/// The names of severity levels.
/// 严重性级别的名称。
RCUTILS_PUBLIC
extern const char * const g_rcutils_log_severity_names[RCUTILS_LOG_SEVERITY_FATAL + 1];

/// Get a severity value from its string representation (e.g. DEBUG).
/// 从字符串表示中获取严重性值（例如，DEBUG）。
/**
 * String representation must match one of the values in
 * `g_rcutils_log_severity_names`, but is not case-sensitive.
 * Examples: UNSET, DEBUG, INFO, WARN, Error, fatal.
 *
 * 字符串表示必须与 `g_rcutils_log_severity_names` 中的一个值匹配，但不区分大小写。
 * 示例：UNSET、DEBUG、INFO、WARN、Error、fatal。
 *
 * \param[in] severity_string String representation of the severity, must be a
 *   null terminated c string
 * \param[in] allocator rcutils_allocator_t to be used
 * \param[in,out] severity The severity level as a represented by the
 *   `RCUTILS_LOG_SEVERITY` enum
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT on invalid arguments, or
 * \return #RCUTILS_RET_LOGGING_SEVERITY_STRING_INVALID if unable to match
 *   string, or
 * \return #RCUTILS_RET_ERROR if an unspecified error occured.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_logging_severity_level_from_string(
  const char * severity_string, rcutils_allocator_t allocator, int * severity);

/// The function signature to log messages.
/// 记录消息的函数签名。
/**
 * \param[in] location The location information about where the log came from
 * \param[in] severity The severity of the log message expressed as an integer
 * \param[in] name The name of the logger that this message came from
 * \param[in] timestamp The time at which the log message was generated
 * \param[in] format The list of arguments to insert into the formatted log message
 * \param[in] args The variable argument list
 */
typedef void (*rcutils_logging_output_handler_t)(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  rcutils_time_point_value_t timestamp,
  const char * format,
  va_list * args);

/// Get the current output handler.
/// 获取当前输出处理程序。
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * 属性              | 遵循情况
 * ------------------ | -------------
 * 分配内存          | 否，前提是日志系统已经初始化
 * 线程安全          | 否
 * 使用原子操作      | 否
 * 无锁              | 是
 *
 * \return The function pointer of the current output handler.
 * \return 当前输出处理程序的函数指针。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_logging_output_handler_t rcutils_logging_get_output_handler();

/// 设置当前输出处理器。
/// Set the current output handler.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] function 输出处理器的函数指针。
 * \param[in] function The function pointer of the output handler to be used.
 */
RCUTILS_PUBLIC
void rcutils_logging_set_output_handler(rcutils_logging_output_handler_t function);

/// 根据 RCUTILS_CONSOLE_OUTPUT_FORMAT 格式化日志消息
/// Formats a log message according to RCUTILS_CONSOLE_OUTPUT_FORMAT
/**
 * 用于输出处理器的格式化程序，通过执行令牌替换，使日志消息与 RCUTILS_CONSOLE_OUTPUT_FORMAT 中指定的格式匹配。
 * A formatter that is meant to be used by an output handler to format a log message to match
 * the format specified in RCUTILS_CONSOLE_OUTPUT_FORMAT by performing token replacement.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] location 日志来源的位置信息
 * \param[in] location The location information about where the log came from
 * \param[in] severity 以整数表示的日志消息严重性
 * \param[in] severity The severity of the log message expressed as an integer
 * \param[in] name 此消息来自的记录器名称
 * \param[in] name The name of the logger that this message came from
 * \param[in] timestamp 生成日志消息的时间
 * \param[in] timestamp The time at which the log message was generated
 * \param[in] msg 正在记录的消息
 * \param[in] msg The message being logged
 * \param[out] logging_output 格式化消息的输出缓冲区
 * \param[out] logging_output An output buffer for the formatted message
 * \return 成功时返回 #RCUTILS_RET_OK。
 * \return #RCUTILS_RET_OK if successful.
 * \return 如果发生内存分配错误，则返回 #RCUTILS_RET_BAD_ALLOC
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation error occured
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_logging_format_message(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  rcutils_time_point_value_t timestamp,
  const char * msg,
  rcutils_char_array_t * logging_output);

/// 获取记录器的默认级别。
/// Get the default level for loggers.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \return 级别。
 * \return The level.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_logging_get_default_logger_level();

/// 设置默认日志记录器的严重性级别。 (Set the default severity level for loggers.)
/**
 * 如果请求的严重性级别为 `RCUTILS_LOG_SEVERITY_UNSET`，则默认
 * 将恢复默认日志记录器（`RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL`）的默认值。
 * (If the severity level requested is `RCUTILS_LOG_SEVERITY_UNSET`, the default
 * value for the default logger (`RCUTILS_DEFAULT_LOGGER_DEFAULT_LEVEL`)
 * will be restored instead.)
 *
 * <hr>
 * 属性                | 符合性
 * ------------------ | -------------
 * 分配内存            | 否，前提是日志系统已初始化
 * 线程安全            | 否
 * 使用原子操作        | 否
 * 无锁                | 是
 * 
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] level 要使用的级别。 (The level to be used.)
 */
RCUTILS_PUBLIC
void rcutils_logging_set_default_logger_level(int level);

/// 获取日志记录器的严重性级别。 (Get the severity level for a logger.)
/**
 * 这只考虑指定日志记录器的严重性级别。
 * 要获取给定其祖先严重性级别的日志记录器的有效级别，
 * 请参阅 rcutils_logging_get_logger_effective_level()。
 * (This considers the severity level of the specified logger only.
 * To get the effective level of a logger given the severity level of its
 * ancestors, see rcutils_logging_get_logger_effective_level().)
 *
 * <hr>
 * 属性                | 符合性
 * ------------------ | -------------
 * 分配内存            | 否，前提是日志系统已初始化
 * 线程安全            | 否
 * 使用原子操作        | 否
 * 无锁                | 是
 * 
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] name 日志记录器的名称，必须是以空字符结尾的 c 字符串 (The name of the logger, must be null terminated c string)
 * \return 如果已设置日志记录器的级别，则返回该级别，或者 (The level of the logger if it has been set, or)
 * \return 如果未设置，则返回 `RCUTILS_LOG_SEVERITY_UNSET`，或者 (`RCUTILS_LOG_SEVERITY_UNSET` if unset, or)
 * \return 对于空名称，返回默认日志记录器级别，或者 (the default logger level for an empty name, or)
 * \return 如果参数无效，则返回 -1，或者 (-1 on invalid arguments, or)
 * \return 如果发生错误，则返回 -1 (-1 if an error occurred)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_logging_get_logger_level(const char * name);

/// 获取日志记录器及其名称长度的级别。 (Get the level for a logger and its name length.)
/**
 * 与 rcutils_logging_get_logger_level() 相同，但不依赖
 * 日志记录器名称为以空字符结尾的 c 字符串。
 * (Identical to rcutils_logging_get_logger_level() but without
 * relying on the logger name to be a null terminated c string.)
 *
 * <hr>
 * 属性                | 符合性
 * ------------------ | -------------
 * 分配内存            | 否，前提是日志系统已初始化
 * 线程安全            | 否
 * 使用原子操作        | 否
 * 无锁                | 是
 * 
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] name 日志记录器的名称 (The name of the logger)
 * \param[in] name_length 日志记录器名称长度 (Logger name length)
 * \return 如果已设置日志记录器的级别，则返回该级别，或者 (The level of the logger if it has been set, or)
 * \return 如果未设置，则返回 `RCUTILS_LOG_SEVERITY_UNSET`，或者 (`RCUTILS_LOG_SEVERITY_UNSET` if unset, or)
 * \return 对于空名称，返回默认日志记录器级别，或者 (the default logger level for an empty name, or)
 * \return 如果参数无效，则返回 -1，或者 (-1 on invalid arguments, or)
 * \return 如果发生错误，则返回 -1 (-1 if an error occurred)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_logging_get_logger_leveln(const char * name, size_t name_length);

/// 设置日志记录器的严重性级别。 (Set the severity level for a logger.)
/**
 * 如果指定了空字符串作为名称，则将设置默认日志记录器级别。
 * (If an empty string is specified as the name, the default logger level will be set.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] name 日志记录器的名称，必须是以空字符结尾的 c 字符串。 (The name of the logger, must be null terminated c string.)
 * \param[in] level 要使用的级别。 (The level to be used.)
 * \return `RCUTILS_RET_OK` 如果成功，或 (if successful, or)
 * \return `RCUTILS_RET_INVALID_ARGUMENT` 在无效参数上，或 (on invalid arguments, or)
 * \return `RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID` 如果严重性映射无效，或 (if severity map invalid, or)
 * \return `RCUTILS_RET_ERROR` 如果发生未指定的错误 (if an unspecified error occured)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_logging_set_logger_level(const char * name, int level);

/// 确定日志记录器是否针对严重性级别启用。 (Determine if a logger is enabled for a severity level.)
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] name 日志记录器的名称，必须是以空字符结尾的 c 字符串或 NULL。 (The name of the logger, must be null terminated c string or NULL.)
 * \param[in] severity 严重性级别。 (The severity level.)
 *
 * \return `true` 如果日志记录器针对该级别启用，或 (if the logger is enabled for the level, or)
 * \return `false` 否则。 (otherwise.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool rcutils_logging_logger_is_enabled_for(const char * name, int severity);

/// 确定日志记录器的有效级别。 (Determine the effective level for a logger.)
/**
 * 有效级别是由日志记录器的严重性级别确定的（如果已设置），否则它是日志记录器祖先中首次指定的严重性级别，
 * 从其最近的祖先开始。祖先层次结构由点分隔的日志记录器名称表示：名为“x”的日志记录器是“x.y”的祖先，
 * “x”和“x.y”都是“x.y.z”的祖先，等等。如果未为日志记录器或其任何祖先设置级别，则使用默认级别。
 * (The effective level is determined as the severity level of
 * the logger if it is set, otherwise it is the first specified severity
 * level of the logger's ancestors, starting with its closest ancestor.
 * The ancestor hierarchy is signified by logger names being separated by dots:
 * a logger named `x` is an ancestor of `x.y`, and both `x` and `x.y` are
 * ancestors of `x.y.z`, etc.
 * If the level has not been set for the logger nor any of its
 * ancestors, the default level is used.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] name 日志记录器的名称，必须是以空字符结尾的 c 字符串。 (The name of the logger, must be null terminated c string.)
 *
 * \return 级别，或 (The level, or)
 * \return -1 在无效参数上，或 (-1 on invalid arguments, or)
 * \return -1 如果发生错误。 (-1 if an error occurred.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_logging_get_logger_effective_level(const char * name);

/// 内部调用记录日志消息的函数。
/// Internal call to log a message.
/**
 * 无条件记录日志消息。
 * Unconditionally log a message.
 * 这是一个内部函数，假定调用者已经调用了 rcutils_logging_logger_is_enabled_for()。
 * This is an internal function, and assumes that the caller has already called
 * rcutils_logging_logger_is_enabled_for().
 * 最终用户软件永远不应该调用这个函数，而应该调用 rcutils_log() 或其中一个 RCUTILS_LOG_ 宏。
 * End-user software should never call this, and instead should call rcutils_log()
 * or one of the RCUTILS_LOG_ macros.
 *
 * 此函数的属性受当前设置的输出处理程序的影响。
 * The attributes of this function are influenced by the currently set output handler.
 *
 * <hr>
 * 属性                | 遵循程度
 * Attribute          | Adherence
 * ------------------ | -------------
 * 分配内存            | 不分配，对于格式化输出<=1023个字符
 * Allocates Memory   | No, for formatted outputs <= 1023 characters
 *                    | 是的，对于格式化输出>=1024个字符
 *                    | Yes, for formatted outputs >= 1024 characters
 * 线程安全             | 否
 * Thread-Safe        | No
 * 使用原子操作         | 否
 * Uses Atomics       | No
 * 无锁                 | 是
 * Lock-Free          | Yes
 *
 * \param[in] location 指向位置结构的指针或 NULL
 * \param[in] severity 严重性级别
 * \param[in] name 记录器的名称，必须是以空字符结尾的 c 字符串或 NULL
 * \param[in] format 格式字符串
 * \param[in] ... 可变参数
 */
RCUTILS_PUBLIC
void rcutils_log_internal(
  const rcutils_log_location_t *
    location, ///< 输入参数，指向位置结构的指针或 NULL (Input parameter, pointer to the location struct or NULL)
  int severity, ///< 输入参数，严重性级别 (Input parameter, severity level)
  const char *
    name, ///< 输入参数，记录器的名称，必须是以空字符结尾的 c 字符串或 NULL (Input parameter, name of the logger, must be null terminated c string or NULL)
  const char * format, ///< 输入参数，格式字符串 (Input parameter, format string)
  ...)                 ///< 输入参数，可变参数 (Input parameter, variable arguments)
  /// @cond Doxygen_Suppress
  RCUTILS_ATTRIBUTE_PRINTF_FORMAT(4, 5)
  /// @endcond
  ;

/// 记录日志消息。
/// Log a message.
/**
 * 此函数的属性受当前设置的输出处理程序的影响。
 * The attributes of this function are influenced by the currently set output handler.
 *
 * <hr>
 * 属性                | 遵循程度
 * Attribute          | Adherence
 * ------------------ | -------------
 * 分配内存            | 不分配，对于格式化输出<=1023个字符
 * Allocates Memory   | No, for formatted outputs <= 1023 characters
 *                    | 是的，对于格式化输出>=1024个字符
 *                    | Yes, for formatted outputs >= 1024 characters
 * 线程安全             | 是的，与自身线程安全 [1]
 * Thread-Safe        | Yes, with itself [1]
 * 使用原子操作         | 否
 * Uses Atomics       | No
 * 无锁                 | 是
 * Lock-Free          | Yes
 * <i>[1] 应该与自身线程安全，但与其他记录日志函数不安全</i>
 * <i>[1] should be thread-safe with itself but not with other logging functions</i>
 *
 * 这应该与自身线程安全，但与其他记录日志函数（执行诸如设置记录器级别之类的操作）不安全。
 * This should be thread-safe with itself, but is not thread-safe with other
 * logging functions that do things like set logger levels.
 *
 * \todo 在 rcutils 中，此函数与其他记录日志函数之间没有线程安全保证，
 *   即使用户可能今天就在同时调用它们。
 *   我们需要重新审视这些函数以解决这个问题，并在可以的地方提供保证，
 *   并在不能的地方更改栈中较高的函数以提供线程安全性。
 *
 * \param[in] location 指向位置结构的指针或 NULL
 * \param[in] severity 严重性级别
 * \param[in] name 记录器的名称，必须是以空字符结尾的 c 字符串或 NULL
 * \param[in] format 格式字符串
 * \param[in] ... 可变参数
 */
RCUTILS_PUBLIC
void rcutils_log(
  const rcutils_log_location_t *
    location, ///< 输入参数，指向位置结构的指针或 NULL (Input parameter, pointer to the location struct or NULL)
  int severity, ///< 输入参数，严重性级别 (Input parameter, severity level)
  const char *
    name, ///< 输入参数，记录器的名称，必须是以空字符结尾的 c 字符串或 NULL (Input parameter, name of the logger, must be null terminated c string or NULL)
  const char * format, ///< 输入参数，格式字符串 (Input parameter, format string)
  ...)                 ///< 输入参数，可变参数 (Input parameter, variable arguments)
  /// @cond Doxygen_Suppress
  RCUTILS_ATTRIBUTE_PRINTF_FORMAT(4, 5)
  /// @endcond
  ;

/// 默认输出处理器将日志消息输出到标准流。
/// The default output handler outputs log messages to the standard streams.
/**
 * 严重级别为 `DEBUG` 和 `INFO` 的消息被写入 `stdout`。
 * The messages with a severity level `DEBUG` and `INFO` are written to `stdout`.
 * 严重级别为 `WARN`、`ERROR` 和 `FATAL` 的消息被写入 `stderr`。
 * The messages with a severity level `WARN`, `ERROR`, and `FATAL` are written
 * to `stderr`.
 * 可以通过 `RCUTILS_CONSOLE_OUTPUT_FORMAT` 环境变量配置已记录消息的控制台输出格式：
 * see rcutils_logging_initialize_with_allocator() for details.
 * 若要配置是否使用颜色，可以使用 `RCUTILS_COLORIZED_OUTPUT`：
 * see rcutils_logging_initialize_with_allocator() for details.
 *
 * <hr>
 * 属性                  | 遵循
 * ------------------ | -------------
 * 分配内存            | 否
 * 线程安全             | 是，如果底层 *printf 函数是线程安全的
 * 使用原子操作        | 否
 * 无锁                  | 是
 *
 * \param[in] location 指向位置结构的指针或 NULL
 * \param[in] severity 严重性级别
 * \param[in] name 记录器的名称，必须为 null 结尾的 c 字符串
 * \param[in] timestamp 创建日志消息的时间戳
 * \param[in] format 格式字符串
 * \param[in] args 记录器使用的 `va_list`
 */
RCUTILS_PUBLIC
void rcutils_logging_console_output_handler(
  const rcutils_log_location_t * location,
  int severity,
  const char * name,
  rcutils_time_point_value_t timestamp,
  const char * format,
  va_list * args);

/**
 * \def RCUTILS_LOGGING_AUTOINIT
 * \brief 初始化 rcl 日志库。
 * 通常无需直接调用宏。
 * 所有日志宏确保已调用一次。
 */
#define RCUTILS_LOGGING_AUTOINIT                                                                   \
  RCUTILS_LOGGING_AUTOINIT_WITH_ALLOCATOR(rcutils_get_default_allocator())

/**
 * \def RCUTILS_LOGGING_AUTOINIT_WITH_ALLOCATOR
 * \brief 使用分配器初始化 rcl 日志库。
 * 通常无需直接调用宏。
 * 所有日志宏确保已调用一次。
 */
#define RCUTILS_LOGGING_AUTOINIT_WITH_ALLOCATOR(alloc)                                             \
  do {                                                                                             \
    if (RCUTILS_UNLIKELY(!g_rcutils_logging_initialized)) {                                        \
      if (rcutils_logging_initialize_with_allocator(alloc) != RCUTILS_RET_OK) {                    \
        RCUTILS_SAFE_FWRITE_TO_STDERR(                                                             \
          "[rcutils|" __FILE__ ":" RCUTILS_STRINGIFY(__LINE__) "] error initializing logging: ");  \
        RCUTILS_SAFE_FWRITE_TO_STDERR(rcutils_get_error_string().str);                             \
        RCUTILS_SAFE_FWRITE_TO_STDERR("\n");                                                       \
        rcutils_reset_error();                                                                     \
      }                                                                                            \
    }                                                                                              \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__LOGGING_H_
