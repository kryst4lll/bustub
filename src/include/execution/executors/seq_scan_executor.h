//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.h
//
// Identification: src/include/execution/executors/seq_scan_executor.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/seq_scan_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * 顺序扫描执行器执行顺序表扫描操作。
 */
class SeqScanExecutor : public AbstractExecutor {
 public:
  /**
   * 构造一个新的 SeqScanExecutor 实例。
   * @param exec_ctx 执行上下文
   * @param plan 待执行的顺序扫描计划
   */
  SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan);

  /** 初始化顺序扫描 */
  void Init() override;

  /**
   * 返回顺序扫描中的下一个元组。
   * @param[out] tuple 扫描产生的下一个元组
   * @param[out] rid 扫描产生的下一个元组的 RID
   * @return 如果产生了一个元组，则为 `true`，如果没有更多元组，则为 `false`
   */
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return 顺序扫描的输出模式 */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

 private:
  /** 待执行的顺序扫描计划节点 */
  const SeqScanPlanNode *plan_;
  /** 待扫描表的表堆 */
  TableHeap *table_heap_;
  /** 待扫描表的迭代器 */
  TableIterator table_iter_;
};
}  // namespace bustub

