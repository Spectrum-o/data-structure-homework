#include <algorithm>
#include <vector>

class Hungarian {
  int n_, m_;
  std::vector<std::vector<int>> adj_;
  std::vector<int> match_l_;
  std::vector<int> match_r_;
  std::vector<bool> used_;

  bool dfs(int v) {
    for (int to : adj_[v]) {
      if (used_[to]) continue;
      used_[to] = true;
      if (match_r_[to] < 0 || dfs(match_r_[to])) {
        match_l_[v] = to;
        match_r_[to] = v;
        return true;
      }
    }
    return false;
  }

public:
  explicit Hungarian(int n, int m) : n_(n), m_(m), adj_(n), match_l_(n, -1), match_r_(m, -1), used_(m) {}

  void add_edge(int u, int v) {
    adj_[u].push_back(v);
  }

  int max_matching() {
    int result = 0;
    for (int v = 0; v < n_; ++v) {
      std::fill(used_.begin(), used_.end(), false);
      if (dfs(v)) ++result;
    }
    return result;
  }

  [[nodiscard]] const std::vector<int>& left_match() const noexcept { return match_l_; }
  [[nodiscard]] const std::vector<int>& right_match() const noexcept { return match_r_; }
};

#include <iostream>

int main() {
  int n, m, e;
  std::cin >> n >> m >> e;

  Hungarian hung(n, m);

  for (int i = 0; i < e; ++i) {
    int u, v;
    std::cin >> u >> v;
    if (u >= 1 && u <= n && v >= 1 && v <= m) {
      hung.add_edge(u - 1, v - 1);
    }
  }

  std::cout << hung.max_matching() << '\n';

  return 0;
}
