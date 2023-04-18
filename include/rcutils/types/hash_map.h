// Copyright 2018-2019 Open Source Robotics Foundation, Inc.
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

#ifndef RCUTILS__TYPES__HASH_MAP_H_
#define RCUTILS__TYPES__HASH_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

struct rcutils_hash_map_impl_s;

/// 结构体，保存哈希映射的元数据（The structure holding the metadata for a hash map）.
typedef struct RCUTILS_PUBLIC_TYPE rcutils_hash_map_s
{
  /// 指向 PIMPL 实现类型的指针（A pointer to the PIMPL implementation type）.
  struct rcutils_hash_map_impl_s * impl;
} rcutils_hash_map_t;

/// 键散列函数的函数签名（The function signature for a key hashing function）.
/**
 * \param[in] key 需要进行哈希的键（The key that needs to be hashed）
 * \return 提供字符串的哈希值（A hash value for the provided string）
 */
typedef size_t (*rcutils_hash_map_key_hasher_t)(const void * key);

/// 键比较函数的函数签名（The function signature for a key comparison function）.
/**
 * \param[in] val1 第一个要比较的值（The first value to compare）
 * \param[in] val2 第二个要比较的值（The second value to compare）
 * \return 如果 val1 < val2，返回负数（A negative number if val1 < val2, or）
 * \return 如果 val1 > val2，返回正数（A positve number if val1 > val2, or）
 * \return 如果 val1 == val2，返回零（Zero if val1 == val2）.
 */
typedef int (*rcutils_hash_map_key_cmp_t)(const void * val1, const void * val2);

/**
 * 验证 rcutils_hash_map_t* 是否指向有效的哈希映射（Validates that an rcutils_hash_map_t* points to a valid hash map）.
 * \param[in] map 指向 rcutils_hash_map_t 的指针（A pointer to an rcutils_hash_map_t）
 * \return 如果 map 为空，返回 #RCUTILS_RET_INVALID_ARGUMENT（#RCUTILS_RET_INVALID_ARGUMENT if map is null）
 * \return 如果 map 未初始化，返回 #RCUTILS_RET_NOT_INITIALIZED（#RCUTILS_RET_NOT_INITIALIZED if map is not initialized）.
 */
#define HASH_MAP_VALIDATE_HASH_MAP(map)                                                            \
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(map, RCUTILS_RET_INVALID_ARGUMENT);                              \
  if (NULL == map->impl) {                                                                         \
    RCUTILS_SET_ERROR_MSG("map is not initialized");                                               \
    return RCUTILS_RET_NOT_INITIALIZED;                                                            \
  }

/// 空终止 C 字符串的哈希函数（A hashing function for a null terminated c string）.
/**
 * 空终止 C 字符串的哈希函数（A hashing function for a null terminated c string）.
 * 当您的键只是指向一个 C 字符串的指针时，应该使用此函数（Should be used when your key is just a pointer to a c-string）
 */
RCUTILS_PUBLIC
size_t rcutils_hash_map_string_hash_func(const void * key_str);

/// 空终止 C 字符串的比较函数（A comparison function for a null terminated c string）.
/**
 * 空终止 C 字符串的比较函数（A comparison function for a null terminated c string）.
 * 当您的键只是指向一个 C 字符串的指针时，应该使用此函数（Should be used when your key is just a pointer to a c-string）
 */
RCUTILS_PUBLIC
int rcutils_hash_map_string_cmp_func(const void * val1, const void * val2);

/// 返回一个空的 hash_map 结构体（Return an empty hash_map struct）.
/**
 * 此函数返回一个空且零初始化的 hash_map 结构体（This function returns an empty and zero initialized hash_map struct）.
 * 在使用之前，所有哈希映射都应使用此函数或手动初始化（All hash maps should be initialized with this or manually initialized before being used）.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * 示例（Example）:
 * ```c
 * // 不要这样做（Do not do this）:
 * // rcutils_hash_map_t foo;
 * // rcutils_hash_map_fini(&foo); // 未定义行为（undefined behavior）!
 *
 * // 改为这样做（Do this instead）:
 * rcutils_hash_map_t bar = rcutils_get_zero_initialized_hash_map();
 * rcutils_hash_map_fini(&bar); // ok
 * ```
 * */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_hash_map_t rcutils_get_zero_initialized_hash_map();

/// 初始化 rcutils_hash_map_t，为给定容量分配空间。
/// Initialize a rcutils_hash_map_t, allocating space for given capacity.
/**
 * 本函数使用给定的初始容量初始化 rcutils_hash_map_t。
 * This function initializes the rcutils_hash_map_t with a given initial
 * capacity for entries.
 * 注意，这并不会为哈希映射中的键或值分配空间，只会为指向键和值的指针数组分配空间。
 * Note this does not allocate space for keys or values in the hash_map, just the
 * arrays of pointers to the keys and values.
 * 分配值时仍应使用 rcutils_hash_map_set()。
 * rcutils_hash_map_set() should still be used when assigning values.
 *
 * 哈希映射参数应指向已分配的内存，并且应通过 rcutils_get_zero_initialized_hash_map() 进行零初始化。
 * The hash_map argument should point to allocated memory and should have
 * been zero initialized with rcutils_get_zero_initialized_hash_map().
 *
 * <hr>
 * 属性              | 遵循
 * Attribute          | Adherence
 * ------------------ | -------------
 * 分配内存           | 是
 * Allocates Memory   | Yes
 * 线程安全           | 否
 * Thread-Safe        | No
 * 使用原子操作       | 否
 * Uses Atomics       | No
 * 无锁               | 是
 * Lock-Free          | Yes
 *
 * 示例：
 * Example:
 * ```c
 * rcutils_hash_map_t hash_map = rcutils_get_zero_initialized_hash_map();
 * rcutils_ret_t ret =
 *   rcutils_hash_map_init(&hash_map, 2, rcutils_get_default_allocator());
 * if (ret != RCUTILS_RET_OK) {
 *   // ... 进行错误处理
 *   // ... do error handling
 * }
 * // ... 使用哈希映射，完成后：
 * // ... use the hash_map and when done:
 * ret = rcutils_hash_map_fini(&hash_map);
 * if (ret != RCUTILS_RET_OK) {
 *   // ... 进行错误处理
 *   // ... do error handling
 * }
 * ```
 *
 * \param[inout] hash_map 要初始化的 rcutils_hash_map_t
 * \param[inout] hash_map rcutils_hash_map_t to be initialized
 * \param[in] initial_capacity 哈希映射的初始容量 - 必须大于零，并自动向上舍入为 2 的幂次
 * \param[in] initial_capacity the amount of initial capacity for the hash_map - this must be
 *                             greater than zero and will be automatically rounded up to the next power of 2
 * \param[in] key_size 用于索引数据的键的大小（以字节为单位）
 * \param[in] key_size the size (in bytes) of the key used to index the data
 * \param[in] data_size 存储的数据的大小（以字节为单位）
 * \param[in] data_size the size (in bytes) of the data being stored
 * \param[in] key_hashing_func 返回键的哈希值的函数
 * \param[in] key_hashing_func a function that returns a hashed value for a key
 * \param[in] key_cmp_func 用于比较键的函数
 * \param[in] key_cmp_func a function used to compare keys
 * \param[in] allocator 在哈希映射的生命周期中使用的分配器
 * \param[in] allocator the allocator to use through out the lifetime of the hash_map
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation fails, or
 * \return #RCUTILS_RET_STRING_MAP_ALREADY_INIT 如果已初始化，或
 * \return #RCUTILS_RET_STRING_MAP_ALREADY_INIT if alread initialized, or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_hash_map_init(
  rcutils_hash_map_t * hash_map,
  size_t initial_capacity,
  size_t key_size,
  size_t data_size,
  rcutils_hash_map_key_hasher_t key_hashing_func,
  rcutils_hash_map_key_cmp_t key_cmp_func,
  const rcutils_allocator_t * allocator);

/// 结束之前初始化的 hash_map 结构。
/// Finalize the previously initialized hash_map struct.
/**
 * 此函数将释放在初始化时创建的资源，或者在调用 rcutils_hash_map_set() 时创建的资源。
 * This function will free any resources which were created when initializing
 * or when calling rcutils_hash_map_set().
 *
 * <hr>
 * 属性              | 遵循
 * Attribute          | Adherence
 * ------------------ | -------------
 * 分配内存           | 否
 * Allocates Memory   | No
 * 线程安全           | 否
 * Thread-Safe        | No
 * 使用原子操作       | 否
 * Uses Atomics       | No
 * 无锁               | 是
 * Lock-Free          | Yes
 *
 * \param[inout] hash_map 要结束的 rcutils_hash_map_t
 * \param[inout] hash_map rcutils_hash_map_t to be finalized
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_hash_map_fini(rcutils_hash_map_t * hash_map);

/// 获取 hash_map 的当前容量。
/// Get the current capacity of the hash_map.
/**
 * 此函数将返回哈希映射的内部容量，即哈希映射用于对键进行排序的桶数。
 * This function will return the internal capacity of the hash_map, which is the
 * number of buckets the hash_map uses to sort the keys.
 * 容量并不表示存储在哈希映射中的键值对数量，rcutils_hash_map_get_size() 函数可以提供该信息，也不表示在增加容量之前可以存储的最大数量。
 * The capacity does not indicate how many key value pairs are stored in the
 * hash_map, the rcutils_hash_map_get_size() function can provide that, nor the
 * maximum number that can be stored without increasing the capacity.
 * 初始容量可以通过 rcutils_hash_map_init() 设置。
 * The capacity can be set initially with rcutils_hash_map_init().
 *
 * <hr>
 * 属性              | 遵循
 * Attribute          | Adherence
 * ------------------ | -------------
 * 分配内存           | 否
 * Allocates Memory   | No
 * 线程安全           | 否
 * Thread-Safe        | No
 * 使用原子操作       | 否
 * Uses Atomics       | No
 * 无锁               | 是
 * Lock-Free          | Yes
 *
 * \param[in] hash_map 要查询的 rcutils_hash_map_t
 * \param[in] hash_map rcutils_hash_map_t to be queried
 * \param[out] capacity 哈希映射的容量
 * \param[out] capacity capacity of the hash_map
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_NOT_INITIALIZED 如果哈希映射无效，或
 * \return #RCUTILS_RET_NOT_INITIALIZED if the hash_map is invalid, or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_hash_map_get_capacity(const rcutils_hash_map_t * hash_map, size_t * capacity);

/// 获取hash_map的当前大小。 (Get the current size of the hash_map.)
/**
 * 此函数将返回hash_map的内部大小，即hash_map中当前的键值对数。
 * 调用rcutils_hash_map_set()或rcutils_hash_map_unset()时，大小会发生变化。
 * (This function will return the internal size of the hash_map, which is the
 * current number of key value pairs in the hash_map.
 * The size is changed when calling rcutils_hash_map_set() or rcutils_hash_map_unset().)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] hash_map 要查询的rcutils_hash_map_t (rcutils_hash_map_t to be queried)
 * \param[out] size hash_map的大小 (size of the hash_map)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_NOT_INITIALIZED 如果hash_map无效，或 (if the hash_map is invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。(if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_hash_map_get_size(const rcutils_hash_map_t * hash_map, size_t * size);

/// 在hash_map中设置一个键值对，如有必要增加容量。 (Set a key value pair in the hash_map, increasing capacity if necessary.)
/**
 * 如果键已经存在于映射中，则将值更新为提供的新值。
 * 如果尚不存在，则为新键和值添加一个新条目。如有必要，将增加容量。
 * (If the key already exists in the map then the value is updated to the new value
 * provided. If it does not already exist then a new entry is added for the new key
 * and value. The capacity will be increased if needed.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] hash_map 要更新的rcutils_hash_map_t (rcutils_hash_map_t to be updated)
 * \param[in] key hash_map的键 (hash_map key)
 * \param[in] value 给定hash_map键的值 (value for given hash_map key)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或 (if memory allocation fails, or)
 * \return #RCUTILS_RET_NOT_INITIALIZED 如果hash_map无效，或 (if the hash_map is invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。(if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_hash_map_set(rcutils_hash_map_t * hash_map, const void * key, const void * value);

/// 取消设置hash_map中的键值对。 (Unset a key value pair in the hash_map.)
/**
 * 取消设置hash_map中的键值对，并释放分配给条目的任何内部资源。
 * 删除键时，此函数永远不会减少容量。如果找不到给定的键，则返回RCUTILS_RET_STRING_KEY_NOT_FOUND。
 * (Unsets the key value pair in the hash_map and frees any internal resources allocated
 * for the entry. This function will never decrease the capacity when removing keys.
 * If the given key is not found, RCUTILS_RET_STRING_KEY_NOT_FOUND is returned.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[inout] hash_map 要更新的rcutils_hash_map_t (rcutils_hash_map_t to be updated)
 * \param[in] key hash_map的键，必须是以空字符结尾的C字符串 (hash_map key, must be null terminated c string)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_NOT_INITIALIZED 如果hash_map无效，或 (if the hash_map is invalid, or)
 * \return #RCUTILS_RET_STRING_KEY_NOT_FOUND 如果在映射中找不到键，或 (if the key is not found in the map, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。(if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_hash_map_unset(rcutils_hash_map_t * hash_map, const void * key);

/// 获取键是否存在。 (Get whether or not a key exists.)
/**
 * 如果提供的键存在于hash_map中，则返回true；如果不存在或hash_map或键无效，则返回false。
 * 在所有情况下，都不会设置错误消息。
 * (Returns true if the provided key exists in the hash_map or false if it does not or
 * if the hash_map or key are invalid.
 * In all cases no error message is set.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] hash_map 要搜索的rcutils_hash_map_t (rcutils_hash_map_t to be searched)
 * \param[in] key hash_map的键，必须是以空字符结尾的C字符串 (hash_map key, must be null terminated c string)
 * \return `true` 如果键在hash_map中，或 (if key is in the hash_map, or)
 * \return `false` 如果键不在hash_map中，或 (if key is not in the hash_map, or)
 * \return `false` 对于无效参数，或 (for invalid arguments, or)
 * \return `false` 如果hash_map无效。(if the hash_map is invalid.)
 */
RCUTILS_PUBLIC
bool rcutils_hash_map_key_exists(const rcutils_hash_map_t * hash_map, const void * key);

/// 获取给定键的值。 (Get value given a key)
/**
 * 此函数可用于检索存储数据的浅拷贝。数据指针必须指向一段足够大的内存区域，
 * 以便复制存储的数据的完整大小，该大小在初始化 hash_map 时指定。
 * (This function can be used to retrieve a shallow copy of the stored data. The data
 * pointer must point to a section of memory large enough to copy the full size of
 * the data being stored, which is specified when the hash_map in initialized.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] hash_map 要搜索的 rcutils_hash_map_t (rcutils_hash_map_t to be searched)
 * \param[in] key 查找数据的 hash_map 键 (hash_map key to look up the data for)
 * \param[out] data 映射中存储的数据副本 (A copy of the data stored in the map)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_NOT_INITIALIZED 如果 hash_map 无效，或 (if the hash_map is invalid, or)
 * \return #RCUTILS_RET_NOT_FOUND 如果密钥在映射中不存在，或 (if the key doesn't exist in the map, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
rcutils_ret_t
rcutils_hash_map_get(const rcutils_hash_map_t * hash_map, const void * key, void * data);

/// 获取 hash_map 中的下一个键，除非给定 NULL，则获取第一个键。
/// (Get the next key in the hash_map, unless NULL is given, then get the first key)
/**
 * 此函数允许您迭代地获取 hash_map 中的每个键/值对。
 * (This function allows you to iteratively get each key/value pair in the hash_map.)
 *
 * 如果为 previous_key 给定 NULL，则返回 hash_map 中的第一个键。
 * 如果返回的键作为此函数的下一次调用的 previous_key，则返回 hash_map 中的下一个键。
 * 如果 hash_map 中没有更多的键或给定的键不在 hash_map 中，则返回错误。
 * (If NULL is given for the previous_key, then the first key in the hash_map is returned.
 * If that returned key is given as the previous_key for the next call to this function,
 * then the next key in the hash_map is returned.
 * If there are no more keys in the hash_map or if the given key is not in the hash_map,
 * an error will be returned.)
 *
 * hash_map 中键的顺序是任意的，如果在调用此函数之间修改了 hash_map，则行为是未定义的。
 * 如果修改了 hash_map，则应通过传递 NULL 重新开始迭代以再次获取第一个键。
 * (The order of the keys in the hash_map is arbitrary and if the hash_map is modified
 * between calls to this function the behavior is undefined.
 * If the hash_map is modified then iteration should begin again by passing NULL to
 * get the first key again.)
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * Example:
 * ```c
 * printf("entries in the hash_map:\n");
 * int key = 0, data = 0;
 * rcutils_ret_t status = rcutils_hash_map_get_next_key(&hash_map, NULL, &key, &data);
 * while (RCUTILS_RET_OK == status) {
 *   printf("%i: %i\n", key, data);
 *   status = rcutils_hash_map_get_next_key(&hash_map, &key, &key, &data);
 * }
 * ```
 *
 * \param[in] hash_map 要查询的 rcutils_hash_map_t (rcutils_hash_map_t to be queried)
 * \param[in] previous_key 获取下一个键的 NULL 或前一个键 (NULL to get the first key or the previous key to get the next for)
 * \param[out] key 序列中下一个键的副本 (A copy of the next key in the sequence)
 * \param[out] data 序列中下一个数据的副本 (A copy of the next data in the sequence)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_NOT_INITIALIZED 如果 hash_map 无效，或 (if the hash_map is invalid, or)
 * \return #RCUTILS_RET_NOT_FOUND 如果 previous_key 在映射中不存在，或 (if the previous_key doesn't exist in the map, or)
 * \return #RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES 如果在 previous_key 之后没有更多数据，或 (if there is no more data beyound the previous_key, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_hash_map_get_next_key_and_data(
  const rcutils_hash_map_t * hash_map, const void * previous_key, void * key, void * data);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__TYPES__HASH_MAP_H_
