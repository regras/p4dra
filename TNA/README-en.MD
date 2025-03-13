# Setting the number of devices that can be attested:

To change the number of devices to be attested, edit these 2 files:

- bfrt_python/setup.py: at line `quantidade_dispositivos = 20000` (number of devices = 20000), change the number of devices;
    - This same files defines the switch port in which OPTEE is connected and the switch port in which the controller is connected.
    - The `file_path` variable defines where the file with `device_id:key:proof` is. These variables are defined in hexstrings without the "0x" prefix. As default, the device's expected configuration is defined as empty strings.
- p4src/common/includes/tbl_p4dra_ig.p4: at `size` lines of tables `tbl_p4dra_ig0` and `tbl_p4dra_ig0_fin`, set the number of devices + 2 at table `tbl_p4dra_ig0` and the exact number of devices at table `tbl_p4dra_ig0_fin`.
- By default, these files are set for 20.000 devices.

When running on a real Tofino, don't forget to configure switch ports with ucli and load the table registries with bfrt_python/setup.py.

# Guidelines for Tofino Model

attention to the following assumptions:
- The ICA scripts and tools are at ~/tools;
- bf_sde is at the user's home folder and version 9.9.0 is installed. If you're using a new bf-sde version, update it at `control.sh`'s line 3.

Use the `control.sh` script for these operations:

- ./control.sh p4_build: builds the p4 code
- ./control.sh topo_start: starts the topology (NetNS h1, 08:00:00:00:01:01@10.0.1.1, connected to tofino-model port 1, NetNS h2, 08:00:00:00:01:02@10.0.1.2 connected to tofino-model port 2)
- ./control.sh topo_stop: stops the topology (removes bf processes and the Net NameSpaces)
- ./control.sh shell [h1|h2]: opens host h1 or h2's shell (Bash)

Use the `receive.py` script in a host to receive the packets and script `send.py \<destination ip\> \<operation\> \<arguments\>` to send a packet with P4DRA's operations and its arguments.

For other scenarios, it's necessary to adapt the scripts to your topology.