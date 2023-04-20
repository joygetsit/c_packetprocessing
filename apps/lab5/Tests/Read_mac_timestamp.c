/*
* READ MAC TIMESTAMPS from MAC-CSR
*/

#include <nfp.h>
#include <stdint.h>

// #include <pkt/pkt.h>
// #include <net/eth.h>
#include <nfp/mem_atomic.h>
#include <nfp/mem_bulk.h>
#include <nfp/me.h>
// #include <nfp/mac_time.h>

#include <nfp6000/nfp_nbi_tm.h>
#include <nfp/tmq.h>
#include <nfp/tm_config.h>

/* Counterstestt */
struct counters22 {
    uint64_t check1;
    uint64_t check2;
};

// volatile __export __imem struct mac_time_state mac_time;
__declspec(shared scope(global) export imem) struct counters22 counters2;
// volatile export __mem40 uint32_t timers = 0;

int main (void)
{
    // __gpr uint64_t ts_temp;
    __xwrite uint64_t ts_temp = 0;
    __gpr uint64_t ts_temp_read = 0;
    __xwrite uint64_t ts_temp2 = 0;

    struct nfp_nbi_tm_traffic_manager_config tm_config_1;

    // if(ctx() == 0 && timers == 0) {
    while(1) {
        ts_temp = me_tsc_read();
        ts_temp_read = me_tsc_read();          
        mem_write_atomic(&ts_temp, &counters2.check1, sizeof(ts_temp));
        
        ts_temp2 = ts_temp_read;       
        mem_write_atomic(&ts_temp2, &counters2.check2, sizeof(ts_temp2));
                                
        // if(ts_temp_read/20000000 > 0) {
                                        
        //     tm_config_1.channellevelselect=tm_config_1.channellevelselect^1;
        //     // tm_config_1.channellevelselect=1;
        //     nfp_nbi_tm_config(0, tm_config_1);
        // }
    }

    // mac_time_update();
    // mac_time.mac_time_ns = 3;
    // y = heelo(6);
    // mac_time_fetch(__xread struct mac_time_state *mac_time_xfer);
    // mem_write64(&time_reg_xfer, (__mem __addr40 void *)&mac_time, sizeof(time_reg_xfer));
    // local_csr_write(local_csr_mailbox_0, mac_time_s);
    // local_csr_write(local_csr_mailbox_1, mac_time_ns);

    return 0;
}