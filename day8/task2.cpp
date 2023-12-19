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
#include <numeric>
#include <range/v3/all.hpp>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace day8 {

using node = std::string;

namespace graph {

struct graph {

  bool add_node(node vertex, node left, node right) {
    return adj_list.insert({vertex, {left, right}}).second;
  };

  std::unordered_map<node, std::pair<node, node>> adj_list;
};

node left_of(graph const &g, node const &n) {
  if (!g.adj_list.contains(n)) {
    fmt::println("oops! {} not in graph {}", n, g.adj_list);
    assert(false);
  }
  return g.adj_list.at(n).first;
}
node right_of(graph const &g, node const &n) {
  assert(g.adj_list.contains(n));
  return g.adj_list.at(n).second;
}

} // namespace graph

} // namespace day8

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

  std::string line;

  std::string branches;
  std::getline(ifs, branches);

  auto parse_vertex = []() {};

  day8::graph::graph g;

  std::vector<day8::node> starting_nodes;

  while (std::getline(ifs, line)) {
    if (line.empty()) {
      continue;
    }

    auto letters =
        line | ranges::views::filter([](char c) { return isupper(c) != 0; }) |
        ranges::views::chunk(3) | ranges::views::transform([](auto &&rng) {
          return rng | ranges::to<std::string>;
        }) |
        ranges::to_vector;

    g.add_node(letters[0], letters[1], letters[2]);

    if (letters[0].ends_with('A')) {
      starting_nodes.push_back(letters[0]);
    }
  }

  auto calc_hops = [&](day8::node iter) {
    size_t hops = 0;
    auto next_of = [&hops, &branches, &g](day8::node const &n) {
      return branches[hops % branches.size()] == 'L' ? left_of(g, n)
                                                     : right_of(g, n);
    };
    fmt::print("from {} ", iter);
    while (!iter.ends_with('Z')) {
      iter = next_of(iter);
      ++hops;
    }
    fmt::println("to {} : {} hops!", iter, hops);

    return hops;
  };

  auto hops =
      starting_nodes | ranges::views::transform(calc_hops) | ranges::to_vector;

  auto least_common_multiple =
      ranges::fold_left(hops, 1, [](auto a, auto b) { return std::lcm(a, b); });

  fmt::println("least_common_multiple {}", least_common_multiple);

  return 0;
}
