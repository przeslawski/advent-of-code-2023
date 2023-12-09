#include "trie.hpp"
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <range/v3/all.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

int main(int argc, char *argv[]) {

  std::vector<std::string_view> args{};
  for (int i = 0; i < argc; ++i) {
    args.push_back({argv[i], std::strlen(argv[i])});
  }

  if (argc < 2) {
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
  std::size_t acc{};

  trie prefix_tree = {{"one", 1},   {"1", 1}, {"two", 2},   {"2", 2},
                      {"three", 3}, {"3", 3}, {"four", 4},  {"4", 4},
                      {"five", 5},  {"5", 5}, {"six", 6},   {"6", 6},
                      {"seven", 7}, {"7", 7}, {"eight", 8}, {"8", 8},
                      {"nine", 9},  {"9", 9}, {"zero", 0},  {"0", 0}};

  trie suffix_tree = {{"eno", 1},   {"1", 1}, {"owt", 2},   {"2", 2},
                      {"eerht", 3}, {"3", 3}, {"ruof", 4},  {"4", 4},
                      {"evif", 5},  {"5", 5}, {"xis", 6},   {"6", 6},
                      {"neves", 7}, {"7", 7}, {"thgie", 8}, {"8", 8},
                      {"enin", 9},  {"9", 9}, {"orez", 0},  {"0", 0}};

  auto get_first_full_prefix = [](trie const &prefix_tree,
                                  std::string_view line) {
    int prefix = 0;

    int start = 0;
    std::size_t size = 1;
    std::string_view substr{line.data() + start, size};

    do {

      // find first matching prefix char
      while (!prefix_tree.has_prefix(substr)) {
        ++start;
        if (start == line.size()) {
          throw std::out_of_range(
              "prefix cannot be found in the input string :<");
        }
        size = 1;
        substr = std::string_view{line.data() + start, size};
      }

      if (auto opt_val = prefix_tree.search(substr); opt_val) {
        // found entry
        return *opt_val;
      } else {
        // not found, increase window size
        substr = std::string_view{line.data() + start, ++size};
      }

    } while (true);
  };

  while (ifs >> line) {
    int first = get_first_full_prefix(prefix_tree, line);
    std::reverse(line.begin(), line.end());
    int second = get_first_full_prefix(suffix_tree, line);
    acc += first * 10 + second;
  }

  fmt::println("{}", acc);

  return 0;
}
