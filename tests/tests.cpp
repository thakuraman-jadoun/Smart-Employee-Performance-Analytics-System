// tests.cpp - automated tests (no external framework needed)
// Build: g++ -std=c++17 -Isource tests/tests.cpp -o tests_run
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "analytics/PerformanceSystem.h"
#include "models/EmployeeLoader.h"

// ---------- tiny test framework ----------
static int g_failedChecks = 0;
struct TestCase { const char* name; void (*fn)(); };
static std::vector<TestCase>& registry() { static std::vector<TestCase> r; return r; }
struct Registrar { Registrar(const char* n, void (*f)()) { registry().push_back({n, f}); } };

#define TEST(name) static void name(); static Registrar reg_##name(#name, name); static void name()
#define CHECK(cond) do { if (!(cond)) { ++g_failedChecks; \
    std::cout << "    FAILED: " #cond " (line " << __LINE__ << ")\n"; } } while (0)
#define CHECK_NEAR(a, b) CHECK(std::fabs((a) - (b)) < 1e-9)

static Employee makeEmp(int id, const std::string& dept, const std::string& team, double score,
                        std::vector<std::string> skills = {}) {
    Employee e;
    e.id = id; e.name = "Emp" + std::to_string(id); e.department = dept; e.team = team;
    e.attendance = 90; e.tasksAssigned = 10; e.tasksCompleted = 9;
    e.productivity = 80; e.performanceScore = score; e.feedback = 80; e.skills = skills;
    return e;
}

// ================= AVL Tree =================
TEST(avl_empty_tree) {
    AVLTree<int> t;
    CHECK(t.empty()); CHECK(t.size() == 0); CHECK(t.height() == 0); CHECK(t.search(1) == nullptr);
}
TEST(avl_insert_and_search) {
    AVLTree<int> t;
    CHECK(t.insert(10, 100)); CHECK(t.insert(5, 50)); CHECK(t.insert(20, 200));
    CHECK(t.size() == 3);
    CHECK(t.search(5) != nullptr && *t.search(5) == 50);
    CHECK(t.contains(20));
}
TEST(avl_search_missing_key) {
    AVLTree<int> t; t.insert(1, 1);
    CHECK(t.search(999) == nullptr); CHECK(!t.contains(999));
}
TEST(avl_duplicate_rejected) {
    AVLTree<int> t;
    CHECK(t.insert(7, 1)); CHECK(!t.insert(7, 2));
    CHECK(t.size() == 1); CHECK(*t.search(7) == 1);
}
TEST(avl_rotation_LL) {
    AVLTree<int> t; t.insert(30, 0); t.insert(20, 0); t.insert(10, 0);
    CHECK(t.rotations().ll == 1); CHECK(t.height() == 2); CHECK(t.isBalanced());
}
TEST(avl_rotation_RR) {
    AVLTree<int> t; t.insert(10, 0); t.insert(20, 0); t.insert(30, 0);
    CHECK(t.rotations().rr == 1); CHECK(t.height() == 2); CHECK(t.isBalanced());
}
TEST(avl_rotation_LR) {
    AVLTree<int> t; t.insert(30, 0); t.insert(10, 0); t.insert(20, 0);
    CHECK(t.rotations().lr == 1); CHECK(t.height() == 2); CHECK(t.isBalanced());
}
TEST(avl_rotation_RL) {
    AVLTree<int> t; t.insert(10, 0); t.insert(30, 0); t.insert(20, 0);
    CHECK(t.rotations().rl == 1); CHECK(t.height() == 2); CHECK(t.isBalanced());
}
TEST(avl_sequential_inserts_stay_balanced) {
    AVLTree<int> t;
    for (int i = 1; i <= 1000; ++i) t.insert(i, i);
    CHECK(t.isBalanced()); CHECK(t.isBST());
    CHECK(t.height() <= 14);  // 1.44 * log2(1000) ~ 14.4
}
TEST(avl_random_inserts_valid) {
    std::mt19937 rng(42);
    AVLTree<int> t;
    for (int i = 0; i < 2000; ++i) t.insert(static_cast<int>(rng() % 100000), i);
    CHECK(t.isBalanced()); CHECK(t.isBST());
}
TEST(avl_inorder_is_sorted) {
    AVLTree<int> t;
    for (int k : {50, 20, 70, 10, 30, 60, 80}) t.insert(k, k);
    std::vector<int> keys;
    t.inorder([&](int k, int) { keys.push_back(k); });
    CHECK(keys.size() == 7);
    CHECK(std::is_sorted(keys.begin(), keys.end()));
}
TEST(avl_preorder_postorder) {
    AVLTree<int> t;
    for (int k : {2, 1, 3}) t.insert(k, k);
    std::vector<int> pre, post;
    t.preorder([&](int k, int) { pre.push_back(k); });
    t.postorder([&](int k, int) { post.push_back(k); });
    CHECK((pre == std::vector<int>{2, 1, 3}));
    CHECK((post == std::vector<int>{1, 3, 2}));
}

// ================= Graph =================
TEST(graph_add_edge_counts) {
    Graph g;
    CHECK(g.addEdge("A", "B")); CHECK(g.addEdge("B", "C"));
    CHECK(g.nodeCount() == 3); CHECK(g.edgeCount() == 2);
}
TEST(graph_duplicate_and_self_loop_ignored) {
    Graph g; g.addEdge("A", "B");
    CHECK(!g.addEdge("A", "B")); CHECK(!g.addEdge("B", "A")); CHECK(!g.addEdge("A", "A"));
    CHECK(g.edgeCount() == 1);
}
TEST(graph_is_undirected) {
    Graph g; g.addEdge("A", "B");
    CHECK(g.hasEdge("A", "B")); CHECK(g.hasEdge("B", "A")); CHECK(!g.hasEdge("A", "Z"));
}
TEST(graph_bfs_order) {
    Graph g;
    g.addEdge("A", "B"); g.addEdge("A", "C"); g.addEdge("B", "D"); g.addEdge("C", "E");
    CHECK((g.bfs("A") == std::vector<std::string>{"A", "B", "C", "D", "E"}));
}
TEST(graph_dfs_order) {
    Graph g;
    g.addEdge("A", "B"); g.addEdge("A", "C"); g.addEdge("B", "D"); g.addEdge("C", "E");
    CHECK((g.dfs("A") == std::vector<std::string>{"A", "B", "D", "C", "E"}));
}
TEST(graph_bfs_depth_limit) {
    Graph g; g.addEdge("A", "B"); g.addEdge("B", "C"); g.addEdge("C", "D");
    CHECK((g.bfs("A", 2) == std::vector<std::string>{"A", "B", "C"}));
}
TEST(graph_unknown_start_node) {
    Graph g; g.addEdge("A", "B");
    CHECK(g.bfs("Z").empty()); CHECK(g.dfs("Z").empty()); CHECK(g.neighbors("Z").empty());
}
TEST(graph_handles_cycles) {
    Graph g; g.addEdge("A", "B"); g.addEdge("B", "C"); g.addEdge("C", "A");
    CHECK(g.bfs("A").size() == 3); CHECK(g.dfs("A").size() == 3);
}

// ================= Heap =================
TEST(heap_max_order) {
    Heap<int> h;
    for (int v : {5, 1, 9, 3, 7}) h.push(v);
    std::vector<int> out;
    while (!h.empty()) { out.push_back(h.top()); h.pop(); }
    CHECK((out == std::vector<int>{9, 7, 5, 3, 1}));
}
TEST(heap_min_order) {
    Heap<int, std::greater<int>> h;
    for (int v : {5, 1, 9, 3, 7}) h.push(v);
    std::vector<int> out;
    while (!h.empty()) { out.push_back(h.top()); h.pop(); }
    CHECK((out == std::vector<int>{1, 3, 5, 7, 9}));
}
TEST(heap_empty_throws) {
    Heap<int> h;
    bool thrown = false;
    try { h.top(); } catch (const std::out_of_range&) { thrown = true; }
    CHECK(thrown);
}
TEST(heap_topk_is_non_destructive) {
    Heap<int> h;
    for (int v : {4, 8, 2, 6}) h.push(v);
    auto top2 = h.topK(2);
    CHECK((top2 == std::vector<int>{8, 6}));
    CHECK(h.size() == 4); CHECK(h.top() == 8);
}
TEST(heap_k_larger_than_size) {
    Heap<int> h; h.push(1); h.push(2);
    CHECK(h.topK(10).size() == 2);
}

// ================= Employee model =================
TEST(employee_index_all_hundred) {
    Employee e; e.attendance = 100; e.tasksAssigned = 10; e.tasksCompleted = 10;
    e.productivity = 100; e.performanceScore = 100; e.feedback = 100;
    CHECK_NEAR(e.performanceIndex(), 100.0);
}
TEST(employee_index_weighted_value) {
    Employee e; e.performanceScore = 80; e.tasksAssigned = 10; e.tasksCompleted = 5;
    e.productivity = 60; e.attendance = 90; e.feedback = 70;
    // 0.35*80 + 0.20*50 + 0.20*60 + 0.15*90 + 0.10*70 = 70.5
    CHECK_NEAR(e.performanceIndex(), 70.5);
}
TEST(employee_zero_assigned_tasks) {
    Employee e; e.tasksAssigned = 0; e.tasksCompleted = 0;
    CHECK_NEAR(e.taskCompletionRate(), 0.0);
}
TEST(employee_completion_capped_at_100) {
    Employee e; e.tasksAssigned = 5; e.tasksCompleted = 9;
    CHECK_NEAR(e.taskCompletionRate(), 100.0);
}

// ================= CSV loader =================
TEST(loader_missing_file) {
    LoadResult r = loadEmployees("this_file_does_not_exist.csv");
    CHECK(!r.fileOpened); CHECK(r.employees.empty());
}
TEST(loader_parses_valid_and_skips_invalid) {
    const char* tmp = "tmp_test_employees.csv";
    {
        std::ofstream f(tmp);
        f << "id,name,department,team,attendance,tasks_assigned,tasks_completed,productivity,performance_score,feedback,skills\n"
          << "1,Asha,Eng,Core,90,10,9,80,85,70,C++;SQL\n"
          << "abc,Bad,Eng,Core,90,10,9,80,85,70,C++\n"          // bad id
          << "2,Ravi,Eng,Core,150,10,9,80,85,70,C++\n"          // attendance out of range
          << "3,Short,Eng\n"                                    // too few fields
          << "\n# a comment line\n"
          << "4,Meena,Ops,Run,88,0,0,70,75,65,\n";              // zero tasks, no skills
    }
    LoadResult r = loadEmployees(tmp);
    std::remove(tmp);
    CHECK(r.fileOpened); CHECK(r.employees.size() == 2); CHECK(r.skippedLines == 3);
    CHECK(r.employees[0].skills.size() == 2); CHECK(r.employees[1].skills.empty());
}
TEST(loader_sample_dataset) {
    LoadResult r = loadEmployees("data/employees.csv");
    if (!r.fileOpened) { std::cout << "    (skipped: run from project folder)\n"; return; }
    CHECK(r.employees.size() == 12); CHECK(r.skippedLines == 0);
}

// ================= Integrated system =================
static void fillSystem(PerformanceSystem& s) {
    s.addEmployee(makeEmp(1, "Eng", "Core", 90, {"C++", "SQL"}));
    s.addEmployee(makeEmp(2, "Eng", "Core", 70, {"SQL"}));
    s.addEmployee(makeEmp(3, "Data", "ML", 95, {"Python", "SQL"}));
    s.addEmployee(makeEmp(4, "Data", "ML", 60, {"Python"}));
    s.addEmployee(makeEmp(5, "Sales", "B2B", 80, {"CRM"}));
}
TEST(system_duplicate_id_rejected) {
    PerformanceSystem s; fillSystem(s);
    CHECK(!s.addEmployee(makeEmp(1, "Eng", "Core", 50)));
    CHECK(s.employeeCount() == 5);
}
TEST(system_search_known_and_unknown) {
    PerformanceSystem s; fillSystem(s);
    CHECK(s.find(3) != nullptr && s.find(3)->name == "Emp3");
    CHECK(s.find(999) == nullptr);
}
TEST(system_top_k_order) {
    PerformanceSystem s; fillSystem(s);
    auto top = s.topPerformers(2);
    CHECK(top.size() == 2); CHECK(top[0].id == 3); CHECK(top[1].id == 1);
}
TEST(system_top_k_larger_than_n) {
    PerformanceSystem s; fillSystem(s);
    CHECK(s.topPerformers(100).size() == 5);
}
TEST(system_needs_support_order) {
    PerformanceSystem s; fillSystem(s);
    auto low = s.needsSupport(2);
    CHECK(low.size() == 2); CHECK(low[0].id == 4); CHECK(low[1].id == 2);
}
TEST(system_filter_by_index) {
    PerformanceSystem s; fillSystem(s);
    auto good = s.filterByMinIndex(80);
    for (const Employee& e : good) CHECK(e.performanceIndex() >= 80);
    CHECK(!good.empty() && good.size() < 5);
}
TEST(system_department_dfs_does_not_leak_through_skills) {
    PerformanceSystem s; fillSystem(s);   // Eng and Data share the skill "SQL"
    auto eng = s.employeesInDepartment("Eng");
    CHECK(eng.size() == 2);
    for (const Employee& e : eng) CHECK(e.department == "Eng");
}
TEST(system_best_performer_by_department) {
    PerformanceSystem s; fillSystem(s);
    auto best = s.bestPerformerByDepartment();
    CHECK(best.size() == 3);
    for (const auto& [dept, e] : best) {
        if (dept == "Eng") CHECK(e.id == 1);
        if (dept == "Data") CHECK(e.id == 3);
        if (dept == "Sales") CHECK(e.id == 5);
    }
}
TEST(system_shared_skill_colleagues) {
    PerformanceSystem s; fillSystem(s);
    auto c = s.sharedSkillColleagues(1);    // employee 1 has C++, SQL
    CHECK(c.size() == 2);                   // employees 2 and 3 have SQL
}
TEST(system_graph_structure) {
    PerformanceSystem s; fillSystem(s);
    CHECK(s.graph().hasEdge("E:1", "T:Core"));
    CHECK(s.graph().hasEdge("T:Core", "D:Eng"));
    CHECK(s.graph().hasEdge("E:3", "S:Python"));
}

int main() {
    int passed = 0;
    for (const TestCase& t : registry()) {
        int before = g_failedChecks;
        t.fn();
        bool ok = (g_failedChecks == before);
        if (ok) ++passed;
        std::cout << (ok ? "[PASS] " : "[FAIL] ") << t.name << "\n";
    }
    std::cout << "\n" << passed << " / " << registry().size() << " tests passed\n";
    return passed == static_cast<int>(registry().size()) ? 0 : 1;
}
