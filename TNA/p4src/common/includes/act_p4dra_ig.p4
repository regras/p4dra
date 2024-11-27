/* acoes gerais */

action forward_request (bit<9> port) {
    hdr.p4dra_oper.oper = 0x1;
    ig_tm_md.ucast_egress_port = port;
    ig_tm_md.bypass_egress = 0x1;
    exit;
}

action forward (bit<9> port) {
    ig_tm_md.ucast_egress_port = port;
    ig_tm_md.bypass_egress = 0x1;
    exit;
}

/* Iniciando */

// Carrega o Forro_state para iniciar a verificacao
action carregar_estado_0(bit<256> configuracao,
    hashword_t key0, hashword_t key1, hashword_t key2, hashword_t key3,
    hashword_t key4, hashword_t key5, hashword_t key6, hashword_t key7) {
    //Carregando cabeçalhos
    hdr.forro_rodada.setValid();
    hdr.forro_state.setValid();

    // Carregando chave no estado
    hdr.forro_state.v0  = key0;
    hdr.forro_state.v1  = key1;
    hdr.forro_state.v2  = key2;
    hdr.forro_state.v3  = key3;
    hdr.forro_state.v8  = key4;
    hdr.forro_state.v9  = key5;
    hdr.forro_state.v10 = key6;
    hdr.forro_state.v11 = key7;

    //Carregando configuracao
    hdr.forro_state.v4  = configuracao[255:224] ^ copy32_0.get(hdr.p4dra_oper.id_rodada0); // id_rodada0 XOR conf
    hdr.forro_state.v5  = configuracao[223:192]; // id_rodada1 XOR conf (a fazer)
    hdr.forro_state.v6  = configuracao[191:160]; // id_dispositivo0 XOR conf (feito controlador)
    hdr.forro_state.v7  = configuracao[159:128]; // id_dispositivo0 XOR conf (feito controlador)
    hdr.forro_state.v12 = configuracao[127: 96]; // id_rodada2 XOR conf (a fazer)
    hdr.forro_state.v13 = configuracao[ 95: 64]; // id_rodada3 XOR conf (a fazer)
    hdr.forro_state.v14 = configuracao[ 63: 32]; // id_dispositivo0 XOR conf (feito controlador)
    hdr.forro_state.v15 = configuracao[ 31:  0]; // id_dispositivo0 XOR conf (feito controlador)
}

// Faz os XORs com o id_rodada
action carregar_estado_1() {
    //Fazendo o segundo XOR com o ID_rodada
    hdr.forro_state.v5 = hdr.forro_state.v5 ^ copy32_1.get(hdr.p4dra_oper.id_rodada1);
}

// Faz os XORs com o id_rodada
action carregar_estado_2() {
    //Fazendo o terceiro XOR com o ID_rodada
    hdr.forro_state.v12 = hdr.forro_state.v12 ^ copy32_2.get(hdr.p4dra_oper.id_rodada2);
}

// Faz os XORs com o id_rodada
action carregar_estado_3_iniciar() {
    //Fazendo o quarto XOR com o ID_rodada
    hdr.forro_state.v13 = hdr.forro_state.v13 ^ copy32_3.get(hdr.p4dra_oper.id_rodada3);

    //Carregando informacoes pro estado inicial
    hdr.forro_rodada.rodada = 1;
    hdr.p4dra_oper.oper = 0x3;

    //recirculando e pulando resto do ingress e o egress
    ig_tm_md.bypass_egress = 0x1;
    ig_tm_md.ucast_egress_port=68+128;
    exit;
}

/* finalizacao */

// Somando com estado inicial, exceto primeira parte da config que vai precisar ser calculada
action finalizar_forro_0(bit<256> configuracao,
    hashword_t key0, hashword_t key1, hashword_t key2, hashword_t key3,
    hashword_t key4, hashword_t key5, hashword_t key6, hashword_t key7) {
    hdr.forro_cipher.v0  = hdr.forro_cipher.v0  + key0;
    hdr.forro_cipher.v1  = hdr.forro_cipher.v1  + key1;
    hdr.forro_cipher.v2  = hdr.forro_cipher.v2  + key2;
    hdr.forro_cipher.v3  = hdr.forro_cipher.v3  + key3;
    meta.conf0 = configuracao[255:224] ^ copy32_4.get(hdr.p4dra_oper.id_rodada0); // id_rodada0 XOR conf
    meta.conf1 = configuracao[223:192]; // id_rodada1 XOR conf (a fazer)
    hdr.forro_cipher.v6 = hdr.forro_cipher.v6 + configuracao[191:160]; // id_dispositivo0 XOR conf (feito controlador)
    hdr.forro_cipher.v7 = hdr.forro_cipher.v7 + configuracao[159:128]; // id_dispositivo1 XOR conf (feito controlador)
    hdr.forro_cipher.v8  = hdr.forro_cipher.v8  + key4;
    hdr.forro_cipher.v9  = hdr.forro_cipher.v9  + key5;
    hdr.forro_cipher.v10 = hdr.forro_cipher.v10 + key6;
    hdr.forro_cipher.v11 = hdr.forro_cipher.v11 + key7;
    meta.conf2 = configuracao[127: 96]; // id_rodada2 XOR conf (a fazer)
    meta.conf3 = configuracao[ 95: 64]; // id_rodada3 XOR conf (a fazer)
    hdr.forro_cipher.v14 = hdr.forro_cipher.v14 + configuracao[ 63: 32]; // id_dispositivo2 XOR conf (feito controlador)
    hdr.forro_cipher.v15 = hdr.forro_cipher.v15 + configuracao[ 31:  0]; // id_dispositivo3 XOR conf (feito controlador)
}

// Somando com estado inicial a primeira parte da config que foi calculada
action finalizar_forro_1() {
    hdr.forro_cipher.v4  = hdr.forro_cipher.v4  + meta.conf0;
    meta.conf1 = meta.conf1 ^ copy32_5.get(hdr.p4dra_oper.id_rodada1);
}

// Somando com estado inicial a segunda parte da config que foi calculada
action finalizar_forro_2() {
    hdr.forro_cipher.v5  = hdr.forro_cipher.v5  + meta.conf1;
    meta.conf2 = meta.conf2 ^ copy32_6.get(hdr.p4dra_oper.id_rodada2);
}

// Somando com estado inicial a terceira parte da config que foi calculada
action finalizar_forro_3() {
    hdr.forro_cipher.v12  = hdr.forro_cipher.v12  + meta.conf2;
    meta.conf3 = meta.conf3 ^ copy32_7.get(hdr.p4dra_oper.id_rodada3);
}

// Somando com estado inicial a quarta parte da config que foi calculada e enviando (pula verificacao)
action finalizar_forro_4_enviar(bit<9> port) {
    hdr.forro_cipher.v13  = hdr.forro_cipher.v13  + meta.conf3;
    ig_tm_md.ucast_egress_port = port;
    ig_tm_md.bypass_egress = 0x1;
    exit;
}

// Somando com estado inicial a quarta parte da config que foi calculada e segue verificacao
action finalizar_forro_4(bit<9> port) {
    hdr.forro_cipher.v13  = hdr.forro_cipher.v13  + meta.conf3;
    ig_tm_md.ucast_egress_port = port;
}

action verificar_prova_0(
    in hashword_t v0_cipher, in hashword_t v0_payload,
    in hashword_t v1_cipher, in hashword_t v1_payload,
    in hashword_t v2_cipher, in hashword_t v2_payload,
    in hashword_t v3_cipher, in hashword_t v3_payload) {
    meta.verificacao0 = v0_cipher ^ v0_payload;
    meta.verificacao1 = v1_cipher ^ v1_payload;
    meta.verificacao2 = v2_cipher ^ v2_payload;
    meta.verificacao3 = v3_cipher ^ v3_payload;
}

action verificar_prova_1(
    in hashword_t v0_cipher, in hashword_t v0_payload,
    in hashword_t v1_cipher, in hashword_t v1_payload,
    in hashword_t v2_cipher, in hashword_t v2_payload) {
    meta.verificacao0 = v0_cipher ^ v0_payload;
    meta.verificacao1 = v1_cipher ^ v1_payload;
    meta.verificacao2 = v2_cipher ^ v2_payload;
}