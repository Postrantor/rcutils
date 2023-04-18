// Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef RCUTILS__VISIBILITY_CONTROL_MACROS_H_
#define RCUTILS__VISIBILITY_CONTROL_MACROS_H_

// 定义宏以表示符号是本地的、导入的还是导出的
// Define macros to express whether a symbol is local, imported, or exported
//
// 这些宏与 GCC、clang 和 Microsoft Visual C++ 兼容。它们可用于强制公开库的哪些符号。
// Those macros are compatible with GCC, clang, and Microsoft Visual C++. They can be used to enforce which symbols of a library are publicly accessible.
//
// RCUTILS_IMPORT、RCUTILS_EXPORT 和 RCUTILS_LOCAL 分别声明一个导入的、导出的或本地符号。
// RCUTILS_IMPORT, RCUTILS_EXPORT, and RCUTILS_LOCAL are respectively declaring an imported, exported, or local symbol.
// RCUTILS_LOCAL 可以直接使用。然而，RCUTILS_IMPORT 和 RCUTILS_EXPORT 不能直接使用。每个项目都需要提供一个名为 `visibility_macros.h` 的附加头文件，其中包含：
// RCUTILS_LOCAL can be used directly. However, RCUTILS_IMPORT, and RCUTILS_EXPORT may not be used directly. Every project needs to provide an additional header called `visibility_macros.h` containing:
//
// #ifdef <project>_BUILDING_DLL
// # define <project>_PUBLIC RCUTILS_EXPORT
// #else
// # define <project>_PUBLIC RCUTILS_IMPORT
// #endif // !<project>_BUILDING_DLL
// #define <project>_LOCAL RCUTILS_LOCAL
//
// ...其中 "<project>" 已被项目名称替换，例如 "MY_PROJECT"。
// ...where "<project>" has been replaced by the project name, such as "MY_PROJECT".
// 您的项目 CMakeLists.txt 还应包含以下声明：
// Your project CMakeLists.txt should also contain the following statement:
//
// target_compile_definitions(<your library> PRIVATE "<project>_BUILDING_DLL")
//
// 公共（导出）类应标记为 <project>_PUBLIC，而非公共类应标记为 <project>_LOCAL。
// A public (exported) class should then be tagged as <project>_PUBLIC, whereas a non-public class should be tagged with <project>_LOCAL.
//
// 请参阅 GCC 文档：https://gcc.gnu.org/wiki/Visibility
// See GCC documentation: https://gcc.gnu.org/wiki/Visibility

#if defined(_MSC_VER) || defined(__CYGWIN__)
// 使用 Microsoft Visual C++ 编译时使用 Windows 语法。
// Use the Windows syntax when compiling with Microsoft Visual C++.

// GCC 在 Windows 上也支持这种语法。在使用 cygwin 编译时，
// 更倾向于使用 dllimport/dllexport，因为这些标志的语义更接近
// msvc++ 的行为。参见 GCC 文档：
// https://gcc.gnu.org/onlinedocs/gcc/Microsoft-Windows-Function-Attributes.html
// GCC on Windows also support this syntax. When compiling with cygwin,
// prefer using dllimport/dllexport as the semantic of those flags is closer
// to msvc++ behavior. See GCC documentation:
// https://gcc.gnu.org/onlinedocs/gcc/Microsoft-Windows-Function-Attributes.html
#define RCUTILS_IMPORT __declspec(dllimport)
#define RCUTILS_EXPORT __declspec(dllexport)
#define RCUTILS_LOCAL
#else // defined(_MSC_VER) || defined(__CYGWIN__)
// 在 Linux 上，使用 GCC 语法。其他编译器（如 clang、icpc 和 xlc++）也能理解这种语法。
// On Linux, use the GCC syntax. This syntax is understood by other compilers
// such as clang, icpc, and xlc++.
#define RCUTILS_IMPORT __attribute__((visibility("default")))
#define RCUTILS_EXPORT __attribute__((visibility("default")))
#define RCUTILS_LOCAL __attribute__((visibility("hidden")))
#endif // !defined(_MSC_VER) && !defined(__CYGWIN__)

#endif // RCUTILS__VISIBILITY_CONTROL_MACROS_H_
