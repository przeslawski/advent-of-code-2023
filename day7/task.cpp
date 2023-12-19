#include "args.hpp"
#include "range_split_strs.hpp"
#include <algorithm>
#include <cstddef>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <range/v3/all.hpp>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace day7 {

enum hand_type {
  high_card = 0,
  one_pair,
  two_pair,
  three_of_a_kind,
  full_house,
  four_of_a_kind,
  five_of_kind,
};

using card_rank_map = std::unordered_map<char, int>;
static const card_rank_map card_ranks = {
    {'2', 2}, {'3', 3},  {'4', 4},  {'5', 5},  {'6', 6},  {'7', 7}, {'8', 8},
    {'9', 9}, {'T', 10}, {'J', 11}, {'Q', 12}, {'K', 13}, {'A', 20}};

static const card_rank_map card_ranks_2 = {
    {'2', 2}, {'3', 3},  {'4', 4}, {'5', 5},  {'6', 6},  {'7', 7}, {'8', 8},
    {'9', 9}, {'T', 10}, {'J', 1}, {'Q', 12}, {'K', 13}, {'A', 20}};

struct card_comp {

  card_comp(card_rank_map const &rank_map) : ranks(rank_map) {}
  bool operator()(char l, char r) const { return ranks.at(l) < ranks.at(r); }

  card_rank_map const &ranks;
};

static const std::unordered_map<hand_type, std::string_view> type_names = {
    {one_pair, "Pair"},
    {two_pair, "Two Pairs"},
    {high_card, "High Card"},
    {five_of_kind, "Five of a Kind"},
    {four_of_a_kind, "Four of a Kind"},
    {full_house, "Full"},
    {three_of_a_kind, "Threee"}};

struct hand {

  bool operator==(hand const &other) const {
    return type == other.type and cards == other.cards;
  }

  bool operator<(hand const &other) const {
    if (type == other.type) {
      return ranges::lexicographical_compare(cards, other.cards,
                                             card_comp(card_ranks_2));
    } else {
      return type < other.type;
    }
  };

  std::string_view show() const { return cards; }

private:
  friend hand make_hand(std::string const &);
  friend hand make_hand_with_jokers(std::string const &);

  hand(std::string c, hand_type t) : cards(std::move(c)), type(t) {}

  std::string cards;
  hand_type type;
};

hand make_hand(std::string const &cards) {

  assert(cards.size() == 5);

  hand_type type;

  std::map<char, int> cards_cnt;
  ranges::for_each(cards, [&cards_cnt](char c) { ++cards_cnt[c]; });

  std::vector<int> cnt;
  ranges::copy(cards_cnt | ranges::views::values, std::back_inserter(cnt));
  ranges::sort(cnt, std::greater{});

  switch (cnt.size()) {
  case 5:
    type = high_card;
    break;
  case 4:
    type = one_pair;
    break;
  case 3:
    type = (cnt.front() == 3) ? three_of_a_kind : two_pair;
    break;
  case 2:
    type = (cnt.front() == 4) ? four_of_a_kind : full_house;
    break;
  case 1:
    type = five_of_kind;
    break;
  }

  fmt::println("hand {} of type {}, counts {}", cards, type_names.at(type),
               cnt);

  return hand(cards, type);
}
hand make_hand_with_jokers(std::string const &cards) {

  assert(cards.size() == 5);

  hand_type type;

  std::map<char, int> cards_cnt;
  ranges::for_each(cards, [&cards_cnt](char c) { ++cards_cnt[c]; });

  int jokers = 0;
  if (auto it = cards_cnt.find('J'); it != cards_cnt.end()) {
    jokers = it->second;
    cards_cnt.erase(it);
  }

  std::vector<int> cnt;
  ranges::copy(cards_cnt | ranges::views::values, std::back_inserter(cnt));
  ranges::sort(cnt, std::greater{});

  if (cnt.size()) {
    cnt.front() += jokers;
  } else {
    cnt.push_back(jokers);
  }

  switch (cnt.size()) {
  case 5:
    type = high_card;
    break;
  case 4:
    type = one_pair;
    break;
  case 3:
    type = (cnt.front() == 3) ? three_of_a_kind : two_pair;
    break;
  case 2:
    type = (cnt.front() == 4) ? four_of_a_kind : full_house;
    break;
  case 1:
    type = five_of_kind;
    break;
  }

  fmt::println("hand {} of type {}, counts {}", cards, type_names.at(type),
               cnt);

  return hand(cards, type);
}

} // namespace day7

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

  std::map<day7::hand, int> ranking;
  std::string line;
  while (std::getline(ifs, line)) {
    auto input = line | split_strs(' ') | ranges::to_vector;

    ranking[day7::make_hand_with_jokers(input[0])] = std::stoi(input[1]);
  }

  using namespace ranges;

  for_each(ranking, [](auto const &rank) {
    fmt::println("hand {}", rank.first.show());
  });

  // part 1
  size_t score =
      accumulate(views::zip(views::iota(1), ranking | views::values) |
                     views::transform([](auto const &rank_item) {
                       auto [rank, score] = rank_item;
                       fmt::println("rank {} score {}", rank, score);
                       return rank * score;
                     }),
                 0);
  fmt::println("score: {}", score);

  return 0;
}
