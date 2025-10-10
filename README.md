# Distributed Machine Learning Framework

## overview

distributed machine learning framework with real-time performance monitoring and task tracking.

## prerequisites

* c++17 compiler
* mpi (openmpi or mpich)
* opencv
* eigen3
* cpprestsdk
* nlohmann/json

## build

```bash
mkdir build && cd build
cmake ..
make
```

## run

```bash
mpirun -n <num_processes> ./distributed_ml_app
```

## dashboard

open `http://localhost:8080`

## features

* distributed training
* real-time task monitoring
* performance metrics tracking
* web dashboard

## conventional commits

project follows conventional commits for a clean git history.

### commit message rules

* first line: lowercase and ≤30 characters
* must start with a valid type followed by `:`
* valid types: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `chore`, `ci`, `build`, `revert`

example (compliant):

```
feat: add docker ci/e2e
```

### setup

```bash
cp scripts/commit-msg .git/hooks/commit-msg
chmod +x .git/hooks/commit-msg
```



## architecture

* distributed trainer
* task manager
* performance tracker
* dashboard server

## kubernetes

### docker image

```bash
docker build -t distributed-ml-app:latest .
```

### deployment options

#### direct

```bash
kubectl apply -f k8s/deployment.yaml
kubectl apply -f k8s/hpa.yaml
```

#### helm

```bash
helm install distributed-ml helm/distributed-ml
```

### scaling & monitoring

* horizontal pod autoscaler (cpu/memory)
* auto scale 3–10 replicas
* monitoring via k8s dashboard or `kubectl`

### access dashboard

```bash
kubectl port-forward service/distributed-ml-service 8080:8080
# then open http://localhost:8080
```

### logs

```bash
kubectl logs -l app=distributed-ml
```

## planned improvements

* better error handling
* more unit tests
* performance optimizations

## license

apache 2.0
