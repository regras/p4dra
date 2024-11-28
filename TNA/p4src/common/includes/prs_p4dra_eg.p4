state parse_p4dra_id_dispositivo {
    pkt.extract(hdr.p4dra_id_dispositivo);
    transition select(hdr.p4dra_oper.oper) {
        0x3: parse_forro_rodada_state;
        0x4: parse_forro_cipher_half_payload;
        default: accept;
    }
}

state parse_p4dra_oper {
    pkt.extract(hdr.p4dra_oper);
    transition parse_p4dra_id_dispositivo;
}