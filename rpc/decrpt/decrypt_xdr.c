/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "decrypt.h"

bool_t
xdr_input (XDR *xdrs, input *objp)
{
	register int32_t *buf;

	int i;
	 if (!xdr_vector (xdrs, (char *)objp->inp, 100,
		sizeof (char), (xdrproc_t) xdr_char))
		 return FALSE;
	 if (!xdr_vector (xdrs, (char *)objp->secret, 100,
		sizeof (char), (xdrproc_t) xdr_char))
		 return FALSE;
	return TRUE;
}