state parse_forro_rodada_state {
    pkt.extract(hdr.forro_rodada);
    pkt.extract(hdr.forro_state);
    transition accept;
}

state parse_forro_cipher_payload {
    pkt.extract(hdr.forro_cipher);
    pkt.extract(hdr.forro_payload);
    transition accept;
}