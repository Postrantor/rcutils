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

#ifndef RCUTILS__SHARED_LIBRARY_H_
#define RCUTILS__SHARED_LIBRARY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

/// 处理已加载的共享库的句柄。 (Handle to a loaded shared library.)
typedef struct RCUTILS_PUBLIC_TYPE rcutils_shared_library_s
{
  /// 平台特定的指向共享库的指针 (The platform-specific pointer to the shared library)
  void * lib_pointer;
  /// 共享库的路径 (The path of the shared_library)
  char * library_path;
  /// 分配器 (allocator)
  rcutils_allocator_t allocator;
} rcutils_shared_library_t;

/// 返回一个空的共享库结构体。 (Return an empty shared library struct.)
/**
 * 此函数返回一个空的和零初始化的共享库结构体。 (This function returns an empty and zero initialized shared library struct.)
 *
 * 示例：(Example:)
 *
 * ```c
 * // 不要这样做：(Do not do this:)
 * // rcutils_shared_library_t foo;
 * // rcutils_ret_t ret = rcutils_load_shared_library(
 * //     &foo,
 * //    "library_name",
 * //    rcutils_get_default_allocator()); // 未定义行为！(undefined behavior!)
 * // 或者 (or)
 * // rcutils_ret_t ret = rcutils_unload_shared_library(&foo); // 未定义行为！(undefined behavior!)
 *
 * // 而要这样做：(Do this instead:)
 * rcutils_shared_library_t bar = rcutils_get_zero_initialized_shared_library();
 * rcutils_load_shared_library(&bar, "library_name", rcutils_get_default_allocator()); // ok
 * void * symbol = rcutils_get_symbol(&bar, "bazinga"); // ok
 * bool is_bazinga_symbol = rcutils_has_symbol(&bar, "bazinga"); // ok
 * rcutils_ret_t ret = rcutils_unload_shared_library(&bar); // ok
 * if (ret != RCUTILS_RET_ERROR) {
 *   // 错误处理 (error handling)
 * }
 * ```
 * */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_shared_library_t rcutils_get_zero_initialized_shared_library(void);

/// 返回共享库指针。 (Return shared library pointer.)
/**
 * \param[inout] lib 具有共享库指针和共享库路径名的结构体 (struct with the shared library pointer and shared library path name)
 * \param[in] library_path 库路径字符串 (string with the path of the library)
 * \param[in] allocator 用于分配和释放内存的分配器 (allocator to be used to allocate and deallocate memory)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), 或者 (or)
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败 (if memory allocation fails), 或者 (or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误 (if an unknown error occurs), 或者 (or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数 (for invalid arguments).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_load_shared_library(
  rcutils_shared_library_t * lib, const char * library_path, rcutils_allocator_t allocator);

/// 返回共享库符号指针。 (Return shared library symbol pointer.)
/**
 * \param[in] lib 具有共享库指针和共享库路径名的结构体 (struct with the shared library pointer and shared library path name)
 * \param[in] symbol_name 共享库内的符号名 (name of the symbol inside the shared library)
 * \return 共享库符号指针 (shared library symbol pointer), 或者 (or)
 * \return `NULL` 如果符号不存在 (if the symbol doesn't exist).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
void * rcutils_get_symbol(const rcutils_shared_library_t * lib, const char * symbol_name);

/// 如果共享库包含特定符号名，则返回true，否则返回false。 (Return true if the shared library contains a specific symbol name otherwise returns false.)
/**
 * \param[in] lib 具有共享库指针和共享库路径名的结构体 (struct with the shared library pointer and shared library path name)
 * \param[in] symbol_name 共享库内的符号名 (name of the symbol inside the shared library)
 * \return `true` 如果符号存在 (if the symbol exists), 或者 (or)
 * \return `false` 否则 (otherwise).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool rcutils_has_symbol(const rcutils_shared_library_t * lib, const char * symbol_name);

/// 卸载共享库。 (Unload the shared library.)
/**
 * \param[in] lib 要完成的 rcutils_shared_library_t (rcutils_shared_library_t to be finalized)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), 或者 (or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数 (for invalid arguments), 或者 (or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误 (if an unknown error occurs)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_unload_shared_library(rcutils_shared_library_t * lib);

/// 检查库是否已加载。 (Check if the library is loaded.)
/**
 * 此函数仅确定当前共享库句柄上是否已调用“卸载”。 (This function only determines if "unload" has been called on the current shared library handle.)
 * 它很可能是第二个共享库句柄仍然打开，因此库被加载。 (It could very well be that a second shared library handle is still open and therefore the library being loaded.)
 * \param[in] lib 要检查的 rcutils_shared_library_t (rcutils_shared_library_t  to check)
 * \return `true` 如果库已加载 (if library is loaded), 或者 (or)
 * \return `false` 否则 (otherwise).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool rcutils_is_shared_library_loaded(rcutils_shared_library_t * lib);

/// 获取编译平台的库名称 (Get the library name for the compiled platform)
/**
 * \param[in] library_name 库基本名称（不带前缀和扩展名） (library base name (without prefix and extension))
 * \param[out] library_name_platform 编译平台的库名称 (library name for the compiled platform)
 * \param[in] buffer_size library_name_platform 缓冲区大小 (size of library_name_platform buffer)
 * \param[in] debug 如果为真，则库将返回调试库名称，否则返回正常库路径 (if true the library will return a debug library name, otherwise it returns a normal library path)
 * \return #RCUTILS_RET_OK 如果成功 (if successful), 或者 (or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误 (if an unknown error occurs)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_get_platform_library_name(
  const char * library_name, char * library_name_platform, unsigned int buffer_size, bool debug);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__SHARED_LIBRARY_H_
