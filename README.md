# Distributed Machine Learning Framework

## Overview
This project implements a distributed machine learning framework with real-time performance monitoring and task tracking.

## Prerequisites
- C++17 Compiler
- MPI (OpenMPI or MPICH)
- OpenCV
- Eigen3
- CPP Rest SDK
- nlohmann/json

## Build Instructions
```bash
mkdir build && cd build
cmake ..
make
```

## Running the Application
```bash
mpirun -n <num_processes> ./distributed_ml_app
```

## Dashboard
Access the dashboard at `http://localhost:8080`

## Features
- Distributed Training
- Real-time Task Monitoring
- Performance Metrics Tracking
- Web-based Dashboard

## Architecture
- Distributed Trainer: Manages distributed machine learning tasks
- Task Manager: Tracks and manages individual tasks
- Performance Tracker: Monitors and records performance metrics
- Dashboard Server: Provides a web interface for monitoring

## Kubernetes Deployment

### Prerequisites
- Kubernetes Cluster
- kubectl
- Helm (optional)

### Docker Image Build
```bash
docker build -t distributed-ml-app:latest .
```

### Kubernetes Deployment Options

#### 1. Direct Kubernetes Deployment
```bash
# Apply Kubernetes manifests
kubectl apply -f k8s/deployment.yaml
kubectl apply -f k8s/hpa.yaml
```

#### 2. Helm Chart Deployment
```bash
# Install Helm chart
helm install distributed-ml helm/distributed-ml
```

### Scaling and Monitoring
- Horizontal Pod Autoscaler configured to scale based on CPU and memory utilization
- Automatically scales between 3-10 replicas
- Monitoring available through Kubernetes dashboard or `kubectl`

### Accessing the Dashboard
```bash
kubectl port-forward service/distributed-ml-service 8080:8080
```
Open `http://localhost:8080` in your browser

### Logging and Debugging
```bash
# View logs
kubectl logs -l app=distributed-ml
```

## License
MIT License
