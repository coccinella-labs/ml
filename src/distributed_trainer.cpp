#include "../include/distributed_trainer.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <nlohmann/json.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace DistributedML {

void DistributedTrainer::initializeLogging() {
    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= boost::log::trivial::info
    );
}

DistributedTrainer::DistributedTrainer(int argc, char** argv) {
    // Initialize logging
    initializeLogging();

    // Initialize MPI with error checking
    int mpi_init_result = MPI_Init(&argc, &argv);
    if (mpi_init_result != MPI_SUCCESS) {
        BOOST_LOG_TRIVIAL(error) << "MPI initialization failed";
        throw std::runtime_error("MPI initialization failed");
    }

    // Log MPI initialization
    BOOST_LOG_TRIVIAL(info) << "MPI initialized successfully";

    // Initialize distributed environment
    try {
        initialize();
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Initialization failed: " << e.what();
        MPI_Finalize();
        throw;
    }
}

DistributedTrainer::~DistributedTrainer() {
    try {
        // Ensure clean MPI shutdown
        int finalize_result = MPI_Finalize();
        if (finalize_result != MPI_SUCCESS) {
            BOOST_LOG_TRIVIAL(warning) << "MPI Finalize failed";
        }
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Error during MPI finalization: " << e.what();
    }
}

void DistributedTrainer::initialize() {
    // Retrieve MPI rank with error handling
    int rank_result = MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
    if (rank_result != MPI_SUCCESS) {
        BOOST_LOG_TRIVIAL(error) << "Failed to retrieve MPI rank";
        throw std::runtime_error("MPI rank retrieval failed");
    }

    // Retrieve world size with error handling
    int size_result = MPI_Comm_size(MPI_COMM_WORLD, &m_worldSize);
    if (size_result != MPI_SUCCESS) {
        BOOST_LOG_TRIVIAL(error) << "Failed to retrieve MPI world size";
        throw std::runtime_error("MPI world size retrieval failed");
    }

    // Set communicator
    m_communicator = MPI_COMM_WORLD;

    // Validate and set default configuration
    validateAndSetConfig({0.01, 100, 32});

    BOOST_LOG_TRIVIAL(info) << "Distributed trainer initialized. Rank: " 
                             << m_rank << ", World Size: " << m_worldSize;
}

void DistributedTrainer::validateAndSetConfig(const TrainingConfig& config) {
    if (config.learningRate <= 0 || config.learningRate > 1.0) {
        BOOST_LOG_TRIVIAL(warning) << "Invalid learning rate. Using default.";
        m_config.learningRate = 0.01;
    } else {
        m_config.learningRate = config.learningRate;
    }

    m_config.epochs = std::max(1, config.epochs);
    m_config.batchSize = std::max(1, config.batchSize);

    BOOST_LOG_TRIVIAL(info) << "Configuration set: LR=" << m_config.learningRate 
                             << ", Epochs=" << m_config.epochs 
                             << ", BatchSize=" << m_config.batchSize;
}

void DistributedTrainer::distributeData(const std::vector<cv::Mat>& trainingData) {
    if (trainingData.empty()) {
        BOOST_LOG_TRIVIAL(error) << "Attempted to distribute empty training data";
        throw std::invalid_argument("Training data is empty");
    }

    int totalDataSize = trainingData.size();
    int dataPerNode = totalDataSize / m_worldSize;
    int remainder = totalDataSize % m_worldSize;

    // Compute start and end indices for data distribution
    int startIndex = m_rank * dataPerNode + std::min(m_rank, remainder);
    int endIndex = startIndex + dataPerNode + (m_rank < remainder ? 1 : 0);

    // Distribute data to local node
    m_localData = std::vector<cv::Mat>(
        trainingData.begin() + startIndex, 
        trainingData.begin() + endIndex
    );

    BOOST_LOG_TRIVIAL(info) << "Node " << m_rank << " received " 
                             << m_localData.size() << " training samples";
}

void DistributedTrainer::train() {
    if (m_localData.empty()) {
        BOOST_LOG_TRIVIAL(warning) << "No local data available for training";
        return;
    }

    // Synchronize initial model parameters across all nodes
    synchronizeModelParameters();

    // Distributed training loop
    for (int epoch = 0; epoch < m_config.epochs; ++epoch) {
        BOOST_LOG_TRIVIAL(info) << "Epoch " << epoch + 1 << "/" << m_config.epochs;
        
        // Local batch processing
        std::vector<Eigen::VectorXd> localGradients;
        double localLoss = 0.0;

        // Process local data in mini-batches
        for (size_t batchStart = 0; batchStart < m_localData.size(); batchStart += m_config.batchSize) {
            auto batchEnd = std::min(batchStart + m_config.batchSize, m_localData.size());
            
            // Simulate local batch training
            Eigen::VectorXd batchGradient = processLocalBatch(
                std::vector<cv::Mat>(m_localData.begin() + batchStart, m_localData.begin() + batchEnd)
            );

            localGradients.push_back(batchGradient);
            localLoss += computeLocalLoss(batchGradient);
        }

        // Aggregate gradients and loss across all nodes
        Eigen::VectorXd globalGradient = aggregateGradients(localGradients);
        double globalLoss = aggregateLoss(localLoss);

        // Update model parameters using distributed optimization
        updateModelParameters(globalGradient, globalLoss);

        // Optional: Early stopping condition
        if (shouldStopTraining(globalLoss)) {
            BOOST_LOG_TRIVIAL(info) << "Early stopping triggered";
            break;
        }
    }

    BOOST_LOG_TRIVIAL(info) << "Distributed training completed";
}

Eigen::VectorXd DistributedTrainer::processLocalBatch(const std::vector<cv::Mat>& localBatch) {
    // Simulate local batch processing and gradient computation
    Eigen::VectorXd localGradient(localBatch.size());
    
    for (size_t i = 0; i < localBatch.size(); ++i) {
        // Placeholder for actual gradient computation
        // In a real implementation, this would involve:
        // 1. Forward pass
        // 2. Loss computation
        // 3. Backward pass to compute gradients
        localGradient(i) = static_cast<double>(i) * m_config.learningRate;
    }

    return localGradient;
}

double DistributedTrainer::computeLocalLoss(const Eigen::VectorXd& localGradient) {
    // Compute local loss based on gradient
    return localGradient.norm();
}

Eigen::VectorXd DistributedTrainer::aggregateGradients(const std::vector<Eigen::VectorXd>& localGradients) {
    // Aggregate gradients across nodes using MPI
    Eigen::VectorXd globalGradient = Eigen::VectorXd::Zero(localGradients[0].size());
    
    // MPI reduction to aggregate gradients
    MPI_Allreduce(
        localGradients[0].data(), 
        globalGradient.data(), 
        localGradients[0].size(), 
        MPI_DOUBLE, 
        MPI_SUM, 
        m_communicator
    );

    // Normalize by number of nodes
    globalGradient /= m_worldSize;

    return globalGradient;
}

double DistributedTrainer::aggregateLoss(double localLoss) {
    double globalLoss = 0.0;
    
    // MPI reduction to aggregate loss
    MPI_Allreduce(
        &localLoss, 
        &globalLoss, 
        1, 
        MPI_DOUBLE, 
        MPI_SUM, 
        m_communicator
    );

    // Normalize by number of nodes
    globalLoss /= m_worldSize;

    return globalLoss;
}

void DistributedTrainer::updateModelParameters(const Eigen::VectorXd& globalGradient, double globalLoss) {
    // Simulate model parameter update
    // In a real implementation, this would update neural network weights
    BOOST_LOG_TRIVIAL(info) << "Global Loss: " << globalLoss 
                             << ", Gradient Norm: " << globalGradient.norm();
}

void DistributedTrainer::synchronizeModelParameters() {
    // Synchronize initial model parameters across nodes using MPI
    Eigen::VectorXd modelParameters = Eigen::VectorXd::Zero(10); // Replace with actual model parameters
    
    // MPI broadcast to synchronize model parameters
    MPI_Bcast(
        modelParameters.data(), 
        modelParameters.size(), 
        MPI_DOUBLE, 
        0, // Root node
        m_communicator
    );

    BOOST_LOG_TRIVIAL(info) << "Model parameters synchronized";
}

bool DistributedTrainer::shouldStopTraining(double globalLoss) {
    // Simple early stopping condition
    static double bestLoss = std::numeric_limits<double>::max();
    static int patience = 3;
    static int noImprovementCount = 0;

    if (globalLoss < bestLoss) {
        bestLoss = globalLoss;
        noImprovementCount = 0;
    } else {
        noImprovementCount++;
    }

    return noImprovementCount >= patience;
}

Eigen::MatrixXd DistributedTrainer::aggregateResults() {
    // Aggregate results across nodes using MPI
    Eigen::MatrixXd localResults(m_localData.size(), 1);
    
    for (size_t i = 0; i < m_localData.size(); ++i) {
        localResults(i, 0) = static_cast<double>(i);
    }

    // MPI gather to collect results from all nodes
    Eigen::MatrixXd globalResults(m_localData.size() * m_worldSize, 1);
    MPI_Gather(
        localResults.data(), 
        localResults.size(), 
        MPI_DOUBLE, 
        globalResults.data(), 
        localResults.size(), 
        MPI_DOUBLE, 
        0, // Root node
        m_communicator
    );

    BOOST_LOG_TRIVIAL(info) << "Results aggregated from node " << m_rank;
    return globalResults;
}

nlohmann::json DistributedTrainer::getPerformanceMetrics() const {
    nlohmann::json metrics;
    metrics["rank"] = m_rank;
    metrics["world_size"] = m_worldSize;
    metrics["local_data_size"] = m_localData.size();
    metrics["learning_rate"] = m_config.learningRate;
    metrics["epochs"] = m_config.epochs;
    metrics["batch_size"] = m_config.batchSize;
    metrics["total_data_size"] = m_localData.size() * m_worldSize;

    return metrics;
}

} // namespace DistributedML
