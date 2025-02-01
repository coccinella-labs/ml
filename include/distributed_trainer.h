#pragma once

#include <mpi.h>
#include <vector>
#include <memory>
#include <limits>
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

namespace DistributedML {

class DistributedTrainer {
public:
    // Training hyperparameters
    struct TrainingConfig {
        double learningRate;
        int epochs;
        int batchSize;
    };

    DistributedTrainer(int argc, char** argv);
    ~DistributedTrainer();

    // Initialize logging system
    static void initializeLogging();

    // Initialize MPI and distributed environment
    void initialize();

    // Validate and set training configuration
    void validateAndSetConfig(const TrainingConfig& config);

    // Distribute training data across nodes
    void distributeData(const std::vector<cv::Mat>& trainingData);

    // Perform distributed training
    void train();

    // Aggregate results from all nodes
    Eigen::MatrixXd aggregateResults();

    // Get performance metrics
    nlohmann::json getPerformanceMetrics() const;

private:
    // Local batch processing
    Eigen::VectorXd processLocalBatch(const std::vector<cv::Mat>& localBatch);

    // Compute local loss
    double computeLocalLoss(const Eigen::VectorXd& localGradient);

    // Aggregate gradients across nodes
    Eigen::VectorXd aggregateGradients(const std::vector<Eigen::VectorXd>& localGradients);

    // Aggregate loss across nodes
    double aggregateLoss(double localLoss);

    // Update model parameters
    void updateModelParameters(const Eigen::VectorXd& globalGradient, double globalLoss);

    // Synchronize initial model parameters
    void synchronizeModelParameters();

    // Early stopping condition
    bool shouldStopTraining(double globalLoss);

    int m_rank;
    int m_worldSize;
    MPI_Comm m_communicator;

    // Local training data
    std::vector<cv::Mat> m_localData;

    // Training configuration
    TrainingConfig m_config;
};

} // namespace DistributedML
