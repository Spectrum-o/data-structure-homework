#pragma once
#include "mst_common.hpp"

namespace mst {

struct Kruskal {
  template <Weight W>
  [[nodiscard]] std::optional<Result<W>> solve(int n, std::span<const Edge<W>> edges) const {
    Result<W> res{};
    res.tree.reserve(n > 0 ? n - 1 : 0);

    std::vector<Edge<W>> es(edges.begin(), edges.end());
    std::ranges::sort(es, {}, &Edge<W>::w);

    DSU dsu(n);
    for (auto const& e : es) {
      if (static_cast<int>(res.tree.size()) == n - 1) break;
      if (dsu.unite(e.u, e.v)) {
        res.total = res.total + e.w;
        res.tree.push_back(e);
      }
    }
    if (n == 0 || static_cast<int>(res.tree.size()) != n - 1) return std::nullopt;
    return res;
  }
};

} // namespace mst
