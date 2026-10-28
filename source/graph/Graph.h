// Graph.h - undirected graph using an Adjacency List.
// Nodes are named by strings, e.g. "E:101", "T:Backend", "D:Engineering", "S:Python".
// BFS and DFS both run in O(V + E).
#pragma once
#include <algorithm>
#include <cstddef>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

class Graph {
public:
    // Adds both nodes if missing, then connects them.
    // Returns false for a self-loop or an edge that already exists.
    bool addEdge(const std::string& a, const std::string& b) {
        if (a == b) return false;
        int u = getOrAdd(a), v = getOrAdd(b);
        if (std::find(adj_[u].begin(), adj_[u].end(), v) != adj_[u].end()) return false;
        adj_[u].push_back(v);
        adj_[v].push_back(u);
        ++edges_;
        return true;
    }

    void addNode(const std::string& a) { getOrAdd(a); }
    bool hasNode(const std::string& a) const { return index_.count(a) > 0; }

    bool hasEdge(const std::string& a, const std::string& b) const {
        auto ia = index_.find(a), ib = index_.find(b);
        if (ia == index_.end() || ib == index_.end()) return false;
        const auto& list = adj_[ia->second];
        return std::find(list.begin(), list.end(), ib->second) != list.end();
    }

    std::size_t nodeCount() const { return names_.size(); }
    std::size_t edgeCount() const { return edges_; }

    std::vector<std::string> neighbors(const std::string& a) const {
        std::vector<std::string> out;
        auto it = index_.find(a);
        if (it == index_.end()) return out;
        for (int v : adj_[it->second]) out.push_back(names_[v]);
        return out;
    }

    // All node names that start with the given prefix (in insertion order).
    std::vector<std::string> nodesWithPrefix(const std::string& prefix) const {
        std::vector<std::string> out;
        for (const std::string& n : names_)
            if (n.compare(0, prefix.size(), prefix) == 0) out.push_back(n);
        return out;
    }

    // Breadth-first traversal. maxDepth < 0 means unlimited.
    std::vector<std::string> bfs(const std::string& start, int maxDepth = -1) const {
        std::vector<std::string> order;
        auto it = index_.find(start);
        if (it == index_.end()) return order;

        std::vector<bool> seen(names_.size(), false);
        std::queue<std::pair<int, int>> q;  // (node, depth)
        seen[it->second] = true;
        q.push({it->second, 0});
        while (!q.empty()) {
            auto [u, d] = q.front();
            q.pop();
            order.push_back(names_[u]);
            if (maxDepth >= 0 && d >= maxDepth) continue;
            for (int v : adj_[u]) {
                if (!seen[v]) { seen[v] = true; q.push({v, d + 1}); }
            }
        }
        return order;
    }

    // Depth-first traversal (recursive).
    // Optional 'allowed' filter: nodes for which it returns false are not visited
    // (and are not walked through). Used to stay inside Department -> Team -> Employee.
    std::vector<std::string> dfs(const std::string& start,
                                 const std::function<bool(const std::string&)>& allowed = nullptr) const {
        std::vector<std::string> order;
        auto it = index_.find(start);
        if (it == index_.end()) return order;
        std::vector<bool> seen(names_.size(), false);
        dfsRec(it->second, seen, order, allowed);
        return order;
    }

private:
    std::unordered_map<std::string, int> index_;
    std::vector<std::string> names_;
    std::vector<std::vector<int>> adj_;
    std::size_t edges_ = 0;

    int getOrAdd(const std::string& name) {
        auto it = index_.find(name);
        if (it != index_.end()) return it->second;
        int id = static_cast<int>(names_.size());
        index_[name] = id;
        names_.push_back(name);
        adj_.emplace_back();
        return id;
    }

    void dfsRec(int u, std::vector<bool>& seen, std::vector<std::string>& order,
                const std::function<bool(const std::string&)>& allowed) const {
        seen[u] = true;
        order.push_back(names_[u]);
        for (int v : adj_[u])
            if (!seen[v] && (!allowed || allowed(names_[v]))) dfsRec(v, seen, order, allowed);
    }
};
