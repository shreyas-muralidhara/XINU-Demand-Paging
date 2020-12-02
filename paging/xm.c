/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  STATWORD ps;
  disable(ps);
  if(npages < 1 || npages > BS_PAGES || source < 0 || source >= BS_SIZE || virtpage < NBPG )
  {
    kprintf("The constraints of BS not met for %d", source);
    restore(ps);
    return SYSERR;
  }
  else if(bsm_tab[source].bs_npages < npages && bsm_tab[source].bs_status == BSM_MAPPED){
    //kprintf("The requested bs - %d has been alloted to pid - %d",source, bsm_tab[source].bs_pid);
    restore(ps);
    return SYSERR;
  }
  else if(bsm_tab[source].bs_private == 1){
    kprintf("The map is used as vheap by %d",source);
    restore(ps);
    return SYSERR;
  }
  else{
    //kprintf("currpid = %d, virtpage = %d,source = %d,npages = %d\n",currpid,virtpage,source,npages);
    bsm_map(currpid,virtpage,source,npages);
    bsm_tab[source].bs_private = 0;
  }
  
  restore(ps);
  return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  STATWORD ps;
  disable(ps);
  int i;
  
  /* Check if the virtual page is within bouds*/
  if(virtpage < NBPG){
    kprintf("The virtual address is incorrect");
    restore(ps);
    return SYSERR;
  }
  
  bsm_unmap(currpid,virtpage);
  //if(debug_flag)
    //kprintf(" currpid - %d, virtpage - %d\n",currpid,virtpage);

  /* Clear the backing store for the current process */
  for(i=0;i<BS_SIZE;i++){
    if(bsm_tab[i].bs_vpno == virtpage){
      bsm_tab[i].bs_pid[currpid] = 0;
      bsm_tab[i].bs_vpno[currpid] = 0;
    }
    /* If private heap set bsm as unmapped */
    if(bsm_tab[i].bs_private == 1)
      bsm_tab[i].bs_status = BSM_UNMAPPED;
    else 
      free_bsm(i);
  }
  //kprintf("BSM xmunmap complete");
  restore(ps);
  return OK;
}
