#pragma once

#include <stdint.h>

#include <memory>
#include <string>

#include "common/common_service.h"

namespace spdlog {
class logger;
}

namespace buried {

struct BuriedData {
  std::string title;
  std::string data;
  uint32_t priority;
};

// pimpl模式，相关实现都放在源文件，不暴露到外边，重点学习！！！
class BuriedReportImpl;
class BuriedReport {
 public:
  BuriedReport(std::shared_ptr<spdlog::logger> logger,
               CommonService common_service, std::string work_path);

  ~BuriedReport();

  // 上报模块开关，调用此接口会自动获取数据并上报
  void Start();

  // 外部调用方想要上报某条数据就调用该接口，模块内部会自动挑时机上报
  void InsertData(const BuriedData& data);

 private:
  std::unique_ptr<BuriedReportImpl> impl_;
};

}  // namespace buried
