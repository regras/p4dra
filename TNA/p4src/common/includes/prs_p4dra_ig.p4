state parse_p4dra_id_dispositivo {
    pkt.extract(hdr.p4dra_id_dispositivo);
    transition select(hdr.p4dra_oper.oper) {
        0x2: accept;
        0x3: parse_forro_rodada_state;
        0x4: parse_forro_cipher_payload;
        default: accept;
    }
}

state parse_p4dra_oper {
    pkt.extract(hdr.p4dra_oper);
    transition select(hdr.p4dra_oper.oper) {
        0x0: accept;
        0x1: accept;
        0x2: parse_p4dra_id_dispositivo;
        0x3: parse_p4dra_id_dispositivo;
        0x4: parse_p4dra_id_dispositivo;
        0x5: accept;
        default: accept;
    }
}