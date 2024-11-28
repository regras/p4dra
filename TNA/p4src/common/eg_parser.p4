parser EgressParser(packet_in        pkt,
    /* User */
    out my_egress_headers_t          hdr,
    out my_egress_metadata_t         meta,
    /* Intrinsic */
    out egress_intrinsic_metadata_t  eg_intr_md)
{
    #include "includes/prs_forro_eg.p4"
    #include "includes/prs_p4dra_eg.p4"

    /* This is a mandatory state, required by Tofino Architecture */
    state start {
        pkt.extract(eg_intr_md);
        transition init_metadata;
    }

    state init_metadata {
        meta.verificacao = 0x0;
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ether_type_t.P4DRA:   parse_p4dra_oper;
            default: accept;
        }
    }
}