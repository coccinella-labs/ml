#include "../include/distributed_trainer.h"
#include "../include/dashboard_server.h"
#include <thread>
#include <stdexcept>
#include <iostream>

// Function to generate sample training data
std::vector<cv::Mat> generateTrainingData(int numSamples) {
    std::vector<cv::Mat> trainingData;
    
    // Generate random images for training
    for (int i = 0; i < numSamples; ++i) {
        cv::Mat sample = cv::Mat::zeros(28, 28, CV_32F);
        cv::randu(sample, 0, 1);
        trainingData.push_back(sample);
    }
    
    return trainingData;
}

int main(int argc, char** argv) {
    try {
        // Initialize distributed trainer
        DistributedML::DistributedTrainer trainer(argc, argv);

        // Prepare sample training data
        std::vector<cv::Mat> trainingData = generateTrainingData(1000);

        // Distribute data across nodes
        trainer.distributeData(trainingData);

        // Start training in a separate thread with error handling
        std::exception_ptr trainingException = nullptr;
        std::thread trainingThread([&trainer, &trainingException]() {
            try {
                trainer.train();
            } catch (...) {
                trainingException = std::current_exception();
            }
        });

        // Start dashboard server with error handling
        std::exception_ptr dashboardException = nullptr;
        std::thread dashboardThread([&dashboardException]() {
            try {
                DistributedML::DashboardServer dashboard("http://localhost:8080");
                dashboard.start();
            } catch (...) {
                dashboardException = std::current_exception();
            }
        });

        // Wait for training to complete
        trainingThread.join();

        // Check for training exceptions
        if (trainingException) {
            std::rethrow_exception(trainingException);
        }

        // Stop dashboard
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
