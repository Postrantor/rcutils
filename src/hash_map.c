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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rcutils/macros.h"
#include "rcutils/types/array_list.h"
#include "rcutils/types/hash_map.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

#define LOAD_FACTOR (0.75)
#define BUCKET_INITIAL_CAP ((size_t)2)

/**
 * @brief 哈希表中的一个条目结构体 (Entry structure for a hash map)
 */
typedef struct rcutils_hash_map_entry_s
{
  size_t hashed_key; ///< 已哈希的键值 (Hashed key value)
  void * key;        ///< 键指针 (Pointer to the key)
  void * value;      ///< 值指针 (Pointer to the value)
} rcutils_hash_map_entry_t;

/**
 * @brief 哈希表实现结构体 (Hash map implementation structure)
 */
typedef struct rcutils_hash_map_impl_s
{
  // 这是用来存储键值对的桶数组 (This is the array of buckets that will store the keypairs)
  rcutils_array_list_t * map;
  size_t capacity;                                ///< 容量 (Capacity)
  size_t size;                                    ///< 大小 (Size)
  size_t key_size;                                ///< 键大小 (Key size)
  size_t data_size;                               ///< 数据大小 (Data size)
  rcutils_hash_map_key_hasher_t key_hashing_func; ///< 键哈希函数 (Key hashing function)
  rcutils_hash_map_key_cmp_t key_cmp_func;        ///< 键比较函数 (Key comparison function)
  rcutils_allocator_t allocator;                  ///< 分配器 (Allocator)
} rcutils_hash_map_impl_t;

/**
 * @brief djb2 字符串哈希函数 (djb2 string hash function)
 *
 * @param[in] key_str 要哈希的字符串指针 (Pointer to the string to be hashed)
 * @return 哈希值 (Hash value)
 */
size_t rcutils_hash_map_string_hash_func(const void * key_str)
{
  const char ** ckey_ptr = (const char **)key_str;
  const char * ckey_str = *ckey_ptr;
  size_t hash = 5381;

  // 循环计算哈希值 (Calculate the hash value in a loop)
  while ('\0' != *ckey_str) {
    const char c = *(ckey_str++);
    hash = ((hash << 5) + hash) + (size_t)c; /* hash * 33 + c */
  }

  return hash;
}

/**
 * @brief 字符串比较函数 (String comparison function)
 *
 * @param[in] val1 字符串1指针 (Pointer to string 1)
 * @param[in] val2 字符串2指针 (Pointer to string 2)
 * @return 比较结果，相等返回0，不等返回非0值 (Comparison result, 0 if equal, non-zero if not equal)
 */
int rcutils_hash_map_string_cmp_func(const void * val1, const void * val2)
{
  const char ** cval1 = (const char **)val1;
  const char ** cval2 = (const char **)val2;
  return strcmp(*cval1, *cval2);
}

/**
 * @brief 获取一个零初始化的哈希表 (Get a zero-initialized hash map)
 *
 * @return 零初始化的哈希表结构体 (Zero-initialized hash map structure)
 */
rcutils_hash_map_t rcutils_get_zero_initialized_hash_map()
{
  static rcutils_hash_map_t zero_initialized_hash_map = {NULL};
  return zero_initialized_hash_map;
}

/**
 * @brief 为新的零初始化哈希映射分配内存 (Allocates a new zero initialized hashmap)
 *
 * @param[out] map 指向要分配的哈希映射的指针的指针 (Pointer to the pointer of the hashmap to be allocated)
 * @param[in] capacity 哈希映射的容量 (Capacity of the hashmap)
 * @param[in] allocator 分配器，用于分配内存 (Allocator used for memory allocation)
 * @return rcutils_ret_t 返回操作状态 (Return the operation status)
 */
static rcutils_ret_t hash_map_allocate_new_map(
  rcutils_array_list_t ** map, size_t capacity, const rcutils_allocator_t * allocator)
{
  // 分配哈希映射所需的内存 (Allocate the memory required for the hashmap)
  *map = allocator->allocate(capacity * sizeof(rcutils_hash_map_impl_t), allocator->state);
  if (NULL == *map) {
    // 如果内存分配失败，返回错误状态 (If memory allocation fails, return error status)
    return RCUTILS_RET_BAD_ALLOC;
  }

  // 确保每个桶的列表都是零初始化的 (Make sure the list for every bucket is zero initialized)
  rcutils_array_list_t zero_list = rcutils_get_zero_initialized_array_list();
  for (size_t i = 0; i < capacity; ++i) {
    (*map)[i] = zero_list;
  }

  // 返回操作成功状态 (Return operation success status)
  return RCUTILS_RET_OK;
}

/**
 * @brief 释放映射条目的内存 (Deallocates the memory for a map entry)
 *
 * @param[in] allocator 分配器，用于释放内存 (Allocator used for memory deallocation)
 * @param[in,out] entry 要释放的哈希映射条目指针 (Pointer to the hashmap entry to be deallocated)
 */
static void
hash_map_deallocate_entry(rcutils_allocator_t * allocator, rcutils_hash_map_entry_t * entry)
{
  if (NULL != entry) {
    // 释放键的内存 (Deallocate the memory for the key)
    allocator->deallocate(entry->key, allocator->state);
    // 释放值的内存 (Deallocate the memory for the value)
    allocator->deallocate(entry->value, allocator->state);
    // 释放条目的内存 (Deallocate the memory for the entry)
    allocator->deallocate(entry, allocator->state);
  }
}

/**
 * @brief 释放现有的哈希映射 (Deallocates an existing hashmap)
 *
 * @param map 哈希映射的指针 (Pointer to the hash map)
 * @param capacity 哈希映射的容量 (Capacity of the hash map)
 * @param allocator 分配器的指针 (Pointer to the allocator)
 * @param dealloc_map_entries 是否释放映射条目内存 (Whether to deallocate the memory for map entries as well)
 * @return rcutils_ret_t 返回操作的结果状态 (Return the result status of the operation)
 */
static rcutils_ret_t hash_map_deallocate_map(
  rcutils_array_list_t * map,
  size_t capacity,
  rcutils_allocator_t * allocator,
  bool dealloc_map_entries)
{
  // 初始化返回值为成功 (Initialize return value as success)
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // 遍历哈希映射中的所有桶 (Iterate through all buckets in the hash map)
  for (size_t i = 0; i < capacity && RCUTILS_RET_OK == ret; ++i) {
    // 如果此桶已分配数组列表，则清理它 (If an array list is allocated for this bucket then clean it up)
    rcutils_array_list_t * bucket = &(map[i]);
    if (NULL != bucket->impl) {
      // 如果选择了dealloc_map_entries，则还要释放实际条目的内存 (if dealloc_map_entries is selected, then deallocate the memory for the actual entries as well)
      if (dealloc_map_entries) {
        size_t bucket_size = 0;
        ret = rcutils_array_list_get_size(bucket, &bucket_size);

        // 遍历桶中的所有条目 (Iterate through all entries in the bucket)
        for (size_t b_i = 0; b_i < bucket_size && RCUTILS_RET_OK == ret; ++b_i) {
          rcutils_hash_map_entry_t * entry;
          ret = rcutils_array_list_get(bucket, b_i, &entry);
          if (RCUTILS_RET_OK == ret) {
            // 释放条目内存 (Deallocate entry memory)
            hash_map_deallocate_entry(allocator, entry);
          }
        }
      }

      // 清理实际的列表本身 (Cleanup the actual list itself)
      if (RCUTILS_RET_OK == ret) {
        ret = rcutils_array_list_fini(bucket);
      }
    }
  }

  // 清理为桶数组分配的内存 (Cleanup the memory allocated for the array of buckets)
  if (RCUTILS_RET_OK == ret) {
    allocator->deallocate(map, allocator->state);
  }

  return ret;
}

/**
 * @brief Inserts a new entry into a bucket.
 * 插入一个新条目到指定的桶中。
 *
 * @param[in,out] map Pointer to the array list representing the hash map.
 *                    指向表示哈希映射的数组列表的指针。
 * @param[in] bucket_index The index of the bucket where the entry should be inserted.
 *                         应插入条目的桶的索引。
 * @param[in] entry Pointer to the entry to be inserted.
 *                  要插入的条目的指针。
 * @param[in] allocator Pointer to the rcutils_allocator_t object used for memory management.
 *                      用于内存管理的rcutils_allocator_t对象的指针。
 * @return RCUTILS_RET_OK if successful, or an error code indicating the failure reason.
 *         如果成功，则返回RCUTILS_RET_OK，否则返回表示失败原因的错误代码。
 */
static rcutils_ret_t hash_map_insert_entry(
  rcutils_array_list_t * map,
  size_t bucket_index,
  const rcutils_hash_map_entry_t * entry,
  rcutils_allocator_t * allocator)
{
  // Get the pointer to the specified bucket.
  // 获取指向指定桶的指针。
  rcutils_array_list_t * bucket = &(map[bucket_index]);
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // If we have not initialized this bucket yet, then do so.
  // 如果我们还没有初始化这个桶，那么就初始化它。
  if (NULL == bucket->impl) {
    ret = rcutils_array_list_init(
      bucket, BUCKET_INITIAL_CAP, sizeof(rcutils_hash_map_entry_t *), allocator);
  }

  // If the bucket initialization was successful, add the entry to the bucket.
  // 如果桶初始化成功，将条目添加到桶中。
  if (RCUTILS_RET_OK == ret) {
    ret = rcutils_array_list_add(bucket, &entry);
  }

  // Return the result of the operation.
  // 返回操作的结果。
  return ret;
}

/**
 * @brief 检查哈希映射是否已经超过负载因子并在必要时增长它 (Check if the hash map is already past its load factor and grow it if necessary)
 *
 * @param[in] hash_map 要检查和增长的哈希映射指针 (Pointer to the hash map to be checked and grown)
 * @return rcutils_ret_t 返回操作的结果 (Return the result of the operation)
 */
static rcutils_ret_t hash_map_check_and_grow_map(rcutils_hash_map_t * hash_map)
{
  // 初始化返回值为成功 (Initialize return value as successful)
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // 检查哈希映射的大小是否大于等于负载因子乘以容量 (Check if the size of the hash map is greater than or equal to the load factor multiplied by the capacity)
  if (hash_map->impl->size >= (size_t)(LOAD_FACTOR * (double)hash_map->impl->capacity)) {
    // 计算新的容量 (Calculate the new capacity)
    size_t new_capacity = 2 * hash_map->impl->capacity;
    // 新映射的指针 (Pointer for the new map)
    rcutils_array_list_t * new_map = NULL;

    // 分配新映射 (Allocate the new map)
    ret = hash_map_allocate_new_map(&new_map, new_capacity, &hash_map->impl->allocator);
    if (RCUTILS_RET_OK != ret) {
      return ret;
    }

    // 遍历旧映射中的所有元素，并将其插入新映射 (Iterate through all elements in the old map and insert them into the new map)
    for (size_t map_index = 0; map_index < hash_map->impl->capacity && RCUTILS_RET_OK == ret;
         ++map_index) {
      // 获取当前桶 (Get the current bucket)
      rcutils_array_list_t * bucket = &(hash_map->impl->map[map_index]);
      // 检查当前桶是否有效且包含条目 (Check if the current bucket is valid and has entries)
      if (NULL != bucket->impl) {
        // 获取桶的大小 (Get the size of the bucket)
        size_t bucket_size = 0;
        ret = rcutils_array_list_get_size(bucket, &bucket_size);
        if (RCUTILS_RET_OK != ret) {
          return ret;
        }

        // 遍历桶中的所有条目并将其插入新映射 (Iterate through all entries in the bucket and insert them into the new map)
        for (size_t bucket_index = 0; bucket_index < bucket_size && RCUTILS_RET_OK == ret;
             ++bucket_index) {
          // 当前条目的指针 (Pointer for the current entry)
          rcutils_hash_map_entry_t * entry = NULL;
          // 获取当前条目 (Get the current entry)
          ret = rcutils_array_list_get(bucket, bucket_index, &entry);
          if (RCUTILS_RET_OK == ret) {
            // 计算新映射中的索引 (Calculate the index in the new map)
            size_t new_index = entry->hashed_key % new_capacity;
            // 将条目插入新映射 (Insert the entry into the new map)
            ret = hash_map_insert_entry(new_map, new_index, entry, &hash_map->impl->allocator);
          }
        }
      }
    }

    // 如果在分配新映射后出现问题，请尝试清理它 (If something went wrong after we allocated the new map, try to clean it up)
    if (RCUTILS_RET_OK != ret) {
      hash_map_deallocate_map(new_map, new_capacity, &hash_map->impl->allocator, false);
      return ret;
    }

    // 清理旧映射并替换为新映射 (Cleanup the old map and swap in the new one)
    ret = hash_map_deallocate_map(
      hash_map->impl->map, hash_map->impl->capacity, &hash_map->impl->allocator, false);
    // 到此为止，一切都正常，所以即使我们无法释放旧映射，也要设置新映射 (Everything worked up to this point, so even if we fail to dealloc the old map, still set the new one)
    hash_map->impl->map = new_map;
    hash_map->impl->capacity = new_capacity;
  }

  // 返回操作结果 (Return the operation result)
  return ret;
}

/**
 * @brief 计算大于等于给定值的下一个2的幂次方数
 * Calculate the next power of two greater than or equal to the given value.
 *
 * @param[in] v 输入值 (Input value)
 * @return 大于等于输入值的下一个2的幂次方数 (The next power of two greater than or equal to the input value)
 */
static size_t next_power_of_two(size_t v)
{
  // 初始化右移位数为0
  // Initialize the right shift count to 0
  size_t shf = 0;

  // 将输入值减1，以便在后续操作中正确计算2的幂次方数
  // Decrement the input value by 1 to correctly calculate the power of two in subsequent operations
  v--;

  // 循环遍历，每次右移位数翻倍，直到达到size_t类型的一半位数
  // Loop through, doubling the number of right shifts each time, until half the number of bits in size_t are reached
  for (size_t i = 0; shf < sizeof(size_t) * 4; ++i) {
    shf = (((size_t)1) << i);
    v |= v >> shf;
  }

  // 将结果加1，得到大于等于原始输入值的2的幂次方数
  // Increment the result by 1 to obtain the power of two greater than or equal to the original input value
  v++;

  // 如果结果大于1，则返回结果；否则返回1
  // Return the result if it is greater than 1, otherwise return 1
  return v > 1 ? v : 1;
}

/**
 * @brief 初始化哈希映射
 * Initialize the hash map.
 *
 * @param[out] hash_map 指向待初始化的哈希映射指针 (Pointer to the hash map to be initialized)
 * @param[in] initial_capacity 初始容量 (Initial capacity)
 * @param[in] key_size 键大小 (Key size)
 * @param[in] data_size 数据大小 (Data size)
 * @param[in] key_hashing_func 键哈希函数 (Key hashing function)
 * @param[in] key_cmp_func 键比较函数 (Key comparison function)
 * @param[in] allocator 分配器 (Allocator)
 * @return 成功或错误代码 (Success or error code)
 */
rcutils_ret_t rcutils_hash_map_init(
  rcutils_hash_map_t * hash_map,
  size_t initial_capacity,
  size_t key_size,
  size_t data_size,
  rcutils_hash_map_key_hasher_t key_hashing_func,
  rcutils_hash_map_key_cmp_t key_cmp_func,
  const rcutils_allocator_t * allocator)
{
  // 检查输入参数是否为NULL
  // Check if input arguments are NULL
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(hash_map, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key_hashing_func, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key_cmp_func, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(allocator, return RCUTILS_RET_INVALID_ARGUMENT);

  // 检查初始容量、键大小和数据大小是否大于等于1
  // Check if initial capacity, key size, and data size are greater than or equal to 1
  if (1 > initial_capacity) {
    RCUTILS_SET_ERROR_MSG("initial_capacity cannot be less than 1");
    return RCUTILS_RET_INVALID_ARGUMENT;
  } else if (1 > key_size) {
    RCUTILS_SET_ERROR_MSG("key_size cannot be less than 1");
    return RCUTILS_RET_INVALID_ARGUMENT;
  } else if (1 > data_size) {
    RCUTILS_SET_ERROR_MSG("data_size cannot be less than 1");
    return RCUTILS_RET_INVALID_ARGUMENT;
  }

  // 由于在查找时使用了优化，容量必须是2的幂次方数
  // 如果用户传入的值不是2的幂次方数，则向上取整到下一个2的幂次方数
  // Due to an optimization used during lookup, the capacity must be a power of two
  // If the user passed a non-power-of-two value, round up to the next power of two
  if ((initial_capacity & (initial_capacity - 1)) != 0) {
    initial_capacity = next_power_of_two(initial_capacity);
  }

  // 分配哈希映射实现结构体内存空间
  // Allocate memory for the hash map implementation structure
  hash_map->impl = allocator->allocate(sizeof(rcutils_hash_map_impl_t), allocator->state);
  if (NULL == hash_map->impl) {
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for hash map impl");
    return RCUTILS_RET_BAD_ALLOC;
  }

  // 初始化哈希映射实现结构体成员
  // Initialize the members of the hash map implementation structure
  hash_map->impl->capacity = initial_capacity;
  hash_map->impl->size = 0;
  hash_map->impl->key_size = key_size;
  hash_map->impl->data_size = data_size;
  hash_map->impl->key_hashing_func = key_hashing_func;
  hash_map->impl->key_cmp_func = key_cmp_func;

  // 分配新哈希映射内存空间
  // Allocate memory for the new hash map
  rcutils_ret_t ret = hash_map_allocate_new_map(&hash_map->impl->map, initial_capacity, allocator);
  if (RCUTILS_RET_OK != ret) {
    // 在返回失败前清理已分配的内存
    // Clean up allocated memory before returning failure
    allocator->deallocate(hash_map->impl, allocator->state);
    hash_map->impl = NULL;
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for map data");
    return ret;
  }

  // 设置分配器
  // Set the allocator
  hash_map->impl->allocator = *allocator;

  return RCUTILS_RET_OK;
}

/**
 * @brief 销毁一个哈希映射 (Destroy a hash map)
 *
 * @param[in] hash_map 要销毁的哈希映射指针 (Pointer to the hash map to be destroyed)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_hash_map_fini(rcutils_hash_map_t * hash_map)
{
  // 验证传入的哈希映射是否有效 (Verify if the passed hash_map is valid)
  HASH_MAP_VALIDATE_HASH_MAP(hash_map);

  // 释放哈希映射中的内存 (Deallocate memory in the hash map)
  rcutils_ret_t ret = hash_map_deallocate_map(
    hash_map->impl->map, hash_map->impl->capacity, &hash_map->impl->allocator, true);

  // 如果释放成功，将实现指针设为 NULL (If deallocation is successful, set the implementation pointer to NULL)
  if (RCUTILS_RET_OK == ret) {
    hash_map->impl->allocator.deallocate(hash_map->impl, hash_map->impl->allocator.state);
    hash_map->impl = NULL;
  }

  return ret;
}

/**
 * @brief 获取哈希映射的容量 (Get the capacity of the hash map)
 *
 * @param[in] hash_map 要查询容量的哈希映射指针 (Pointer to the hash map to query its capacity)
 * @param[out] capacity 存储哈希映射容量的指针 (Pointer to store the capacity of the hash map)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_hash_map_get_capacity(const rcutils_hash_map_t * hash_map, size_t * capacity)
{
  // 验证传入的哈希映射是否有效 (Verify if the passed hash_map is valid)
  HASH_MAP_VALIDATE_HASH_MAP(hash_map);

  // 检查参数 capacity 是否为 NULL (Check if the capacity parameter is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(capacity, RCUTILS_RET_INVALID_ARGUMENT);

  // 获取哈希映射的容量并存储到 capacity 中 (Get the capacity of the hash map and store it in capacity)
  *capacity = hash_map->impl->capacity;

  return RCUTILS_RET_OK;
}

/**
 * @brief 获取哈希映射的大小 (Get the size of the hash map)
 *
 * @param[in] hash_map 要查询大小的哈希映射指针 (Pointer to the hash map to query its size)
 * @param[out] size 存储哈希映射大小的指针 (Pointer to store the size of the hash map)
 * @return rcutils_ret_t 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_hash_map_get_size(const rcutils_hash_map_t * hash_map, size_t * size)
{
  // 验证传入的哈希映射是否有效 (Verify if the passed hash_map is valid)
  HASH_MAP_VALIDATE_HASH_MAP(hash_map);

  // 检查参数 size 是否为 NULL (Check if the size parameter is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(size, RCUTILS_RET_INVALID_ARGUMENT);

  // 获取哈希映射的大小并存储到 size 中 (Get the size of the hash map and store it in size)
  *size = hash_map->impl->size;

  return RCUTILS_RET_OK;
}

/**
 * @brief 查找哈希映射中是否存在给定的键 (Find if the given key exists in the hash map)
 *
 * @param[in] hash_map 要在其中查找的哈希映射 (The hash_map to look up in)
 * @param[in] key 要查找的键 (The key to lookup)
 * @param[out] key_hash 键的哈希值 (The key's hashed value)
 * @param[out] map_index 数组桶的索引 (The index into the array of buckets)
 * @param[out] bucket_index 条目在其桶中的索引 (The index of the entry in its bucket)
 * @param[out] entry 将设置为指向条目数据的指针 (Will be set to a pointer to the entry's data)
 * @return 如果找到则返回true，否则返回false (Returns true if found or false if it doesn't exist)
 */
static bool hash_map_find(
  const rcutils_hash_map_t * hash_map,
  const void * key,
  size_t * key_hash,
  size_t * map_index,
  size_t * bucket_index,
  rcutils_hash_map_entry_t ** entry)
{
  // 存储桶的大小 (Size of the bucket)
  size_t bucket_size = 0;
  // 指向桶条目的指针 (Pointer to the bucket entry)
  rcutils_hash_map_entry_t * bucket_entry = NULL;

  // 计算键的哈希值 (Calculate the hash value of the key)
  *key_hash = hash_map->impl->key_hashing_func(key);
  // 下面等价于：(*map_index = (*key_hash) % hash_map->impl->capacity;)
  // 这种实现更快，因为它避免了除法，但只有当容量是2的幂时才有效。我们在 rcutils_hash_map_init() 函数中强制执行此操作。
  // (The below is equivalent to: *map_index = (*key_hash) % hash_map->impl->capacity;
  // This implementation is significantly faster since it avoids a divide, but
  // only works when the capacity is a power of two. We enforce that in the
  // rcutils_hash_map_init() function.)
  *map_index = (*key_hash) & (hash_map->impl->capacity - 1);

  // 查找条目所在的桶，并检查其是否有效 (Find the bucket the entry should be in and check that it's valid)
  rcutils_array_list_t * bucket = &(hash_map->impl->map[*map_index]);
  if (NULL == bucket->impl) {
    return false;
  }
  if (RCUTILS_RET_OK != rcutils_array_list_get_size(bucket, &bucket_size)) {
    return false;
  }
  for (size_t i = 0; i < bucket_size; ++i) {
    if (RCUTILS_RET_OK != rcutils_array_list_get(bucket, i, &bucket_entry)) {
      return false;
    }
    // 首先检查哈希是否匹配，因为这将是更快的比较以快速失败
    // (Check that the hashes match first as that will be the quicker comparison to quick fail on)
    if (
      bucket_entry->hashed_key == *key_hash &&
      (0 == hash_map->impl->key_cmp_func(bucket_entry->key, key))) {
      *bucket_index = i;
      *entry = bucket_entry;
      return true;
    }
  }

  return false;
}

/**
 * @brief 将键值对插入到哈希映射中 (Insert a key-value pair into the hash map)
 *
 * @param[in] hash_map 指向 rcutils_hash_map_t 结构体的指针 (Pointer to an rcutils_hash_map_t structure)
 * @param[in] key 需要插入的键 (The key to be inserted)
 * @param[in] value 与键关联的值 (The value associated with the key)
 * @return 返回 RCUTILS_RET_OK，如果成功插入或更新键值对；否则返回相应的错误代码 (Returns RCUTILS_RET_OK if the key-value pair is successfully inserted or updated, otherwise returns the corresponding error code)
 */
rcutils_ret_t
rcutils_hash_map_set(rcutils_hash_map_t * hash_map, const void * key, const void * value)
{
  // 验证传入的 hash_map 是否有效 (Validate whether the input hash_map is valid)
  HASH_MAP_VALIDATE_HASH_MAP(hash_map);
  // 检查 key 和 value 是否为空指针，如果是，则返回无效参数错误 (Check if key and value are NULL pointers, if so, return invalid argument error)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(value, RCUTILS_RET_INVALID_ARGUMENT);

  size_t key_hash = 0, map_index = 0, bucket_index = 0;
  bool already_exists = false;
  rcutils_hash_map_entry_t * entry = NULL;
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // 查找键是否已经存在于哈希映射中 (Find if the key already exists in the hash map)
  already_exists = hash_map_find(hash_map, key, &key_hash, &map_index, &bucket_index, &entry);

  if (already_exists) {
    // 如果键已经存在，只需更新现有值以匹配新值 (If the key already exists, just update the existing value to match the new value)
    memcpy(entry->value, value, hash_map->impl->data_size);
  } else {
    // 我们需要在映射中创建一个新条目 (We need to create a new entry in the map)
    rcutils_allocator_t * allocator = &hash_map->impl->allocator;

    // 首先尝试为新条目分配所需的内存 (Start by trying to allocate the memory we need for the new entry)
    entry = allocator->allocate(sizeof(rcutils_hash_map_entry_t), allocator->state);
    if (NULL == entry) {
      return RCUTILS_RET_BAD_ALLOC;
    }
    entry->key = allocator->allocate(hash_map->impl->key_size, allocator->state);
    entry->value = allocator->allocate(hash_map->impl->data_size, allocator->state);
    if (NULL == entry->key || NULL == entry->value) {
      ret = RCUTILS_RET_BAD_ALLOC;
    }

    // 设置条目数据并尝试插入到 bucket 中 (Set the entry data and try to insert into the bucket)
    if (RCUTILS_RET_OK == ret) {
      entry->hashed_key = key_hash;
      memcpy(entry->value, value, hash_map->impl->data_size);
      memcpy(entry->key, key, hash_map->impl->key_size);

      // 查看 hash_map_find 中的注释，了解为什么要这样做 (See the comment in hash_map_find for why we do this)
      bucket_index = key_hash & (hash_map->impl->capacity - 1);
      ret = hash_map_insert_entry(hash_map->impl->map, bucket_index, entry, allocator);
    }

    if (RCUTILS_RET_OK != ret) {
      // 如果某个地方出现问题，则清理我们分配的内存 (If something went wrong somewhere, clean up the memory we've allocated)
      hash_map_deallocate_entry(allocator, entry);
      return ret;
    }
    hash_map->impl->size++;
  }

  // 检查是否超过了负载因子，如果是，则增加映射容量 (Check if we've exceeded our Load Factor and grow the map if so)
  ret = hash_map_check_and_grow_map(hash_map);
  // 如果扩展失败，只需记录此错误，因为映射可以继续以降低性能运行 (Just log on this failure because the map can continue to operate with degraded performance)
  RCUTILS_LOG_ERROR_EXPRESSION(RCUTILS_RET_OK != ret, "Failed to grow hash_map. Reason: %d", ret);

  return RCUTILS_RET_OK;
}

/**
 * @brief 从哈希映射中移除一个键值对 (Remove a key-value pair from the hash map)
 *
 * @param[in] hash_map 哈希映射指针 (A pointer to the hash map)
 * @param[in] key 要移除的键 (The key to be removed)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t rcutils_hash_map_unset(rcutils_hash_map_t * hash_map, const void * key)
{
  // 验证哈希映射是否有效 (Validate whether the hash map is valid)
  HASH_MAP_VALIDATE_HASH_MAP(hash_map);
  // 检查键是否为空 (Check if the key is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key, RCUTILS_RET_INVALID_ARGUMENT);

  size_t key_hash = 0, map_index = 0, bucket_index = 0;
  bool already_exists = false;
  rcutils_hash_map_entry_t * entry = NULL;

  // 如果哈希映射中没有任何内容，不需要计算键 (If there is nothing in the hash map, don't bother computing the key)
  if (hash_map->impl->size == 0) {
    return RCUTILS_RET_OK;
  }

  // 查找哈希映射中的键 (Find the key in the hash map)
  already_exists = hash_map_find(hash_map, key, &key_hash, &map_index, &bucket_index, &entry);

  // 如果键不存在于映射中，直接退出 (If the key doesn't exist in the map, just exit)
  if (!already_exists) {
    return RCUTILS_RET_OK;
  }

  // 从哈希桶中移除条目并释放内存 (Remove the entry from its bucket and deallocate it)
  rcutils_array_list_t * bucket = &(hash_map->impl->map[map_index]);
  if (RCUTILS_RET_OK == rcutils_array_list_remove(bucket, bucket_index)) {
    // 更新哈希映射的大小 (Update the size of the hash map)
    hash_map->impl->size--;
    // 释放已移除条目的内存 (Deallocate memory for the removed entry)
    hash_map_deallocate_entry(&hash_map->impl->allocator, entry);
  }

  return RCUTILS_RET_OK;
}

/**
 * @brief 检查哈希映射中是否存在给定的键 (Check if a key exists in the hash map)
 *
 * @param[in] hash_map 哈希映射指针 (Pointer to the hash map)
 * @param[in] key 要检查的键 (Key to check)
 * @return 如果键存在于哈希映射中，则返回 true，否则返回 false (Returns true if the key exists in the hash map, false otherwise)
 */
bool rcutils_hash_map_key_exists(const rcutils_hash_map_t * hash_map, const void * key)
{
  // 验证输入 (Verify input)
  if (NULL == hash_map) {
    return false;
  } else if (NULL == hash_map->impl) {
    return false;
  } else if (NULL == key) {
    return false;
  }

  size_t key_hash = 0, map_index = 0, bucket_index = 0;
  bool already_exists = false;
  rcutils_hash_map_entry_t * entry = NULL;

  // 如果哈希映射中没有任何内容，不要计算键 (If there is nothing in the hash map, don't bother computing the key)
  if (hash_map->impl->size == 0) {
    return RCUTILS_RET_OK;
  }

  already_exists = hash_map_find(hash_map, key, &key_hash, &map_index, &bucket_index, &entry);

  return already_exists;
}

/**
 * @brief 获取哈希映射中给定键对应的数据 (Get the data associated with a given key in the hash map)
 *
 * @param[in] hash_map 哈希映射指针 (Pointer to the hash map)
 * @param[in] key 要获取数据的键 (Key to get data for)
 * @param[out] data 存储从哈希映射获取的数据的指针 (Pointer to store the data retrieved from the hash map)
 * @return 成功时返回 RCUTILS_RET_OK，否则返回相应的错误代码 (Returns RCUTILS_RET_OK on success, appropriate error code otherwise)
 */
rcutils_ret_t
rcutils_hash_map_get(const rcutils_hash_map_t * hash_map, const void * key, void * data)
{
  // 验证哈希映射 (Validate the hash map)
  HASH_MAP_VALIDATE_HASH_MAP(hash_map);
  // 检查参数是否为空 (Check arguments for null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(data, RCUTILS_RET_INVALID_ARGUMENT);

  size_t key_hash = 0, map_index = 0, bucket_index = 0;
  bool already_exists = false;
  rcutils_hash_map_entry_t * entry = NULL;

  // 如果哈希映射中没有任何内容，不要计算键 (If there is nothing in the hash map, don't bother computing the key)
  if (hash_map->impl->size == 0) {
    return RCUTILS_RET_NOT_FOUND;
  }

  already_exists = hash_map_find(hash_map, key, &key_hash, &map_index, &bucket_index, &entry);

  if (already_exists) {
    // 将找到的值复制到提供的数据指针中 (Copy the found value into the provided data pointer)
    memcpy(data, entry->value, hash_map->impl->data_size);
    return RCUTILS_RET_OK;
  }

  return RCUTILS_RET_NOT_FOUND;
}

/**
 * @brief 获取下一个键值对（Get the next key-value pair）
 *
 * @param[in] hash_map 哈希映射的指针（Pointer to the hash map）
 * @param[in] previous_key 上一个键的指针（Pointer to the previous key）
 * @param[out] key 下一个键的指针（Pointer to the next key）
 * @param[out] data 下一个数据的指针（Pointer to the next data）
 * @return rcutils_ret_t 返回操作结果（Return operation result）
 */
rcutils_ret_t rcutils_hash_map_get_next_key_and_data(
  const rcutils_hash_map_t * hash_map, const void * previous_key, void * key, void * data)
{
  // 验证哈希映射是否有效（Validate if the hash map is valid）
  HASH_MAP_VALIDATE_HASH_MAP(hash_map);
  // 检查 key 和 data 参数是否为空（Check if key and data arguments are not null）
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(data, RCUTILS_RET_INVALID_ARGUMENT);

  size_t key_hash = 0, map_index = 0, bucket_index = 0;
  bool already_exists = false;
  rcutils_hash_map_entry_t * entry = NULL;
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // 如果哈希映射中没有任何内容，不需要计算键（If there is nothing in the hash map, don't bother computing the key）
  if (hash_map->impl->size == 0) {
    if (NULL != previous_key) {
      return RCUTILS_RET_NOT_FOUND;
    } else {
      return RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES;
    }
  }

  if (NULL != previous_key) {
    // 查找给定的 previous_key（Find the given previous_key）
    already_exists =
      hash_map_find(hash_map, previous_key, &key_hash, &map_index, &bucket_index, &entry);
    if (!already_exists) {
      return RCUTILS_RET_NOT_FOUND;
    }
    bucket_index++; // 我们希望从下一个对象开始搜索（We want to start our search from the next object）
  }

  // 遍历哈希映射（Iterate through the hash map）
  for (; map_index < hash_map->impl->capacity; ++map_index) {
    rcutils_array_list_t * bucket = &(hash_map->impl->map[map_index]);
    if (NULL != bucket->impl) {
      size_t bucket_size = 0;
      // 获取桶的大小（Get the size of the bucket）
      ret = rcutils_array_list_get_size(bucket, &bucket_size);
      if (RCUTILS_RET_OK != ret) {
        return ret;
      }

      // 检查此桶中的下一个索引是否有效，如果有效则找到下一个项目（Check if the next index in this bucket is valid and if so we've found the next item）
      if (bucket_index < bucket_size) {
        rcutils_hash_map_entry_t * bucket_entry = NULL;
        // 获取桶中的条目（Get the entry in the bucket）
        ret = rcutils_array_list_get(bucket, bucket_index, &bucket_entry);
        if (RCUTILS_RET_OK == ret) {
          // 复制键和数据（Copy the key and data）
          memcpy(key, bucket_entry->key, hash_map->impl->key_size);
          memcpy(data, bucket_entry->value, hash_map->impl->data_size);
        }

        return ret;
      }
    }
    // 在第一个桶之后，下一个条目必须位于具有条目的下一个桶的开头（After the first bucket the next entry must be at the start of the next bucket with entries）
    bucket_index = 0;
  }

  return RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES;
}

#ifdef __cplusplus
}
#endif
