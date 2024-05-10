//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_htable_directory_page.h
//
// Identification: src/include/storage/page/extendible_htable_directory_page.h
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

/**
 * Directory page format:
 *  --------------------------------------------------------------------------------------
 * | MaxDepth (4) | GlobalDepth (4) | LocalDepths (512) | BucketPageIds(2048) | Free(1528)
 *  --------------------------------------------------------------------------------------
 */

#pragma once

#include <cassert>
#include <climits>
#include <cstdlib>
#include <string>

#include "common/config.h"
#include "storage/index/generic_key.h"

namespace bustub {

static constexpr uint64_t HTABLE_DIRECTORY_PAGE_METADATA_SIZE = sizeof(uint32_t) * 2;

/**
 * HTABLE_DIRECTORY_ARRAY_SIZE 是可容纳扩展哈希索引目录页中的页ID数量。
 * 这是因为目录数组必须按2的幂进行增长，而1024个页ID会留下零空间
 * 用于存储其他成员变量。
 */
static constexpr uint64_t HTABLE_DIRECTORY_MAX_DEPTH = 9;
static constexpr uint64_t HTABLE_DIRECTORY_ARRAY_SIZE = 1 << HTABLE_DIRECTORY_MAX_DEPTH;

/**
 * 扩展哈希表的目录页。
 */
class ExtendibleHTableDirectoryPage {
 public:
  // 在新目录页面的构造函数中复制映射关系
  ExtendibleHTableDirectoryPage(const ExtendibleHTableDirectoryPage *source_page) {
      // 复制映射关系
      for (uint32_t i = 0; i < HTABLE_DIRECTORY_ARRAY_SIZE; ++i) {
          bucket_page_ids_[i] = source_page->bucket_page_ids_[i];
          local_depths_[i] = source_page->local_depths_[i];
      }
  }


  // 删除所有构造函数/析构函数以确保内存安全性
  ExtendibleHTableDirectoryPage() = delete;
  DISALLOW_COPY_AND_MOVE(ExtendibleHTableDirectoryPage);

  /**
   * 从缓冲池创建新目录页后，必须调用 initialize 方法设置默认值。
   * @param max_depth 目录页中的最大深度
   */
  void Init(uint32_t max_depth = HTABLE_DIRECTORY_MAX_DEPTH);

  /**
   * 获取键被哈希到的桶索引
   *
   * @param hash 键的哈希值
   * @return 当前键被哈希到的桶索引
   */
  auto HashToBucketIndex(uint32_t hash) const -> uint32_t;

  /**
   * 使用目录索引查找桶页
   *
   * @param bucket_idx 要查找的目录中的索引
   * @return 对应于 bucket_idx 的桶页ID
   */
  auto GetBucketPageId(uint32_t bucket_idx) const -> page_id_t;

  /**
   * 使用桶索引和页ID更新目录索引
   *
   * @param bucket_idx 要插入 page_id 的目录索引
   * @param bucket_page_id 要插入的页ID
   */
  void SetBucketPageId(uint32_t bucket_idx, page_id_t bucket_page_id);

  /**
   * 获取索引的分裂映像
   *
   * @param bucket_idx 要查找分裂映像的目录索引
   * @return 分裂映像的目录索引
   **/
  auto GetSplitImageIndex(uint32_t bucket_idx) const -> uint32_t;

  /**
   * GetGlobalDepthMask - 返回全局深度为1的掩码和其余为0的掩码。
   *
   * 在扩展哈希中，我们将键映射到目录索引
   * 使用以下哈希+掩码函数。
   *
   * DirectoryIndex = Hash(key) & GLOBAL_DEPTH_MASK
   *
   * 其中 GLOBAL_DEPTH_MASK 是一个从LSB开始具有全局深度为1的掩码。
   * 例如，全局深度3对应于32位表示中的0x00000007。
   *
   * @return 全局深度为1的掩码和其余为0的掩码（从LSB开始为1）
   */
  auto GetGlobalDepthMask() const -> uint32_t;

  /**
   * GetLocalDepthMask - 获取局部深度掩码，与全局深度掩码相同，但使用位于 bucket_idx 处的桶的局部深度
   *
   * @param bucket_idx 用于查找局部深度的索引
   * @return 包含局部 1 的掩码，其余位为 0（从最低有效位向上为 1 的位）
   */
  auto GetLocalDepthMask(uint32_t bucket_idx) const -> uint32_t;

  /**
   * 获取哈希表目录的全局深度
   *
   * @return 目录的全局深度
   */
  auto GetGlobalDepth() const -> uint32_t;

  auto GetMaxDepth() const -> uint32_t;
  /**
   * 增加目录的全局深度
   */
  void IncrGlobalDepth();

  /**
   * 减少目录的全局深度
   */
  void DecrGlobalDepth();

  /**
   * @return 如果目录可以收缩则返回 true
   */
  auto CanShrink() -> bool;

  /**
   * @return 当前目录大小
   */
  auto Size() const -> uint32_t;

  /**
   * @return 最大目录大小
   */
  auto MaxSize() const -> uint32_t;

  /**
   * 获取位于 bucket_idx 处的桶的局部深度
   *
   * @param bucket_idx 要查找的桶的索引
   * @return 位于 bucket_idx 处的桶的局部深度
   */
  auto GetLocalDepth(uint32_t bucket_idx) const -> uint32_t;

  /**
   * 将位于 bucket_idx 处的桶的局部深度设置为 local_depth
   *
   * @param bucket_idx 要更新的桶的索引
   * @param local_depth 新的局部深度
   */
  void SetLocalDepth(uint32_t bucket_idx, uint8_t local_depth);

  /**
   * 增加位于 bucket_idx 处的桶的局部深度
   * @param bucket_idx 要增加深度的桶的索引
   */
  void IncrLocalDepth(uint32_t bucket_idx);

  /**
   * 减少位于 bucket_idx 处的桶的局部深度
   * @param bucket_idx 要减少深度的桶的索引
   */
  void DecrLocalDepth(uint32_t bucket_idx);

  /**
   * VerifyIntegrity
   *
   * 验证以下不变性：
   * (1) 所有 LD <= GD。
   * (2) 每个桶正好有 2^(GD - LD) 个指针指向它。
   * (3) 每个具有相同 bucket_page_id 的索引处的 LD 相同。
   */
  void VerifyIntegrity() const;

  /**
   * 打印当前目录
   */
  void PrintDirectory() const;

 private:
  uint32_t max_depth_;  //页头可以处理的最大深度
  uint32_t global_depth_;   //当前目录的全局深度
  uint8_t local_depths_[HTABLE_DIRECTORY_ARRAY_SIZE];   //记录一个桶局部深度的数组
  page_id_t bucket_page_ids_[HTABLE_DIRECTORY_ARRAY_SIZE];  //记录包含第三层的BucketPage的PageId数组
};

static_assert(sizeof(page_id_t) == 4);

static_assert(sizeof(ExtendibleHTableDirectoryPage) <= BUSTUB_PAGE_SIZE);

}  // namespace bustub
