#include "args.hpp"
#include <algorithm>
#include <cstddef>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <iterator>
#include <queue>
#include <range/v3/all.hpp>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace aoc {
using i64 = int64_t;

struct range {
  i64 beg;
  i64 end;
};

bool in_range(size_t num, range rng) {
  return num >= rng.beg and num <= rng.end;
}

struct shifting_range {
  range rng;
  i64 by;
};

std::pair<std::vector<range>, std::vector<range>>
transform(range in, shifting_range shift) {
  std::vector<range> touched;
  std::vector<range> untouched;

  // either input range is completely before or after shifting range on the
  // numbers axis - no shift
  if (in.beg > shift.rng.end or in.end < shift.rng.beg) {
    untouched.push_back(in);
    return {untouched, touched};
  }

  // find common range
  touched.push_back({std::max(in.beg, shift.rng.beg) + shift.by,
                     std::min(in.end, shift.rng.end) + shift.by});

  if (in.beg < shift.rng.beg) {
    untouched.push_back({in.beg, shift.rng.beg - 1});
  }

  if (in.end > shift.rng.end) {
    untouched.push_back({shift.rng.end + 1, in.end});
  }

  return {untouched, touched};
}
} // namespace aoc

struct mapping {

  void add_rule(aoc::i64 to, aoc::i64 from, aoc::i64 len) {
    assert(len != 0);
    rules.push_back(
        aoc::shifting_range{aoc::range{from, from + len - 1}, to - from});
  }

  // [seed1, seed2, seed3 ...] -> [..]
  std::vector<aoc::range> operator()(aoc::range input_range) const {

    std::vector<aoc::range> in;
    in.push_back(input_range);

    std::vector<aoc::range> out;
    for (auto const &rule : rules) {
      std::vector<aoc::range> proc;
      for (auto const &rng : in) {
        auto [left, transformed] = aoc::transform(rng, rule);

        std::move(transformed.begin(), transformed.end(),
                  std::back_inserter(out));
        std::move(left.begin(), left.end(), std::back_inserter(proc));
      }
      std::swap(in, proc);
    }

    std::move(in.begin(), in.end(), std::back_inserter(out));
    return out;
  }

private:
  std::vector<aoc::shifting_range> rules;
};

auto split_strs = [](auto &&pattern) {
  using namespace ranges;
  return views::split(pattern) | views::transform([](auto p) {
           auto c = p | views::common;
           return std::string(c.begin(), c.end());
         });
};

struct almanac_parser {

  void operator()(std::string_view line) {

    if (!lines) {
      parse_seeds(line);
    } else if (line.empty() && parsing_context) {
      mappings.push_back(*parsing_context);
      parsing_context.reset();
    } else {
      auto label = std::string(":pam");
      if (ranges::all_of(
              ranges::views::zip(line | ranges::views::reverse, label),
              [](auto chars) {
                return std::get<0>(chars) == std::get<1>(chars);
              })) {
        parsing_context = mapping{};

      } else {
        // numbers
        auto rule_bounds = line | split_strs(' ') |
                           ranges::views::transform([](std::string const &str) {
                             return std::stol(str);
                           }) |
                           ranges::to<std::vector<size_t>>;
        assert(parsing_context);
        assert(rule_bounds.size() == 3);
        parsing_context->add_rule(rule_bounds[0], rule_bounds[1],
                                  rule_bounds[2]);
      }
    }

    ++lines;
  }

  void parse_seeds(std::string_view line) {

    using namespace std::literals::string_view_literals;

    seeds = line | ranges::views::drop(("seeds: "sv).size()) | split_strs(' ') |
            ranges::views::transform(
                [](std::string const &str) { return std::stol(str); }) |
            ranges::views::chunk(2) |
            ranges::views::transform([](auto seed_rng) {
              std::vector<long> seed = seed_rng | ranges::to_vector;
              return aoc::range{seed[0], seed[0] + seed[1] - 1};
            }) |
            ranges::to<std::vector<aoc::range>>;
  }

  std::vector<mapping> get_mappings() const { return mappings; }
  std::vector<aoc::range> get_seeds() const { return seeds; }

  int lines{0};
  std::vector<aoc::range> seeds;
  std::vector<mapping> mappings;
  std::optional<mapping> parsing_context;
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

  almanac_parser parser;

  std::string line;
  while (std::getline(ifs, line)) {
    parser(line);
  }

  auto seeds = parser.get_seeds();
  auto mappings = parser.get_mappings();
  fmt::println("seeds: ");
  for (auto const &seed : seeds) {
    fmt::println("<{}, {}>", seed.beg, seed.end);
  }
  fmt::println("mappings amount: {}", mappings.size());

  int map_stage = 1;

  for (auto const &map : mappings) {
    std::vector<aoc::range> processed;

    fmt::println(" maping stage ({}) seeds to map {}", map_stage, seeds.size());
    for (auto const &seed : seeds) {
      fmt::println("processing ({}, {})", seed.beg, seed.end);
      auto post_map = map(seed);
      std::move(post_map.begin(), post_map.end(),
                std::back_inserter(processed));
    }
    std::swap(seeds, processed);
    ++map_stage;
  }

  size_t min_location = ranges::min(
      seeds | ranges::views::transform([](aoc::range rng) { return rng.beg; }));

  fmt::println("part2 closest location: {}", min_location);

  return 0;
}
