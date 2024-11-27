#!/usr/bin/env bash

pasta_repositorio_optee_dra="${HOME}/p4dra/optee_dra" #pasta onde esta o clone do p4dra com a pasta optee_dra

#Tofino model utiliza a porta 8000, a mesma que o agente de controle do Kea, entao trocamos para o KEA utilizar a porta 7999, mas não utilizamos o agente.

docker run -d --restart="always" \
	--privileged \
	--name kea-dhcp4 \
	--network host \
	--volume "${pasta_repositorio_optee_dra}/kea-dhcp4.json:/etc/kea/kea-dhcp4.conf" \
	--volume "${pasta_repositorio_optee_dra}/kea-ctrl-agent.conf:/etc/kea/kea-ctrl-agent.conf" \
	docker.cloudsmith.io/isc/docker/kea-dhcp4
