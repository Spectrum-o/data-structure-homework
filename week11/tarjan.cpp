#include <algorithm>
#include <stack>
#include <vector>

class Tarjan {
  int n_;
  std::vector<std::vector<int>> adj_;
  std::vector<int> dfn_;
  std::vector<int> low_;
  std::vector<bool> on_stack_;
  std::stack<int> stk_;
  std::vector<int> scc_id_;
  std::vector<std::vector<int>> sccs_;
  int timestamp_ = 0;

  void dfs(int u) {
    dfn_[u] = low_[u] = ++timestamp_;
    stk_.push(u);
    on_stack_[u] = true;

    for (int v : adj_[u]) {
      if (dfn_[v] == 0) {
        dfs(v);
        low_[u] = std::min(low_[u], low_[v]);
      } else if (on_stack_[v]) {
        low_[u] = std::min(low_[u], dfn_[v]);
      }
    }

    if (low_[u] == dfn_[u]) {
      std::vector<int> scc;
      int v;
      do {
        v = stk_.top(); stk_.pop();
        on_stack_[v] = false;
        scc_id_[v] = static_cast<int>(sccs_.size());
        scc.push_back(v);
      } while (v != u);
      sccs_.push_back(std::move(scc));
    }
  }

public:
  explicit Tarjan(int n) : n_(n), adj_(n), dfn_(n, 0), low_(n, 0), on_stack_(n, false), scc_id_(n, -1) {}

  void add_edge(int from, int to) {
    adj_[from].push_back(to);
  }

  int solve() {
    for (int i = 0; i < n_; ++i) {
      if (dfn_[i] == 0) dfs(i);
    }
    return static_cast<int>(sccs_.size());
  }

  [[nodiscard]] const std::vector<std::vector<int>>& sccs() const noexcept { return sccs_; }
  [[nodiscard]] const std::vector<int>& scc_id() const noexcept { return scc_id_; }
};

#include <iostream>

int main() {
  int n, m;
  std::cin >> n >> m;

  Tarjan tarjan(n);

  for (int i = 0; i < m; ++i) {
    int u, v;
    std::cin >> u >> v;
    tarjan.add_edge(u - 1, v - 1);
  }

  int scc_count = tarjan.solve();
  std::cout << scc_count << '\n';

  const auto& sccs = tarjan.sccs();
  for (int i = 0; i < scc_count; ++i) {
    for (int v : sccs[i]) {
      std::cout << v + 1 << ' ';
    }
    std::cout << '\n';
  }

  return 0;
}
