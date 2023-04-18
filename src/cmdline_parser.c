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

#include <string.h>

#include "rcutils/cmdline_parser.h"

/**
 * @brief 检查命令行参数中是否存在指定的选项 (Check if the specified option exists in the command line arguments)
 *
 * @param begin 指向命令行参数数组的开始指针 (Pointer to the beginning of the command line arguments array)
 * @param end 指向命令行参数数组的结束指针 (Pointer to the end of the command line arguments array)
 * @param option 要检查的选项字符串 (The option string to check for)
 * @return 如果找到指定选项，返回 true，否则返回 false (Returns true if the specified option is found, otherwise returns false)
 */
bool rcutils_cli_option_exist(char ** begin, char ** end, const char * option)
{
  // 遍历命令行参数数组 (Iterate through the command line arguments array)
  for (size_t i = 0; i < (size_t)(end - begin); ++i) {
    // 如果找到匹配的选项，则返回 true (If a matching option is found, return true)
    if (strcmp(begin[i], option) == 0) {
      return true;
    }
  }
  // 如果没有找到匹配的选项，返回 false (Return false if no matching option is found)
  return false;
}

/**
 * @brief 获取命令行参数中指定选项的值 (Get the value of the specified option in the command line arguments)
 *
 * @param begin 指向命令行参数数组的开始指针 (Pointer to the beginning of the command line arguments array)
 * @param end 指向命令行参数数组的结束指针 (Pointer to the end of the command line arguments array)
 * @param option 要获取值的选项字符串 (The option string to get the value for)
 * @return 如果找到指定选项的值，返回指向该值的指针，否则返回 NULL (Returns a pointer to the value of the specified option if found, otherwise returns NULL)
 */
char * rcutils_cli_get_option(char ** begin, char ** end, const char * option)
{
  size_t idx = 0;
  size_t end_idx = (size_t)(end - begin);

  // 遍历命令行参数数组，查找指定选项 (Iterate through the command line arguments array, looking for the specified option)
  for (; idx < end_idx; ++idx) {
    if (strncmp(begin[idx], option, strlen(option)) == 0) {
      break;
    }
  }

  // 如果找到了指定选项并且它有对应的值，则返回该值的指针 (If the specified option is found and it has a corresponding value, return a pointer to that value)
  if (idx < end_idx - 1 && begin[idx++] != NULL) {
    return begin[idx];
  }

  // 如果没有找到指定选项或者它没有对应的值，返回 NULL (Return NULL if the specified option is not found or it has no corresponding value)
  return NULL;
}
