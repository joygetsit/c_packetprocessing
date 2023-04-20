
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
#include <nfp/mac_time.h>
#include <nfp/me.h>

#ifndef NBI
#define NBI 0
#endif

/* Timestamp Store */
struct Timestamp_S {
    uint64_t ts1;
    uint64_t ts2;
    uint64_t ts3;
    uint64_t debug_value;
};

/* Metadata rtsym storage */
struct metadata_store {
    uint64_t metadata1;
    uint64_t metadata2;
    uint64_t metadata3;
    uint64_t metadata4;
};

/* For accessing the xwrite structures as runtime symbols using nfp-rtsym */
__declspec(shared scope(global) export imem) struct Timestamp_S ts5;
__declspec(shared scope(global) export imem) struct metadata_store metadata_storex;

int
main(void)
{

    uint32_t Count_for_5sec = 197812500;
    uint32_t Count_for_30sec = 1186875000;
    uint32_t Count_for_2sec = 79125000;
    uint32_t Count_for_80microseconds = 3165;
    uint32_t Count_for_32microseconds = 1266;
    uint32_t Count_for_100milliseconds = 3956250;
    uint32_t Count_for_40milliseconds = 1582500;
    uint32_t Count_for_400milliseconds = 15825000;

    uint32_t QueueST1 = 0;
    uint32_t QueueBE1 = 16;
    __xread struct nfp_nbi_tm_queue_status QueueLevel_1, QueueLevel_2, QueueLevel_3;
    __xwrite uint64_t QueueLevel1;
    __xwrite uint64_t QueueLevel2;

    struct Timestamp_S RegTimestamp5;
    __xwrite struct Timestamp_S writeRegTimestamp5;

    for (;;) {

        RegTimestamp5.ts1 = me_tsc_read();
        RegTimestamp5.debug_value = RegTimestamp5.ts1 % Count_for_30sec ;
        if (RegTimestamp5.debug_value < Count_for_5sec) {//&& (RegTimestamp5.debug_value > 79124500)) {
            RegTimestamp5.debug_value = 65;
            RegTimestamp5.ts2 = me_tsc_read();
            RegTimestamp5.ts3 = me_tsc_read(); // weightt;
        } else {
            RegTimestamp5.debug_value = 3020;
        }

        writeRegTimestamp5 = RegTimestamp5;
        mem_write64(&writeRegTimestamp5, &ts5, sizeof(writeRegTimestamp5));

        // Read Queue levels for ST (Queue1) and BE (Queue2)
        tmq_status_read(&QueueLevel_1, 0, QueueST1, 1);
        QueueLevel1 = QueueLevel_1.queuelevel;
        mem_write_atomic(&QueueLevel1, &metadata_storex.metadata1, sizeof(QueueLevel1));

        tmq_status_read(&QueueLevel_2, 0, QueueBE1, 1);
        QueueLevel2 = QueueLevel_2.queuelevel;
        mem_add64_imm(QueueLevel_2.queuelevel, &metadata_storex.metadata2);

    }

    return 0;
}
