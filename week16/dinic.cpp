#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

template <typename Cap>
class Dinic {
  struct Edge {
    int to;
    Cap cap;
    int rev;
  };

  int n_;
  std::vector<std::vector<Edge>> graph_;
  std::vector<int> level_;
  std::vector<int> iter_;

  bool bfs(int s, int t) {
    level_.assign(n_, -1);
    std::queue<int> q;
    level_[s] = 0;
    q.push(s);
    while (!q.empty()) {
      int v = q.front(); q.pop();
      for (const auto& e : graph_[v]) {
        if (e.cap > Cap{} && level_[e.to] < 0) {
          level_[e.to] = level_[v] + 1;
          q.push(e.to);
        }
      }
    }
    return level_[t] >= 0;
  }

  Cap dfs(int v, int t, Cap f) {
    if (v == t) return f;
    for (int& i = iter_[v]; i < static_cast<int>(graph_[v].size()); ++i) {
      Edge& e = graph_[v][i];
      if (e.cap > Cap{} && level_[v] < level_[e.to]) {
        Cap d = dfs(e.to, t, std::min(f, e.cap));
        if (d > Cap{}) {
          e.cap -= d;
          graph_[e.to][e.rev].cap += d;
          return d;
        }
      }
    }
    return Cap{};
  }

public:
  explicit Dinic(int n) : n_(n), graph_(n), level_(n), iter_(n) {}

  void add_edge(int from, int to, Cap cap) {
    graph_[from].push_back({to, cap, static_cast<int>(graph_[to].size())});
    graph_[to].push_back({from, Cap{}, static_cast<int>(graph_[from].size()) - 1});
  }

  Cap max_flow(int s, int t) {
    Cap flow{};
    while (bfs(s, t)) {
      iter_.assign(n_, 0);
      Cap f;
      while ((f = dfs(s, t, std::numeric_limits<Cap>::max())) > Cap{}) {
        flow += f;
      }
    }
    return flow;
  }
};

#include <iostream>

int main() {
  int n, m, s, t;
  std::cin >> n >> m >> s >> t;

  Dinic<long long> dinic(n + 1);

  for (int i = 0; i < m; ++i) {
    int u, v;
    long long c;
    std::cin >> u >> v >> c;
    dinic.add_edge(u, v, c);
  }

  std::cout << dinic.max_flow(s, t) << '\n';

  return 0;
}
