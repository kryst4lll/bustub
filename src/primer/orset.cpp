#include "primer/orset.h"
#include <algorithm>
#include <string>
#include <vector>
#include "common/exception.h"
#include "fmt/format.h"

namespace bustub {

template <typename T>
auto ORSet<T>::Contains(const T &elem) const -> bool {
  auto it = this->set_add.begin();
  // 遍历set_add中的元素，解包temp与elem匹配
  for (it = this->set_add.begin(); it != this->set_add.end(); it++) {
    T temp;
    uid_t uid;
    std::tie(temp, uid) = *it;
    if (elem == temp) {
      return true;
    }
  }
  return false;
  // TODO(student): Implement this
  throw NotImplementedException("ORSet<T>::Contains is not implemented");
}

template <typename T>
void ORSet<T>::Add(const T &elem, uid_t uid) {
  // 标记与元素一起存储为元组（e，n）插入
  this->set_add.insert(std::make_tuple(elem, uid));
  return;
  // TODO(student): Implement this
  throw NotImplementedException("ORSet<T>::Add is not implemented");
}

template <typename T>
void ORSet<T>::Remove(const T &elem) {
  auto it = this->set_add.begin();
  // 遍历set_add中的元素，解包temp与elem匹配
  for (it = this->set_add.begin(); it != this->set_add.end(); it++) {
    T temp;
    uid_t uid;
    std::tie(temp, uid) = *it;
    if (temp == elem)  // find elem and set the corrsepending tuple to set_add
    {
      this->set_add.erase(std::make_tuple(temp, uid));
      this->set_remove.insert(std::make_tuple(temp, uid));
      continue;
    }
  }
  return;
  // TODO(student): Implement this
  throw NotImplementedException("ORSet<T>::Remove is not implemented");
}

template <typename T>
void ORSet<T>::Merge(const ORSet<T> &ot) {
  // 合并已添加元素集合
  auto it = ot.set_add.begin();
  for (it = ot.set_add.begin(); it != ot.set_add.end(); it++) {
    this->set_add.insert(*it);
  }
  // 合并已移除元素集合
  for (it = ot.set_remove.begin(); it != ot.set_remove.end(); it++) {
    this->set_remove.insert(*it);
  }
  // 从已添加元素集合 set_add 中移除对应的元素
  for (it = this->set_remove.begin(); it != this->set_remove.end(); it++) {
    T temp;
    uid_t uid;
    std::tie(temp, uid) = *it;
    this->set_add.erase(std::make_tuple(temp, uid));
  }
  return;
  // TODO(student): Implement this
  throw NotImplementedException("ORSet<T>::Merge is not implemented");
}

template <typename T>
auto ORSet<T>::Elements() const -> std::vector<T> {
  std::vector<T> vec;
  auto it = this->set_add.begin();
  // 遍历set_add中的元素，解包temp插入vec中
  for (it = this->set_add.begin(); it != this->set_add.end(); it++) {
    T temp;
    uid_t uid;
    std::tie(temp, uid) = *it;
    vec.push_back(temp);
  }

  return vec;

  // TODO(student): Implement this
  throw NotImplementedException("ORSet<T>::Elements is not implemented");
}

template <typename T>
auto ORSet<T>::ToString() const -> std::string {
  auto elements = Elements();
  std::sort(elements.begin(), elements.end());
  return fmt::format("{{{}}}", fmt::join(elements, ", "));
}

template class ORSet<int>;
template class ORSet<std::string>;

}  // namespace bustub
