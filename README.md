# mpidb
Easily debug MPI applications using familiar tools such as *gdb* or your favorite IDE.

## Examples

```$ git clone https://github.com/manie204/mpidb.git
$ mpirun -n 8 -- mpidb mpi_app
```
Starts *mpi_app* using 8 MPI ranks with remote debuggers attached and writes the information required for debugging to the console.

```$ git clone https://github.com/manie204/mpidb.git
$ mpirun -n 8 -- mpidb -r 0-2,5 mpi_app
```
Same, but only debugs ranks 0, 1, 2 and 5.

```$ git clone https://github.com/manie204/mpidb.git
$ mpirun -n 8 -- mpidb mpi_app | mpidbc vscode launch.json
```
Same as the first example, but the output required for debugging is formatted by the helper tool *mpidbc*.
The resulting *launch.json* file can be used to start interactive debugging sessions in [Visual Studio Code](https://github.com/Microsoft/vscode).

## Requirements

* A c++11-capable compiler

* gdbserver
  * Required on all nodes/hosts where MPI ranks need to be debugged.
  * In case the remote environment does not provide a gdbserver, the path to a gdbserver executable can be supplied by specifying ```-g``` on the command-line.

* cmake v3.10+ **(recommended)**
  * Required to create the build environment.
  
## Installing

### Using cmake **(recommended)**

Here is an example that uses a build pipeline for GNU make on Ubuntu starting from the *mpidb* root directory:
```shell
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_BUILD_TYPE=RelWithDebInfo
$ make
```

The built executable can be found in ```build/bin/```.

### Manual compilation

Currently, the code base is very small, so manual compilation is straight forward.
Here is an example using *g++*:
```shell
$ mkdir build
$ cd src
$ g++ -std=c++11 -O3 -g Options.cpp mpidb.cpp -o ../build/mpidb
$ cd ../build
```
