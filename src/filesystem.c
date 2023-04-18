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

#ifdef __cplusplus
extern "C" {
#endif
#include "rcutils/filesystem.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <dirent.h>
#include <unistd.h>
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
#include <direct.h>
#endif // _WIN32

#include "rcutils/env.h"
#include "rcutils/error_handling.h"
#include "rcutils/format_string.h"
#include "rcutils/repl_str.h"
#include "rcutils/strdup.h"

#ifdef _WIN32
#define RCUTILS_PATH_DELIMITER "\\"
#else
#define RCUTILS_PATH_DELIMITER "/"
#endif // _WIN32

/**
 * @brief 结构体 rcutils_dir_iter_state_t 用于记录目录迭代的状态（Structure rcutils_dir_iter_state_t for recording the state of directory iteration）
 */
typedef struct rcutils_dir_iter_state_t
{
#ifdef _WIN32
  HANDLE handle;        ///< Windows 系统下的句柄（Handle on Windows system）
  WIN32_FIND_DATA data; ///< Windows 系统下的文件查找数据（File find data on Windows system）
#else
  DIR * dir; ///< 其他系统下的目录指针（Directory pointer on other systems）
#endif
} rcutils_dir_iter_state_t;

/**
 * @brief 获取当前工作目录（Get current working directory）
 *
 * @param[in] buffer 存储当前工作目录的缓冲区（Buffer to store the current working directory）
 * @param[in] max_length 缓冲区的最大长度（Maximum length of the buffer）
 * @return 成功返回 true，失败返回 false（Return true if successful, false if failed）
 */
bool rcutils_get_cwd(char * buffer, size_t max_length)
{
  // 检查输入参数是否有效（Check if input parameters are valid）
  if (NULL == buffer || max_length == 0) {
    return false;
  }
#ifdef _WIN32
  // Windows 系统下获取当前工作目录（Get current working directory on Windows system）
  if (NULL == _getcwd(buffer, (int)max_length)) {
    return false;
  }
#else
  // 其他系统下获取当前工作目录（Get current working directory on other systems）
  if (NULL == getcwd(buffer, max_length)) {
    return false;
  }
#endif // _WIN32
  return true;
}

/**
 * @brief 判断给定的绝对路径是否为目录（Determine if the given absolute path is a directory）
 *
 * @param[in] abs_path 给定的绝对路径（The given absolute path）
 * @return 是目录返回 true，否则返回 false（Return true if it is a directory, otherwise return false）
 */
bool rcutils_is_directory(const char * abs_path)
{
  struct stat buf; // 存储文件状态信息的结构体（Structure for storing file status information）

  // 获取文件状态信息（Get file status information）
  if (stat(abs_path, &buf) < 0) {
    return false;
  }

#ifdef _WIN32
  // Windows 系统下判断是否为目录（Determine if it is a directory on Windows system）
  return (buf.st_mode & S_IFDIR) == S_IFDIR;
#else
  // 其他系统下判断是否为目录（Determine if it is a directory on other systems）
  return S_ISDIR(buf.st_mode);
#endif // _WIN32
}

/**
 * @brief 检查给定的绝对路径是否是一个文件 (Check if the given absolute path is a file)
 *
 * @param[in] abs_path 文件的绝对路径 (Absolute path of the file)
 * @return 如果是文件则返回 true，否则返回 false (Returns true if it's a file, otherwise returns false)
 */
bool rcutils_is_file(const char * abs_path)
{
  // 定义一个 stat 结构体变量 buf (Define a stat structure variable named buf)
  struct stat buf;

  // 获取文件状态并将结果存储在 buf 中，如果失败则返回 false (Get the file status and store the result in buf, return false if failed)
  if (stat(abs_path, &buf) < 0) {
    return false;
  }

#ifdef _WIN32
  // 在 Windows 系统下，检查 st_mode 是否为普通文件 (On Windows systems, check if st_mode is a regular file)
  return (buf.st_mode & S_IFREG) == S_IFREG;
#else
  // 在非 Windows 系统下，使用 S_ISREG 宏检查 st_mode 是否为普通文件 (On non-Windows systems, use the S_ISREG macro to check if st_mode is a regular file)
  return S_ISREG(buf.st_mode);
#endif // _WIN32
}

/**
 * @brief 检查给定的绝对路径是否存在 (Check if the given absolute path exists)
 *
 * @param[in] abs_path 要检查的绝对路径 (Absolute path to check)
 * @return 如果存在则返回 true，否则返回 false (Returns true if exists, otherwise returns false)
 */
bool rcutils_exists(const char * abs_path)
{
  // 定义一个 stat 结构体变量 buf (Define a stat structure variable named buf)
  struct stat buf;

  // 获取文件状态并将结果存储在 buf 中，如果失败则返回 false (Get the file status and store the result in buf, return false if failed)
  if (stat(abs_path, &buf) < 0) {
    return false;
  }

  // 如果成功获取文件状态，则返回 true (If the file status is successfully obtained, return true)
  return true;
}

/**
 * @brief 检查给定的绝对路径是否可读 (Check if the given absolute path is readable)
 *
 * @param[in] abs_path 要检查的绝对路径 (Absolute path to check)
 * @return 如果可读则返回 true，否则返回 false (Returns true if readable, otherwise returns false)
 */
bool rcutils_is_readable(const char * abs_path)
{
  // 定义一个 stat 结构体变量 buf (Define a stat structure variable named buf)
  struct stat buf;

  // 获取文件状态并将结果存储在 buf 中，如果失败则返回 false (Get the file status and store the result in buf, return false if failed)
  if (stat(abs_path, &buf) < 0) {
    return false;
  }

#ifdef _WIN32
  // 在 Windows 系统下，检查 st_mode 是否具有读权限 (On Windows systems, check if st_mode has read permission)
  if (!(buf.st_mode & _S_IREAD)) {
#else
  // 在非 Windows 系统下，检查 st_mode 是否具有用户读权限 (On non-Windows systems, check if st_mode has user read permission)
  if (!(buf.st_mode & S_IRUSR)) {
#endif // _WIN32
    // 如果没有读权限，则返回 false (If there is no read permission, return false)
    return false;
  }

  // 如果具有读权限，则返回 true (If there is read permission, return true)
  return true;
}

/**
 * @brief 检查给定的绝对路径文件是否可写 (Check if the given absolute path file is writable)
 * 
 * @param[in] abs_path 给定的绝对路径 (The given absolute path)
 * @return bool 文件是否可写 (Whether the file is writable)
 */
bool rcutils_is_writable(const char * abs_path)
{
  struct stat buf;
  // 判断文件状态，小于0表示错误 (Determine file status, less than 0 indicates an error)
  if (stat(abs_path, &buf) < 0) {
    return false;
  }
#ifdef _WIN32
  // Windows系统下检查文件是否可写 (Check if the file is writable under Windows system)
  if (!(buf.st_mode & _S_IWRITE)) {
#else
  // 非Windows系统下检查文件是否可写 (Check if the file is writable under non-Windows systems)
  if (!(buf.st_mode & S_IWUSR)) {
#endif // _WIN32
    return false;
  }
  return true;
}

/**
 * @brief 检查给定的绝对路径文件是否可读且可写 (Check if the given absolute path file is readable and writable)
 * 
 * @param[in] abs_path 给定的绝对路径 (The given absolute path)
 * @return bool 文件是否可读且可写 (Whether the file is readable and writable)
 */
bool rcutils_is_readable_and_writable(const char * abs_path)
{
  struct stat buf;
  // 判断文件状态，小于0表示错误 (Determine file status, less than 0 indicates an error)
  if (stat(abs_path, &buf) < 0) {
    return false;
  }
#ifdef _WIN32
  // NOTE(marguedas) on windows all writable files are readable
  // hence the following check is equivalent to "& _S_IWRITE"
  // Windows系统下检查文件是否可读且可写 (Check if the file is readable and writable under Windows system)
  if (!((buf.st_mode & _S_IWRITE) && (buf.st_mode & _S_IREAD))) {
#else
  // 非Windows系统下检查文件是否可读且可写 (Check if the file is readable and writable under non-Windows systems)
  if (!((buf.st_mode & S_IWUSR) && (buf.st_mode & S_IRUSR))) {
#endif // _WIN32
    return false;
  }
  return true;
}

/**
 * @brief 将两个路径字符串连接起来，使用指定的分配器 (Join two path strings together, using the specified allocator)
 * 
 * @param[in] left_hand_path 左侧路径字符串 (Left-hand path string)
 * @param[in] right_hand_path 右侧路径字符串 (Right-hand path string)
 * @param[in] allocator 指定的分配器 (The specified allocator)
 * @return char* 连接后的路径字符串 (The joined path string)
 */
char * rcutils_join_path(
  const char * left_hand_path, const char * right_hand_path, rcutils_allocator_t allocator)
{
  // 如果左侧路径为空，返回NULL (If the left-hand path is NULL, return NULL)
  if (NULL == left_hand_path) {
    return NULL;
  }
  // 如果右侧路径为空，返回NULL (If the right-hand path is NULL, return NULL)
  if (NULL == right_hand_path) {
    return NULL;
  }

  // 使用指定的分配器连接两个路径字符串 (Join the two path strings using the specified allocator)
  return rcutils_format_string(
    allocator, "%s%s%s", left_hand_path, RCUTILS_PATH_DELIMITER, right_hand_path);
}

/**
 * \brief 将路径转换为本地系统格式 (Convert a path to the native system format)
 * \param[in] path 要转换的路径字符串 (The path string to be converted)
 * \param[in] allocator 用于分配内存的 rcutils_allocator_t 结构体 (The rcutils_allocator_t structure used for memory allocation)
 * \return 转换后的路径字符串，如果输入路径为 NULL，则返回 NULL (The converted path string, or NULL if the input path is NULL)
 */
char * rcutils_to_native_path(const char * path, rcutils_allocator_t allocator)
{
  // 如果路径为 NULL，直接返回 NULL (If the path is NULL, return NULL directly)
  if (NULL == path) {
    return NULL;
  }

  // 替换路径中的斜杠，并返回新的路径字符串 (Replace slashes in the path and return the new path string)
  return rcutils_repl_str(path, "/", RCUTILS_PATH_DELIMITER, &allocator);
}

/**
 * \brief 扩展用户主目录 (~) 到完整路径 (Expand the user's home directory (~) to a full path)
 * \param[in] path 包含用户主目录符号的路径字符串 (The path string containing the user's home directory symbol)
 * \param[in] allocator 用于分配内存的 rcutils_allocator_t 结构体 (The rcutils_allocator_t structure used for memory allocation)
 * \return 扩展后的路径字符串，如果输入路径为 NULL，则返回 NULL (The expanded path string, or NULL if the input path is NULL)
 */
char * rcutils_expand_user(const char * path, rcutils_allocator_t allocator)
{
  // 如果路径为 NULL，直接返回 NULL (If the path is NULL, return NULL directly)
  if (NULL == path) {
    return NULL;
  }

  // 如果路径第一个字符不是 '~'，直接返回原路径字符串 (If the first character of the path is not '~', return the original path string directly)
  if ('~' != path[0]) {
    return rcutils_strdup(path, allocator);
  }

  // 获取用户主目录路径 (Get the user's home directory path)
  const char * homedir = rcutils_get_home_dir();
  // 如果获取失败，返回 NULL (If the acquisition fails, return NULL)
  if (NULL == homedir) {
    return NULL;
  }
  // 格式化并拼接完整路径字符串 (Format and concatenate the full path string)
  return rcutils_format_string_limit(
    allocator, strlen(homedir) + strlen(path), "%s%s", homedir, path + 1);
}

/**
 * \brief 创建指定的绝对路径目录 (Create the specified absolute path directory)
 * \param[in] abs_path 要创建的绝对路径字符串 (The absolute path string to be created)
 * \return 创建成功返回 true，否则返回 false (Return true if the creation is successful, otherwise return false)
 */
bool rcutils_mkdir(const char * abs_path)
{
  // 如果路径为 NULL，返回 false (If the path is NULL, return false)
  if (NULL == abs_path) {
    return false;
  }

  // 如果路径为空字符串，返回 false (If the path is an empty string, return false)
  if (abs_path[0] == '\0') {
    return false;
  }

  bool success = false;
#ifdef _WIN32
  // TODO(clalancette): Check to ensure that the path is absolute on Windows.
  // In theory we can use PathRelativeA to do this, but I was unable to make
  // it work.  Needs further investigation.

  // 在 Windows 系统下创建目录 (Create a directory under the Windows system)
  int ret = _mkdir(abs_path);
#else
  // 如果路径不是以 '/' 开头，返回 false (If the path does not start with '/', return false)
  if (abs_path[0] != '/') {
    return false;
  }

  // 在非 Windows 系统下创建目录 (Create a directory under non-Windows systems)
  int ret = mkdir(abs_path, 0775);
#endif
  // 如果创建成功或者目录已存在且确实是一个目录，则将 success 设为 true (If the creation is successful or the directory already exists and is indeed a directory, set success to true)
  if (ret == 0 || (errno == EEXIST && rcutils_is_directory(abs_path))) {
    success = true;
  }

  // 返回结果 (Return the result)
  return success;
}

/**
 * @brief 计算目录大小 (Calculate the size of a directory)
 *
 * @param[in] directory_path 目录路径 (Directory path)
 * @param[out] size 计算出的目录大小 (Calculated directory size)
 * @param[in] allocator 分配器 (Allocator)
 * @return rcutils_ret_t 返回状态 (Return status)
 */
rcutils_ret_t rcutils_calculate_directory_size(
  const char * directory_path, uint64_t * size, rcutils_allocator_t allocator)
{
  // 调用递归函数计算目录大小 (Call the recursive function to calculate the directory size)
  return rcutils_calculate_directory_size_with_recursion(directory_path, 1, size, allocator);
}

// 定义目录列表结构体 (Define the directory list structure)
typedef struct dir_list_t
{
  char * path;              // 路径 (Path)
  uint32_t depth;           // 与基路径相比的深度 (Depth compared with base path)
  struct dir_list_t * next; // 指向下一个目录的指针 (Pointer to the next directory)
} dir_list_t;

/**
 * @brief 释放目录列表内存 (Free the memory of the directory list)
 *
 * @param[in] dir_list 目录列表 (Directory list)
 * @param[in] allocator 分配器 (Allocator)
 */
static void free_dir_list(dir_list_t * dir_list, rcutils_allocator_t allocator)
{
  dir_list_t * next_dir;
  do {
    next_dir = dir_list->next;
    allocator.deallocate(dir_list->path, allocator.state);
    allocator.deallocate(dir_list, allocator.state);
    dir_list = next_dir;
  } while (dir_list);
}

/**
 * @brief 从列表中移除第一个目录 (Remove the first directory from the list)
 *
 * @param[in,out] dir_list 目录列表指针的引用 (Reference to the pointer of the directory list)
 * @param[in] allocator 分配器 (Allocator)
 */
static void remove_first_dir_from_list(dir_list_t ** dir_list, rcutils_allocator_t allocator)
{
  dir_list_t * next_dir = (*dir_list)->next;
  allocator.deallocate((*dir_list)->path, allocator.state);
  allocator.deallocate(*dir_list, allocator.state);
  *dir_list = next_dir;
}

/**
 * @brief 检查并计算大小 (Check and calculate size)
 *
 * @param[in] filename 文件名 (Filename)
 * @param[out] dir_size 目录大小 (Directory size)
 * @param[in] max_depth 最大深度 (Max depth)
 * @param[in] dir_list 目录列表 (Directory list)
 * @param[in] allocator 分配器 (Allocator)
 * @return rcutils_ret_t 返回状态 (Return status)
 */
static rcutils_ret_t check_and_calculate_size(
  const char * filename,
  uint64_t * dir_size,
  const size_t max_depth,
  dir_list_t * dir_list,
  rcutils_allocator_t allocator)
{
  // 跳过本地文件夹句柄（`.`）和父文件夹（`..`）(Skip over local folder handle (`.`) and parent folder (`..`))
  if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
    return RCUTILS_RET_OK;
  }

  // 拼接文件路径 (Join the file path)
  char * file_path = rcutils_join_path(dir_list->path, filename, allocator);
  if (NULL == file_path) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("rcutils_join_path return NULL !\n");
    return RCUTILS_RET_BAD_ALLOC;
  }

  // 判断路径是否为目录 (Determine if the path is a directory)
  if (rcutils_is_directory(file_path)) {
    if ((max_depth == 0) || ((dir_list->depth + 1) <= max_depth)) {
      // 将新目录添加到 dir_list (Add new directory to dir_list)
      dir_list_t * found_new_dir = allocator.allocate(sizeof(dir_list_t), allocator.state);
      if (NULL == found_new_dir) {
        RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
          "Failed to allocate memory for path %s !\n", file_path);
        allocator.deallocate(file_path, allocator.state);
        return RCUTILS_RET_BAD_ALLOC;
      }
      found_new_dir->path = file_path;
      found_new_dir->depth = dir_list->depth + 1;
      found_new_dir->next = dir_list->next;
      dir_list->next = found_new_dir;
      return RCUTILS_RET_OK;
    }
  } else {
    // 计算文件大小并累加到 dir_size (Calculate the file size and add it to dir_size)
    *dir_size += rcutils_get_file_size(file_path);
  }

  // 释放分配的文件路径内存 (Free the allocated file path memory)
  allocator.deallocate(file_path, allocator.state);

  return RCUTILS_RET_OK;
}

/**
 * @brief 计算具有递归的目录大小 (Calculate the size of a directory with recursion)
 *
 * @param[in] directory_path 指向要计算大小的目录的路径 (Path to the directory whose size needs to be calculated)
 * @param[in] max_depth 递归搜索的最大深度 (Maximum depth of recursion for searching)
 * @param[out] size 目录大小的输出指针 (Output pointer for the size of the directory)
 * @param[in] allocator 分配器用于分配内存 (Allocator for memory allocation)
 * @return rcutils_ret_t 返回操作结果 (Return the result of the operation)
 */
rcutils_ret_t rcutils_calculate_directory_size_with_recursion(
  const char * directory_path,
  const size_t max_depth,
  uint64_t * size,
  rcutils_allocator_t allocator)
{
  // 定义目录列表指针 (Define directory list pointer)
  dir_list_t * dir_list = NULL;
  // 初始化返回值为成功 (Initialize return value as successful)
  rcutils_ret_t ret = RCUTILS_RET_OK;
  // 定义目录迭代器指针 (Define directory iterator pointer)
  rcutils_dir_iter_t * iter = NULL;

  // 检查输入的目录路径是否为空 (Check if the input directory path is NULL)
  if (NULL == directory_path) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("directory_path is NULL !");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 检查输出大小指针是否为空 (Check if the output size pointer is NULL)
  if (NULL == size) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("size pointer is NULL !");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 检查路径是否为目录 (Check if the path is a directory)
  if (!rcutils_is_directory(directory_path)) {
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING(
      "Path is not a directory: %s\n", directory_path);
    return RCUTILS_RET_ERROR;
  }

  // 为目录列表分配内存 (Allocate memory for the directory list)
  dir_list = allocator.zero_allocate(1, sizeof(dir_list_t), allocator.state);
  if (NULL == dir_list) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to allocate memory !\n");
    return RCUTILS_RET_BAD_ALLOC;
  }

  // 设置目录列表的初始深度 (Set the initial depth of the directory list)
  dir_list->depth = 1;

  // 复制输入的目录路径到目录列表 (Copy the input directory path to the directory list)
  dir_list->path = rcutils_strdup(directory_path, allocator);
  if (NULL == dir_list->path) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to duplicate directory path !\n");
    allocator.deallocate(dir_list, allocator.state);
    return RCUTILS_RET_BAD_ALLOC;
  }

  // 初始化输出大小为0 (Initialize output size to 0)
  *size = 0;

  // 遍历目录列表 (Traverse the directory list)
  do {
    iter = rcutils_dir_iter_start(dir_list->path, allocator);
    if (NULL == iter) {
      ret = RCUTILS_RET_ERROR;
      goto fail;
    }

    // 遍历当前目录并计算大小 (Traverse the current directory and calculate the size)
    do {
      ret = check_and_calculate_size(iter->entry_name, size, max_depth, dir_list, allocator);
      if (RCUTILS_RET_OK != ret) {
        goto fail;
      }
    } while (rcutils_dir_iter_next(iter));

    // 结束当前目录迭代 (End the current directory iteration)
    rcutils_dir_iter_end(iter);

    // 从目录列表中删除第一个目录 (Remove the first directory from the directory list)
    remove_first_dir_from_list(&dir_list, allocator);
  } while (dir_list);

  // 返回操作结果 (Return the operation result)
  return ret;

fail:
  // 结束目录迭代并释放目录列表 (End the directory iteration and free the directory list)
  rcutils_dir_iter_end(iter);
  free_dir_list(dir_list, allocator);
  return ret;
}

/**
 * @brief 开始遍历指定目录 (Start iterating over the specified directory)
 * 
 * @param[in] directory_path 要遍历的目录路径 (Path of the directory to iterate)
 * @param[in] allocator 用于分配内存的内存分配器 (Memory allocator used for memory allocation)
 * @return rcutils_dir_iter_t* 指向目录迭代器的指针，如果失败则为NULL (Pointer to the directory iterator, NULL if failed)
 */
rcutils_dir_iter_t *
rcutils_dir_iter_start(const char * directory_path, const rcutils_allocator_t allocator)
{
  // 检查参数是否为空 (Check if the argument is null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(directory_path, NULL);
  // 检查内存分配器是否有效 (Check if the allocator is valid)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(&allocator, "allocator is invalid", return NULL);

  // 使用内存分配器分配一个迭代器结构体 (Allocate an iterator structure using the allocator)
  rcutils_dir_iter_t * iter =
    (rcutils_dir_iter_t *)allocator.zero_allocate(1, sizeof(rcutils_dir_iter_t), allocator.state);
  // 分配失败时返回NULL (Return NULL when allocation fails)
  if (NULL == iter) {
    return NULL;
  }
  // 设置迭代器的内存分配器 (Set the iterator's allocator)
  iter->allocator = allocator;

  // 使用内存分配器分配一个状态结构体 (Allocate a state structure using the allocator)
  rcutils_dir_iter_state_t * state = (rcutils_dir_iter_state_t *)allocator.zero_allocate(
    1, sizeof(rcutils_dir_iter_state_t), allocator.state);
  // 分配失败时设置错误消息并跳转到失败处理 (Set error message and jump to failure handling when allocation fails)
  if (NULL == state) {
    RCUTILS_SET_ERROR_MSG("Failed to allocate memory.\n");
    goto rcutils_dir_iter_start_fail;
  }
  // 设置迭代器的状态指针 (Set the iterator's state pointer)
  iter->state = (void *)state;

#ifdef _WIN32
  // 拼接搜索路径 (Join search path)
  char * search_path = rcutils_join_path(directory_path, "*", allocator);
  // 拼接失败时跳转到失败处理 (Jump to failure handling when join fails)
  if (NULL == search_path) {
    goto rcutils_dir_iter_start_fail;
  }
  // 查找第一个文件 (Find the first file)
  state->handle = FindFirstFile(search_path, &state->data);
  // 释放搜索路径内存 (Free search path memory)
  allocator.deallocate(search_path, allocator.state);
  // 判断句柄是否有效 (Check if the handle is valid)
  if (INVALID_HANDLE_VALUE == state->handle) {
    DWORD error = GetLastError();
    // 如果错误不是文件未找到或目录不存在，则设置错误消息并跳转到失败处理 (If the error is not file not found or directory does not exist, set error message and jump to failure handling)
    if (ERROR_FILE_NOT_FOUND != error || !rcutils_is_directory(directory_path)) {
      RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Can't open directory %s. Error code: %d\n", directory_path, error);
      goto rcutils_dir_iter_start_fail;
    }
  } else {
    // 设置迭代器的条目名称 (Set the iterator's entry name)
    iter->entry_name = state->data.cFileName;
  }
#else
  // 打开目录 (Open directory)
  state->dir = opendir(directory_path);
  // 打开失败时设置错误消息并跳转到失败处理 (Set error message and jump to failure handling when opening fails)
  if (NULL == state->dir) {
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Can't open directory %s. Error code: %d\n", directory_path, errno);
    goto rcutils_dir_iter_start_fail;
  }

  // 清除错误号 (Clear error number)
  errno = 0;
  // 读取目录条目 (Read directory entry)
  struct dirent * entry = readdir(state->dir);
  // 如果读取成功，设置迭代器的条目名称 (If reading succeeds, set the iterator's entry name)
  if (NULL != entry) {
    iter->entry_name = entry->d_name;
  } else if (0 != errno) {
    // 读取失败时设置错误消息并跳转到失败处理 (Set error message and jump to failure handling when reading fails)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING(
      "Can't iterate directory %s. Error code: %d\n", directory_path, errno);
    goto rcutils_dir_iter_start_fail;
  }
#endif

  // 返回迭代器指针 (Return iterator pointer)
  return iter;

// 失败处理标签 (Failure handling label)
rcutils_dir_iter_start_fail:
  // 结束迭代并返回NULL (End iteration and return NULL)
  rcutils_dir_iter_end(iter);
  return NULL;
}

/**
 * @brief 判断给定的迭代器是否有下一个目录项，并更新迭代器的 entry_name。
 *        Determine if the given iterator has a next directory entry and update the iterator's entry_name.
 *
 * @param[in,out] iter 指向 rcutils_dir_iter_t 结构体的指针，用于存储迭代器状态。
 *                     Pointer to the rcutils_dir_iter_t struct, which stores the iterator state.
 * @return 返回 true 表示有下一个目录项，false 表示没有。
 *         Return true if there is a next directory entry, false otherwise.
 */
bool rcutils_dir_iter_next(rcutils_dir_iter_t * iter)
{
  // 检查输入参数是否为空，如果为空则返回 false
  // Check if the input argument is NULL, return false if it is
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(iter, false);

  // 获取迭代器的内部状态
  // Get the iterator's internal state
  rcutils_dir_iter_state_t * state = (rcutils_dir_iter_state_t *)iter->state;

  // 检查状态是否为空，如果为空则输出错误信息并返回 false
  // Check if the state is NULL, output an error message and return false if it is
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(state, "iter is invalid", false);

#ifdef _WIN32
  // Windows 平台代码
  // Code for Windows platform

  // 查找下一个文件，如果找到则更新 entry_name 并返回 true
  // Find the next file; if found, update entry_name and return true
  if (FindNextFile(state->handle, &state->data)) {
    iter->entry_name = state->data.cFileName;
    return true;
  }

  // 关闭文件查找句柄
  // Close the file search handle
  FindClose(state->handle);
#else
  // 其他平台代码（如 Linux）
  // Code for other platforms (e.g., Linux)

  // 读取下一个目录项
  // Read the next directory entry
  struct dirent * entry = readdir(state->dir);

  // 如果找到下一个目录项，更新 entry_name 并返回 true
  // If a next directory entry is found, update entry_name and return true
  if (NULL != entry) {
    iter->entry_name = entry->d_name;
    return true;
  }
#endif

  // 没有找到下一个目录项，将 entry_name 设置为 NULL 并返回 false
  // No next directory entry found, set entry_name to NULL and return false
  iter->entry_name = NULL;
  return false;
}

/**
 * @brief 结束给定的迭代器并释放其资源。
 *        End the given iterator and release its resources.
 *
 * @param[in,out] iter 指向 rcutils_dir_iter_t 结构体的指针，用于存储迭代器状态。
 *                     Pointer to the rcutils_dir_iter_t struct, which stores the iterator state.
 */
void rcutils_dir_iter_end(rcutils_dir_iter_t * iter)
{
  // 检查输入参数是否为空，如果为空则直接返回
  // Check if the input argument is NULL; if it is, return immediately
  if (NULL == iter) {
    return;
  }

  // 获取迭代器的内存分配器
  // Get the iterator's memory allocator
  rcutils_allocator_t allocator = iter->allocator;

  // 获取迭代器的内部状态
  // Get the iterator's internal state
  rcutils_dir_iter_state_t * state = (rcutils_dir_iter_state_t *)iter->state;

  // 检查状态是否为空
  // Check if the state is NULL
  if (NULL != state) {
#ifdef _WIN32
    // Windows 平台代码
    // Code for Windows platform

    // 关闭文件查找句柄
    // Close the file search handle
    FindClose(state->handle);
#else
    // 其他平台代码（如 Linux）
    // Code for other platforms (e.g., Linux)

    // 如果目录指针不为空，关闭目录流
    // If the directory pointer is not NULL, close the directory stream
    if (NULL != state->dir) {
      closedir(state->dir);
    }
#endif

    // 使用分配器释放状态内存
    // Use the allocator to release the state memory
    allocator.deallocate(state, allocator.state);
  }

  // 使用分配器释放迭代器内存
  // Use the allocator to release the iterator memory
  allocator.deallocate(iter, allocator.state);
}

/**
 * \brief 获取文件大小的函数 (Get the file size function)
 *
 * \param[in] file_path 文件路径 (File path)
 *
 * \return 返回文件大小，如果不是文件或错误，则返回0 (Returns the file size, if not a file or error, returns 0)
 */
size_t rcutils_get_file_size(const char * file_path)
{
  // 检查给定路径是否为文件 (Check if the given path is a file)
  if (!rcutils_is_file(file_path)) {
    // 如果不是文件，将错误信息写入标准错误流 (If it's not a file, write the error message to the standard error stream)
    RCUTILS_SAFE_FWRITE_TO_STDERR_WITH_FORMAT_STRING("Path is not a file: %s\n", file_path);
    // 返回0表示错误 (Return 0 to indicate an error)
    return 0;
  }

  // 定义一个stat结构体变量用于存储文件信息 (Define a stat structure variable for storing file information)
  struct stat stat_buffer;

  // 调用stat函数获取文件信息，并将结果存储在stat_buffer中 (Call the stat function to get the file information and store the result in stat_buffer)
  int rc = stat(file_path, &stat_buffer);

  // 如果stat函数调用成功（返回值为0），则返回文件大小；否则，返回0 (If the stat function call is successful (return value is 0), return the file size; otherwise, return 0)
  return rc == 0 ? (size_t)(stat_buffer.st_size) : 0;
}

#ifdef __cplusplus
}
#endif
