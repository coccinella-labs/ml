#include "../include/dashboard_server.h"
#include <iostream>

namespace DistributedML {

DashboardServer::DashboardServer(const std::string& address) 
    : m_listener(address) {
    
    // Setup routes
    m_listener.support(web::http::methods::GET,
        [this](web::http::http_request request) {
            if (request.request_uri().path() == "/") {
                handleGetRoot(request);
            } else if (request.request_uri().path() == "/tasks") {
                handleGetTasks(request);
            } else if (request.request_uri().path() == "/performance") {
                handleGetPerformance(request);
            } else {
                request.reply(web::http::status_codes::NotFound);
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
        taskJson["id"] = web::json::value::string(task.id);
        taskJson["type"] = web::json::value::string(task.type);
        taskJson["status"] = web::json::value::string(
            std::to_string(static_cast<int>(task.status))
        );
        taskJson["progress"] = web::json::value::number(task.progress);
        
        response[response.size()] = taskJson;
    }

    request.reply(web::http::status_codes::OK, response);
}

void DashboardServer::handleGetPerformance(web::http::http_request request) {
    auto metrics = m_performanceTracker.getMetrics();
    web::json::value response;

    for (const auto& metric : metrics) {
        response["metrics"][metric["name"].get<std::string>()] =
            web::json::value::number(metric["duration"].get<double>());
    }

    request.reply(web::http::status_codes::OK, response);
}

void DashboardServer::handleGetRoot(web::http::http_request request) {
    web::json::value response;
    response["message"] = web::json::value::string("Distributed ML Dashboard API");
    response["version"] = web::json::value::string("1.0.0");
    response["endpoints"] = web::json::value::array();
    response["endpoints"][0] = web::json::value::string("GET /tasks - Get all tasks");
    response["endpoints"][1] = web::json::value::string("GET /performance - Get performance metrics");
    response["endpoints"][2] = web::json::value::string("POST /tasks - Create a new task");

    request.reply(web::http::status_codes::OK, response);
}

void DashboardServer::handleCreateTask(web::http::http_request request) {
    request.extract_json().then([this, request](web::json::value body) {
        std::string taskType = body["type"].as_string();
        nlohmann::json metadata = nlohmann::json::parse(body.serialize());

        std::string taskId = m_taskManager.addTask(taskType, metadata);

        web::json::value response;
        response["task_id"] = web::json::value::string(taskId);
        request.reply(web::http::status_codes::Created, response);
    }).wait();
}

} // namespace DistributedML
