#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct trie {

  trie() = default;

  trie(std::initializer_list<std::pair<std::string, int>> init) {
    std::for_each(init.begin(), init.end(), [this](auto const &entry) {
      insert(entry.first, entry.second);
    });
  }

  bool insert(std::string_view word, int val) {
    node *ptr = &root_;
    for (int i = 0; i < word.size(); ++i) {
      ptr = &ptr->next[word[i]];
    }
    if (!ptr->value) {
      ptr->value = val;
      return true;
    }
    return false;
  }

  std::vector<std::string> list_all() const {

    std::vector<std::string> all;

    std::function<void(node const &, std::string)> gather =
        [&all, &gather](node const &root, std::string prefix) {
          if (root.value) {
            all.push_back(prefix);
          }

          for (auto const &entry : root.next) {
            gather(entry.second, prefix + entry.first);
          }
        };

    gather(root_, "");

    return all;
  }

  bool has_prefix(std::string_view prefix) const {

    auto iter = prefix.begin();
    node const *ptr = &root_;

    while (iter != prefix.end()) {

      if (auto next = ptr->next.find(*iter); next != ptr->next.end()) {
        ptr = &(next->second);
        ++iter;
      } else {
        return false;
      }
    }

    return true;
  }

  std::optional<int> search(std::string_view word) const {

    auto iter = word.begin();
    node const *ptr = &root_;

    while (iter != word.end()) {

      if (auto next = ptr->next.find(*iter); next != ptr->next.end()) {
        ptr = &(next->second);
        ++iter;
      } else {
        return std::nullopt;
      }
    }

    return ptr->value;
  }

private:
  struct node {
    std::unordered_map<char, node> next;
    std::optional<int> value;
  };

  node root_;
};
