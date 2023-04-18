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

#ifndef _WIN32
#error time_win32.c is only intended to be used with win32 based systems
#endif // _WIN32

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/time.h"

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

#include "./common.h"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"

/**
 * @brief 获取系统当前时间（Get the current system time）
 * @param[out] now 存储系统当前时间的变量指针（Pointer to a variable that stores the current system time）
 * @return rcutils_ret_t 返回操作结果（Return the operation result）
 */
rcutils_ret_t rcutils_system_time_now(rcutils_time_point_value_t * now)
{
  // 检查输入参数是否为空（Check if the input argument is NULL）
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);

  // 定义 FILETIME 结构体变量（Define a FILETIME structure variable）
  FILETIME ft;

  // 获取系统当前精确时间，并存储到 FILETIME 结构体中（Get the current precise system time and store it in the FILETIME structure）
  GetSystemTimePreciseAsFileTime(&ft);

  // 定义 LARGE_INTEGER 结构体变量（Define a LARGE_INTEGER structure variable）
  LARGE_INTEGER li;

  // 将 FILETIME 结构体中的低位和高位分别赋值给 LARGE_INTEGER 结构体（Assign the low and high parts of the FILETIME structure to the LARGE_INTEGER structure）
  li.LowPart = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;

  // 调整为从1970年1月1日开始，参考：https://support.microsoft.com/en-us/kb/167296
  // (Adjust for January 1st, 1970, see: https://support.microsoft.com/en-us/kb/167296)
  li.QuadPart -= 116444736000000000;

  // 将100纳秒单位转换为纳秒单位（Convert from 100-nanosecond units to nanoseconds）
  *now = li.QuadPart * 100;

  // 返回操作成功（Return operation success）
  return RCUTILS_RET_OK;
}

/**
 * @brief 获取稳定的当前时间（Get the current stable time）
 * @param[out] now 存储稳定当前时间的变量指针（Pointer to a variable that stores the current stable time）
 * @return rcutils_ret_t 返回操作结果（Return the operation result）
 */
rcutils_ret_t rcutils_steady_time_now(rcutils_time_point_value_t * now)
{
  // 检查输入参数是否为空（Check if the input argument is NULL）
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(now, RCUTILS_RET_INVALID_ARGUMENT);

  // 定义 LARGE_INTEGER 结构体变量（Define LARGE_INTEGER structure variables）
  LARGE_INTEGER cpu_frequency, performance_count;

  // 查询性能频率和性能计数器，这些操作在 Windows XP 及更高版本中不会失败
  // (Query performance frequency and performance counter, these operations will not fail in Windows XP or higher)
  QueryPerformanceFrequency(&cpu_frequency);
  QueryPerformanceCounter(&performance_count);

  // 分别计算纳秒和秒，以防止中间计算溢出
  // (Calculate nanoseconds and seconds separately to prevent overflow in intermediate calculations)
  const rcutils_time_point_value_t whole_seconds =
    performance_count.QuadPart / cpu_frequency.QuadPart;
  const rcutils_time_point_value_t remainder_count =
    performance_count.QuadPart % cpu_frequency.QuadPart;
  const rcutils_time_point_value_t remainder_ns =
    RCUTILS_S_TO_NS(remainder_count) / cpu_frequency.QuadPart;
  const rcutils_time_point_value_t total_ns = RCUTILS_S_TO_NS(whole_seconds) + remainder_ns;

  // 将计算结果赋值给输出参数（Assign the calculated result to the output argument）
  *now = total_ns;

  // 返回操作成功（Return operation success）
  return RCUTILS_RET_OK;
}

#ifdef __cplusplus
}
#endif
