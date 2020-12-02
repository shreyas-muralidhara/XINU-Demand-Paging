/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
    STATWORD ps;
    int i, j;
    disable(ps);
    for(i=0;i<BS_SIZE; i++){
        bsm_tab[i].bs_status = BSM_UNMAPPED; /*Initialize the backing store to unammped */
        bsm_tab[i].bs_npages = 0;   /* No of pages on the BS is initially set to 0 */
        bsm_tab[i].bs_sem = -1;
        bsm_tab[i].bs_private = 0; /* backing store is shared during initialization */
        for(j=0;j<NPROC;j++){
            bsm_tab[i].bs_pid[j] = 0;
            bsm_tab[i].bs_vpno[j] = VPAGE_BASE;
        }
    }
    restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
    STATWORD ps;
    int i;
    disable(ps);
    for(i=0; i<BS_SIZE; i++)
        /* return the bsm only if it is unmapped */
        if(bsm_tab[i].bs_status == BSM_UNMAPPED)
        {
            *avail = i;
            restore(ps);
            return OK;
        }
    restore(ps);
    return (SYSERR);
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
    STATWORD ps;
    int j;
    disable(ps);
    bsm_tab[i].bs_status = BSM_UNMAPPED; /*Initialize the backing store to unammped */
    bsm_tab[i].bs_npages = 0;   /* No of pages on the BS is reset to 0 */
    bsm_tab[i].bs_sem = 0;      //Not sure what needs to be initialized as it is not used anywhere
    bsm_tab[i].bs_private = 0; /* backing store is set to shared again, in case it was private */
    for(j=0;j<NPROC;j++){
        bsm_tab[i].bs_pid[j] = 0;
        bsm_tab[i].bs_vpno[j] = 0;
    }
    restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
    STATWORD ps;
    int i;
    unsigned int bs_start, bs_end;
    unsigned int vpageno = (unsigned int)vaddr/NBPG;
    //kprintf("vpageno = %d\n", vpageno);
    disable(ps);
    for(i=0; i<BS_SIZE;i++){
        /* Check if the process this using the backing store*/
        if(bsm_tab[i].bs_pid[pid] == 1){
            /* Compute page number and confrim if the address is in the range */
            bs_start = bsm_tab[i].bs_vpno[pid];
            
            bs_end = bsm_tab[i].bs_vpno[pid] + bsm_tab[i].bs_npages;
            //kprintf("bsm_tab[%d] = %d, vpageno = %d, bs_start=%d, bs_end=%d\n", i,bsm_tab[i].bs_vpno[pid],vpageno,bs_start,bs_end);
            if( vpageno >= bs_start && vpageno <= bs_end){
                *store = i;
                *pageth = (int) (vpageno - bs_start);
                restore(ps);
                return OK;
            }
        }
    }
    restore(ps);
    return (SYSERR);
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
    STATWORD ps;
    disable(ps);
    /* Assign the map to specified backing store source */
    bsm_tab[source].bs_status = BSM_MAPPED; /*Initialize the backing store to unammped */
    bsm_tab[source].bs_npages = npages;
    bsm_tab[source].bs_pid[pid] = 1;
    bsm_tab[source].bs_vpno[pid] = vpno;
    bsm_tab[source].bs_private = 0;
    restore(ps);
    return (OK);
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno)
{
    STATWORD ps;
    disable(ps);
    if(debug_flag)
        kprintf("pid - %d, vpno - %d NBPG*vpno = %d\n", pid, vpno, NBPG*vpno);
    /* Lookup the inverted page table - Sanity Check and Write back if dirty */
    int i, store, pageth;
    pd_t *pd_entry;
    pt_t *pt_entry;
    virt_addr_t *fault_addr;
    unsigned int pd_off, pt_off;

    for(i=0; i<NFRAMES;i++)
        if(frm_tab[i].fr_type == FR_PAGE && frm_tab[i].fr_pid == pid)
        {   
            /* Storing the faulted address */
            unsigned long virt_addr = frm_tab[i].fr_vpno*NBPG;
            fault_addr= (virt_addr_t *) &virt_addr;
            pd_off = fault_addr->pd_offset;
            pt_off = fault_addr->pt_offset;

            /* get the page directory entry  and page table entry*/
            pd_entry = (pd_t *) ((proctab[frm_tab[i].fr_pid].pdbr) + (pd_off * 4));
            pt_entry = (pt_t *) (pd_entry->pd_base * NBPG + (pt_off * 4));

            bsm_lookup(pid, vpno*NBPG, &store, &pageth);
            /* Write back bs of the Page table entry is dirty
            if(pt_entry->pt_dirty == 1){
                //if(debug_flag)
                    //kprintf("Initiating write back for the pid - %d\n", frm_tab[i].fr_pid);
                unsigned int src = (FRAME0+i)*NBPG;
                write_bs((char*) src, store, pageth);
            }*/

            bsm_tab[store].bs_status = BSM_UNMAPPED; /*Initialize the backing store to unammped */
            bsm_tab[store].bs_pid[pid] = 0;
            bsm_tab[store].bs_npages = 0;
            bsm_tab[store].bs_private = 0;
            bsm_tab[store].bs_sem = 0;
            bsm_tab[store].bs_vpno[pid] = vpno;
            
        }
    
    //kprintf("BSM Unmapped!!\n");
    restore(ps);
    return (OK);
}


