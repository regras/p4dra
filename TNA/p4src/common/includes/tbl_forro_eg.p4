#include "act_forro_eg_qr1.p4"
#include "act_forro_eg_qr3.p4"
#include "act_forro_eg_qr5.p4"
#include "act_forro_eg_qr7.p4"

table tbl_forro_eg0 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e0_qr1;
        e0_qr3;
        e0_qr5;
        e0_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg1 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e1_qr1;
        e1_qr3;
        e1_qr5;
        e1_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg2 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e2_qr1;
        e2_qr3;
        e2_qr5;
        e2_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg3 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e3_qr1;
        e3_qr3;
        e3_qr5;
        e3_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg4 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e4_qr1;
        e4_qr3;
        e4_qr5;
        e4_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg5 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e5_qr1;
        e5_qr3;
        e5_qr5;
        e5_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg6 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e6_qr1;
        e6_qr3;
        e6_qr5;
        e6_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg7 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e7_qr1;
        e7_qr3;
        e7_qr5;
        e7_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg8 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e8_qr1;
        e8_qr3;
        e8_qr5;
        e8_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg9 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e9_qr1;
        e9_qr3;
        e9_qr5;
        e9_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg10 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e10_qr1;
        e10_qr3;
        e10_qr5;
        e10_qr7;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}

table tbl_forro_eg11 {
    key = {
        hdr.forro_rodada.rodada: exact;
    }
    actions = {
        e11_qr1;
        e11_qr3;
        e11_qr5;
        e11_qr7;
        e11_qr7_fin;
        NoAction;
    }
    size = 28;
    default_action = NoAction;
}