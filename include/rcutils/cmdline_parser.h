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

#ifndef RCUTILS__CMDLINE_PARSER_H_
#define RCUTILS__CMDLINE_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "rcutils/visibility_control.h"

/// 返回命令行参数中是否定义了选项，如果定义了返回 `true`，否则返回 `false`。
/// Return `true` if the option is defined in the command line arguments or `false` otherwise.
/**
 * \param[in] begin 要检查的数组中的第一个元素
 * \param[in] end 要检查的数组中的最后一个元素
 * \param[in] option 要在参数数组中查找的字符串
 * \return 如果选项存在，则返回 `true`
 * \return 否则返回 `false`。
 */
RCUTILS_PUBLIC
bool rcutils_cli_option_exist(char ** begin, char ** end, const char * option);

/// 返回命令行参数中特定选项的值。
/// Return the value for a specific option of the command line arguments.
/**
 * \param[in] begin 要检查的数组中的第一个元素
 * \param[in] end 要检查的数组中的最后一个元素
 * \param[in] option 要在参数数组中查找的字符串
 * \return 命令行参数中特定选项的值，或者
 * \return 如果选项不存在，则返回 `NULL`。
 */
RCUTILS_PUBLIC
char * rcutils_cli_get_option(char ** begin, char ** end, const char * option);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__CMDLINE_PARSER_H_
