#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>

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
char *arquivo_provas;
char *iface;
uint32_t contador_pacotes = 0;
uint32_t total_pacotes = 1000;
double time1, time2, timedif, timesum = 0;
double time_arquivo1, time_arquivo2, time_arquivodif, time_arquivosum = 0;
double time_calculo1, time_calculo2, time_calculodif, time_calculosum = 0;

// Estrutura para carregar arquivo de ids/chaves/provas na memoria
struct id_chave_prova_t {
    char id_dispositivo[ID_DISPOSITIVO_LEN];
    char chave[CHAVE_LEN];
    char prova[PROVA_LEN];
};

struct id_chave_prova_t dados[ID_MAX]; 

// Estrutura para o cabeçalho do protocolo personalizado
struct custom_header {
    uint8_t oper;
    char id_rodada[ID_RODADA_LEN_BYTES];
    uint8_t id_dispositivo[ID_DISPOSITIVO_LEN_BYTES]; // 128 bits = 16 bytes
    uint8_t prova[PROVA_LEN_BYTES]; // 512 bits = 64 bytes
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

int buscarPorIdMemoria(const char *id_procurado, char *chave, char *prova) {
    for (int i = 0; i < ID_MAX; i++) {
        if (memcmp(dados[i].id_dispositivo, id_procurado, ID_DISPOSITIVO_LEN) == 0) {
            // Copiar a chave e a prova para as variáveis fornecidas
            memcpy(chave, dados[i].chave, CHAVE_LEN);
            memcpy(prova, dados[i].prova, PROVA_LEN);
            return 0; // Sucesso
        }
    }
    return -1; //nao encontrado
}

int buscarPorIdDireto(uint32_t id, char *chave, char *prova) {
    memcpy(chave, dados[id].chave, CHAVE_LEN);
    memcpy(prova, dados[id].prova, PROVA_LEN);
    return 0; // Sucesso
}

int buscarPorId(const char *arquivo, const char *id_procurado, char *chave, char *prova) {
    FILE *fp = fopen(arquivo, "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    char *linha = NULL;
    size_t len = 0;

    while (getline(&linha, &len, fp) != -1) {
        char id_dispositivo_hex[ID_DISPOSITIVO_LEN];
        char chave_hex[CHAVE_LEN];
        char prova_hex[PROVA_LEN];

        // Dividir a linha com sscanf
        if (sscanf(linha, "%32c:%64c:%128c:", id_dispositivo_hex, chave_hex, prova_hex) == 3) {
            // Verificar se o ID corresponde ao ID procurado
            if (memcmp(id_dispositivo_hex, id_procurado, ID_DISPOSITIVO_LEN) == 0) {
                // Copiar a chave e a prova para as variáveis fornecidas
                memcpy(chave, chave_hex, CHAVE_LEN);
                memcpy(prova, prova_hex, PROVA_LEN);
                fclose(fp);
                return 0; // Sucesso
            }
        }
    }

    fclose(fp);
    return -1; // ID não encontrado
}

void print_custom_header(const struct custom_header *header) {
    printf("Oper: %u\n", header->oper);
    printf("ID Rodada: ");
    for (int i = 0; i < ID_RODADA_LEN_BYTES; i++) {
        printf("%02x", header->id_rodada[i]);
    }
    printf("\n");

    printf("ID Dispositivo: ");
    for (int i = 0; i < ID_DISPOSITIVO_LEN_BYTES; i++) {
        printf("%02x", header->id_dispositivo[i]);
    }
    printf("\n");

    printf("Prova: ");
    for (int i = 0; i < PROVA_LEN_BYTES; i++) {
        printf("%02x", header->prova[i]);
    }
    printf("\n\n");
}

void packet_handler(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // Cabeçalho Ethernet
    struct ether_header *eth_header = (struct ether_header *) packet;

    // Verifica se o Ethertype é o esperado
    if (ntohs(eth_header->ether_type) == ETHERTYPE_CUSTOM) {
        // printf("Pacote com Ethertype 0x%04x encontrado! Numero %u\n", ETHERTYPE_CUSTOM, contador_pacotes);
        // if (contador_pacotes % 1000 == 0) {
            // printf("pacote numero %u recebido!\n", contador_pacotes);
        // }

        // Calcula o offset para o cabeçalho personalizado
        const struct custom_header *custom_hdr = (struct custom_header *)(packet + sizeof(struct ether_header));

        // Começar a contar se for o primeiro pacote de resposta
        if (contador_pacotes == 0 && custom_hdr->oper == 0x2) {
            time1 = (double) clock();
        }

        // Imprime os campos do cabeçalho personalizado
        // print_custom_header(custom_hdr);

        // extraindo conteudo dos cabeçalhos
        char *id_rodada_hex = byte_array_to_hexstring(custom_hdr->id_rodada, ID_RODADA_LEN_BYTES);
        char *id_dispositivo_hex = byte_array_to_hexstring(custom_hdr->id_dispositivo, ID_DISPOSITIVO_LEN_BYTES);
        char *prova_hex = byte_array_to_hexstring(custom_hdr->prova, PROVA_LEN_BYTES);
        
        // vindo da rede, eh preciso inverter o Endianess
        swapEndian(id_dispositivo_hex, ID_DISPOSITIVO_LEN);
        swapEndian(prova_hex, PROVA_LEN);
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

        // Buscando no arquivo
        char chave_hex[CHAVE_LEN];
        char verificacao_hex[PROVA_LEN];

        time_arquivo1 = (double)clock();
        // if (buscarPorId(arquivo_provas, id_dispositivo_hex, chave_hex, verificacao_hex) == 0) {
        // if (buscarPorIdMemoria(id_dispositivo_hex, chave_hex, verificacao_hex) == 0) {
        if (buscarPorIdDireto(contador_pacotes, chave_hex, verificacao_hex) == 0) {
            time_arquivo2 = (double)clock();
            time_arquivodif = ((double) time_arquivo2 - time_arquivo1) / CLOCKS_PER_SEC;
            time_arquivosum += time_arquivodif;
            // Constrói o comando para chamar o outro programa
            char comando[512];
            snprintf(comando, sizeof(comando), "./forro-args-dra2 %.*s %.*s %.*s %.*s 0", CHAVE_LEN, chave_hex, NONCE_LEN, param2_hex, NONCE_LEN, param1_hex, ID_DISPOSITIVO_LEN, id_dispositivo_hex);
            // printf("Executando comando: %s\n", comando);

            time_calculo1 = (double)clock();
            FILE *processo = popen(comando, "r");
            if (processo == NULL) {
                perror("Erro ao executar o comando");
                exit(1);
            }

            char *verificacao = NULL;
            size_t len = 0;

            getline(&verificacao, &len, processo);
            // printf("verificacao: %s\n", verificacao);

            // while (getline(&verificacao, &len, processo) != -1) {
            //     printf("verificacao: %s\n", verificacao);
            // }

            pclose(processo);
            time_calculo2 = (double)clock();
            time_calculodif = ((double) time_calculo2 - time_calculo1) / CLOCKS_PER_SEC;
            time_calculosum += time_calculodif;

            // printf("prova: %.*s\n", PROVA_LEN, prova_hex);
            // printf("verificacao: %.*s\n", PROVA_LEN, verificacao_hex);
            // if (memcmp(prova_hex, verificacao, PROVA_LEN) == 0) {
            //     printf("Prova valida: %.*s\n", PROVA_LEN, prova_hex);
            // } else {
            //     printf("Prova invalida!\n");
            // }
        } else {
            printf("ID não encontrado.\n");
        }

        contador_pacotes += 1;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arquivo_provas> <interface_captura> [quantidade_pacotes] \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    arquivo_provas = argv[1];
    iface = argv[2];

    if (argc == 4) {
        total_pacotes = (uint32_t)strtoul(argv[3], NULL, 10);
    }

    printf("carregando na memoria... \n");
    carregarArquivoMemoria(arquivo_provas);
    printf("carregado!\n");

    char error_buffer[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
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
    if (pcap_loop(handle, total_pacotes, packet_handler, NULL) < 0) {
        fprintf(stderr, "Erro ao capturar pacotes: %s\n", pcap_geterr(handle));
        return 2;
    }

    time2 = (double) clock();
    timedif = ((double) time2 - time1) / CLOCKS_PER_SEC;
    // printf("tempo total para processar %i pacotes: %f segundos\n", total_pacotes, timedif);
    // printf("tempo para buscar %i IDs: %f segundos\n", total_pacotes, time_arquivosum);
    // printf("tempo para calcular %i verificacoes: %f segundos\n", total_pacotes, time_calculosum);
    printf("%i;%f;%f;%f\n", total_pacotes, timedif, time_arquivosum, time_calculosum);

    // Libere o filtro e feche o handle
    pcap_freecode(&filter);
    pcap_close(handle);
    return 0;
}
