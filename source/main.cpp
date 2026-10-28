// main.cpp - Smart Employee Performance Analytics System (console application)
// Usage:  ./analytics [path/to/employees.csv] [--demo]
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "analytics/PerformanceSystem.h"
#include "models/EmployeeLoader.h"

namespace {

void printHeader() {
    std::cout << std::left << std::setw(6) << "ID" << std::setw(18) << "Name"
              << std::setw(13) << "Department" << std::setw(13) << "Team"
              << std::right << std::setw(9) << "Index" << "  Skills\n"
              << std::string(86, '-') << "\n";
}

void printRow(const Employee& e) {
    std::string skills;
    for (size_t i = 0; i < e.skills.size(); ++i) skills += (i ? ", " : "") + e.skills[i];
    std::cout << std::left << std::setw(6) << e.id << std::setw(18) << e.name
              << std::setw(13) << e.department << std::setw(13) << e.team
              << std::right << std::setw(9) << std::fixed << std::setprecision(2)
              << e.performanceIndex() << "  " << skills << "\n";
}

void printList(const std::vector<Employee>& list) {
    if (list.empty()) { std::cout << "No employees found.\n"; return; }
    printHeader();
    for (const Employee& e : list) printRow(e);
}

void printDetails(const Employee& e) {
    std::cout << "ID: " << e.id << "\nName: " << e.name << "\nDepartment: " << e.department
              << "\nTeam: " << e.team << "\nAttendance: " << e.attendance << "%"
              << "\nTasks: " << e.tasksCompleted << "/" << e.tasksAssigned
              << " (" << std::fixed << std::setprecision(1) << e.taskCompletionRate() << "%)"
              << "\nProductivity: " << e.productivity
              << "\nPerformance Score: " << e.performanceScore
              << "\nFeedback: " << e.feedback
              << "\nPerformance Index: " << std::setprecision(2) << e.performanceIndex() << "\n";
}

void printStats(const PerformanceSystem& sys) {
    std::cout << "Employees in AVL Tree : " << sys.tree().size() << "\n"
              << "AVL height            : " << sys.tree().height() << "\n"
              << "AVL balanced          : " << (sys.tree().isBalanced() ? "yes" : "NO") << "\n"
              << "AVL is valid BST      : " << (sys.tree().isBST() ? "yes" : "NO") << "\n"
              << "Rotations (LL/RR/LR/RL): " << sys.tree().rotations().ll << "/" << sys.tree().rotations().rr
              << "/" << sys.tree().rotations().lr << "/" << sys.tree().rotations().rl << "\n"
              << "Graph nodes           : " << sys.graph().nodeCount() << "\n"
              << "Graph edges           : " << sys.graph().edgeCount() << "\n";
}

void printBestByDept(const PerformanceSystem& sys) {
    printHeader();
    for (const auto& [dept, e] : sys.bestPerformerByDepartment()) {
        std::cout << "[" << dept << "]\n";
        printRow(e);
    }
}

void printTraversals(const PerformanceSystem& sys) {
    auto show = [](const char* title, const std::function<void(std::function<void(int, const Employee&)>)>& run) {
        std::cout << title << ": ";
        run([](int id, const Employee&) { std::cout << id << " "; });
        std::cout << "\n";
    };
    show("In-order  ", [&](std::function<void(int, const Employee&)> f) { sys.tree().inorder(f); });
    show("Pre-order ", [&](std::function<void(int, const Employee&)> f) { sys.tree().preorder(f); });
    show("Post-order", [&](std::function<void(int, const Employee&)> f) { sys.tree().postorder(f); });
}

bool readInt(const std::string& prompt, int& out) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) return false;
    try {
        size_t pos = 0;
        out = std::stoi(line, &pos);
        return pos == line.size();
    } catch (...) {
        std::cout << "Invalid number.\n";
        return false;
    }
}

void runDemo(const PerformanceSystem& sys) {
    std::cout << "\n=== System statistics ===\n"; printStats(sys);
    std::cout << "\n=== Tree traversals (employee IDs) ===\n"; printTraversals(sys);
    std::cout << "\n=== Search ID 105 ===\n";
    if (const Employee* e = sys.find(105)) printDetails(*e); else std::cout << "Not found\n";
    std::cout << "\n=== Search ID 999 ===\n";
    if (sys.find(999)) std::cout << "Found\n"; else std::cout << "Employee 999 not found\n";
    std::cout << "\n=== Filter: Performance Index >= 85 ===\n"; printList(sys.filterByMinIndex(85));
    std::cout << "\n=== Top 3 performers (Max Heap) ===\n"; printList(sys.topPerformers(3));
    std::cout << "\n=== Employees who may need support (Min Heap, 2) ===\n"; printList(sys.needsSupport(2));
    std::cout << "\n=== Engineering employees (DFS) ===\n"; printList(sys.employeesInDepartment("Engineering"));
    std::cout << "\n=== Best performer per department ===\n"; printBestByDept(sys);
    std::cout << "\n=== Shared-skill colleagues of ID 105 ===\n"; printList(sys.sharedSkillColleagues(105));
    std::cout << "\n=== Related employees of ID 105 (BFS, 2 hops) ===\n"; printList(sys.relatedEmployees(105));
}

void menu() {
    std::cout << "\n========== Smart Employee Performance Analytics ==========\n"
              << " 1. Search employee by ID (AVL Tree)\n"
              << " 2. List all employees sorted by ID (in-order)\n"
              << " 3. Filter by minimum Performance Index\n"
              << " 4. Top-K performers (Max Heap)\n"
              << " 5. Employees who may need support (Min Heap)\n"
              << " 6. Employees of a department (DFS)\n"
              << " 7. Best performer of each department (DFS + AVL + Heap)\n"
              << " 8. Colleagues with shared skills / related employees (Graph, BFS)\n"
              << " 9. Tree traversals\n"
              << "10. System statistics\n"
              << " 0. Exit\n"
              << "Choice: ";
}

} // namespace

int main(int argc, char** argv) {
    std::string path = "data/employees.csv";
    bool demo = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--demo") demo = true;
        else path = a;
    }

    LoadResult loaded = loadEmployees(path);
    if (!loaded.fileOpened) {
        std::cerr << "Error: could not open file '" << path << "'.\n"
                  << "Run the program from the project folder or pass the CSV path as an argument.\n";
        return 1;
    }

    PerformanceSystem sys;
    int duplicates = 0;
    for (const Employee& e : loaded.employees)
        if (!sys.addEmployee(e)) ++duplicates;

    std::cout << "Loaded " << sys.employeeCount() << " employees from " << path;
    if (loaded.skippedLines) std::cout << " (" << loaded.skippedLines << " invalid rows skipped)";
    if (duplicates) std::cout << " (" << duplicates << " duplicate IDs ignored)";
    std::cout << "\n";

    if (demo) { runDemo(sys); return 0; }

    std::string line;
    while (true) {
        menu();
        if (!std::getline(std::cin, line)) break;
        int choice = -1;
        try { choice = std::stoi(line); } catch (...) { std::cout << "Please enter a number.\n"; continue; }
        if (choice == 0) break;

        int n = 0;
        switch (choice) {
            case 1:
                if (readInt("Employee ID: ", n)) {
                    if (const Employee* e = sys.find(n)) printDetails(*e);
                    else std::cout << "Employee " << n << " not found.\n";
                }
                break;
            case 2: printList(sys.allSortedById()); break;
            case 3: {
                std::cout << "Minimum Performance Index (0-100): ";
                std::string s; std::getline(std::cin, s);
                try { printList(sys.filterByMinIndex(std::stod(s))); }
                catch (...) { std::cout << "Invalid number.\n"; }
                break;
            }
            case 4: if (readInt("K: ", n) && n > 0) printList(sys.topPerformers(n)); break;
            case 5: if (readInt("K: ", n) && n > 0) printList(sys.needsSupport(n)); break;
            case 6: {
                std::cout << "Departments:";
                for (const auto& d : sys.departments()) std::cout << " " << d;
                std::cout << "\nDepartment name: ";
                std::string s; std::getline(std::cin, s);
                std::string d = sys.findDepartment(s);
                if (d.empty()) std::cout << "Department not found.\n";
                else printList(sys.employeesInDepartment(d));
                break;
            }
            case 7: printBestByDept(sys); break;
            case 8:
                if (readInt("Employee ID: ", n)) {
                    if (!sys.find(n)) { std::cout << "Employee " << n << " not found.\n"; break; }
                    std::cout << "-- Share at least one skill --\n"; printList(sys.sharedSkillColleagues(n));
                    std::cout << "-- Related within 2 hops (BFS) --\n"; printList(sys.relatedEmployees(n));
                }
                break;
            case 9: printTraversals(sys); break;
            case 10: printStats(sys); break;
            default: std::cout << "Invalid choice.\n";
        }
    }
    std::cout << "Goodbye!\n";
    return 0;
}
