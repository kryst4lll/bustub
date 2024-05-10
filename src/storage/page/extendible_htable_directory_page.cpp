//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_htable_directory_page.cpp
//
// Identification: src/storage/page/extendible_htable_directory_page.cpp
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/extendible_htable_directory_page.h"

#include <algorithm>
#include <unordered_map>

#include "common/config.h"
#include "common/logger.h"

namespace bustub {

//调用 initialize 方法设置默认值
void ExtendibleHTableDirectoryPage::Init(uint32_t max_depth) {
  //将目录页的最大深度（max_depth）设置为传入的参数值，将全局深度（globaldepth）初始化为0
  this->max_depth_ = max_depth;
  global_depth_ = 0;
  //将本地深度数组中的所有值设为0
  std::fill(local_depths_, local_depths_ + (1 << max_depth), 0);
  //将桶页ID数组中的所有值设为无效页ID
  std::fill(bucket_page_ids_, bucket_page_ids_ + (1 << max_depth), INVALID_PAGE_ID);
}

//获取键被哈希到的桶索引
auto ExtendibleHTableDirectoryPage::HashToBucketIndex(uint32_t hash) const -> uint32_t {
  // 返回hash的后global_depth位
  return hash & GetGlobalDepthMask();
}

//使用目录索引查找桶页
auto ExtendibleHTableDirectoryPage::GetBucketPageId(uint32_t bucket_idx) const -> page_id_t {
  return this->bucket_page_ids_[bucket_idx];
}

//使用桶索引和页ID更新目录索引
void ExtendibleHTableDirectoryPage::SetBucketPageId(uint32_t bucket_idx, page_id_t bucket_page_id) {
  this->bucket_page_ids_[bucket_idx] = bucket_page_id;
}

//获取索引的分裂映像
auto ExtendibleHTableDirectoryPage::GetSplitImageIndex(uint32_t bucket_idx) const -> uint32_t {
  //使用位移运算符 << 将数字 1 左移 global_depth_ - 1 位，得到一个二进制数，其中只有 global_depth_ 位是 1，其余位是 0
  return bucket_idx + (1 << (global_depth_ - 1));
}

//返回全局深度为1的掩码和其余为0的掩码
auto ExtendibleHTableDirectoryPage::GetGlobalDepthMask() const -> uint32_t {
  // 使用位移运算符 << 将数字 1 左移 global_depth_ 位，对结果减去 1，以确保掩码的低 global_depth_ 位都是 1，其余位都是 0。
  auto depth = global_depth_;
  uint32_t result = (1 << depth) - 1;
  return result;
}

//获取局部深度掩码
auto ExtendibleHTableDirectoryPage::GetLocalDepthMask(uint32_t bucket_idx) const -> uint32_t {
  // 首先检查 bucket_idx 是否有效
  if (bucket_idx >= static_cast<uint32_t>(1 << global_depth_)) {
    throw std::out_of_range("Bucket index out of range");
  }
  // 获取 bucket_idx 处的局部深度
  uint32_t local_depth = local_depths_[bucket_idx];
  // 计算掩码，局部深度位是 1，其余位是 0
  return 1 << (local_depth - 1);
}

//获取哈希表目录的全局深度
auto ExtendibleHTableDirectoryPage::GetGlobalDepth() const -> uint32_t { return global_depth_; }

//增加目录的全局深度
void ExtendibleHTableDirectoryPage::IncrGlobalDepth() {
  //检查全局深度是否已经达到最大深度
  if (global_depth_ >= max_depth_) {
    return;
  }
  // 对于每个当前全局深度下存在的桶
  for (int i = 0; i < 1 << global_depth_; i++) {
    // 将桶的页ID和局部深度复制到哈希表中新的位置，新的位置是原来位置加上 2^globaldepth，相当于在原来的位置上追加一组新的桶
    bucket_page_ids_[(1 << global_depth_) + i] = bucket_page_ids_[i];
    local_depths_[(1 << global_depth_) + i] = local_depths_[i];
  }
  // 增加全局深度的值。
  global_depth_++;
}

//减少目录的全局深度
void ExtendibleHTableDirectoryPage::DecrGlobalDepth() {
  //检查全局深度是否已经小于等于 0
  if (global_depth_ <= 0) {
    return;
  }
  global_depth_--;
}

//判断当前是否可以收缩（减小全局深度）
auto ExtendibleHTableDirectoryPage::CanShrink() -> bool { 
  // 如果全局深度为0，则不能再收缩
  if (global_depth_ == 0) {
    return false;
  }
  // 检查所有桶的局部深度是否都小于全局深度
  for (uint32_t i = 0; i < Size(); i++) {
    // 有局部深度等于全局深度的，则不能收缩
    if (local_depths_[i] == global_depth_) {
      return false;
    }
  }
  return true;
}

//返回当前目录大小
auto ExtendibleHTableDirectoryPage::Size() const -> uint32_t {
  // 目录的当前大小是2的global_depth_次方
  return 1 << global_depth_;
}

//获取位于 bucket_idx 处的桶的局部深度
auto ExtendibleHTableDirectoryPage::GetLocalDepth(uint32_t bucket_idx) const -> uint32_t {
  return local_depths_[bucket_idx];
}

//获取哈希表的最大深度
auto ExtendibleHTableDirectoryPage::GetMaxDepth() const -> uint32_t { return max_depth_; }

//将位于 bucket_idx 处的桶的局部深度设置为 local_depth
void ExtendibleHTableDirectoryPage::SetLocalDepth(uint32_t bucket_idx, uint8_t local_depth) {
  local_depths_[bucket_idx] = local_depth;
}

//增加位于 bucket_idx 处的桶的局部深度
void ExtendibleHTableDirectoryPage::IncrLocalDepth(uint32_t bucket_idx) {
  if (local_depths_[bucket_idx] < global_depth_) {
    ++local_depths_[bucket_idx];
  }
}

//减少位于 bucket_idx 处的桶的局部深度
void ExtendibleHTableDirectoryPage::DecrLocalDepth(uint32_t bucket_idx) {
  if (local_depths_[bucket_idx] > 0) {
    --local_depths_[bucket_idx];
  }
}



}  // namespace bustub
