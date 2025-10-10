## distributed ml framework

* [x] c++17 distributed training
* [x] real-time performance and task monitoring
* [x] web dashboard — `http://localhost:8080`
* [x] simple build: `cmake .. && make`
* [x] run: `mpirun -n <num_processes> ./distributed_ml_app`
* [x] kubernetes-ready (docker, helm, autoscaling)
* [x] ci/cd on github actions (macos m1)
* [x] apache 2.0 license

### example output

```
dashboard server listening on: http://localhost:8080/
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
