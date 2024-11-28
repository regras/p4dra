#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ID_DISPOSITIVO_LEN 32 // 128 bits = 16 bytes -> 32 caracteres hexadecimais
#define CHAVE_LEN 64  // 256 bits = 32 bytes -> 64 caracteres hexadecimais
#define PROVA_LEN 128 // 512 bits = 64 bytes -> 128 caracteres hexadecimais
#define NONCE_LEN 16 // 64 bits = 8 bytes -> 16 caracteres hexadecimais

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

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <arquivo_provas> <id_rodada> <nonce>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *id_rodada = argv[2];
    // swapEndian(id_rodada, NONCE_LEN);
    char *nonce = argv[3];
    // swapEndian(nonce, NONCE_LEN);

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    char *linha = NULL;
    size_t len = 0;

    // Leitura de cada linha
    while (getline(&linha, &len, file) != -1) {
        char id_dispositivo[ID_DISPOSITIVO_LEN];
        char chave[CHAVE_LEN];
        char prova[PROVA_LEN];

        // Extrai os valores da linha
        if (sscanf(linha, "%32c:%64c:%128c:", id_dispositivo, chave, prova) == 3) {
            // convertendo Endianess
            // swapEndian(id_dispositivo, ID_DISPOSITIVO_LEN);
            // swapEndian(chave, CHAVE_LEN);
            // swapEndian(prova, PROVA_LEN);

            // Constrói o comando para chamar o outro programa
            char comando[256];
            snprintf(comando, sizeof(comando), "./forro-args-dra2 %.*s %.*s %.*s %.*s 0", CHAVE_LEN, chave, NONCE_LEN, nonce, NONCE_LEN, id_rodada, ID_DISPOSITIVO_LEN, id_dispositivo);

            FILE *processo = popen(comando, "r");
            if (processo == NULL) {
                perror("Erro ao executar o comando");
                return 1;
            }

            char *verificacao = NULL;
            size_t len = 0;

            printf("prova: %.*s\n", PROVA_LEN, prova);
            while (getline(&verificacao, &len, processo) != -1) {
                printf("verificacao: %s\n", verificacao);
            }

            pclose(processo);
        } else {
            fprintf(stderr, "Formato de linha inválido: %s", linha);
        }
    }

    // Libera a memória alocada para getline
    free(linha);
    fclose(file);

    return 0;
}
