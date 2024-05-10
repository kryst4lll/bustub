#pragma once

#include <set>  // for std::set
#include <string>
#include <tuple>  // for std::tuple
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bustub {

/** @brief 唯一标识类型。 */
using uid_t = int64_t;

/** @brief 观察到的移除集合数据类型。 */
template <typename T>
class ORSet {
 public:
  ORSet() = default;

  /**
   * @brief 检查集合中是否存在元素。
   *
   * @param elem 要检查的元素
   * @return 如果元素在集合中，则返回true，否则返回false
   */
  auto Contains(const T &elem) const -> bool;

  /**
   * @brief 向集合中添加一个元素。
   *
   * @param elem 要添加的元素
   * @param uid 与添加操作关联的唯一标记。
   */
  void Add(const T &elem, uid_t uid);

  /**
   * @brief 如果存在，则从集合中移除一个元素。
   *
   * @param elem 要移除的元素。
   */
  void Remove(const T &elem);

  /**
   * @brief 合并另一个ORSet的更改。
   *
   * @param other 另一个ORSet
   */
  void Merge(const ORSet<T> &other);

  /**
   * @brief 获取集合中的所有元素。
   *
   * @return 集合中的所有元素。
   */
  auto Elements() const -> std::vector<T>;

  /**
   * @brief 获取集合的字符串表示。
   *
   * @return 集合的字符串表示。
   */
  auto ToString() const -> std::string;

 private:
  std::set<std::tuple<T, uid_t>> set_add;  // 已添加元素集合，存储了已经添加的元素以及它们的唯一标识符。
  std::set<std::tuple<T, uid_t>> set_remove;  // 已移除元素集合，存储了已经移除的元素以及它们的唯一标识符。
};

}  // namespace bustub
