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

/// \file

#ifndef RCUTILS__ENV_H_
#define RCUTILS__ENV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// 设置或取消设置进程范围的环境变量。
/// Set or un-set a process-scoped environment variable.
/**
 * 此函数通过将给定的字符串值复制到进程的全局环境变量存储中，修改当前进程的环境变量。
 * This function modifies the environment variables for the current process by
 * copying given string values into the process' global environment variable
 * store.
 *
 * \par 线程安全性：
 * \par Thread Safety:
 * 此函数不是线程安全的。在另一个线程可能正在读取或写入环境变量时，请注意不要修改环境变量。
 * This function is not thread-safe. Take care not to modify the environment variables while
 * another thread might be reading or writing environment variables.
 *
 * \par 平台一致性：
 * \par Platform Consistency:
 * 当将变量设置为空字符串（`""`）时，行为在各个平台之间有所不同。
 * The behavior when setting a variable to an empty string (`""`) differs
 * between platforms.
 * 在 Windows 上，该变量将被取消设置（就像 \p env_value 是 `NULL` 一样），而在其他平台上，该变量将如预期地设置为空字符串。
 * On Windows, the variable is un-set (as if \p env_value was
 * `NULL`), while on other platforms the variable is set to an empty string as
 * expected.
 *
 * \param[in] env_name 要修改的环境变量的名称。
 * \param[in] env_name Name of the environment variable to modify.
 * \param[in] env_value 要设置的环境变量的值，或为 `NULL` 以取消设置。
 * \param[in] env_value Value to set the environment variable to, or `NULL` to
 *   un-set.
 * \return 如果成功，则返回 `true`，或
 * \return `true` if success, or
 * \return 如果 env_name 无效或为 NULL，则返回 `false`，或
 * \return `false` if env_name is invalid or NULL, or
 * \return 如果失败，则返回 `false`。
 * \return `false` on failure.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool rcutils_set_env(const char * env_name, const char * env_value);

/// 检索给定环境变量的值（如果存在），否则返回 ""。
/// Retrieve the value of the given environment variable if it exists, or "".
/**
 * 返回到 env_value 输出参数的 c-string 只在下次调用此函数之前有效，因为它是指向静态存储的直接指针。
 * The c-string which is returned in the env_value output parameter is only valid until the next time this function is called, because it is a direct pointer to the static storage.
 *
 * 由此函数填充的 env_value 变量永远不应调用 free()。
 * The variable env_value populated by this function should never have free() called on it.
 *
 * 如果环境变量未设置，则返回空字符串。
 * If the environment variable is not set, an empty string will be returned.
 *
 * 在这两种情况下，环境变量设置或未设置，除非发生异常，否则将返回 NULL，在这种情况下，将返回错误字符串。
 * In both cases, environment variable set or unset, NULL is returned unless an exception has occurred, in which case the error string is returned.
 * 例如：
 * For example:
 *
 * ```c
 * #include <stdio.h>
 * #include <rcutils/env.h>
 * const char * env_value;
 * const char * error_str;
 * error_str = rcutils_get_env("SOME_ENV_VAR", &env_value);
 * if (error_str != NULL) {
 *   fprintf(stderr, "Error getting env var: %s\n", error_str);
 * }
 * printf("Valued of 'SOME_ENV_VAR': %s\n", env_value);
 * ```
 *
 * 不能同时在不同线程上与 rcutils_set_env（或任何平台特定等效项）一起并发调用此函数，但多个并发调用此函数是线程安全的。
 * This function cannot be concurrently called together with rcutils_set_env (or any platform specific equivalent) on different threads, but multiple concurrent calls to this function are thread safe.
 *
 * \param[in] env_name 环境变量的名称
 * \param[out] env_value 指向值 cstring 的指针，如果未设置，则为 ""
 * \return 成功时返回 NULL（成功可以返回空字符串），或
 * \return 失败时返回错误字符串。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
const char * rcutils_get_env(const char * env_name, const char ** env_value);

/// 获取主目录的完整路径。 (Retrieve the full path to the home directory.)
/**
 * 返回的 c-string 仅在下次调用此函数之前有效，因为它是指向静态存储的直接指针。
 * (The c-string which is returned is only valid until the next time this
 * function is called, because it is a direct pointer to the static storage.)
 * 另请注意，此处返回的字符串不应被释放。 (Also note that the string returned here should *not* be freed.)
 *
 * 该函数首先尝试获取 HOME 环境变量。 (The function first tries to get the HOME environment variable.)
 * 如果该变量存在且非空，将返回该变量。 (If that variable exists and is non-empty, that will be returned.)
 * 否则，在 Windows 上，函数尝试获取 USERPROFILE 环境变量。 (Otherwise, on Windows, the function tries to get the USERPROFILE environment variable.)
 * 如果该变量存在且非空，将返回该变量。 (If that variable exists and is non-empty, that will be returned.)
 * 否则，返回 NULL。 (Otherwise, NULL will be returned.)
 *
 * 与 rcutils_set_env（或任何平台特定的等效项）一起线程安全地调用此函数是不可能的，
 * 但是对这个函数的多次调用是线程安全的。
 * (This function cannot be thread-safely called together with rcutils_set_env
 * (or any platform specific equivalent), but multiple calls to this function are thread safe.)
 *
 * \return 成功时的主目录，或 (The home directory on success, or)
 * \return `NULL` 失败时。 (`NULL` on failure.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
const char * rcutils_get_home_dir(void);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__ENV_H_
