#include <array>
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <optional>
#include <utility>
#include <algorithm>
#include <span>
#include <ranges>
#include <cassert>

namespace kd {

template<class T>
concept Arithmetic = std::integral<T> || std::floating_point<T>;

template<std::size_t K, class T>
concept ArrayK = requires { typename std::array<T, K>; } && Arithmetic<T>;

template<std::size_t K, Arithmetic T, class Payload = std::monostate>
class KDTree {
public:
    static_assert(K > 0);

    using point_type = std::array<T, K>;
    using payload_type = std::conditional_t<std::is_void_v<Payload>, std::monostate, Payload>;

    struct Item {
        point_type p{};
        payload_type payload{};
    };

private:
    struct Node {
        std::size_t item_index{};
        int axis{};
        std::size_t left = npos, right = npos;
        point_type  lo{}, hi{};
    };

    std::vector<Item> items_;
    std::vector<Node> nodes_;
    std::size_t root_ = npos;

    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

    [[nodiscard]] static constexpr point_type point_minmax(bool is_lo) {
        point_type v{};
        if constexpr (std::floating_point<T>) {
            auto inf = is_lo ? std::numeric_limits<T>::infinity() : -std::numeric_limits<T>::infinity();
            for (auto& x : v) x = is_lo ? inf : -inf; // 先置反，后续会 union 修正
        } else {
            auto lim = is_lo ? std::numeric_limits<T>::max() : std::numeric_limits<T>::lowest();
            for (auto& x : v) x = lim;
        }
        return v;
    }

    [[nodiscard]] static inline long double sqr_ld(long double x) { return x * x; }

    [[nodiscard]] static long double dist2(const point_type& a, const point_type& b) {
        long double s = 0;
        for (std::size_t d = 0; d < K; ++d) {
            long double dx = static_cast<long double>(a[d]) - static_cast<long double>(b[d]);
            s += dx * dx;
        }
        return s;
    }

    [[nodiscard]] static long double box_dist2(const point_type& q, const point_type& lo, const point_type& hi) {
        long double s = 0;
        for (std::size_t d = 0; d < K; ++d) {
            long double v = static_cast<long double>(q[d]);
            long double l = static_cast<long double>(lo[d]);
            long double h = static_cast<long double>(hi[d]);
            if (v < l) s += sqr_ld(l - v);
            else if (v > h) s += sqr_ld(v - h);
        }
        return s;
    }

    static void bbox_union(point_type& lo, point_type& hi, const point_type& p) {
        for (std::size_t d = 0; d < K; ++d) {
            if (p[d] < lo[d]) lo[d] = p[d];
            if (p[d] > hi[d]) hi[d] = p[d];
        }
    }
    static void bbox_union(point_type& lo, point_type& hi, const point_type& lo2, const point_type& hi2) {
        for (std::size_t d = 0; d < K; ++d) {
            if (lo2[d] < lo[d]) lo[d] = lo2[d];
            if (hi2[d] > hi[d]) hi[d] = hi2[d];
        }
    }

    std::size_t build_rec(std::size_t l, std::size_t r, int depth) {
        if (l >= r) return npos;
        const std::size_t mid  = l + (r - l) / 2;
        const int axis = depth % static_cast<int>(K);

        auto first = items_.begin() + static_cast<std::ptrdiff_t>(l);
        auto nth   = items_.begin() + static_cast<std::ptrdiff_t>(mid);
        auto last  = items_.begin() + static_cast<std::ptrdiff_t>(r);

        std::ranges::nth_element(first, nth, last, [axis](const Item& a, const Item& b){ return a.p[static_cast<std::size_t>(axis)] < b.p[static_cast<std::size_t>(axis)]; });

        const std::size_t left = build_rec(l, mid, depth + 1);
        const std::size_t right = build_rec(mid + 1, r, depth + 1);

        Node node{};
        node.item_index = mid;
        node.axis = axis;
        node.left = left;
        node.right = right;
        node.lo = node.hi = items_[mid].p;

        if (left != npos)  bbox_union(node.lo, node.hi, nodes_[left].lo,  nodes_[left].hi);
        if (right != npos) bbox_union(node.lo, node.hi, nodes_[right].lo, nodes_[right].hi);

        nodes_.push_back(node);
        return nodes_.size() - 1;
    }

    void nearest_rec(std::size_t u, const point_type& q, std::size_t& best_idx, long double& best_dist2) const {
        if (u == npos) return;
        const Node& nd = nodes_[u];
        const Item& it = items_[nd.item_index];

        long double d2 = dist2(it.p, q);
        if (d2 < best_dist2) {
            best_dist2 = d2;
            best_idx   = nd.item_index;
        }

        long double dl = (nd.left  != npos) ? box_dist2(q, nodes_[nd.left].lo,  nodes_[nd.left].hi)  : std::numeric_limits<long double>::infinity();
        long double dr = (nd.right != npos) ? box_dist2(q, nodes_[nd.right].lo, nodes_[nd.right].hi) : std::numeric_limits<long double>::infinity();

        std::size_t first = (dl < dr) ? nd.left  : nd.right;
        std::size_t second = (dl < dr) ? nd.right : nd.left;
        long double df = std::min(dl, dr);
        long double ds = std::max(dl, dr);

        if (df < best_dist2) nearest_rec(first, q, best_idx, best_dist2);
        if (ds < best_dist2) nearest_rec(second, q, best_idx, best_dist2);
    }

    template<class Heap>
    void knn_rec(std::size_t u, const point_type& q, std::size_t k, Heap& heap) const {
        if (u == npos) return;
        const Node& nd = nodes_[u];
        const Item& it = items_[nd.item_index];

        long double d2 = dist2(it.p, q);
        if (heap.size() < k) {
            heap.emplace(d2, nd.item_index);
        } else if (d2 < heap.top().first) {
            heap.pop();
            heap.emplace(d2, nd.item_index);
        }

        long double dl = (nd.left  != npos) ? box_dist2(q, nodes_[nd.left].lo,  nodes_[nd.left].hi)  : std::numeric_limits<long double>::infinity();
        long double dr = (nd.right != npos) ? box_dist2(q, nodes_[nd.right].lo, nodes_[nd.right].hi) : std::numeric_limits<long double>::infinity();

        std::size_t first  = (dl < dr) ? nd.left  : nd.right;
        std::size_t second = (dl < dr) ? nd.right : nd.left;

        auto worst = [&]() -> long double {
            return heap.empty() ? std::numeric_limits<long double>::infinity() : heap.top().first;
        };

        if ((first  != npos) && (std::min(dl, dr) < worst()))  knn_rec(first,  q, k, heap);
        if ((second != npos) && (std::max(dl, dr) < worst()))  knn_rec(second, q, k, heap);
    }

    [[nodiscard]] static bool box_intersect(const point_type& lo1, const point_type& hi1, const point_type& lo2, const point_type& hi2) {
        for (std::size_t d = 0; d < K; ++d) {
            if (hi1[d] < lo2[d] || hi2[d] < lo1[d]) return false;
        }
        return true;
    }

    void range_rec(std::size_t u, const point_type& qlo, const point_type& qhi, std::vector<std::size_t>& out) const {
        if (u == npos) return;
        const Node& nd = nodes_[u];
        if (!box_intersect(nd.lo, nd.hi, qlo, qhi)) return;

        const Item& it = items_[nd.item_index];
        bool inside = true;
        for (std::size_t d = 0; d < K; ++d) {
            if (it.p[d] < qlo[d] || it.p[d] > qhi[d]) { inside = false; break; }
        }
        if (inside) out.push_back(nd.item_index);

        range_rec(nd.left,  qlo, qhi, out);
        range_rec(nd.right, qlo, qhi, out);
    }

    void radius_rec(std::size_t u, const point_type& q, long double r2, std::vector<std::size_t>& out) const {
        if (u == npos) return;
        const Node& nd = nodes_[u];
        if (box_dist2(q, nd.lo, nd.hi) > r2) return;

        const Item& it = items_[nd.item_index];
        if (dist2(it.p, q) <= r2) out.push_back(nd.item_index);

        radius_rec(nd.left,  q, r2, out);
        radius_rec(nd.right, q, r2, out);
    }

public:
    KDTree() = default;

    explicit KDTree(std::span<const point_type> pts) {
        items_.reserve(pts.size());
        for (auto const& p : pts) items_.push_back(Item{p, payload_type{}});
        build();
    }

    explicit KDTree(std::span<const std::pair<point_type, payload_type>> data) {
        items_.reserve(data.size());
        for (auto const& v : data) items_.push_back(Item{v.first, v.second});
        build();
    }

    template<std::ranges::input_range R, class Proj = std::identity>
    requires std::convertible_to<std::invoke_result_t<Proj, std::ranges::range_reference_t<R>>, point_type>
    explicit KDTree(R&& r, Proj proj = {}) {
        for (auto&& x : r) items_.push_back(Item{std::invoke(proj, x), payload_type{}});
        build();
    }

    template<std::ranges::input_range R, class Proj = std::identity>
    requires requires(std::ranges::range_reference_t<R> x, Proj proj) {
        { std::get<0>(std::invoke(proj, x)) } -> std::convertible_to<point_type>;
        { std::get<1>(std::invoke(proj, x)) } -> std::convertible_to<payload_type>;
    }
    explicit KDTree(R&& r, Proj proj, int ) {
        for (auto&& x : r) {
            auto y = std::invoke(proj, x);
            items_.push_back(Item{static_cast<point_type>(std::get<0>(y)), static_cast<payload_type>(std::get<1>(y))});
        }
        build();
    }

    void build() {
        nodes_.clear();
        if (items_.empty()) { root_ = npos; return; }
        nodes_.reserve(items_.size());
        root_ = build_rec(0, items_.size(), 0);
    }

    [[nodiscard]] std::size_t size()   const noexcept { return items_.size(); }
    [[nodiscard]] bool empty()  const noexcept { return items_.empty(); }
    [[nodiscard]] std::size_t root()   const noexcept { return root_; }

    [[nodiscard]] const Item& item(std::size_t idx) const { return items_[idx]; }
    [[nodiscard]] const point_type& point(std::size_t idx) const { return items_[idx].p; }

    struct Neighbor { std::size_t index; long double dist2; };

    [[nodiscard]] Neighbor nearest(const point_type& q) const {
        if (empty()) return Neighbor{npos, std::numeric_limits<long double>::infinity()};
        std::size_t best = npos;
        long double bd2 = std::numeric_limits<long double>::infinity();
        nearest_rec(root_, q, best, bd2);
        return Neighbor{best, bd2};
    }

    [[nodiscard]] std::vector<Neighbor> knn(const point_type& q, std::size_t k) const {
        std::vector<Neighbor> ans;
        if (empty() || k == 0) return ans;
        k = std::min(k, size());

        using Pair = std::pair<long double, std::size_t>;
        auto cmp = [](const Pair& a, const Pair& b){ return a.first < b.first ? false : (a.first > b.first ? true : a.second < b.second); };
        std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> heap(cmp);

        knn_rec(root_, q, k, heap);
        ans.reserve(heap.size());
        while (!heap.empty()) {
            auto [d2, idx] = heap.top(); heap.pop();
            ans.push_back(Neighbor{idx, d2});
        }
        std::ranges::sort(ans, [](const Neighbor& a, const Neighbor& b){ return a.dist2 < b.dist2; });
        return ans;
    }

    [[nodiscard]] std::vector<std::size_t> radius_search(const point_type& q, long double radius) const {
        std::vector<std::size_t> out;
        if (empty()) return out;
        long double r2 = radius * radius;
        radius_rec(root_, q, r2, out);
        return out;
    }

    [[nodiscard]] std::vector<std::size_t> range_search(const point_type& qlo, const point_type& qhi) const {
        std::vector<std::size_t> out;
        if (empty()) return out;
        range_rec(root_, qlo, qhi, out);
        return out;
    }
};

} // namespace kd