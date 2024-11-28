#!/usr/bin/env bash

if [ -z "${1}" ]; then
        id=1
else
        id="${1}"
fi

printf -v id "%02i" $id
printf -v idh "%02x" $id

cd /optee/build/../out/bin && \
        /optee/build/../qemu/build/aarch64-softmmu/qemu-system-aarch64 \
        -nographic \
        -smp 2 -cpu max,sme=on,pauth-impdef=on \
        -d unimp -semihosting-config enable=on,target=native \
        -m 545 -bios bl1.bin \
        -initrd rootfs.cpio.gz \
        -kernel Image -append 'console=ttyAMA0,38400 keep_bootcon root=/dev/vda2 ' \
        -object rng-random,filename=/dev/urandom,id=rng0 \
        -device virtio-rng-pci,rng=rng0,max-bytes=1024,period=1000 \
        -net nic,model=virtio,macaddr=52:54:00:00:00:${idh} \
        -net bridge,br=br0 \
        -machine virt,acpi=off,secure=on,mte=off,gic-version=3,virtualization=false \
        -serial telnet:127.0.0.1:$((54320+(${id}*2)-1)),server,nowait \
        -serial telnet:127.0.0.1:$((54320+(${id}*2))),server,nowait
