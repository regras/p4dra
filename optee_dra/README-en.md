# optee_dra
OPTEE application for the DRA protocol

# Container for OPTEE build

For ease of build, the `optee-container` folder has a Dockerfile for a container definition with the minimum requirements to build OPTEE and QEMU to the ARMv8 architecture. To build the container, run the `build.sh` script from inside the `optee-container` folder.

Make the first OPTEE build by accessing the container through the `run_optee_build.sh` script and run the command `make -j4 toolchains && make -j$(nproc)` inside the `optee/build` folder in the container.

# Adding libpcap and P4DRA service in OPTEE

To add the libpcap in OPTEE's build and the DRA service script, add the files from folder `python_qemuv8` in the corresponding folders related to the root folder of the build container (i.e, file `python_qemuv8/optee/build/common.mk` has to copied into `/optee/build/common.mk`). These files will add the files as a rootfs overlay and add the service initialization script into boot and the libpcap library inside OPTEE build.

# P4DRA build

Only follow these steps after having built OPTEE for the first time according to the previous section.

For building P4DRA tools, insert folders `dra` and `dra_service` into the `optee_examples` directory at OPTEE's root and run the `make` command inside the `build` folder at OPTEE's root inside the build container.

# Helper scripts and KEA (dhcpv4)

Considering a scenario running on Virtualbox or QEMU with SLIRP, the `configure_network.sh` script configures a bridge to run QEMU with OPTEE connected to it. The script `run_kea.sh` start a DHCPv4 server listening to the bridge for tests. It's important to verify if other services (like dnsmasq) are not using UDP ports 67 and 68. Use the command `ss -nulp` to visualize the processes listening to UDP ports.

# Start OP-TEE with QEMU inside the container

Before starting OPTEE, start the br0 bridge so that tap0 interfacee can be created by QEMU: `brctl addbr br0; ip link set dev br0 up; brctl setageing br0 0`.

The `run_optee.sh` script starts the build container with OPTEE and the needed packages running the `start_qemu.sh` script inside the container. This script assumes that you'll make the OPTEE build at `${HOME}/optee`, but that can be configured in the variables at the beggining of the script.

You can pass as an argument the ID of the instance that you wish to start, so you cna start more than one instance. i.e.: `./run_optee.sh 1` starts container `optee01` with MAC `52:54:00:00:00:01`, while `./run_optee.sh 10` starts container `optee10` with MAC `52:54:00:00:00:0a`. However, the tap interface ins enumerated by QEMU according to avaliability, so QEMU from container `optee01` will use interface `tap0` and container `optee10` will use interface `tap1` if no other instance is started.

For ease of instance management, the followin commands can be used and modified:

```sh
num_devices=100

##isolates OPTEEs##
#Local Server -> OPTEEs scenario
for i in $(seq 1 ${num_devices}); do ebtables -A FORWARD -i veth1 -o tap$((10#$i-1)) -d 52:54:00:00:00:$(printf "%02x\n" $i) -j ACCEPT; done
for i in $(seq 1 ${num_devices}); do ebtables -A FORWARD -i tap$((10#$i-1)) -o veth1 -s 52:54:00:00:00:$(printf "%02x\n" $i) -j ACCEPT; done
ebtables -A FORWARD -j DROP

#Local server -> Switch -> OPTEEs scenario
interface_switch="ens1f0np0" # modify according to your scenario
ebtables -A FORWARD -i veth1 -o ${interface_switch} -j ACCEPT
ebtables -A FORWARD -i ${interface_switch} -o veth1 -j ACCEPT
for i in $(seq 1 ${num_devices}); do ebtables -A FORWARD -i ${interface_switch} -o tap$((10#$i-1)) -d 52:54:00:00:00:$(printf "%02x\n" $i) -j ACCEPT; done
for i in $(seq 1 ${num_devices}); do ebtables -A FORWARD -o ${interface_switch} -i tap$((10#$i-1)) -s 52:54:00:00:00:$(printf "%02x\n" $i) -j ACCEPT; done
ebtables -A FORWARD -j DROP
##

#disable OPTEEs isolation
ebtables -F FORWARD
##

#Start containers - Run from 'optee_dra/script_auxiliares' folder.
for i in $(seq 1 ${num_devices}); do ./run_optee.sh $i; sleep 3; done

#tracks the loading - Run from another terminal
watch 'ss -ntlp4 | grep "127.0.0.1:54" | sed "s, * ,;,g" | cut -d";" -f4 | sort -n -t":" -k2 | tail -n1'

#Stop the containers
for i in $(seq 1 ${num_devices}); do docker rm -f optee$(printf "%02i" $i); done
```

# Connect to OPTEE instance

To connect to an OPTEE instance, run the command `telnet 127.0.0.1 543219` for Normal World and `telnet 127.0.0.1 54320` for Secure World. These values (54319 and 54320) references the optee01 container. The script `start_screens.sh` helps starting `screen`s with the telnet sessions (you must have the `screen` package installed at your system).

# Tests

To test if DRA is working inside optee, you can using the following commands as examples:

```sh
# writes on Secure Storage (oper 1) "key" "device_ID"
optee_dra 1 "3132333435363738393031323334353637383930313233343536373839303132" "31323334353637383930313233343536"

# reads from Secure Storage (oper 0)
optee_dra 0

# Request proof using written key and Device_id (oper 3) "round_id" "nonce"
optee_dra 3 "3132333435363738" "3132333435363738"
#it must return: ef38e2dfffe258d31856116a1bedcb35a6caea99f859ac964f853e8aefd7832f8116ab97b37ea2a49cd9c4ec3874e5654c31cc05e7f0760b721391033314d2e4
```