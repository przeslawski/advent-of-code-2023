#include "args.hpp"
#include <cstddef>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <range/v3/all.hpp>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace day6 {

struct race_info {
  size_t time;
  size_t record;
};
} // namespace day6

auto split_strs = [](auto &&pattern) {
  using namespace ranges;
  return views::split(pattern) | views::transform([](auto p) {
           auto c = p | views::common;
           return std::string(c.begin(), c.end());
         });
};

int main(int argc, char **argv) {

  auto args = parse_args(argc, argv);

  if (args.size() < 2) {
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

  std::string time_line;
  std::string dist_line;
  std::getline(ifs, time_line);
  std::getline(ifs, dist_line);

  using namespace ranges;
  using namespace ranges::views;

  auto calc_win_variants = [](day6::race_info ri) {
    size_t cnt = 0;
    for (int i = 1; i <= ri.time; ++i) {
      size_t const speed = i;
      size_t const time = ri.time - i;
      if (time * speed > ri.record) {
        ++cnt;
      }
    }

    return cnt;
  };

  // part 1
  {
    auto races = zip(time_line | split_strs(' ') | filter(&std::string::size),
                     dist_line | split_strs(' ') | filter(&std::string::size)) |
                 drop(1) | views::transform([](auto tup) {
                   auto time = std::stoull(std::get<0>(tup));
                   auto dist = std::stoull(std::get<1>(tup));
                   return day6::race_info{time, dist};
                 }) |
                 views::transform(calc_win_variants);

    size_t ans = fold_left(races, 1, [](size_t l, size_t r) { return l * r; });

    fmt::println("part1: {}", ans);
  }

  // part 2
  {

    auto races = zip(time_line | filter(isnumber) | split_strs('\n'),
                     dist_line | filter(isnumber) | split_strs('\n')) |
                 views::transform([](auto tup) {
                   size_t time = std::stoull(std::get<0>(tup));
                   size_t dist = std::stoull(std::get<1>(tup));
                   return day6::race_info{time, dist};
                 }) |
                 views::transform(calc_win_variants);

    size_t ans = fold_left(races, 1, [](size_t l, size_t r) { return l * r; });

    fmt::println("part2: {}", ans);
  }

  return 0;
}
