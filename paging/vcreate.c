/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
	disable(ps);
	
	int pid, bsm_id;
	/* Create a new process using the create function */
	pid = create(procaddr,ssize,priority,name,nargs,args);
	
	/*Check if the heap size is within the bounds */
	if(hsize < 0 || hsize >128){
		kprintf("Heap size - %d  not within bounds\n", hsize);
		restore(ps);
		return SYSERR;
	}
	
	if(get_bsm(&bsm_id) == SYSERR){
		//kprintf("unable to create backing store for private heap\n");
		restore(ps);
		return SYSERR;
	}
	/* Set the process parameters for the new heap */
	proctab[pid].store = bsm_id;
	proctab[pid].vhpnpages = hsize;
	proctab[pid].vhpno = 4096 ;
	proctab[pid].vmemlist->mnext = 4096 * NBPG;
	
	/*Upadate the bsm entry */
	bsm_tab[bsm_id].bs_private = 1;
	bsm_tab[bsm_id].bs_status = BSM_MAPPED;
	bsm_map(pid, FRAME0, bsm_id, hsize);
	
	/* Point to 1st page of allocated BS */
	struct mblock *heapbase = BACKING_STORE_BASE + (bsm_id * BACKING_STORE_UNIT_SIZE);
	heapbase->mlen = hsize * NBPG;
	heapbase->mnext = NULL;
	
	restore(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
