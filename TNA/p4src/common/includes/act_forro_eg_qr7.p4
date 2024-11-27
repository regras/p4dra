action e0_qr7() {
   hdr.forro_state.v14 = hdr.forro_state.v14 + hdr.forro_state.v2;
}

action e1_qr7() {
   hdr.forro_state.v9 = hdr.forro_state.v9 ^ hdr.forro_state.v14;
}

action e2_qr7() {
   hdr.forro_state.v4 = hdr.forro_state.v4 + hdr.forro_state.v9;
}

action e3_qr7() {
   hdr.forro_state.v4 = hdr.forro_state.v4[21:0] ++ hdr.forro_state.v4[31:22];
}

action e4_qr7() {
   hdr.forro_state.v3 = hdr.forro_state.v3 + hdr.forro_state.v4;
}

action e5_qr7() {
   hdr.forro_state.v2 = hdr.forro_state.v2 ^ hdr.forro_state.v3;
}

action e6_qr7() {
   hdr.forro_state.v14 = hdr.forro_state.v14 + hdr.forro_state.v2;
}

action e7_qr7() {
   hdr.forro_state.v14 = hdr.forro_state.v14[4:0] ++ hdr.forro_state.v14[31:5];
}

action e8_qr7() {
   hdr.forro_state.v9 = hdr.forro_state.v9 + hdr.forro_state.v14;
}

action e9_qr7() {
   hdr.forro_state.v4 = hdr.forro_state.v4 ^ hdr.forro_state.v9;
}

action e10_qr7() {
   hdr.forro_state.v3 = hdr.forro_state.v3 + hdr.forro_state.v4;
}

action e11_qr7() {
   hdr.forro_state.v3 = hdr.forro_state.v3[23:0] ++ hdr.forro_state.v3[31:24];
   hdr.forro_rodada.rodada = hdr.forro_rodada.rodada + 1;
}

action e11_qr7_fin() {
   hdr.forro_state.v3 = hdr.forro_state.v3[23:0] ++ hdr.forro_state.v3[31:24];
   
   hdr.p4dra_oper.oper = 0x4;
   hdr.forro_rodada.setInvalid();
}