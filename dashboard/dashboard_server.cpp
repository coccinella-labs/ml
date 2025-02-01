#include "../include/dashboard_server.h"
#include <iostream>

namespace DistributedML {

DashboardServer::DashboardServer(const std::string& address) 
    : m_listener(address) {
    
    // Setup routes
    m_listener.support(web::http::methods::GET, 
        [this](web::http::http_request request) {
            if (request.request_uri().path() == "/tasks") {
                handleGetTasks(request);
            } else if (request.request_uri().path() == "/performance") {
                handleGetPerformance(request);
            }
        });

    m_listener.support(web::http::methods::POST, 
        [this](web::http::http_request request) {
            if (request.request_uri().path() == "/tasks") {
                handleCreateTask(request);
            }
        });
}

void DashboardServer::start() {
    m_listener.open().wait();
    std::cout << "Dashboard server listening on: " 
              << m_listener.uri().to_string() << std::endl;
}

void DashboardServer::stop() {
    m_listener.close().wait();
}

void DashboardServer::handleGetTasks(web::http::http_request request) {
    auto tasks = m_taskManager.getAllTasks();
    web::json::value response = web::json::value::array();

    for (const auto& task : tasks) {
        web::json::value taskJson;
        taskJson[U"id"] = web::json::value::string(task.id);
        taskJson[U"type"] = web::json::value::string(task.type);
        taskJson[U"status"] = web::json::value::string(
            std::to_string(static_cast<int>(task.status))
        );
        taskJson[U"progress"] = web::json::value::number(task.progress);
        
        response[response.size()] = taskJson;
    }

    request.reply(web::http::status_codes::OK, response);
}

void DashboardServer::handleGetPerformance(web::http::http_request request) {
    auto metrics = m_performanceTracker.getMetrics();
    web::json::value response;

    for (const auto& metric : metrics) {
        response[U"metrics"][metric["name"].get<std::string>()] = 
            web::json::value::number(metric["duration_ms"].get<double>());
    }

    request.reply(web::http::status_codes::OK, response);
}

void DashboardServer::handleCreateTask(web::http::http_request request) {
    request.extract_json().then([this, request](web::json::value body) {
        std::string taskType = body[U"type"].as_string();
        nlohmann::json metadata = nlohmann::json::parse(body.serialize());

        std::string taskId = m_taskManager.addTask(taskType, metadata);
        
        web::json::value response;
        response[U"task_id"] = web::json::value::string(taskId);
        request.reply(web::http::status_codes::Created, response);
    }).wait();
}

} // namespace DistributedML
