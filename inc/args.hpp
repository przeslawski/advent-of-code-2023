#include <filesystem>
#include <string_view>
#include <vector>

using args_t = std::vector<std::string_view>;

inline args_t parse_args(int argc, char **argv) {
  std::vector<std::string_view> args{};
  for (int i = 0; i < argc; ++i) {
    args.push_back({argv[i], std::strlen(argv[i])});
  }

  return args;
}
