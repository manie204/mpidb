# mpidb
Easily debug MPI applications using familiar tools such as *gdb* or an IDE (currently, only [Visual Studio Code](https://github.com/Microsoft/vscode) is supported. Feel free to leave requests/suggestions for other IDEs or debugging tools).

## Examples

```$ git clone https://github.com/manie204/mpidb.git
$ mpirun -n 8 -- mpidb mpi_app
```
Starts *mpi_app* using 8 MPI ranks with remote debuggers attached and writes the information required for debugging to the terminal.

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

* gdbserver
  * Required on all nodes/hosts where MPI ranks need to be debugged.
  * In case the remote environment does not provide a gdbserver, the path to a gdbserver executable can be supplied by specifying ```-g``` on the command-line.

* Python >= 3.6
  * For running the helper tools.

* A c++11-capable compiler **(for building)**

* cmake >= 3.10 **(optional)**
  * Used to create the build environment.
  
## Installing

### Using cmake **(recommended)**

Here is an example that uses a build pipeline for *GNU make* on Ubuntu starting from the *mpidb* root directory:
```shell
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_BUILD_TYPE=RelWithDebInfo
$ make
```

The built executable can be found in ```build/bin/```.

### Manual compilation

Currently, the code base is so small that manual compilation is straight forward.
Here is an example using *g++*:
```shell
$ mkdir build
$ cd src
$ g++ -std=c++11 -O3 -g Options.cpp mpidb.cpp -o ../build/mpidb
$ cd ../build
```

## Notes

* *mpidb* does **not** debug the MPI layer. If there are bugs related to sending or receiving MPI messages of any kind, this tool might not be able to resolve those issues.
* Depending on your MPI library, the command-line argument ```--``` may or may not be required to seperate the options for *mpirun* from the ones for *mpidb*.
* Even though the MPI scheduler should terminate every child process cleanly, there can sometimes be zombie processes left alive. This may rarely happen if an internal error is encountered in either *mpidb* or *mpidbc*. I therefore suggest searching and killing any lingering processes (i.e. *gdbserver*, *mpidb*, etc.) on the affected nodes after such an error occurs.
* Remote-debugging communication happens over a certain range of TCP ports (that can be specified via ```-p``` command-line option). Make sure those ports are available and open between the remote and host machines.
