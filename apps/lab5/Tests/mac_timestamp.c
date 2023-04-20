/*
* Read Timestamps from MAC/ME CSRs and adjust scheduling weights.
*/

#include <nfp.h>
#include <stdint.h>

#include <nfp/mem_atomic.h>
#include <nfp/mem_bulk.h>
#include <nfp/me.h>
#include <nfp/mac_time.h>

#include <nfp/tmq.h>
#include <nfp/tm_config.h>

/* Timestamp Store */
struct Timestamp_Store {
    uint64_t ts1;
    uint64_t ts2;
    uint64_t ts3;
    uint64_t debug_value;
};

/* Scheduler Weights */
struct Scheduler_Weights {
    uint64_t weight0;
    uint64_t weight1;
};

/* For accessing the xwrite structures as runtime symbols using nfp-rtsym */
__declspec(shared scope(global) export imem) struct Timestamp_Store ts_st;
__declspec(shared scope(global) export imem) struct Scheduler_Weights sw_store;

int main (void)
{
    uint32_t Count_for_5sec = 197812500;
    uint32_t Count_for_2sec = 79125000;
    uint32_t Count_for_80microseconds = 3165;
    uint32_t Count_for_32microseconds = 1266;
    uint32_t Count_for_100milliseconds = 3956250;
    uint32_t Count_for_40milliseconds = 1582500;
    uint32_t Count_for_400milliseconds = 15825000;

    struct nfp_nbi_tm_config_spec tm_specc;

    __xrw uint64_t weightt;
    struct Timestamp_Store RegTimestamp;
    __xwrite struct Timestamp_Store writeRegTimestamp;
    struct Scheduler_Weights writeRegSchedWeight;
//    writeRegSchedWeight.weight0 = (1<<24) - 1;
//    writeRegSchedWeight.weight1 = (1<<12) - 1;
//    weightt = (1<<24) - 1;

//    tm_specc.tm_scheduler_cluster[0].in_use=1;
//    tm_specc.tm_scheduler_cluster[0].start=0;
//    tm_specc.tm_scheduler_cluster[0].end=0;
//    tm_specc.tm_scheduler_cluster[0].last=0;
//    tm_specc.tm_scheduler_cluster[0].scheduler_config.dwrrenable=1;
//    tm_specc.tm_scheduler_cluster[0].weight[0].weight=500;
//    tm_specc.tm_scheduler_cluster[0].weight[1].weight=5;

//    nfp_nbi_tm_config_schedulers(0, &tm_specc.tm_scheduler_cluster[0]);

    while(1) {
        // temp44 = 2048;
        RegTimestamp.ts1 = me_tsc_read();
        RegTimestamp.debug_value = RegTimestamp.ts1 % Count_for_80microseconds ;

        if (RegTimestamp.debug_value < Count_for_32microseconds) {//&& (RegTimestamp.debug_value > 79124500)) {
            RegTimestamp.debug_value = 65;
            // nfp_nbi_tm_config(0, my_TM_config);
            RegTimestamp.debug_value = weightt; // writeRegSchedWeight.weight0;
            // tm_config_l2_dwrr_weight_write(&weightt, 0, 0, 0, 1);
//            tm_config_l2_dwrr_weight_write(&writeRegSchedWeight.weight0, 0, 0, 0, 1);
//            tm_config_l2_dwrr_weight_write(&writeRegSchedWeight.weight1, 0, 0, 1, 1);
            // mem_write_atomic(&timestamp_start, &ts_store.ts1, sizeof(timestamp_start));
            // mem_write_atomic(&timestamp_finish, &ts_store.ts2, sizeof(timestamp_start));
            RegTimestamp.ts2 = me_tsc_read();
//            tm_specc.tm_scheduler_cluster[0].weight[0].weight=500;
//            tm_specc.tm_scheduler_cluster[0].weight[1].weight=1;
//            nfp_nbi_tm_config_schedulers(0, &tm_specc.tm_scheduler_cluster[0]);
            RegTimestamp.ts3 = me_tsc_read(); // weightt;
        } else {
            RegTimestamp.debug_value = 3020;
            RegTimestamp.debug_value = writeRegSchedWeight.weight1;
            // temp33 = writeRegSchedWeight.weight0;

            // tm_config_l2_dwrr_weight_write(&weightt, 0, 0, 0, 1);
            // temp33 = weightt;
            // tm_config_l2_dwrr_weight_write(writeRegSchedWeight.weight1, 0, 0, 0, 1);
            // tm_config_l2_dwrr_weight_write(writeRegSchedWeight.weight0, 0, 0, 1, 1);
//            tm_specc.tm_scheduler_cluster[0].weight[0].weight=2;
//            tm_specc.tm_scheduler_cluster[0].weight[1].weight=500;
//            nfp_nbi_tm_config_schedulers(0, &tm_specc.tm_scheduler_cluster[0]);
        }

        writeRegTimestamp = RegTimestamp;
        mem_write64(&writeRegTimestamp, &ts_st, sizeof(writeRegTimestamp));

        // local_csr_write(local_csr_mailbox_0, 48);
        // local_csr_write(local_csr_mailbox_1, 50);
        // local_csr_write(local_csr_mailbox_2, 52);
        // local_csr_write(local_csr_mailbox_3, 60);
    }

    // mac_time_update();
    // mac_time.mac_time_ns = 3;
    // y = heelo(6);
    // mac_time_fetch(__xread struct mac_time_state *mac_time_xfer);
    // mem_write64(&time_reg_xfer, (__mem __addr40 void *)&mac_time, sizeof(time_reg_xfer));
    // local_csr_write(local_csr_mailbox_0, mac_time_s);
    // local_csr_write(local_csr_mailbox_1, mac_time_ns);
    // __xrw uint64_t weight_y = 2^12 - 1;
    // if(ctx() == 0 && timers == 0) {
    // __gpr struct Timestamp_Store gprRegTimestamp;
    // gprRegTimestamp.ts1 = temp11;
    // gprRegTimestamp.ts2 = temp22;
    // gprRegTimestamp.ts3 = temp33;
    // gprRegTimestamp.checkk = temp44;

    // struct nfp_nbi_tm_traffic_manager_config my_TM_config;
    // struct nfp_nbi_tm_scheduler_config my_Scheduler_config;
    // // struct nfp_nbi_tm_scheduler_weight mySched_0_weight0;
    // // struct nfp_nbi_tm_scheduler_weight mySched_0_weight1;
    // struct nfp_nbi_tm_l1_l2_scheduler_weight mySched_0_weight0;
    // struct nfp_nbi_tm_l1_l2_scheduler_weight mySched_0_weight;

    // mem_write_atomic(&timestamp_keeping, &ts_store.ts3, sizeof(timestamp_start));

    return 0;
}
