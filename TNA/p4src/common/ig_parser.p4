parser IngressParser(packet_in        pkt,
    /* User */
    out my_ingress_headers_t          hdr,
    out my_ingress_metadata_t         meta,
    /* Intrinsic */
    out ingress_intrinsic_metadata_t  ig_intr_md)
{
    #include "includes/prs_forro_ig.p4"
    #include "includes/prs_p4dra_ig.p4"

    /* This is a mandatory state, required by Tofino Architecture */
    state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE);
        transition init_metadata;
    }

    state init_metadata {
        meta.conf0 = 0x0;
        meta.conf1 = 0x0;
        meta.conf2 = 0x0;
        meta.conf3 = 0x0;
        meta.verificacao0 = 0x0;
        meta.verificacao1 = 0x0;
        meta.verificacao2 = 0x0;
        meta.verificacao3 = 0x0;
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ether_type_t.P4DRA:          parse_p4dra_oper;
            default:                     accept;
        }
    }
}