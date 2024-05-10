//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// disk_extendible_hash_table.h
//
// Identification: src/include/container/disk/hash/extendible_hash_table.h
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <deque>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "buffer/buffer_pool_manager.h"
#include "common/config.h"
#include "concurrency/transaction.h"
#include "container/hash/hash_function.h"
#include "storage/page/extendible_htable_bucket_page.h"
#include "storage/page/extendible_htable_directory_page.h"
#include "storage/page/extendible_htable_header_page.h"
#include "storage/page/page_guard.h"

namespace bustub {

/**
 * 基于缓冲池管理器的可扩展哈希表实现。支持非唯一键。支持插入和删除。随着桶变满/空，表动态增长/收缩。
 */
template <typename K, typename V, typename KC>
class DiskExtendibleHashTable {
 public:
  /**
   * @brief 创建一个新的DiskExtendibleHashTable。
   *
   * @param name
   * @param bpm 要使用的缓冲池管理器
   * @param cmp 键的比较器
   * @param hash_fn 哈希函数
   * @param header_max_depth 头页面允许的最大深度
   * @param directory_max_depth 目录页面允许的最大深度
   * @param bucket_max_size 桶页面数组允许的最大大小
   */
  explicit DiskExtendibleHashTable(const std::string &name, BufferPoolManager *bpm, const KC &cmp,
                                   const HashFunction<K> &hash_fn, uint32_t header_max_depth = HTABLE_HEADER_MAX_DEPTH,
                                   uint32_t directory_max_depth = HTABLE_DIRECTORY_MAX_DEPTH,
                                   uint32_t bucket_max_size = HTableBucketArraySize(sizeof(std::pair<K, V>)));

  /** TODO(P2): 添加实现
   * 将键值对插入哈希表。
   *
   * @param key 要创建的键
   * @param value 与键关联的值
   * @param transaction 当前事务
   * @return 如果插入成功，则为true；否则为false
   */
  auto Insert(const K &key, const V &value, Transaction *transaction = nullptr) -> bool;

  /** TODO(P2): 添加实现
   * 从哈希表中删除键值对。
   *
   * @param key 要删除的键
   * @param value 要删除的值
   * @param transaction 当前事务
   * @return 如果删除成功，则为true；否则为false
   */
  auto Remove(const K &key, Transaction *transaction = nullptr) -> bool;

  /** TODO(P2): 添加实现
   * 获取哈希表中与给定键关联的值。
   *
   * 注意（fall2023）：这个学期你只需要支持唯一键值对。
   *
   * @param key 要查找的键
   * @param[out] result 与给定键关联的值（们）
   * @param transaction 当前事务
   * @return 与给定键关联的值（们）
   */
  auto GetValue(const K &key, std::vector<V> *result, Transaction *transaction = nullptr) const -> bool;

  /**
   * 验证可扩展哈希表目录完整性的辅助函数。
   */
  void VerifyIntegrity() const;

  /**
   * 暴露头页面ID的辅助函数。
   */
  auto GetHeaderPageId() const -> page_id_t;

  /**
   * 打印HashTable的辅助函数。
   */
  void PrintHT() const;

 private:
  /**
   * Hash - 将MurmurHash的64位哈希值简单地转换为32位，以用于可扩展哈希。
   *
   * @param key 要哈希的键
   * @return 转换后的32位哈希值
   */
  auto Hash(K key) const -> uint32_t;

  auto InsertToNewDirectory(ExtendibleHTableHeaderPage *header, uint32_t directory_idx, uint32_t hash, const K &key,
                            const V &value) -> bool;

  auto InsertToNewBucket(ExtendibleHTableDirectoryPage *directory, uint32_t bucket_idx, const K &key, const V &value)
      -> bool;

  void UpdateDirectoryMapping(ExtendibleHTableDirectoryPage *directory, uint32_t new_bucket_idx,
                              page_id_t new_bucket_page_id, uint32_t new_local_depth, uint32_t local_depth_mask);

  void MigrateEntries(ExtendibleHTableBucketPage<K, V, KC> *old_bucket,
                      ExtendibleHTableBucketPage<K, V, KC> *new_bucket, uint32_t new_bucket_idx,
                      uint32_t local_depth_mask);
  
  auto SplitBucket(ExtendibleHTableDirectoryPage *directory, ExtendibleHTableBucketPage<K, V, KC> *bucket, uint32_t bucket_idx)
                    -> bool;
  // member variables
  std::string index_name_;  //索引名称
  BufferPoolManager *bpm_;  //缓冲池管理器
  KC cmp_;  //键的比较器
  HashFunction<K> hash_fn_; //哈希函数
  uint32_t header_max_depth_; //头部页面允许的最大深度
  uint32_t directory_max_depth_;  //目录页面允许的最大深度
  uint32_t bucket_max_size_;  //桶页面允许的最大深度
  page_id_t header_page_id_;  //头目录PageId
};

}  // namespace bustub
