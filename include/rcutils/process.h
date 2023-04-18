// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__PROCESS_H_
#define RCUTILS__PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// 获取当前进程ID。 (Retrieve the current process ID.)
/**
 * 此函数返回当前进程ID，始终成功。 (This function returns the current process ID, and is always successful.)
 *
 * 此功能是线程安全的。 (This function is thread-safe.)
 *
 * \return 当前进程ID。 (The current process ID.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int rcutils_get_pid(void);

/// 获取当前可执行文件名。 (Retrieve the current executable name.)
/**
 * 该函数以可移植的方式检索当前程序名称并返回其副本。 (This function portably retrieves the current program name and returns a copy of it.)
 * 调用者需要释放内存。 (It is up to the caller to free the memory.)
 *
 * 此功能是线程安全的。 (This function is thread-safe.)
 *
 * \param[in] allocator 要使用的分配器 (the allocator to use)
 * \return 成功时的程序名称，或 (The program name on success, or)
 * \return 失败时返回NULL。 (NULL on failure.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
char * rcutils_get_executable_name(rcutils_allocator_t allocator);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__PROCESS_H_
