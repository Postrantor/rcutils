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

#if __cplusplus
extern "C" {
#endif

#include "rcutils/time.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/snprintf.h"

/**
 * @brief 将时间点值转换为纳秒字符串 (Convert a time point value to a nanoseconds string)
 *
 * @param[in] time_point 时间点指针，不能为空 (Pointer to the time point value, must not be NULL)
 * @param[out] str 用于存储结果的字符串指针，不能为空 (Pointer to the string where the result will be stored, must not be NULL)
 * @param[in] str_size 字符串缓冲区的大小 (Size of the string buffer)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_time_point_value_as_nanoseconds_string(
  const rcutils_time_point_value_t * time_point, char * str, size_t str_size)
{
  // 检查 time_point 是否为空，如果为空则返回 RCUTILS_RET_INVALID_ARGUMENT 错误
  // Check if time_point is NULL, return RCUTILS_RET_INVALID_ARGUMENT error if it is
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(time_point, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查 str 是否为空，如果为空则返回 RCUTILS_RET_INVALID_ARGUMENT 错误
  // Check if str is NULL, return RCUTILS_RET_INVALID_ARGUMENT error if it is
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(str, RCUTILS_RET_INVALID_ARGUMENT);

  // 如果字符串容量为0，则返回 RCUTILS_RET_OK
  // If the string capacity is 0, return RCUTILS_RET_OK
  if (0 == str_size) {
    return RCUTILS_RET_OK;
  }

  // 格式化 time_point 值并将其写入 str，如果格式化失败，则设置错误消息并返回 RCUTILS_RET_ERROR
  // Format the time_point value and write it to str, if formatting fails, set error message and return RCUTILS_RET_ERROR
  if (rcutils_snprintf(str, str_size, "%.19" PRId64, *time_point) < 0) {
    RCUTILS_SET_ERROR_MSG("failed to format time point into string as nanoseconds");
    return RCUTILS_RET_ERROR;
  }

  // 返回操作成功
  // Return operation success
  return RCUTILS_RET_OK;
}

/**
 * @brief 将时间点值转换为秒字符串 (Convert a time point value to a seconds string)
 *
 * @param[in] time_point 时间点指针，不能为空 (Pointer to the time point value, must not be NULL)
 * @param[out] str 用于存储结果的字符串指针，不能为空 (Pointer to the string where the result will be stored, must not be NULL)
 * @param[in] str_size 字符串缓冲区的大小 (Size of the string buffer)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_time_point_value_as_seconds_string(
  const rcutils_time_point_value_t * time_point, char * str, size_t str_size)
{
  // 检查 time_point 是否为空，如果为空则返回 RCUTILS_RET_INVALID_ARGUMENT 错误
  // Check if time_point is NULL, return RCUTILS_RET_INVALID_ARGUMENT error if it is
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(time_point, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查 str 是否为空，如果为空则返回 RCUTILS_RET_INVALID_ARGUMENT 错误
  // Check if str is NULL, return RCUTILS_RET_INVALID_ARGUMENT error if it is
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(str, RCUTILS_RET_INVALID_ARGUMENT);

  // 如果字符串容量为0，则返回 RCUTILS_RET_OK
  // If the string capacity is 0, return RCUTILS_RET_OK
  if (0 == str_size) {
    return RCUTILS_RET_OK;
  }

  // 计算 time_point 的绝对值，避免 C89 中负数问题，参见：https://stackoverflow.com/a/3604984/671658
  // Calculate the absolute value of time_point to avoid negative number issues in C89, see: https://stackoverflow.com/a/3604984/671658
  uint64_t abs_time_point = (uint64_t)llabs(*time_point);

  // 将时间点值分为两部分以避免浮点误差
  // Divide the time point value into two parts to avoid floating point error
  uint64_t seconds = abs_time_point / (1000u * 1000u * 1000u);
  uint64_t nanoseconds = abs_time_point % (1000u * 1000u * 1000u);

  // 格式化 time_point 值并将其写入 str，如果格式化失败，则设置错误消息并返回 RCUTILS_RET_ERROR
  // Format the time_point value and write it to str, if formatting fails, set error message and return RCUTILS_RET_ERROR
  if (
    rcutils_snprintf(
      str, str_size, "%s%.10" PRId64 ".%.9" PRId64, (*time_point >= 0) ? "" : "-", seconds,
      nanoseconds) < 0) {
    RCUTILS_SET_ERROR_MSG("failed to format time point into string as float seconds");
    return RCUTILS_RET_ERROR;
  }

  // 返回操作成功
  // Return operation success
  return RCUTILS_RET_OK;
}

#if __cplusplus
}
#endif
