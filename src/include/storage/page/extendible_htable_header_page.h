//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_htable_header_page.h
//
// Identification: src/include/storage/page/extendible_htable_header_page.h
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

/**
 * Header page format:
 *  ---------------------------------------------------
 * | DirectoryPageIds(2048) | MaxDepth (4) | Free(2044)
 *  ---------------------------------------------------
 */

#pragma once

#include <cstdlib>
#include "common/config.h"
#include "common/macros.h"

namespace bustub {

static constexpr uint64_t HTABLE_HEADER_PAGE_METADATA_SIZE = sizeof(uint32_t);
static constexpr uint64_t HTABLE_HEADER_MAX_DEPTH = 9;
static constexpr uint64_t HTABLE_HEADER_ARRAY_SIZE = 1 << HTABLE_HEADER_MAX_DEPTH;

class ExtendibleHTableHeaderPage {
public:
// 删除所有构造函数/析构函数以确保内存安全
ExtendibleHTableHeaderPage() = delete;
DISALLOW_COPY_AND_MOVE(ExtendibleHTableHeaderPage);

/**

从缓冲池创建新的头部页面后，必须调用初始化方法来设置默认值
@param max_depth 头部页面的最大深度 */ void Init(uint32_t max_depth = HTABLE_HEADER_MAX_DEPTH);
/**

获取键被散列到的目录索引
@param hash 键的哈希值
@return 键被散列到的目录索引 */ auto HashToDirectoryIndex(uint32_t hash) const -> uint32_t;
/**

获取指定索引处的目录页 id
@param directory_idx 目录页 id 数组中的索引
@return 索引处的目录页 id */ auto GetDirectoryPageId(uint32_t directory_idx) const -> uint32_t;
/**

@brief 设置指定索引处的目录页 id
@param directory_idx 目录页 id 数组中的索引
@param directory_page_id 目录的页面 id */ void SetDirectoryPageId(uint32_t directory_idx, page_id_t directory_page_id);
/**

@brief 获取头部页面可以处理的最大目录页 id 数量 */ auto MaxSize() const -> uint32_t;
/**

打印头部的占用信息 */ void PrintHeader() const;
private:
page_id_t directory_page_ids_[HTABLE_HEADER_ARRAY_SIZE];    //记录第二层DirectoryPage的PageId数组
uint32_t max_depth_;    //最大深度
};


static_assert(sizeof(page_id_t) == 4);

static_assert(sizeof(ExtendibleHTableHeaderPage) <= BUSTUB_PAGE_SIZE);

}  // namespace bustub
