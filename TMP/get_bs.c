#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */

    if(bsm_tab[bs_id].bs_private == 1)
    {
      kprintf("Invalid request, private backing store\n");
      return 0;
    }
    if(bs_id < 0 || bs_id > BS_SIZE || npages < 1 || npages > BS_PAGES){
      kprintf("ERROR: Invalid BS parameters. bs_id = %d, npages = %d\n", bs_id, npages);
      return 0;
    }
    if (bsm_tab[bs_id].bs_status == BSM_MAPPED)
    {  
      if(debug_flag)
        kprintf("bsm_tab[%d].bs_pages = %d, status = MAPPED\n",bs_id,bsm_tab[bs_id].bs_npages);
      return bsm_tab[bs_id].bs_npages;
    }
    if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED)
    {
      bsm_tab[bs_id].bs_status = BSM_MAPPED;
      bsm_tab[bs_id].bs_pid[currpid] = 1;
      bsm_tab[bs_id].bs_npages = npages;
    }
    if(debug_flag)
      kprintf("bsm_tab[%d].bs_pages = %d, status = UNMAPPED\n",bs_id,bsm_tab[bs_id].bs_npages);
    return npages;

}


