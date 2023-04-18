// Copyright 2015-2016 Laird Shaw
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

// This function is based on the repl_str() from (on 2017-04-25):
//
//   http://creativeandcritical.net/str-replace-c
//
// It is released under the Public Domain, and has been placed additionally
// under the Apache 2.0 license by me (William Woodall).
//
// It has been modified to take a custom allocator and to fit some of our
// style standards.

/// \file

#ifndef RCUTILS__REPL_STR_H_
#define RCUTILS__REPL_STR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

/// Replace all the occurrences of one string for another in the given string.
/**
 * Documentation copied from the source with minor tweaks:
 *
 * ----
 *
 * **Description:**
 *
 * Replaces in the string `str` all the occurrences of the source string `from`
 * with the destination string `to`.
 * The lengths of the strings `from` and `to` may differ.
 * The string `to` may be of any length, but the string `from` must be of
 * non-zero length - the penalty for providing an empty string for the `from`
 * parameter is an infinite loop.
 * In addition, none of the three parameters may be NULL.
 *
 * **Returns:**
 *
 * The post-replacement string, or NULL if memory for the new string could not
 * be allocated.
 * Does not modify the original string.
 * The memory for the returned post-replacement string may be deallocated with
 * given allocator's deallocate function when it is no longer required.
 *
 * **Performance:**
 *
 * In case you are curious enough to want to compare this implementation with
 * alternative implementations, you are welcome to compile and run the code in
 * the benchmarking file,
 * [replacebench.c](http://creativeandcritical.net/downloads/replacebench.c).
 * In that file, the above function is included as `replace_cache`, and the
 * functions originally published on this page as `replace_str2` and
 * `replace_str` are included as `replace_opt2` and `replace_opt`.
 * Code/functions are included from the comp.lang.c thread,
 * [how to replace a substring in a string using C?]
 * (https://groups.google.com/forum/#!msg/comp.lang.c/sgydS2lDgxc/v2MRxRrAQncJ),
 * from answers to the stackoverflow question,
 * [What is the function to replace string in C?]
 * (http://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c),
 * and also from private correspondence.
 * See the comments top of file for instructions on compiling and running it.
 *
 * In most scenarios, the fastest replacement function, by around 10-20%,
 * is Albert Chan's `replace_smart`, followed by the above function:
 * `repl_str` aka `replace_cache`.
 * There are some scenarios, though, where `repl_str` is faster than
 * `replace_smart`, sometimes by up to 200%.
 * These scenarios involve long strings with few matches.
 * Why, if Albert's function is generally slightly faster than the above
 * `repl_str` function, is it not the focus of this page?
 * Because it generally uses much more memory than `repl_str`.
 *
 * The third fastest implementation is typically `replace_str2` aka
 * `replace_opt2`.
 * For longer strings in the case in which the lengths of the "from" and "to"
 * strings differ, `repl_str` aka `replace_cache` beats it by margins of up to
 * about 80%.
 * For smaller strings, and in the case where the lengths of the "from" and
 * "to" strings are identical, `replace_str2` aka `replace_opt2` is faster,
 * by a maximum margin of about 35%, sometimes in those scenarios beating
 * `replace_smart` too.
 * Some of the other functions are also faster for smaller strings.
 * The even-match point between `replace_str2` and `repl_str`
 * (assuming "from" and "to" string lengths differ) depends on how far into
 * the string the final match occurs, how many matches there are, and the
 * comparative lengths of the old and "to" strings, but roughly it occurs for
 * strings of 700-800 bytes in length.
 *
 * This analysis is based on compiling with [GCC](https://gcc.gnu.org/) and
 * testing on a 64-bit Intel platform running Linux, however brief testing with
 * [Microsoft Visual C++ 2010 Express]
 * (https://www.visualstudio.com/en-US/products/visual-studio-express-vs)
 * (scroll down to "Additional information" at that link) on Windows 7 seemed
 * to produce similar results.
 *
 * ----
 *
 * Here continues additional documentation added by OSRF.
 *
 * The allocator must not be NULL.
 *
 * \param[in] str string to have substrings found and replaced within
 * \param[in] from string to match for replacement
 * \param[in] to string to replace matched strings with
 * \param[in] allocator structure defining functions to be used for allocation
 * \return duplicated `str` with all matches of `from` replaced with `to`.
 */
/// 替换给定字符串中一个字符串的所有出现。(Replace all the occurrences of one string for another in the given string.)
/**
 * 从源码中复制的文档，并进行了少量修改：
 *
 * ----
 *
 * **描述：**(Description:)
 *
 * 在字符串 `str` 中，将源字符串 `from` 的所有出现替换为目标字符串 `to`。
 * 字符串 `from` 和 `to` 的长度可能不同。
 * 字符串 `to` 可以是任意长度，但字符串 `from` 必须是非零长度——为 `from` 参数提供空字符串的惩罚是无限循环。
 * 此外，这三个参数都不能为 NULL。
 *
 * **返回：**(Returns:)
 *
 * 替换后的字符串，如果无法为新字符串分配内存，则返回 NULL。
 * 不修改原始字符串。
 * 当不再需要返回的替换后字符串的内存时，可以使用给定分配器的 deallocate 函数释放它。
 *
 * **性能：**(Performance:)
 *
 * 如果你足够好奇并想要将此实现与其他实现进行比较，欢迎编译并运行基准测试文件中的代码，
 * [replacebench.c](http://creativeandcritical.net/downloads/replacebench.c)。
 * 在该文件中，上述函数被包含为 `replace_cache`，而最初在此页面上发布的函数 `replace_str2` 和
 * `replace_str` 分别被包含为 `replace_opt2` 和 `replace_opt`。
 * 从 comp.lang.c 线程中包含了代码/函数，
 * [how to replace a substring in a string using C?]
 * (https://groups.google.com/forum/#!msg/comp.lang.c/sgydS2lDgxc/v2MRxRrAQncJ)，
 * 从 stackoverflow 问题的答案中，
 * [What is the function to replace string in C?]
 * (http://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c)，
 * 以及从私人通信中。
 * 查看文件顶部的注释以获取有关编译和运行它的说明。
 *
 * 在大多数情况下，最快的替换函数大约快 10-20%，是 Albert Chan 的 `replace_smart`，其次是上述函数：
 * `repl_str` 又名 `replace_cache`。
 * 但是，在某些情况下，`repl_str` 比 `replace_smart` 更快，有时甚至快 200%。
 * 这些情况涉及到长字符串与少量匹配。
 * 为什么，如果 Albert 的函数通常比上面的 `repl_str` 函数稍快一些，那么它不是本页关注的重点呢？
 * 因为它通常比 `repl_str` 使用更多的内存。
 *
 * 第三快的实现通常是 `replace_str2` 又名 `replace_opt2`。
 * 对于较长的字符串，在 "from" 和 "to" 字符串长度不同的情况下，`repl_str` 又名 `replace_cache` 以最高约 80% 的优势击败它。
 * 对于较小的字符串，在 "from" 和 "to" 字符串长度相同的情况下，`replace_str2` 又名 `replace_opt2` 更快，
 * 最大优势约为 35%，有时在这些情况下也击败 `replace_smart`。
 * 其他一些函数对于较小的字符串也更快。
 * `replace_str2` 和 `repl_str` 之间的平衡点（假设 "from" 和 "to" 字符串长度不同）取决于最后一个匹配在字符串中出现的位置、匹配的数量以及旧字符串和 "to" 字符串的相对长度，但大致发生在 700-800 字节长度的字符串上。
 *
 * 这种分析是基于使用 [GCC](https://gcc.gnu.org/) 编译并在运行 Linux 的 64 位 Intel 平台上进行测试，但是在 Windows 7 上使用
 * [Microsoft Visual C++ 2010 Express]
 * (https://www.visualstudio.com/en-US/products/visual-studio-express-vs)
 * （在该链接中向下滚动到 "Additional information"）进行简短测试似乎产生了类似的结果。
 *
 * ----
 *
 * 下面是 OSRF 添加的其他文档。
 *
 * 分配器不能为 NULL。
 *
 * \param[in] str 要在其中查找并替换子字符串的字符串。(string to have substrings found and replaced within)
 * \param[in] from 要匹配以进行替换的字符串。(string to match for replacement)
 * \param[in] to 用于替换匹配字符串的字符串。(string to replace matched strings with)
 * \param[in] allocator 定义要用于分配的函数的结构。(structure defining functions to be used for allocation)
 * \return 所有 `from` 匹配项替换为 `to` 的复制 `str`。(duplicated `str` with all matches of `from` replaced with `to`.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
char * rcutils_repl_str(
  const char * str, const char * from, const char * to, const rcutils_allocator_t * allocator);

// Implementation copied from above mentioned source continues in repl_str.c.

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__REPL_STR_H_
