#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  STATWORD ps;
  disable(ps);

  /* release the backing store with ID bs_id */
  if(bs_id < 0 || bs_id > BS_SIZE)
  {
      kprintf("Invalid Backing store ID - %d\n", bs_id);
      restore(ps);
      return SYSERR;
  }
  /* Check BS private if yes SYSERR*/
  if(bsm_tab[bs_id].bs_private == 1)
  {
      kprintf("ERROR: Backing store is private cannot release\n");
      restore(ps);
      return SYSERR;  
  }
  //free the backing store
  free_bsm(bs_id);
  restore(ps);
  return OK;

}

