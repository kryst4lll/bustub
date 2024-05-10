//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// disk_extendible_hash_table.cpp
//
// Identification: src/container/disk/hash/disk_extendible_hash_table.cpp
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "common/exception.h"
#include "common/logger.h"
#include "common/macros.h"
#include "common/rid.h"
#include "common/util/hash_util.h"
#include "container/disk/hash/disk_extendible_hash_table.h"
#include "storage/index/hash_comparator.h"
#include "storage/page/extendible_htable_bucket_page.h"
#include "storage/page/extendible_htable_directory_page.h"
#include "storage/page/extendible_htable_header_page.h"
#include "storage/page/page_guard.h"

namespace bustub {

//创建一个新的DiskExtendibleHashTable
template <typename K, typename V, typename KC>
DiskExtendibleHashTable<K, V, KC>::DiskExtendibleHashTable(const std::string &name, BufferPoolManager *bpm,
                                                           const KC &cmp, const HashFunction<K> &hash_fn,
                                                           uint32_t header_max_depth, uint32_t directory_max_depth,
                                                           uint32_t bucket_max_size)
    : bpm_(bpm),
      cmp_(cmp),
      hash_fn_(std::move(hash_fn)),
      header_max_depth_(header_max_depth),
      directory_max_depth_(directory_max_depth),
      bucket_max_size_(bucket_max_size) {
  this->index_name_ = name;
  //初始化header页
  header_page_id_ = INVALID_PAGE_ID;
  // 通过缓冲池管理器创建一个新的页并获取其页ID
  auto header_guard = bpm->NewPageGuarded(&header_page_id_);
  // 初始化该页面为 ExtendibleHTableHeaderPage 类型，并设置其最大深度
  auto header_page = header_guard.AsMut<ExtendibleHTableHeaderPage>();
  header_page->Init(header_max_depth_);

  //初始化目录页
  page_id_t directory_page_id = INVALID_PAGE_ID;
  // 通过缓冲池管理器创建一个新的页并获取其页ID
  BasicPageGuard directory_guard = bpm->NewPageGuarded(&directory_page_id);
  // 初始化该页面为 ExtendibleHTableDirectoryPage 类型，并设置其最大深度
  auto directory_page = directory_guard.AsMut<ExtendibleHTableDirectoryPage>();
  directory_page->Init(directory_max_depth_);

  //初始化bucket页
  page_id_t bucket_page_id = INVALID_PAGE_ID;
  // 通过缓冲池管理器创建一个新的页并获取其页ID
  BasicPageGuard bucket_guard = bpm->NewPageGuarded(&bucket_page_id);
  // 初始化该页面为 ExtendibleHTableBucketPage 类型，并设置其最大大小
  // 将 bucket_guard 转换为指向 ExtendibleHTableBucketPage 类型的可变指针
  auto bucket_page = bucket_guard.AsMut<ExtendibleHTableBucketPage<GenericKey<8>, RID, GenericComparator<8>>>();
  bucket_page->Init(bucket_max_size_);

  //连接header页和目录页
  // 在头部页上设置目录页的页ID
  header_page->SetDirectoryPageId(0, directory_page_id);
  // 在目录页上设置桶页的页ID
  directory_page->SetBucketPageId(0, bucket_page_id);
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
//获取哈希表中与给定键关联的值
template <typename K, typename V, typename KC>
// 在哈希表中查找与给定键相关联的值
// key:要查找的键;result:与给定键相关联的值; transaction 当前事务
auto DiskExtendibleHashTable<K, V, KC>::GetValue(const K &key, std::vector<V> *result, Transaction *transaction) const
    -> bool {
  // 获取header page
  ReadPageGuard header_guard = bpm_->FetchPageRead(header_page_id_);
  auto header_page = header_guard.As<ExtendibleHTableHeaderPage>();
  // 通过hash值获取dir_page_id。若dir_page_id为非法id则未找到
  auto hash = Hash(key);
  auto dirIndex = header_page->HashToDirectoryIndex(hash);
  page_id_t directory_page_id = header_page->GetDirectoryPageId(dirIndex);
  if (directory_page_id == INVALID_PAGE_ID) {
    return false;
  }
  // 获取dir_page
  header_guard.Drop();
  ReadPageGuard directory_guard = bpm_->FetchPageRead(directory_page_id);
  auto directory_page = directory_guard.As<ExtendibleHTableDirectoryPage>();
  // 通过hash值获取bucket_page_id。若bucket_page_id为非法id则未找到
  auto bucket_index = directory_page->HashToBucketIndex(hash);
  auto bucket_page_id = directory_page->GetBucketPageId(bucket_index);
  if (bucket_page_id == INVALID_PAGE_ID) {
    return false;
  }
  ReadPageGuard bucket_guard = bpm_->FetchPageRead(bucket_page_id);
  // 获取bucket_page
  directory_guard.Drop();
  auto bucket_page = bucket_guard.As<ExtendibleHTableBucketPage<K, V, KC>>();
  // 在bucket_page上查找
  V value;
  if (bucket_page->Lookup(key, value, cmp_)) {
    result->push_back(value);
    return true;
  }
  return false;
}



/*****************************************************************************
 * INSERTION
 *****************************************************************************/

template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::Insert(const K &key, const V &value, Transaction *transaction) -> bool {
  //计算key的哈希值
  uint32_t hash = Hash(key);
  //先根据header_page_id_获取header页
  auto header_read_guard= bpm_->FetchPageRead(header_page_id_);
  auto header_page_data = bpm_->FetchPage(header_page_id_);
  if (header_page_data == nullptr) {
    throw Exception("Failed to fetch header page");
  }
  // 获取header_page
  auto header_page = reinterpret_cast<ExtendibleHTableHeaderPage *>(header_page_data->GetData());
  //使用header_page来获取目录索引
  auto directory_index = header_page->HashToDirectoryIndex(hash);
  //用目录索引获取目录页，然后找到目录页的ID
  page_id_t directory_page_id = header_page->GetDirectoryPageId(directory_index);
  header_page_data->RUnlatch();
  header_read_guard.Drop();
  bpm_->UnpinPage(header_page_id_, false); // 取消固定header_page
  // 获取目录页的只读访问权限
  auto directory_read_guard= bpm_->FetchPageRead(directory_page_id);
  // 获取目录页,可以进行读取和修改操作
  auto directory_page_data = bpm_->FetchPage(directory_page_id);
  // 获取目录页
  auto directory_page = reinterpret_cast<ExtendibleHTableDirectoryPage *>(directory_page_data->GetData());

  //使用directory_page来获取目录索引
  auto bucket_index = directory_page->HashToBucketIndex(hash);
  // 根据目录索引找到桶页的ID
  page_id_t bucket_page_id = directory_page->GetBucketPageId(bucket_index);
  directory_page_data->RUnlatch();
  directory_read_guard.Drop();
  bpm_->UnpinPage(directory_page_id, false);// 取消固定directory_page_id

  auto bucket_page_data = bpm_->FetchPage(bucket_page_id);
  //获取桶页
  auto bucket_page = reinterpret_cast<ExtendibleHTableBucketPage<K, V, KC> *>(bucket_page_data->GetData());

  //尝试将键值对插入到新的目录中
  bool insert_result = InsertToNewDirectory(header_page, directory_index, hash, key, value);
  //如果插入成功，将bucket_page_id写回目录页
  //插入不成功，检查是否需要扩展目录页
  if (insert_result) {
    bpm_->UnpinPage(bucket_page_id, true); // 标记为脏并取消固定桶页
  } else {
    // 检查是否需要扩展全局目录
    if (directory_page->GetLocalDepth(bucket_index) == directory_page->GetGlobalDepth()) {
      // 需要扩展全局目录，因为全局深度等于局部深度
      if(directory_page->GetGlobalDepth() == directory_max_depth_){
        return false;
      }
      directory_page->IncrGlobalDepth();
    }
    // 进行桶分裂
    uint32_t original_bucket_idx = directory_page->HashToBucketIndex(hash);
    uint32_t split_bucket_idx = directory_page->GetSplitImageIndex(original_bucket_idx);
    //初始化新bucket页
    page_id_t new_bucket_page_id = INVALID_PAGE_ID;
    BasicPageGuard new_bucket_guard = bpm_->NewPageGuarded(&new_bucket_page_id);
    auto new_bucket_page = new_bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
    new_bucket_page->Init(bucket_max_size_);

    // 更新原始和新分裂桶的局部深度
    directory_page->IncrLocalDepth(original_bucket_idx);
    directory_page->SetLocalDepth(split_bucket_idx, directory_page->GetLocalDepth(original_bucket_idx));
    auto new_local_depth = directory_page->GetLocalDepth(original_bucket_idx);
    auto local_depth_mask = directory_page->GetLocalDepthMask(original_bucket_idx);
    directory_page->SetBucketPageId(split_bucket_idx, new_bucket_page_id);
    // 局部深度已经增加，且split_bucket_idx是新桶对应的目录索引，進行目錄更新
    UpdateDirectoryMapping(directory_page, split_bucket_idx, new_bucket_page_id, new_local_depth, local_depth_mask);

    //重新分配键到原桶和新桶
    auto size = bucket_page->Size();
    for (uint32_t i = 0; i < size; ++i) {
      // 将键值对分配到原桶和新桶
      auto bucket_key = bucket_page->KeyAt(i);
      uint32_t key_hash = Hash(bucket_key);
      uint32_t bucketIndex = directory_page->HashToBucketIndex(key_hash);
      // 确定是否应该在当前桶中
      if (split_bucket_idx == bucketIndex) {
        // 键应该存储在新桶中
        bucket_page->Remove(bucket_key,cmp_);
        new_bucket_page->Insert(bucket_page->KeyAt(i), bucket_page->ValueAt(i), cmp_);
      }
    }
    // 在分裂完成后，再次尝试插入
    insert_result = Insert(key, value);
  }
  // WritePageGuard的析构函数会自动处理页面的解固定和脏页写回操作
  return insert_result;

}

//将键值对插入到新的目录页中
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::InsertToNewDirectory(ExtendibleHTableHeaderPage *header, uint32_t directory_idx,
                                                             uint32_t hash, const K &key, const V &value) -> bool {
  // 计算新的目录索引，这里假设已经进行了全局深度的增加
  uint32_t new_directory_index = header->HashToDirectoryIndex(hash);

  // 获取新的目录页ID
  page_id_t new_directory_page_id = header->GetDirectoryPageId(new_directory_index);

  // 获取新的目录页
  auto new_directory_page_data = bpm_->FetchPage(new_directory_page_id);
  if (new_directory_page_data == nullptr) {
    throw Exception("Failed to fetch new directory page");
  }
  auto new_directory_page = reinterpret_cast<ExtendibleHTableDirectoryPage *>(new_directory_page_data->GetData());

  // 使用新的目录页进行插入
  uint32_t new_bucket_idx = new_directory_page->HashToBucketIndex(hash);
  bool insert_result = InsertToNewBucket(new_directory_page, new_bucket_idx, key, value);

  // 完成操作，释放页面
  bpm_->UnpinPage(new_directory_page_id, insert_result); // 如果插入成功，标记页面为脏
  return insert_result;
}

//将给定的键值对插入到新的桶页中
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::InsertToNewBucket(ExtendibleHTableDirectoryPage *directory, uint32_t bucket_idx,
                                                          const K &key, const V &value) -> bool {
  // 获取桶页面ID
  page_id_t bucket_page_id = directory->GetBucketPageId(bucket_idx);
  // 获取桶页面

  auto bucket_page_data = bpm_->FetchPage(bucket_page_id);
  if (bucket_page_data == nullptr) {
    throw Exception("Failed to fetch bucket page");
  }
  auto bucket_page = reinterpret_cast<ExtendibleHTableBucketPage<K, V, KC> *>(bucket_page_data->GetData());
  // 尝试插入键值对
  bool insert_result = bucket_page->Insert(key, value, cmp_);

  // 完成操作，释放页面
  bpm_->UnpinPage(bucket_page_id, insert_result); // 如果插入成功，标记页面为脏

  return insert_result;
}

//根据桶的分裂情况更新目录中的映射，确保目录中的每个条目都正确指向了对应的桶
template <typename K, typename V, typename KC>
void DiskExtendibleHashTable<K, V, KC>::UpdateDirectoryMapping(ExtendibleHTableDirectoryPage *directory,
                                                               uint32_t new_bucket_idx, page_id_t new_bucket_page_id,
                                                               uint32_t new_local_depth, uint32_t local_depth_mask) {
  for (uint32_t i = 0; i < (1U << directory->GetGlobalDepth()); ++i) {
    // 检查目录条目是否需要更新为指向新桶
    // 如果目录项对应的是原桶
    if (directory->GetBucketPageId(i) == directory->GetBucketPageId(new_bucket_idx)) {
      if (i & local_depth_mask) {
        // 如果这个目录项的在新局部深度位上的值为1，应该指向新桶，因为哈希值对应的桶已经发生了分裂
        directory->SetBucketPageId(i, new_bucket_page_id);
        directory->SetLocalDepth(i, new_local_depth);
      } else {
        // 否则，它仍然指向原桶，但其局部深度需要更新
        directory->SetLocalDepth(i, new_local_depth);
      }
    }

  }
}



/*****************************************************************************
 * REMOVE
 *****************************************************************************/
//从哈希表中删除键值对
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::Remove(const K &key, Transaction *transaction) -> bool {
  //计算key的哈希值
  uint32_t hash = Hash(key);

  //先根据header_page_id_获取header页
  auto header_page_data = bpm_->FetchPage(header_page_id_);
  // 获取header_page
  auto header_page = reinterpret_cast<ExtendibleHTableHeaderPage *>(header_page_data->GetData());


  //使用header_page来获取目录索引
  auto directory_index = header_page->HashToDirectoryIndex(hash);
  //用目录索引获取目录页，然后找到目錄頁的ID
  page_id_t directory_page_id = header_page->GetDirectoryPageId(directory_index);
  if(directory_page_id == INVALID_PAGE_ID){
    return false;
  }
  bpm_->UnpinPage(header_page_id_, false); // 取消固定header_page

  auto directory_page_data = bpm_->FetchPage(directory_page_id);
  // 获取目录页
  auto directory_page = reinterpret_cast<ExtendibleHTableDirectoryPage *>(directory_page_data->GetData());
  //使用directory_page来获取目录索引
  auto bucket_index = directory_page->HashToBucketIndex(hash);
  // 根据目录索引找到桶页的ID
  page_id_t bucket_page_id = directory_page->GetBucketPageId(bucket_index);
  if(bucket_page_id == INVALID_PAGE_ID){
    return false;
  }
  bpm_->UnpinPage(directory_page_id, false);// 取消固定directory_page_id

  auto bucket_page_data = bpm_->FetchPage(bucket_page_id);
  //获取桶页
  auto bucket_page = reinterpret_cast<ExtendibleHTableBucketPage<K, V, KC> *>(bucket_page_data->GetData());
  //  调用桶页的删除方法，尝试删除键值对
  if (bucket_page->Remove(key, cmp_)) {
    // 如果删除成功，则进入下一步处理；否则，直接跳过后续处理。
    // 检查桶是否为空
    if(bucket_page->IsEmpty()) {
      // 如果桶为空，调用 bpm_->DeletePage(bucket_page_id) 删除桶页
      bpm_->DeletePage(bucket_page_id);
      // 同时，减少目录页中对应桶的局部深度
      directory_page->DecrLocalDepth(bucket_index);
      // 获取分裂位置的索引 split_index
      auto split_index = directory_page->GetSplitImageIndex(bucket_index);
      // 如果分裂位置的局部深度比当前桶的局部深度大 1，则进行合并操作
      if (directory_page->GetLocalDepth(split_index) == (directory_page->GetLocalDepth(bucket_index)+1)) {
        // 减少分裂位置的局部深度
        directory_page->DecrLocalDepth(split_index);
        // 更新目录页中的桶页索引，将当前桶的索引指向分裂位置的桶，同时删除分裂位置的桶
        for (uint32_t i = 0; i < (1U << directory_page->GetGlobalDepth()); ++i) {
          if (directory_page->GetBucketPageId(i) == bucket_page_id) {
            directory_page->SetBucketPageId(i, directory_page->GetBucketPageId(bucket_index));
          }
        }
        directory_page->SetBucketPageId(bucket_index, INVALID_PAGE_ID);
        bpm_->DeletePage(bucket_page_id);
        bpm_->UnpinPage(directory_page_id, false);
      }
      //  检查目录页是否可以收缩
      if (directory_page->CanShrink()) {
        // 如果目录页可以收缩，那么就收缩
        auto ole_global_depth = directory_page->GetGlobalDepth();
        // 减少目录页的全局深度
        directory_page->DecrGlobalDepth();
        auto new_global_depth = directory_page->GetGlobalDepth();
        for (uint32_t i = (1U << new_global_depth); i < (1U << ole_global_depth); ++i) {
          // 将未使用的桶页索引设置为无效页
          directory_page->SetBucketPageId(i, INVALID_PAGE_ID);
        }
      }
    }
    // 如果成功删除，标记页面为脏
    bpm_->UnpinPage(bucket_page_id, true);
    return true;
  }
  return false;
}

template class DiskExtendibleHashTable<int, int, IntComparator>;
template class DiskExtendibleHashTable<GenericKey<4>, RID, GenericComparator<4>>;
template class DiskExtendibleHashTable<GenericKey<8>, RID, GenericComparator<8>>;
template class DiskExtendibleHashTable<GenericKey<16>, RID, GenericComparator<16>>;
template class DiskExtendibleHashTable<GenericKey<32>, RID, GenericComparator<32>>;
template class DiskExtendibleHashTable<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub

