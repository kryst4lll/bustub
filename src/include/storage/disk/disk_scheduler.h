//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// disk_scheduler.h
//
// Identification: src/include/storage/disk/disk_scheduler.h
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <future>  // NOLINT
#include <optional>
#include <thread>  // NOLINT

#include "common/channel.h"
#include "storage/disk/disk_manager.h"

namespace bustub {

/**
 * @brief 表示用于执行 DiskManager 写入或读取请求的请求。
 */
struct DiskRequest {
  /** Flag indicating whether the request is a write or a read. */
  bool is_write_;

  /**
   *  指向内存位置起始的指针，其中一个页面正在：
   *   1. 从磁盘读入（读取操作）。
   *   2. 写入到磁盘（写入操作）。
   */
  char *data_;

  /** 读取/写入磁盘的页面的 ID。 */
  page_id_t page_id_;

  /** 用于通知请求发起者请求已完成的回调。 */
  std::promise<bool> callback_;
};

/**
 * @brief DiskScheduler 负责调度磁盘读写操作。
 *
 * 调度请求通过使用适当的 DiskRequest 对象调用
 * DiskScheduler::Schedule()。调度程序维护一个后台工作线程，使用磁盘管理器处理已调度的请求。 后台线程在 DiskScheduler
 * 构造函数中创建，并在其析构函数中加入。
 */
class DiskScheduler {
 public:
  explicit DiskScheduler(DiskManager *disk_manager);
  ~DiskScheduler();

  /**
   * TODO(P1): 添加实现
   *
   * @brief 调度一个请求，供 DiskManager 执行。
   *
   * @param r 要调度的请求。
   */
  void Schedule(DiskRequest r);

  /**
   * TODO(P1): 添加实现
   *
   * @brief 后台工作线程函数，处理已调度的请求。
   *
   * 后台线程需要在 DiskScheduler 存在时处理请求，即该函数在调用 ~DiskScheduler() 之前不应返回。
   * 在那时，您需要确保函数确实返回。
   */
  void StartWorkerThread();

  using DiskSchedulerPromise = std::promise<bool>;

  /**
   * @brief 创建一个 Promise 对象。如果您想实现自己版本的 promise，可以更改此函数，以便我们的测试用例可以使用您的
   * promise 实现。
   *
   * @return std::promise<bool>
   */
  auto CreatePromise() -> DiskSchedulerPromise { return {}; };

 private:
  /** 指向磁盘管理器的指针。 */
  DiskManager *disk_manager_ __attribute__((__unused__));
  /** 一个共享队列，用于并发调度和处理请求。当调用 DiskScheduler 的析构函数时，将 `std::nullopt`
   * 放入队列中，以向后台线程发出停止执行的信号。 */
  Channel<std::optional<DiskRequest>> request_queue_;
  /** 负责将已调度的请求发送给磁盘管理器的后台线程。 */
  std::optional<std::thread> background_thread_;
};
}  // namespace bustub
