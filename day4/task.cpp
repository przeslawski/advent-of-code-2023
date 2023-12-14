#include "args.hpp"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <range/v3/all.hpp>
#include <string_view>
#include <unordered_set>

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

  std::size_t acc{};

  using card_id = int;
  std::vector<std::size_t> points;
  std::unordered_map<card_id, int> copies;

  std::string line;
  while (std::getline(ifs, line)) {

    std::unordered_set<int> numbers;

    line = line | ranges::views::split(':') | ranges::views::drop(1) |
           ranges::views::join | ranges::to<std::string>();

    auto l = ranges::views::transform(
        line | ranges::views::split('|'),
        [](auto &&rng) { return rng | ranges::views::split(' '); });

    auto not_empty = [](auto &&rng) { return ranges::distance(rng) > 0; };
    auto str_rng_to_int = [](auto &&str_rng) {
      auto str = str_rng | ranges::to<std::string>();
      return std::stoi(str);
    };

    auto add_number = [&](int number) { numbers.insert(number); };

    ranges::for_each(l | ranges::views::take(1) | ranges::views::join |
                         ranges::views::filter(not_empty) |
                         ranges::views::transform(str_rng_to_int),
                     add_number);

    auto is_my_number = [&](int number) { return numbers.contains(number); };

    auto hits = l | ranges::views::drop(1) | ranges::views::take(1) |
                ranges::views::join | ranges::views::filter(not_empty) |
                ranges::views::transform(str_rng_to_int) |
                ranges::views::filter(is_my_number);

    auto calc_points = [](int len) {
      if (len == 0) {
        return 0;
      }
      return 0x1 << (len - 1);
    };

    int amount = ranges::distance(hits);
    int card_points = calc_points(amount);

    // part 1
    acc += card_points;

    // part 2
    card_id id = points.size();
    copies[id] += 1;
    int copies_of_this = copies[id];
    points.push_back(card_points * copies_of_this);

    fmt::println("{} matching for card {} which has {} copies", amount, id,
                 copies[id]);
    for (int idx{1}; idx <= amount; ++idx) {
      copies[id + idx] += copies_of_this;
      fmt::println("{} copies of card {}", copies[id + idx], id + idx);
    }
  }

  std::size_t after_copies =
      ranges::accumulate(copies | ranges::views::values, 0);
  fmt::println("has {} points", acc);
  fmt::println("with copies has {} points", after_copies);

  return 0;
}
