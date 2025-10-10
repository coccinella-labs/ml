#include <gtest/gtest.h>
#include "performance_tracker.h"

namespace {

TEST(PerformanceTrackerTest, AddMetric) {
    DistributedML::PerformanceTracker pt;
    pt.addMetric("test_operation", 1.5);

    auto metrics = pt.getMetrics();
    ASSERT_EQ(metrics.size(), 1);
    EXPECT_EQ(metrics[0]["name"], "test_operation");
    EXPECT_EQ(metrics[0]["duration_ms"], 1.5);
}

TEST(PerformanceTrackerTest, GetMetrics) {
    DistributedML::PerformanceTracker pt;
    pt.addMetric("op1", 1.0);
    pt.addMetric("op2", 2.0);

    auto metrics = pt.getMetrics();
    ASSERT_EQ(metrics.size(), 2);
    EXPECT_EQ(metrics[0]["name"], "op1");
    EXPECT_EQ(metrics[1]["name"], "op2");
}

} // namespace