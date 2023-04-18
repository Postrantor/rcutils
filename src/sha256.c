// Copyright 2023 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <assert.h>
#include <string.h>

#include "rcutils/sha256.h"

/**
 * @brief 计算两个数中的最小值 (Calculate the minimum of two numbers)
 * @param a 第一个数 (The first number)
 * @param b 第二个数 (The second number)
 * @return 返回两个数中的最小值 (Return the minimum of the two numbers)
 */
static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }

/**
 * @brief 计算两个数中的最大值 (Calculate the maximum of two numbers)
 * @param a 第一个数 (The first number)
 * @param b 第二个数 (The second number)
 * @return 返回两个数中的最大值 (Return the maximum of the two numbers)
 */
static inline size_t max(size_t a, size_t b) { return a > b ? a : b; }

/**
 * @brief 对一个32位无符号整数进行循环右移 (Perform a circular right shift on a 32-bit unsigned integer)
 * @param a 要进行循环右移的数 (The number to be rotated right)
 * @param b 循环右移的位数 (The number of bits to rotate right)
 * @return 返回循环右移后的结果 (Return the result after the circular right shift)
 */
static inline uint32_t rotright(uint32_t a, const uint8_t b)
{
  assert(b < 32); // 确保移位数小于32 (Ensure that the number of bits to shift is less than 32)
  return (a >> b) |
         (a
          << (32 -
              b)); // 进行循环右移并返回结果 (Perform the circular right shift and return the result)
}

/**
 * @brief 计算布尔函数CH的值 (Calculate the value of the boolean function CH)
 * @param x 32位无符号整数x (32-bit unsigned integer x)
 * @param y 32位无符号整数y (32-bit unsigned integer y)
 * @param z 32位无符号整数z (32-bit unsigned integer z)
 * @return 返回布尔函数CH的值 (Return the value of the boolean function CH)
 */
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }

/**
 * @brief 计算布尔函数MAJ的值 (Calculate the value of the boolean function MAJ)
 * @param x 32位无符号整数x (32-bit unsigned integer x)
 * @param y 32位无符号整数y (32-bit unsigned integer y)
 * @param z 32位无符号整数z (32-bit unsigned integer z)
 * @return 返回布尔函数MAJ的值 (Return the value of the boolean function MAJ)
 */
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
{
  return (x & y) ^ (x & z) ^ (y & z);
}

/**
 * @brief 计算函数EP0的值 (Calculate the value of the function EP0)
 * @param x 32位无符号整数x (32-bit unsigned integer x)
 * @return 返回函数EP0的值 (Return the value of the function EP0)
 */
static inline uint32_t ep0(uint32_t x)
{
  return rotright(x, 2) ^ rotright(x, 13) ^
         rotright(
           x, 22); // 计算并返回函数EP0的值 (Calculate and return the value of the function EP0)
}

/**
 * @brief 计算函数EP1的值 (Calculate the value of the function EP1)
 * @param x 32位无符号整数x (32-bit unsigned integer x)
 * @return 返回函数EP1的值 (Return the value of the function EP1)
 */
static inline uint32_t ep1(uint32_t x)
{
  return rotright(x, 6) ^ rotright(x, 11) ^
         rotright(
           x, 25); // 计算并返回函数EP1的值 (Calculate and return the value of the function EP1)
}

/**
 * @brief 计算函数SIG0的值 (Calculate the value of the function SIG0)
 * @param x 32位无符号整数x (32-bit unsigned integer x)
 * @return 返回函数SIG0的值 (Return the value of the function SIG0)
 */
static inline uint32_t sig0(uint32_t x) { return rotright(x, 7) ^ rotright(x, 18) ^ (x >> 3); }

/**
 * @brief 计算函数SIG1的值 (Calculate the value of the function SIG1)
 * @param x 32位无符号整数x (32-bit unsigned integer x)
 * @return 返回函数SIG1的值 (Return the value of the function SIG1)
 */
static inline uint32_t sig1(uint32_t x) { return rotright(x, 17) ^ rotright(x, 19) ^ (x >> 10); }

/*!
 * \file sha256_transform.c
 * \brief ROS2 rclcpp 相关的代码 - SHA-256 变换函数
 */

// k 是一个 64 个元素的常量数组，用于 SHA-256 变换过程中的计算
// k is a constant array of 64 elements used in the calculation during the SHA-256 transformation process
static const uint32_t k[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

/*!
 * \brief 对给定的 SHA-256 上下文执行一次 SHA-256 变换
 * \param ctx 指向一个 rcutils_sha256_ctx_t 结构的指针，表示 SHA-256 上下文
 *
 * Perform a single SHA-256 transformation on the given SHA-256 context
 * \param[in,out] ctx A pointer to an rcutils_sha256_ctx_t structure representing the SHA-256 context
 */
static void sha256_transform(rcutils_sha256_ctx_t * ctx)
{
  uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
  uint8_t * data = ctx->data;

  // 将数据块分成 16 个 32 位的大端序整数
  // Divide the data block into 16 32-bit big-endian integers
  for (i = 0, j = 0; i < 16; ++i, j += 4) {
    m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
  }
  // 扩展 16 个 32 位整数为 64 个 32 位整数
  // Expand the 16 32-bit integers into 64 32-bit integers
  for (; i < 64; ++i) {
    m[i] = sig1(m[i - 2]) + m[i - 7] + sig0(m[i - 15]) + m[i - 16];
  }

  // 初始化工作变量
  // Initialize working variables
  a = ctx->state[0];
  b = ctx->state[1];
  c = ctx->state[2];
  d = ctx->state[3];
  e = ctx->state[4];
  f = ctx->state[5];
  g = ctx->state[6];
  h = ctx->state[7];

  // 进行 64 轮 SHA-256 变换
  // Perform 64 rounds of SHA-256 transformation
  for (i = 0; i < 64; ++i) {
    t1 = h + ep1(e) + ch(e, f, g) + k[i] + m[i];
    t2 = ep0(a) + maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  // 将工作变量的值累加到上下文状态中
  // Add the working variable values to the context state
  ctx->state[0] += a;
  ctx->state[1] += b;
  ctx->state[2] += c;
  ctx->state[3] += d;
  ctx->state[4] += e;
  ctx->state[5] += f;
  ctx->state[6] += g;
  ctx->state[7] += h;
}

/**
 * @brief 初始化 SHA256 上下文
 * @param ctx 指向 rcutils_sha256_ctx_t 结构体的指针
 *
 * @brief Initialize the SHA256 context
 * @param ctx Pointer to the rcutils_sha256_ctx_t structure
 */
void rcutils_sha256_init(rcutils_sha256_ctx_t * ctx)
{
  // 设置数据长度为 0
  // Set data length to 0
  ctx->datalen = 0;

  // 设置位长度为 0
  // Set bit length to 0
  ctx->bitlen = 0;

  // 初始化状态数组
  // Initialize state array
  ctx->state[0] = 0x6a09e667;
  ctx->state[1] = 0xbb67ae85;
  ctx->state[2] = 0x3c6ef372;
  ctx->state[3] = 0xa54ff53a;
  ctx->state[4] = 0x510e527f;
  ctx->state[5] = 0x9b05688c;
  ctx->state[6] = 0x1f83d9ab;
  ctx->state[7] = 0x5be0cd19;
}

/**
 * @brief 更新 SHA256 上下文，处理输入数据
 * @param ctx 指向 rcutils_sha256_ctx_t 结构体的指针
 * @param data 输入数据的指针
 * @param len 输入数据的长度
 *
 * @brief Update the SHA256 context, process input data
 * @param ctx Pointer to the rcutils_sha256_ctx_t structure
 * @param data Pointer to the input data
 * @param len Length of the input data
 */
void rcutils_sha256_update(rcutils_sha256_ctx_t * ctx, const uint8_t * data, size_t len)
{
  size_t i, data_remaining, block_remaining, copy_len;
  i = 0;

  // 遍历输入数据
  // Iterate through input data
  while (i < len) {
    // 计算剩余数据和块的长度
    // Calculate remaining data and block length
    data_remaining = len - i;
    block_remaining = 64 - ctx->datalen;
    copy_len = min(min(block_remaining, data_remaining), 64);

    // 将数据复制到上下文中
    // Copy data into context
    memcpy(ctx->data + ctx->datalen, data + i, copy_len);
    ctx->datalen += copy_len;
    i += copy_len;

    // 如果数据长度大于等于 64，进行 SHA256 变换
    // If the data length is greater than or equal to 64, perform the SHA256 transform
    if (ctx->datalen >= 64) {
      sha256_transform(ctx);
      ctx->bitlen += 512;
      ctx->datalen = 0;
    }
  }
}

/**
 * @brief 完成 SHA256 操作，输出哈希值
 * @param ctx 指向 rcutils_sha256_ctx_t 结构体的指针
 * @param output_hash 输出哈希值的数组
 *
 * @brief Finalize the SHA256 operation, output the hash value
 * @param ctx Pointer to the rcutils_sha256_ctx_t structure
 * @param output_hash Array for the output hash value
 */
void rcutils_sha256_final(
  rcutils_sha256_ctx_t * ctx, uint8_t output_hash[RCUTILS_SHA256_BLOCK_SIZE])
{
  size_t i = ctx->datalen;

  // 填充缓冲区中剩余的数据
  // Pad any remaining data in the buffer
  if (ctx->datalen < 56) {
    ctx->data[i++] = 0x80;
    memset(ctx->data + i, 0x00, 56 - i);
  } else {
    ctx->data[i++] = 0x80;
    if (i < 64) {
      memset(ctx->data + i, 0x00, 64 - i);
    }
    sha256_transform(ctx);
    memset(ctx->data, 0, 56);
  }

  // 将总消息长度（以位为单位）添加到填充中，并进行变换
  // Append the total message length in bits to the padding and transform
  ctx->bitlen += ctx->datalen * 8;
  ctx->data[63] = (uint8_t)(ctx->bitlen >> 0);
  ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
  ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
  ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
  ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
  ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
  ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
  ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
  sha256_transform(ctx);

  // 由于此实现使用小端字节序，而 SHA 使用大端字节序，因此在将最终状态复制到输出哈希时反转所有字节
  // Reverse all the bytes when copying the final state to the output hash, as this implementation uses little endian byte ordering and SHA uses big endian
  for (i = 0; i < 4; ++i) {
    output_hash[i + 0] = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
    output_hash[i + 4] = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
    output_hash[i + 8] = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
    output_hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
    output_hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
    output_hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
    output_hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
    output_hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
  }
}
