/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  STATWORD ps;
  disable(ps);
  int i;
  for(i=0; i<NFRAMES; i++){
    frm_tab[i].fr_status = FRM_UNMAPPED;  /* Initialize the frame status to unmapped */
  	frm_tab[i].fr_pid = -1;               /* reset the PID for the frame*/
  	frm_tab[i].fr_vpno = 0;               /* Corresponding vpage no is set to 0 */
    frm_tab[i].fr_refcnt = 0;             /* Reference count set to 0 */
    frm_tab[i].fr_type = FR_PAGE;         /* Type assigned as PT by defaut */
    frm_tab[i].fr_dirty = 0;              /*Reset the dirty bit */
  }

  frm_queue = NULL;
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  STATWORD ps;
  disable(ps);
  int i;
  for(i=0;i<NFRAMES;i++){
    if(frm_tab[i].fr_status == FRM_UNMAPPED){
      *avail = i;
      restore(ps);
      return OK;
    }
  }
  /* Function call to page replacement policy */
  //int page_removed = srpolicy();
   
  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  kprintf("To be implemented!\n");
  return OK;
}



