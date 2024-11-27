#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <time.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <sys/socket.h>
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
#define SNAP_LEN 176 //tamanho de pacote a capturar, 176 é suficiente.

// Parametros globais
pcap_t *handle;
char *iface;
uint32_t contador_pacotes = 0;
uint32_t total_pacotes = 1000;
double time1, time2, timedif, timesum = 0;
double time_calculo1, time_calculo2, time_calculodif, time_calculosum = 0;
struct timespec ts;

char *chave_hex;
char *id_dispositivo_hex;

// Estrutura para o cabeçalho do protocolo personalizado
struct p4dra_oper_h {
    uint8_t oper;
    uint8_t id_rodada[ID_RODADA_LEN_BYTES];
};

// Estrutura para o cabeçalho do protocolo personalizado
struct p4dra_reply_h {
    uint8_t id_dispositivo[ID_DISPOSITIVO_LEN_BYTES];
    uint8_t prova[PROVA_LEN_BYTES];
};

// Estrutura para o cabeçalho do protocolo personalizado
struct p4dra_verificacao_h {
    uint8_t id_dispositivo[ID_DISPOSITIVO_LEN_BYTES];
    uint8_t verificacao[PROVA_LEN_BYTES];
    uint8_t prova[PROVA_LEN_BYTES];
};

struct p4dra_setup_h {
    uint8_t chave[CHAVE_LEN_BYTES]; // 512 bits = 64 bytes
    uint8_t id_dispositivo[ID_DISPOSITIVO_LEN_BYTES]; // 128 bits = 16 bytes
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

char *byte_array_to_hexstring(const uint8_t *array, size_t array_size) {
    // Aloca memória para a string hexadecimal (2 caracteres por byte + 1 para o terminador nulo)
    char *hexstr = malloc(array_size * 2);
    if (hexstr == NULL) {
        return NULL; // Retorna NULL em caso de falha na alocação
    }

    // Converte cada byte do array em dois caracteres hexadecimais
    for (size_t i = 0; i < array_size; i++) {
        sprintf(&hexstr[i * 2], "%02x", array[i]);
    }

    return hexstr;
}

void create_custom_packet(uint8_t *packet, const uint8_t *dest_mac, const uint8_t *src_mac, struct p4dra_oper_h *oper_hdr, struct p4dra_reply_h *reply_hdr) {
    struct ether_header *eth_header = (struct ether_header *) packet;

    // Configura o cabeçalho Ethernet
    memcpy(eth_header->ether_dhost, dest_mac, 6);
    memcpy(eth_header->ether_shost, src_mac, 6);
    eth_header->ether_type = htons(ETHERTYPE_CUSTOM);

    // Adiciona o cabeçalho personalizado após o cabeçalho Ethernet
    memcpy(packet + sizeof(struct ether_header), oper_hdr, sizeof(struct p4dra_oper_h));
    memcpy(packet + sizeof(struct ether_header) + sizeof(struct p4dra_oper_h), reply_hdr, sizeof(struct p4dra_reply_h));
}

uint8_t envia_pacote_resposta(uint8_t *dest_mac, uint8_t *src_mac, char *id_rodada_hex, char *id_dispositivo_hex, char *prova_hex) {
    int sockfd;
    uint8_t packet[sizeof(struct ether_header) + sizeof(struct p4dra_oper_h) + sizeof(struct p4dra_reply_h)];
    struct sockaddr_ll sa;
    struct p4dra_oper_h *oper_hdr = (struct p4dra_oper_h *)(packet + sizeof(struct ether_header));
    struct p4dra_reply_h *reply_hdr = (struct p4dra_reply_h *)(packet + sizeof(struct ether_header) + sizeof(struct p4dra_oper_h));

    // Necessario fazer uma copia pelo SwapEndian fazer a modificação in-place
    char id_dispositivo_hex_LE[ID_DISPOSITIVO_LEN];
    memcpy(id_dispositivo_hex_LE, id_dispositivo_hex, ID_DISPOSITIVO_LEN);

    // Enviando para o switch, entao invertendo:
    swapEndian(id_rodada_hex,ID_RODADA_LEN);
    swapEndian(id_dispositivo_hex_LE, ID_DISPOSITIVO_LEN);
    swapEndian(prova_hex, PROVA_LEN);

    // Configura os campos do cabeçalho personalizado
    oper_hdr->oper = 0x2;
    hexstring_to_byte_array(oper_hdr->id_rodada, id_rodada_hex, ID_RODADA_LEN);
    hexstring_to_byte_array(reply_hdr->id_dispositivo, id_dispositivo_hex_LE, ID_DISPOSITIVO_LEN);
    hexstring_to_byte_array(reply_hdr->prova, prova_hex, PROVA_LEN);

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

    // Cria o pacote, invertendo smac e dmac
    create_custom_packet(packet, src_mac, dest_mac, oper_hdr, reply_hdr);

    // Envia o pacote
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&sa, sizeof(struct sockaddr_ll)) < 0) {
        perror("Erro ao enviar o pacote");
        close(sockfd);
        exit(1);
    }

    // Fecha o socket
    close(sockfd);
    return 0;
}

void print_p4dra_oper_h(const struct p4dra_oper_h *header) {
    printf("Oper: %u\n", header->oper);
    printf("ID Rodada: ");
    for (int i = 0; i < ID_RODADA_LEN_BYTES; i++) {
        printf("%02x", header->id_rodada[i]);
    }
    printf("\n\n");
}

void print_p4dra_setup_h(const struct p4dra_setup_h *header) {
    printf("ID Dispositivo: ");
    for (int i = 0; i < ID_DISPOSITIVO_LEN_BYTES; i++) {
        printf("%02x", header->id_dispositivo[i]);
    }
    printf("\n");

    printf("Chave: ");
    for (int i = 0; i < CHAVE_LEN_BYTES; i++) {
        printf("%02x", header->chave[i]);
    }
    printf("\n\n");
}

void packet_handler(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // Cabeçalho Ethernet
    struct ether_header *eth_header = (struct ether_header *) packet;

    // Verifica se o Ethertype é o esperado
    if (ntohs(eth_header->ether_type) == ETHERTYPE_CUSTOM) {
        // Calcula o offset para o cabeçalho personalizado
        const struct p4dra_oper_h *oper_hdr = (struct p4dra_oper_h *)(packet + sizeof(struct ether_header));
        char *id_rodada_hex = byte_array_to_hexstring(oper_hdr->id_rodada, ID_RODADA_LEN_BYTES);

        // Se estiver vindo do switch, eh preciso inverter o Endianess
        swapEndian(id_rodada_hex,ID_RODADA_LEN);

        // dividindo ID_RODADA com entre Param1 (id_rodada[15:8]) e Param2(id_rodada[7:0], ou nonce)
        char param1_hex[NONCE_LEN];
        for (int i = 0; i < NONCE_LEN; i ++) {
            param1_hex[i] = id_rodada_hex[i];
        }

        char param2_hex[NONCE_LEN];
        for (int i = NONCE_LEN; i < ID_RODADA_LEN; i ++) {
            param2_hex[i-NONCE_LEN] = id_rodada_hex[i];
        }

        // Imprime os campos do cabeçalho personalizado
        // print_p4dra_oper_h(oper_hdr);

        // Se for pedido, calcula verificação
        if (oper_hdr->oper == 0x1) {
            // Começar a contar se for o primeiro pacote de solicitação
            if (contador_pacotes == 0) {
                time1 = (double) clock();
            }

            clock_gettime(CLOCK_REALTIME, &ts);
            long long microssegundos = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
            // time2 = (double) clock() / CLOCKS_PER_SEC;
            printf("%lld;", microssegundos);
            for (int i = 0; i < 6; i++) {
                if (i == 5) {
                    printf("%02x;", eth_header->ether_shost[i]);
                    continue;
                }
                printf("%02x:", eth_header->ether_shost[i]);
            }
            for (int i = 0; i < 6; i++) {
                if (i == 5) {
                    printf("%02x;", eth_header->ether_dhost[i]);
                    continue;
                }
                printf("%02x:", eth_header->ether_dhost[i]);
            }
            printf("%x;", oper_hdr->oper);
            for (int i = 0; i < ID_RODADA_LEN_BYTES; i++) {
                printf("%02x", oper_hdr->id_rodada[i]);
            }
            printf(";\n");

            /* // Constrói o comando para chamar o outro programa
            char comando[512];
            snprintf(comando, sizeof(comando), "./forro-args-dra2 %.*s %.*s %.*s %.*s 0", CHAVE_LEN, chave_hex, NONCE_LEN, param2_hex, NONCE_LEN, param1_hex, ID_DISPOSITIVO_LEN, id_dispositivo_hex);
            // printf("Executando comando: %s\n", comando);

            time_calculo1 = (double)clock();
            FILE *processo = popen(comando, "r");
            if (processo == NULL) {
                perror("Erro ao executar o comando");
                exit(1);
            }

            char *prova_hex = NULL;
            size_t len = 0;

            getline(&prova_hex, &len, processo);

            // printf("prova: %.*s\n", PROVA_LEN, prova_hex);

            pclose(processo);
            time_calculo2 = (double)clock();
            time_calculodif = ((double) time_calculo2 - time_calculo1) / CLOCKS_PER_SEC;
            time_calculosum += time_calculodif;

            envia_pacote_resposta(eth_header->ether_dhost, eth_header->ether_shost, id_rodada_hex, id_dispositivo_hex, prova_hex);

            // Incrementando contador
            contador_pacotes += 1;

            if (contador_pacotes == total_pacotes) {
                pcap_breakloop(handle);
            } */

        // Se for SETUP, extrai o cabeçalho e carrega os valores como hexstring
        } else if (oper_hdr->oper == 0x5) {
            const struct p4dra_setup_h *setup_hdr = (struct p4dra_setup_h *)(packet + sizeof(struct ether_header) + sizeof(struct p4dra_oper_h));
            chave_hex = byte_array_to_hexstring(setup_hdr->chave, CHAVE_LEN_BYTES);
            id_dispositivo_hex = byte_array_to_hexstring(setup_hdr->id_dispositivo, ID_DISPOSITIVO_LEN_BYTES);

            clock_gettime(CLOCK_REALTIME, &ts);
            long long microssegundos = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
            // time2 = (double) clock() / CLOCKS_PER_SEC;
            printf("%lld;", microssegundos);
            for (int i = 0; i < 6; i++) {
                if (i == 5) {
                    printf("%02x;", eth_header->ether_shost[i]);
                    continue;
                }
                printf("%02x:", eth_header->ether_shost[i]);
            }
            for (int i = 0; i < 6; i++) {
                if (i == 5) {
                    printf("%02x;", eth_header->ether_dhost[i]);
                    continue;
                }
                printf("%02x:", eth_header->ether_dhost[i]);
            }
            printf("%x;", oper_hdr->oper);
            for (int i = 0; i < ID_RODADA_LEN_BYTES; i++) {
                printf("%02x", oper_hdr->id_rodada[i]);
            }
            printf(";");
            for (int i = 0; i < ID_DISPOSITIVO_LEN_BYTES; i++) {
                printf("%02x", setup_hdr->id_dispositivo[i]);
            }
            printf(";");
            for (int i = 0; i < CHAVE_LEN_BYTES; i++) {
                printf("%02x", setup_hdr->chave[i]);
            }
            printf(";\n");

            // vindo da rede, eh preciso inverter o Endianess
            swapEndian(chave_hex, CHAVE_LEN);
            swapEndian(id_dispositivo_hex, ID_DISPOSITIVO_LEN);

            // Imprime os campos do cabeçalho personalizado
            // print_p4dra_setup_h(setup_hdr);
        // Se for REPLY, extrai o cabeçalho e imprime
        } else if (oper_hdr->oper == 0x2) {
            const struct p4dra_reply_h *reply_hdr = (struct p4dra_reply_h *)(packet + sizeof(struct ether_header) + sizeof(struct p4dra_oper_h));

            clock_gettime(CLOCK_REALTIME, &ts);
            long long microssegundos = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
            // time2 = (double) clock() / CLOCKS_PER_SEC;
            printf("%lld;", microssegundos);
            for (int i = 0; i < 6; i++) {
                if (i == 5) {
                    printf("%02x;", eth_header->ether_shost[i]);
                    continue;
                }
                printf("%02x:", eth_header->ether_shost[i]);
            }
            for (int i = 0; i < 6; i++) {
                if (i == 5) {
                    printf("%02x;", eth_header->ether_dhost[i]);
                    continue;
                }
                printf("%02x:", eth_header->ether_dhost[i]);
            }
            printf("%x;", oper_hdr->oper);
            for (int i = 0; i < ID_RODADA_LEN_BYTES; i++) {
                printf("%02x", oper_hdr->id_rodada[i]);
            }
            printf(";");
            for (int i = 0; i < ID_DISPOSITIVO_LEN_BYTES; i++) {
                printf("%02x", reply_hdr->id_dispositivo[i]);
            }
            printf(";");
            for (int i = 0; i < PROVA_LEN_BYTES; i++) {
                printf("%02x", reply_hdr->prova[i]);
            }
            printf(";\n");
        // Se for VERIFICACAO, extrai o cabeçalho e imprime
        } else if (oper_hdr->oper == 0x4) {
            const struct p4dra_verificacao_h *verif_hdr = (struct p4dra_verificacao_h *)(packet + sizeof(struct ether_header) + sizeof(struct p4dra_oper_h));
            
            clock_gettime(CLOCK_REALTIME, &ts);
            long long microssegundos = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
            // time2 = (double) clock() / CLOCKS_PER_SEC;
            printf("%lld;", microssegundos);
            for (int i = 0; i < 6; i++) {
                if (i == 5) {
                    printf("%02x;", eth_header->ether_shost[i]);
                    continue;
                }
                printf("%02x:", eth_header->ether_shost[i]);
            }
            for (int i = 0; i < 6; i++) {
                if (i == 5) {
                    printf("%02x;", eth_header->ether_dhost[i]);
                    continue;
                }
                printf("%02x:", eth_header->ether_dhost[i]);
            }
            printf("%x;", oper_hdr->oper);
            for (int i = 0; i < ID_RODADA_LEN_BYTES; i++) {
                printf("%02x", oper_hdr->id_rodada[i]);
            }
            printf(";");
            for (int i = 0; i < ID_DISPOSITIVO_LEN_BYTES; i++) {
                printf("%02x", verif_hdr->id_dispositivo[i]);
            }
            printf(";");
            for (int i = 0; i < PROVA_LEN_BYTES; i++) {
                printf("%02x", verif_hdr->verificacao[i]);
            }
            printf(";");
            for (int i = 0; i < PROVA_LEN_BYTES; i++) {
                printf("%02x", verif_hdr->prova[i]);
            }
            printf(";\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <interface_captura> [quantidade_pacotes] \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    iface = argv[1];

    if (argc == 3) {
        total_pacotes = (uint32_t)strtoul(argv[2], NULL, 10);
    }

    char error_buffer[PCAP_ERRBUF_SIZE];
    struct bpf_program filter;
    char filter_exp[] = "ether proto 0x1234";  // Filtro para Ethertype 0x1234
    bpf_u_int32 net;

    // Abra a interface para captura
    // handle = pcap_open_live(iface, BUFSIZ, 1, 1000, error_buffer);
    handle = pcap_create(iface, error_buffer);
    if (handle == NULL) {
        fprintf(stderr, "Erro ao abrir a interface: %s\n", error_buffer);
        return 2;
    }

    //Ajustando buffer para 200MB e SNAP LEN para 105B para otimizar a captura e nao perder pacotes
    pcap_set_buffer_size(handle, 200 * 1024 * 1024);
    pcap_set_snaplen(handle, SNAP_LEN);
    pcap_set_immediate_mode(handle, 1);
    pcap_set_promisc(handle, 1);
    pcap_set_timeout(handle, 0);

    // Ativa a captura
    if (pcap_activate(handle) != 0) {
        fprintf(stderr, "Erro ao ativar o handle: %s\n", pcap_geterr(handle));
        return 2;
    }

    // Compile o filtro
    if (pcap_compile(handle, &filter, filter_exp, 0, net) == -1) {
        fprintf(stderr, "Erro ao compilar o filtro: %s\n", pcap_geterr(handle));
        return 2;
    }

    // Aplique o filtro
    if (pcap_setfilter(handle, &filter) == -1) {
        fprintf(stderr, "Erro ao aplicar o filtro: %s\n", pcap_geterr(handle));
        return 2;
    }

    // Captura de pacotes
    pcap_loop(handle, 0, packet_handler, NULL);
    
    time2 = (double) clock();
    timedif = ((double) time2 - time1) / CLOCKS_PER_SEC;
    // printf("tempo total para processar %i pacotes: %f segundos\n", total_pacotes, timedif);
    // printf("tempo para calcular %i verificacoes: %f segundos\n", total_pacotes, time_calculosum);
    printf("%i;%f;%f\n", total_pacotes, timedif, time_calculosum);

    // Libere o filtro e feche o handle
    pcap_freecode(&filter);
    pcap_close(handle);
    return 0;
}
