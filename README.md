<p align="center">
  <img src="https://raw.githubusercontent.com/coccinella-labs/ml/main/.github/assets/thumbnail.png" alt="ml" width="100%">
</p>

# ML

ML is an MPI coordination scaffold written in C++17. It distributes data across processes, aggregates gradients with `MPI_Allreduce`, broadcasts parameters, and serves training metrics through a REST dashboard. Deployment scaffolding exists for Kubernetes and Docker, and CI runs on GitHub Actions.

**The training math is not implemented.** What is real is the distributed coordination and the monitoring surface. What is simulated is the learning: the training data is randomly generated, and per-batch "gradients" are `i * learningRate` rather than the output of a forward and backward pass. See [Known Limitations](#known-limitations).

Status: experimental. Coordination and monitoring are functional; no model is trained.

## Getting Started

ML requires C++17, CMake 3.10+, and an MPI implementation (OpenMPI or MPICH). On macOS with Homebrew, install MPI and dependencies with `brew install open-mpi`. On Linux, use your package manager to install `libopenmpi-dev` or `mpich-dev`.

To build locally, run `cmake -B build -S .` to configure the project, then `cmake --build build` to compile. The binary is at `build/distributed_ml`. To run a single-process training session, execute `./build/distributed_ml`. To run distributed training across multiple processes, use `mpirun -n 4 ./build/distributed_ml` to spawn four training processes that synchronize with each other. The first process will launch a web dashboard on `http://localhost:8080` showing real-time training metrics and task status.

For Docker-based deployment, build the image with `docker build -t distributed-ml:latest .` (this takes several minutes) and run it with `docker run -p 8080:8080 distributed-ml:latest`. The dashboard will be accessible at `http://localhost:8080`. The container runs training and keeps the dashboard server running indefinitely.

To test locally, run the example with `mpirun -np 1 ./build/distributed_ml` and watch the console output for training progress. Early stopping will trigger when convergence is detected, and training metrics will be printed in JSON format at the end.

## Architecture

ML has three main components. The distributed trainer at `src/distributed_trainer.cpp` manages MPI process synchronization, parameter broadcasting, and gradient aggregation. It partitions the dataset across processes by rank and keeps parameters synchronized by broadcasting from rank 0. There is no model file and no model. The web dashboard server at `src/dashboard_server.cpp` exposes REST endpoints for querying task status and performance metrics.

The training loop runs across all processes simultaneously, and this is where the simulation boundary sits. Each process takes its slice of the dataset, walks it in batches, and for each batch calls `processLocalBatch`, which returns `i * learningRate` for sample `i` rather than a computed gradient. The source marks this explicitly, listing the forward pass, loss computation, and backward pass as the work a real implementation would add. Those gradient vectors are then genuinely reduced across ranks with `MPI_Allreduce` and normalized by world size, and loss is averaged the same way. So the collective operations are real and the values flowing through them are synthetic. `updateModelParameters` logs the loss and gradient norm and returns without touching any weights.

The dashboard server runs on rank 0 and serves metrics from every process through a web interface and REST API.

Key code anchors are `src/distributed_trainer.cpp` (MPI synchronization and distributed logic, including the `TrainingConfig` struct), `src/dashboard_server.cpp` (REST API and web server), `include/` (headers), and `deploy/` (Kubernetes manifests and Helm charts).

## Configuration

Training is configured through the `TrainingConfig` struct in `include/distributed_trainer.h` (learning rate default 0.01, epoch and batch counts clamped to at least 1). There is no config header file and no CMake `ENABLE_MPI` or `ENABLE_DASHBOARD` options, and there are no command-line flags: the values come from a single hardcoded call to `validateAndSetConfig({0.01, 100, 32})` in `src/distributed_trainer.cpp`. The deployment manifests set `MPI_NODES` and `LOG_LEVEL`, but no source file calls `getenv`, so neither variable currently reaches the program.

Those three values are the only ones, and they are fixed in source: learning rate 0.01, 100 epochs, batch size 32. Early stopping triggers when the loss plateaus, using a patience of 3 declared as a function-local `static int` in `src/distributed_trainer.cpp`. It is not part of `TrainingConfig` and cannot be changed without editing the source.

## API

The web dashboard server exposes a REST API on port 8080. `GET /` returns API information and available endpoints. `GET /tasks` lists all current tasks being trained, including their rank (which process), world size (total processes), and status. `GET /performance` returns aggregated performance metrics including average loss, gradient norms, and throughput. `POST /tasks` creates a new training task with specified hyperparameters. All endpoints return JSON responses.

The dashboard also provides a web interface at `http://localhost:8080` showing real-time training curves, loss progression across epochs, and per-process metrics. The interface updates periodically to reflect training progress.

## Contributing

Fork the repository, create a feature branch, make changes to `src/` or `include/`, add tests to `tests/`, and open a PR. Build and test with `cmake -B build -S . && cmake --build build && ctest --test-dir build`; note that `unit_tests` covers the trackers only, so a green run says nothing about `distributed_trainer.cpp`. Tests for the coordinator are wanted. Code standards: use C++17 features and idioms, keep MPI communication in `distributed_trainer.cpp`, and document any new configuration parameters in `include/distributed_trainer.h`.

When adding a new training algorithm, implement it in `src/distributed_trainer.cpp` or create a new model file. When adding monitoring capabilities, extend the dashboard server in `src/dashboard_server.cpp`. When adding distributed coordination logic, extend `distributed_trainer.cpp` with clear comments explaining the MPI communication pattern.

## Build and Deploy

Local development uses CMake as described in Getting Started. Build a release binary with `cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build`. The binary scales to any number of processes via `mpirun`. For Docker, the Dockerfile builds the project inside a container and exposes port 8080 for the dashboard.

For Kubernetes deployment, a Helm chart is provided in `deploy/helm/distributed-ml/`. Install it with `helm install ml-training deploy/helm/distributed-ml --set replicaCount=4`. The chart sets `replicaCount`, a Service, and an HPA; it does not configure an MPI launcher, so `mpirun` has to come from elsewhere. Kubernetes manifests in `deploy/k8s/` provide a lower-level alternative without Helm templating.

CI/CD runs on GitHub Actions with builds tested on macOS M1 and Linux runners. Pull requests trigger automatic testing; merges to main trigger the full matrix and artifact uploads. No release artifacts are published from this workflow.

## Known Limitations

**No training is performed.** The gradient vectors are synthetic and the parameter update is a no-op, so no accuracy, convergence, or model-quality claim can be made from a run. Benchmark the coordination layer, not learning.

The training data is generated with `setRandom()` in `src/main.cpp`, so there is no dataset loading, no file format, and no I/O to profile. Data loading is synchronous by construction.

MPI coordination requires either local processes on a single machine via `mpirun`, or an HPC cluster with MPI enabled. Kubernetes deployment requires additional MPI configuration or a custom network overlay; the Helm chart does not configure a launcher. GPU acceleration is not implemented.

Communication is all-reduce for gradient and loss aggregation plus a broadcast from rank 0. All-reduce scales well to tens of processes and becomes bandwidth-limited at hundreds, but this has not been measured here. Fault tolerance is not built in: if a process crashes, training must restart.

The dashboard runs only on rank 0; if that process dies, monitoring stops while the remaining processes continue.

Error handling is marked `TODO` in `src/distributed_trainer.cpp` and `src/main.cpp`, and data distribution is marked as needing more efficient handling.

**The coordinator has no test coverage.** `tests/` covers `performance_tracker` and `task_manager` only; nothing imports `distributed_trainer.cpp` or `main.cpp`. The MPI synchronisation, the parameter broadcast, and the all-reduce are therefore unverified by the test suite, which builds and runs `unit_tests` but never exercises the part of the codebase this project exists for. A passing `ctest` here means the trackers work, not that the distributed layer does.

## Performance

No throughput or scaling figures are recorded. Earlier versions of this document quoted batches per second, a 3-8x speedup at four processes, and 5-20% all-reduce overhead; those numbers were not measured and have been removed.

What can be stated: the work being timed is a synthetic gradient computation, so any throughput figure would describe the cost of the collective operations and nothing about training. Measuring this layer is still worthwhile for validating that all-reduce behaves as expected at a given rank count.

Early stopping exists and triggers when the loss stops improving, using a patience counter of 3 in `src/distributed_trainer.cpp`. Since the loss it watches is `localGradient.norm()` on synthetic vectors, its effect on a real training run is untested, and the previously quoted 30-50% saving has been removed as unmeasured.

## Roadmap

The substantive item is implementing the training math: a forward pass, a loss function, a backward pass producing real gradients, and a parameter update that actually applies them. That is what would turn this from a coordination scaffold into a training framework, and nothing else on this list matters as much until it exists.

After that: dataset loading to replace `setRandom()`, checkpointing and resume, GPU acceleration, asynchronous gradient accumulation, gradient compression for slow networks, and mixed-precision training.

## Related Documentation

CMakeLists.txt documents build options and dependencies. The Dockerfile provides a reproducible build and container environment. The `deploy/` tree holds Kubernetes manifests and a Helm chart, which cover scheduling and services but not MPI process launch. See the main gpucomm documentation for where this fits in the wider compute environment.

## License

Apache 2.0. See LICENSE file.

## Contact

Questions? Open an issue on GitHub or see the repository for discussion.
