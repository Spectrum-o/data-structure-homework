#include <string_view>
#include <vector>

inline std::vector<int> kmp_prefix_function(std::string_view p) {
    std::vector<int> pi(p.size(), 0);
    for (size_t i = 1; i < p.size(); ++i) {
        int j = pi[i - 1];
        while (j > 0 && p[i] != p[(size_t)j]) j = pi[(size_t)(j - 1)];
        if (p[i] == p[(size_t)j]) ++j;
        pi[i] = j;
    }
    return pi;
}

inline std::vector<int> kmp_search(std::string_view t, std::string_view p) {
    std::vector<int> ans;
    if (p.empty()) {
        ans.reserve(t.size() + 1);
        for (size_t i = 0; i <= t.size(); ++i) ans.push_back(static_cast<int>(i));
        return ans;
    }

    auto pi = kmp_prefix_function(p);
    int j = 0;
    for (size_t i = 0; i < t.size(); ++i) {
        while (j > 0 && t[i] != p[(size_t)j]) j = pi[(size_t)(j - 1)];
        if (t[i] == p[(size_t)j]) ++j;
        if (j == (int)p.size()) {
            ans.push_back(static_cast<int>(i + 1 - p.size()));
            j = pi[(size_t)(j - 1)];
        }
    }
    return ans;
}
