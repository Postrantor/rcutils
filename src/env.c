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
#include <stdio.h>
#include <stdlib.h>

#include "rcutils/env.h"
#include "rcutils/error_handling.h"

/**
 * @brief 设置环境变量的值 (Set the value of an environment variable)
 *
 * @param[in] env_name 环境变量的名称 (The name of the environment variable)
 * @param[in] env_value 要设置的环境变量的值，如果为NULL，则删除该环境变量 (The value to set for the environment variable, if NULL, the variable will be removed)
 * @return 成功返回true，失败返回false (Returns true on success, false on failure)
 */
bool rcutils_set_env(const char * env_name, const char * env_value)
{
  // 如果发生错误，可以返回false (Can return false in case of error)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(false);

  // 检查env_name是否为空，如果为空则返回错误消息并返回false (Check if env_name is null, if it is, return an error message and return false)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(env_name, "env_name is null", return false);

#ifdef _WIN32
  // 如果env_value为空，将其设置为空字符串 (If env_value is null, set it to an empty string)
  if (NULL == env_value) {
    env_value = "";
  }
  // 使用_putenv_s函数设置环境变量，如果失败则设置错误消息并返回false (Use _putenv_s function to set the environment variable, if it fails, set an error message and return false)
  if (0 != _putenv_s(env_name, env_value)) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("_putenv_s failed: %d", errno);
    return false;
  }
#else
  // 如果env_value为空 (If env_value is null)
  if (NULL == env_value) {
    // 使用unsetenv函数删除环境变量，如果失败则设置错误消息并返回false (Use unsetenv function to remove the environment variable, if it fails, set an error message and return false)
    if (0 != unsetenv(env_name)) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("unsetenv failed: %d", errno);
      return false;
    }
  } else {
    // 使用setenv函数设置环境变量，如果失败则设置错误消息并返回false (Use setenv function to set the environment variable, if it fails, set an error message and return false)
    if (0 != setenv(env_name, env_value, 1)) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("setenv failed: %d", errno);
      return false;
    }
  }
#endif

  // 如果成功执行到这里，返回true (If successfully executed up to this point, return true)
  return true;
}

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

/**
 * @brief 获取环境变量的值 (Get the value of an environment variable)
 *
 * @param[in] env_name 环境变量的名称 (Name of the environment variable)
 * @param[out] env_value 存储环境变量值的指针 (Pointer to store the value of the environment variable)
 * @return 如果成功返回 NULL，否则返回错误信息 (Return NULL on success, otherwise return error message)
 */
const char * rcutils_get_env(const char * env_name, const char ** env_value)
{
  // 检查是否可以返回错误信息 (Check if it can return with an error message)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF("some string error");

  // 如果 env_name 为空，返回错误信息 (If env_name is NULL, return an error message)
  if (NULL == env_name) {
    return "argument env_name is null";
  }
  // 如果 env_value 为空，返回错误信息 (If env_value is NULL, return an error message)
  if (NULL == env_value) {
    return "argument env_value is null";
  }

  // 注意：在 Windows 上，getenv 已被弃用；请考虑使用 getenv_s 代替
  // Note: getenv is deprecated on Windows; consider using getenv_s instead
  *env_value = getenv(env_name);

  // 如果获取到的环境变量值为空，将其设置为空字符串 (If the obtained environment variable value is NULL, set it to an empty string)
  if (NULL == *env_value) {
    *env_value = "";
  }
  // 成功返回 NULL (Return NULL on success)
  return NULL;
}

#ifdef _WIN32
#pragma warning(pop)
#endif

/**
 * @brief 获取用户的主目录路径 (Get the user's home directory path)
 *
 * @return 主目录路径，如果无法获取则返回NULL (Home directory path, or NULL if not found)
 */
const char * rcutils_get_home_dir(void)
{
  const char * homedir;

  // 尝试获取 "HOME" 环境变量的值 (Try to get the value of the "HOME" environment variable)
  if (rcutils_get_env("HOME", &homedir) == NULL && *homedir != '\0') {
    // 如果 "HOME" 环境变量设置且非空，则返回它的值 (If the HOME environment variable is set and non-empty, return its value)
    return homedir;
  }

#ifdef _WIN32
  // 如果没有找到 "HOME" 变量，请尝试在 Windows 上使用 "USERPROFILE" 变量 (If the HOME variable wasn't found, try using the USERPROFILE variable on Windows)
  if (rcutils_get_env("USERPROFILE", &homedir) == NULL && *homedir != '\0') {
    // 如果 "USERPROFILE" 环境变量设置且非空，则返回它的值 (If the USERPROFILE environment variable is set and non-empty, return its value)
    return homedir;
  }
#endif

  // 如果无法获取主目录，则返回 NULL (If the home directory couldn't be obtained, return NULL)
  return NULL;
}

#ifdef __cplusplus
}
#endif
