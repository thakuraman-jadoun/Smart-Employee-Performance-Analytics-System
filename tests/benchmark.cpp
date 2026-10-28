// benchmark.cpp - measures AVL Tree and Heap behaviour for 10 to 5000 employees.
// Build: g++ -std=c++17 -O2 -Isource tests/benchmark.cpp -o benchmark
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

#include "analytics/PerformanceSystem.h"

using Clock = std::chrono::steady_clock;
static double microsSince(Clock::time_point t0) {
    return std::chrono::duration<double, std::micro>(Clock::now() - t0).count();
}

static Employee randomEmployee(int id, std::mt19937& rng) {
    static const char* depts[] = {"Engineering", "Data", "Sales", "HR", "Finance"};
    std::uniform_real_distribution<double> pct(50.0, 100.0);
    Employee e;
    e.id = id;
    e.name = "Emp" + std::to_string(id);
    e.department = depts[rng() % 5];
    e.team = e.department + "-T" + std::to_string(rng() % 4);
    e.attendance = pct(rng);
    e.tasksAssigned = 20 + static_cast<int>(rng() % 30);
    e.tasksCompleted = static_cast<int>(e.tasksAssigned * pct(rng) / 100.0);
    e.productivity = pct(rng);
    e.performanceScore = pct(rng);
    e.feedback = pct(rng);
    return e;
}

int main() {
    const std::vector<int> sizes = {10, 100, 500, 1000, 5000};
    std::mt19937 rng(2026);

    std::cout << std::left << std::setw(8) << "n" << std::setw(8) << "height" << std::setw(12) << "1.44*log2n"
              << std::setw(16) << "insert all(us)" << std::setw(16) << "search avg(us)"
              << std::setw(18) << "Top-5 heap(us)" << std::setw(18) << "Top-5 sort(us)" << "\n"
              << std::string(96, '-') << "\n";

    for (int n : sizes) {
        std::vector<int> ids(n);
        std::iota(ids.begin(), ids.end(), 1);
        std::shuffle(ids.begin(), ids.end(), rng);

        AVLTree<Employee> tree;
        std::vector<Employee> all;
        for (int id : ids) all.push_back(randomEmployee(id, rng));

        auto t0 = Clock::now();
        for (const Employee& e : all) tree.insert(e.id, e);
        double insertUs = microsSince(t0);

        // average search time over 1000 random lookups
        volatile int sink = 0;
        t0 = Clock::now();
        for (int i = 0; i < 1000; ++i) {
            const Employee* e = tree.search(1 + static_cast<int>(rng() % n));
            if (e) sink = sink + e->id;
        }
        double searchUs = microsSince(t0) / 1000.0;

        MaxHeap heap;
        tree.inorder([&](int id, const Employee& e) { heap.push({e.performanceIndex(), id}); });

        const int reps = 200;
        t0 = Clock::now();
        for (int i = 0; i < reps; ++i) { auto r = heap.topK(5); sink = sink + static_cast<int>(r.size()); }
        double heapUs = microsSince(t0) / reps;

        // baseline: sort every employee, take first 5
        std::vector<Ranked> flat;
        tree.inorder([&](int id, const Employee& e) { flat.push_back({e.performanceIndex(), id}); });
        t0 = Clock::now();
        for (int i = 0; i < reps; ++i) {
            std::vector<Ranked> copy = flat;
            std::sort(copy.begin(), copy.end(), [](const Ranked& a, const Ranked& b) { return b < a; });
            sink = sink + static_cast<int>(copy[0].id);
        }
        double sortUs = microsSince(t0) / reps;

        std::cout << std::left << std::fixed << std::setprecision(2)
                  << std::setw(8) << n << std::setw(8) << tree.height()
                  << std::setw(12) << 1.44 * std::log2(n + 1)
                  << std::setw(16) << insertUs << std::setw(16) << searchUs
                  << std::setw(18) << heapUs << std::setw(18) << sortUs << "\n";
    }
    std::cout << "\nNote: times depend on your computer; run it on your machine and use YOUR numbers in the report.\n";
    return 0;
}
