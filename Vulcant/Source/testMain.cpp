#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>

struct VkDrawIndirectCommandCPU
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct CullParams
{
    uint32_t activeSphereCount;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
};

TEST_CASE("Indirect Draw Struct Sizes", "[indirect]")
{
    REQUIRE(sizeof(VkDrawIndirectCommandCPU) == 16);
    REQUIRE(sizeof(CullParams) == 16);
}

void enforceWorkingDir(std::string exeDir) {
  const size_t last_slash_idx = exeDir.find_last_of("\\/");
  if (std::string::npos != last_slash_idx) {
    exeDir.erase(last_slash_idx + 1);
  }
  std::filesystem::current_path(exeDir);
}

int main(int argc, char* argv[]) {
  enforceWorkingDir(std::string(argv[0]));
  int result = Catch::Session().run(argc, argv);
  return result;
}