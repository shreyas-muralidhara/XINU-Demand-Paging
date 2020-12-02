/* initialize_paging.c - initialize_PD */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * initialize_PD - Create and Initialize Page directory
 *-------------------------------------------------------------------------
 */
SYSCALL initialize_PD(int pid){
    STATWORD ps;
    disable(ps);

    int pt_frm = 0;
    int i;

    get_frm(&pt_frm);

    frm_tab[pt_frm].fr_status = FRM_MAPPED;
    frm_tab[pt_frm].fr_pid = pid;
    frm_tab[pt_frm].fr_type = FR_DIR;
     
    pd_t *pd_entry = (FRAME0+pt_frm) * NBPG;
    
    for(i=0;i<1024;i++){
        if(i<4){
            pd_entry[i].pd_pres =1;
            pd_entry[i].pd_base = FRAME0 + i;
            //if(debug_flag)
                //kprintf("pd_entry[%d].pd_base = %d  proctab[%d].pd_base = %d  currpid = %d\n",i,pd_entry[i].pd_base,i, proctab[pid].pdbr, currpid);
        }
        else{
            pd_entry[i].pd_pres = 0;
            pd_entry[i].pd_base = 0;
        }
        pd_entry[i].pd_write = 1;
        pd_entry[i].pd_user = 0;
        pd_entry[i].pd_pwt = 0;
        pd_entry[i].pd_pcd = 0;
        pd_entry[i].pd_acc = 0;
        pd_entry[i].pd_mbz = 0;
        pd_entry[i].pd_fmb = 0;
        pd_entry[i].pd_global = 0;
        pd_entry[i].pd_avail = 0;
    }
    proctab[pid].pdbr = (unsigned long)pd_entry;
    restore(ps);
    return OK;
}