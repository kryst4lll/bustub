//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_htable_bucket_page.cpp
//
// Identification: src/storage/page/extendible_htable_bucket_page.cpp
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <optional>
#include <utility>

#include "common/exception.h"
#include "storage/page/extendible_htable_bucket_page.h"

namespace bustub {

//初始化扩展哈希表桶页
template <typename K, typename V, typename KC>
void ExtendibleHTableBucketPage<K, V, KC>::Init(uint32_t max_size) {
  this->max_size_ = max_size;
  //  size_ 初始化为 0，表示初始时该桶页中没有元素
  this->size_ = 0;
}

//查找一个键
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::Lookup(const K &key, V &value, const KC &cmp) const -> bool {
  // 使用比较器 cmp 将其键（array_[i].first）与给定的键 key 进行比较
  for (uint32_t i = 0; i < size_; ++i) {
    // 如果比较结果为 0，表示找到了对应的键
    if (cmp(array_[i].first, key) == 0) {
      // 将对应的值 array_[i].second 赋值给参数 value
      value = array_[i].second;
      return true;
    }
  }
  return false;
}

//尝试在桶中插入一个键和值
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::Insert(const K &key, const V &value, const KC &cmp) -> bool {
  // 检查当前桶页是否已满
  if (this->IsFull()) {
    return false;
  }
  // 对于每个元素，使用比较器 cmp 将其键（array_[i].first）与要插入的键 key 进行比较
  for (uint32_t i = 0; i < size_; ++i) {
    // 如果存在相同的键，表示无法插入重复的键
    if (cmp(this->array_[i].first, key) == 0) {
      return false;
    }
  }
  // 将新的键值对 {key, value} 插入到桶页中
  array_[size_++] = std::make_pair(key, value);
  return true;
}

//删除一个键和值
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::Remove(const K &key, const KC &cmp) -> bool {
  // 对于每个元素，使用比较器 cmp 将其键（array_[i].first）与要移除的键 key 进行比较
  for (uint32_t i = 0; i < size_; ++i) {
    if (cmp(array_[i].first, key) == 0) {
      // 使用另一个循环从找到的位置开始，将后面的元素依次向前移动一个位置，以覆盖要移除的元素
      for (uint32_t j = i + 1; j < size_; ++j) {
        array_[j - 1] = array_[j];
      }
      size_--;
      return true;
    }
  }
  return false;
}

//从扩展哈希表桶页中移除指定索引位置的键值对
template <typename K, typename V, typename KC>
void ExtendibleHTableBucketPage<K, V, KC>::RemoveAt(uint32_t bucket_idx) {
  // 指定的索引位置 bucket_idx 开始，对于每个元素，将其后一个元素的值复制到当前位置
  for (uint32_t i = bucket_idx; i < size_-1; ++i) {
    array_[i] = array_[i + 1];
  }
  size_--;
}

//获取桶中指定索引处的键
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::KeyAt(uint32_t bucket_idx) const -> K {
  // 调用 EntryAt(bucket_idx) 获取该索引处的键值对，然后返回该键值对的键部分 first
  return EntryAt(bucket_idx).first;
}

//获取桶中指定索引处的值
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::ValueAt(uint32_t bucket_idx) const -> V {
  // 调用 EntryAt(bucket_idx) 获取该索引处的键值对，然后返回该键值对的值部分 second
  return EntryAt(bucket_idx).second;
}

//获取桶中指定索引处的条目
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::EntryAt(uint32_t bucket_idx) const -> const std::pair<K, V> & {
  // 直接返回存储在数组 array_ 中索引位置为 bucket_idx 的键值对
  return array_[bucket_idx];
}

//返回桶中的条目数
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::Size() const -> uint32_t {
  // 返回成员变量 size_ 的值，该变量表示当前桶页中存储的键值对数量
  return this->size_;
}

//判断桶是否已满
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::IsFull() const -> bool {
  // 如果 size_ 等于 max_size_，则说明桶页已满，函数返回 true；否则返回 false
  return this->size_ == this->max_size_;
}

//判断桶是否为空
template <typename K, typename V, typename KC>
auto ExtendibleHTableBucketPage<K, V, KC>::IsEmpty() const -> bool {
  // 如果 size_ 等于0，则说明桶页已空，函数返回 true；否则返回 false
  return this->size_ == 0;
}


template class ExtendibleHTableBucketPage<int, int, IntComparator>;
template class ExtendibleHTableBucketPage<GenericKey<4>, RID, GenericComparator<4>>;
template class ExtendibleHTableBucketPage<GenericKey<8>, RID, GenericComparator<8>>;
template class ExtendibleHTableBucketPage<GenericKey<16>, RID, GenericComparator<16>>;
template class ExtendibleHTableBucketPage<GenericKey<32>, RID, GenericComparator<32>>;
template class ExtendibleHTableBucketPage<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
