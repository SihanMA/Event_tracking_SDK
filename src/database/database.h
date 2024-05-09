#pragma once

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

namespace buried {

class BuriedDbImpl;
class BuriedDb {
 public:
  struct Data {
    int32_t id;
    int32_t priority;
    uint64_t timestamp;
    std::vector<char> content;
  };

 public:
  BuriedDb(std::string db_path);

  ~BuriedDb();

  void InsertData(const Data& data);

  void DeleteData(const Data& data);

  void DeleteDatas(const std::vector<Data>& datas);

  std::vector<Data> QueryData(int32_t limit);

 private:
  std::unique_ptr<BuriedDbImpl> impl_; //该设计模式（通常称为Pimpl，即Pointer to Implementation）
                                        // 有助于减少编译依赖和隐藏实现细节，从而使接口与实现分离，提高了代码的可维护性。
};

}  // namespace buried
