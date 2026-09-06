<p align="center">
  <img src="https://raw.githubusercontent.com/Coccinella-Labs/ml/main/.github/assets/thumbnail.png" alt="ml" width="100%">
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/coccinella-labs/ml/main/.github/assets/thumbnail.png" alt="ml" width="100%">
</p>

## distributed ml framework

* [x] c++17 distributed training
* [x] real-time performance and task monitoring
* [x] web dashboard — `http://localhost:8080`
* [x] simple build: `cmake -B build -S . && cmake --build build`
* [x] run: `mpirun -n <num_processes> ./distributed_ml`
* [x] kubernetes-ready (docker, helm, autoscaling)
* [x] ci/cd on github actions (macos m1)
* [x] apache 2.0 license

### docker

build the docker image — this may take several minutes:

```bash
docker build -t distributed-ml:latest .
```

run the container (exposes dashboard on port 8080):

```bash
docker run -p 8080:8080 distributed-ml:latest
```

the container will run training then keep the dashboard server running indefinitely.

### api endpoints

- `GET /` - api info and available endpoints
- `GET /tasks` - list all tasks
- `GET /performance` - performance metrics
- `POST /tasks` - create a new task

### example (local run)

```
$ cd build && mpirun -np 1 ./distributed_ml
[info] mpi initialized successfully
[info] configuration set: lr=0.01, epochs=100, batchsize=32
[info] distributed trainer initialized. rank: 0, world size: 1
[info] node 0 received 1000 training samples
[info] model parameters synchronized
[info] epoch 1/100 - global loss: 31.7566, gradient norm: 1.02059
[info] epoch 2/100 - global loss: 31.7566, gradient norm: 1.02059
[info] epoch 3/100 - global loss: 31.7566, gradient norm: 1.02059
[info] epoch 4/100 - global loss: 31.7566, gradient norm: 1.02059
[info] early stopping triggered
[info] distributed training completed
dashboard server listening on: http://0.0.0.0:8080/
[info] results aggregated from node 0
training metrics: {
    "batch_size": 32,
    "epochs": 100,
    "learning_rate": 0.01,
    "local_data_size": 1000,
    "rank": 0,
    "total_data_size": 1000,
    "world_size": 1
}
```