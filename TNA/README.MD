# English version

There's an english version of this README [available here](README-en.md)

# Configurando a quantidade de dispositivos que podem ser atestados:

Para alterar a quantidade de dispositivos, alterar estes 2 arquivos:
- bfrt_python/setup.py: na linha `quantidade_dispositivos = 20000`, alterar para a quantidade de dispositivo;
    - Esse mesmo arquivo define a porta do switch onde o OPTEE está conectado e a porta do switch onde o Controlador está conectado.
    - O `file_path` define onde esta o arquivo com as informações de `id_dispositivo:chave:prova` em formatos de hexstring sem "0x". Por padrão as configurações são definidas como string vazia.
- p4sc/common/includes/tbl_p4dra_ig.p4: nas linhas de `size` das tabelas `tbl_p4dra_ig0` e `tbl_p4dra_ig0_fin` colocar a quantidade de dispositivos + 2 na tabela `tbl_p4dra_ig0` e a quantidade exata de dispositivos na tabela `tbl_p4dra_ig0_fin`.
- Por padrão, esses arquivos estão configurados para 20.000 dispositivos.

Não esquecer de, ao rodar no ambiente Tofino real, fazer a configuração das portas do switch e carregar as regras do script bfrt_python/setup.y

# Orientações para uso no Tofino Model

Atenção às seguintes suposições:
- os scripts de ferramentas do ICA estao em ~/tools
- o bf_sde está no home do usuario e a versão 9.9.0 está instalada. Caso use uma versão diferente, atualize na linha 3 do control.sh

Usar o script control.sh para fazer as operações.

- ./control.sh p4_build: realiza o build do arquivo P4
- ./control.sh topo_start: inicia a topologia (NetNS h1, 08:00:00:00:01:01@10.0.1.1, conectado na porta 1 do tofino-model, NetNS h2, 08:00:00:00:01:02@10.0.1.2 conectado na porta 2 do tofino-model)
- ./control.sh topo_stop: para a topologia (remove processos do bf e os NetNS)
- ./control.sh shell [h1|h2]: abre o shell (Bash) do host h1 ou h2

Use os script "receive.py" em um host para receber os pacotes e o script "send.py \<ip destino\> \<operação\> \<parametros\>" para enviar um pacote com as operações do P4DRA e seus parâmetros.

Para outros cenários, é necessário adaptar os scripts à sua topologia.