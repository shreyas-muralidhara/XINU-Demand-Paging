/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;
	disable(ps);    
	struct	mblock	*p, *q, *leftover;
	//struct mblock *vmemlist = proctab[currpid].vmemlist;
	//kprintf("Entering vgetmem\n");
	
	
	if (nbytes==0 || proctab[currpid].vmemlist->mnext== (struct mblock *) NULL) {
		kprintf("ERROR: Insufficient memory for heap. nbytes - %d, mnext->len - %d\n", nbytes, proctab[currpid].vmemlist->mnext->mlen);
		restore(ps);
		kprintf("Stmt 1\n");
		return( (WORD *)SYSERR);
	}
	
	nbytes = (unsigned int) roundmb(nbytes);
	for ( q= &proctab[currpid].vmemlist,p=proctab[currpid].vmemlist->mnext ;
	     p != (struct mblock *) NULL ;
	     q=p,p=p->mnext)    {
		//kprintf("Entering for\n");
		if ( p->mlen == nbytes) {
			q->mnext = p->mnext;
			restore(ps);
			//kprintf("Stmt 2\n");
			return( (WORD *)p );
		} else if ( p->mlen > nbytes ) {
			leftover = (struct mblock *)( (unsigned)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			restore(ps);
			//kprintf("Stmt 3\n");
			return( (WORD *)p );
		}
		//kprintf("Entering here\n");
	}
	//kprintf("Exiting vgetmem\n");
	restore(ps);
	return( (WORD *)SYSERR );
}


