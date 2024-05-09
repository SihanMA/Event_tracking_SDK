// 对外暴露C语言接口的头文件；定义了如何创建和管理埋点，以及启动和上报。

#pragma once

#include <stdint.h>

#define BURIED_EXPORT __declspec(dllexport)

extern "C" { // 确保在C++代码中定义的函数可以被C语言或其他语言调用时，保持C语言的链接方式，避免名字改编（name mangling）

typedef struct Buried Buried; // 前向声明，意味着具体结构在这个文件中没有定义，只是告诉编译器这是一个存在的类型，是一种常见的封装技术，用以隐藏细节

struct BuriedConfig {
  const char* host;
  const char* port;
  const char* topic;
  const char* user_id;
  const char* app_version;
  const char* app_name;
  const char* custom_data; // Json的字符串
};

BURIED_EXPORT Buried* Buried_Create(const char* work_dir);

BURIED_EXPORT void Buried_Destroy(Buried* buried);

// 开启埋点上报能力
BURIED_EXPORT int32_t Buried_Start(Buried* buried, BuriedConfig* config);

// 上报具体数据
BURIED_EXPORT int32_t Buried_Report(Buried* buried, const char* title,
                                    const char* data, uint32_t priority);
}
