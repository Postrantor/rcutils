// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <string.h>

#include "rcutils/strerror.h"

/**
 * @brief 获取错误消息的字符串表示，将其存储在提供的缓冲区中 (Get the string representation of the error message and store it in the provided buffer)
 *
 * @param[out] buffer 用于存储错误消息的缓冲区 (Buffer to store the error message)
 * @param[in] buffer_length 缓冲区的长度 (Length of the buffer)
 */
void rcutils_strerror(char * buffer, size_t buffer_length)
{
#if defined(_WIN32)
  // 使用 Windows 平台特定的 strerror_s 函数获取错误消息 (Use the platform-specific strerror_s function on Windows to get the error message)
  strerror_s(buffer, buffer_length, errno);
#elif defined(_GNU_SOURCE) && (!defined(ANDROID) || __ANDROID_API__ >= 23) && !defined(__QNXNTO__)
  // 使用 GNU 特定的 strerror_r 函数获取错误消息 (Use the GNU-specific strerror_r function to get the error message)
  char * msg = strerror_r(errno, buffer, buffer_length);
  if (msg != buffer) {
    // 如果 msg 不等于缓冲区，则将 msg 的内容复制到缓冲区 (If msg is not equal to the buffer, copy the contents of msg to the buffer)
    strncpy(buffer, msg, buffer_length);
    // 确保缓冲区以空字符结尾 (Ensure the buffer ends with a null character)
    buffer[buffer_length - 1] = '\0';
  }
#else
  // 使用 XSI 兼容的 strerror_r 函数获取错误消息 (Use the XSI-compliant strerror_r function to get the error message)
  int error_status = strerror_r(errno, buffer, buffer_length);
  if (error_status != 0) {
    // 如果无法获取错误消息，则将缓冲区设置为 "Failed to get error" (If unable to get the error message, set the buffer to "Failed to get error")
    strncpy(buffer, "Failed to get error", buffer_length);
    // 确保缓冲区以空字符结尾 (Ensure the buffer ends with a null character)
    buffer[buffer_length - 1] = '\0';
  }
#endif
}

#ifdef __cplusplus
}
#endif
