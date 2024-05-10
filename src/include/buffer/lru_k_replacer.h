//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.h
//
// Identification: src/include/buffer/lru_k_replacer.h
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <limits>
#include <list>
#include <mutex>  // NOLINT
#include <unordered_map>
#include <vector>

#include "common/config.h"
#include "common/macros.h"

namespace bustub {

enum class AccessType { Unknown = 0, Lookup, Scan, Index };

class LRUKNode {
 private:
  /** 记录该页面最近K个时间戳的历史。最不近的时间戳存储在最前面。 */
  // 如果您开始使用它们，请移除 maybeunused。随意根据需要更改成员变量。
  // std::list history;
  size_t k_;
  frame_id_t fid_;

 public:
  std::list<size_t> history_;
  bool is_evictable_{false};
};

/**

1.LRUKReplacer 实现了LRU-k替换策略。
*
2.LRU-k算法会淘汰具有最大后向k距离的页面。
3.后向k距离是指当前时间戳与第k次之前访问的时间戳之间的差。
*
4.如果一个页面的历史引用少于k次，则将+inf作为其后向k距离。当多个页面的后向k距离为+inf时，
5.将使用经典的LRU算法来选择受害者。
*/

class LRUKReplacer {
 public:
  /**
     *

  1.TODO(P1): 添加实现
  *
  2.@brief 创建一个新的 LRUKReplacer。
  3.@param num_frames LRUKReplacer 需要存储的最大页面数
  */

  explicit LRUKReplacer(size_t num_frames, size_t k);

  DISALLOW_COPY_AND_MOVE(LRUKReplacer);

  /**
   * TODO(P1): 添加实现
   *
   * @brief 销毁 LRUReplacer。
   */
  ~LRUKReplacer() = default;

  /**
   * TODO(P1): 添加实现
   *
   * @brief 查找具有最大后向k距离的页面，并淘汰该页面。只有标记为“可淘汰”的页面才能被淘汰。
   *
   * 如果一个页面的历史引用少于k次，则将+inf作为其后向k距离。
   * 如果多个页面的后向k距离为inf，则根据LRU基于最早的时间戳淘汰页面。
   *
   * 成功淘汰一个页面应该将替换器的大小减小，并移除页面的访问历史。
   *
   * @param[out] frame_id 被淘汰的页面的ID。
   * @return 如果成功淘汰了一个页面，则返回true；如果没有页面可以被淘汰，则返回false。
   */

  auto Evict(frame_id_t *frame_id) -> bool;

  /**
   * TODO(P1): 添加实现
   *
   * @brief 记录给定帧ID在当前时间戳被访问的事件。
   * 如果帧ID之前没有被访问过，则为其创建一个新的访问历史记录。
   *
   * 如果帧ID无效（即大于replacer_size_），则抛出异常。您还可以使用BUSTUB_ASSERT来中止进程，如果帧ID无效。
   *
   * @param frame_id 接收新访问的帧的ID。
   * @param access_type 接收到的访问类型。此参数仅在领导者测试中需要。
   */

  void RecordAccess(frame_id_t frame_id, AccessType access_type = AccessType::Unknown);

  /**
   * TODO(P1): 添加实现
   *
   * @brief 切换页面是否可淘汰的状态。此函数还控制替换器的大小。请注意，大小等于可淘汰条目的数量。
   *
   * 如果页面之前是可淘汰的，并且要设置为不可淘汰，则大小应该减少。如果页面之前是不可淘汰的，并且要设置为可淘汰，则大小应该增加。
   *
   * 如果页面ID无效，则抛出异常或中止进程。
   *
   * 对于其他情况，此函数应在不修改任何内容的情况下终止。
   *
   * @param frame_id 要修改“可淘汰”状态的页面的ID
   * @param set_evictable 给定页面是否可淘汰
   */

  void SetEvictable(frame_id_t frame_id, bool set_evictable);

  /**
   * TODO(P1): 添加实现
   *
   * @brief 从替换器中移除一个可淘汰的页面，以及其访问历史。
   * 如果移除成功，该函数还应该减少替换器的大小。
   *
   * 请注意，这与淘汰页面是不同的，淘汰页面总是移除具有最大后向k距离的页面。该函数移除指定的页面ID，无论其后向k距离是多少。
   *
   * 如果在非可淘汰的页面上调用Remove，则抛出异常或中止进程。
   *
   * 如果未找到指定的页面，则直接从该函数返回。
   *
   * @param frame_id 要移除的页面的ID
   */
  void Remove(frame_id_t frame_id);

  /**
   * TODO(P1): 添加实现
   *
   * @brief 返回替换器的大小，该大小跟踪可淘汰的页面数量。
   *
   * @return size_t
   */

  auto Size() -> size_t;

 private:
  // TODO(student): implement me! You can replace these member variables as you like.
  // Remove maybe_unused if you start using them.
  std::unordered_map<frame_id_t, LRUKNode> node_store_;
  size_t current_timestamp_{0};
  size_t curr_size_{0};
  size_t replacer_size_;
  size_t k_;
  std::mutex latch_;

  size_t MaxSize; //最多可驱逐页面数量
  using timestamp = std::list<size_t>;  //记录单个页时间戳的列表
  using k_time = std::pair<frame_id_t, size_t>; //页号对应的第k次的时间戳
  std::unordered_map<frame_id_t, timestamp> alltime;        //用于记录所有页的时间戳
  std::unordered_map<frame_id_t, size_t> record_count_;  //用于记录,访问了多少次
  std::unordered_map<frame_id_t, bool> evictable_;       //用于记录是否可以被驱逐

  std::list<frame_id_t> unfull_frame_;  //用于记录不满k次的页
  std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> unfull_hist_map;//页号到历史访问队列的哈希表,将页面号映射到历史访问队列中相应页面的位置

  std::list<k_time> full_frame_;  //用于记录到达k次的页
  std::unordered_map<frame_id_t, std::list<k_time>::iterator> full_hist_map;  //页号到缓存队列的哈希表,将页面号映射到缓存队列中相应页面的位置
  static auto CompareTime(const k_time &f1, const k_time &f2) -> bool; //比较时间大小
}; 

}  // namespace bustub