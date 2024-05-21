#include "common/common_service.h"

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <random>

#include "buried_config.h"

namespace buried {

CommonService::CommonService() { Init(); }

static void WriteRegister(const std::string& key, const std::string& value) {
  HKEY h_key;
  // 在HKEY_CURRENT_USER下打开或创建注册表项
  LONG ret = ::RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Buried", 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                               &h_key, NULL);
  if (ret != ERROR_SUCCESS) {
    return;
  }
  // 将指定的键值对写入注册表
  ret = ::RegSetValueExA(h_key, key.c_str(), 0, REG_SZ,
                         reinterpret_cast<const BYTE*>(value.c_str()),
                         value.size());
  if (ret != ERROR_SUCCESS) {
    return;
  }
  ::RegCloseKey(h_key);
}

static std::string ReadRegister(const std::string& key) {
  HKEY h_key;
  LONG ret = ::RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Buried", 0,
                             KEY_ALL_ACCESS, &h_key);
  if (ret != ERROR_SUCCESS) {
    return "";
  }
  char buf[1024] = {0};
  DWORD buf_size = sizeof(buf);
  ret = ::RegQueryValueExA(h_key, key.c_str(), NULL, NULL,
                           reinterpret_cast<BYTE*>(buf), &buf_size);
  if (ret != ERROR_SUCCESS) {
    return "";
  }
  ::RegCloseKey(h_key);
  return buf;
}

// 先生成一个随机数，作为设备ID，存到设备的某个固定位置，后面每次都从这个位置读取，这样可以保证重启程序或设备，读取的设备ID一致
static std::string GetDeviceId() {
  static constexpr auto kDeviceIdKey = "device_id";
  static std::string device_id = ReadRegister(kDeviceIdKey);
  if (device_id.empty()) {
    device_id = CommonService::GetRandomId();
    WriteRegister(kDeviceIdKey, device_id);
  }
  return device_id;
}

static std::string GetLifeCycleId() {
  static std::string life_cycle_id = CommonService::GetRandomId();
  return life_cycle_id;
}

// 调用系统API获取Windows的系统版本
static std::string GetSystemVersion() {
  OSVERSIONINFOEXA os_version_info;
  ZeroMemory(&os_version_info, sizeof(OSVERSIONINFOEXA));
  // 设置结构体大小
  os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
  ::GetVersionExA(reinterpret_cast<OSVERSIONINFOA*>(&os_version_info));
  std::string system_version =
      std::to_string(os_version_info.dwMajorVersion) + "." +
      std::to_string(os_version_info.dwMinorVersion) + "." +
      std::to_string(os_version_info.dwBuildNumber);
  return system_version;
}

static std::string GetDeviceName() {
  char buf[1024] = {0};
  DWORD buf_size = sizeof(buf);
  ::GetComputerNameA(buf, &buf_size);
  std::string device_name = buf;
  return device_name;
}

// 获取进程的运行时间，直接调用API。不过需要对API获取的结果进行转换
std::string CommonService::GetProcessTime() {
    // 获取当前进程ID
  DWORD pid = ::GetCurrentProcessId();
  HANDLE h_process =
      ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (h_process == NULL) {
    return "";
  }
  FILETIME create_time;
  FILETIME exit_time;
  FILETIME kernel_time;
  FILETIME user_time;
  // 获取进程创建时间、退出时间、内核时间和用户时间
  BOOL ret = ::GetProcessTimes(h_process, &create_time, &exit_time,
                               &kernel_time, &user_time);
  ::CloseHandle(h_process);
  if (ret == 0) {
    return "";
  }

  FILETIME create_local_time;
  ::FileTimeToLocalFileTime(&create_time, &create_local_time);

  SYSTEMTIME create_sys_time;
  ::FileTimeToSystemTime(&create_local_time, &create_sys_time);
  char buf[128] = {0};
  // year month day hour minute second millisecond
  sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", create_sys_time.wYear,
            create_sys_time.wMonth, create_sys_time.wDay, create_sys_time.wHour,
            create_sys_time.wMinute, create_sys_time.wSecond,
            create_sys_time.wMilliseconds);
  return buf;
}

std::string CommonService::GetNowDate() {
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  return std::ctime(&t);
}

std::string CommonService::GetRandomId() {
  static constexpr size_t len = 32;
  static constexpr auto chars =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  // 定义一个静态的64位Mersenne Twister 随机数生成器，使用random_device初始化
  static std::mt19937_64 rng{std::random_device{}()};
  // 定义一个静态的均匀分布 dist，范围从0到60（字符数组 chars 的索引范围）。
  static std::uniform_int_distribution<size_t> dist{0, 61};
  std::string result;
  result.reserve(len);
  std::generate_n(std::back_inserter(result), len,
                  [&]() { return chars[dist(rng)]; });
  return result;
}

void CommonService::Init() {
  system_version = GetSystemVersion();
  device_name = GetDeviceName();
  device_id = GetDeviceId();
  buried_version = PROJECT_VER;
  lifecycle_id = GetLifeCycleId();
}

}  // namespace buried
