#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <linux/if_packet.h>
#include <unistd.h>
#include <net/if.h>

#define ETHERTYPE_CUSTOM 0x1234  // Define o Ethertype do seu protocolo
#define ID_DISPOSITIVO_LEN 32 // 128 bits = 16 bytes -> 32 caracteres hexadecimais
#define CHAVE_LEN 64  // 256 bits = 32 bytes -> 64 caracteres hexadecimais
#define PROVA_LEN 128 // 512 bits = 64 bytes -> 128 caracteres hexadecimais
#define NONCE_LEN 16 // 64 bits = 8 bytes -> 16 caracteres hexadecimais
#define ID_RODADA_LEN 32 // 128 bits = 16 bytes -> 32 caracteres hexadecimais
#define ID_DISPOSITIVO_LEN_BYTES 16
#define CHAVE_LEN_BYTES 32
#define PROVA_LEN_BYTES 64
#define NONCE_LEN_BYTES 8
#define ID_RODADA_LEN_BYTES 16
#define ID_MAX 1000000
uint32_t quantidade_provas = 1000;

// Estrutura para carregar arquivo de ids/chaves/provas na memoria
struct id_chave_prova_t {
    char id_dispositivo[ID_DISPOSITIVO_LEN];
    char chave[CHAVE_LEN];
    char prova[PROVA_LEN];
};

struct id_chave_prova_t dados[ID_MAX];

// Função para converter uma hexstring para um array de uint8_t
int hexstring_to_byte_array(uint8_t *byte_array, const char *hexstring, size_t len) {
    // Calcular o tamanho do array de bytes
    size_t byte_array_len = len / 2;

    // Converter cada par de caracteres hexadecimais para um byte
    for (size_t i = 0; i < byte_array_len; i++) {
        char byte_str[3] = { hexstring[2 * i], hexstring[2 * i + 1], '\0' };
        byte_array[i] = (uint8_t)strtoul(byte_str, NULL, 16);
    }

    return 0;
}

int carregarArquivoMemoria(char *arquivo) {
    FILE *fp = fopen(arquivo, "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    char *linha = NULL;
    size_t len = 0;

    uint32_t id = 0;

    while (getline(&linha, &len, fp) != -1) {
        // Dividir a linha com sscanf
        sscanf(linha, "%32c:%64c:%128c:", dados[id].id_dispositivo, dados[id].chave, dados[id].prova);
        id = id+1;
    }

    fclose(fp);
};

void swapEndian(char *saida, size_t tamanho) {
    char temp[tamanho];
    for (size_t i = 0; i < tamanho; i++) {
        temp[i] = saida[i];
    }
    for (size_t i = 0; i < tamanho; i += 8) {
        saida[i + 0] = temp[i + 6];
        saida[i + 1] = temp[i + 7];
        saida[i + 2] = temp[i + 4];
        saida[i + 3] = temp[i + 5];
        saida[i + 4] = temp[i + 2];
        saida[i + 5] = temp[i + 3];
        saida[i + 6] = temp[i + 0];
        saida[i + 7] = temp[i + 1];
    }
}

// Estrutura para o cabeçalho do protocolo personalizado
struct custom_header {
    uint8_t oper;
    uint8_t id_rodada[ID_RODADA_LEN_BYTES];
    uint8_t id_dispositivo[ID_DISPOSITIVO_LEN_BYTES];
    uint8_t prova[PROVA_LEN_BYTES];
};

void create_custom_packet(uint8_t *packet, const uint8_t *dest_mac, const uint8_t *src_mac, struct custom_header *custom_hdr) {
    struct ether_header *eth_header = (struct ether_header *) packet;

    // Configura o cabeçalho Ethernet
    memcpy(eth_header->ether_dhost, dest_mac, 6);
    memcpy(eth_header->ether_shost, src_mac, 6);
    eth_header->ether_type = htons(ETHERTYPE_CUSTOM);

    // Adiciona o cabeçalho personalizado após o cabeçalho Ethernet
    memcpy(packet + sizeof(struct ether_header), custom_hdr, sizeof(struct custom_header));
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <arquivo_provas> <interface> <id_rodada> [quantidade provas]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 5) {
        quantidade_provas = (uint32_t)strtoul(argv[4], NULL, 10);
    }

    if (quantidade_provas > ID_MAX) {
        quantidade_provas = ID_MAX;
    }

    char *arquivo_provas = argv[1];
    printf("carregando na memoria... \n");
    carregarArquivoMemoria(arquivo_provas);
    printf("carregado!\n");

    char *iface = argv[2];

    int sockfd;
    uint8_t packet[sizeof(struct ether_header) + sizeof(struct custom_header)];
    struct sockaddr_ll sa;
    struct custom_header custom_hdr;

    // Define os endereços MAC de origem e destino 52:54:00:00:00:01
    uint8_t dest_mac[6] = {0x08, 0x00, 0x00, 0x00, 0x01, 0x01};  // Controlador
    uint8_t src_mac[6] = {0x52, 0x54, 0x00, 0x00, 0x00, 0x00};  // MAC Host

    // Configura os campos do cabeçalho personalizado
    custom_hdr.oper = 0x02;

    swapEndian(argv[3], ID_RODADA_LEN);
    // Convertendo ID_Rodada em Bytes e já colocando na estrutura do cabeçalho do P4DRA
    hexstring_to_byte_array(custom_hdr.id_rodada, argv[3], ID_RODADA_LEN);

    // Cria o socket raw
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETHERTYPE_CUSTOM));
    if (sockfd < 0) {
        perror("Erro ao criar o socket");
        exit(1);
    }

    // Configura a estrutura sockaddr_ll para enviar pela interface de rede
    memset(&sa, 0, sizeof(struct sockaddr_ll));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETHERTYPE_CUSTOM);
    sa.sll_ifindex = if_nametoindex(iface);  // Substitua "eth0" pela interface de rede desejada
    sa.sll_halen = 6;
    memcpy(sa.sll_addr, dest_mac, 6);

    for (int id = 0; id < quantidade_provas; id++) {
        // Invertendo Endianess
        swapEndian(dados[id%20000].id_dispositivo, ID_DISPOSITIVO_LEN);
        swapEndian(dados[id%20000].prova, PROVA_LEN);

        // Passa os valores para Bytes
        if (hexstring_to_byte_array(custom_hdr.id_dispositivo, dados[id%20000].id_dispositivo, ID_DISPOSITIVO_LEN) != 0 ||
            hexstring_to_byte_array(custom_hdr.prova, dados[id%20000].prova, PROVA_LEN) != 0) {
            fprintf(stderr, "Erro ao converter valores hexadecimais: %64s", dados[id%20000].id_dispositivo);
            id = id+1;
            continue;
        }

        // Calculando SRC MAC
        src_mac[5] = ((id+1)%256);
        src_mac[4] = ((id/256)%256);
        src_mac[3] = (id/65536);

        // Cria o pacote
        create_custom_packet(packet, dest_mac, src_mac, &custom_hdr);
 
        // Envia o pacote
        if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&sa, sizeof(struct sockaddr_ll)) < 0) {
            perror("Erro ao enviar o pacote");
            close(sockfd);
            exit(1);
        } 
    }

    // Fecha o socket
    close(sockfd);
    return 0;
}
