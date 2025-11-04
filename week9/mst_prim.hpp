#pragma once
#include "mst_common.hpp"

namespace mst {

struct Prim {
  int start{0};
  explicit Prim(int s = 0) : start(s) {}

  template <Weight W>
  [[nodiscard]] std::optional<Result<W>>
  solve(int n, std::span<const Edge<W>> edges) const {
    if (n == 0) return Result<W>{};
    if (start < 0 || start >= n) return std::nullopt;

    std::vector<std::vector<std::pair<int, W>>> adj(n);
    adj.reserve(n);
    for (auto const& [u, v, w] : edges) {
      adj[u].emplace_back(v, w);
      adj[v].emplace_back(u, w);
    }

    Result<W> res{};
    res.tree.reserve(n - 1);

    std::vector<bool> in(n, false);
    using Item = std::tuple<W, int, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<>> pq;

    auto push_edges = [&](int u) {
      in[u] = true;
      for (auto const& [v, w] : adj[u]) if (!in[v]) pq.emplace(w, v, u);
    };

    push_edges(start);
    int cnt = 1;

    while (!pq.empty() && cnt < n) {
      auto [w, v, p] = pq.top(); pq.pop();
      if (in[v]) continue;
      res.total = res.total + w;
      res.tree.push_back(Edge<W>{p, v, w});
      ++cnt;
      push_edges(v);
    }

    if (cnt != n) return std::nullopt;
    return res;
  }
};

} // namespace mst
