//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan) 
                : AbstractExecutor(exec_ctx), plan_(plan), table_iter_(table_heap_, RID(), RID()) {}

void SeqScanExecutor::Init() {
        TableInfo *table_info = exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid());

        table_heap_ = dynamic_cast<TableHeap *>(table_info->table_.get());
        // 使用 table_heap_ 的 MakeIterator() 方法获取表的迭代器起始位置
        TableIterator new_iter = table_heap_->MakeIterator();
        table_iter_.SetRID(new_iter.GetRID());
        table_iter_.SetSTOP(new_iter.GetSTOP());
        table_iter_.SetTH(new_iter.GetTH());
    }

auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
        // 遍历结束返回 false
        if (table_iter_.IsEnd()) {
            return false;
        }

        // 填充元组信息及元组的 rid
        *tuple = table_iter_.GetTuple().second; 
        *rid = tuple->GetRid();
        ++table_iter_;
        return true;
    }

}  // namespace bustub
