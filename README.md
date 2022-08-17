## NS3-ARDUPILOT-SIMULATION


### Installation steps:
- To use it, you need to have a [NS3](https://www.nsnam.org/) simulator installed.
- Clone [this repo](https://github.com/furkan-guvenc/ns3-ardupilot-simulation) in scratch/ardupilot/
- Clone [mavlink library](https://github.com/mavlink/c_library_v2) in scratch/ardupilot/c_library_v2/

### [Build ns3](https://www.nsnam.org/docs/release/3.36/tutorial/html/getting-started.html)
```shell
CXXFLAGS="-W -Wall -g"  ./ns3 configure --build-profile=debug --enable-examples --enable-tests --enable-sudo 
```
```shell
./ns3 build
```

### Run
```shell
./ns3 run scratch/ardupilot/ardupilot
```

After runing the application it will print which ips and ports being listened. To see any result, mavlink messages need to be sent to these targets. Ardupilot SITL can be used with [this repo](https://github.com/furkan-guvenc/ardupilot-sitl-docker-v2).
