#include "args.hpp"
#include <algorithm>
#include <cassert>
#include <charconv>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <functional>
#include <numeric>
#include <optional>
#include <range/v3/all.hpp>
#include <string_view>
#include <unordered_set>

struct position {
  int x{};
  int y{};
  bool operator==(position const &other) const {
    return x == other.x and y == other.y;
  }
};

struct area {
  position origin;
  int width;
  int height;

  area() = default;
  area(position orig, int len) : origin(orig), width(len), height(3) {}
};

struct Part {
  area area;
  int val;
};

struct Symbol {
  bool is_gear;
  std::vector<int> adj_part_nums;
};

namespace hash {

inline void hash_combine(std::size_t &seed) {}

template <typename T, typename... Rest>
inline void hash_combine(std::size_t &seed, const T &v, Rest... rest) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  hash_combine(seed, rest...);
}

} // namespace hash

namespace std {
template <> struct hash<position> {
  size_t operator()(position const &pos) const {
    size_t hash = 0;
    ::hash::hash_combine(hash, pos.x, pos.y);
    return hash;
  }
};

} // namespace std

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
  /* std::vector<std::string> schematic; */

  auto is_digit = [](char c) { return c >= '0' and c <= '9'; };
  auto is_empty = [](char c) { return c == '.'; };

  std::unordered_map<position, Symbol> symbols;

  std::vector<Part> parts;
  std::string line;
  int row = 0;
  while (std::getline(ifs, line)) {
    for (int col = 0; col < line.size(); ++col) {

      char c = line[col];
      if (is_digit(c)) {

        position orig{row - 1, col - 1};
        int val = 0;

        while (col < line.size() and is_digit(line[col])) {
          val = val * 10 + line[col++] - '0';
        }

        int width = (col--) - orig.y + 1;
        parts.push_back({area{orig, width}, val});

      } else if (!is_empty(c)) {
        symbols[{row, col}] = {c == '*'};
      }
    }
    ++row;
  }

  auto has_adjacent_symbol = [&symbols](Part const &part) -> bool {
    // TODO: try to do it with ranges
    // ranges::views::iota(part.area.origin.y) |
    // ranges::views::take(part.area.width)
    //

    auto is_part = false;

    for (int i = 0; i < part.area.width; ++i) {
      for (int j = 0; j < part.area.height; ++j) {
        if (auto it = symbols.find(
                position{part.area.origin.x + j, part.area.origin.y + i});
            it != symbols.end()) {

          if (it->second.is_gear) {
            auto sym = *it;
            it->second.adj_part_nums.push_back(part.val);
          }

          is_part = true;
        }
      }
    }
    return is_part;
  };

  // part 1
  size_t sum =
      ranges::accumulate(parts | ranges::views::filter(has_adjacent_symbol) |
                             ranges::views::transform(&Part::val),
                         0);

  // part 2
  size_t gear_ratio_sum = ranges::accumulate(
      symbols | ranges::views::values |
          ranges::views::filter([](Symbol const &sym) {
            return sym.is_gear and sym.adj_part_nums.size() == 2;
          }) |
          ranges::views::transform([](auto const &sym) {
            return sym.adj_part_nums[0] * sym.adj_part_nums[1];
          }),
      0);

  fmt::println("sum of all part numbers {}", sum);
  fmt::println("sum of all gear ratios {}", gear_ratio_sum);

  return 0;
}
