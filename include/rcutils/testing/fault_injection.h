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

#ifndef RCUTILS__TESTING__FAULT_INJECTION_H_
#define RCUTILS__TESTING__FAULT_INJECTION_H_
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rcutils/macros.h"
#include "rcutils/visibility_control.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RCUTILS_FAULT_INJECTION_NEVER_FAIL -1

#define RCUTILS_FAULT_INJECTION_FAIL_NOW 0

RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
bool rcutils_fault_injection_is_test_complete(void);

/**
 * \brief 原子地设置故障注入计数器。 (Atomically set the fault injection counter.)
 *
 * 这通常不是直接与故障注入逻辑交互的首选方法，而是使用 `RCUTILS_FAULT_INJECTION_TEST`。 
 * (This is typically not the preferred method of interacting directly with the fault injection
 * logic, instead use `RCUTILS_FAULT_INJECTION_TEST` instead.)
 *
 * 该函数还可用于暂停 `RCUTILS_FAULT_INJECTION_TEST` 内部的代码，如下所示：
 * (This function may also be used for pausing code inside of a `RCUTILS_FAULT_INJECTION_TEST` with
 * something like the following:)
 *
 * RCUTILS_FAULT_INJECTION_TEST({
 *     ...  // 使用故障注入运行的代码 (code to run with fault injection)
 *     int64_t count = rcutils_fault_injection_get_count();
 *     rcutils_fault_injection_set_count(RCUTILS_FAULT_INJECTION_NEVER_FAIL);
 *     ...  // 不使用故障注入运行的代码 (code to run without fault injection)
 *     rcutils_fault_injection_set_count(count);
 *     ...  // 使用故障注入运行的代码 (code to run with fault injection)
 * });
 *
 * \param count 要将故障注入计数器设置为的计数。如果 count 为负，则禁用故障注入错误。
 * 计数器在全局范围内初始化为 RCUTILS_FAULT_INJECTION_NEVER_FAIL。
 * (The count to set the fault injection counter to. If count is negative, then fault
 * injection errors will be disabled. The counter is globally initialized to
 * RCUTILS_FAULT_INJECTION_NEVER_FAIL.)
 */
RCUTILS_PUBLIC
void rcutils_fault_injection_set_count(int_least64_t count);

/**
 * \brief 原子地获取故障注入计数器的值 (Atomically get the fault injection counter value)
 *
 * 这个函数通常不会被直接使用，而是间接地在 `RCUTILS_FAULT_INJECTION_TEST` 中使用
 * (This function is typically not used directly but instead indirectly inside an `RCUTILS_FAULT_INJECTION_TEST`)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int_least64_t rcutils_fault_injection_get_count(void);

/**
 * \brief 故障注入递减器的实现 (Implementation of fault injection decrementer)
 *
 * 这个函数包含在宏中，所以需要将其导出为公共函数，但是不应该直接使用它
 * (This is included inside of macros, so it needs to be exported as a public function, but it should not be used directly.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
int_least64_t _rcutils_fault_injection_maybe_fail(void);

/**
 * \def RCUTILS_FAULT_INJECTION_MAYBE_RETURN_ERROR
 * \brief 此宏检查并递减静态全局变量原子计数器，如果为0，则返回 `return_value_on_error`
 * (This macro checks and decrements a static global variable atomic counter and returns `return_value_on_error` if 0.)
 *
 * 此宏本身不是一个函数，因此当此宏返回时，它将使调用函数返回具有返回值
 * (This macro is not a function itself, so when this macro returns it will cause the calling function to return with the return value.)
 *
 * 使用 `RCUTILS_FAULT_INJECTION_SET_COUNT` 设置计数器。如果计数小于0，则
 * `RCUTILS_FAULT_INJECTION_MAYBE_RETURN_ERROR` 不会导致提前返回
 * (Set the counter with `RCUTILS_FAULT_INJECTION_SET_COUNT`. If the count is less than 0, then `RCUTILS_FAULT_INJECTION_MAYBE_RETURN_ERROR` will not cause an early return.)
 *
 * 此宏是线程安全的，并确保每次使用 `RCUTILS_FAULT_INJECTION_SET_COUNT` 设置故障注入计数器时，最多只有一个调用会导致失败
 * (This macro is thread-safe, and ensures that at most one invocation results in a failure for each time the fault injection counter is set with `RCUTILS_FAULT_INJECTION_SET_COUNT`)
 *
 * \param return_value_on_error 注入故障失败时要返回的值 (the value to return in the case of fault injected failure)
 */
#define RCUTILS_FAULT_INJECTION_MAYBE_RETURN_ERROR(return_value_on_error)                          \
  if (RCUTILS_FAULT_INJECTION_FAIL_NOW == _rcutils_fault_injection_maybe_fail()) {                 \
    printf(                                                                                        \
      "%s:%d Injecting fault and returning " #return_value_on_error "\n", __FILE__, __LINE__);     \
    return return_value_on_error;                                                                  \
  }

/**
 * \def RCUTILS_FAULT_INJECTION_MAYBE_FAIL
 * \brief 此宏检查并递减静态全局变量原子计数器，在计数器为0时执行 `failure_code`
 * (This macro checks and decrements a static global variable atomic counter and executes `failure_code` if the counter is 0 inside a scoped block)
 *
 * 此宏不是函数本身，因此它将使调用函数在 if 循环中执行代码
 * (This macro is not a function itself, so it will cause the calling function to execute the code from within an if loop.)
 *
 * 使用 `RCUTILS_FAULT_INJECTION_SET_COUNT` 设置计数器。如果计数小于0，则
 * `RCUTILS_FAULT_INJECTION_MAYBE_FAIL` 不会执行失败代码
 * (Set the counter with `RCUTILS_FAULT_INJECTION_SET_COUNT`. If the count is less than 0, then `RCUTILS_FAULT_INJECTION_MAYBE_FAIL` will not execute the failure code.)
 *
 * 此宏是线程安全的，并确保每次使用 `RCUTILS_FAULT_INJECTION_SET_COUNT` 设置故障注入计数器时，最多只有一个调用会导致失败
 * (This macro is thread-safe, and ensures that at most one invocation results in a failure for each time the fault injection counter is set with `RCUTILS_FAULT_INJECTION_SET_COUNT`)
 *
 * \param failure_code 注入故障失败时要执行的代码 (the code to execute in the case of fault injected failure)
 */
#define RCUTILS_FAULT_INJECTION_MAYBE_FAIL(failure_code)                                           \
  if (RCUTILS_FAULT_INJECTION_FAIL_NOW == _rcutils_fault_injection_maybe_fail()) {                 \
    printf("%s:%d Injecting fault and executing " #failure_code "\n", __FILE__, __LINE__);         \
    failure_code;                                                                                  \
  }

/**
 * \def RCUTILS_FAULT_INJECTION_TEST
 * \brief 故障注入宏，用于单元测试检查 `code` 是否能在所有执行路径中容忍故障注入。
 * \brief The fault injection macro for use with unit tests to check that `code` can tolerate injected failures at all points along the execution path.
 * 
 * 该宏应在 gtest 函数宏（如 'TEST'，'TEST_F' 等）内使用。
 * This macro is intended to be used within a gtest function macro like 'TEST', 'TEST_F', etc.
 *
 * `code` 在 do-while 循环中执行，因此其内声明的变量位于自己的作用域块中。
 * `code` is executed within a do-while loop and therefore any variables declared within are in their own scope block.
 *
 * 简单示例：
 * Here's a simple example:
 *
 *  RCUTILS_FAULT_INJECTION_TEST(
 *    rcl_ret_t ret = rcl_init(argc, argv, options, context);
 *    if (RCL_RET_OK == ret)
 *    {
 *        ret = rcl_shutdown(context);
 *    }
 * });
 *
 * 在这个例子中，你需要根据 `rcl_init` 的返回值进行条件执行。
 * In this example, you will need have conditional execution based on the return value of `rcl_init`.
 * 如果失败，则不调用 rcl_shutdown。在您的测试中，可能存在类似的逻辑，需要条件检查。
 * If it failed, then it wouldn't make sense to call rcl_shutdown. In your own test, there might be similar logic that requires conditional checks.
 * 编写此测试的目的不仅在于检查行为是否一致，而且在于确保故障不会导致程序崩溃、内存错误或不必要的内存泄漏。
 * The goal of writing this test is less about checking the behavior is consistent, but instead that failures do not cause program crashes, memory errors, or unnecessary memory leaks.
 */
#define RCUTILS_FAULT_INJECTION_TEST(code)                                                          \
  do {                                                                                              \
    int fault_injection_count = 0; /* 故障注入计数器 */                                      \
    /* Fault injection counter */                                                                   \
    do {                                                                                            \
      rcutils_fault_injection_set_count(fault_injection_count++); /* 设置故障注入计数器 */ \
      /* Set the fault injection counter */                                                         \
      code; /* 执行代码 */                                                                      \
      /* Execute the code */                                                                        \
    } while (!rcutils_fault_injection_is_test_complete()); /* 检查测试是否完成 */           \
    /* Check if the test is complete */                                                             \
    rcutils_fault_injection_set_count(RCUTILS_FAULT_INJECTION_NEVER_FAIL); /* 设置永不失败 */ \
    /* Set to never fail */                                                                         \
  } while (0)

/**
 * \def RCUTILS_NO_FAULT_INJECTION
 * \brief 在 `code` 执行期间暂停故障注入的便捷宏。
 * \brief A convenience macro built around rcutils_fault_injection_set_count() to pause fault injection during `code` execution.
 *
 * 该宏应在 RCUTILS_FAULT_INJECTION_TEST() 块内使用。
 * This macro is intended to be used within RCUTILS_FAULT_INJECTION_TEST() blocks.
 *
 * `code` 在 do-while 循环中执行，因此其内声明的变量位于自己的作用域块中。
 * `code` is executed within a do-while loop and therefore any variables declared within are in their own scope block.
 *
 * 简单示例：
 * Here's a simple example:
 *
 *  RCUTILS_FAULT_INJECTION_TEST({
 *    rcl_ret_t ret = rcl_init(argc, argv, options, context);
 *    if (RCL_RET_OK == ret)
 *    {
 *      RCUTILS_NO_FAULT_INJECTION({
 *        ret = rcl_shutdown(context);
 *      });
 *    }
 * });
 *
 * 在这个例子中，在成功的 rcl_init() 上调用 rcl_shutdown()，同时确保它不会由于故障注入而失败。
 * In this example, on successful rcl_init(), rcl_shutdown() is called while ensuring that it will not fail due to fault injection.
 */
#define RCUTILS_NO_FAULT_INJECTION(code)                                                                 \
  do {                                                                                                   \
    int64_t no_fault_injection_count =                                                                   \
      rcutils_fault_injection_get_count(); /* 获取当前故障注入计数 */                          \
    /* Get the current fault injection count */                                                          \
    rcutils_fault_injection_set_count(RCUTILS_FAULT_INJECTION_NEVER_FAIL); /* 设置永不失败 */      \
    /* Set to never fail */                                                                              \
    code; /* 执行代码 */                                                                             \
    /* Execute the code */                                                                               \
    rcutils_fault_injection_set_count(no_fault_injection_count); /* 恢复之前的故障注入计数 */ \
    /* Restore the previous fault injection count */                                                     \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__TESTING__FAULT_INJECTION_H_
