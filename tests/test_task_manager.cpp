#include <gtest/gtest.h>
#include "task_manager.h"

namespace {

TEST(TaskManagerTest, AddTask) {
    DistributedML::TaskManager tm;
    nlohmann::json metadata = {{"priority", "high"}};
    std::string taskId = tm.addTask("test_task", metadata);

    EXPECT_FALSE(taskId.empty());

    auto tasks = tm.getAllTasks();
    ASSERT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].id, taskId);
    EXPECT_EQ(tasks[0].type, "test_task");
    EXPECT_EQ(tasks[0].status, DistributedML::TaskStatus::PENDING);
    EXPECT_EQ(tasks[0].metadata, metadata);
}

TEST(TaskManagerTest, UpdateTaskStatus) {
    DistributedML::TaskManager tm;
    std::string taskId = tm.addTask("test_task", nlohmann::json{});

    tm.updateTaskStatus(taskId, DistributedML::TaskStatus::RUNNING);

    auto task = tm.getTaskById(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->status, DistributedML::TaskStatus::RUNNING);
}

TEST(TaskManagerTest, GetAllTasks) {
    DistributedML::TaskManager tm;
    tm.addTask("task1", nlohmann::json{});
    tm.addTask("task2", nlohmann::json{});

    auto tasks = tm.getAllTasks();
    ASSERT_EQ(tasks.size(), 2);
    EXPECT_EQ(tasks[0].type, "task1");
    EXPECT_EQ(tasks[1].type, "task2");
}

TEST(TaskManagerTest, GetTaskById) {
    DistributedML::TaskManager tm;
    std::string taskId = tm.addTask("test_task", nlohmann::json{});

    auto task = tm.getTaskById(taskId);
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->id, taskId);

    auto invalidTask = tm.getTaskById("invalid_id");
    EXPECT_FALSE(invalidTask.has_value());
}

} // namespace