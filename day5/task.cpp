#include "args.hpp"
#include <cstddef>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <range/v3/all.hpp>
#include <string_view>
#include <unordered_set>
#include <vector>

struct mapping {

  void add_rule(size_t to, size_t from, size_t len) {
    rules.push_back({to, from, len});
  }

  size_t operator()(size_t from) const {

    for (auto const &rule : rules) {
      // fmt::println("num: {} rule: <{}, {}> to <{},{}>", from, rule.from,
      // rule.from + rule.len, rule.to, rule.to + rule.len);
      size_t offset = from - rule.from;
      if (offset >= 0 and offset < rule.len) {
        return rule.to + offset;
      }
    }
    return from;
  }

private:
  struct map_rule {
    size_t to;
    size_t from;
    size_t len;
  };

  std::vector<map_rule> rules;
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
            ranges::to<std::vector<size_t>>;
  }

  std::vector<mapping> get_mappings() const { return mappings; }
  std::vector<size_t> get_seeds() const { return seeds; }

  int lines{0};
  std::vector<size_t> seeds;
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
    /* fmt::println("{}", line); */
  }

  auto seeds = parser.get_seeds();
  auto mappings = parser.get_mappings();
  fmt::println("seeds: {}", seeds);
  fmt::println("mappings amount: {}", mappings.size());

  auto locations = ranges::views::transform(seeds, [&mappings](size_t seed) {
    for (auto const &map : mappings) {

      auto transformed = map(seed);
      fmt::println("{} -> {}", seed, transformed);
      seed = transformed;
    }
    fmt::println("----");
    return seed;
  });
  size_t min_location = ranges::min(locations);

  fmt::println("part1 closest location: {}", min_location);

  // part 2
  {
    using namespace ranges;
    auto seed_gen =
        seeds | views::chunk(2) | views::transform([](auto seed_rng) {
          auto seed_bounds = seed_rng | to_vector;
          return views::iota(seed_bounds[0]) | views::take(seed_bounds[1]);
        }) |
        views::join;
    auto locations =
        ranges::views::transform(seed_gen, [&mappings](size_t seed) {
          for (auto const &map : mappings) {

            auto transformed = map(seed);
            /* fmt::println("{} -> {}", seed, transformed); */
            seed = transformed;
          }
          /* fmt::println("----"); */
          return seed;
        });
    size_t min_location = ranges::min(locations);
    fmt::println("part2 closest location: {}", min_location);
  }

  return 0;
}
