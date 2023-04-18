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

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32 || defined __CYGWIN__
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
#include <Windows.h>
#pragma warning(pop)
#else
#include <libgen.h>
#include <unistd.h>
#endif

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/process.h"
#include "rcutils/strdup.h"

/**
 * @brief 获取当前进程的ID (Get the current process ID)
 *
 * @return int 返回当前进程的ID (Returns the current process ID)
 */
int rcutils_get_pid(void)
{
#if defined _WIN32 || defined __CYGWIN__
  // 对于Windows和Cygwin系统，使用GetCurrentProcessId()函数获取进程ID
  // (For Windows and Cygwin systems, use the GetCurrentProcessId() function to get the process ID)
  return (int)GetCurrentProcessId();
#else
  // 对于其他系统（如Linux），使用getpid()函数获取进程ID
  // (For other systems (such as Linux), use the getpid() function to get the process ID)
  return (int)getpid();
#endif
}

/**
 * @brief 获取可执行文件的名称 (Get the name of the executable)
 *
 * @param[in] allocator 分配器，用于分配内存空间 (Allocator for memory allocation)
 * @return char* 可执行文件名的指针 (Pointer to the executable name)
 */
char * rcutils_get_executable_name(rcutils_allocator_t allocator)
{
  // 检查分配器是否有效 (Check if the allocator is valid)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(&allocator, "invalid allocator", return NULL);

#if defined __APPLE__ || defined __FreeBSD__ || (defined __ANDROID__ && __ANDROID_API__ >= 21)
  // 获取程序名 (Get the program name)
  const char * appname = getprogname();
#elif defined __GNUC__ && !defined(__QNXNTO__) && !defined(__OHOS__)
  // 获取程序调用名 (Get the program invocation name)
  const char * appname = program_invocation_name;
#elif defined _WIN32 || defined __CYGWIN__
  // 定义程序名数组 (Define an array for the program name)
  char appname[MAX_PATH];
  // 获取模块文件名 (Get the module file name)
  int32_t size = GetModuleFileNameA(NULL, appname, MAX_PATH);
  if (size == 0) {
    return NULL;
  }
#elif defined __QNXNTO__ || defined __OHOS__
  // 声明外部变量 __progname (Declare external variable __progname)
  extern char * __progname;
  // 获取程序名 (Get the program name)
  const char * appname = __progname;
#else
#error "Unsupported OS"
#endif

  // 计算程序名长度 (Calculate the length of the program name)
  size_t applen = strlen(appname);

  // 由于上面的内存可能是静态的，调用者可能需要修改参数，因此在此处创建并返回副本 (Since the above memory may be static, and the caller may want to modify the argument, make and return a copy here)
  char * executable_name = allocator.allocate(applen + 1, allocator.state);
  if (NULL == executable_name) {
    return NULL;
  }

  // 获取可执行文件名（Unix可能返回绝对路径） (Get just the executable name (Unix may return the absolute path))
#if defined __APPLE__ || defined __FreeBSD__ || defined __GNUC__
  // 需要一个中间副本，因为basename可能会修改其参数 (We need an intermediate copy because basename may modify its arguments)
  char * intermediate = rcutils_strdup(appname, allocator);
  if (NULL == intermediate) {
    allocator.deallocate(executable_name, allocator.state);
    return NULL;
  }

  // 获取基本名称 (Get the base name)
  char * bname = basename(intermediate);
  // 计算基本名称长度 (Calculate the base name length)
  size_t baselen = strlen(bname);
  // 复制基本名称到可执行文件名 (Copy the base name to the executable name)
  memcpy(executable_name, bname, baselen);
  // 添加字符串结束符 (Add string terminator)
  executable_name[baselen] = '\0';
  // 释放中间副本内存 (Deallocate the memory of the intermediate copy)
  allocator.deallocate(intermediate, allocator.state);
#elif defined _WIN32 || defined __CYGWIN__
  // 分割路径 (Split the path)
  errno_t err = _splitpath_s(appname, NULL, 0, NULL, 0, executable_name, applen, NULL, 0);
  if (err != 0) {
    allocator.deallocate(executable_name, allocator.state);
    return NULL;
  }
#else
#error "Unsupported OS"
#endif

  // 返回可执行文件名 (Return the executable name)
  return executable_name;
}

#ifdef __cplusplus
}
#endif
