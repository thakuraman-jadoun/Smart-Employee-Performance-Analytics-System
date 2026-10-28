// EmployeeLoader.h - reads employees from a CSV file
// Format: id,name,department,team,attendance,tasks_assigned,tasks_completed,
//         productivity,performance_score,feedback,skills   (skills separated by ';')
#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "Employee.h"

struct LoadResult {
    std::vector<Employee> employees;
    bool fileOpened = false;
    int skippedLines = 0;   // malformed / invalid rows
};

namespace loader_detail {

inline std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) --e;
    return s.substr(b, e - b);
}

inline std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == delim) { out.push_back(trim(cur)); cur.clear(); }
        else cur += c;
    }
    out.push_back(trim(cur));
    return out;
}

inline bool toInt(const std::string& s, int& out) {
    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) { return false; }
}

inline bool toDouble(const std::string& s, double& out) {
    try {
        size_t pos = 0;
        out = std::stod(s, &pos);
        return pos == s.size();
    } catch (...) { return false; }
}

inline bool inRange(double v) { return v >= 0.0 && v <= 100.0; }

} // namespace loader_detail

// Parses one CSV line. Returns false if the line is invalid.
inline bool parseEmployeeLine(const std::string& line, Employee& e) {
    using namespace loader_detail;
    std::vector<std::string> f = split(line, ',');
    if (f.size() < 10) return false;

    Employee t;
    if (!toInt(f[0], t.id) || t.id <= 0) return false;
    if (f[1].empty() || f[2].empty() || f[3].empty()) return false;
    t.name = f[1];
    t.department = f[2];
    t.team = f[3];
    if (!toDouble(f[4], t.attendance) || !inRange(t.attendance)) return false;
    if (!toInt(f[5], t.tasksAssigned) || t.tasksAssigned < 0) return false;
    if (!toInt(f[6], t.tasksCompleted) || t.tasksCompleted < 0) return false;
    if (!toDouble(f[7], t.productivity) || !inRange(t.productivity)) return false;
    if (!toDouble(f[8], t.performanceScore) || !inRange(t.performanceScore)) return false;
    if (!toDouble(f[9], t.feedback) || !inRange(t.feedback)) return false;
    if (f.size() > 10 && !f[10].empty()) {
        for (const std::string& s : split(f[10], ';'))
            if (!s.empty()) t.skills.push_back(s);
    }
    e = t;
    return true;
}

inline LoadResult loadEmployees(const std::string& path) {
    LoadResult result;
    std::ifstream in(path);
    if (!in.is_open()) return result;
    result.fileOpened = true;

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        std::string t = loader_detail::trim(line);
        if (t.empty() || t[0] == '#') continue;
        if (lineNo == 1 && t.rfind("id", 0) == 0) continue;  // header row
        Employee e;
        if (parseEmployeeLine(t, e)) result.employees.push_back(e);
        else ++result.skippedLines;
    }
    return result;
}
