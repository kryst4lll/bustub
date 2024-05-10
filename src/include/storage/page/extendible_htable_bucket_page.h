//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_htable_bucket_page.h
//
// Identification: src/include/storage/page/extendible_htable_bucket_page.h
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

/**
 * Bucket page format:
 *  ----------------------------------------------------------------------------
 * | METADATA | KEY(1) + VALUE(1) | KEY(2) + VALUE(2) | ... | KEY(n) + VALUE(n)
 *  ----------------------------------------------------------------------------
 *
 * Metadata format (size in byte, 8 bytes in total):
 *  --------------------------------
 * | CurrentSize (4) | MaxSize (4)
 *  --------------------------------
 */
#pragma once

#include <optional>
#include <utility>
#include <vector>

#include "common/config.h"
#include "common/macros.h"
#include "storage/index/int_comparator.h"
#include "storage/page/b_plus_tree_page.h"
#include "type/value.h"

namespace bustub {

static constexpr uint64_t HTABLE_BUCKET_PAGE_METADATA_SIZE = sizeof(uint32_t) * 2;

constexpr auto HTableBucketArraySize(uint64_t mapping_type_size) -> uint64_t {
  return (BUSTUB_PAGE_SIZE - HTABLE_BUCKET_PAGE_METADATA_SIZE) / mapping_type_size;
};

/**
 * 扩展哈希表的桶页。
 */
template <typename KeyType, typename ValueType, typename KeyComparator>
class ExtendibleHTableBucketPage {
 public:
  // 删除所有构造函数/析构函数以确保内存安全
  ExtendibleHTableBucketPage() = delete;
  DISALLOW_COPY_AND_MOVE(ExtendibleHTableBucketPage);

  /**
   * 创建一个新的桶页从缓冲池中，并调用initialize方法来设置默认值
   * @param max_size 桶数组的最大大小
   */
  void Init(uint32_t max_size = HTableBucketArraySize(sizeof(MappingType)));

  /**
   * 查找一个键
   *
   * @param key 要查找的键
   * @param[out] value 要设置的值
   * @param cmp 比较器
   * @return 如果键和值存在，则返回true，如果未找到则返回false。
   */
  auto Lookup(const KeyType &key, ValueType &value, const KeyComparator &cmp) const -> bool;

  /**
   * 尝试在桶中插入一个键和值。
   *
   * @param key 要插入的键
   * @param value 要插入的值
   * @param cmp 要使用的比较器
   * @return 如果插入成功，则返回true，如果桶已满或相同的键已存在则返回false
   */
  auto Insert(const KeyType &key, const ValueType &value, const KeyComparator &cmp) -> bool;

  /**
   * 删除一个键和值。
   *
   * @return 如果移除成功，则返回true，如果未找到则返回false
   */
  auto Remove(const KeyType &key, const KeyComparator &cmp) -> bool;

  void RemoveAt(uint32_t bucket_idx);

  /**
   * @brief 获取桶中指定索引处的键。
   *
   * @param bucket_idx 桶中要获取键的索引
   * @return 桶中索引为bucket_idx的键
   */
  auto KeyAt(uint32_t bucket_idx) const -> KeyType;

  /**
   * 获取桶中指定索引处的值。
   *
   * @param bucket_idx 桶中要获取值的索引
   * @return 桶中索引为bucket_idx的值
   */
  auto ValueAt(uint32_t bucket_idx) const -> ValueType;

  /**
   * 获取桶中指定索引处的条目。
   *
   * @param bucket_idx 桶中要获取条目的索引
   * @return 桶中索引为bucket_idx的条目
   */
  auto EntryAt(uint32_t bucket_idx) const -> const std::pair<KeyType, ValueType> &;

  /**
   * @return 桶中的条目数
   */
  auto Size() const -> uint32_t;

  /**
   * @return 桶是否已满
   */
  auto IsFull() const -> bool;

  /**
   * @return 桶是否为空
   */
  auto IsEmpty() const -> bool;

  /**
   * 打印桶的占用信息
   */
  void PrintBucket() const;


  void Clear(){
    size_ = 0;  // 将 size_ 置为 0，即清空 bucket
  }


 private:
  uint32_t size_; //桶中已存储的键值对数量
  uint32_t max_size_; //桶可以处理的键值对的最大数量
  MappingType array_[HTableBucketArraySize(sizeof(MappingType))]; //一个桶本地深度的数组
};

}  // namespace bustub
