#include "../include/task_manager.h"

// Comprehensive standard library headers
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <iostream>

namespace DistributedML {

std::string TaskManager::addTask(const std::string& taskType, const nlohmann::json& metadata) {
    std::lock_guard<std::mutex> lock(m_taskMutex);
    
    Task newTask;
    newTask.id = "task_" + std::to_string(++m_taskCounter);
    newTask.type = taskType;
    newTask.status = TaskStatus::PENDING;
    newTask.metadata = metadata;
    newTask.progress = 0.0;

    m_tasks.push_back(std::move(newTask));
    return m_tasks.back().id;
}

void TaskManager::updateTaskStatus(const std::string& taskId, TaskStatus status) {
    std::lock_guard<std::mutex> lock(m_taskMutex);
    
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(), 
        [&taskId](const Task& task) { return task.id == taskId; });
    
    if (it != m_tasks.end()) {
        it->status = status;
        it->progress = (status == TaskStatus::COMPLETED) ? 100.0 : 
                       (status == TaskStatus::RUNNING) ? 50.0 : 0.0;
    }
}

std::vector<TaskManager::Task> TaskManager::getAllTasks() const {
    std::lock_guard<std::mutex> lock(m_taskMutex);
    return m_tasks;
}

TaskManager::Task TaskManager::getTaskById(const std::string& taskId) const {
    std::lock_guard<std::mutex> lock(m_taskMutex);
    
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(), 
        [&taskId](const Task& task) { return task.id == taskId; });
    
    return (it != m_tasks.end()) ? *it : Task{};
}

} // namespace DistributedML
