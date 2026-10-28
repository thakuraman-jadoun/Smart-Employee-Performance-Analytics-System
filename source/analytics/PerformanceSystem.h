// PerformanceSystem.h - combines AVL Tree + Graph + Heap into one analytics system.
//   AVL Tree : fast search / filtering of employee records by ID
//   Graph    : Employee-Team, Team-Department, Employee-Skill relationships
//   Heap     : Top-K performers and employees who need support
#pragma once
#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../graph/Graph.h"
#include "../heap/Heap.h"
#include "../models/Employee.h"
#include "../trees/AVLTree.h"

// (score, id) pair stored in the heaps. Ties are broken by lower ID first.
struct Ranked {
    double score = 0;
    int id = 0;
    bool operator<(const Ranked& o) const {
        if (score != o.score) return score < o.score;
        return id > o.id;
    }
    bool operator>(const Ranked& o) const { return o < *this; }
};
using MaxHeap = Heap<Ranked, std::less<Ranked>>;
using MinHeap = Heap<Ranked, std::greater<Ranked>>;

class PerformanceSystem {
public:
    static std::string empNode(int id) { return "E:" + std::to_string(id); }
    static std::string teamNode(const std::string& t) { return "T:" + t; }
    static std::string deptNode(const std::string& d) { return "D:" + d; }
    static std::string skillNode(const std::string& s) { return "S:" + s; }

    // Adds an employee to the AVL Tree and the Graph. False if the ID already exists.
    bool addEmployee(const Employee& e) {
        if (!tree_.insert(e.id, e)) return false;
        graph_.addEdge(empNode(e.id), teamNode(e.team));
        graph_.addEdge(teamNode(e.team), deptNode(e.department));
        for (const std::string& s : e.skills)
            graph_.addEdge(empNode(e.id), skillNode(s));
        return true;
    }

    const Employee* find(int id) const { return tree_.search(id); }
    std::size_t employeeCount() const { return tree_.size(); }
    const AVLTree<Employee>& tree() const { return tree_; }
    const Graph& graph() const { return graph_; }

    // In-order traversal -> employees sorted by ID.
    std::vector<Employee> allSortedById() const {
        std::vector<Employee> out;
        tree_.inorder([&](int, const Employee& e) { out.push_back(e); });
        return out;
    }

    // Filter using in-order traversal and any condition.
    std::vector<Employee> filter(const std::function<bool(const Employee&)>& pred) const {
        std::vector<Employee> out;
        tree_.inorder([&](int, const Employee& e) { if (pred(e)) out.push_back(e); });
        return out;
    }

    std::vector<Employee> filterByMinIndex(double minIndex) const {
        return filter([&](const Employee& e) { return e.performanceIndex() >= minIndex; });
    }

    // Top-K performers (Max Heap)
    std::vector<Employee> topPerformers(std::size_t k) const {
        MaxHeap heap;
        tree_.inorder([&](int id, const Employee& e) { heap.push({e.performanceIndex(), id}); });
        return resolve(heap.topK(k));
    }

    // Employees who may need support = lowest scores (Min Heap)
    std::vector<Employee> needsSupport(std::size_t k) const {
        MinHeap heap;
        tree_.inorder([&](int id, const Employee& e) { heap.push({e.performanceIndex(), id}); });
        return resolve(heap.topK(k));
    }

    // Department names found in the graph.
    std::vector<std::string> departments() const {
        std::vector<std::string> out;
        for (const std::string& n : graph_.nodesWithPrefix("D:")) out.push_back(n.substr(2));
        return out;
    }

    // Case-insensitive lookup of a department name. Returns "" if not found.
    std::string findDepartment(const std::string& name) const {
        for (const std::string& d : departments())
            if (iequals(d, name)) return d;
        return "";
    }

    // DFS from the department node collects every employee of that department.
    std::vector<Employee> employeesInDepartment(const std::string& dept) const {
        std::vector<Employee> out;
        // Stay inside Department -> Team -> Employee (do not walk through skill nodes).
        auto allowed = [](const std::string& n) { return n.compare(0, 2, "S:") != 0; };
        for (const std::string& n : graph_.dfs(deptNode(dept), allowed)) {
            if (n.compare(0, 2, "E:") != 0) continue;
            const Employee* e = tree_.search(std::stoi(n.substr(2)));
            if (e) out.push_back(*e);
        }
        std::sort(out.begin(), out.end(), [](const Employee& a, const Employee& b) { return a.id < b.id; });
        return out;
    }

    // DFS (graph) -> AVL Tree (records) -> Max Heap (best performer)
    std::vector<std::pair<std::string, Employee>> bestPerformerByDepartment() const {
        std::vector<std::pair<std::string, Employee>> out;
        for (const std::string& d : departments()) {
            MaxHeap heap;
            for (const Employee& e : employeesInDepartment(d)) heap.push({e.performanceIndex(), e.id});
            if (heap.empty()) continue;
            const Employee* best = tree_.search(heap.top().id);
            if (best) out.push_back({d, *best});
        }
        return out;
    }

    // Employees who have a given skill (neighbours of the skill node).
    std::vector<Employee> employeesWithSkill(const std::string& skill) const {
        std::vector<Employee> out;
        for (const std::string& n : graph_.neighbors(skillNode(skill))) {
            if (n.compare(0, 2, "E:") != 0) continue;
            const Employee* e = tree_.search(std::stoi(n.substr(2)));
            if (e) out.push_back(*e);
        }
        std::sort(out.begin(), out.end(), [](const Employee& a, const Employee& b) { return a.id < b.id; });
        return out;
    }

    // Colleagues who share at least one skill with the given employee.
    std::vector<Employee> sharedSkillColleagues(int id) const {
        std::set<int> ids;
        for (const std::string& s : graph_.neighbors(empNode(id))) {
            if (s.compare(0, 2, "S:") != 0) continue;
            for (const std::string& n : graph_.neighbors(s))
                if (n.compare(0, 2, "E:") == 0) {
                    int other = std::stoi(n.substr(2));
                    if (other != id) ids.insert(other);
                }
        }
        std::vector<Employee> out;
        for (int x : ids) if (const Employee* e = tree_.search(x)) out.push_back(*e);
        return out;
    }

    // BFS up to 2 hops: employees connected through the same team or a shared skill.
    std::vector<Employee> relatedEmployees(int id) const {
        std::vector<Employee> out;
        for (const std::string& n : graph_.bfs(empNode(id), 2)) {
            if (n.compare(0, 2, "E:") != 0) continue;
            int other = std::stoi(n.substr(2));
            if (other == id) continue;
            if (const Employee* e = tree_.search(other)) out.push_back(*e);
        }
        return out;
    }

private:
    AVLTree<Employee> tree_;
    Graph graph_;

    std::vector<Employee> resolve(const std::vector<Ranked>& ranked) const {
        std::vector<Employee> out;
        for (const Ranked& r : ranked)
            if (const Employee* e = tree_.search(r.id)) out.push_back(*e);
        return out;
    }

    static bool iequals(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
                return false;
        return true;
    }
};
