# optee_dra
Aplicação para o OPTEE para o protocolo DRA

# Container para compilação do OPTEE

Para facilitar a compilação, a pasta `optee-container` contém um Dockerfile de um container com os requisitos mínimos para compilar o OPTEE para QEMU com arquitetura ARMv8. Para construir o container, basta rodar o script `build.sh` de dentro da pasta.

Faça a primeira compilação do OPTEE acessando o container pelo script `run_optee_build.sh` e executando o comando `make -j4 toolchains && make -j$(nproc)` dentro da pasta `optee/build`

# Adicionando o libpcap e serviço do P4DRA no OPTEE

Para adicionar o libpcap na compilação do OPTEE e o script de serviço do DRA, adicione os arquivos da pasta "python_qemuv8" nas pastas correspondentes em relação à raiz do container de compilação (ex, arquivo `python_qemuv8/optee/build/common.mk` deve ser copiado em `/optee/build/common.mk`). Esses arquivos irão adicionar os arquivos como um overlay do rootfs e adicionar um script de inicialização do serviço no boot e a biblioteca libpcap na compilação do OPTEE.

# Compilação do P4DRA
Apenas faça esse passo após ter feito a primeira compilação do OPTEE conforme seção anterior.

Para fazer a compilação das ferramentas do P4DRA, coloque as pastas `dra` e `dra_service` no diretorio `optee_examples` na raiz do diretorio do OPTEE e rode o comando `make` dentro da pasta `build` da raiz do OPTEE no container de compilação.

# Scripts auxiliares e KEA (dhcpv4)

Pensando em um cenário rodando em um VirtualBox ou QEMU com SLIRP, o script `configure_network.sh` configura uma Bridge para rodar o QEMU com o OPTEE conectado a essa. O script `run_kea.sh` pode ser utilizado para subir um DHCPv4 na interface br0 rapidamente para testes. É importante verificar se outros serviços (como o dnsmasq) não está utilizando as portas UDP 67 e 68. Para isso, utilize o comando `ss -nulp` para visualizar as portas UDP em Listen por processos.

# Iniciando o OP-TEE com QEMU dentro do container

Antes de iniciar o OPTEE, inicie a br0 para que a interface tap0 possa ser criada pelo QEMU: `brctl addbr br0; ip link set dev br0 up; brctl setageing br0 0`.

O script `run_optee.sh` inicia o container de build do OPTEE com os pacotes necessários e o script `start_qemu.sh` é executado dentro do container de build do OPTEE para iniciar o QEMU com o OPTEE compilado. Esse script assume que você irá fazer a build do optee em `${HOME}/optee`, mas pode ser configurado nas variáveis no começo do script.

Você pode passar como parâmetro o "ID" da instância que deseja iniciar para que inicie mais de uma instância. Ex: `./run_optee.sh 1` inicia o container `optee01` com MAC `52:54:00:00:00:01`, já `./run_optee.sh 10` inicia o container `optee10` com MAC `52:54:00:00:00:0a`. No entanto, a interface tap é enumerada pelo QEMU de acordo com a disponibilidade, então o QEMU do `optee01` usará a interface `tap0` e o `optee10` usará a interface `tap1`.

Para facilitar a gerência de várias instâncias, os seguinte comandos podem ser aproveitados e modificados:

```sh
quantidade_dispositivos=100

##isola OPTEEs##
#Cenário Servidor Local -> OPTEEs
for i in $(seq 1 ${quantidade_dispositivos}); do ebtables -A FORWARD -i veth1 -o tap$((10#$i-1)) -d 52:54:00:00:00:$(printf "%02x\n" $i) -j ACCEPT; done
for i in $(seq 1 ${quantidade_dispositivos}); do ebtables -A FORWARD -i tap$((10#$i-1)) -o veth1 -s 52:54:00:00:00:$(printf "%02x\n" $i) -j ACCEPT; done
ebtables -A FORWARD -j DROP

#Cenário Servidor Local -> Switch -> OPTEEs
interface_switch="ens1f0np0" # modificar para seu cenário
ebtables -A FORWARD -i veth1 -o ${interface_switch} -j ACCEPT
ebtables -A FORWARD -i ${interface_switch} -o veth1 -j ACCEPT
for i in $(seq 1 ${quantidade_dispositivos}); do ebtables -A FORWARD -i ${interface_switch} -o tap$((10#$i-1)) -d 52:54:00:00:00:$(printf "%02x\n" $i) -j ACCEPT; done
for i in $(seq 1 ${quantidade_dispositivos}); do ebtables -A FORWARD -o ${interface_switch} -i tap$((10#$i-1)) -s 52:54:00:00:00:$(printf "%02x\n" $i) -j ACCEPT; done
ebtables -A FORWARD -j DROP
##

#Desativar isolamento OPTEEs
ebtables -F FORWARD
##

#inicia containers - Rodar da pasta scripts_auxiliares do optee_dra
for i in $(seq 1 ${quantidade_dispositivos}); do ./run_optee.sh $i; sleep 3; done

#acompanha carregamento - Rodar de outro terminal
watch 'ss -ntlp4 | grep "127.0.0.1:54" | sed "s, * ,;,g" | cut -d";" -f4 | sort -n -t":" -k2 | tail -n1'

#pára containers
for i in $(seq 1 ${quantidade_dispositivos}); do docker rm -f optee$(printf "%02i" $i); done
```

# Conectando à instância do OPTEE

Para conectar à instância do QEMU com OPTEE, utilize o comando `telnet 127.0.0.1 54319` para o Mundo Normal e `telnet 127.0.0.1 54320` para o mundo seguro. Esses valores "54319" e "54320" são referentes à instância 1 do OPTEE. O script `start_screens.sh` auxilia iniciando `screens` com os telnet conectados. (necessário o pacote *screen* instalado)

# Testes

Para testar o funcionamento do DRA dentro do optee, utilize os seguinte comandos de exemplo:

```sh
# grava no storage seguro (oper 1) "chave" "id_dispositivo"
optee_dra 1 "3132333435363738393031323334353637383930313233343536373839303132" "31323334353637383930313233343536"

# recupera do storage seguro (oper 0)
optee_dra 0

# Solicita o calculo da prova usando a chave e id_dispositivo agravados (oper 3) "id_rodada" "nonce"
optee_dra 3 "3132333435363738" "3132333435363738"
#Ele deve retornar: ef38e2dfffe258d31856116a1bedcb35a6caea99f859ac964f853e8aefd7832f8116ab97b37ea2a49cd9c4ec3874e5654c31cc05e7f0760b721391033314d2e4
```