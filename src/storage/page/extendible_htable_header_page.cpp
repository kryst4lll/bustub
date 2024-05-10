//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_htable_header_page.cpp
//
// Identification: src/storage/page/extendible_htable_header_page.cpp
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/extendible_htable_header_page.h"

#include "common/exception.h"

namespace bustub {

//初始化扩展哈希表头部页
void ExtendibleHTableHeaderPage::Init(uint32_t max_depth) {
  this->max_depth_ = max_depth;
  // 调用 MaxSize() 方法获取目录页ID数组的大小，并使用循环将目录页ID数组中的所有值设置为无效ID
  auto size = MaxSize();
  for (uint32_t i = 0; i < size; ++i) {
    directory_page_ids_[i] = INVALID_PAGE_ID;
  }
}

//获取键被散列到的目录索引
auto ExtendibleHTableHeaderPage::HashToDirectoryIndex(uint32_t hash) const -> uint32_t {
  //如果最大深度为0，则说明哈希表为空，直接返回0作为目录索引
  if (this->max_depth_ == 0) {
    return 0;
  }
  // 将hash右移(32 - this->max_depth_)位,丢弃哈希值的低this->max_depth_位，保留高位的部分。
  return hash >> (sizeof(uint32_t) * 8 - this->max_depth_);
}

//获取指定索引处的目录页 id
auto ExtendibleHTableHeaderPage::GetDirectoryPageId(uint32_t directory_idx) const -> uint32_t {
  //如果目录索引超出了最大大小，则返回无效的页ID
  if (directory_idx >= this->MaxSize()) {
    return INVALID_PAGE_ID;
  }
  return this->directory_page_ids_[directory_idx];
}

//设置指定索引处的目录页 id
void ExtendibleHTableHeaderPage::SetDirectoryPageId(uint32_t directory_idx, page_id_t directory_page_id) {
  //如果目录索引超出了最大大小，则直接返回
  if(directory_idx >= this->MaxSize()){
    return;
  }
  directory_page_ids_[directory_idx] = directory_page_id;
}

//获取头部页面可以处理的最大目录页 id 数量
auto ExtendibleHTableHeaderPage::MaxSize() const -> uint32_t {
  //返回结果为 2^max_depth_
  return 1 << this->max_depth_;
}


}  // namespace bustub
