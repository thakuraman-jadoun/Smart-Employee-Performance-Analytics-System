# Smart Employee Performance Analytics System

**Course:** Data Structures and Algorithms – II (CCSE0301), NIET Greater Noida
**Student:** Aman Pratap Singh (0251CSE287 / 2501330100057)
**Faculty:** Mr. Shamshad Ali  |  **SDG 8** – Decent Work and Economic Growth

A C++ console prototype that stores employee performance data and analyses it with
Trees, Graphs and Heaps.

## Data structures used

| Structure | File | Purpose |
|-----------|------|---------|
| AVL Tree | `source/trees/AVLTree.h` | Balanced storage, O(log n) search by Employee ID, sorted/filtered output via in-order traversal |
| Graph (adjacency list) + BFS/DFS | `source/graph/Graph.h` | Employee–Team, Team–Department, Employee–Skill relationships |
| Max Heap / Min Heap | `source/heap/Heap.h` | Top-K performers and employees who may need support |
| Performance Index | `source/models/Employee.h` | Weighted score: Performance 35%, Task Completion 20%, Productivity 20%, Attendance 15%, Feedback 10% |

`source/analytics/PerformanceSystem.h` combines all three, e.g. best performer of a department =
**DFS** (graph) → **AVL Tree** (records) → **Max Heap** (best score).

## Project structure

```
Smart-Employee-Performance-Analytics-System/
├── data/
│   └── employees.csv
├── source/
│   ├── analytics/PerformanceSystem.h
│   ├── graph/Graph.h
│   ├── heap/Heap.h
│   ├── models/Employee.h
│   ├── models/EmployeeLoader.h
│   ├── trees/AVLTree.h
│   └── main.cpp
├── tests/
│   ├── tests.cpp
│   └── benchmark.cpp
├── Makefile
└── README.md
```

## Build and run

Needs a C++17 compiler (g++ / MinGW). Run all commands from the project folder.

```bash
# Linux / macOS (with make)
make            # builds analytics, tests_run, benchmark
make run        # interactive menu
make demo       # runs every feature once
make test       # automated tests
make bench      # benchmark (10 to 5000 employees)

# Windows (PowerShell / CMD) or without make
g++ -std=c++17 -O2 -Isource source/main.cpp -o analytics.exe
g++ -std=c++17 -O2 -Isource tests/tests.cpp -o tests_run.exe
g++ -std=c++17 -O2 -Isource tests/benchmark.cpp -o benchmark.exe
.\analytics.exe            # menu
.\analytics.exe --demo     # demo
.\tests_run.exe
.\benchmark.exe
```

You can pass your own CSV: `./analytics path/to/file.csv`

## CSV format

```
id,name,department,team,attendance,tasks_assigned,tasks_completed,productivity,performance_score,feedback,skills
105,Sneha Iyer,Data,Analytics,98,42,41,94,96,95,Python;SQL;Machine-Learning
```
Percent fields must be 0–100; skills are separated by `;`. Invalid rows are skipped and counted,
duplicate IDs are ignored.

## Complexity

| Operation | Time |
|-----------|------|
| AVL insert / search | O(log n) |
| BFS / DFS | O(V + E) |
| Heap push / pop | O(log n) |
| Top-K (non-destructive, works on a copy of the heap) | O(n + K log n) |

## Limitations

Interim prototype on a small sample dataset; no database or user interface; review-history
trend analysis is not implemented.
