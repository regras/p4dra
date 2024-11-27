/* -*- P4_16 -*- */

/*************************************************************************
 ****************  E G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  H E A D E R S  ************************/

struct my_egress_headers_t {
    ethernet_h                      ethernet;
    p4dra_oper_t                    p4dra_oper;
    p4dra_id_dispositivo_t          p4dra_id_dispositivo;
    forro_rodada_t                  forro_rodada;
    forro_state_t                   forro_state;
    forro_state_cipher_t            forro_cipher;
    forro_state_cipher_verify_t     forro_cipher_verify;
    forro_payload_t                 forro_payload;
    forro_half_payload_t            forro_half_payload;
}

    /********  G L O B A L   E G R E S S   M E T A D A T A  *********/

struct my_egress_metadata_t {
    bit<160> verificacao;
}

    /***********************  P A R S E R  **************************/

   #include "eg_parser.p4"

    /***************** M A T C H - A C T I O N  *********************/

control Egress(
    /* User */
    inout my_egress_headers_t                          hdr,
    inout my_egress_metadata_t                         meta,
    /* Intrinsic */
    in    egress_intrinsic_metadata_t                  eg_intr_md,
    in    egress_intrinsic_metadata_from_parser_t      eg_prsr_md,
    inout egress_intrinsic_metadata_for_deparser_t     eg_dprsr_md,
    inout egress_intrinsic_metadata_for_output_port_t  eg_oport_md)
{

    // Mark the package to be dropped at the end of ingress and skip the rest of the pipeline
    action drop(){
        eg_dprsr_md.drop_ctl = 1;
        exit;
    }

    //include de tabelas - cada arquivo inclue suas actions
    #include "includes/tbl_forro_eg.p4"
    #include "includes/tbl_p4dra_eg.p4"

    apply {
        if (hdr.p4dra_oper.oper == 0x3) {
            tbl_forro_eg0.apply();
            tbl_forro_eg1.apply();
            tbl_forro_eg2.apply();
            tbl_forro_eg3.apply();
            tbl_forro_eg4.apply();
            tbl_forro_eg5.apply();
            tbl_forro_eg6.apply();
            tbl_forro_eg7.apply();
            tbl_forro_eg8.apply();
            tbl_forro_eg9.apply();
            tbl_forro_eg10.apply();
            tbl_forro_eg11.apply();
        } else {
            tbl_p4dra_eg0_fin.apply();
            tbl_p4dra_eg1_fin.apply();
        }
    }
}

    /*********************  D E P A R S E R  ************************/

control EgressDeparser(packet_out pkt,
    /* User */
    inout my_egress_headers_t                       hdr,
    in    my_egress_metadata_t                      meta,
    /* Intrinsic */
    in    egress_intrinsic_metadata_for_deparser_t  eg_dprsr_md)
{
    apply {
        pkt.emit(hdr);
    }
}

