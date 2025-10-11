#include "../include/distributed_trainer.h"
#include "../include/dashboard_server.h"
#include <thread>
#include <stdexcept>
#include <iostream>
#include <Eigen/Dense>
#include <nlohmann/json.hpp>

// Function to generate sample training data
std::vector<Eigen::MatrixXd> generateTrainingData(int numSamples) {
    std::vector<Eigen::MatrixXd> trainingData;
    
    // Generate random images for training
    for (int i = 0; i < numSamples; ++i) {
        Eigen::MatrixXd sample = Eigen::MatrixXd::Zero(28, 28);
        sample.setRandom();
        trainingData.push_back(sample);
    }
    
    return trainingData;
}

int main(int argc, char** argv) {
    try {
        // Initialize distributed trainer
        DistributedML::DistributedTrainer trainer(argc, argv);

        // Prepare sample training data
        std::vector<Eigen::MatrixXd> trainingData = generateTrainingData(1000);

        // Distribute data across nodes
        trainer.distributeData(trainingData);

        // Create dashboard server
        auto dashboard = std::make_shared<DistributedML::DashboardServer>("http://0.0.0.0:8080");

        // Start dashboard server with error handling
        std::exception_ptr dashboardException = nullptr;
        std::thread dashboardThread([dashboard, &dashboardException]() {
            try {
                dashboard->start();
            } catch (...) {
                dashboardException = std::current_exception();
            }
        });

        // Start training in a separate thread with error handling
        std::exception_ptr trainingException = nullptr;
        std::thread trainingThread([&trainer, &trainingException]() {
            try {
                trainer.train();
            } catch (...) {
                trainingException = std::current_exception();
            }
        });

        // Wait for training to complete
        trainingThread.join();

        // Check for training exceptions
        if (trainingException) {
            std::rethrow_exception(trainingException);
        }

        // Wait for dashboard thread to finish (server keeps running)
        dashboardThread.join();

        // Check for dashboard exceptions
        if (dashboardException) {
            std::rethrow_exception(dashboardException);
        }

        // Aggregate and log results
        Eigen::MatrixXd results = trainer.aggregateResults();
        nlohmann::json metrics = trainer.getPerformanceMetrics();

        std::cout << "Training Metrics: " << metrics.dump(4) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 2;
    }

    return 0;
}

// TODO: Improve error handling
// TODO: Refactor code to be more modular
