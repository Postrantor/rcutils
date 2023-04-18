// Copyright 2023 Open Source Robotics Foundation, Inc.
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

/** \file sha256.h
 *  \brief SHA256 实现 (SHA256 implementation)
 *
 *  这包含了一个 SHA256 算法的实现 (This contains an implementation of the SHA256 algorithm)
 *  它最初是从 Brad Conte 复制的 (It was originally copied from Brad Conte)
 *  https://github.com/B-Con/crypto-algorithms/blob/master/sha256.c
 *  并修改以满足 ros2 代码格式和编译器警告要求。 (and modified to meet ros2 code formatting and compiler warning requirements.)
 *  算法规范可以在这里找到： (Algorithm specification can be found here:)
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
 *  该实现使用小端字节序。 (This implementation uses little endian byte order.)
 *  该实现不提供任何安全保证，其用例是非敏感消息摘要的比较。 (This implementation makes no security guarantees, its use case if for non-sensitive comparison of message digests.)
 */

#ifndef RCUTILS__SHA256_H_
#define RCUTILS__SHA256_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "rcutils/visibility_control.h"

#define RCUTILS_SHA256_BLOCK_SIZE 32

// 定义一个公共类型的 SHA256 上下文结构体
// Define a public type for the SHA256 context structure
typedef struct RCUTILS_PUBLIC_TYPE rcutils_sha256_ctx_s
{
  uint8_t data[64]; // 存储数据块，用于进行哈希计算（Store the data block for hashing）
  size_t datalen; // 当前数据块中的数据长度（Length of the data in the current data block）
  uint64_t bitlen;   // 已处理数据的位数（Number of bits processed so far）
  uint32_t state[8]; // SHA-256 算法的内部状态（Internal state of the SHA-256 algorithm）
} rcutils_sha256_ctx_t;

/// 使用初始状态初始化 sha256 算法上下文。
/// Initialize the sha256 algorithm context with starting state.
/**
 * 在开始输入数据之前，请在任何新上下文上调用此方法。
 * Call this on any new context before starting to input data.
 *
 * \param[inout] ctx 要初始化的上下文（The context to be initialized）
 * \return void
 */
RCUTILS_PUBLIC
void rcutils_sha256_init(rcutils_sha256_ctx_t * ctx);

/// 将数据添加到 sha256 算法
/// Add data to the sha256 algorithm
/**
 * 可以在已初始化的上下文上重复调用此方法。
 * This may be called repeatedly on an initialized context.
 *
 * \param[inout] ctx 已初始化的 sha256 上下文结构（Initialized sha256 context struct）
 * \param[in] data 要添加到正在哈希的总消息中的数据（Data to add to the total message being hashed）
 * \param[in] data_len 输入数据的大小（Size of the input data）
 * \return void
 */
RCUTILS_PUBLIC
void rcutils_sha256_update(rcutils_sha256_ctx_t * ctx, const uint8_t * data, size_t data_len);

/// 对已添加数据进行最终计算并输出 sha256 哈希值。
/// Finalize and output sha256 hash for all data added.
/**
 * 仅在已初始化的上下文上调用一次，可以选择使用数据进行更新。
 * Call only once on a context that has been initialized, and optionally updated with data.
 *
 * \param[inout] ctx 已初始化的 sha256 上下文结构（Initialized sha256 context struct）
 * \param[out] output_hash 要填充的已计算 sha256 消息摘要（Calculated sha256 message digest to be filled）
 * \return void
 */
#ifdef DOXYGEN_ONLY
// rosdoc2 使用的一个工具将 uint8_t[] 错误地理解为 uint8_t，
// 因此，为了文档目的，请将其设为指针。
// One of the tools used by rosdoc2 misunderstands uint8_t[] as a uint8_t,
// so make it a pointer for documentation purposes.
RCUTILS_PUBLIC
void rcutils_sha256_final(rcutils_sha256_ctx_t * ctx, uint8_t * output_hash);
#else
RCUTILS_PUBLIC
void rcutils_sha256_final(
  rcutils_sha256_ctx_t * ctx, uint8_t output_hash[RCUTILS_SHA256_BLOCK_SIZE]);
#endif

#ifdef __cplusplus
}
#endif

#endif // RCUTILS__SHA256_H_
