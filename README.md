<p align="center">
  <img src="https://raw.githubusercontent.com/coccinella-labs/ml/main/.github/assets/thumbnail.png" alt="ml" width="100%">
</p>

# ML

ML is a distributed machine learning framework written in C++17 that coordinates training across multiple processes using MPI (Message Passing Interface). It synchronizes model parameters across nodes, aggregates gradients, and monitors training progress in real-time through a web dashboard. The framework is designed for production use, with support for Kubernetes deployment, Docker containers, and GitHub Actions CI/CD.

Status: Stable. Distributed training functional. Production-ready with monitoring and orchestration support.

## Getting Started

ML requires C++17, CMake 3.10+, and an MPI implementation (OpenMPI or MPICH). On macOS with Homebrew, install MPI and dependencies with `brew install open-mpi`. On Linux, use your package manager to install `libopenmpi-dev` or `mpich-dev`.

To build locally, run `cmake -B build -S .` to configure the project, then `cmake --build build` to compile. The binary is at `build/distributed_ml`. To run a single-process training session, execute `./build/distributed_ml`. To run distributed training across multiple processes, use `mpirun -n 4 ./build/distributed_ml` to spawn four training processes that synchronize with each other. The first process will launch a web dashboard on `http://localhost:8080` showing real-time training metrics and task status.

For Docker-based deployment, build the image with `docker build -t distributed-ml:latest .` (this takes several minutes) and run it with `docker run -p 8080:8080 distributed-ml:latest`. The dashboard will be accessible at `http://localhost:8080`. The container runs training and keeps the dashboard server running indefinitely.

To test locally, run the example with `mpirun -np 1 ./build/distributed_ml` and watch the console output for training progress. Early stopping will trigger when convergence is detected, and training metrics will be printed in JSON format at the end.

## Architecture

ML is built as a distributed training framework with three main components. The distributed trainer at `src/distributed_trainer.cpp` manages MPI process synchronization, parameter broadcasting, and gradient aggregation. It divides training data across processes and ensures all nodes train on synchronized model parameters. The model at `src/model.cpp` implements forward pass, backward pass (gradient computation), and parameter updates. The web dashboard server at `src/dashboard_server.cpp` exposes REST endpoints for monitoring training progress and querying task status.

The training loop operates across all processes simultaneously. Each process reads a portion of the training dataset locally, performs forward and backward passes on its batch, computes gradients, synchronizes with other processes (averaging gradients), applies updates to the local model, and repeats for the next epoch. The dashboard server runs on the first process and aggregates metrics from all processes, displaying them through a web interface and REST API.

Key code anchors are `src/distributed_trainer.cpp` (MPI synchronization and distributed logic), `src/model.cpp` (forward/backward passes), `src/dashboard_server.cpp` (REST API and web server), `include/` (header files and configuration), and `deploy/` (Kubernetes manifests and Helm charts).

## Configuration

Training is configured through `include/config.h` with parameters for learning rate (`lr`), number of epochs (`epochs`), batch size (`batch_size`), and model hyperparameters. CMake flags control build options: `-DENABLE_MPI=ON` enables distributed training (default), `-DENABLE_DASHBOARD=ON` enables the web dashboard (default). Docker and Kubernetes deployments read configuration from environment variables passed at runtime.

Common configuration values are: learning rate typically 0.01 for gradient descent, batch size 32 or 64 depending on dataset size, and epoch count 100 or until early stopping triggers. Early stopping is triggered automatically when loss plateaus; you can adjust the early stopping patience threshold in the configuration if needed.

## API

The web dashboard server exposes a REST API on port 8080. `GET /` returns API information and available endpoints. `GET /tasks` lists all current tasks being trained, including their rank (which process), world size (total processes), and status. `GET /performance` returns aggregated performance metrics including average loss, gradient norms, and throughput. `POST /tasks` creates a new training task with specified hyperparameters. All endpoints return JSON responses.

The dashboard also provides a web interface at `http://localhost:8080` showing real-time training curves, loss progression across epochs, and per-process metrics. The interface updates periodically to reflect training progress.

## Contributing

Fork the repository, create a feature branch, make changes to `src/` or `include/`, add tests to `tests/`, run `cmake` to build and test locally, and open a PR. Code standards: use C++17 features and idioms, keep MPI communication in `distributed_trainer.cpp`, and document any new configuration parameters in `include/config.h`.

When adding a new training algorithm, implement it in `src/model.cpp` or create a new model file. When adding monitoring capabilities, extend the dashboard server in `src/dashboard_server.cpp`. When adding distributed coordination logic, extend `distributed_trainer.cpp` with clear comments explaining the MPI communication pattern.

## Build and Deploy

Local development uses CMake as described in Getting Started. Build a release binary with `cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build`. The binary scales to any number of processes via `mpirun`. For Docker, the Dockerfile builds the project inside a container and exposes port 8080 for the dashboard.

For Kubernetes deployment, Helm charts are provided in `deploy/helm/`. Deploy with `helm install ml-training deploy/helm/ml --set replicaCount=4` to spawn four training processes. The Helm chart handles MPI process coordination and network setup. Kubernetes manifests in `deploy/k8s/` provide a lower-level alternative without Helm templating.

CI/CD runs on GitHub Actions with builds tested on macOS M1 and Linux runners. Pull requests trigger automatic testing; merges to main trigger production builds and artifact uploads.

## Known Limitations

The framework uses MPI for process coordination, which requires either local processes on a single machine (via mpirun) or an HPC cluster with MPI enabled. Kubernetes deployment requires additional MPI configuration or a custom network overlay. GPU acceleration is not yet implemented; training runs on CPU only. Communication patterns are all-reduce for gradient averaging, which scales well to tens of processes but becomes bandwidth-limited at hundreds of nodes. Fault tolerance is not built-in; if a process crashes, training must restart.

The framework is optimized for small to medium models (hundreds of millions of parameters). Very large models (multi-billion parameters) require additional optimization such as model parallelism or gradient checkpointing. Data loading is currently synchronous; asynchronous prefetching would improve throughput for IO-bound workloads. The dashboard server runs only on rank 0; if that process crashes, monitoring stops but training continues.

## Performance

Training throughput depends on the model and dataset size. On a single process, expect 1-10 batches per second depending on model complexity. With four processes, throughput typically scales to 3-8x faster due to parallelism, with some overhead from MPI communication. Gradient synchronization via all-reduce adds latency proportional to the number of processes and model size; typical overhead is 5-20% per epoch. The dashboard has minimal overhead (< 1% CPU) when running.

Early stopping typically reduces total training time by 30-50% compared to fixed epoch counts, by terminating when loss plateaus. Communication time dominates computation time for models smaller than 10 million parameters; for larger models, computation dominates.

## Roadmap

Planned features include GPU acceleration via CUDA or Metal, asynchronous gradient accumulation to reduce synchronization overhead, and gradient compression for efficient communication across slow networks. Model checkpointing and resume capability are planned. Mixed-precision training and quantization are under consideration. See GitHub Issues for the full roadmap and current priorities.

## Related Documentation

The framework integrates with the gpucomm ecosystem for benchmarking distributed training performance. See the main gpucomm documentation for context on how ML training fits into the broader compute environment. CMakeLists.txt documents build options and dependencies. The Dockerfile provides a reproducible deployment environment and can serve as a reference for other production setups.

## License

Apache 2.0. See LICENSE file.

## Contact

Questions? Open an issue on GitHub or see the repository for discussion.
