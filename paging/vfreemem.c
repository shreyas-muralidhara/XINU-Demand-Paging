/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;    
	disable(ps);
	kprintf("Entering vfreemem\n");
	struct	mblock	*p, *q;
	unsigned top;

	struct mblock *vmemlist = proctab[currpid].vmemlist;

	if (size==0 || size > 256 * NBPG)
		return(SYSERR);

	if( (unsigned) block > (unsigned) (proctab[currpid].vhpno + proctab[currpid].vhpnpages)*NBPG || ((unsigned)block)<((unsigned) 4096*NBPG))
		return(SYSERR);

	size = (unsigned)roundmb(size);
	
	for( p=(vmemlist->mnext),q= vmemlist;
	     p != (struct mblock *) NULL && p < block ;
	     q=p,p=p->mnext ){
		 kprintf("block=%8x \t block->mnext=%8x \t block->mlen=%d\nq=%8x \t q->mnext=%8x \t q->mlen=%d \np=%8x \t p->mnext=%8x \t p->mlen=%d \n",
			block,block->mnext,block->mlen,
			q,q->mnext,q->mlen,p,
			p->mnext,p->mlen);
	}

	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= vmemlist) ||
	    (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
		restore(ps);
		return(SYSERR);
	}
	if ( q!= vmemlist && top == (unsigned)block )
			q->mlen += size;
	else {
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	kprintf("vfreemem exit\n");
	restore(ps);
	return(OK);

}
