#include "act_p4dra_eg.p4"

table tbl_p4dra_eg0_fin {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        verificar_prova(hdr.forro_cipher_verify.verificar, hdr.forro_half_payload.verificar); // Sempre Verifica
    }
    size = 1;
    default_action = verificar_prova(hdr.forro_cipher_verify.verificar, hdr.forro_half_payload.verificar);
}

table tbl_p4dra_eg1_fin {
    key = {
        meta.verificacao: exact;
    }
    actions = {
        NoAction; // Se Valida
        drop; // Se Invalida
    }
    size = 1;
    default_action = drop;
}
