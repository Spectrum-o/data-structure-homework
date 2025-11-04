#pragma once
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

namespace mst {

template <class W>
concept Weight =
  std::totally_ordered<W> &&
  requires(W a, W b) {
    { a + b } -> std::same_as<W>;
    W{};
  };

template <Weight W>
struct Edge {
  int u{}, v{};
  W w{};
};

template <Weight W>
struct [[nodiscard]] Result {
  W total{};
  std::vector<Edge<W>> tree;
};

template <class Solver, class W>
concept MSTSolver =
  Weight<W> &&
  requires(Solver s, int n, std::span<const Edge<W>> es) {
    { s.template solve<W>(n, es) } -> std::same_as<std::optional<Result<W>>>;
  };

class DSU {
  std::vector<int> uf;
public:
  explicit DSU(int n) { uf.assign(n, -1); }
  int find(int x) {
    return uf[x] < 0 ? x : uf[x] = find(uf[x]);
  }

  bool unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) return false;
    if (-uf[a] < -uf[b]) {
      std::swap(a, b);
    }

    uf[a] += uf[b];
    uf[b] = a;
    return true;
  }
};

} // namespace mst
