// Faz o XOR, se todos os valores resultarem em 0, então a prova eh valida
action verificar_prova(in bit<160> cipher, in bit<160> payload) {
    meta.verificacao = cipher ^ payload;
}