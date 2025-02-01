#pragma once

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <nlohmann/json.hpp>
#include "task_manager.h"
#include "performance_tracker.h"

namespace DistributedML {

class DashboardServer {
public:
    DashboardServer(const std::string& address);
    
    void start();
    void stop();

private:
    web::http::experimental::listener::http_listener m_listener;
    TaskManager m_taskManager;
    PerformanceTracker m_performanceTracker;

    void handleGetTasks(web::http::http_request request);
    void handleGetPerformance(web::http::http_request request);
    void handleCreateTask(web::http::http_request request);
};
