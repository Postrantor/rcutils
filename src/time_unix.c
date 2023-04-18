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

#if defined(_WIN32)
#error time_unix.c is not intended to be used with win32 based systems
#endif // defined(_WIN32)

#include "rcutils/time.h"

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif // defined(__MACH__) && defined(__APPLE__)
#include <math.h>

#if defined(__ZEPHYR__)
#include <version.h>
#if ZEPHYR_VERSION_CODE >= ZEPHYR_VERSION(3, 1, 0)
#include <zephyr/posix/time.h> //  Points to Zephyr toolchain posix time implementation
#else
#include <posix/time.h> //  Points to Zephyr toolchain posix time implementation
#endif
#else //  #if KERNELVERSION >= ZEPHYR_VERSION(3, 1, 0)
#include <time.h>
#endif //  defined(__ZEPHYR__)

#include <errno.h>
#include <unistd.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"

#if !defined(__MACH__) && !defined(__APPLE__) // Assume clock_get_time is available on OS X.
// This is an appropriate check for clock_gettime() according to:
//   http://man7.org/linux/man-pages/man2/clock_gettime.2.html
#if !defined(_POSIX_TIMERS) || !_POSIX_TIMERS
#error no monotonic clock function available
#endif // !defined(_POSIX_TIMERS) || !_POSIX_TIMERS
#endif // !defined(__MACH__) && !defined(__APPLE__)

/**
 * @brief 判断时间是否为负值 (Determine if the time is negative)
 *
 * @param[in] now 当前时间结构体指针 (Pointer to the current timespec structure)
 * @return 如果时间为负值返回true，否则返回false (Return true if the time is negative, false otherwise)
 */
static inline bool would_be_negative(const struct timespec * const now)
{
  // 检查秒数是否小于0 (Check if the seconds are less than 0)
  // 检查纳秒数是否小于0且秒数等于0 (Check if the nanoseconds are less than 0 and the seconds are equal to 0)
  return now->tv_sec < 0 || (now->tv_nsec < 0 && now->tv_sec == 0);
}

/**
 * @brief 获取系统当前时间 (Get the current system time)
 *
 * @param[out] now 系统时间的输出参数 (Output parameter for the system time)
 * @return 成功返回RCUTILS_RET_OK，失败返回相应的错误代码 (Return RCUTILS_RET_OK on success, corresponding error code on failure)
 */
rcutils_ret_t rcutils_system_time_now(rcutils_time_point_value_t * now)
{
  // 检查传入的参数是否为空 (Check if the input parameter is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);

  struct timespec timespec_now;

  // 使用clock_gettime(CLOCK_REALTIME)函数获取系统时间，这在Linux和macOS上都是一致的
  // (Use clock_gettime(CLOCK_REALTIME) function to get the system time, which is consistent on both Linux and macOS)
  // 在macOS上，请参考clang的实现 (For macOS, see the clang implementation at)
  // (https://github.com/llvm/llvm-project/blob/baebe12ad0d6f514cd33e418d6504075d3e79c0a/libcxx/src/chrono.cpp)
  if (clock_gettime(CLOCK_REALTIME, &timespec_now) < 0) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to get system time: %d", errno);
    return RCUTILS_RET_ERROR;
  }

  // 检查时间是否为负值，如果是则返回错误 (Check if the time is negative, return an error if it is)
  if (would_be_negative(&timespec_now)) {
    RCUTILS_SET_ERROR_MSG("unexpected negative time");
    return RCUTILS_RET_ERROR;
  }

  // 将秒数和纳秒数转换成纳秒，并将结果存入输出参数now中 (Convert seconds and nanoseconds to nanoseconds and store the result in the output parameter now)
  *now = RCUTILS_S_TO_NS((int64_t)timespec_now.tv_sec) + timespec_now.tv_nsec;

  return RCUTILS_RET_OK;
}

/**
 * @brief 获取稳定的当前时间 (Get the steady current time)
 *
 * @param[out] now 稳定时间的输出参数 (Output parameter for the steady time)
 * @return 成功返回RCUTILS_RET_OK，失败返回相应的错误代码 (Return RCUTILS_RET_OK on success, corresponding error code on failure)
 */
rcutils_ret_t rcutils_steady_time_now(rcutils_time_point_value_t * now)
{
  // 检查传入的参数是否为空 (Check if the input parameter is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);

  struct timespec timespec_now;
  clockid_t monotonic_clock = CLOCK_MONOTONIC;

#if defined(__MACH__) && defined(__APPLE__)
  // 在macOS上，使用CLOCK_MONOTONIC_RAW，这与clang的实现一致
  // (On macOS, use CLOCK_MONOTONIC_RAW, which matches the clang implementation)
  // (https://github.com/llvm/llvm-project/blob/baebe12ad0d6f514cd33e418d6504075d3e79c0a/libcxx/src/chrono.cpp)
  monotonic_clock = CLOCK_MONOTONIC_RAW;
#endif // defined(__MACH__) && defined(__APPLE__)

  // 使用clock_gettime(monotonic_clock)函数获取稳定时间 (Use clock_gettime(monotonic_clock) function to get the steady time)
  if (clock_gettime(monotonic_clock, &timespec_now) < 0) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to get steady time: %d", errno);
    return RCUTILS_RET_ERROR;
  }

  // 检查时间是否为负值，如果是则返回错误 (Check if the time is negative, return an error if it is)
  if (would_be_negative(&timespec_now)) {
    RCUTILS_SET_ERROR_MSG("unexpected negative time");
    return RCUTILS_RET_ERROR;
  }

  // 将秒数和纳秒数转换成纳秒，并将结果存入输出参数now中 (Convert seconds and nanoseconds to nanoseconds and store the result in the output parameter now)
  *now = RCUTILS_S_TO_NS((int64_t)timespec_now.tv_sec) + timespec_now.tv_nsec;

  return RCUTILS_RET_OK;
}
