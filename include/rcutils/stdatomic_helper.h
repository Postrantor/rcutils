// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__STDATOMIC_HELPER_H_
#define RCUTILS__STDATOMIC_HELPER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// disable unused function warnings within this file, due to inline in a header
#if !defined(_WIN32)
#pragma GCC diagnostic push
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-function"
#endif
#endif

#if !defined(_WIN32)

// The my__has_feature avoids a preprocessor error when you check for it and
// use it on the same line below.
#if defined(__has_feature)
#define my__has_feature __has_feature
#else
#define my__has_feature(x) 0
#endif

#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ <= 9
// If GCC and below GCC-4.9, use the compatability header.
#include "stdatomic_helper/gcc/stdatomic.h"
#else // !defined(__clang__) && defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ <= 9
#if __cplusplus
// NOLINTNEXTLINE
#error "cannot be used with C++ due to a conflict with the C++ <atomic> header, see: p0943r1"
// See: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0943r1.html"
#else
#if defined(__has_feature) && !my__has_feature(c_atomic)
// If Clang and no c_atomics (true for some older versions), use the compatability header.
#include "stdatomic_helper/gcc/stdatomic.h"
#else
#include <stdatomic.h>
#endif
#endif
#endif // !defined(__clang__) && defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ <= 9

/**
 * @brief 宏 rcutils_atomic_load 用于原子地加载对象中的值。
 * @param[in] object 要加载值的原子对象。
 * @param[out] out 存储加载到的值的变量。
 *
 * Macro rcutils_atomic_load is used to atomically load the value in the object.
 * @param[in] object The atomic object to load the value from.
 * @param[out] out The variable to store the loaded value.
 */
#define rcutils_atomic_load(object, out) (out) = atomic_load(object)

/**
 * @brief 宏 rcutils_atomic_compare_exchange_strong 用于原子地比较并交换对象中的值。
 * @param[in] object 要进行比较和交换操作的原子对象。
 * @param[out] out 存储操作结果的变量，成功时为 true，失败时为 false。
 * @param[in,out] expected 期望值，如果对象值与期望值相等，则执行交换操作。否则，将对象值存储在此变量中。
 * @param[in] desired 如果对象值与期望值相等，则将对象值设置为 desired。
 *
 * Macro rcutils_atomic_compare_exchange_strong is used to atomically compare and exchange the value in the object.
 * @param[in] object The atomic object to perform the compare and exchange operation on.
 * @param[out] out The variable to store the result of the operation, true if successful, false if failed.
 * @param[in,out] expected The expected value, if the object value equals the expected value, the exchange operation is performed. Otherwise, the object value is stored in this variable.
 * @param[in] desired If the object value equals the expected value, set the object value to desired.
 */
#define rcutils_atomic_compare_exchange_strong(object, out, expected, desired)                     \
  (out) = atomic_compare_exchange_strong(object, expected, desired)

/**
 * @brief 宏 rcutils_atomic_exchange 用于原子地交换对象中的值。
 * @param[in] object 要进行交换操作的原子对象。
 * @param[out] out 存储旧值的变量。
 * @param[in] desired 将对象值设置为 desired。
 *
 * Macro rcutils_atomic_exchange is used to atomically exchange the value in the object.
 * @param[in] object The atomic object to perform the exchange operation on.
 * @param[out] out The variable to store the old value.
 * @param[in] desired Set the object value to desired.
 */
#define rcutils_atomic_exchange(object, out, desired) (out) = atomic_exchange(object, desired)

/**
 * @brief 宏 rcutils_atomic_store 用于原子地存储对象中的值。
 * @param[in] object 要存储值的原子对象。
 * @param[in] desired 将对象值设置为 desired。
 *
 * Macro rcutils_atomic_store is used to atomically store the value in the object.
 * @param[in] object The atomic object to store the value.
 * @param[in] desired Set the object value to desired.
 */
#define rcutils_atomic_store(object, desired) atomic_store(object, desired)

/**
 * @brief 宏 rcutils_atomic_fetch_add 用于原子地对对象中的值进行加法操作。
 * @param[in] object 要进行加法操作的原子对象。
 * @param[out] out 存储操作之前的值的变量。
 * @param[in] arg 要添加到对象值的值。
 *
 * Macro rcutils_atomic_fetch_add is used to atomically perform addition operation on the value in the object.
 * @param[in] object The atomic object to perform the addition operation on.
 * @param[out] out The variable to store the value before the operation.
 * @param[in] arg The value to be added to the object value.
 */
#define rcutils_atomic_fetch_add(object, out, arg) (out) = atomic_fetch_add(object, arg)

#else // !defined(_WIN32)

#include "./stdatomic_helper/win32/stdatomic.h"

#define rcutils_atomic_load(object, out) rcutils_win32_atomic_load(object, out)

#define rcutils_atomic_compare_exchange_strong(object, out, expected, desired)                     \
  rcutils_win32_atomic_compare_exchange_strong(object, out, expected, desired)

#define rcutils_atomic_exchange(object, out, desired)                                              \
  rcutils_win32_atomic_exchange(object, out, desired)

#define rcutils_atomic_store(object, desired) rcutils_win32_atomic_store(object, desired)

#define rcutils_atomic_fetch_add(object, out, arg) rcutils_win32_atomic_fetch_add(object, out, arg)

#endif // !defined(_WIN32)

/**
 * @brief 从一个 atomic_bool 类型的变量中原子性地加载一个布尔值 (Atomically loads a boolean value from an atomic_bool variable)
 * @param a_bool 指向要加载的 atomic_bool 类型变量的指针 (Pointer to the atomic_bool variable to load from)
 * @return 返回加载到的布尔值 (Returns the loaded boolean value)
 */
static inline bool rcutils_atomic_load_bool(atomic_bool * a_bool)
{
  // 初始化结果变量为 false (Initialize result variable to false)
  bool result = false;

  // 原子性地加载布尔值 (Atomically load the boolean value)
  rcutils_atomic_load(a_bool, result);

  // 返回加载到的布尔值 (Return the loaded boolean value)
  return result;
}

/**
 * @brief 从一个 atomic_int_least64_t 类型的变量中原子性地加载一个 int64_t 值 (Atomically loads an int64_t value from an atomic_int_least64_t variable)
 * @param a_int64_t 指向要加载的 atomic_int_least64_t 类型变量的指针 (Pointer to the atomic_int_least64_t variable to load from)
 * @return 返回加载到的 int64_t 值 (Returns the loaded int64_t value)
 */
static inline int64_t rcutils_atomic_load_int64_t(atomic_int_least64_t * a_int64_t)
{
  // 初始化结果变量为 0 (Initialize result variable to 0)
  int64_t result = 0;

  // 原子性地加载 int64_t 值 (Atomically load the int64_t value)
  rcutils_atomic_load(a_int64_t, result);

  // 返回加载到的 int64_t 值 (Return the loaded int64_t value)
  return result;
}

/**
 * @brief 从一个 atomic_uint_least64_t 类型的变量中原子性地加载一个 uint64_t 值 (Atomically loads a uint64_t value from an atomic_uint_least64_t variable)
 * @param a_uint64_t 指向要加载的 atomic_uint_least64_t 类型变量的指针 (Pointer to the atomic_uint_least64_t variable to load from)
 * @return 返回加载到的 uint64_t 值 (Returns the loaded uint64_t value)
 */
static inline uint64_t rcutils_atomic_load_uint64_t(atomic_uint_least64_t * a_uint64_t)
{
  // 初始化结果变量为 0 (Initialize result variable to 0)
  uint64_t result = 0;

  // 原子性地加载 uint64_t 值 (Atomically load the uint64_t value)
  rcutils_atomic_load(a_uint64_t, result);

  // 返回加载到的 uint64_t 值 (Return the loaded uint64_t value)
  return result;
}

/**
 * @brief 从一个 atomic_uintptr_t 类型的变量中原子性地加载一个 uintptr_t 值 (Atomically loads a uintptr_t value from an atomic_uintptr_t variable)
 * @param a_uintptr_t 指向要加载的 atomic_uintptr_t 类型变量的指针 (Pointer to the atomic_uintptr_t variable to load from)
 * @return 返回加载到的 uintptr_t 值 (Returns the loaded uintptr_t value)
 */
static inline uintptr_t rcutils_atomic_load_uintptr_t(atomic_uintptr_t * a_uintptr_t)
{
  // 初始化结果变量为 0 (Initialize result variable to 0)
  uintptr_t result = 0;

  // 原子性地加载 uintptr_t 值 (Atomically load the uintptr_t value)
  rcutils_atomic_load(a_uintptr_t, result);

  // 返回加载到的 uintptr_t 值 (Return the loaded uintptr_t value)
  return result;
}

/**
 * @brief 对一个 atomic_uint_least64_t 类型的变量执行原子性的比较和交换操作 (Performs an atomic compare-and-exchange operation on an atomic_uint_least64_t variable)
 * @param a_uint_least64_t 指向要操作的 atomic_uint_least64_t 类型变量的指针 (Pointer to the atomic_uint_least64_t variable to operate on)
 * @param expected 期望值的指针，如果与实际值相等，则执行交换操作 (Pointer to the expected value, if equal to the actual value, perform exchange operation)
 * @param desired 要设置的新值 (The new value to set)
 * @return 如果交换成功返回 true，否则返回 false (Returns true if the exchange was successful, otherwise returns false)
 */
static inline bool rcutils_atomic_compare_exchange_strong_uint_least64_t(
  atomic_uint_least64_t * a_uint_least64_t, uint64_t * expected, uint64_t desired)
{
  // 初始化结果变量 (Initialize result variable)
  bool result;

#if defined(__clang__)
#pragma clang diagnostic push
  // we know it's a gnu feature, but clang supports it, so suppress pedantic warning
#pragma clang diagnostic ignored "-Wgnu-statement-expression"
#endif

  // 原子性地执行比较和交换操作 (Atomically perform the compare-and-exchange operation)
  rcutils_atomic_compare_exchange_strong(a_uint_least64_t, result, expected, desired);

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

  // 返回操作结果 (Return the operation result)
  return result;
}

/**
 * @brief 对一个 atomic_bool 类型的变量执行原子性的交换操作 (Performs an atomic exchange operation on an atomic_bool variable)
 * @param a_bool 指向要操作的 atomic_bool 类型变量的指针 (Pointer to the atomic_bool variable to operate on)
 * @param desired 要设置的新值 (The new value to set)
 * @return 返回交换前的旧值 (Returns the old value before the exchange)
 */
static inline bool rcutils_atomic_exchange_bool(atomic_bool * a_bool, bool desired)
{
  // 初始化结果变量 (Initialize result variable)
  bool result;

  // 原子性地执行交换操作 (Atomically perform the exchange operation)
  rcutils_atomic_exchange(a_bool, result, desired);

  // 返回交换前的旧值 (Return the old value before the exchange)
  return result;
}

/**
 * @brief 以原子方式交换 int64_t 类型的值 (Atomically exchange an int64_t value)
 *
 * @param[in] a_int64_t 指向要交换的原子整数的指针 (Pointer to the atomic integer to be exchanged)
 * @param[in] desired 要设置的新值 (The new value to be set)
 * @return 返回被替换的旧值 (Returns the old value that was replaced)
 */
static inline int64_t
rcutils_atomic_exchange_int64_t(atomic_int_least64_t * a_int64_t, int64_t desired)
{
  // 声明一个 int64_t 类型的变量存储结果 (Declare an int64_t variable to store the result)
  int64_t result;

  // 调用 rcutils_atomic_exchange 函数进行原子交换操作 (Call the rcutils_atomic_exchange function to perform the atomic exchange operation)
  rcutils_atomic_exchange(a_int64_t, result, desired);

  // 返回交换后的结果 (Return the exchanged result)
  return result;
}

/**
 * @brief 以原子方式交换 uint64_t 类型的值 (Atomically exchange a uint64_t value)
 *
 * @param[in] a_uint64_t 指向要交换的无符号原子整数的指针 (Pointer to the unsigned atomic integer to be exchanged)
 * @param[in] desired 要设置的新值 (The new value to be set)
 * @return 返回被替换的旧值 (Returns the old value that was replaced)
 */
static inline uint64_t
rcutils_atomic_exchange_uint64_t(atomic_uint_least64_t * a_uint64_t, uint64_t desired)
{
  // 声明一个 uint64_t 类型的变量存储结果 (Declare a uint64_t variable to store the result)
  uint64_t result;

  // 调用 rcutils_atomic_exchange 函数进行原子交换操作 (Call the rcutils_atomic_exchange function to perform the atomic exchange operation)
  rcutils_atomic_exchange(a_uint64_t, result, desired);

  // 返回交换后的结果 (Return the exchanged result)
  return result;
}

/**
 * @brief 以原子方式交换 uintptr_t 类型的值 (Atomically exchange a uintptr_t value)
 *
 * @param[in] a_uintptr_t 指向要交换的无符号指针整数的指针 (Pointer to the unsigned pointer integer to be exchanged)
 * @param[in] desired 要设置的新值 (The new value to be set)
 * @return 返回被替换的旧值 (Returns the old value that was replaced)
 */
static inline uintptr_t
rcutils_atomic_exchange_uintptr_t(atomic_uintptr_t * a_uintptr_t, uintptr_t desired)
{
  // 声明一个 uintptr_t 类型的变量存储结果 (Declare a uintptr_t variable to store the result)
  uintptr_t result;

  // 调用 rcutils_atomic_exchange 函数进行原子交换操作 (Call the rcutils_atomic_exchange function to perform the atomic exchange operation)
  rcutils_atomic_exchange(a_uintptr_t, result, desired);

  // 返回交换后的结果 (Return the exchanged result)
  return result;
}

/**
 * @brief 以原子方式对 uint64_t 类型的值进行加法操作 (Atomically perform addition operation on a uint64_t value)
 *
 * @param[in] a_uint64_t 指向要进行加法操作的无符号原子整数的指针 (Pointer to the unsigned atomic integer to perform addition operation on)
 * @param[in] arg 要添加的值 (The value to be added)
 * @return 返回执行加法操作前的旧值 (Returns the old value before performing the addition operation)
 */
static inline uint64_t
rcutils_atomic_fetch_add_uint64_t(atomic_uint_least64_t * a_uint64_t, uint64_t arg)
{
  // 声明一个 uint64_t 类型的变量存储结果 (Declare a uint64_t variable to store the result)
  uint64_t result;

  // 调用 rcutils_atomic_fetch_add 函数进行原子加法操作 (Call the rcutils_atomic_fetch_add function to perform the atomic addition operation)
  rcutils_atomic_fetch_add(a_uint64_t, result, arg);

  // 返回执行加法操作前的旧值 (Return the old value before performing the addition operation)
  return result;
}

#if !defined(_WIN32)
#pragma GCC diagnostic pop
#endif

#endif // RCUTILS__STDATOMIC_HELPER_H_
