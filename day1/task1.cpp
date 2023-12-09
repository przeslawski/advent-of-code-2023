#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <range/v3/all.hpp>
#include <string>
#include <string_view>
#include <vector>

int main(int argc, char *argv[]) {

  std::vector<std::string_view> args{};
  for (int i = 0; i < argc; ++i) {
    args.push_back({argv[i], std::strlen(argv[i])});
  }

  if (argc < 2) {
    fmt::println("usage: {} <input_file>",
                 std::filesystem::path(args[0]).filename().string());
    return -1;
  }

  std::filesystem::path path(args[1]);
  std::ifstream ifs;

  ifs.open(path);

  if (!ifs.good()) {
    fmt::println("cannot open file: {}", args[1]);
    return -1;
  }

  std::string line;
  std::size_t acc{};

  while (ifs >> line) {

    auto is_digit = [](char c) -> bool { return c >= '0' && c <= '9'; };

    int first = ranges::accumulate(line | ranges::views::filter(is_digit) |
                                       ranges::views::take(1),
                                   -static_cast<int>('0'));
    int last = ranges::accumulate(line | ranges::views::reverse |
                                      ranges::views::filter(is_digit) |
                                      ranges::views::take(1),
                                  -static_cast<int>('0'));

    acc += (first * 10) + last;
  }

  fmt::println("{}", acc);

  return 0;
}
