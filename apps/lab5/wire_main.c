/*
 * Copyright (C) 2015-2018,  Netronome Systems, Inc.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          apps/wire/wire_main.c
 * @brief         Main program for ME cut-through wire app
 *
 * This application acts as a "wire" which by default will return packets
 * back out on the port they were received on.
 *
 *
 * It also doubles up as test code for the header extract
 * code. The packet headers are extracted and counts are maintained
 * for different types of packets.
 */

/* Flowenv */
#include <nfp.h>
#include <stdint.h>

#include <pkt/pkt.h>
#include <net/eth.h>

#include <net/ip.h>
#include <net/udp.h>

#include <nfp/mem_atomic.h>
#include <nfp/mem_bulk.h>

#include <nfp6000/nfp_nbi_tm.h>
#include <nfp/tmq.h>
#include <nfp/tm_config.h>
//#include <nfp/mac_time.h>
#include <nfp/me.h>

/*
 * Mapping between channel and TM queue
 */
#ifndef NBI
#define NBI 0
#endif

#ifndef PKT_NBI_OFFSET
#define PKT_NBI_OFFSET 128
#endif

/* DEBUG MACROS */

__volatile __export __emem uint32_t debug[8192*64];
__volatile __export __emem uint32_t debug_idx;
#define DEBUG(_a, _b, _c, _d) do { \
    __xrw uint32_t _idx_val = 4; \
    __xwrite uint32_t _dvals[4]; \
    mem_test_add(&_idx_val, \
            (__mem40 void *)&debug_idx, sizeof(_idx_val)); \
    _dvals[0] = _a; \
    _dvals[1] = _b; \
    _dvals[2] = _c; \
    _dvals[3] = _d; \
    mem_write_atomic(_dvals, (__mem40 void *)\
                    (debug + (_idx_val % (1024 * 64))), sizeof(_dvals)); \
    } while(0)


/* Counters */
struct counters {
    uint64_t no_vlan;
    uint64_t ip_packets;
    uint64_t udp_packets;
    uint32_t vlan_2;
    uint32_t vlan_3;
    uint32_t vlan_other;
    uint32_t extra;
};

struct statistics {
    uint64_t no_vlan;
    uint64_t vlan_2;
    uint64_t vlan_3;
    uint64_t vlan_other;
};



/**
 * Joy's custom metadata structure used for debugging with nfp-rtsym
 */

struct CustomData {
    uint8_t custom1;
    uint8_t custom2;
    uint16_t custom3;
    uint32_t custom4;
    uint64_t custom5;
    uint64_t custom6;
    uint64_t custom7;
};

__packed struct metadata {
    uint64_t timestamp1;                      /** timestamp1 */
    uint64_t timestamp2;                      /** timestamp */
    uint64_t timestamp3;                      /** timestamp */
    uint64_t timestamp4;                      /** timestamp */
    uint64_t timestamp5;                      /** timestamp */    
};

struct Timestamp_Store {
    uint64_t ts1;
    uint64_t ts2;
    uint64_t ts3;
    uint64_t debug_value;
};

// __declspec(shared ctm) is one copy shared by all threads in an ME, in CTM
// __declspec(shared export ctm) is one copy shared by all MEs in an island in CTM (CTM default scope for 'export' of island)
// __declspec(shared export imem) is one copy shared by all MEs on the chip in IMU (IMU default scope for 'export' of global)
__declspec(shared scope(island) export cls) struct statistics stats;
__declspec(shared scope(global) export imem) struct counters counters;

__declspec(shared scope(global) export imem) struct CustomData customData;


struct pkt_hdr {
    struct {
        uint32_t mac_timestamp;
        uint32_t mac_prepend;
    };
    struct eth_vlan_hdr pkt;
    struct metadata metadata_m;
    struct ip4_hdr ip_pkt;
    struct udp_hdr udp_pkt;
};

struct pkt_rxed {
    struct nbi_meta_catamaran nbi_meta;
    struct pkt_hdr            pkt_hdr;
};

__intrinsic void
stats_packet( struct pkt_rxed *pkt_rxed,
              __mem40 struct pkt_hdr *pkt_hdr )
{
    __xwrite uint32_t bytes_to_add;
    SIGNAL   sig;
    int address;

    bytes_to_add = pkt_rxed->nbi_meta.pkt_info.len;

    if (pkt_rxed->pkt_hdr.pkt.tpid!=0x8100) {
        address = (uint32_t) &(stats.no_vlan);
    } else {
        if ((pkt_rxed->pkt_hdr.pkt.tci & 0xfff)==2) {
            address = (uint32_t) &(stats.vlan_2);
        } else if ((pkt_rxed->pkt_hdr.pkt.tci & 0xfff)==3) {
            address = (uint32_t) &(stats.vlan_3);
        } else {
            address = (uint32_t) &(stats.vlan_other);
        }
    }

    __asm {
        cls[statistic, bytes_to_add, address, 0, 1], ctx_swap[sig]
    }
}

__mem40 struct pkt_hdr *
receive_packet( struct pkt_rxed *pkt_rxed,
                size_t size )
{
    __xread struct pkt_rxed pkt_rxed_in;
    int island, pnum;
    int pkt_off;
    __mem40 struct pkt_hdr *pkt_hdr;

    pkt_nbi_recv(&pkt_rxed_in, sizeof(pkt_rxed->nbi_meta));
    pkt_rxed->nbi_meta = pkt_rxed_in.nbi_meta;

    pkt_off  = PKT_NBI_OFFSET;
    island   = pkt_rxed->nbi_meta.pkt_info.isl;
    pnum     = pkt_rxed->nbi_meta.pkt_info.pnum;
    pkt_hdr  = pkt_ctm_ptr40(island, pnum, pkt_off);

    mem_read32(&(pkt_rxed_in.pkt_hdr), pkt_hdr, sizeof(pkt_rxed_in.pkt_hdr));
    pkt_rxed->pkt_hdr = pkt_rxed_in.pkt_hdr;

    return pkt_hdr;}

void
rewrite_packet( struct pkt_rxed *pkt_rxed,
                __mem40 struct pkt_hdr *pkt_hdr )
{
    int vlan;

    uint64_t test_me_timestamp = 0;
    static uint64_t QueueLevel0_0 = 0;
    uint64_t QueueLevel1_1 = 0;

    vlan = 0;
    if (pkt_rxed->pkt_hdr.pkt.tpid==0x8100) {
        vlan = pkt_rxed->pkt_hdr.pkt.tci & 0xfff;
        // if ((vlan==2) || (vlan==3)) {
        //     // pkt_hdr->pkt.tci = pkt_rxed->pkt_hdr.pkt.tci ^ 1;
        //     pkt_hdr->pkt.tci = 0x202f;
        // }
        if (vlan==2) {
            pkt_hdr->pkt.tci = pkt_hdr->pkt.tci & 0x1fff; // Priority 0
            pkt_hdr->pkt.tci = pkt_hdr->pkt.tci; // + 5 (For debug);
            //****** Checking static variable ******//
            // QueueLevel0_0 = 0;
            QueueLevel0_0 += local_csr_read(local_csr_mailbox_0);
            QueueLevel1_1 += local_csr_read(local_csr_mailbox_1);
            mem_add64_imm(QueueLevel0_0, &customData.custom5);
            mem_add64_imm(QueueLevel1_1, &customData.custom7);
            //***********************//
        } else if (vlan==3) {
            pkt_hdr->pkt.tci = (pkt_hdr->pkt.tci & 0x1fff) | (1 << 13); // Priority 1
            pkt_hdr->pkt.tci = pkt_hdr->pkt.tci + 15;        
        } else if (vlan==4) {
            pkt_hdr->pkt.tci = (pkt_hdr->pkt.tci & 0x1fff) | (1 << 14); // Priority 2
        }
    }

    /***
    * Joy - Add NFP ME to custom metadata header in packet
    ***/
    test_me_timestamp = me_tsc_read(); // 0xf1f1f9f8f7f6f5f2; //
    // mem_write_atomic(&QueueLevel0, &customData_2.qlevel_0, sizeof(QueueLevel0));
    // test_me_timestamp = 0xf6f9f7f5f3f1; // local_csr_read(local_csr_mailbox_1);
    if ((pkt_rxed->pkt_hdr.pkt.tci & 0xfff)==2) {
        pkt_hdr->metadata_m.timestamp3 = test_me_timestamp; //258; //test_me_timestamp; //me_tsc_read();
        pkt_hdr->metadata_m.timestamp4 = test_me_timestamp;
        pkt_hdr->metadata_m.timestamp5 = test_me_timestamp;
        // test_me_timestamp = me_tsc_read();
        // pkt_hdr->metadata_m.timestamp3 = test_me_timestamp; //me_tsc_read();
        // pkt_hdr->metadata_m.timestamp4 = test_me_timestamp;
    } else if ((pkt_rxed->pkt_hdr.pkt.tci & 0xfff)==3) {
        pkt_hdr->pkt.tci = pkt_hdr->pkt.tci + 15;
    }
}

void
count_packet( struct pkt_rxed *pkt_rxed,
              __mem40 struct pkt_hdr *pkt_hdr )
{
    if (pkt_rxed->pkt_hdr.pkt.tpid!=0x8100) {
        mem_incr64(&counters.no_vlan);
    } else {
        if ((pkt_rxed->pkt_hdr.pkt.tci & 0xfff)==2) {
            mem_incr32(&counters.vlan_2);
            mem_add32_imm(PORT_TO_CHANNEL(pkt_rxed->nbi_meta.port), &customData.custom1); // Add Port Number
        } else if ((pkt_rxed->pkt_hdr.pkt.tci & 0xfff)==3) {
            mem_incr32(&counters.vlan_3);
        } else {
            mem_incr32(&counters.vlan_other);
        }
    }
}

void
send_packet( struct nbi_meta_catamaran *nbi_meta,
              __mem40 struct pkt_hdr *pkt_hdr )
{
    int island, pnum, plen;
    int pkt_off;
    __gpr struct pkt_ms_info msi;
    __mem40 char *pbuf;
    uint16_t q_dst = 0;
    uint8_t channel_dst = 0;

    uint32_t Queue0 = 5;
    uint32_t Queue1 = 1;
    uint32_t Queue2 = 24;
    __xread struct nfp_nbi_tm_queue_status QueueDataStructure[3];
    __xwrite uint64_t QueueLevel0;
    __xwrite uint64_t QueueLevel1;
    // uint64_t QueueLevel0_0 = 0;
    // uint64_t QueueLevel1_1 = 0;
    uint64_t counter_debug = 0;

    /*******-------------------------------------------------------------*****/
    /** TAS-CT code */
    // struct Timestamp_Store RegTimestamp1;

    // uint32_t Count_for_80microseconds = 3165;
    // uint32_t Count_for_100milliseconds = 3956250;

    // RegTimestamp1.ts1 = me_tsc_read();
    // RegTimestamp1.debug_value = RegTimestamp1.ts1 % (Count_for_100milliseconds*4) ;
    // Queue0 = local_csr_read(local_csr_mailbox_0);
    // if (RegTimestamp1.debug_value < Count_for_100milliseconds*2/5) {
    //     RegTimestamp1.debug_value = 65;
    //     Queue1 = local_csr_read(local_csr_mailbox_1);
    //     Queue1 = 16;
    //     RegTimestamp1.ts2 = me_tsc_read();
    //     RegTimestamp1.ts3 = me_tsc_read();
    // } else {
    //     RegTimestamp1.debug_value = 3020;
    //     Queue1 = local_csr_read(local_csr_mailbox_2);
    // }
    /*******-------------------------------------------------------------*****/

    /* Write the MAC egress CMD and adjust offset and len accordingly */
    pkt_off = PKT_NBI_OFFSET + 2*MAC_PREPEND_BYTES;
    island = nbi_meta->pkt_info.isl;
    pnum   = nbi_meta->pkt_info.pnum;
    pbuf   = pkt_ctm_ptr40(island, pnum, 0);
    plen   = nbi_meta->pkt_info.len - MAC_PREPEND_BYTES;

    channel_dst = nbi_meta->port;

    /* Set egress tm queue.
     * Set tm_que to mirror pkt to port on which in ingressed. */
    /** Select 1 of the options below **/
    /*******-------------------------------------------------------------*****/

    // q_dst = PORT_TO_CHANNEL(channel_dst);
    // q_dst = 0;
    /*** Set egress queue to the other port ***/
    q_dst = PORT_TO_CHANNEL(channel_dst) ? 0 : 128;

    /*** Select egress queue/channel to the other port such that ping and arp etc. can flow. ***/
    /** Ping and arp use the 4th queue of the channel, we don't know the reason yet **/
    // if (channel_dst == 3 || channel_dst == 19) {
    //     channel_dst = (channel_dst==19) ? 3 : 19;
    // } else {
    //     channel_dst = (channel_dst==16) ? 0 : 16;
    // }
    // q_dst = PORT_TO_CHANNEL(channel_dst);

    // if ((pkt_hdr->pkt.tci & 0xfff)==2) {
    //     q_dst = Queue0;
    // } else if ((pkt_hdr->pkt.tci & 0xfff)==3) {
    //     q_dst = Queue1;
    // } else {
    //     q_dst = Queue2;
    // }
    /*******-------------------------------------------------------------*****/

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

    /*******-------------------------------------------------------------*****/
    /** Queue Occupancy Level function **/
    // tmq_status_read(&QueueDataStructure[0], 0, Queue0, 1);
    // QueueLevel0 = QueueDataStructure[0].queuelevel;
    // QueueLevel0_0 = QueueDataStructure[0].queuelevel;
    // // mem_write_atomic(&QueueLevel0, &customData.custom5, sizeof(QueueLevel0));
    // mem_add64_imm(QueueLevel0_0, &customData.custom5);

    // tmq_status_read(&QueueDataStructure[1], 0, Queue1, 1);
    // QueueLevel1 = QueueDataStructure[1].queuelevel;
    // // mem_write_atomic(&QueueLevel1, &customData.custom6, sizeof(QueueLevel1));
    // mem_add64_imm(QueueLevel1_1, &customData.custom6);
    // counter_debug += 1;
    // mem_add64_imm(counter_debug, &customData.custom7);
}

    // uint64_t tmp_qlevel_1 = 0;
    // uint64_t avg_qlevel_1 = 0;
    // tmp_qlevel1 = QueueLevel_1.queuelevel;
    // for (i=1; i<n; i++) {
    // avg_qlevel_1 = (tmp_qlevel_1/i);
    // return avg_qlevel_1;
    // }
    // mem_add64_imm(avg_qlevel_1, &customData_2.qlevel_1);
    /*******-------------------------------------------------------------*****/


int
main(void)
{
    struct pkt_rxed pkt_rxed; /* The packet header received by the thread */
    __mem40 struct pkt_hdr *pkt_hdr;    /* The packet in the CTM */

    /*
     * Endless loop
     *
     * 1. Get a packet from the wire (NBI)
     * 2. Rewrite the packet ready for retransmission
     * 3. Count the packet as required
     * 4. Do statistics on the packet
     * 5. Send the packet back to the wire (NBI)
     */
    for (;;) {
        /* Receive a packet */
        pkt_hdr = receive_packet(&pkt_rxed, sizeof(pkt_rxed));

        /* Rewrite the packet */
        rewrite_packet(&pkt_rxed, pkt_hdr);

        /* Count the packet */
        count_packet(&pkt_rxed, pkt_hdr);

        /* Do stats on the packet */
        //stats_packet(&pkt_rxed, pkt_hdr);

        /* Send the packet */
        send_packet(&pkt_rxed.nbi_meta, pkt_hdr);

    }

    return 0;
}

/* -*-  Mode:C; c-basic-offset:4; tab-width:4 -*- */