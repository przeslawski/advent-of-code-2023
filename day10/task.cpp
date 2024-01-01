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
#include <queue>
#include <range/v3/all.hpp>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hash_util {

inline void hash_combine(std::size_t &) {}

template <typename T>
inline void hash_combine(std::size_t &seed, const T &val) {
  seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T, typename... Types>
inline void hash_combine(std::size_t &seed, const T &val,
                         const Types &...args) {
  hash_combine(seed, val);
  hash_combine(seed, args...);
}

template <typename... Types> inline std::size_t hash_val(const Types &...args) {
  std::size_t seed = 0;
  hash_combine(seed, args...);
  return seed;
}

} // namespace hash_util

enum class dir { up, down, left, right };

template <> struct std::hash<std::pair<dir, char>> {
  std::size_t operator()(std::pair<dir, char> const &p) const {
    return hash_util::hash_val(p.first, p.second);
  }
};

struct pos2 {
  int x;
  int y;

  pos2() : x(0), y(0) {}
  pos2(int _x, int _y) : x(_x), y(_y) {}

  pos2(pos2 const &other) {
    x = other.x;
    y = other.y;
  }
  pos2(pos2 &&other) {
    using std::swap;
    swap(x, other.x);
    swap(y, other.y);
  }

  pos2 &operator=(pos2 const &other) {
    x = other.x;
    y = other.y;
    return *this;
  }

  pos2 &operator=(pos2 &&other) {
    using std::swap;
    swap(x, other.x);
    swap(y, other.y);
    return *this;
  }

  pos2 &operator+=(pos2 const &other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  bool operator==(pos2 const &other) const {
    return x == other.x and y == other.y;
  }
};

pos2 operator+(pos2 lhs, pos2 const &rhs) {
  lhs += rhs;
  return lhs;
}

pos2 UP{-1, 0};
pos2 DOWN{1, 0};
pos2 LEFT{0, -1};
pos2 RIGHT{0, 1};

pos2 operator+(pos2 lhs, dir const &d) {
  static std::array<pos2, 4> dir_pos{UP, DOWN, LEFT, RIGHT};
  static_assert(dir_pos.size() == static_cast<int>(dir::right) + 1);
  lhs += dir_pos[static_cast<int>(d)];
  return lhs;
}

bool connects_with_dir(char pipe, dir d) {

  static std::array<std::unordered_set<char>, 4> segments_connectd_to_dir = {
      {{'|', 'J', 'L'}, {'|', 'F', '7'}, {'7', 'J', '-'}, {'L', 'F', '-'}}};

  auto const &connecting_pipes_set =
      segments_connectd_to_dir[static_cast<int>(d)];

  return connecting_pipes_set.find(pipe) != connecting_pipes_set.end();
}

dir opposite_to(dir d) {
  int i = static_cast<int>(d);
  i = i % 2 ? i - 1 : i + 1;
  assert(i >= 0);
  return static_cast<dir>(i);
}

template <> struct fmt::formatter<dir> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(dir const &d, FormatContext &ctx) const {
    static std::array<std::string_view, 4> const dir_names = {"up", "down",
                                                              "left", "right"};
    return fmt::format_to(ctx.out(), "{}", dir_names[static_cast<int>(d)]);
  }
};

template <> struct fmt::formatter<pos2> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(pos2 const &pos, FormatContext &ctx) const {

    return fmt::format_to(ctx.out(), "({},{})", pos.x, pos.y);
  }
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

  using std::string;
  using std::vector;

  vector<string> pipes;
  pos2 start_pos;

  string line;
  while (std::getline(ifs, line)) {
    if (line.empty()) {
      continue;
    }
    if (auto it = line.find('S'); it != string::npos) {
      start_pos = {static_cast<int>(pipes.size()), static_cast<int>(it)};
    }
    pipes.push_back(line);
  }

  int width = pipes.front().size();
  int height = pipes.size();

  auto move = [=](pos2 from, dir d) -> pos2 {
    auto to = from + d;
    return {std::clamp(to.x, 0, height), std::clamp(to.y, 0, height)};
  };

  auto pipe_at = [&pipes](pos2 const &coord) -> char {
    assert(!pipes.empty());
    assert(coord.x >= 0 and coord.x < pipes.size());
    assert(coord.y >= 0 and coord.y < pipes.front().size());
    return pipes[coord.x][coord.y];
  };

  vector<dir> dirs = {dir::up, dir::down, dir::left, dir::right};
  auto potential_dirs =
      dirs | ranges::views::filter([=, &pipes](dir d) {
        pos2 adj_pipe = move(start_pos, d);

        return connects_with_dir(pipe_at(adj_pipe), opposite_to(d));
      }) |
      ranges::to_vector;

  auto pipe_from_dirs = [](dir from, dir to) {
    if (from == dir::up) {
      switch (to) {
      case dir::left:
        return 'J';
      case dir::down:
        return '|';
      case dir::right:
        return 'L';
      default:
        assert(false);
      }
    } else if (from == dir::down) {
      switch (to) {
      case dir::left:
        return '7';
      case dir::up:
        return '|';
      case dir::right:
        return 'F';
      default:
        assert(false);
      }

    } else if (from == dir::left) {
      switch (to) {
      case dir::up:
        return 'J';
      case dir::down:
        return '7';
      case dir::right:
        return '-';
      default:
        assert(false);
      }

    } else {
      switch (to) {
      case dir::left:
        return '-';
      case dir::down:
        return 'F';
      case dir::up:
        return 'L';
      default:
        assert(false);
      }
    }
  };
  pipes[start_pos.x][start_pos.y] =
      pipe_from_dirs(potential_dirs.front(), potential_dirs.back());

  // fmt::println("no need to drop dead ends from potential starting points");
  assert(potential_dirs.size() == 2);

  std::unordered_map<std::pair<dir, char>, dir> const directed_pipes = {
      {{dir::up, '|'}, dir::up},     {{dir::down, '|'}, dir::down},
      {{dir::left, '-'}, dir::left}, {{dir::right, '-'}, dir::right},
      {{dir::down, 'J'}, dir::left}, {{dir::right, 'J'}, dir::up},
      {{dir::left, 'F'}, dir::down}, {{dir::up, 'F'}, dir::right},
      {{dir::left, 'L'}, dir::up},   {{dir::down, 'L'}, dir::right},
      {{dir::up, '7'}, dir::left},   {{dir::right, '7'}, dir::down},
  };

  auto next = [&directed_pipes](dir entering_from, char pipe) -> dir {
    assert(directed_pipes.contains({entering_from, pipe}));
    return directed_pipes.at({entering_from, pipe});
  };

  auto is_turn = [](char pipe) -> bool {
    if (pipe == '|' or pipe == '-' or pipe == '.')
      return false;
    else
      return true;
  };

  vector<pos2> loop_turns;

  if (is_turn(pipe_at(start_pos))) {
    loop_turns.push_back(start_pos);
  }

  pos2 prev = start_pos;
  dir step_dir = potential_dirs.front();
  pos2 curr = prev + step_dir;
  int steps = 1;

  while (curr != start_pos) {
    if (is_turn(pipe_at(curr))) {
      loop_turns.push_back(curr);
    }

    prev = curr;
    dir move_dir = next(step_dir, pipe_at(curr));
    curr = curr + move_dir;
    ++steps;
    step_dir = move_dir;
  }

  auto calc_area = [](vector<pos2> const &vertices) {
    int area =
        ranges::fold_left(
            ranges::views::zip(
                vertices, ranges::views::concat(
                              vertices | ranges::views::drop(1),
                              ranges::views::single(ranges::front(vertices)))),
            0,
            [](int a, std::tuple<pos2, pos2> t) -> int {
              return a + std::get<0>(t).x * std::get<1>(t).y -
                     std::get<0>(t).y * std::get<1>(t).x;
            }) /
        2;
    return std::abs(area);
  };

  int inside_points = calc_area(loop_turns) - steps / 2 + 1;
  fmt::println("furthest point from start: {}\npoints inside loop: {}",
               steps / 2, inside_points);

  return 0;
}
