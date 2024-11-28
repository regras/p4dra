#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "src/forro/ref/forro-debug.h"
#include "src/forro/ref/forro.h"

// Função para trocar a ordem dos bytes (big-endian para little-endian ou vice-versa)
uint32_t swapEndianChunk(uint32_t value) {
    return ((value >> 24) & 0xFF) |        // Byte 1 -> Byte 4
           ((value >> 8) & 0xFF00) |       // Byte 2 -> Byte 3
           ((value << 8) & 0xFF0000) |     // Byte 3 -> Byte 2
           ((value << 24) & 0xFF000000);   // Byte 4 -> Byte 1
}

// Função para trocar a endianness de uma entrada
void swapEndian(uint8_t *input, uint8_t *output, size_t size) {
    for (size_t i = 0; i < size; i += 4) {
        // Lê 4 bytes como um uint32_t
        uint32_t chunk;
        chunk = (input[i] << 24) | (input[i + 1] << 16) | (input[i + 2] << 8) | input[i + 3];
        
        // Realiza o swap dos bytes
        uint32_t swappedChunk = swapEndianChunk(chunk);
        
        // Armazena o resultado no array de saída
        output[i] = (swappedChunk >> 24) & 0xFF;
        output[i + 1] = (swappedChunk >> 16) & 0xFF;
        output[i + 2] = (swappedChunk >> 8) & 0xFF;
        output[i + 3] = swappedChunk & 0xFF;
    }
}

// Função para converter uma hexstring para um array de uint8_t
int hexstring_to_byte_array(uint8_t *byte_array, const char *hexstring) {
    size_t len = strlen(hexstring);

    // Calcular o tamanho do array de bytes
    size_t byte_array_len = len / 2;

    // Converter cada par de caracteres hexadecimais para um byte
    for (size_t i = 0; i < byte_array_len; i++) {
        char byte_str[3] = { hexstring[2 * i], hexstring[2 * i + 1], '\0' };
        byte_array[i] = (uint8_t)strtoul(byte_str, NULL, 16);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    // criando contexto e saida
    stream_ctx input;
    uint64_t number_of_bytes = 64;
    uint8_t *output_forro = (uint8_t *)malloc(number_of_bytes);
    uint8_t *mensagem = (uint8_t *)malloc(number_of_bytes);
    uint8_t *cifrado = (uint8_t *)malloc(number_of_bytes);

    uint32_t chunk;

    // capturando chave do primeiro argumento
    uint8_t key[32];
    char key_HEX[64];
    memcpy(key_HEX, argv[1], 64);
    hexstring_to_byte_array(key, key_HEX);
    // swapEndian(key, key, 32);

    // capturando nonce do segundo argumento
    uint8_t nonce[8];
    char nonce_HEX[16];
    memcpy(nonce_HEX, argv[2], 16);
    hexstring_to_byte_array(nonce, nonce_HEX);
    // swapEndian(nonce, nonce, 8);

    // capturando id_rodada no terceiro argumento
    uint8_t id_rodada[8];
    char id_rodada_HEX[16];
    memcpy(id_rodada_HEX, argv[3], 16);
    hexstring_to_byte_array(id_rodada, id_rodada_HEX);
    // swapEndian(id_rodada, id_rodada, 8);

    // capturando id_dispositivo no quarto argumento
    uint8_t id_dispositivo[16];
    char id_dispositivo_HEX[32];
    memcpy(id_dispositivo_HEX, argv[4], 32);
    hexstring_to_byte_array(id_dispositivo, id_dispositivo_HEX);
    // swapEndian(id_dispositivo, id_dispositivo, 16);

    // capturando configuracao a ser verificada
    uint8_t configuracao[32];
    char configuracao_HEX[64];
    memcpy(configuracao_HEX, argv[5], 64);
    hexstring_to_byte_array(configuracao, configuracao_HEX);
    // swapEndian(configuracao, configuracao, 32);

    // carregando chave, iv e rodando o algoritmo
    // printf("rodando Key Setup\n");
    forro_keysetup(&input, key);
    // printf("rodando IV Setup\n");
    // forro_ivsetup(&input, iv);

    uint8_t configuracao_rodada[16] ;
    uint8_t configuracao_dispositivo[16];

    // id_rodada XOR C0 e C1 (0-3 e 4-7)
    for (int i=0; i < 8; i++) {
        configuracao_rodada[i] = id_rodada[i] ^ configuracao[i];
    }

    // id_dispositivo XOR C2 e C3 (8-11 e 12-15)
    for (int i=0; i < 8; i++) {
        configuracao_dispositivo[i] = id_dispositivo[i] ^ configuracao[i+8];
    }

    // nonce XOR C4 e C5 (16-19 e 20-23)
    for (int i=0; i < 8; i++) {
        configuracao_rodada[i+8] = nonce[i] ^ configuracao[i+16];
    }

    // id_dispositivo XOR C6 e C7 (24-27 e 28-31)
    for (int i=0; i < 8; i++) {
        configuracao_dispositivo[i+8] = id_dispositivo[i+8] ^ configuracao[i+24];
    }

    // printf("rodando Verificador Setup\n");
    forro_versetup(&input, configuracao_rodada, configuracao_dispositivo);

    // Rodando a cifra da mensagem informada
    // printf("Cifra da Mensagem\n");
    forro_encrypt_bytes(&input, mensagem, cifrado, number_of_bytes);

    // Cifra uma sequência de zeros como mensagem
    // printf("Rodando a cifra com zeros\n");
    // forro_keystream_bytes(&input, output_forro, number_of_bytes);

    // imprimindo mensagem
    /* printf("Mensagem:\n");
    for (int i = 0; i < number_of_bytes; i++)
    {
        printf("%02x", mensagem[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n"); */

    // imprimindo configuracao
    /* printf("Configuracao:\n");
    for (int i = 0; i < 32; i++)
    {
        printf("%02x", configuracao[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n"); */

    // imprimindo id_dispositivo
    /* printf("id_dispositivo:\n");
    for (int i = 0; i < 16; i++)
    {
        printf("%02x", id_dispositivo[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n"); */

    // imprimindo id_rodada
    /* printf("id_rodada:\n");
    for (int i = 0; i < 8; i++)
    {
        printf("%02x", id_rodada[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n"); */

    // imprimindo configuracao_dispositivo e configuracao_rodada
    /* printf("\nconfiguracao_dispositivo:\n");
    for (int i = 0; i < 16; i++)
    {
        printf("%02x", configuracao_dispositivo[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n");
    printf("configuracao_rodada:\n");
    for (int i = 0; i < 16; i++)
    {
        printf("%02x", configuracao_rodada[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n"); */

    // imprimindo detalhes da chave e do Nonce
    /* printf("KEY:\n");
    for (int i = 0; i < 32; i++)
    {
        printf("%02x", key[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\nNONCE:\n");
    for (int i = 0; i < 8; i++)
    {
        printf("%02x", nonce[i]);
        if (i % 8 == 7)
            printf("\n");
    } */

     // dividir o resultado sempre em 8 linhas de x/8 bytes
    int div_bloco = number_of_bytes/8;

    // imprimindo resultado em bloco e depois em texto corrido
/*    printf("\nResultado do estado:\n");
    for (int i = 0; i < 8; i++) {
        for (int k = 0; k < div_bloco; k++) {
            printf("%02x", output_forro[div_bloco * i + k]);
        }
        printf("\n");
    }

    printf("\n"); */

/*     for (int i = 0; i < 8; i++) {
        for (int k = 0; k < div_bloco; k++) {
            printf("%02x", output_forro[div_bloco * i + k]);
        }
    }
    printf("\n");

    printf("\nCifrado:\n");
    for (int i = 0; i < 8; i++) {
        for (int k = 0; k < div_bloco; k++) {
            printf("%02x", cifrado[div_bloco * i + k]);
        }
        printf("\n");
    }

    printf("\n"); */

    for (int i = 0; i < 8; i++) {
        for (int k = 0; k < div_bloco; k++) {
            printf("%02x", cifrado[div_bloco * i + k]);
        }
    }
    // printf("\n");

    return 0;
}
