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

#ifdef __cplusplus
extern "C" {
#endif

#include "rcutils/types/string_map.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "./common.h"
#include "rcutils/format_string.h"
#include "rcutils/strdup.h"
#include "rcutils/types/rcutils_ret.h"

/**
 * @struct key_value_pair
 * @brief 键值对结构体 (Key-Value pair structure)
 */
typedef struct key_value_pair
{
  char * key;   ///< 键 (Key)
  char * value; ///< 值 (Value)
} key_value_pair_t;

/**
 * @struct rcutils_string_map_impl_s
 * @brief 字符串映射实现结构体 (String map implementation structure)
 */
typedef struct rcutils_string_map_impl_s
{
  key_value_pair_t * key_value_pairs; ///< 键值对数组 (Array of key-value pairs)
  size_t capacity;                    ///< 容量 (Capacity)
  size_t size;                        ///< 大小 (Size)
  rcutils_allocator_t allocator;      ///< 分配器 (Allocator)
} rcutils_string_map_impl_t;

/**
 * @brief 获取零初始化的字符串映射 (Get a zero-initialized string map)
 * @return 返回零初始化的字符串映射 (Returns a zero-initialized string map)
 */
rcutils_string_map_t rcutils_get_zero_initialized_string_map(void)
{
  static rcutils_string_map_t zero_initialized_string_map;
  zero_initialized_string_map.impl = NULL;
  return zero_initialized_string_map;
}

/**
 * @brief 初始化字符串映射 (Initialize the string map)
 * @param[out] string_map 指向字符串映射的指针 (Pointer to the string map)
 * @param[in] initial_capacity 初始容量 (Initial capacity)
 * @param[in] allocator 分配器 (Allocator)
 * @return 返回操作结果 (Return operation result)
 */
rcutils_ret_t rcutils_string_map_init(
  rcutils_string_map_t * string_map, size_t initial_capacity, rcutils_allocator_t allocator)
{
  // 检查 string_map 是否为空 (Check if string_map is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查 string_map 是否已经初始化 (Check if string_map is already initialized)
  if (string_map->impl != NULL) {
    RCUTILS_SET_ERROR_MSG("string_map already initialized");
    return RCUTILS_RET_STRING_MAP_ALREADY_INIT;
  }

  // 检查分配器是否有效 (Check if allocator is valid)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
    &allocator, "invalid allocator", return RCUTILS_RET_INVALID_ARGUMENT)

  // 为字符串映射实现结构体分配内存 (Allocate memory for the string map implementation structure)
  string_map->impl = allocator.allocate(sizeof(rcutils_string_map_impl_t), allocator.state);

  // 检查分配的内存是否有效 (Check if allocated memory is valid)
  if (NULL == string_map->impl) {
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for string map impl struct");
    return RCUTILS_RET_BAD_ALLOC;
  }

  // 初始化字符串映射实现结构体的成员 (Initialize members of the string map implementation structure)
  string_map->impl->key_value_pairs = NULL;
  string_map->impl->capacity = 0;
  string_map->impl->size = 0;
  string_map->impl->allocator = allocator;

  // 为字符串映射预留初始容量 (Reserve initial capacity for the string map)
  rcutils_ret_t ret = rcutils_string_map_reserve(string_map, initial_capacity);

  // 检查操作结果 (Check operation result)
  if (ret != RCUTILS_RET_OK) {
    // 错误消息已经设置，清理并返回结果 (Error message is already set, clean up and return the result)
    allocator.deallocate(string_map->impl, allocator.state);
    string_map->impl = NULL;
    return ret;
  }

  // 返回操作成功 (Return operation success)
  return RCUTILS_RET_OK;
}

/**
 * @brief 销毁字符串映射 (Destroy a string map)
 *
 * @param[in,out] string_map 要销毁的字符串映射指针 (Pointer to the string map to be destroyed)
 * @return rcutils_ret_t 操作结果 (Operation result)
 */
rcutils_ret_t rcutils_string_map_fini(rcutils_string_map_t * string_map)
{
  // 检查参数是否为空 (Check if the argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);

  // 如果实现为空，则返回成功 (If the implementation is NULL, return success)
  if (NULL == string_map->impl) {
    return RCUTILS_RET_OK;
  }

  // 清除字符串映射 (Clear the string map)
  rcutils_ret_t ret = rcutils_string_map_clear(string_map);
  if (ret != RCUTILS_RET_OK) {
    // 错误消息已设置 (Error message already set)
    return ret;
  }

  // 为字符串映射保留空间 (Reserve space for the string map)
  ret = rcutils_string_map_reserve(string_map, 0);
  if (ret != RCUTILS_RET_OK) {
    // 错误消息已设置 (Error message already set)
    return ret;
  }

  // 获取分配器 (Get the allocator)
  rcutils_allocator_t allocator = string_map->impl->allocator;

  // 释放内存 (Deallocate memory)
  allocator.deallocate(string_map->impl, allocator.state);
  string_map->impl = NULL;

  // 返回操作结果 (Return operation result)
  return RCUTILS_RET_OK;
}

/**
 * @brief 获取字符串映射的容量 (Get the capacity of a string map)
 *
 * @param[in] string_map 要查询的字符串映射指针 (Pointer to the string map to query)
 * @param[out] capacity 字符串映射的容量 (Capacity of the string map)
 * @return rcutils_ret_t 操作结果 (Operation result)
 */
rcutils_ret_t
rcutils_string_map_get_capacity(const rcutils_string_map_t * string_map, size_t * capacity)
{
  // 检查参数是否为空 (Check if the argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查字符串映射是否有效 (Check if the string map is valid)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_map->impl, "invalid string map", return RCUTILS_RET_STRING_MAP_INVALID);

  // 检查容量参数是否为空 (Check if the capacity argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(capacity, RCUTILS_RET_INVALID_ARGUMENT);

  // 获取容量 (Get the capacity)
  *capacity = string_map->impl->capacity;

  // 返回操作结果 (Return operation result)
  return RCUTILS_RET_OK;
}

/**
 * @brief 获取字符串映射的大小 (Get the size of a string map)
 *
 * @param[in] string_map 要查询的字符串映射指针 (Pointer to the string map to query)
 * @param[out] size 字符串映射的大小 (Size of the string map)
 * @return rcutils_ret_t 操作结果 (Operation result)
 */
rcutils_ret_t rcutils_string_map_get_size(const rcutils_string_map_t * string_map, size_t * size)
{
  // 检查参数是否为空 (Check if the argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查字符串映射是否有效 (Check if the string map is valid)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_map->impl, "invalid string map", return RCUTILS_RET_STRING_MAP_INVALID);

  // 检查大小参数是否为空 (Check if the size argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(size, RCUTILS_RET_INVALID_ARGUMENT);

  // 获取大小 (Get the size)
  *size = string_map->impl->size;

  // 返回操作结果 (Return operation result)
  return RCUTILS_RET_OK;
}

/**
 * @brief 移除指定索引处的键值对 (Remove the key-value pair at the specified index)
 *
 * @param[in] string_map_impl 字符串映射实现结构体指针 (Pointer to the string map implementation structure)
 * @param[in] index 要移除的键值对的索引 (Index of the key-value pair to be removed)
 */
static void __remove_key_and_value_at_index(rcutils_string_map_impl_t * string_map_impl, size_t index)
{
  // 获取分配器对象 (Get the allocator object)
  rcutils_allocator_t allocator = string_map_impl->allocator;

  // 释放指定索引处的键内存 (Free the memory of the key at the specified index)
  allocator.deallocate(string_map_impl->key_value_pairs[index].key, allocator.state);
  string_map_impl->key_value_pairs[index].key = NULL;

  // 释放指定索引处的值内存 (Free the memory of the value at the specified index)
  allocator.deallocate(string_map_impl->key_value_pairs[index].value, allocator.state);
  string_map_impl->key_value_pairs[index].value = NULL;

  // 减少映射的大小 (Decrease the size of the map)
  string_map_impl->size--;
}

/**
 * @brief 预留字符串映射的容量 (Reserve capacity for the string map)
 *
 * @param[in,out] string_map 字符串映射对象指针 (Pointer to the string map object)
 * @param[in] capacity 预期的容量 (Expected capacity)
 * @return rcutils_ret_t 成功或错误代码 (Success or error code)
 */
rcutils_ret_t rcutils_string_map_reserve(rcutils_string_map_t * string_map, size_t capacity)
{
  // 检查输入参数的有效性 (Check the validity of input arguments)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_map->impl, "invalid string map", return RCUTILS_RET_STRING_MAP_INVALID);

  // 获取分配器对象 (Get the allocator object)
  rcutils_allocator_t allocator = string_map->impl->allocator;

  // 如果请求的容量小于映射的大小，则短路 (Short circuit if the requested capacity is less than the size of the map)
  if (capacity < string_map->impl->size) {
    // 将容量设置为当前大小 (Set the capacity to the current size instead)
    return rcutils_string_map_reserve(string_map, string_map->impl->size);
  }

  // 如果请求的容量等于当前容量，无需执行操作 (If the requested capacity is equal to the current capacity, no action is required)
  if (capacity == string_map->impl->capacity) {
    return RCUTILS_RET_OK;
  } else if (capacity == 0) {
    // 如果请求的容量为零，请确保释放现有键和值的内存 (If the requested capacity is zero, make sure to free the memory of existing keys and values)
    // 由于上面的递归调用，这里的大小已知为0 (The size is known to be 0 here because of the recursive call above)
    allocator.deallocate(string_map->impl->key_value_pairs, allocator.state);
    string_map->impl->key_value_pairs = NULL;
  } else {
    // 如果容量非零且不同，请使用realloc增加/缩小大小 (If the capacity is non-zero and different, use realloc to increase/shrink the size)
    // 注意当指针为NULL时，realloc与malloc相同 (Note that realloc when the pointer is NULL is the same as malloc)
    // 还要注意，如果需要，realloc会缩小空间 (Also note that realloc will shrink the space if needed)

    // 确保重新分配不会溢出SIZE_MAX (Ensure that reallocate won't overflow SIZE_MAX)
    if (capacity > (SIZE_MAX / sizeof(key_value_pair_t))) {
      RCUTILS_SET_ERROR_MSG("requested capacity for string_map too large");
      return RCUTILS_RET_BAD_ALLOC;
    }

    // 调整键和值的大小，并在成功时分配结果 (Resize the keys and values, assigning the result only if it succeeds)
    key_value_pair_t * new_key_value_pairs = allocator.reallocate(
      string_map->impl->key_value_pairs, capacity * sizeof(key_value_pair_t), allocator.state);
    if (NULL == new_key_value_pairs) {
      RCUTILS_SET_ERROR_MSG("failed to allocate memory for string_map key-value pairs");
      return RCUTILS_RET_BAD_ALLOC;
    }
    string_map->impl->key_value_pairs = new_key_value_pairs;

    // 如果容量大于映射的容量，则将新内存清零 (Zero out the new memory if the capacity is greater than the map's capacity)
    if (capacity > string_map->impl->capacity) {
      for (size_t i = string_map->impl->capacity; i < capacity; ++i) {
        string_map->impl->key_value_pairs[i].key = NULL;
        string_map->impl->key_value_pairs[i].value = NULL;
      }
    }
  }

  // 更新映射的容量 (Update the map's capacity)
  string_map->impl->capacity = capacity;
  return RCUTILS_RET_OK;
}

/**
 * @brief 清空字符串映射 (Clear the string map)
 *
 * @param[inout] string_map 要清空的字符串映射指针 (Pointer to the string map to clear)
 * @return 成功返回 RCUTILS_RET_OK，否则返回相应的错误代码 (Returns RCUTILS_RET_OK on success, or an error code on failure)
 */
rcutils_ret_t rcutils_string_map_clear(rcutils_string_map_t * string_map)
{
  // 检查输入参数是否为空 (Check if input argument is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);
  // 检查字符串映射实现是否为空 (Check if string map implementation is NULL)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_map->impl, "invalid string map", return RCUTILS_RET_STRING_MAP_INVALID);

  // 遍历字符串映射中的所有键值对 (Iterate through all key-value pairs in the string map)
  for (size_t i = 0; i < string_map->impl->capacity; ++i) {
    // 如果键不为空，则删除此索引处的键和值 (If the key is not NULL, remove the key and value at this index)
    if (string_map->impl->key_value_pairs[i].key != NULL) {
      __remove_key_and_value_at_index(string_map->impl, i);
    }
  }

  return RCUTILS_RET_OK;
}

/**
 * @brief 在字符串映射中设置键值对 (Set a key-value pair in the string map)
 *
 * @param[inout] string_map 要设置键值对的字符串映射指针 (Pointer to the string map to set the key-value pair)
 * @param[in] key 要设置的键 (Key to set)
 * @param[in] value 要设置的值 (Value to set)
 * @return 成功返回 RCUTILS_RET_OK，否则返回相应的错误代码 (Returns RCUTILS_RET_OK on success, or an error code on failure)
 */
rcutils_ret_t
rcutils_string_map_set(rcutils_string_map_t * string_map, const char * key, const char * value)
{
  // 检查输入参数是否为空 (Check if input arguments are NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_map->impl, "invalid string map", return RCUTILS_RET_STRING_MAP_INVALID);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(value, RCUTILS_RET_INVALID_ARGUMENT);

  // 尝试在字符串映射中设置键值对，但不重新调整大小 (Try to set the key-value pair in the string map without resizing)
  rcutils_ret_t ret = rcutils_string_map_set_no_resize(string_map, key, value);

  // 如果失败是由于空间不足，则调整大小并重试 (If it fails due to not enough space, resize and try again)
  if (ret == RCUTILS_RET_NOT_ENOUGH_SPACE) {
    rcutils_reset_error();
    // 默认将映射容量加倍 (Default to doubling the size of the map's capacity)
    size_t new_capacity = (string_map->impl->capacity) ? 2 * string_map->impl->capacity : 1;
    // 调整字符串映射的容量 (Resize the string map's capacity)
    ret = rcutils_string_map_reserve(string_map, new_capacity);
    if (ret != RCUTILS_RET_OK) {
      // 错误消息已设置 (Error message is already set)
      return ret;
    }
    // 重试设置键值对 (Try setting the key-value pair again)
    return rcutils_string_map_set_no_resize(string_map, key, value);
  }

  return ret;
}

/*!
 * \brief 检查键是否存在于字符串映射中，若存在，则返回其索引。
 * \param[in] string_map_impl 字符串映射实现的指针。
 * \param[in] key 要查找的键。
 * \param[in] key_length 键的长度。
 * \param[out] index 如果键存在，存储其在字符串映射中的索引。
 * \return 如果键存在，则返回 true，否则返回 false。
 *
 * Check if a key exists in the string map, and if it does, return its index.
 * \param[in] string_map_impl Pointer to the string map implementation.
 * \param[in] key The key to look for.
 * \param[in] key_length Length of the key.
 * \param[out] index If the key exists, store its index in the string map.
 * \return True if the key exists, false otherwise.
 */
static bool __get_index_of_key_if_exists(
  rcutils_string_map_impl_t * string_map_impl, const char * key, size_t key_length, size_t * index)
{
  // 初始化索引变量
  // Initialize the index variable
  size_t i = 0;
  // 遍历字符串映射的容量
  // Iterate through the capacity of the string map
  for (; i < string_map_impl->capacity; ++i) {
    // 如果当前索引处的键为 NULL，则跳过此次循环
    // If the key at the current index is NULL, skip this iteration
    if (NULL == string_map_impl->key_value_pairs[i].key) {
      continue;
    }
    // 获取要比较的字符数量
    // Get the number of characters to compare
    size_t cmp_count = strlen(string_map_impl->key_value_pairs[i].key);
    if (key_length > cmp_count) {
      cmp_count = key_length;
    }
    // 如果键匹配，将索引存储在输出参数中并返回 true
    // If the keys match, store the index in the output parameter and return true
    if (strncmp(key, string_map_impl->key_value_pairs[i].key, cmp_count) == 0) {
      *index = i;
      return true;
    }
  }
  // 如果循环结束后未找到匹配的键，则返回 false
  // If no matching key is found after the loop, return false
  return false;
}

/*!
 * \brief 在不调整字符串映射大小的情况下设置键值对。
 * \param[inout] string_map 要更新的字符串映射。
 * \param[in] key 要设置的键。
 * \param[in] value 要设置的值。
 * \return rcutils_ret_t 操作结果。
 *
 * Set a key-value pair in the string map without resizing it.
 * \param[inout] string_map The string map to update.
 * \param[in] key The key to set.
 * \param[in] value The value to set.
 * \return rcutils_ret_t The result of the operation.
 */
rcutils_ret_t rcutils_string_map_set_no_resize(
  rcutils_string_map_t * string_map, const char * key, const char * value)
{
  // 检查输入参数是否为 NULL
  // Check if input arguments are NULL
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_map->impl, "invalid string map", return RCUTILS_RET_STRING_MAP_INVALID);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key, RCUTILS_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(value, RCUTILS_RET_INVALID_ARGUMENT);

  // 获取分配器
  // Get the allocator
  rcutils_allocator_t allocator = string_map->impl->allocator;

  // 初始化键索引变量和错误处理标志
  // Initialize key index variable and error handling flag
  size_t key_index;
  bool should_free_key_on_error = false;

  // 检查键是否已存在于字符串映射中
  // Check if the key already exists in the string map
  bool key_exists = __get_index_of_key_if_exists(string_map->impl, key, strlen(key), &key_index);
  if (!key_exists) {
    // 如果键不存在，则为其创建空间并存储
    // If the key does not exist, create space for it and store it
    assert(string_map->impl->size <= string_map->impl->capacity); // defensive, should not happen
    if (string_map->impl->size == string_map->impl->capacity) {
      return RCUTILS_RET_NOT_ENOUGH_SPACE;
    }
    for (key_index = 0; key_index < string_map->impl->capacity; ++key_index) {
      if (NULL == string_map->impl->key_value_pairs[key_index].key) {
        break;
      }
    }
    assert(key_index < string_map->impl->capacity); // defensive, this should not happen
    string_map->impl->key_value_pairs[key_index].key = rcutils_strdup(key, allocator);
    if (NULL == string_map->impl->key_value_pairs[key_index].key) {
      RCUTILS_SET_ERROR_MSG("failed to allocate memory for key");
      return RCUTILS_RET_BAD_ALLOC;
    }
    should_free_key_on_error = true;
  }
  // 现在键已经在映射中，等待设置/覆盖值
  // At this point, the key is in the map, waiting for the value to be set/overwritten
  char * original_value = string_map->impl->key_value_pairs[key_index].value;
  char * new_value = rcutils_strdup(value, allocator);
  if (NULL == new_value) {
    RCUTILS_SET_ERROR_MSG("failed to allocate memory for value");
    if (should_free_key_on_error) {
      allocator.deallocate(string_map->impl->key_value_pairs[key_index].key, allocator.state);
      string_map->impl->key_value_pairs[key_index].key = NULL;
    }
    return RCUTILS_RET_BAD_ALLOC;
  }
  string_map->impl->key_value_pairs[key_index].value = new_value;

  // 如果原始值不为 NULL，则清理旧值
  // If the original value is not NULL, clean up the old value
  if (original_value != NULL) {
    allocator.deallocate(original_value, allocator.state);
  }

  // 如果键不存在，则需要添加它，因此增加大小
  // If the key did not exist, then we had to add it, so increase the size
  if (!key_exists) {
    string_map->impl->size++;
  }

  // 返回操作成功
  // Return operation success
  return RCUTILS_RET_OK;
}

/**
 * @brief 删除指定键值对 (Remove the specified key-value pair)
 * 
 * @param[in] string_map 字符串映射结构体指针 (Pointer to the string map structure)
 * @param[in] key 要删除的键 (The key to be deleted)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t rcutils_string_map_unset(rcutils_string_map_t * string_map, const char * key)
{
  // 检查 string_map 是否为空，如果为空则返回 RCUTILS_RET_INVALID_ARGUMENT
  // (Check if string_map is NULL, and return RCUTILS_RET_INVALID_ARGUMENT if it is)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(string_map, RCUTILS_RET_INVALID_ARGUMENT);

  // 检查 string_map->impl 是否为空，如果为空则返回 RCUTILS_RET_STRING_MAP_INVALID
  // (Check if string_map->impl is NULL, and return RCUTILS_RET_STRING_MAP_INVALID if it is)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    string_map->impl, "invalid string map", return RCUTILS_RET_STRING_MAP_INVALID);

  // 检查 key 是否为空，如果为空则返回 RCUTILS_RET_INVALID_ARGUMENT
  // (Check if key is NULL, and return RCUTILS_RET_INVALID_ARGUMENT if it is)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(key, RCUTILS_RET_INVALID_ARGUMENT);

  size_t key_index;
  // 判断 key 是否存在于 string_map 中，并获取其索引值
  // (Determine if key exists in string_map and get its index value)
  if (!__get_index_of_key_if_exists(string_map->impl, key, strlen(key), &key_index)) {
    // 如果 key 不存在，则设置错误信息并返回 RCUTILS_RET_STRING_KEY_NOT_FOUND
    // (If key does not exist, set the error message and return RCUTILS_RET_STRING_KEY_NOT_FOUND)
    RCUTILS_SET_ERROR_MSG_WITH_FORMAT_STRING("key '%s' not found", key);
    return RCUTILS_RET_STRING_KEY_NOT_FOUND;
  }
  // 如果 key 存在，则根据索引值移除键值对
  // (If key exists, remove the key-value pair based on the index value)
  __remove_key_and_value_at_index(string_map->impl, key_index);
  return RCUTILS_RET_OK;
}

/**
 * @brief 判断指定键是否存在于字符串映射中 (Determine if the specified key exists in the string map)
 * 
 * @param[in] string_map 字符串映射结构体指针 (Pointer to the string map structure)
 * @param[in] key 要检查的键 (The key to check)
 * @return bool 返回键是否存在 (Return whether the key exists)
 */
bool rcutils_string_map_key_exists(const rcutils_string_map_t * string_map, const char * key)
{
  // 检查 key 是否为空，如果为空则返回 false
  // (Check if key is NULL, and return false if it is)
  if (NULL == key) {
    return false;
  }
  // 调用 rcutils_string_map_key_existsn 函数判断键是否存在
  // (Call rcutils_string_map_key_existsn function to determine if the key exists)
  return rcutils_string_map_key_existsn(string_map, key, strlen(key));
}

/**
 * @brief 判断指定长度的键是否存在于字符串映射中 (Determine if the specified length key exists in the string map)
 * 
 * @param[in] string_map 字符串映射结构体指针 (Pointer to the string map structure)
 * @param[in] key 要检查的键 (The key to check)
 * @param[in] key_length 键的长度 (The length of the key)
 * @return bool 返回键是否存在 (Return whether the key exists)
 */
bool rcutils_string_map_key_existsn(
  const rcutils_string_map_t * string_map, const char * key, size_t key_length)
{
  // 检查 string_map、string_map->impl 和 key 是否为空，如果有一个为空则返回 false
  // (Check if string_map, string_map->impl, and key are NULL, and return false if any one is)
  if (NULL == string_map || NULL == string_map->impl || NULL == key) {
    return false;
  }
  size_t key_index;
  // 判断 key 是否存在于 string_map 中，并获取其索引值
  // (Determine if key exists in string_map and get its index value)
  bool key_exists = __get_index_of_key_if_exists(string_map->impl, key, key_length, &key_index);
  return key_exists;
}

/**
 * @brief 获取指定键对应的值 (Get the value corresponding to the specified key)
 * 
 * @param[in] string_map 字符串映射结构体指针 (Pointer to the string map structure)
 * @param[in] key 要获取值的键 (The key to get the value)
 * @return const char* 返回键对应的值，如果键不存在则返回 NULL (Return the value corresponding to the key, or NULL if the key does not exist)
 */
const char * rcutils_string_map_get(const rcutils_string_map_t * string_map, const char * key)
{
  // 检查 key 是否为空，如果为空则返回 NULL
  // (Check if key is NULL, and return NULL if it is)
  if (NULL == key) {
    return NULL;
  }
  // 调用 rcutils_string_map_getn 函数获取键对应的值
  // (Call rcutils_string_map_getn function to get the value corresponding to the key)
  return rcutils_string_map_getn(string_map, key, strlen(key));
}

/**
 * @brief 获取字符串映射中特定键对应的值 (Get the value associated with a specific key in the string map)
 *
 * @param[in] string_map 字符串映射对象指针 (Pointer to the string map object)
 * @param[in] key 要查询的键 (Key to query)
 * @param[in] key_length 键的长度 (Length of the key)
 * @return 如果找到键，则返回与之关联的值，否则返回 NULL (Returns the value associated with the key if found, otherwise returns NULL)
 */
const char * rcutils_string_map_getn(
  const rcutils_string_map_t * string_map, const char * key, size_t key_length)
{
  // 检查输入参数是否有效 (Check if input parameters are valid)
  if (NULL == string_map || NULL == string_map->impl || NULL == key) {
    return NULL;
  }
  size_t key_index;
  // 如果存在键，则获取其索引 (Get the index of the key if it exists)
  if (__get_index_of_key_if_exists(string_map->impl, key, key_length, &key_index)) {
    // 返回与键关联的值 (Return the value associated with the key)
    return string_map->impl->key_value_pairs[key_index].value;
  }
  // 若未找到键，则返回 NULL (Return NULL if the key is not found)
  return NULL;
}

/**
 * @brief 获取字符串映射中给定键的下一个键 (Get the next key in the string map after the given key)
 *
 * @param[in] string_map 字符串映射对象指针 (Pointer to the string map object)
 * @param[in] key 当前键 (Current key)
 * @return 返回下一个键（如果找到），否则返回 NULL (Returns the next key if found, otherwise returns NULL)
 */
const char *
rcutils_string_map_get_next_key(const rcutils_string_map_t * string_map, const char * key)
{
  // 检查输入参数是否有效 (Check if input parameters are valid)
  if (NULL == string_map || !string_map->impl) {
    return NULL;
  }
  // 如果字符串映射为空，则返回 NULL (Return NULL if the string map is empty)
  if (string_map->impl->size == 0) {
    return NULL;
  }
  size_t start_index = 0;
  if (key != NULL) {
    // 如果给定了键，尝试查找它 (If given a key, try to find it)
    bool given_key_found = false;
    size_t i = 0;
    for (; i < string_map->impl->capacity; ++i) {
      if (string_map->impl->key_value_pairs[i].key == key) {
        given_key_found = true;
        // 在索引 i 处找到给定的键，从那里 +1 开始 (Found the given key at index i, start from there + 1)
        start_index = i + 1;
      }
    }
    // 如果未找到给定的键，则无法使用该键返回下一个键 (If the given key is not found, cannot return the next key with that)
    if (!given_key_found) {
      return NULL;
    }
  }
  // 遍历存储空间，寻找另一个非 NULL 键以返回 (Iterate through the storage and look for another non-NULL key to return)
  size_t i = start_index;
  for (; i < string_map->impl->capacity; ++i) {
    if (string_map->impl->key_value_pairs[i].key != NULL) {
      // 找到下一个键，返回它 (Found the next key, return it)
      return string_map->impl->key_value_pairs[i].key;
    }
  }
  // 未找到下一个键（或第一个键）(Next key (or first key) not found)
  return NULL;
}

/**
 * @brief 复制一个字符串映射到另一个字符串映射 (Copy a string map to another string map)
 *
 * @param[in] src_string_map 源字符串映射指针 (Pointer to the source string map)
 * @param[out] dst_string_map 目标字符串映射指针 (Pointer to the destination string map)
 * @return rcutils_ret_t 返回操作结果 (Return the operation result)
 */
rcutils_ret_t rcutils_string_map_copy(
  const rcutils_string_map_t * src_string_map, rcutils_string_map_t * dst_string_map)
{
  // 检查源字符串映射是否为空 (Check if the source string map is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(src_string_map, RCUTILS_RET_INVALID_ARGUMENT);
  // 检查目标字符串映射是否为空 (Check if the destination string map is NULL)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dst_string_map, RCUTILS_RET_INVALID_ARGUMENT);
  // 检查源字符串映射的实现是否为空 (Check if the implementation of the source string map is NULL)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    src_string_map->impl, "source string map is invalid", return RCUTILS_RET_STRING_MAP_INVALID);
  // 检查目标字符串映射的实现是否为空 (Check if the implementation of the destination string map is NULL)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(
    dst_string_map->impl, "destination string map is invalid",
    return RCUTILS_RET_STRING_MAP_INVALID);

  // 获取源字符串映射中的第一个键 (Get the first key in the source string map)
  const char * key = rcutils_string_map_get_next_key(src_string_map, NULL);

  // 遍历源字符串映射中的所有键值对 (Iterate through all key-value pairs in the source string map)
  while (key != NULL) {
    // 获取当前键对应的值 (Get the value for the current key)
    const char * value = rcutils_string_map_get(src_string_map, key);
    if (NULL == value) {
      // 如果无法获取已知键的值，则设置错误消息并返回错误 (If unable to get the value for a known key, set error message and return error)
      RCUTILS_SET_ERROR_MSG("unable to get value for known key, should not happen");
      return RCUTILS_RET_ERROR;
    }
    // 将当前键值对添加到目标字符串映射中 (Add the current key-value pair to the destination string map)
    rcutils_ret_t ret = rcutils_string_map_set(dst_string_map, key, value);
    if (ret != RCUTILS_RET_OK) {
      // 如果添加失败，错误消息已设置，直接返回错误结果 (If the addition fails, the error message is already set, return the error result directly)
      return ret;
    }
    // 获取源字符串映射中的下一个键 (Get the next key in the source string map)
    key = rcutils_string_map_get_next_key(src_string_map, key);
  }

  // 返回操作成功 (Return operation success)
  return RCUTILS_RET_OK;
}

#ifdef __cplusplus
}
#endif
