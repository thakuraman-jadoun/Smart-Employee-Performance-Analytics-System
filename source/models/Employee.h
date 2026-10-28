// Employee.h - employee data model and weighted Performance Index
#pragma once
#include <string>
#include <vector>

struct Employee {
    // Weights of the Performance Index (sum = 1.0)
    static constexpr double W_PERFORMANCE = 0.35;
    static constexpr double W_TASKS       = 0.20;
    static constexpr double W_PRODUCTIVITY = 0.20;
    static constexpr double W_ATTENDANCE  = 0.15;
    static constexpr double W_FEEDBACK    = 0.10;

    int id = 0;
    std::string name;
    std::string department;
    std::string team;
    double attendance = 0;        // percentage 0-100
    int tasksAssigned = 0;
    int tasksCompleted = 0;
    double productivity = 0;      // 0-100
    double performanceScore = 0;  // 0-100
    double feedback = 0;          // 0-100
    std::vector<std::string> skills;

    // Task completion as a percentage; 0 when no tasks are assigned (no divide-by-zero).
    double taskCompletionRate() const {
        if (tasksAssigned <= 0) return 0.0;
        double rate = 100.0 * tasksCompleted / tasksAssigned;
        return rate > 100.0 ? 100.0 : rate;
    }

    // Weighted Performance Index (0-100)
    double performanceIndex() const {
        return W_PERFORMANCE * performanceScore +
               W_TASKS * taskCompletionRate() +
               W_PRODUCTIVITY * productivity +
               W_ATTENDANCE * attendance +
               W_FEEDBACK * feedback;
    }
};
