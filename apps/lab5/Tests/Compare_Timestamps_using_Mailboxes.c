
/*
This snippet is to read timestamps from special variables which when accessed give you specific values 
(here: CSR Timestamp values), capture it in a variable, then write those values back from the variable 
to mailoxes and also another value from catamaran pico engine's packet metadata's mac_timestamp (only nanosecond value)
to another mailbox so that we can read them together in one shot. This is to compare if the timestamps align well.
It does align well!!
*/


/*
 * RECEIVE PACKET FUNCTION
 */
__mem40 struct pkt_hdr *
receive_packet( struct pkt_rxed *pkt_rxed, size_t size )
{
    __xread struct pkt_rxed pkt_rxed_in;
    int island, pnum;
    int pkt_off;
    __mem40 struct pkt_hdr *pkt_hdr;
    uint32_t me_tstamp_hi;
    uint32_t me_tstamp_lo;

    me_tstamp_hi = local_csr_read(local_csr_timestamp_high);
    me_tstamp_lo = local_csr_read(local_csr_timestamp_low);
    pkt_nbi_recv(&pkt_rxed_in, sizeof(pkt_rxed->nbi_meta));
    pkt_rxed->nbi_meta = pkt_rxed_in.nbi_meta;

    pkt_off  = PKT_NBI_OFFSET;
    island   = pkt_rxed->nbi_meta.pkt_info.isl;
    pnum     = pkt_rxed->nbi_meta.pkt_info.pnum;
    pkt_hdr  = pkt_ctm_ptr40(island, pnum, pkt_off);

    /* use the following command to read all the mailbox at once */
    // nfp-reg mecsr:i3{2..6}.me{0..9}.Mailbox{0..2}
    local_csr_write(local_csr_mailbox_0, pkt_rxed->pkt_hdr.mac_timestamp);
    local_csr_write(local_csr_mailbox_1, me_tstamp_lo);
    local_csr_write(local_csr_mailbox_2, me_tstamp_hi);
    mem_read32(&(pkt_rxed_in.pkt_hdr), pkt_hdr, sizeof(pkt_rxed_in.pkt_hdr));
    pkt_rxed->pkt_hdr = pkt_rxed_in.pkt_hdr;

    return pkt_hdr;
}