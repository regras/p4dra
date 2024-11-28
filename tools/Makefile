BUILD_DIR=build
SRC_DIR=src
CC=gcc
LPCAP=-lpcap

all: le_provas_rede le_provas_arquivo envia_provas_arquivo envia_provas envia_configuracoes_arquivo envia_pedidos envia_pacotes_pcap

le_provas_rede:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@ $(LPCAP)

le_provas_arquivo:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@

le_p4dra_rede:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@ $(LPCAP)

le_p4dra_rede_optee:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@ $(LPCAP)

verifica_p4dra_rede:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@ $(LPCAP)

envia_provas_arquivo:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@

envia_configuracoes_arquivo:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@


envia_setup_host_arquivo:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@

envia_pedidos:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@

envia_pedidos_host:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@

envia_provas:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@

envia_pacotes_pcap:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(SRC_DIR)/$@.c -o $(BUILD_DIR)/$@  $(LPCAP)

clean:
	@rm -r $(BUILD_DIR)
