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

#ifndef RCUTILS__TIME_H_
#define RCUTILS__TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "rcutils/macros.h"
#include "rcutils/types.h"
#include "rcutils/visibility_control.h"

/// Convenience macro to convert seconds to nanoseconds.
#define RCUTILS_S_TO_NS(seconds) ((seconds) * (1000LL * 1000LL * 1000LL))
/// Convenience macro to convert milliseconds to nanoseconds.
#define RCUTILS_MS_TO_NS(milliseconds) ((milliseconds) * (1000LL * 1000LL))
/// Convenience macro to convert microseconds to nanoseconds.
#define RCUTILS_US_TO_NS(microseconds) ((microseconds)*1000LL)

/// Convenience macro to convert nanoseconds to seconds.
#define RCUTILS_NS_TO_S(nanoseconds) ((nanoseconds) / (1000LL * 1000LL * 1000LL))
/// Convenience macro to convert nanoseconds to milliseconds.
#define RCUTILS_NS_TO_MS(nanoseconds) ((nanoseconds) / (1000LL * 1000LL))
/// Convenience macro to convert nanoseconds to microseconds.
#define RCUTILS_NS_TO_US(nanoseconds) ((nanoseconds) / 1000LL)
/// Convenience macro for rcutils_steady_time_now(rcutils_time_point_value_t *).
#define RCUTILS_STEADY_TIME rcutils_steady_time_now

/// A single point in time, measured in nanoseconds since the Unix epoch.
typedef int64_t rcutils_time_point_value_t;
/// A duration of time, measured in nanoseconds.
typedef int64_t rcutils_duration_value_t;

/**
 * 这个函数从系统时钟返回时间。
 * 最接近的等价物是 std::chrono::system_clock::now();
 * (This function returns the time from a system clock.
 * The closest equivalent would be to std::chrono::system_clock::now();)
 *
 * 分辨率（例如，纳秒与微秒）不能保证。
 * (The resolution (e.g. nanoseconds vs microseconds) is not guaranteed.)
 *
 * 现在参数必须指向一个已分配的 rcutils_time_point_value_t 对象，
 * 因为结果被复制到这个变量中。
 * (The now argument must point to an allocated rcutils_time_point_value_t object,
 * as the result is copied into this variable.)
 *
 * <hr>
 * 属性                    | 遵循
 * ---------------------- | -------------
 * 分配内存                | 否
 * 线程安全                | 是
 * 使用原子操作            | 否
 * 无锁                    | 是
 * (Attribute             | Adherence
 * ---------------------- | -------------
 * Allocates Memory       | No
 * Thread-Safe            | Yes
 * Uses Atomics           | No
 * Lock-Free              | Yes)
 *
 * \param[out] now 存储当前时间的数据字段
 * (\param[out] now a datafield in which the current time is stored)
 * \return #RCUTILS_RET_OK 如果成功获取当前时间，或者
 * (\return #RCUTILS_RET_OK if the current time was successfully obtained, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果任何参数无效，或者
 * (\return #RCUTILS_RET_INVALID_ARGUMENT if any arguments are invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未指定的错误。
 * (\return #RCUTILS_RET_ERROR if an unspecified error occur.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_system_time_now(rcutils_time_point_value_t * now);

/// 以 rcutils_time_point_value_t 对象形式获取当前时间。
/// (Retrieve the current time as a rcutils_time_point_value_t object.)
/**
 * 这个函数从单调递增时钟返回时间。
 * 最接近的等价物是 std::chrono::steady_clock::now();
 * (This function returns the time from a monotonically increasing clock.
 * The closest equivalent would be to std::chrono::steady_clock::now();)
 *
 * 分辨率（例如，纳秒与微秒）不能保证。
 * (The resolution (e.g. nanoseconds vs microseconds) is not guaranteed.)
 *
 * 现在参数必须指向一个已分配的 rcutils_time_point_value_t 对象，
 * 因为结果被复制到这个变量中。
 * (The now argument must point to an allocated rcutils_time_point_value_t object,
 * as the result is copied into this variable.)
 *
 * <hr>
 * 属性                    | 遵循
 * ---------------------- | -------------
 * 分配内存                | 否
 * 线程安全                | 是
 * 使用原子操作            | 否
 * 无锁                    | 是
 * (Attribute             | Adherence
 * ---------------------- | -------------
 * Allocates Memory       | No
 * Thread-Safe            | Yes
 * Uses Atomics           | No
 * Lock-Free              | Yes)
 *
 * \param[out] now 存储当前时间的结构
 * (\param[out] now a struct in which the current time is stored)
 * \return #RCUTILS_RET_OK 如果成功获取当前时间，或者
 * (\return #RCUTILS_RET_OK if the current time was successfully obtained, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果任何参数无效，或者
 * (\return #RCUTILS_RET_INVALID_ARGUMENT if any arguments are invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未指定的错误。
 * (\return #RCUTILS_RET_ERROR if an unspecified error occur.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_steady_time_now(rcutils_time_point_value_t * now);

/// 以字符串形式返回纳秒级的时间点。 (Return a time point as nanoseconds in a string.)
/**
 * 此数字总是固定宽度，左边用零填充至时间点最大可表示的位数。
 * 目前对于有符号的64位整数，这是19位（所以19个字符）。
 * 负值将有一个前导`-`，因此它们比正值长一个字符。
 * (The number is always fixed width, with left padding zeros up to the maximum
 * number of digits the time point can represent.
 * Right now that is 19 digits (so 19 characters) for a signed 64-bit integer.
 * Negative values will have a leading `-`, so they will be one character
 * longer than the positive values.)
 *
 * 建议输入字符串的最小大小为32个字符，但
 * 21（` `或`-`表示符号，19位数字，空终止符）应足够大，可容纳正值和负值。
 * 如果给定的字符串不够大，结果将被截断。
 * 如果需要具有可变宽度的字符串，建议直接使用`snprintf()`。
 * (The recommended minimum size of the input string is 32 characters, but
 * 21 (` ` or `-` for sign, 19 digits, null terminator) should be sufficiently
 * large for both positive and negative values.
 * If the given string is not large enough, the result will be truncated.
 * If you need a string with variable width, using `snprintf()` directly is
 * recommended.)
 *
 * <hr>
 * 属性                | 遵循性
 * ------------------ | -------------
 * 分配内存            | 否 [1]
 * 线程安全            | 是
 * 使用原子操作        | 否
 * 无锁                | 是
 * <i>[1] 如果`snprintf()`不会在内部分配额外的内存</i>
 * (Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No [1]
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] if `snprintf()` does not allocate additional memory internally</i>)
 *
 * \param[in] time_point 要转换为字符串的时间 (the time to be made into a string)
 * \param[out] str 存储输出字符串的位置 (the output string in which it is stored)
 * \param[in] str_size 输出字符串的大小 (the size of the output string)
 * \return #RCUTILS_RET_OK 如果成功（即使被截断），或者 (if successful (even if truncated), or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果任何参数无效，或者 (if any arguments are invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未指定的错误。 (if an unspecified error occur.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_time_point_value_as_nanoseconds_string(
  const rcutils_time_point_value_t * time_point, char * str, size_t str_size);

/// 以字符串形式返回浮点秒级的时间点。 (Return a time point as floating point seconds in a string.)
/**
 * 此数字总是固定宽度，左边用零填充至时间点最大可表示的尾数位数，且具有
 * 固定宽度为9位的特征（小数部分）。
 * 现在这意味着尾数始终为10位，加起来是19位，用于有符号的64位时间点类型。
 * 负值将有一个前导`-`，因此它们比正值长一个字符。
 * (The number is always fixed width, with left padding zeros up to the maximum
 * number of digits for the mantissa that the time point can represent and a
 * characteristic (fractional-part) with a fixed width of 9 digits.
 * Right now that means the mantissa is always 10 digits to add up to 19 total
 * for the signed 64-bit time point type.
 * Negative values will have a leading `-`, so they will be one character
 * longer then positive values.)
 *
 * 建议输入字符串的最小大小为32个字符，但
 * 22（` `或`-`表示符号，19位数字，小数点，空终止符）现在应该足够了。
 * 如果给定的字符串不够大，结果将被截断。
 * (The recommended minimum size of the input string is 32 characters, but
 * 22 (` ` or `-` for sign, 19 digits, decimal point, null terminator) should
 * be sufficient for now.
 * If the given string is not large enough, the result will be truncated.)
 *
 * <hr>
 * 属性                | 遵循性
 * ------------------ | -------------
 * 分配内存            | 否 [1]
 * 线程安全            | 是
 * 使用原子操作        | 否
 * 无锁                | 是
 * <i>[1] 如果`snprintf()`不会在内部分配额外的内存</i>
 * (Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No [1]
 * Thread-Safe        | Yes
 * Uses Atomics       | No
 * Lock-Free          | Yes
 * <i>[1] if `snprintf()` does not allocate additional memory internally</i>)
 *
 * \param[in] time_point 要转换为字符串的时间 (the time to be made into a string)
 * \param[out] str 存储输出字符串的位置 (the output string in which it is stored)
 * \param[in] str_size 输出字符串的大小 (the size of the output string)
 * \return #RCUTILS_RET_OK 如果成功（即使被截断），或者 (if successful (even if truncated), or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 如果任何参数无效，或者 (if any arguments are invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未指定的错误。 (if an unspecified error occur.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_time_point_value_as_seconds_string(
  const rcutils_time_point_value_t * time_point, char * str, size_t str_size);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__TIME_H_
