#include "args.hpp"
#include "range_split_strs.hpp"
#include <_ctype.h>
#include <algorithm>
#include <cstddef>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <range/v3/all.hpp>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

using seq = std::vector<int64_t>;

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

  std::function<seq::value_type(seq)> extrapolate_next =
      [&](auto &&rng) -> seq::value_type {
    using namespace ranges;
    if (all_of(rng, [](auto val) { return val == 0; })) {
      return 0;
    } else {
      auto back = accumulate(rng | views::reverse | views::take(1), 0);
      return back + extrapolate_next(views::zip(rng | views::drop(1), rng) |
                                     views::transform([](auto t) {
                                       return std::get<0>(t) - std::get<1>(t);
                                     }) |
                                     to_vector);
    }
  };

  std::function<seq::value_type(seq)> extrapolate_prev =
      [&](auto &&rng) -> seq::value_type {
    using namespace ranges;
    if (all_of(rng, [](auto val) { return val == 0; })) {
      return 0;
    } else {
      auto front = accumulate(rng | views::take(1), 0);
      return front - extrapolate_prev(views::zip(rng | views::drop(1), rng) |
                                      views::transform([](auto t) {
                                        return std::get<0>(t) - std::get<1>(t);
                                      }) |
                                      to_vector);
    }
  };

  int64_t next_elements_sum = 0;
  int64_t prev_elements_sum = 0;
  std::string line;
  while (std::getline(ifs, line)) {
    if (line.empty()) {
      continue;
    }

    auto rng = line | split_strs(' ') |
               ranges::views::transform(
                   [](std::string const &str) { return std::stoll(str); }) |
               ranges::to_vector;
    int64_t next = extrapolate_next(rng);
    int64_t prev = extrapolate_prev(rng);
    next_elements_sum += next;
    prev_elements_sum += prev;
    fmt::println("range: {} extrapolated: {}, {}", rng, prev, next);
  }
  fmt::println("next elements summed: {}", next_elements_sum);
  fmt::println("prev elements summed: {}", prev_elements_sum);

  return 0;
}
