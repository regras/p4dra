#include "act_p4dra_ig.p4"

table tbl_p4dra_ig0 {
    key = {
        hdr.p4dra_oper.oper: exact;
        hdr.p4dra_id_dispositivo.id_dispositivo: exact;
    }
    actions = {
        forward_request();
        carregar_estado_0();
        forward();
        drop;
    }
    size = 20002; //Num dispositivos + 2
    default_action = drop;
}

table tbl_p4dra_ig1 {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        carregar_estado_1();
        NoAction();
    }
    size = 1;
    default_action = NoAction;
}

table tbl_p4dra_ig2 {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        carregar_estado_2();
        NoAction();
    }
    size = 1;
    default_action = NoAction;
}

table tbl_p4dra_ig3 {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        carregar_estado_3_iniciar();
        NoAction();
    }
    size = 1;
    default_action = NoAction;
}

table tbl_p4dra_ig0_fin {
    key = {
        hdr.p4dra_id_dispositivo.id_dispositivo: exact;
    }
    actions = {
        finalizar_forro_0();
        NoAction();
    }
    size = 20000; //Num dispositivos
    default_action = NoAction;
}

table tbl_p4dra_ig1_fin {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        finalizar_forro_1();
        NoAction();
    }
    size = 1;
    default_action = NoAction;
}

table tbl_p4dra_ig2_fin {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        finalizar_forro_2();
        NoAction();
    }
    size = 1;
    default_action = NoAction();
}

table tbl_p4dra_ig3_fin {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        finalizar_forro_3();
        NoAction();
    }
    size = 1;
    default_action = NoAction;
}

table tbl_p4dra_ig4_fin {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        finalizar_forro_4_enviar();
        finalizar_forro_4();
        NoAction();
    }
    size = 1;
    default_action = NoAction;
}

table tbl_p4dra_ig5_fin {
    key = {
        hdr.p4dra_oper.oper: exact; //dummy
    }
    actions = {
        verificar_prova_0(hdr.forro_cipher.v15, hdr.forro_payload.v15,
                        hdr.forro_cipher.v14, hdr.forro_payload.v14,
                        hdr.forro_cipher.v13, hdr.forro_payload.v13,
                        hdr.forro_cipher.v12, hdr.forro_payload.v12);
    }
    size = 1;
    default_action = verificar_prova_0(hdr.forro_cipher.v15, hdr.forro_payload.v15,
                        hdr.forro_cipher.v14, hdr.forro_payload.v14,
                        hdr.forro_cipher.v13, hdr.forro_payload.v13,
                        hdr.forro_cipher.v12, hdr.forro_payload.v12);
}

table tbl_p4dra_ig6_fin {
    key = {
        meta.verificacao0: exact;
        meta.verificacao1: exact;
        meta.verificacao2: exact;
        meta.verificacao3: exact;
    }
    actions = {
        verificar_prova_0(hdr.forro_cipher.v11, hdr.forro_payload.v11,
                        hdr.forro_cipher.v10, hdr.forro_payload.v10,
                        hdr.forro_cipher.v9, hdr.forro_payload.v9,
                        hdr.forro_cipher.v8, hdr.forro_payload.v8); // Se Valida
        drop(); // Se Invalida
    }
    size = 1;
    default_action = drop();
}

table tbl_p4dra_ig7_fin {
    key = {
        meta.verificacao0: exact;
        meta.verificacao1: exact;
        meta.verificacao2: exact;
        meta.verificacao3: exact;
    }
    actions = {
        verificar_prova_1(hdr.forro_cipher.v7, hdr.forro_payload.v7,
                        hdr.forro_cipher.v6, hdr.forro_payload.v6,
                        hdr.forro_cipher.v5, hdr.forro_payload.v5); // Se Valida
        drop(); // Se Invalida
    }
    size = 1;
    default_action = drop();
}

table tbl_p4dra_ig8_fin {
    key = {
        meta.verificacao0: exact;
        meta.verificacao1: exact;
        meta.verificacao2: exact;
    }
    actions = {
        NoAction(); // Se Valida
        drop(); // Se Invalida
    }
    size = 1;
    default_action = drop();
}