# Copyright 2020 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

# 下面这段代码是在 ros2 项目中 rmw 相关的代码，请以此为基础，用支持 doxygen
# 的形式为如下的代码段添加参数列表的说明，并以双语（中文、英文）的形式对代码块中的每一行都尽可能的添加详细的注释（双语注释分行添加；注释需要尽可能详细，因为我只是个初学者需要了解更多）。请注意，并不需要对代码进行完善扩展。
#
# The following code is related to the rmw part of the ros2 project. Based on
# this, add a description of the parameter list in a doxygen-supported form for
# the following code segment, and add detailed comments (bilingual comments are
# added separately; comments need to be as detailed as possible because I am
# just a beginner who needs to know more) for each line in the code block in
# both Chinese and English. Please note that there is no need to improve and
# expand the code.

# 检查 C 编译器是否使用 glibc。 Checks if the C compiler uses glibc.
#
# @private
#

# 定义一个名为 check_c_compiler_uses_glibc 的函数 Define a function named
# check_c_compiler_uses_glibc
function(check_c_compiler_uses_glibc result_variable)
  # 引入 CheckCSourceCompiles 模块，以检查 C 代码是否可以编译 Include the CheckCSourceCompiles
  # module to check if C code can be compiled
  include(CheckCSourceCompiles)

  # 设置一个名为 GLIBC_TEST_CODE 的变量，其内容为测试代码 Set a variable named GLIBC_TEST_CODE
  # with test code content
  set(GLIBC_TEST_CODE
      [====[
    #include <execinfo.h>

    int main() {
      // 定义一个名为 buffer 的指针数组，大小为 1
      // Define a pointer array named buffer with a size of 1
      void * buffer[1];

      // 计算 buffer 的大小（以指针为单位）
      // Calculate the size of buffer (in pointers)
      int size = sizeof(buffer) / sizeof(void *);

      // 调用 backtrace 函数，检查 glibc 是否可用
      // Call the backtrace function to check if glibc is available
      backtrace(&buffer, size);

      // 返回 0，表示程序正常退出
      // Return 0, indicating that the program exits normally
      return 0;
    }
  ]====])

  # 使用 check_c_source_compiles 函数检查 GLIBC_TEST_CODE 是否可以编译，并将结果存储在
  # result_variable 中 Use the check_c_source_compiles function to check if
  # GLIBC_TEST_CODE can be compiled and store the result in result_variable
  check_c_source_compiles("${GLIBC_TEST_CODE}" ${result_variable})

  # 将 result_variable 的值传递给父级作用域 Pass the value of result_variable to the parent
  # scope
  set(${result_variable}
      ${result_variable}
      PARENT_SCOPE)

  # 结束函数定义 End the function definition
endfunction()
