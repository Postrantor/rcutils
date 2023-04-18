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

#ifndef RCUTILS__FILESYSTEM_H_
#define RCUTILS__FILESYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// 返回当前工作目录。 (Return current working directory.)
/**
 * \param[in] buffer 用于存储当前目录路径的已分配字符串 (Allocated string to store current directory path to)
 * \param[in] max_length 可存储在缓冲区中的最大长度 (maximum length to be stored in buffer)
 * \return `true` 如果成功 (if success), or
 * \return `false` 如果缓冲区为空 (if buffer is NULL), or
 * \return `false` 如果失败 (on failure).
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool rcutils_get_cwd(char * buffer, size_t max_length);

/// 检查提供的路径是否指向一个目录。 (Check if the provided path points to a directory.)
/**
 * \param[in] abs_path 要检查的绝对路径 (Absolute path to check.)
 * \return `true` 如果提供的路径是一个目录 (if provided path is a directory), or
 * \return `false` 如果 abs_path 为空 (if abs_path is NULL), or
 * \return `false` 如果失败 (on failure).
 */
RCUTILS_PUBLIC
bool rcutils_is_directory(const char * abs_path);

/// 检查提供的路径是否指向一个文件。 (Check if the provided path points to a file.)
/**
 * \param[in] abs_path 要检查的绝对路径 (Absolute path to check.)
 * \return `true` 如果提供的路径是一个文件 (if provided path is a file), or
 * \return `false` 如果 abs_path 为空 (if abs_path is NULL), or
 * \return `false` 如果失败 (on failure).
 */
RCUTILS_PUBLIC
bool rcutils_is_file(const char * abs_path);

/// 检查提供的路径是否指向一个存在的文件/文件夹。 (Check if the provided path points to an existing file/folder.)
/**
 * \param[in] abs_path 要检查的绝对路径 (Absolute path to check.)
 * \return `true` 如果路径存在 (if the path exists), or
 * \return `false` 如果 abs_path 为空 (if abs_path is NULL), or
 * \return `false` 如果失败 (on failure).
 */
RCUTILS_PUBLIC
bool rcutils_exists(const char * abs_path);

/// 检查提供的路径是否指向当前用户可读的文件/文件夹。
/// Check if the provided path points to a file/folder readable by current user.
/**
 * \param[in] abs_path 要检查的绝对路径。
 * \param[in] abs_path Absolute path to check.
 * \return 如果文件可读，则返回 `true`，或
 * \return `true` if the file is readable, or
 * \return 如果 abs_path 为 NULL，则返回 `false`，或
 * \return `false` if abs_path is NULL, or
 * \return 失败时返回 `false`。
 * \return `false` on failure.
 */
RCUTILS_PUBLIC
bool rcutils_is_readable(const char * abs_path);

/// 检查提供的路径是否指向当前用户可写的文件/文件夹。
/// Check if the provided path points to a file/folder writable by current user.
/**
 * \param[in] abs_path 要检查的绝对路径。
 * \param[in] abs_path Absolute path to check.
 * \return 如果文件可写，则返回 `true`，或
 * \return `true` if the file is writable, or
 * \return 如果 abs_path 为 NULL，则返回 `false`，或
 * \return `false` if abs_path is NULL, or
 * \return 失败时返回 `false`。
 * \return `false` on failure.
 */
RCUTILS_PUBLIC
bool rcutils_is_writable(const char * abs_path);

/// 检查提供的路径是否指向当前用户既可读又可写的文件/文件夹。
/// Check if the provided path points to a file/folder both readable and writable by current user.
/**
 * \param[in] abs_path 要检查的绝对路径。
 * \param[in] abs_path Absolute path to check.
 * \return 如果文件可读且可写，则返回 `true`，或
 * \return `true` if the file is readable and writable, or
 * \return 如果 abs_path 为 NULL，则返回 `false`
 * \return `false` if abs_path is NULL
 * \return 失败时返回 `false`。
 * \return `false` on failure.
 */
RCUTILS_PUBLIC
bool rcutils_is_readable_and_writable(const char * abs_path);

/// 返回新分配的字符串，其中参数由平台正确的分隔符分隔。
/// Return newly allocated string with arguments separated by correct delimiter for the platform.
/**
 * 此函数分配内存并将其返回给调用者。
 * This function allocates memory and returns it to the caller.
 * 当调用者使用完内存后，需要通过在此处传递相同的分配器来调用 `deallocate` 以释放内存。
 * It is up to the caller to release the memory once it is done with it by
 * calling `deallocate` on the same allocator passed here.
 *
 * \param[in] left_hand_path
 * \param[in] right_hand_path
 * \param[in] allocator
 * \return 成功时返回连接的路径
 * \return concatenated path on success
 * \return 对于无效参数返回 `NULL`
 * \return `NULL` on invalid arguments
 * \return 失败时返回 `NULL`
 * \return `NULL` on failure
 */
RCUTILS_PUBLIC
char * rcutils_join_path(
  const char * left_hand_path, const char * right_hand_path, rcutils_allocator_t allocator);

/// 返回一个新分配的字符串，其中所有参数的 "/" 被替换为特定于平台的分隔符。
/// Return newly allocated string with all argument's "/" replaced by platform specific separator.
/**
 * 此函数分配内存并将其返回给调用者。
 * This function allocates memory and returns it to the caller.
 * 调用者在使用完内存后需要通过在此处传递的相同分配器调用 `deallocate` 来释放内存。
 * It is up to the caller to release the memory once it is done with it by
 * calling `deallocate` on the same allocator passed here.
 *
 * \param[in] path 路径
 * \param[in] path
 * \param[in] allocator 分配器
 * \param[in] allocator
 * \return 成功时返回使用特定于平台的分隔符的路径
 * \return path using platform specific delimiters on success
 * \return 对于无效参数返回 `NULL`
 * \return `NULL` on invalid arguments
 * \return 失败时返回 `NULL`
 * \return `NULL` on failure
 */
RCUTILS_PUBLIC
char * rcutils_to_native_path(const char * path, rcutils_allocator_t allocator);

/// 在路径中展开用户目录。
/// Expand user directory in path.
/**
 * 此函数将初始 '~' 扩展为当前用户的主目录。
 * This function expands an initial '~' to the current user's home directory.
 * 使用 `rcutils_get_home_dir()` 获取主目录。
 * The home directory is fetched using `rcutils_get_home_dir()`.
 * 此函数在成功时返回一个新分配的字符串。
 * This function returns a newly allocated string on success.
 * 调用者在使用完内存后需要通过在此处传递的相同分配器调用 `deallocate` 来释放内存。
 * It is up to the caller to release the memory once it is done with it by
 * calling `deallocate` on the same allocator passed here.
 *
 * \param[in] path 表示路径的空终止 C 字符串。
 * \param[in] path A null-terminated C string representing a path.
 * \param[in] allocator 分配器
 * \param[in] allocator
 * \return 成功时返回展开的主目录路径，或
 * \return path with expanded home directory on success, or
 * \return 对于无效参数返回 `NULL`，或
 * \return `NULL` on invalid arguments, or
 * \return 失败时返回 `NULL`。
 * \return `NULL` on failure.
 */
RCUTILS_PUBLIC
char * rcutils_expand_user(const char * path, rcutils_allocator_t allocator);

/// 创建指定的目录。
/// Create the specified directory.
/**
 * 此函数创建一个绝对指定的目录。
 * This function creates an absolutely-specified directory.
 * 如果中间的任何目录不存在，此函数将返回 False。
 * If any of the intermediate directories do not exist, this function will
 * return False.
 * 如果 abs_path 已经存在，并且是一个目录，那么此函数将返回 true。
 * If the abs_path already exists, and is a directory, this function will
 * return true.
 *
 * 由于 mkdir 竞争，如 openat(2) 文档所述，此函数不是线程安全的。
 * This function is not thread-safe due to mkdir races as described in the
 * openat(2) documentation.
 *
 * \param[in] abs_path 绝对路径
 * \param[in] abs_path
 * \return 如果创建目录成功返回 `true`，或
 * \return `true` if making the directory was successful, or
 * \return 如果路径为 NULL 返回 `false`，或
 * \return `false` if path is NULL, or
 * \return 如果路径为空返回 `false`，或
 * \return `false` if path is empty, or
 * \return 如果路径不是绝对的返回 `false`，或
 * \return `false` if path is not absolute, or
 * \return 如果任何中间目录不存在返回 `false`。
 * \return `false` if any intermediate directories don't exist.
 */
RCUTILS_PUBLIC
bool rcutils_mkdir(const char * abs_path);

/// 计算指定目录的大小。
/// Calculate the size of the specified directory.
/**
 * 通过汇总所有文件的文件大小来计算目录的大小。
 * Calculates the size of a directory by summarizing the file size of all files.
 * \note 此操作不是递归的。
 * \note This operation is not recursive.
 * \param[in] directory_path 要计算大小的目录路径。
 * \param[in] directory_path The directory path to calculate the size of.
 * \param[out] size 成功时目录的大小（以字节为单位）。
 * \param[out] size The size of the directory in bytes on success.
 * \param[in] allocator 用于内部文件路径组合的分配器。
 * \param[in] allocator Allocator being used for internal file path composition.
 * \return 如果成功返回 #RCUTILS_RET_OK，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return 对于无效参数返回 #RCUTILS_RET_INVALID_ARGUMENT，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return 如果内存分配失败返回 #RCUTILS_RET_BAD_ALLOC
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation fails
 * \return 如果发生其他错误返回 #RCUTILS_RET_ERROR
 * \return #RCUTILS_RET_ERROR if other error occurs
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_calculate_directory_size(
  const char * directory_path, uint64_t * size, rcutils_allocator_t allocator);

/// 递归计算指定目录的大小。
/// Calculate the size of the specified directory with recursion.
/**
 * 计算目录及其子目录的大小，通过汇总所有文件的文件大小。
 * Calculates the size of a directory and subdirectory by summarizing the file size of all files.
 * 如果需要，可以指定最大目录深度进行递归。
 * If necessary, you can specify the maximum directory depth to recurse into.
 * 深度定义如下。
 * Depth definition as below.
 * \code
 * directory_path  <= depth 1
 *    |- subdirectory <= depth 2
 *            |- subdirectory <= depth 3
 *                    ...
 * \endcode
 *
 * \note 此 API 不遵循指向文件或目录的符号链接。
 * \note This API does not follow symlinks to files or directories.
 * \param[in] directory_path 要计算大小的目录路径。
 * \param[in] directory_path The directory path to calculate the size of.
 * \param[in] max_depth 子目录的最大深度。0 表示无限制。
 * \param[in] max_depth The maximum depth of subdirectory. 0 means no limitation.
 * \param[out] size 成功时目录的字节大小。
 * \param[out] size The size of the directory in bytes on success.
 * \param[in] allocator 用于内部文件路径组合的分配器。
 * \param[in] allocator Allocator being used for internal file path composition.
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation fails
 * \return #RCUTILS_RET_ERROR 如果发生其他错误
 * \return #RCUTILS_RET_ERROR if other error occurs
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_calculate_directory_size_with_recursion(
  const char * directory_path,
  const size_t max_depth,
  uint64_t * size,
  rcutils_allocator_t allocator);

/// 计算指定文件的大小。
/// Calculate the size of the specifed file.
/**
 * \param[in] file_path 要获取其大小的文件路径。
 * \param[in] file_path The path of the file to obtain its size of.
 * \return 文件的字节大小。
 * \return The size of the file in bytes.
 */
RCUTILS_PUBLIC
size_t rcutils_get_file_size(const char * file_path);

/// 用于枚举目录内容的迭代器
/// An iterator used for enumerating directory contents
typedef struct rcutils_dir_iter_s
{
  /// 枚举的文件或目录的名称
  /// The name of the enumerated file or directory
  const char * entry_name;
  /// 迭代函数内部使用的分配器
  /// The allocator used internally by iteration functions
  rcutils_allocator_t allocator;
  /// 平台特定的迭代状态
  /// The platform-specific iteration state
  void * state;
} rcutils_dir_iter_t;

/// 开始遍历指定目录的内容。 (Begin iterating over the contents of the specified directory.)
/**
 * 此函数用于列出包含在指定目录中的文件和目录。 (This function is used to list the files and directories that are contained in a specified directory.)
 * 完成迭代后，必须使用::rcutils_dir_iter_end释放由它返回的结构。 (The structure returned by it must be deallocated using ::rcutils_dir_iter_end when the iteration is completed.)
 * 枚举条目的名称存储在返回对象的`entry_name`成员中， (The name of the enumerated entry is stored in the `entry_name` member of the returned object,)
 * 并且在此函数完成时，已经填充了第一个条目。 (and the first entry is already populated upon completion of this function.)
 * 要使用::rcutils_dir_iter_next函数填充下一个条目的名称，请执行以下操作。 (To populate the entry with the name of the next entry, use the ::rcutils_dir_iter_next function.)
 * 请注意，"." 和 ".." 条目通常在枚举的条目中。 (Note that the "." and ".." entries are typically among the entries enumerated.)
 * \param[in] directory_path 要遍历其内容的目录路径。 (The directory path to iterate over the contents of.)
 * \param[in] allocator 分配器，用于创建返回的结构。 (Allocator used to create the returned structure.)
 * \return 用于继续迭代目录内容的迭代器对象 (An iterator object used to continue iterating directory contents)
 * \return 如果发生错误，则返回NULL (NULL if an error occurred)
 */
RCUTILS_PUBLIC
rcutils_dir_iter_t *
rcutils_dir_iter_start(const char * directory_path, const rcutils_allocator_t allocator);

/// 继续遍历目录的内容。 (Continue iterating over the contents of a directory.)
/**
 * \param[in] iter 由::rcutils_dir_iter_start创建的迭代器。 (An iterator created by ::rcutils_dir_iter_start.)
 * \return 如果找到另一个条目，则为`true`，或者 (true if another entry was found, or)
 * \return 如果目录中没有更多条目，则为`false`。 (false if there are no more entries in the directory.)
 */
RCUTILS_PUBLIC
bool rcutils_dir_iter_next(rcutils_dir_iter_t * iter);

/// 完成对目录内容的迭代。 (Finish iterating over the contents of a directory.)
/**
 * \param[in] iter 由::rcutils_dir_iter_start创建的迭代器。 (An iterator created by ::rcutils_dir_iter_start.)
 */
RCUTILS_PUBLIC
void rcutils_dir_iter_end(rcutils_dir_iter_t * iter);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__FILESYSTEM_H_
