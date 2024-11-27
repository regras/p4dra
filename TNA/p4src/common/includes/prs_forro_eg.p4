state parse_forro_rodada_state {
    pkt.extract(hdr.forro_rodada);
    pkt.extract(hdr.forro_state);
    transition accept;
}

state parse_forro_cipher_half_payload {
    pkt.extract(hdr.forro_cipher_verify);
    pkt.extract(hdr.forro_half_payload);
    transition accept;
}