#include "../include/performance_tracker.h"
#include <algorithm>

namespace DistributedML {

void PerformanceTracker::startTracking(const std::string& metricName) {
    PerformanceMetric metric;
    metric.name = metricName;
    metric.startTime = std::chrono::high_resolution_clock::now();
    m_metrics.push_back(metric);
}

void PerformanceTracker::stopTracking(const std::string& metricName) {
    auto it = std::find_if(m_metrics.begin(), m_metrics.end(), 
        [&metricName](const PerformanceMetric& metric) { 
            return metric.name == metricName; 
        });

    if (it != m_metrics.end()) {
        it->endTime = std::chrono::high_resolution_clock::now();
        it->duration = std::chrono::duration<double>(it->endTime - it->startTime).count();
    }
}

nlohmann::json PerformanceTracker::getMetrics() const {
    nlohmann::json metricsJson = nlohmann::json::array();
    for (const auto& metric : m_metrics) {
        metricsJson.push_back({
            {"name", metric.name},
            {"duration", metric.duration}
        });
    }
    return metricsJson;
}

} // namespace DistributedML
