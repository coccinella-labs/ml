#pragma once

#include <chrono>
#include <vector>
#include <nlohmann/json.hpp>

namespace DistributedML {

class PerformanceTracker {
public:
    // Start tracking a specific metric
    void startTracking(const std::string& metricName);

    // Stop tracking and record metric
    void stopTracking(const std::string& metricName);

    // Add a metric with duration (for testing)
    void addMetric(const std::string& metricName, double duration);

    // Get performance metrics as JSON
    nlohmann::json getMetrics() const;

private:
    struct PerformanceMetric {
        std::string name;
        std::chrono::high_resolution_clock::time_point startTime;
        std::chrono::high_resolution_clock::time_point endTime;
        double duration;
    };

    std::vector<PerformanceMetric> m_metrics;
};

} // namespace DistributedML
