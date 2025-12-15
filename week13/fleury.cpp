#include <algorithm>
#include <list>
#include <vector>

class Fleury {
  int n_;
  std::vector<std::list<std::pair<int, int>>> adj_;
  std::vector<bool> used_;
  int edge_count_ = 0;

  void remove_edge(int u, int v, int eid) {
    used_[eid] = true;
    for (auto it = adj_[u].begin(); it != adj_[u].end(); ++it) {
      if (it->first == v && it->second == eid) {
        adj_[u].erase(it);
        break;
      }
    }
    for (auto it = adj_[v].begin(); it != adj_[v].end(); ++it) {
      if (it->first == u && it->second == eid) {
        adj_[v].erase(it);
        break;
      }
    }
  }

  int dfs_count(int u, std::vector<bool>& visited) {
    visited[u] = true;
    int count = 1;
    for (const auto& [v, eid] : adj_[u]) {
      if (!visited[v]) {
        count += dfs_count(v, visited);
      }
    }
    return count;
  }

  int count_reachable(int u) {
    std::vector<bool> visited(n_, false);
    return dfs_count(u, visited);
  }

  bool is_bridge(int u, int v, int eid) {
    if (adj_[u].size() == 1) return false;
    int before = count_reachable(u);
    remove_edge(u, v, eid);
    int after = count_reachable(u);
    adj_[u].emplace_back(v, eid);
    adj_[v].emplace_back(u, eid);
    used_[eid] = false;
    return after < before;
  }

public:
  explicit Fleury(int n) : n_(n), adj_(n) {}

  void add_edge(int u, int v) {
    int eid = edge_count_++;
    adj_[u].emplace_back(v, eid);
    adj_[v].emplace_back(u, eid);
    used_.push_back(false);
  }

  std::pair<bool, std::vector<int>> find_euler_path() {
    std::vector<int> odd_vertices;
    for (int i = 0; i < n_; ++i) {
      if (adj_[i].size() % 2 == 1) {
        odd_vertices.push_back(i);
      }
    }

    if (odd_vertices.size() != 0 && odd_vertices.size() != 2) {
      return {false, {}};
    }

    int start = 0;
    if (odd_vertices.size() == 2) {
      start = odd_vertices[0];
    } else {
      for (int i = 0; i < n_; ++i) {
        if (!adj_[i].empty()) {
          start = i;
          break;
        }
      }
    }

    std::vector<int> path;
    path.push_back(start);
    int u = start;

    while (!adj_[u].empty()) {
      int next = -1;
      int next_eid = -1;

      for (const auto& [v, eid] : adj_[u]) {
        if (!used_[eid]) {
          if (adj_[u].size() == 1 || !is_bridge(u, v, eid)) {
            next = v;
            next_eid = eid;
            break;
          }
          if (next == -1) {
            next = v;
            next_eid = eid;
          }
        }
      }

      if (next == -1) break;
      remove_edge(u, next, next_eid);
      path.push_back(next);
      u = next;
    }

    bool valid = (static_cast<int>(path.size()) == edge_count_ + 1);
    return {valid, path};
  }
};

class Hierholzer {
  int n_;
  std::vector<std::list<std::pair<int, int>>> adj_;
  std::vector<bool> used_;
  int edge_count_ = 0;

  void dfs(int u, std::vector<int>& path) {
    while (!adj_[u].empty()) {
      auto [v, eid] = adj_[u].front();
      adj_[u].pop_front();
      if (used_[eid]) continue;
      used_[eid] = true;
      for (auto it = adj_[v].begin(); it != adj_[v].end(); ++it) {
        if (it->second == eid) {
          adj_[v].erase(it);
          break;
        }
      }
      dfs(v, path);
    }
    path.push_back(u);
  }

public:
  explicit Hierholzer(int n) : n_(n), adj_(n) {}

  void add_edge(int u, int v) {
    int eid = edge_count_++;
    adj_[u].emplace_back(v, eid);
    adj_[v].emplace_back(u, eid);
    used_.push_back(false);
  }

  std::pair<bool, std::vector<int>> find_euler_path() {
    std::vector<int> odd_vertices;
    for (int i = 0; i < n_; ++i) {
      if (adj_[i].size() % 2 == 1) {
        odd_vertices.push_back(i);
      }
    }

    if (odd_vertices.size() != 0 && odd_vertices.size() != 2) {
      return {false, {}};
    }

    int start = 0;
    if (odd_vertices.size() == 2) {
      start = odd_vertices[0];
    } else {
      for (int i = 0; i < n_; ++i) {
        if (!adj_[i].empty()) {
          start = i;
          break;
        }
      }
    }

    std::vector<int> path;
    dfs(start, path);
    std::reverse(path.begin(), path.end());

    bool valid = (static_cast<int>(path.size()) == edge_count_ + 1);
    return {valid, path};
  }
};

#include <iostream>

int main() {
  int type;
  std::cin >> type;

  int n, m;
  std::cin >> n >> m;

  if (type == 1) {
    Fleury fleury(n);
    for (int i = 0; i < m; ++i) {
      int u, v;
      std::cin >> u >> v;
      fleury.add_edge(u - 1, v - 1);
    }
    auto [valid, path] = fleury.find_euler_path();
    if (valid) {
      for (int v : path) std::cout << v + 1 << ' ';
      std::cout << '\n';
    } else {
      std::cout << "No Euler path\n";
    }
  } else {
    Hierholzer hierholzer(n);
    for (int i = 0; i < m; ++i) {
      int u, v;
      std::cin >> u >> v;
      hierholzer.add_edge(u - 1, v - 1);
    }
    auto [valid, path] = hierholzer.find_euler_path();
    if (valid) {
      for (int v : path) std::cout << v + 1 << ' ';
      std::cout << '\n';
    } else {
      std::cout << "No Euler path\n";
    }
  }

  return 0;
}
