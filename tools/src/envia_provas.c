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

int hex_to_bytes(const char *hex_str, unsigned char *bytes, size_t byte_len) {
    size_t hex_len = strlen(hex_str);
    if (hex_len != byte_len * 2) {
        return -1; // Erro: comprimento de string incorreto
    }
    for (size_t i = 0; i < byte_len; i++) {
        sscanf(hex_str + 2 * i, "%2hhx", &bytes[i]);
    }
    return 0;
}

// Estrutura para o cabeçalho do protocolo personalizado
struct custom_header {
    uint8_t oper;
    uint8_t id_rodada[8];
    uint8_t id_dispositivo[16];
    uint8_t prova[64];
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
    int sockfd;
    uint8_t packet[sizeof(struct ether_header) + sizeof(struct custom_header)];
    struct sockaddr_ll sa;
    struct custom_header custom_hdr;

    // Define os endereços MAC de origem e destino
    uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // Broadcast
    uint8_t src_mac[6] = {0x00, 0x0a, 0x95, 0x9d, 0x68, 0x16};    // MAC fictício de origem

    // Configura os campos do cabeçalho personalizado
    unsigned char oper_bytes[1];
    custom_hdr.oper = (uint8_t)strtoul(argv[1], NULL, 10);
    //memcpy(custom_hdr.oper, argv[1], sizeof(custom_hdr.oper));
    memcpy(custom_hdr.id_rodada, argv[2], sizeof(custom_hdr.id_rodada));
    memcpy(custom_hdr.id_dispositivo, argv[3], sizeof(custom_hdr.id_dispositivo));
    memcpy(custom_hdr.prova, argv[4], sizeof(custom_hdr.prova));

    // Cria o pacote
    create_custom_packet(packet, dest_mac, src_mac, &custom_hdr);

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
    sa.sll_ifindex = if_nametoindex("enp0s3");  // Substitua "eth0" pela interface de rede desejada
    sa.sll_halen = 6;
    memcpy(sa.sll_addr, dest_mac, 6);

    // Envia o pacote
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&sa, sizeof(struct sockaddr_ll)) < 0) {
        perror("Erro ao enviar o pacote");
        close(sockfd);
        exit(1);
    }

    printf("Pacote enviado com sucesso!\n");

    // Fecha o socket
    close(sockfd);
    return 0;
}
