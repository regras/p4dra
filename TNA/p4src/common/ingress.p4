/* -*- P4_16 -*- */

/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  H E A D E R S  ************************/

struct my_ingress_headers_t {
    ethernet_h              ethernet;
    p4dra_oper_t            p4dra_oper;
    p4dra_id_dispositivo_t  p4dra_id_dispositivo;
    forro_rodada_t          forro_rodada;
    forro_state_t           forro_state;
    forro_state_cipher_t    forro_cipher;
    forro_payload_t         forro_payload;
}

    /******  G L O B A L   I N G R E S S   M E T A D A T A  *********/

struct my_ingress_metadata_t {
    hashword_t conf0;
    hashword_t conf1;
    hashword_t conf2;
    hashword_t conf3;
    hashword_t verificacao0;
    hashword_t verificacao1;
    hashword_t verificacao2;
    hashword_t verificacao3;
}

    /***********************  P A R S E R  **************************/
    #include "ig_parser.p4"

    /***************** M A T C H - A C T I O N  *********************/

control Ingress(
    /* User */
    inout my_ingress_headers_t                       hdr,
    inout my_ingress_metadata_t                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_t               ig_intr_md,
    in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md)
{
	Hash<hashword_t>(HashAlgorithm_t.IDENTITY) copy32_0;
	Hash<hashword_t>(HashAlgorithm_t.IDENTITY) copy32_1;
    Hash<hashword_t>(HashAlgorithm_t.IDENTITY) copy32_2;
	Hash<hashword_t>(HashAlgorithm_t.IDENTITY) copy32_3;
    Hash<hashword_t>(HashAlgorithm_t.IDENTITY) copy32_4;
	Hash<hashword_t>(HashAlgorithm_t.IDENTITY) copy32_5;
    Hash<hashword_t>(HashAlgorithm_t.IDENTITY) copy32_6;
	Hash<hashword_t>(HashAlgorithm_t.IDENTITY) copy32_7;

    // Mark the package to be dropped at the end of ingress and skip the rest of the pipeline
    action drop(){
        ig_dprsr_md.drop_ctl = 1;
        exit;
    }

    //include de tabelas - cada arquivo inclui suas actions utilizadas
    #include "includes/tbl_forro_ig.p4"
    #include "includes/tbl_p4dra_ig.p4"

    apply {
        if (hdr.p4dra_oper.oper == 0x3) {
            tbl_forro_ig0.apply();
            tbl_forro_ig1.apply();
            tbl_forro_ig2.apply();
            tbl_forro_ig3.apply();
            tbl_forro_ig4.apply();
            tbl_forro_ig5.apply();
            tbl_forro_ig6.apply();
            tbl_forro_ig7.apply();
            tbl_forro_ig8.apply();
            tbl_forro_ig9.apply();
            tbl_forro_ig10.apply();
            tbl_forro_ig11.apply();
        } else {
            if (hdr.p4dra_oper.oper == 0x4) {
                tbl_p4dra_ig0_fin.apply();
                tbl_p4dra_ig1_fin.apply();
                tbl_p4dra_ig2_fin.apply();
                tbl_p4dra_ig3_fin.apply();
                tbl_p4dra_ig4_fin.apply();
                tbl_p4dra_ig5_fin.apply();
                tbl_p4dra_ig6_fin.apply();
                tbl_p4dra_ig7_fin.apply();
                tbl_p4dra_ig8_fin.apply();
            } else {
                tbl_p4dra_ig0.apply();
                tbl_p4dra_ig1.apply();
                tbl_p4dra_ig2.apply();
                tbl_p4dra_ig3.apply();
            }    
        }
    }

}


    /*********************  D E P A R S E R  ************************/

control IngressDeparser(packet_out pkt,
    /* User */
    inout my_ingress_headers_t                       hdr,
    in    my_ingress_metadata_t                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md)
{
    apply {
        pkt.emit(hdr);
    }
}

