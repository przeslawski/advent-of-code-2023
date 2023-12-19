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

namespace day8 {

using node = std::string;

namespace graph {

template <typename Iter> Iter right(Iter iter) {
  iter.right();
  return iter;
}

template <typename Iter> Iter left(Iter iter) {
  iter.left();
  return iter;
}

/* struct graph_iterator { */
/**/
/*   graph_iterator() */
/**/
/*   void right() { iter = iter->second.second; } */
/*   void left() { iter = iter->second.first; } */
/**/
/*   std::unordered_map<node, std::pair<node, node>>::const_iterator iter; */
/* }; */

struct graph {

  /* using iterator = graph_iterator; */

  bool add_node(node vertex, node left, node right) {
    return adj_list.insert({vertex, {left, right}}).second;
  };

  /* graph_iterator begin() { return graph_iterator{adj_list.find("AAA")}; } */

  /* private: */
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
      fmt::println(" start from: {}", letters[0]);
    }

    fmt::println("{}", letters);
  }

  size_t hops = 0;

  auto next_of = [&hops, &branches, &g](day8::node const &n) {
    return branches[hops % branches.size()] == 'L' ? left_of(g, n)
                                                   : right_of(g, n);
  };

  auto is_done = [](auto const &nodes) {
    return ranges::all_of(nodes,
                          [](auto const &node) { return node.ends_with('Z'); });
  };

  // part1
  /*
  day8::node iter = "AAA";
  while (iter != "ZZZ") {
    iter = next_of(iter);
    ++hops;
  }
  fmt::println("part1 {} hops!", hops);
  */

  hops = 0;
  // part2
  while (!is_done(starting_nodes)) {

    ranges::for_each(starting_nodes, [&](auto &node) { node = next_of(node); });
    /* iter = next_of(iter); */
    ++hops;
  }
  fmt::println("part2 {} hops!", hops);

  return 0;
}
