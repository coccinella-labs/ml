#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <nlohmann/json.hpp>

// Explicitly use global namespace for standard types
using std::string;
using std::vector;
using std::mutex;

namespace DistributedML {

enum class TaskStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED
};

class TaskManager {
public:
    struct Task {
        string id;
        string type;
        TaskStatus status;
        nlohmann::json metadata;
        double progress;
    };

    // Constructor and destructor
    TaskManager() : m_taskCounter(0) {}
    ~TaskManager() = default;

    // Prevent copying
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;

    // Task management methods
    string addTask(const string& taskType, const nlohmann::json& metadata);
    void updateTaskStatus(const string& taskId, TaskStatus status);
    vector<Task> getAllTasks() const;
    Task getTaskById(const string& taskId) const;

private:
    mutable mutex m_taskMutex;
    vector<Task> m_tasks;
    size_t m_taskCounter;
}; // class TaskManager

} // namespace DistributedML
