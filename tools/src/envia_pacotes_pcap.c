#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <linux/if_packet.h>

#define SNAP_LEN 1518 // Tamanho máximo do pacote

uint32_t quantidade_pacotes = 1000; // quantidade de pacotes a enviar por padrao

// Função para reenvio de pacotes pela interface
void resend_packet(pcap_t *handle_out, const u_char *packet, struct pcap_pkthdr *header) {
    if (pcap_sendpacket(handle_out, packet, header->len) != 0) {
        fprintf(stderr, "Erro ao enviar pacote: %s\n", pcap_geterr(handle_out));
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arquivo.pcap> <interface> [quantidade_pacotes]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 4) {
        quantidade_pacotes = (uint32_t)strtoul(argv[3], NULL, 10);
    }

    char *pcap_file = argv[1];
    char *iface = argv[2];

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle_in, *handle_out;

    // Abrir o arquivo PCAP para leitura
    handle_in = pcap_open_offline(pcap_file, errbuf);
    if (handle_in == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo PCAP: %s\n", errbuf);
        return 1;
    }

    // Abrir a interface para envio de pacotes
    handle_out = pcap_open_live(iface, SNAP_LEN, 1, 1000, errbuf);
    if (handle_out == NULL) {
        fprintf(stderr, "Erro ao abrir a interface %s: %s\n", iface, errbuf);
        pcap_close(handle_in);
        return 1;
    }

    struct pcap_pkthdr *header;
    const u_char *packet;
    int res;
    uint32_t pacotes_enviados = 0;

    // Ler cada pacote do arquivo PCAP e reenviar
    while ((res = pcap_next_ex(handle_in, &header, &packet)) >= 0) {
        if (res == 0) {
            // Timeout, continuar para o próximo pacote
            continue;
        }

        // Reenviar o pacote
        resend_packet(handle_out, packet, header);

        pacotes_enviados += 1;

        if (pacotes_enviados == quantidade_pacotes) {
            printf("enviado %u pacotes\n", pacotes_enviados);
            break;
        }
    }

    // Checar por erros de leitura
    if (res == -1) {
        fprintf(stderr, "Erro ao ler pacote: %s\n", pcap_geterr(handle_in));
    }

    // Fechar os handles
    pcap_close(handle_in);
    pcap_close(handle_out);

    printf("Reenvio completo.\n");
    return 0;
}
