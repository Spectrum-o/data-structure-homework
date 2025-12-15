#include <algorithm>
#include <deque>
#include <vector>

class Knapsack {
public:
  static long long zero_one(int capacity, const std::vector<int>& weights, const std::vector<long long>& values) {
    int n = static_cast<int>(weights.size());
    std::vector<long long> dp(capacity + 1, 0);
    for (int i = 0; i < n; ++i) {
      for (int j = capacity; j >= weights[i]; --j) {
        dp[j] = std::max(dp[j], dp[j - weights[i]] + values[i]);
      }
    }
    return dp[capacity];
  }

  static long long complete(int capacity, const std::vector<int>& weights, const std::vector<long long>& values) {
    int n = static_cast<int>(weights.size());
    std::vector<long long> dp(capacity + 1, 0);
    for (int i = 0; i < n; ++i) {
      for (int j = weights[i]; j <= capacity; ++j) {
        dp[j] = std::max(dp[j], dp[j - weights[i]] + values[i]);
      }
    }
    return dp[capacity];
  }

  static long long multiple_binary(int capacity, const std::vector<int>& weights, const std::vector<long long>& values, const std::vector<int>& counts) {
    int n = static_cast<int>(weights.size());
    std::vector<long long> dp(capacity + 1, 0);
    for (int i = 0; i < n; ++i) {
      if (weights[i] == 0) continue;
      if (static_cast<long long>(counts[i]) * weights[i] >= capacity) {
        for (int j = weights[i]; j <= capacity; ++j) {
          dp[j] = std::max(dp[j], dp[j - weights[i]] + values[i]);
        }
      } else {
        int k = 1;
        int remaining = counts[i];
        while (remaining > 0) {
          int take = std::min(k, remaining);
          int w = take * weights[i];
          long long v = take * values[i];
          for (int j = capacity; j >= w; --j) {
            dp[j] = std::max(dp[j], dp[j - w] + v);
          }
          remaining -= take;
          k *= 2;
        }
      }
    }
    return dp[capacity];
  }

  static long long multiple_monotonic(int capacity, const std::vector<int>& weights, const std::vector<long long>& values, const std::vector<int>& counts) {
    int n = static_cast<int>(weights.size());
    std::vector<long long> dp(capacity + 1, 0);
    std::vector<long long> prev(capacity + 1);

    for (int i = 0; i < n; ++i) {
      int w = weights[i];
      long long v = values[i];
      int c = counts[i];

      if (w == 0) continue;
      if (static_cast<long long>(c) * w >= capacity) {
        for (int j = w; j <= capacity; ++j) {
          dp[j] = std::max(dp[j], dp[j - w] + v);
        }
        continue;
      }

      prev = dp;
      for (int r = 0; r < w; ++r) {
        std::deque<std::pair<int, long long>> dq;
        for (int j = r, k = 0; j <= capacity; j += w, ++k) {
          long long val = prev[j] - static_cast<long long>(k) * v;
          while (!dq.empty() && dq.back().second <= val) {
            dq.pop_back();
          }
          dq.emplace_back(k, val);
          while (!dq.empty() && k - dq.front().first > c) {
            dq.pop_front();
          }
          dp[j] = dq.front().second + static_cast<long long>(k) * v;
        }
      }
    }
    return dp[capacity];
  }

  static long long group(int capacity, const std::vector<std::vector<std::pair<int, long long>>>& groups) {
    std::vector<long long> dp(capacity + 1, 0);
    for (const auto& group : groups) {
      for (int j = capacity; j >= 0; --j) {
        for (const auto& [w, v] : group) {
          if (j >= w) {
            dp[j] = std::max(dp[j], dp[j - w] + v);
          }
        }
      }
    }
    return dp[capacity];
  }

  static long long mixed(int capacity, const std::vector<int>& weights, const std::vector<long long>& values, const std::vector<int>& counts) {
    int n = static_cast<int>(weights.size());
    std::vector<long long> dp(capacity + 1, 0);
    std::vector<long long> prev(capacity + 1);

    for (int i = 0; i < n; ++i) {
      int w = weights[i];
      long long v = values[i];
      int c = counts[i];

      if (w == 0) continue;

      if (c == -1) {
        for (int j = w; j <= capacity; ++j) {
          dp[j] = std::max(dp[j], dp[j - w] + v);
        }
      } else if (c == 1) {
        for (int j = capacity; j >= w; --j) {
          dp[j] = std::max(dp[j], dp[j - w] + v);
        }
      } else if (static_cast<long long>(c) * w >= capacity) {
        for (int j = w; j <= capacity; ++j) {
          dp[j] = std::max(dp[j], dp[j - w] + v);
        }
      } else {
        prev = dp;
        for (int r = 0; r < w; ++r) {
          std::deque<std::pair<int, long long>> dq;
          for (int j = r, k = 0; j <= capacity; j += w, ++k) {
            long long val = prev[j] - static_cast<long long>(k) * v;
            while (!dq.empty() && dq.back().second <= val) {
              dq.pop_back();
            }
            dq.emplace_back(k, val);
            while (!dq.empty() && k - dq.front().first > c) {
              dq.pop_front();
            }
            dp[j] = dq.front().second + static_cast<long long>(k) * v;
          }
        }
      }
    }
    return dp[capacity];
  }

  static std::vector<long long> zero_one_count(int capacity, const std::vector<int>& weights, long long mod = 1e9 + 7) {
    std::vector<long long> dp(capacity + 1, 0);
    dp[0] = 1;
    for (int w : weights) {
      for (int j = capacity; j >= w; --j) {
        dp[j] = (dp[j] + dp[j - w]) % mod;
      }
    }
    return dp;
  }

  static std::vector<long long> complete_count(int capacity, const std::vector<int>& weights, long long mod = 1e9 + 7) {
    std::vector<long long> dp(capacity + 1, 0);
    dp[0] = 1;
    for (int w : weights) {
      for (int j = w; j <= capacity; ++j) {
        dp[j] = (dp[j] + dp[j - w]) % mod;
      }
    }
    return dp;
  }

  struct Item {
    int weight;
    long long value;
  };

  static std::vector<Item> zero_one_path(int capacity, const std::vector<int>& weights, const std::vector<long long>& values) {
    int n = static_cast<int>(weights.size());
    std::vector<long long> dp(capacity + 1, 0);
    std::vector<std::vector<bool>> chosen(n, std::vector<bool>(capacity + 1, false));

    for (int i = 0; i < n; ++i) {
      for (int j = capacity; j >= weights[i]; --j) {
        if (dp[j - weights[i]] + values[i] > dp[j]) {
          dp[j] = dp[j - weights[i]] + values[i];
          chosen[i][j] = true;
        }
      }
    }

    std::vector<Item> result;
    int j = capacity;
    for (int i = n - 1; i >= 0; --i) {
      if (chosen[i][j]) {
        result.push_back({weights[i], values[i]});
        j -= weights[i];
      }
    }
    return result;
  }
};

#include <iostream>

int main() {
  int type;
  std::cin >> type;

  if (type == 1) {
    int n, c;
    std::cin >> n >> c;
    std::vector<int> w(n);
    std::vector<long long> v(n);
    for (int i = 0; i < n; ++i) std::cin >> w[i] >> v[i];
    std::cout << Knapsack::zero_one(c, w, v) << '\n';
  } else if (type == 2) {
    int n, c;
    std::cin >> n >> c;
    std::vector<int> w(n);
    std::vector<long long> v(n);
    for (int i = 0; i < n; ++i) std::cin >> w[i] >> v[i];
    std::cout << Knapsack::complete(c, w, v) << '\n';
  } else if (type == 3) {
    int n, c;
    std::cin >> n >> c;
    std::vector<int> w(n), cnt(n);
    std::vector<long long> v(n);
    for (int i = 0; i < n; ++i) std::cin >> w[i] >> v[i] >> cnt[i];
    std::cout << Knapsack::multiple_binary(c, w, v, cnt) << '\n';
  } else if (type == 4) {
    int n, c;
    std::cin >> n >> c;
    std::vector<int> w(n), cnt(n);
    std::vector<long long> v(n);
    for (int i = 0; i < n; ++i) std::cin >> w[i] >> v[i] >> cnt[i];
    std::cout << Knapsack::multiple_monotonic(c, w, v, cnt) << '\n';
  } else if (type == 5) {
    int g, c;
    std::cin >> g >> c;
    std::vector<std::vector<std::pair<int, long long>>> groups(g);
    for (int i = 0; i < g; ++i) {
      int k;
      std::cin >> k;
      groups[i].resize(k);
      for (int j = 0; j < k; ++j) std::cin >> groups[i][j].first >> groups[i][j].second;
    }
    std::cout << Knapsack::group(c, groups) << '\n';
  } else if (type == 6) {
    int n, c;
    std::cin >> n >> c;
    std::vector<int> w(n), cnt(n);
    std::vector<long long> v(n);
    for (int i = 0; i < n; ++i) std::cin >> w[i] >> v[i] >> cnt[i];
    std::cout << Knapsack::mixed(c, w, v, cnt) << '\n';
  } else if (type == 7) {
    int n, c;
    std::cin >> n >> c;
    std::vector<int> w(n);
    for (int i = 0; i < n; ++i) std::cin >> w[i];
    auto dp = Knapsack::zero_one_count(c, w);
    std::cout << dp[c] << '\n';
  } else if (type == 8) {
    int n, c;
    std::cin >> n >> c;
    std::vector<int> w(n);
    std::vector<long long> v(n);
    for (int i = 0; i < n; ++i) std::cin >> w[i] >> v[i];
    auto items = Knapsack::zero_one_path(c, w, v);
    long long total = 0;
    for (const auto& it : items) total += it.value;
    std::cout << total << '\n';
    for (const auto& it : items) std::cout << it.weight << ' ' << it.value << '\n';
  }

  return 0;
}
