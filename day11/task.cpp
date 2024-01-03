#include "args.hpp"
#include "pos2.hpp"
#include <algorithm>
#include <cstddef>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <range/v3/all.hpp>
#include <set>
#include <string>
#include <string_view>
#include <sys/_types/_int64_t.h>
#include <unordered_set>
#include <utility>
#include <vector>

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

  using std::string;
  using std::vector;

  char const galaxy_symbol = '#';

  int width = 0;
  int height = 0;

  auto less_y = [](pos2 l, pos2 r) {
    if (l.y < r.y) {
      return true;
    } else {
      return l.x < r.x;
    }
  };

  vector<pos2> galaxies;

  size_t expansion_factor = 1000000;

  vector<size_t> can_col_expand;
  string line;
  while (std::getline(ifs, line)) {
    if (width == 0) {
      width = line.size();
      can_col_expand = vector<size_t>(width, true);
    }

    // no galaxy in a row
    if (auto it = line.find(galaxy_symbol); it == string::npos) {
      height += expansion_factor;
    } else {
      while (it != string::npos) {
        pos2 glxy(height, it);
        galaxies.push_back(glxy);
        fmt::println("galaxies: {} {}", galaxies.size(), galaxies);
        can_col_expand.at(it) = false;

        it = line.find(galaxy_symbol, it + 1);
      }
      ++height;
    }
  }
  fmt::println("galaxies: {}", galaxies);

  vector<size_t> cols_to_expand =
      ranges::views::enumerate(can_col_expand) |
      ranges::views::filter([](auto const &t) { return std::get<1>(t) == 1; }) |
      ranges::views::transform(
          [](auto const &t) { return static_cast<size_t>(std::get<0>(t)); }) |
      ranges::to_vector;

  ranges::actions::reverse(cols_to_expand);
  fmt::println("expand cols: {}", cols_to_expand);

  width += cols_to_expand.size();

  for (size_t exp_c : cols_to_expand) {
    for (pos2 &glxy : galaxies) {
      if (glxy.y > exp_c) {
        glxy.y += expansion_factor - 1;
      }
    }
  }
  fmt::println("galaxies: {}", galaxies);

  /* for (size_t h = 0; h < height; ++h) { */
  /*   string line(width, '.'); */
  /*   for (size_t w = 0; w < width; ++w) { */
  /*     if (std::find(galaxies.begin(), galaxies.end(), pos2{h, w}) != */
  /*         galaxies.end()) { */
  /*       line[w] = '#'; */
  /*     } */
  /*   } */
  /*   fmt::println("{}", line); */
  /* } */

  size_t distance_total{0};
  for (int i = 0; i < galaxies.size() - 1; ++i) {
    for (int j = i + 1; j < galaxies.size(); ++j) {
      distance_total += std::abs(galaxies[i].x - galaxies[j].x) +
                        std::abs(galaxies[i].y - galaxies[j].y);
    }
  }

  fmt::println("total distance; {}", distance_total);

  return 0;
}
