//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include <algorithm>
#include "common/exception.h"
namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) { MaxSize = num_frames; }

//查找具有最大后向k距离的页面，并淘汰该页面。只有标记为“可淘汰”的页面才能被淘汰。
bool LRUKReplacer::Evict(frame_id_t *frame_id) {
  std::lock_guard<std::mutex> lock(latch_);

  // 如果没有可以驱逐的元素
  if (Size() == 0) {
    return false;
  }

  // 看不满k次历史列表里，有无帧可以删除
  for (auto it = unfull_frame_.rbegin(); it != unfull_frame_.rend(); it++) {
    auto frame = *it;
    if (evictable_[frame]) {  // 如果可以被删除
      record_count_[frame] = 0;
      unfull_hist_map.erase(frame); //从未满的缓存队列的历史映射 unfull_hist_map 中删除该页面。
      unfull_frame_.remove(frame);  //从不满k次的页队列中删除该页面
      *frame_id = frame;  //将该页面的 ID 存储在 frame_id 中
      curr_size_--;
      alltime[frame].clear(); //清空存储在 alltime 映射中键为 frame 的页面的时间戳记录
      return true;
    }
  }

  // 看已满k次缓存队列里有无帧可以删除
  for (auto it = full_frame_.begin(); it != full_frame_.end(); it++) {
    auto frame = (*it).first;
    if (evictable_[frame]) {  // 如果可以被删除
      record_count_[frame] = 0;
      full_frame_.erase(it);  //从已满k次的页队列中删除该页面
      full_hist_map.erase(frame); //从已满的缓存队列的历史映射 full_hist_map 中删除该页面。
      *frame_id = frame;
      curr_size_--;
      alltime[frame].clear(); //清空存储在 alltime 映射中键为 frame 的页面的时间戳记录
      return true;
    }
  }

  return false;
}

//记录给定帧ID在当前时间戳被访问的事件
void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {
  std::lock_guard<std::mutex> lock(latch_);

  // 如果frame_id大于replacer_size_，抛出异常
  if (frame_id > static_cast<frame_id_t>(replacer_size_)) {
    throw std::exception();
  }

  // 增加当前时间戳
  current_timestamp_++;
  record_count_[frame_id]++;
  auto cnt = record_count_[frame_id];
  alltime[frame_id].push_back(current_timestamp_);

  // 如果是新加入的记录
  if (cnt == 1) {
    //若当前缓存已满
    if (curr_size_ == MaxSize) {
      frame_id_t frame;
      Evict(&frame);  //驱逐出一个页面
    }
    evictable_[frame_id] = true;  //将该页面标记为可驱逐
    curr_size_++;
    unfull_frame_.push_front(frame_id); //将该页面添加到未满的缓存队列 unfull_frame_ 的最前面
    unfull_hist_map[frame_id] = unfull_frame_.begin();  //更新未满的缓存队列的历史映射 unfull_hist_map，将页面 ID 映射到其在队列中的位置
  }

  // 如果记录达到k次，则需要从新队列中加入到老队列中
  if (cnt == k_) {
    unfull_frame_.erase(unfull_hist_map[frame_id]);  // 从新队列中删除
    unfull_hist_map.erase(frame_id);

    auto kth_time = alltime[frame_id].front();  // 获取当前页面的倒数第k次出现的时间
    k_time new_cache(frame_id, kth_time); //创建一个新的 k_time 对象，其中包含页面 ID 和倒数第 k 次访问的时间戳
    auto it = upper_bound(full_frame_.begin(), full_frame_.end(), new_cache, CompareTime);  // 通过比较两个页面的倒数第k次出现时间找到该插入的位置，返回一个迭代器，指向序列中第一个大于 new_cache 的元素
    it = full_frame_.insert(it, new_cache); //将该页面插入到已满的缓存队列 full_frame_ 中,it 被重新赋值为插入操作后新元素的迭代器位置
    full_hist_map[frame_id] = it; //更新已满的缓存队列的历史映射 full_hist_map
    return;
  }

  // 如果记录在k次以上，需要将该frame放到指定的位置
  if (cnt > k_) {
    alltime[frame_id].erase(alltime[frame_id].begin()); //删除该页面访问历史记录中的最早的时间戳，以确保只保留最近的 k 次访问的时间戳
    full_frame_.erase(full_hist_map[frame_id]);  // 去除原来的位置
    auto kth_time = alltime[frame_id].front();       // 获取当前页面的倒数第k次出现的时间
    k_time new_cache(frame_id, kth_time);

    auto it = upper_bound(full_frame_.begin(), full_frame_.end(), new_cache, CompareTime);  // 找到该插入的位置
    it = full_frame_.insert(it, new_cache);
    full_hist_map[frame_id] = it;
    return;
  }
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  std::lock_guard<std::mutex> lock(latch_);

  // 如果该frame_id的记录次数为0，则直接返回
  if (record_count_[frame_id] == 0) {
    return;
  }

  // 获取当前frame_id的evictable状态
  auto status = evictable_[frame_id];

  // 更新evictable状态
  evictable_[frame_id] = set_evictable;

  // 如果状态由true改为false，减少当前和最大尺寸
  if (status && !set_evictable) {
    MaxSize--;
    curr_size_--;
  }
  // 如果状态由false改为true，增加当前和最大尺寸
  if (!status && set_evictable) {
    MaxSize++;
    curr_size_++;
  }
}

//从替换器中移除一个可淘汰的页面，以及其访问历史
void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(latch_);

  // 如果frame_id大于replacer_size_，抛出异常
  if (frame_id > static_cast<frame_id_t>(replacer_size_)) {
    throw std::exception();
  }

  // 获取frame_id的记录次数
  auto cnt = record_count_[frame_id];

  // 如果记录次数为0，直接返回
  if (cnt == 0) {
    return;
  }

  // 如果frame_id不可驱逐，抛出异常
  if (!evictable_[frame_id]) {
    throw std::exception();
  }

  // 如果记录次数小于k_
  if (cnt < k_) {
    unfull_frame_.erase(unfull_hist_map[frame_id]);
    unfull_hist_map.erase(frame_id);
    record_count_[frame_id] = 0;
    alltime[frame_id].clear();
    curr_size_--;
  } else {
    full_frame_.erase(full_hist_map[frame_id]);
    full_hist_map.erase(frame_id);
    record_count_[frame_id] = 0;
    alltime[frame_id].clear();
    curr_size_--;
  }
}

//返回LRUKReplacer对象中当前节点的数量
auto LRUKReplacer::Size() -> size_t { return curr_size_; }

//比较倒数第k次出现时间的大小
auto LRUKReplacer::CompareTime(const LRUKReplacer::k_time &f1, const LRUKReplacer::k_time &f2) -> bool {
  return f1.second < f2.second;
}
}  // namespace bustub