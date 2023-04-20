
/*
This snippet is to read timestamps from special variables which when accessed give you specific values 
(here: CSR Timestamp values), capture it in a variable, then write those values back from the variable 
to mailoxes and also another value from catamaran pico engine's packet metadata's mac_timestamp (only nanosecond value)
to another mailbox so that we can read them together in one shot. This is to compare if the timestamps align well.
It does align well!!
*/

struct pkt_hdr {
    struct {
        uint32_t mac_timestamp;
        uint32_t mac_prepend;
    };
    struct eth_vlan_hdr pkt;
    struct vlan_hdr pkthdr_vlan;
    struct ip4_hdr pkthdr_ip;
    struct udp_hdr pkthdr_udp;
};

/*
 * COUNT PACKET FUNCTION
 */
void
count_packet( struct pkt_rxed *pkt_rxed,
              __mem40 struct pkt_hdr *pkt_hdr )
{
    if (pkt_rxed->pkt_hdr.pkt.tpid!=0x8100) {
        mem_incr64(&counters.no_vlan);
        mem_add32_imm(PORT_TO_CHANNEL(pkt_rxed->nbi_meta.port), &counters.port_num);
        // if (pkt_rxed->pkt_hdr.pkthdr_ip.src>=1) {
        //     mem_incr64(&counters.ip_packets);
        // }
        // if (((pkt_rxed->pkt_hdr.pkthdr_udp.sport & 0xffff)!=0)) {
        //     mem_incr64(&counters.udp_packets);
        // }
    } else {
    }
}

/*
Snippet to add sequencer number and sequence number of packet to counters, 
but there must be a better way not yet explored
*/

__xwrite uint32_t timestamp_1 = 0;
__xread uint32_t timestamp_2 = 0;

/*
 * COUNT PACKET FUNCTION
 */
void
count_packet( struct pkt_rxed *pkt_rxed,
              __mem40 struct pkt_hdr *pkt_hdr )
{
    if (pkt_rxed->pkt_hdr.pkt.tpid!=0x8100) {
        mem_incr64(&counters.no_vlan);
        mem_add32_imm(PORT_TO_CHANNEL(pkt_rxed->nbi_meta.port), &counters.port_num);
    } else {
        if ((pkt_rxed->pkt_hdr.pkt.tci & 0xfff)==2) {
            mem_incr32(&counters.vlan_2);
        } else if ((pkt_rxed->pkt_hdr.pkt.tci & 0xfff)==3) {
            mem_incr32(&counters.vlan_3);
            mem_add32_imm(PORT_TO_CHANNEL(pkt_rxed->nbi_meta.port), &counters.port_num);
//            mem_add32_imm(pkt_rxed->nbi_meta.seqr, &counters.sequencer);    //NEWLY ADDED
//            mem_add32_imm(pkt_rxed->nbi_meta.seq, &counters.sequence);      //NEWLY ADDED
            mem_add32_imm(pkt_rxed->pkt_hdr.mac_prepend, &counters.sequencer);          //NEWLY ADDED
//            mem_add32_imm(pkt_rxed->pkt_hdr.mac_timestamp, &counters.sequence);         //NEWLY ADDED
            timestamp_1 = pkt_rxed->pkt_hdr.mac_timestamp;
            mem_write_atomic(&timestamp_1, &counters.sequence, sizeof(timestamp_1));
//            pkt_rxed->pkt_hdr.pkthdr_ip.src =
        } else {
            mem_incr32(&counters.vlan_other);
        }
    }
}