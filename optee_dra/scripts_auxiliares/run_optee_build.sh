#!/usr/bin/env bash

if [ -z "${1}" ]; then
	id=1
else
	id="${1}"
fi

pasta_repositorio_optee_dra="${HOME}/p4dra/optee_dra" #pasta onde esta o clone do p4dra com a pasta optee_dra
pasta_scripts="${pasta_repositorio_optee_dra}/scripts_auxiliares" #pasta dos scripts auxiliares no optee_dra
pasta_optee="${HOME}/optee" #pasta para instalação do OP-TEE

printf -v id "%02i" $id

docker run -it --rm --net=host --cap-add=NET_ADMIN --name optee${id} \
        --device /dev/kvm --device /dev/net/tun \
	        -v "${pasta_scripts}/start_qemu.sh:/optee/start_qemu.sh:ro" \
        	-v "${pasta_repositorio_optee_dra}/bridge.conf:/optee/qemu/build/qemu-bundle/usr/local/etc/qemu/bridge.conf:ro" \
	        -v "${pasta_repositorio_optee_dra}/optee/:/optee/" \
	        -v "${pasta_optee}/:/optee/" \
		optee_build bash
