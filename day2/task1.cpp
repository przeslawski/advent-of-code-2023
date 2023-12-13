#include "args.hpp"
#include <algorithm>
#include <cassert>
#include <charconv>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <optional>
#include <range/v3/all.hpp>
#include <string_view>

struct roll {
  int red;
  int green;
  int blue;

  roll() : red(0), green(0), blue(0) {}
};

struct game {
  std::vector<roll> rolls;
};

auto parse_roll(std::string_view::const_iterator iter) {

  std::optional<roll> rl;

  return std::pair{rl, iter};
}

auto split(std::string_view str, char sep) {
  std::vector<std::string_view> substrs;

  auto beg = str.begin();

  do {

    auto iter = std::find(beg, str.end(), sep);

    auto len = std::distance(beg, iter);

    if (len) {
      substrs.push_back({beg, iter});
    }

    if (iter != str.end()) {
      ++iter;
    }

    beg = iter;

  } while (beg != str.end());

  return substrs;
}

// removes leading 'space'
std::string_view trim_front(std::string_view str) {
  auto beg = str.begin();
  while (beg != str.end() && *beg == ' ') {
    ++beg;
  }
  return {beg, str.end()};
}

enum class color { red, green, blue };

template <typename To> std::optional<To> to(std::string_view);

template <> std::optional<color> to(std::string_view str) {
  if (str == "red") {
    return color::red;
  } else if (str == "blue") {
    return color::blue;
  } else if (str == "green") {
    return color::green;
  }

  return std::nullopt;
}

template <> std::optional<int> to(std::string_view str) {

  int out;
  std::from_chars_result const result =
      std::from_chars(str.data(), str.end(), out);

  if (result.ec == std::errc::invalid_argument ||
      result.ec == std::errc::result_out_of_range) {
    return std::nullopt;
  }

  return out;
}

template <> std::optional<roll> to(std::string_view str) {

  // split each color cubes count
  auto cubes = split(str, ',');

  roll rl;

  // for each xxx red / yyy blue / zzz green
  for (auto cube : cubes) {

    auto r = split(cube, ' ');

    auto cnt = to<int>(r[0]);
    auto col = to<color>(r[1]);

    if (cnt && col) {
      switch (*col) {
      case color::blue:
        rl.blue += *cnt;
        break;
      case color::red:
        rl.red += *cnt;
        break;
      case color::green:
        rl.green += *cnt;
        break;
      default:
        break;
      }
    } else {
      return std::nullopt;
    }
  }

  return rl;
}

auto parse_line(std::string_view str) {

  // remove "Game xxx: " prefix
  str = split(str, ':')[1];

  // split game into rolls
  auto rolls_str = split(str, ';');

  std::vector<roll> rolls;
  ranges::for_each(rolls_str, [&rolls](std::string_view roll_str) {
    auto trimmed = trim_front(roll_str);

    auto opt_roll = to<roll>(trimmed);
    if (opt_roll) {
      rolls.push_back(*opt_roll);
    }
  });

  return rolls;
}

int main(int argc, char **argv) {

  auto args = parse_args(argc, argv);

  if (args.size() < 5) {
    fmt::println(
        "usage: {} <input_file> <red_cubes> <green_cubes> <blue_cubes>",
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

  int red_cubes = *to<int>(args[2]);
  int green_cubes = *to<int>(args[3]);
  int blue_cubes = *to<int>(args[4]);

  std::size_t acc{};
  std::size_t acc_power{};

  std::string line;

  auto is_possible = [](roll const &r, int red, int blue, int green) {
    return r.red <= red and r.green <= green and r.blue <= blue;
  };

  int game_idx = 1;
  while (std::getline(ifs, line)) {

    auto rolls = parse_line(line);

    if (ranges::all_of(rolls, [=](auto const &r) {
          return is_possible(r, red_cubes, blue_cubes, green_cubes);
        })) {
      acc += game_idx;
    }

    int min_red = ranges::max(
        rolls | ranges::views::transform([](auto const &r) { return r.red; }));
    int min_blue = ranges::max(
        rolls | ranges::views::transform([](auto const &r) { return r.blue; }));
    int min_green =
        ranges::max(rolls | ranges::views::transform(
                                [](auto const &r) { return r.green; }));

    acc_power += (min_red * min_blue * min_green);

    ++game_idx;
  }

  fmt::println("sum of possible game IDs:{}", acc);
  fmt::println("sum of power of minimum required cubes:{}", acc_power);
  return 0;
}
