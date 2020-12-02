/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include<proc.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
  STATWORD ps;
  disable(ps);
  //if(debug_flag)
    //kprintf("Interrupt begin\n");
  unsigned long virt_addr, pdbr;  
  unsigned int vp, pd_off, pt_off, pg_off, pg_addr;
  int i, store, pageth, pt_frm, pg_frm;
  pd_t *pd_entry;
  pt_t *pt_entry, *pt_ptr;
  virt_addr_t *fault_addr;

  /* Storing the faulted address */
  virt_addr = read_cr2();
  fault_addr= (virt_addr_t *) &virt_addr;

  /* Virtual Page Directory, Page Table, Page offset and virtual page*/
  pd_off = fault_addr->pd_offset;
  pt_off = fault_addr->pt_offset;
  pg_off = fault_addr->pg_offset;
  vp = virt_addr/NBPG; 

  if (debug_flag){
    //kprintf(" pd_off = %d     pt_off = %d    pg_off = %d\n  ", virt_addr,fault_addr, pg_off);
    kprintf("vp= %d, virt_arr=%d, NBPG = %d, currpid=%d store=%d pageth= %d \n", vp, virt_addr,NBPG, currpid,store,pageth);
  }

  /* Check for legal address, else kill the process*/
  if(bsm_lookup(currpid,virt_addr, &store, &pageth) == SYSERR)
  {
    if(debug_flag)
      kprintf("bsm lookup failed killed the process\n");
    kill(currpid);
    restore(ps);
    return SYSERR;
  }
  
  /* get the page directory entry */
  pd_entry = (pd_t *) ((proctab[currpid].pdbr) + (pd_off * 4));
  if (debug_flag){
    kprintf("proctab[%d].pdbr =  %d, pd_entry = %d    pd_off = %d\n",currpid,proctab[currpid].pdbr, pd_entry,pd_off );
  }

  /* If page directory entry doesn't exist create new Page table and update directory*/
  if(pd_entry->pd_pres == 0) {
    
    /* Create the page table*/
    pt_frm = -1;
    get_frm(&pt_frm);
    
    /* If page frame cannot be created return SYSERR*/
    if(pt_frm == SYSERR){
      
      kprintf("Page table frame cannot be created, as no free entry is available.\n");
      kill(currpid);
      restore(ps);
      return SYSERR;
    }
    /* Initialize all the frames in table to zero */
    pt_ptr = (pt_t *) ((FRAME0 + pt_frm)*NBPG);
    for(i=0;i<NFRAMES;i++){
      pt_ptr[i].pt_pres = 0;
      pt_ptr[i].pt_write = 1;
      pt_ptr[i].pt_user = 0;
      pt_ptr[i].pt_pwt = 0;
      pt_ptr[i].pt_pcd = 0;
      pt_ptr[i].pt_acc = 0;
      pt_ptr[i].pt_dirty = 0;
      pt_ptr[i].pt_mbz = 0;
      pt_ptr[i].pt_global = 0;
      pt_ptr[i].pt_avail = 0;
      pt_ptr[i].pt_base = 0;
    }

    /* update the global page table */
    frm_tab[pt_frm].fr_status = FRM_MAPPED;
    frm_tab[pt_frm].fr_pid = currpid;
    frm_tab[pt_frm].fr_refcnt = 0;
    frm_tab[pt_frm].fr_type = FR_TBL;
    
    /* update the page directory entry */
    pd_entry->pd_pres = 1;
    pd_entry->pd_write = 1;
    pd_entry->pd_base = FRAME0 + pt_frm;
    
    if(debug_flag)
      kprintf("Global page table entry and Page directory entry was updated\n");
  }

  /* get the page table entry */
  pt_entry = (pt_t *) (pd_entry->pd_base * NBPG + (pt_off * 4));
  
  if(pt_entry->pt_pres == 0){
    /* Create the page table, if not created return SYSERR*/
    pg_frm = -1;
    if(get_frm(&pg_frm) == SYSERR){
      kprintf("pg_frame cannot be created, as no free entry is available.\n");
      kill(currpid);
      restore(ps);
      return SYSERR;
    }
    
    /* update the global page table */
    frm_tab[pg_frm].fr_status = FRM_MAPPED;
    frm_tab[pg_frm].fr_pid = currpid;
    frm_tab[pg_frm].fr_vpno = virt_addr/NBPG;
    frm_tab[pd_entry->pd_base - FRAME0].fr_refcnt++;
    frm_tab[pg_frm].fr_type = FR_PAGE;
    
    /* update the page table entry */
    pt_entry->pt_pres = 1;
    pt_entry->pt_base = FRAME0 + pg_frm;
    if(debug_flag)
      kprintf("Stmt 6 - pg_frm = %d  store = %d pageth = %d *dst = %d\n",pg_frm, store, pageth,(FRAME0+pg_frm)*NBPG);
    unsigned int dst = (FRAME0+pg_frm)*NBPG;
    /* read the contents to a frame */
    read_bs((char*) dst, store, pageth);
    
  }

  write_cr3(proctab[currpid].pdbr);
  restore(ps);
  return OK;
}
