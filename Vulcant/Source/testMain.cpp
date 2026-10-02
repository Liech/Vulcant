#define CATCH_CONFIG_MAIN
#include <catch2/catch_session.hpp>

#include <filesystem>

void enforceWorkingDir(std::string exeDir) {
  const size_t last_slash_idx = exeDir.find_last_of("\\/");
  if (std::string::npos != last_slash_idx) {
    exeDir.erase(last_slash_idx + 1);
  }
  std::filesystem::current_path(exeDir);
}

void setupVirtualGraphics() {
  if (getenv("VK_DRIVER_FILES") == nullptr && getenv("VK_ICD_FILENAMES") == nullptr) {
    const char* candidates[] = {
      "/opt/google/chrome/vk_swiftshader_icd.json",
      "/usr/share/vulkan/icd.d/lvp_icd.x86_64.json",
      "/usr/share/vulkan/icd.d/lvp_icd.i686.json",
      "/etc/vulkan/icd.d/lvp_icd.x86_64.json"
    };
    for (const char* path : candidates) {
      if (std::filesystem::exists(path)) {
#if defined(_WIN32)
        _putenv_s("VK_DRIVER_FILES", path);
        _putenv_s("VK_ICD_FILENAMES", path);
#else
        setenv("VK_DRIVER_FILES", path, 1);
        setenv("VK_ICD_FILENAMES", path, 1);
#endif
        break;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  enforceWorkingDir(std::string(argv[0]));
  setupVirtualGraphics();
  int result = Catch::Session().run(argc, argv);
  return result;
}