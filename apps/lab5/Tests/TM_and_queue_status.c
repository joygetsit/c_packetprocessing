
/*

*/

/*
 * SEND PACKET FUNCTION
 */
void
send_packet( struct nbi_meta_catamaran *nbi_meta,
              __mem40 struct pkt_hdr *pkt_hdr )
{
    int island, pnum, plen;
    int pkt_off;
    __gpr struct pkt_ms_info msi;
    __mem40 char *pbuf;
    uint16_t q_dst = 0;
    uint64_t queueue;

    __xread struct nfp_nbi_tm_queue_status  tmq_statusE_1,
                                            tmq_statusE,
                                            tmq_statusE_2;


    /* Write the MAC egress CMD and adjust offset and len accordingly */
    pkt_off = PKT_NBI_OFFSET + 2*MAC_PREPEND_BYTES;
    island = nbi_meta->pkt_info.isl;
    pnum   = nbi_meta->pkt_info.pnum;
    pbuf   = pkt_ctm_ptr40(island, pnum, 0);
    plen   = nbi_meta->pkt_info.len - MAC_PREPEND_BYTES;

    /* Set egress tm queue.
     * Set tm_que to mirror pkt to port on which in ingressed. */
//    q_dst  = PORT_TO_CHANNEL(nbi_meta->port) + 132;
//    q_dst = 8;

    // q_dst = PORT_TO_CHANNEL(nbi_meta->port);

    // q_dst = PORT_TO_CHANNEL(nbi_meta->port) ? 0 : 128;

//    queueue = nbi_meta->port;
//    if (queueue == 3 || queueue == 19) {
//        queueue = (queueue==19) ? 3 : 19;
//    } else {
//        queueue = (queueue==16) ? 0 : 16;
//    }
//    q_dst = PORT_TO_CHANNEL(queueue);

    if ((pkt_hdr->pkt.tci & 0xfff)==2) {
        q_dst = 0;
    } else if ((pkt_hdr->pkt.tci & 0xfff)==3) {
        q_dst = 1;
    } else {
        q_dst = 8;
    }


    pkt_mac_egress_cmd_write(pbuf, pkt_off, 1, 1); // Write data to make the packet MAC egress generate L3 and L4 checksums

    msi = pkt_msd_write(pbuf, pkt_off - MAC_PREPEND_BYTES); // Write a packet modification script of NULL

    pkt_nbi_send(island,
                 pnum,
                 &msi,
                 plen,
                 NBI,
                 q_dst,
                 nbi_meta->seqr,
                 nbi_meta->seq,
                 PKT_CTM_SIZE_256);

    uint32_t QueueNo0 = 0;
    uint32_t QueueNo1 = 1;
    tmq_status_read(&tmq_statusE, 0, QueueNo0, 1);
    mem_add64_imm(tmq_statusE.queuelevel, &counters.qlevelE);
    tmq_status_read(&tmq_statusE_1, 0, QueueNo0, 1);
    mem_add64_imm(tmq_statusE_1.queuelevel, &counters.qlevele_1);
    tmq_status_read(&tmq_statusE_2, 0, QueueNo1, 1);
    mem_add64_imm(tmq_statusE_2.queuelevel, &counters.qlevele_2);
}


int
main(void)
{
    struct pkt_rxed pkt_rxed; /* The packet header received by the thread */
    __mem40 struct pkt_hdr *pkt_hdr;    /* The packet in the CTM */

    /* Try your code for QoS TM config */
/*    struct nfp_nbi_tm_traffic_manager_config tm_config_1;
    __xwrite struct nfp_nbi_tm_queue_cluster tm_q_cluster_1;
    int return_value_tm_config;

    tm_config_1.sequencer0enable=0;
    tm_config_1.schedulerenable=1;
    tm_config_1.shaperenable=0;
    tm_config_1.channellevelselect=1;
    tm_config_1.numsequencers=0;
    tm_config_1.l1inputselect=1;

    tm_q_cluster_1.in_use=1;
    tm_q_cluster_1.start=0;
    tm_q_cluster_1.end=1;
    tm_q_cluster_1.last=1;
    tm_q_cluster_1.queue_config.queueenable=1;
    tm_q_cluster_1.queue_config.dropraterangeselect=0;
    tm_q_cluster_1.queue_config.queuesize=4;
    tm_q_cluster_1.queue_config.dmapacketthreshold=0;
    tm_q_cluster_1.queue_config.redenable=0;
    tm_q_cluster_1.queue_config.dropenable=1;*/

//    return_value_tm_config = nfp_nbi_tm_config(0, tm_config_1);
}
