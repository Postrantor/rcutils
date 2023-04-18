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

#include "rcutils/testing/fault_injection.h"

#include "rcutils/stdatomic_helper.h"

// 定义一个原子变量 g_rcutils_fault_injection_count，初始值为 -1
// Define an atomic variable g_rcutils_fault_injection_count, initial value is -1
static atomic_int_least64_t g_rcutils_fault_injection_count = ATOMIC_VAR_INIT(-1);

// 设置 g_rcutils_fault_injection_count 的值
// Set the value of g_rcutils_fault_injection_count
void rcutils_fault_injection_set_count(int_least64_t count)
{
  rcutils_atomic_store(&g_rcutils_fault_injection_count, count);
}

// 获取 g_rcutils_fault_injection_count 的值
// Get the value of g_rcutils_fault_injection_count
int_least64_t rcutils_fault_injection_get_count()
{
  int_least64_t count = 0;
  rcutils_atomic_load(&g_rcutils_fault_injection_count, count);
  return count;
}

// 检查是否完成测试
// Check if the test is complete
bool rcutils_fault_injection_is_test_complete()
{
#ifndef RCUTILS_ENABLE_FAULT_INJECTION
  // 如果未启用故障注入，则返回 true
  // If fault injection is not enabled, return true
  return true;
#else  // RCUTILS_ENABLE_FAULT_INJECTION
  // 否则，检查 g_rcutils_fault_injection_count 是否大于 RCUTILS_FAULT_INJECTION_NEVER_FAIL
  // Otherwise, check if g_rcutils_fault_injection_count is greater than RCUTILS_FAULT_INJECTION_NEVER_FAIL
  return rcutils_fault_injection_get_count() > RCUTILS_FAULT_INJECTION_NEVER_FAIL;
#endif  // RCUTILS_ENABLE_FAULT_INJECTION
}

/**
 * @brief Attempts to decrement the fault injection counter and returns its current value.
 * @details This function is used in conjunction with fault injection tests to determine if a failure should be injected.
 * @return The current value of the fault injection counter.
 */
int_least64_t _rcutils_fault_injection_maybe_fail()
{
  // 设置原子操作成功标志为 false
  // Set atomic operation success flag to false
  bool set_atomic_success = false;

  // 获取当前故障注入计数值
  // Get the current fault injection count value
  int_least64_t current_count = rcutils_fault_injection_get_count();

  // 循环直到原子操作成功
  // Loop until atomic operation is successful
  do {
    // 如果 fault_injection_count 小于等于 0，表示 maybe_fail 不会失败，因此直接返回。
    // If fault_injection_count is less than or equal to 0, it means that maybe_fail will not fail, so just return.
    if (current_count <= RCUTILS_FAULT_INJECTION_NEVER_FAIL) {
      return current_count;
    }

    // 否则将其减少 1，但要以线程安全的方式进行，以便恰好一个调用线程获得 0 情况。
    // Otherwise decrement by one, but do so in a thread-safe manner so that exactly one calling thread gets the 0 case.
    int_least64_t desired_count = current_count - 1;

    // 使用原子比较交换函数来更新 g_rcutils_fault_injection_count 的值
    // Use the atomic compare exchange function to update the value of g_rcutils_fault_injection_count
    rcutils_atomic_compare_exchange_strong(
      &g_rcutils_fault_injection_count, set_atomic_success, &current_count, desired_count);

  // 如果原子操作未成功，则继续循环
  // Continue looping if atomic operation is not successful
  } while (!set_atomic_success);

  // 返回当前故障注入计数值
  // Return the current fault injection count value
  return current_count;
}
