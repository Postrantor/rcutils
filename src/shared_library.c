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
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(_GNU_SOURCE) && !defined(__QNXNTO__)
#include <link.h>
#elif defined(__QNXNTO__)
#include <sys/link.h>
#endif
#include <dlfcn.h>
#else
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
C_ASSERT(sizeof(void *) == sizeof(HINSTANCE));
#ifdef UNICODE
#error "rcutils does not support Unicode paths"
#endif
C_ASSERT(sizeof(char) == sizeof(TCHAR));
#endif // _WIN32

#include "rcutils/error_handling.h"
#include "rcutils/macros.h"
#include "rcutils/shared_library.h"
#include "rcutils/strdup.h"

/**
 * @brief 初始化一个 rcutils_shared_library_t 结构体 (Initialize an rcutils_shared_library_t structure)
 * 
 * @return rcutils_shared_library_t 返回一个初始化后的结构体 (Return an initialized structure)
 */
rcutils_shared_library_t rcutils_get_zero_initialized_shared_library(void)
{
  // 定义一个 rcutils_shared_library_t 类型的变量 (Define a variable of type rcutils_shared_library_t)
  rcutils_shared_library_t zero_initialized_shared_library;

  // 将 library_path 成员设置为 NULL (Set the library_path member to NULL)
  zero_initialized_shared_library.library_path = NULL;

  // 将 lib_pointer 成员设置为 NULL (Set the lib_pointer member to NULL)
  zero_initialized_shared_library.lib_pointer = NULL;

  // 使用 rcutils_get_zero_initialized_allocator() 函数初始化 allocator 成员 (Initialize the allocator member using the rcutils_get_zero_initialized_allocator() function)
  zero_initialized_shared_library.allocator = rcutils_get_zero_initialized_allocator();

  // 返回初始化后的结构体 (Return the initialized structure)
  return zero_initialized_shared_library;
}

/**
 * @brief 加载共享库 (Load a shared library)
 * @param[out] lib 一个指向 rcutils_shared_library_t 结构体的指针，用于存储加载后的库信息 (A pointer to an rcutils_shared_library_t structure to store the loaded library information)
 * @param[in] library_path 要加载的共享库文件的路径 (The path to the shared library file to load)
 * @param[in] allocator 分配器，用于分配内存 (The allocator to be used for memory allocation)
 * @return 返回 rcutils_ret_t 类型的状态码 (Returns an rcutils_ret_t type status code)
 */
rcutils_ret_t rcutils_load_shared_library(
  rcutils_shared_library_t * lib, const char * library_path, rcutils_allocator_t allocator)
{
  // 可以返回 RCUTILS_RET_INVALID_ARGUMENT 错误 (Can return with error of RCUTILS_RET_INVALID_ARGUMENT)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCUTILS_RET_INVALID_ARGUMENT);
  // 可以返回 RCUTILS_RET_BAD_ALLOC 错误 (Can return with error of RCUTILS_RET_BAD_ALLOC)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCUTILS_RET_BAD_ALLOC);
  // 可以返回 RCUTILS_RET_ERROR 错误 (Can return with error of RCUTILS_RET_ERROR)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RCUTILS_RET_ERROR);

  // 检查 lib 参数是否为空，为空则返回 RCUTILS_RET_INVALID_ARGUMENT 错误 (Check if lib argument is null, return RCUTILS_RET_INVALID_ARGUMENT error if null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib, RCUTILS_RET_INVALID_ARGUMENT);
  // 检查 library_path 参数是否为空，为空则返回 RCUTILS_RET_INVALID_ARGUMENT 错误 (Check if library_path argument is null, return RCUTILS_RET_INVALID_ARGUMENT error if null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(library_path, RCUTILS_RET_INVALID_ARGUMENT);
  // 检查分配器是否有效，无效则返回 RCUTILS_RET_INVALID_ARGUMENT 错误 (Check if allocator is valid, return RCUTILS_RET_INVALID_ARGUMENT error if invalid)
  RCUTILS_CHECK_ALLOCATOR(&allocator, return RCUTILS_RET_INVALID_ARGUMENT);
  // 如果 lib->lib_pointer 不为空，则设置错误消息并返回 RCUTILS_RET_INVALID_ARGUMENT 错误 (If lib->lib_pointer is not null, set error message and return RCUTILS_RET_INVALID_ARGUMENT error)
  if (NULL != lib->lib_pointer) {
    RCUTILS_SET_ERROR_MSG("lib argument is not zero-initialized");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 初始化 ret 为 RCUTILS_RET_OK (Initialize ret as RCUTILS_RET_OK)
  rcutils_ret_t ret = RCUTILS_RET_OK;
  // 将分配器赋值给 lib->allocator (Assign the allocator to lib->allocator)
  lib->allocator = allocator;

  // Delegate library path resolution to dynamic linker.
  // 将库路径解析委托给动态链接器。

  // Since the given library path might be relative, let the dynamic
  // linker search for the binary object in all standard locations and
  // current process space (if already loaded) first.
  // 然后使用获得的句柄查找其完整路径。
  // See POSIX dlopen() API and Windows' LoadLibrary() API documentation
  // for further reference.
  // 有关更多信息，请参阅 POSIX dlopen() API 和 Windows 的 LoadLibrary() API 文档。

#ifndef _WIN32
  // 如果不是在 Windows 系统上
  // If not on a Windows system

  // 使用 dlopen 函数尝试打开库文件，参数 RTLD_LAZY 表示延迟加载
  // Use the dlopen function to try opening the library file with the RTLD_LAZY flag, which indicates lazy loading
  lib->lib_pointer = dlopen(library_path, RTLD_LAZY);

  // 检查库指针是否为空，如果为空，则表示加载库失败
  // Check if the library pointer is NULL, if it is, then loading the library failed
  if (NULL == lib->lib_pointer) {
    // 设置错误消息，使用 dlerror 函数获取详细的错误信息
    // Set the error message, using the dlerror function to get detailed error information
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("dlopen error: %s", dlerror());
    // 返回错误代码
    // Return an error code
    return RCUTILS_RET_ERROR;
  }

#if defined(__APPLE__)
  const char * image_name =
    NULL; // 定义一个图像名指针，初始化为 NULL (Define an image name pointer, initialized to NULL)
  uint32_t image_count =
    _dyld_image_count(); // 获取当前进程加载的所有镜像数量 (Get the number of all images loaded by the current process)

  for (uint32_t i = 0; NULL == image_name && i < image_count; ++i) {
    // 反向迭代，因为库可能在列表的末尾附近 (Iterate in reverse as the library is likely near the end of the list)
    const char * candidate_name =
      _dyld_get_image_name(image_count - i - 1); // 获取候选图像名 (Get the candidate image name)
    if (NULL == candidate_name) {
      RCUTILS_SET_ERROR_MSG("dyld image index out of range");
      ret = RCUTILS_RET_ERROR;
      goto fail;
    }
    void * handle = dlopen(
      candidate_name,
      RTLD_LAZY | RTLD_NOLOAD); // 使用 dlopen 打开候选图像 (Open the candidate image using dlopen)
    if (handle == lib->lib_pointer) { // 如果找到匹配的句柄 (If the matching handle is found)
      image_name =
        candidate_name; // 设置图像名为候选图像名 (Set the image name as the candidate image name)
    }
    if (dlclose(handle) != 0) { // 关闭句柄，如果失败 (Close the handle, if it fails)
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("dlclose error: %s", dlerror());
      ret = RCUTILS_RET_ERROR;
      goto fail;
    }
  }
  if (NULL == image_name) {
    RCUTILS_SET_ERROR_MSG("dyld image name could not be found");
    ret = RCUTILS_RET_ERROR;
    goto fail;
  }
  lib->library_path = rcutils_strdup(
    image_name,
    lib
      ->allocator); // 分配内存并复制图像名到 library_path (Allocate memory and copy the image name to library_path)
#elif defined(_GNU_SOURCE) && !defined(__QNXNTO__) && !defined(__ANDROID__) && !defined(__OHOS__)
  struct link_map * map =
    NULL; // 定义一个 link_map 结构体指针，初始化为 NULL (Define a link_map struct pointer, initialized to NULL)
  if (
    dlinfo(lib->lib_pointer, RTLD_DI_LINKMAP, &map) !=
    0) { // 使用 dlinfo 获取链接映射信息 (Get the link map information using dlinfo)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("dlinfo error: %s", dlerror());
    ret = RCUTILS_RET_ERROR;
    goto fail;
  }
  lib->library_path = rcutils_strdup(
    map->l_name,
    lib
      ->allocator); // 分配内存并复制链接映射名到 library_path (Allocate memory and copy the link map name to library_path)
#else
  lib->library_path = rcutils_strdup(
    library_path,
    lib
      ->allocator); // 分配内存并复制库路径到 library_path (Allocate memory and copy the library path to library_path)
#endif
  // 这个函数用于处理库文件路径的分配和错误处理 (This function is for handling library path allocation and error handling)
  // 如果库路径为空，则进行错误处理 (If the library path is NULL, perform error handling)
  if (NULL == lib->library_path) {
    // 设置错误消息，表示无法分配内存 (Set error message, indicating inability to allocate memory)
    RCUTILS_SET_ERROR_MSG("unable to allocate memory");
    // 将返回值设为 RCUTILS_RET_BAD_ALLOC，表示分配失败 (Set return value to RCUTILS_RET_BAD_ALLOC, indicating allocation failure)
    ret = RCUTILS_RET_BAD_ALLOC;
    // 跳转至 fail 标签处进行清理操作 (Jump to the fail label for cleanup)
    goto fail;
  }

  // 如果一切正常，返回 RCUTILS_RET_OK 表示成功 (If everything is fine, return RCUTILS_RET_OK to indicate success)
  return RCUTILS_RET_OK;

// 定义 fail 标签，用于处理错误情况下的清理操作 (Define the fail label for handling cleanup in case of errors)
fail:
  // 如果 dlclose 函数返回非0值，表示关闭动态库失败 (If the dlclose function returns a non-zero value, it indicates that closing the dynamic library failed)
  if (dlclose(lib->lib_pointer) != 0) {
    // 将 dlclose 的错误信息安全地写入到标准错误输出 (Safely write the dlclose error message to standard error output)
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING("dlclose error: %s\n", dlerror());
  }
  // 将库指针设为 NULL，表示已经关闭 (Set the library pointer to NULL, indicating that it has been closed)
  lib->lib_pointer = NULL;
  // 返回之前设置的错误代码 (Return the previously set error code)
  return ret;
#else
  // 加载指定路径的库，并获取库文件名。
  // Load the library file
  HMODULE module = LoadLibrary(library_path);

  // 检查库文件是否加载成功
  // Check if the library file is loaded successfully
  if (!module) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("LoadLibrary error: %lu", GetLastError());
    return RCUTILS_RET_ERROR;
  }

  // 循环，直到找到足够大的缓冲区来存储库文件名
  // Loop until a large enough buffer is found to store the library file name
  for (DWORD buffer_capacity = MAX_PATH;; buffer_capacity *= 2) {
    // 分配缓冲区内存
    // Allocate memory for the buffer
    LPSTR buffer = lib->allocator.allocate(buffer_capacity, lib->allocator.state);

    // 检查内存分配是否成功
    // Check if memory allocation is successful
    if (NULL == buffer) {
      RCUTILS_SET_ERROR_MSG("unable to allocate memory");
      ret = RCUTILS_RET_BAD_ALLOC;
      goto fail;
    }

    // 获取库文件名并检查是否成功
    // Get the library file name and check if it is successful
    DWORD buffer_size = GetModuleFileName(module, buffer, buffer_capacity);
    if (0 == buffer_size) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("GetModuleFileName error: %lu", GetLastError());
      ret = RCUTILS_RET_ERROR;
      goto fail;
    }

    // 检查是否需要更大的缓冲区，如果是，则释放当前缓冲区并继续循环
    // Check if a larger buffer is needed, if so, release the current buffer and continue looping
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      lib->allocator.deallocate(buffer, lib->allocator.state);
      continue;
    }

    // 重新分配库路径内存并检查是否成功
    // Reallocate memory for the library path and check if it is successful
    lib->library_path = lib->allocator.reallocate(buffer, buffer_size + 1, lib->allocator.state);
    if (NULL == lib->library_path) {
      lib->library_path = buffer;
    }
    break;
  }

  // 保存库指针
  // Save the library pointer
  lib->lib_pointer = (void *)module;

  return RCUTILS_RET_OK;

fail:
  // 释放库并检查是否成功
  // Release the library and check if it is successful
  if (!FreeLibrary(module)) {
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING("FreeLibrary error: %lu\n", GetLastError());
  }
  return ret;
#endif // _WIN32
}

/**
 * @brief 获取共享库中的符号地址 (Get the address of a symbol in the shared library)
 *
 * @param[in] lib 指向 rcutils_shared_library_t 结构体的指针，表示要查找的共享库 (Pointer to the rcutils_shared_library_t structure representing the shared library to be searched)
 * @param[in] symbol_name 要查找的符号名称 (Name of the symbol to search for)
 * @return 返回符号在共享库中的地址，如果找不到或有错误发生，则返回 NULL (Returns the address of the symbol in the shared library, or NULL if not found or an error occurred)
 */
void * rcutils_get_symbol(const rcutils_shared_library_t * lib, const char * symbol_name)
{
  // 判断输入参数是否有效 (Check if input arguments are valid)
  if (!lib || !lib->lib_pointer || (symbol_name == NULL)) {
    // 设置错误信息 (Set error message)
    RCUTILS_SET_ERROR_MSG("invalid inputs arguments");
    return NULL;
  }

#ifndef _WIN32
  // 在共享库中查找符号并获取其地址（非 Windows 系统）(Find the symbol in the shared library and get its address (non-Windows systems))
  void * lib_symbol = dlsym(lib->lib_pointer, symbol_name);
  // 获取动态链接器错误信息 (Get dynamic linker error information)
  char * error = dlerror();
  if (error != NULL) {
    // 设置错误信息 (Set error message)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting the symbol '%s'. Error '%s'", symbol_name, error);
    return NULL;
  }
#else
  // 在共享库中查找符号并获取其地址（Windows 系统）(Find the symbol in the shared library and get its address (Windows systems))
  void * lib_symbol = GetProcAddress((HINSTANCE)(lib->lib_pointer), symbol_name);
  if (lib_symbol == NULL) {
    // 设置错误信息 (Set error message)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Error getting the symbol '%s'. Error '%d'", symbol_name, GetLastError());
    return NULL;
  }
#endif // _WIN32

  // 检查是否成功获取到符号地址 (Check if the symbol address was successfully obtained)
  if (!lib_symbol) {
    // 设置错误信息 (Set error message)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "symbol '%s' does not exist in the library '%s'", symbol_name, lib->library_path);
    return NULL;
  }

  // 返回符号地址 (Return the symbol address)
  return lib_symbol;
}

/**
 * @brief 检查共享库中是否存在给定的符号（函数或变量）
 * Check if the given symbol (function or variable) exists in the shared library.
 *
 * @param[in] lib 指向 rcutils_shared_library_t 结构的指针，表示共享库
 *                Pointer to rcutils_shared_library_t structure representing the shared library
 * @param[in] symbol_name 要检查的符号名称
 *                        Name of the symbol to check for
 * @return 如果符号存在，则返回 true，否则返回 false
 *         Returns true if the symbol exists, false otherwise
 */
bool rcutils_has_symbol(const rcutils_shared_library_t * lib, const char * symbol_name)
{
  // 检查输入参数是否有效
  // Check if input arguments are valid
  if (!lib || !lib->lib_pointer || symbol_name == NULL) {
    return false;
  }

#ifndef _WIN32
  // 正确的检查错误的方法是先调用 dlerror() 清除任何旧的错误条件，
  // 然后调用 dlsym()，再次调用 dlerror()，将其返回值保存到变量中，
  // 并检查此保存值是否不为 NULL。
  // The correct way to test for an error is to call dlerror() to clear any old error conditions,
  // then call dlsym(), and then call dlerror() again, saving its return value into a variable,
  // and check whether this saved value is not NULL.
  dlerror(); /* Clear any existing error */
  void * lib_symbol = dlsym(lib->lib_pointer, symbol_name);
  return dlerror() == NULL && lib_symbol != 0;
#else
  void * lib_symbol = GetProcAddress((HINSTANCE)(lib->lib_pointer), symbol_name);
  return lib_symbol != NULL;
#endif // _WIN32
}

/**
 * @brief 卸载共享库
 * Unload the shared library.
 *
 * @param[in,out] lib 指向 rcutils_shared_library_t 结构的指针，表示共享库
 *                    Pointer to rcutils_shared_library_t structure representing the shared library
 * @return 返回操作结果，成功返回 RCUTILS_RET_OK，否则返回相应的错误代码
 *         Returns the operation result, RCUTILS_RET_OK if successful, otherwise the corresponding error code
 */
rcutils_ret_t rcutils_unload_shared_library(rcutils_shared_library_t * lib)
{
  // 检查输入参数是否有效
  // Check if input arguments are valid
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->lib_pointer, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(lib->library_path, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&lib->allocator, return RCUTILS_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = RCUTILS_RET_OK;
#ifndef _WIN32
  // 函数 dlclose() 在成功时返回 0，在错误时返回非零。
  // The function dlclose() returns 0 on success, and nonzero on error.
  int error_code = dlclose(lib->lib_pointer);
  if (error_code) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("dlclose error: %s", dlerror());
#else
  // 如果函数成功，则返回值为非零。
  // If the function succeeds, the return value is nonzero.
  int error_code = FreeLibrary((HINSTANCE)(lib->lib_pointer));
  if (!error_code) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("FreeLibrary error: %lu", GetLastError());
#endif // _WIN32
    ret = RCUTILS_RET_ERROR;
  }

  // 释放 library_path 内存，并将指针置空
  // Deallocate the memory for library_path and set the pointer to NULL
  lib->allocator.deallocate(lib->library_path, lib->allocator.state);
  lib->library_path = NULL;
  lib->lib_pointer = NULL;

  // 将分配器重置为零初始化状态
  // Reset the allocator to zero-initialized state
  lib->allocator = rcutils_get_zero_initialized_allocator();
  
  return ret;
}

/**
 * @brief 获取平台相关的库名称 (Get the platform-specific library name)
 *
 * @param[in] library_name 库名称 (Library name)
 * @param[out] library_name_platform 平台相关的库名称 (Platform-specific library name)
 * @param[in] buffer_size library_name_platform 缓冲区的大小 (Size of the buffer for library_name_platform)
 * @param[in] debug 是否是调试版本的库 (Whether it is a debug version of the library)
 * @return rcutils_ret_t 返回状态 (Return status)
 */
rcutils_ret_t rcutils_get_platform_library_name(
  const char * library_name, char * library_name_platform, unsigned int buffer_size, bool debug)
{
  // 检查 library_name 参数是否为空 (Check if the library_name argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(library_name, RCUTILS_RET_INVALID_ARGUMENT);
  // 检查 library_name_platform 参数是否为空 (Check if the library_name_platform argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(library_name_platform, RCUTILS_RET_INVALID_ARGUMENT);

  // 初始化 written 变量 (Initialize the written variable)
  int written = 0;

  // 判断操作系统类型 (Determine the operating system type)
#if defined(__linux__) || defined(__QNXNTO__)
  // 如果是调试版本 (If it is a debug version)
  if (debug) {
    // 检查缓冲区大小是否足够 (Check if the buffer size is large enough)
    if (buffer_size >= (strlen(library_name) + 8)) {
      // 格式化输出平台相关的库名称 (Format the output platform-specific library name)
      written = rcutils_snprintf(
        library_name_platform, strlen(library_name) + 8, "lib%sd.so", library_name);
    }
  } else {
    // 检查缓冲区大小是否足够 (Check if the buffer size is large enough)
    if (buffer_size >= (strlen(library_name) + 7)) {
      // 格式化输出平台相关的库名称 (Format the output platform-specific library name)
      written =
        rcutils_snprintf(library_name_platform, strlen(library_name) + 7, "lib%s.so", library_name);
    }
  }
#elif __APPLE__
  // 如果是调试版本 (If it is a debug version)
  if (debug) {
    // 检查缓冲区大小是否足够 (Check if the buffer size is large enough)
    if (buffer_size >= (strlen(library_name) + 11)) {
      // 格式化输出平台相关的库名称 (Format the output platform-specific library name)
      written = rcutils_snprintf(
        library_name_platform, strlen(library_name) + 11, "lib%sd.dylib", library_name);
    }
  } else {
    // 检查缓冲区大小是否足够 (Check if the buffer size is large enough)
    if (buffer_size >= (strlen(library_name) + 10)) {
      // 格式化输出平台相关的库名称 (Format the output platform-specific library name)
      written = rcutils_snprintf(
        library_name_platform, strlen(library_name) + 10, "lib%s.dylib", library_name);
    }
  }
#elif _WIN32
  // 如果是调试版本 (If it is a debug version)
  if (debug) {
    // 检查缓冲区大小是否足够 (Check if the buffer size is large enough)
    if (buffer_size >= (strlen(library_name) + 6)) {
      // 格式化输出平台相关的库名称 (Format the output platform-specific library name)
      written =
        rcutils_snprintf(library_name_platform, strlen(library_name) + 6, "%sd.dll", library_name);
    }
  } else {
    // 检查缓冲区大小是否足够 (Check if the buffer size is large enough)
    if (buffer_size >= (strlen(library_name) + 5)) {
      // 格式化输出平台相关的库名称 (Format the output platform-specific library name)
      written =
        rcutils_snprintf(library_name_platform, strlen(library_name) + 5, "%s.dll", library_name);
    }
  }
#endif
  // 判断格式化输出是否成功 (Determine whether the formatted output is successful)
  if (written <= 0) {
    // 设置错误信息 (Set error message)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("failed to format library name: '%s'\n", library_name);
    // 返回错误状态 (Return error status)
    return RCUTILS_RET_ERROR;
  }
  // 返回成功状态 (Return success status)
  return RCUTILS_RET_OK;
}

/**
 * @brief 检查共享库是否已加载 (Check if a shared library is loaded)
 *
 * @param lib 指向 rcutils_shared_library_t 结构体的指针 (Pointer to an rcutils_shared_library_t structure)
 * @return 如果共享库已加载，则返回 true，否则返回 false (Returns true if the shared library is loaded, false otherwise)
 */
bool rcutils_is_shared_library_loaded(rcutils_shared_library_t *lib)
{
  // 检查 lib->lib_pointer 是否不为 NULL，如果不为 NULL，表示共享库已加载
  // (Check if lib->lib_pointer is not NULL, if it's not NULL, the shared library is loaded)
  return lib->lib_pointer != NULL;
}

#ifdef __cplusplus
}
#endif
