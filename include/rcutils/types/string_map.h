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

#ifndef RCUTILS__TYPES__STRING_MAP_H_
#define RCUTILS__TYPES__STRING_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

struct rcutils_string_map_impl_s;

/// 字符串映射的元数据结构。 (The structure holding the metadata for a string map.)
typedef struct RCUTILS_PUBLIC_TYPE rcutils_string_map_s
{
  /// 指向 PIMPL 实现类型的指针。 (A pointer to the PIMPL implementation type.)
  struct rcutils_string_map_impl_s * impl;
} rcutils_string_map_t;

/// 返回一个空的字符串映射结构。 (Return an empty string map struct.)
/**
 * 此函数返回一个空的且初始化为零的字符串映射结构。 (This function returns an empty and zero initialized string map struct.)
 *
 * 示例： (Example:)
 *
 * ```c
 * // 不要这样做： (Do not do this:)
 * // rcutils_string_map_t foo;
 * // rcutils_string_map_fini(&foo); // 未定义行为！ (undefined behavior!)
 *
 * // 而是这样做： (Do this instead:)
 * rcutils_string_map_t bar = rcutils_get_zero_initialized_string_map();
 * rcutils_string_map_fini(&bar); // ok
 * ```
 * */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_string_map_t rcutils_get_zero_initialized_string_map();

/// 初始化 rcutils_string_map_t，为给定容量分配空间。 (Initialize a rcutils_string_map_t, allocating space for given capacity.)
/**
 * 本函数使用给定的初始容量来初始化 rcutils_string_map_t 中的条目。 (This function initializes the rcutils_string_map_t with a given initial capacity for entries.)
 * 请注意，这并不会为映射中的键或值分配空间，只分配指向键和值的指针数组。 (Note this does not allocate space for keys or values in the map, just the arrays of pointers to the keys and values.)
 * 在分配值时，仍应使用 rcutils_string_map_set()。 (rcutils_string_map_set() should still be used when assigning values.)
 *
 * string_map 参数应指向已分配的内存，并且应使用 rcutils_get_zero_initialized_string_map() 进行零初始化。 (The string_map argument should point to allocated memory and should have been zero initialized with rcutils_get_zero_initialized_string_map().)
 * 例如： (For example:)
 *
 * ```c
 * rcutils_string_map_t string_map = rcutils_get_zero_initialized_string_map();
 * rcutils_ret_t ret =
 *   rcutils_string_map_init(&string_map, 10, rcutils_get_default_allocator());
 * if (ret != RCUTILS_RET_OK) {
 *   // ... 错误处理 (do error handling)
 * }
 * // ... 使用字符串映射，完成后： (use the string map and when done:)
 * ret = rcutils_string_map_fini(&string_map);
 * if (ret != RCUTILS_RET_OK) {
 *   // ... 错误处理 (do error handling)
 * }
 * ```
 *
 * \param[inout] string_map 要初始化的 rcutils_string_map_t (rcutils_string_map_t to be initialized)
 * \param[in] initial_capacity 字符串映射的初始容量 (the amount of initial capacity for the string map)
 * \param[in] allocator 在映射的生命周期中要使用的分配器 (the allocator to use through out the lifetime of the map)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或 (if memory allocation fails, or)
 * \return #RCUTILS_RET_STRING_MAP_ALREADY_INIT 如果已经初始化，或 (if already initialized, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_map_init(
  rcutils_string_map_t * string_map, size_t initial_capacity, rcutils_allocator_t allocator);

/// 结束先前初始化的字符串映射结构。 (Finalize the previously initialized string map struct.)
/**
 * 该函数将释放在初始化时或调用 rcutils_string_map_set() 时创建的任何资源。 (This function will free any resources which were created when initializing or when calling rcutils_string_map_set().)
 *
 * \param[inout] string_map 要结束的 rcutils_string_map_t (rcutils_string_map_t to be finalized)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_map_fini(rcutils_string_map_t * string_map);

/// 获取字符串映射的当前容量。 (Get the current capacity of the string map.)
/**
 * 此函数将返回映射的内部容量，即映射最多可以容纳的键值对数。 (This function will return the internal capacity of the map, which is the)
 * 最大的键值对数。 (maximum number of key value pairs the map could hold.)
 * 容量可以使用 rcutils_string_map_init() 或 rcutils_string_map_reserve() 初始化设置。 (The capacity can be set initially with rcutils_string_map_init() or)
 * 使用 rcutils_string_map_reserve() 进行设置。 (with rcutils_string_map_reserve().)
 * 容量并不表示存储在映射中的键值对数，rcutils_string_map_get_size() 函数可以提供这个信息。 (The capacity does not indicate how many key value paris are stored in the)
 * 映射，rcutils_string_map_get_size() 函数可以提供该信息。 (map, the rcutils_string_map_get_size() function can provide that.)
 *
 * \param[in] string_map 要查询的 rcutils_string_map_t (rcutils_string_map_t to be queried)
 * \param[out] capacity 字符串映射的容量 (capacity of the string map)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_STRING_MAP_INVALID 如果字符串映射无效，或 (if the string map is invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_string_map_get_capacity(const rcutils_string_map_t * string_map, size_t * capacity);

/// 获取字符串映射的当前大小。 (Get the current size of the string map.)
/**
 * 此函数将返回映射的内部大小，即映射中当前键值对数。 (This function will return the internal size of the map, which is the)
 * 当前的键值对数。 (current number of key value pairs in the map.)
 * 调用 rcutils_string_map_set_no_resize()、rcutils_string_map_set() 或 rcutils_string_map_unset() 时，大小会发生变化。 (The size is changed when calling rcutils_string_map_set_no_resize(),)
 * 调用 rcutils_string_map_set() 或 rcutils_string_map_unset() 时，大小会发生变化。 (rcutils_string_map_set(), or rcutils_string_map_unset().)
 *
 * \param[in] string_map 要查询的 rcutils_string_map_t (rcutils_string_map_t to be queried)
 * \param[out] size 字符串映射的大小 (size of the string map)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_STRING_MAP_INVALID 如果字符串映射无效，或 (if the string map is invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_map_get_size(const rcutils_string_map_t * string_map, size_t * size);

/// 为映射中预留给定数量的容量。 (Reserve a given amount of capacity in the map.)
/**
 * 将映射的容量增加到至少给定大小。 (Increases the capacity of the map to at least the given size.)
 *
 * 如果当前容量小于请求的容量，则使用在 rcutils_string_map_init() 中初始化映射时给定的分配器来增加容量。 (If the current capacity is less than requested capacity then the capacity)
 * 使用在 rcutils_string_map_init() 中初始化映射时给定的分配器来增加容量。 (is increased using the allocator given during initialization of the map in)
 * 在 rcutils_string_map_init() 中。 (rcutils_string_map_init().)
 * 如果请求的容量小于当前容量，那么容量可能会减少，但是不会因此截断任何现有的键值对。 (If the requested capacity is less than the current capacity, the capacity)
 * 可能会减少，但是不会因此截断任何现有的键值对。 (may be reduced, but no existing key value pairs will be truncated to do so.)
 * 实际上，容量将缩小以适应映射中的项目数或请求的容量，以较大者为准。 (In effect, the capacity will be shrunk to fit the number of items in map or)
 * 请求的容量，以较大者为准。 (the requested capacity, which ever is larger.)
 *
 * 如果需要回收所有资源，请先调用 rcutils_string_map_clear()，然后使用容量 0 调用此函数。 (If recovering all resources is desired first call rcutils_string_map_clear())
 * 然后使用容量 0 调用此函数。 (and then this function with a capacity of 0.)
 *
 * \param[inout] string_map 要预留空间的 rcutils_string_map_t (rcutils_string_map_t to have space reserved in)
 * \param[in] capacity 请求在映射中预留的大小 (requested size to reserve in the map)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或 (if memory allocation fails, or)
 * \return #RCUTILS_RET_STRING_MAP_INVALID 如果字符串映射无效，或 (if the string map is invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_map_reserve(rcutils_string_map_t * string_map, size_t capacity);

/// 从映射中删除所有键值对。 (Remove all key value pairs from the map.)
/**
 * 此函数将从映射中删除所有键值对，并回收由于设置键值对而分配的所有资源。 (This function will remove all key value pairs from the map, and it will)
 * 回收由于设置键值对而分配的所有资源。 (reclaim all resources allocated as a result of setting key value pairs.)
 * 此后仍应调用 rcutils_string_map_fini()。 (rcutils_string_map_fini() should still be called after this.)
 *
 * \param[inout] string_map 要清除的 rcutils_string_map_t (rcutils_string_map_t to be cleared)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_STRING_MAP_INVALID 如果字符串映射无效，或 (if the string map is invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_map_clear(rcutils_string_map_t * string_map);

/// 在映射中设置一个键值对，必要时增加容量。 (Set a key value pair in the map, increasing capacity if necessary.)
/**
 * 如果需要，将使用 rcutils_string_map_reserve() 增加容量。 (The capacity will be increased if needed using rcutils_string_map_reserve().)
 * 否则，它与 rcutils_string_map_set_no_resize() 相同。 (Otherwise it is the same as rcutils_string_map_set_no_resize().)
 *
 * \see rcutils_string_map_set_no_resize()
 *
 * \param[inout] string_map 要更新的 rcutils_string_map_t (rcutils_string_map_t to be updated)
 * \param[in] key 映射键，必须是以空终止的 c 字符串 (map key, must be null terminated c string)
 * \param[in] value 给定映射键的值，必须是以空终止的 c 字符串 (value for given map key, must be null terminated c string)
 * \return #RCUTILS_RET_OK 如果成功，或 (if successful, or)
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或 (for invalid arguments, or)
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或 (if memory allocation fails, or)
 * \return #RCUTILS_RET_STRING_MAP_INVALID 如果字符串映射无效，或 (if the string map is invalid, or)
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。 (if an unknown error occurs.)
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t
rcutils_string_map_set(rcutils_string_map_t * string_map, const char * key, const char * value);

/// 设置映射中的键值对，但仅在映射具有足够容量时设置。
/// Set a key value pair in the map but only if the map has enough capacity.
/**
 * 如果映射已包含给定的键，则现有值将被替换为给定值。
 * If the map already contains the given key, the existing value will be
 * replaced with the given value.
 * 如果映射不包含给定的键，并且映射具有额外的未使用容量，
 * 则它将在映射中存储给定的键和值。
 * If the map does not contain the given key, and the map has additional
 * unused capacity, then it will store the given key and value in the map.
 * 如果映射中没有未使用的容量，则返回 RCUTILS_RET_NOT_ENOUGH_SPACE。
 * If there is no unused capacity in the map, then RCUTILS_RET_NOT_ENOUGH_SPACE
 * is returned.
 *
 * 将给定的键和值 c 字符串复制到映射中，因此在必要时调用此函数时会为它们在映射中分配存储空间。
 * The given key and value c strings are copied into the map, and so storage is
 * allocated for them in the map when this function is called if necessary.
 * 在调用 rcutils_string_map_fini() 释放此映射或使用此函数或 rcutils_string_map_unset() 时，回收为此目的分配的存储空间。
 * The storage allocated for this purpose is reclaimed either when
 * rcutils_string_map_fini() is called on this map or when using this function
 * or rcutils_string_map_unset().
 *
 * 本函数中发生的任何分配都使用映射的分配器，该分配器在 rcutils_string_map_init() 中初始化映射时给出。
 * Any allocation that occurs in this functions uses the allocator of the map,
 * which is given when the map is initialized in rcutils_string_map_init().
 *
 * \param[inout] string_map 要更新的 rcutils_string_map_t
 * \param[inout] string_map rcutils_string_map_t to be updated
 * \param[in] key 映射键，必须是以 null 结尾的 c 字符串
 * \param[in] key map key, must be null terminated c string
 * \param[in] value 给定映射键的值，必须是以 null 结尾的 c 字符串
 * \param[in] value value for given map key, must be null terminated c string
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_BAD_ALLOC 如果内存分配失败，或
 * \return #RCUTILS_RET_BAD_ALLOC if memory allocation fails, or
 * \return #RCUTILS_RET_STRING_MAP_INVALID 如果字符串映射无效，或
 * \return #RCUTILS_RET_STRING_MAP_INVALID if the string map is invalid, or
 * \return #RCUTILS_RET_NOT_ENOUGH_SPACE 如果映射已满，或
 * \return #RCUTILS_RET_NOT_ENOUGH_SPACE if map is full, or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_map_set_no_resize(
  rcutils_string_map_t * string_map, const char * key, const char * value);

/// 在映射中取消设置键值对。
/// Unset a key value pair in the map.
/**
 * 键需要是以 null 结尾的 c 字符串。
 * The key needs to be a null terminated c string.
 * 如果找不到给定的键，则返回 RCUTILS_RET_STRING_KEY_NOT_FOUND。
 * If the given key is not found, RCUTILS_RET_STRING_KEY_NOT_FOUND is returned.
 *
 * \param[inout] string_map 要更新的 rcutils_string_map_t
 * \param[inout] string_map rcutils_string_map_t to be updated
 * \param[in] key 映射键，必须是以 null 结尾的 c 字符串
 * \param[in] key map key, must be null terminated c string
 * \return #RCUTILS_RET_OK 如果成功，或
 * \return #RCUTILS_RET_OK if successful, or
 * \return #RCUTILS_RET_INVALID_ARGUMENT 对于无效参数，或
 * \return #RCUTILS_RET_INVALID_ARGUMENT for invalid arguments, or
 * \return #RCUTILS_RET_STRING_MAP_INVALID 如果字符串映射无效，或
 * \return #RCUTILS_RET_STRING_MAP_INVALID if the string map is invalid, or
 * \return #RCUTILS_RET_STRING_KEY_NOT_FOUND 如果找不到键，或
 * \return #RCUTILS_RET_STRING_KEY_NOT_FOUND if key not found, or
 * \return #RCUTILS_RET_ERROR 如果发生未知错误。
 * \return #RCUTILS_RET_ERROR if an unknown error occurs.
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_map_unset(rcutils_string_map_t * string_map, const char * key);

/// 获取 key 是否存在。
/// Get whether or not a key exists.
/**
 * 键需要是一个以空字符结尾的 c 字符串。
 * The key needs to be a null terminated c string.
 *
 * 此函数可能失败并返回 false，如果未找到键，
 * 或者 string_map 为空或无效，或者键为 NULL。
 * 在所有情况下都不会设置错误消息。
 * This function can fail and return false if the key is not found,
 * or the string_map is NULL or invalid, or if the key is NULL.
 * In all cases no error message is set.
 *
 * \param[in] string_map 要搜索的 rcutils_string_map_t
 * \param[in] string_map rcutils_string_map_t to be searched
 * \param[in] key 映射键，必须是以空字符结尾的 c 字符串
 * \param[in] key map key, must be null terminated c string
 * \return 若键在映射中，则返回 `true`
 * \return `true` if key is in the map, or
 * \return 若键不在映射中，则返回 `false`
 * \return `false` if key is not in the map, or
 * \return 若参数无效，则返回 `false`
 * \return `false` for invalid arguments, or
 * \return 若字符串映射无效，则返回 `false`
 * \return `false` if the string map is invalid.
 */
RCUTILS_PUBLIC
bool rcutils_string_map_key_exists(const rcutils_string_map_t * string_map, const char * key);

/// 获取已知长度的键是否存在。 (Get whether or not a key of known length exists)
/**
 * 与 rcutils_string_map_key_exists() 相同，但不依赖于键是以空字符结尾的 C 字符串。 (Identical to rcutils_string_map_key_exists() but without relying on key to be a null terminated c string.)
 *
 * \param[in] string_map 要搜索的 rcutils_string_map_t (rcutils_string_map_t to be searched)
 * \param[in] key 映射键 (map key)
 * \param[in] key_length 映射键长度 (map key length)
 * \return 如果键在映射中，则返回 `true`，或 ( `true` if key is in the map, or)
 * \return 如果键不在映射中，则返回 `false`，或 ( `false` if key is not in the map, or)
 * \return 对于无效参数返回 `false`，或 ( `false` for invalid arguments, or)
 * \return 如果字符串映射无效，则返回 `false`。 ( `false` if the string map is invalid.)
 */
RCUTILS_PUBLIC
bool rcutils_string_map_key_existsn(
  const rcutils_string_map_t * string_map, const char * key, size_t key_length);
/// 获取给定键的值。 (Get value given a key)
/**
 * 键需要是以空字符结尾的 C 字符串。 (The key needs to be a null terminated c string.)
 *
 * 此函数可能会失败，因此返回 NULL，如果找不到键、string_map 为 NULL 或无效，或者键为 NULL。 (This function can fail, and therefore return NULL, if the key is not found, or the string_map is NULL or invalid, or if the key is NULL. In all cases no error message is set.)
 *
 * 返回的值字符串仍由映射拥有，不应修改或释放。 (The returned value string is still owned by the map, and it should not be modified or free'd.)
 * 这也意味着如果调用 rcutils_string_map_clear() 或 rcutils_string_map_fini()，或者使用 rcutils_string_map_set()、rcutils_string_map_set_no_resize() 或 rcutils_string_map_unset() 更新或删除键值对，值指针将变为无效。 (This also means that the value pointer becomes invalid if either rcutils_string_map_clear() or rcutils_string_map_fini() are called or if the key value pair is updated or removed with one of rcutils_string_map_set() or rcutils_string_map_set_no_resize() or rcutils_string_map_unset().)
 *
 * \param[in] string_map 要搜索的 rcutils_string_map_t (rcutils_string_map_t to be searched)
 * \param[in] key 映射键，必须是以空字符结尾的 C 字符串 (map key, must be null terminated c string)
 * \return 如果成功，则返回给定键的值，或 (value for the given key if successful, or)
 * \return 对于无效参数返回 `NULL`，或 ( `NULL` for invalid arguments, or)
 * \return 如果字符串映射无效，则返回 `NULL`，或 ( `NULL` if the string map is invalid, or)
 * \return 如果找不到键，则返回 `NULL`，或 ( `NULL` if key not found, or)
 * \return 如果发生未知错误，则返回 `NULL`。 ( `NULL` if an unknown error occurs.)
 */
RCUTILS_PUBLIC
const char * rcutils_string_map_get(const rcutils_string_map_t * string_map, const char * key);

/// 根据给定的键和键长度获取值。 (Get value given a key and key length)
/**
 * 与 rcutils_string_map_get() 相同，但不依赖于键是以空字符结尾的 C 字符串。 (Identical to rcutils_string_map_get() but without relying on key to be a null terminated c string.)
 *
 * \param[in] string_map 要搜索的 rcutils_string_map_t (rcutils_string_map_t to be searched)
 * \param[in] key 映射键 (map key)
 * \param[in] key_length 映射键长度 (map key length)
 * \return 如果成功，则返回给定键的值，或 (value for the given key if successful, or)
 * \return 对于无效参数返回 `NULL`，或 ( `NULL` for invalid arguments, or)
 * \return 如果字符串映射无效，则返回 `NULL`，或 ( `NULL` if the string map is invalid, or)
 * \return 如果找不到键，则返回 `NULL`，或 ( `NULL` if key not found, or)
 * \return 如果发生未知错误，则返回 `NULL`。 ( `NULL` if an unknown error occurs.)
 */
RCUTILS_PUBLIC
const char * rcutils_string_map_getn(
  const rcutils_string_map_t * string_map, const char * key, size_t key_length);

/// 获取映射中的下一个键，除非给定 NULL，则获取第一个键。
/// (Get the next key in the map, unless NULL is given, then get the first key.)
/**
 * 此函数允许您迭代地获取映射中的每个键。
 * (This function allows you iteratively get each key in the map.)
 *
 * 如果为键提供 NULL，则返回映射中的第一个键。
 * (If NULL is given for the key, then the first key in the map is returned.)
 * 如果返回该键给此函数，则返回映射中的下一个键。
 * (If that returned key if given to this function, then the next key in the map is returned.)
 * 如果映射中没有更多键或者给定的键不在映射中，则返回 NULL。
 * (If there are no more keys in the map or if the given key is not in the map, NULL is returned.)
 *
 * 映射中键的顺序是任意的，如果在调用此函数之间修改了映射，则行为是未定义的。
 * (The order of the keys in the map is arbitrary and if the map is modified between calls to this function the behavior is undefined.)
 * 如果修改了映射，则应通过传递 NULL 以再次获取第一个键以开始迭代。
 * (If the map is modifeid then iteration should begin again by passing NULL to get the first key again.)
 *
 * 此函数基于指针的地址操作，您不能传递一个键的副本来获取下一个键。
 * (This function operates based on the address of the pointer, you cannot pass a copy of a key to get the next key.)
 *
 * 示例 (Example)：
 *
 * ```c
 * printf("keys in the map:\n");
 * const char * current_key = rcutils_string_map_get_next_key(&map, NULL);
 * while (current_key) {
 *   printf("  - %s\n", current_key);
 *   current_key = rcutils_string_map_get_next_key(&map, current_key);
 * }
 * ```
 *
 * 如果为 string_map 提供 NULL 或者 string_map 无效，也可以返回 NULL。
 * (NULL can also be returned if NULL is given for the string_map or if the string_map is invalid.)
 *
 * \param[in] string_map 要查询的 rcutils_string_map_t
 * \param[in] key 获取第一个键的 NULL 或获取下一个的前一个键
 * \return 成功时给定键的值，或
 * \return 对于无效参数，返回 `NULL`，或
 * \return 如果字符串映射无效，则返回 `NULL`，或
 * \return 如果找不到键，则返回 `NULL`，或
 * \return 如果映射中没有更多键，则返回 `NULL`，或
 * \return 如果发生未知错误，则返回 `NULL`。
 */
RCUTILS_PUBLIC
const char *
rcutils_string_map_get_next_key(const rcutils_string_map_t * string_map, const char * key);

/// 将一个映射中的所有键值对复制到另一个映射中，如果需要，覆盖并调整大小。
/// (Copy all the key value pairs from one map into another, overwritting and resizing if needed.)
/**
 * 如果目标字符串映射没有足够的存储空间，则将调整大小。
 * (If the destination string map does not have enough storage, then it is will be resized.)
 * 如果目标映射中存在键值对，则其值将替换为源映射的值。
 * (If a key value pair exists in the destination map, its value will be replaced with the source map's value.)
 *
 * 如果在复制过程中发生错误，例如内存分配失败，则只能复制部分值。
 * (It is possible for only some of the values to be copied if an error happens during the copying process, e.g. if memory allocation fails.)
 *
 * \param[in] src_string_map 要从中复制的 rcutils_string_map_t
 * \param[inout] dst_string_map 要复制到的 rcutils_string_map_t
 * \return 成功时返回 #RCUTILS_RET_OK，或
 * \return 对于无效参数，返回 #RCUTILS_RET_INVALID_ARGUMENT，或
 * \return 如果内存分配失败，则返回 #RCUTILS_RET_BAD_ALLOC，或
 * \return 如果字符串映射无效，则返回 #RCUTILS_RET_STRING_MAP_INVALID，或
 * \return 如果发生未知错误，则返回 #RCUTILS_RET_ERROR。
 */
RCUTILS_PUBLIC
RCUTILS_WARN_UNUSED
rcutils_ret_t rcutils_string_map_copy(
  const rcutils_string_map_t * src_string_map, rcutils_string_map_t * dst_string_map);

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__TYPES__STRING_MAP_H_
