                        readme for comm directory
/*D
    CommIntro - This is the introductory manual page for the comm 
directory. The comm directory contains routines and macros for the 
low level communication (message passing) between parallel processors.
For high-level (i.e. much easier and no less efficient) communications, 
see the mesh directory (BCintro).

Description:
   The comm directory contains routines for the easy startup of 
parallel processors on a network of workstations or a parallel computer.
It is intended to provide a uniform interface for the user on top of
non-uniform machines.

   Standard message passing macros are also provided. These run on 
top of the native message passing environment. In this way an application 
code is not written for a single architecture but is portable.

Communication logging:
   The comm macros provide for several levels of communication logging,
controlled both by compile-time and run-time switches.  The compile-time
switches allow you to remove all of the logging code, thus ensuring the
smallest and fastest executable.  The run-time switches allow you to 
leave logging code in your executable, but to control whether it is 
executed.  Since this is a rather complicated area, the various options
are discussed here.

Compile-time flags:
. LOGEVENTS   - log events in alog/blog format.  These include the 
	        communication operations, including the global ops (as states)
. NOCOMMLOG   - do NOT keep track of the number of sends and recv, or the
                amount of data sent

Run-time flags:
. cloglevel   - level of logging:
$             0 - no logging
$             1 - event logs (to a file for postmortum use)
$             2 - print out for each send and recv
$   Or the bits together to get the combinations (ie, 3 is print + log).
	For cloglevel to work, LOGEVENTS must be defined and NOCOMMLOG must
	NOT be defined. The default value of cloglevel is 1.

D*/

/* Here begin definitions for the man pages of these macros */
/*M
     PInsendm - Start an asynchronous send

     Synopsis:
     void PInsendm(type, buffer, length, to, datatype, id)
     int  type, length, to, datatype;
     ASYNCSendId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type     
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    id       - identifies operation for PIwsendm.

     Note:
    "buffer" must have been allocated with MSGALLOCSEND.
M*/
/*M
     PInrecvm - Start an asynchronous receive

     Synopsis:
     void PInrecvm( type, buffer, length, datatype, id)
     int type, length, datatype;
     ASYNCRecvId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    id       - identifies operation for PInwaitm.

     Note:
    "buffer" must have been allocated with MSGALLOCRECV.
M*/
/*M
     PIbsendm - Send a message to another processor

     Synopsis:
     void PIbsendm( type, buffer, length, to, datatype)
     int type, length, to, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer   
.    length   - length (in units of sizeof(char))
.    to       - processor to send to
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Note:
    "buffer" must have been allocated with MSGALLOCSEND.
M*/
/*M
     PIbrecvm - Receive a message from another processor

     Synopsis:
     void PIbrecvm( type, buffer, length, datatype)
     int type, length, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Note:
    "buffer" must have been allocated with MSGALLOCRECV.
M*/
/*M
     PIwsendm - Complete an asynchronous send

     Synopsis:
     void PIwsendm( type, buffer, length, to, datatype, id)
     int type, length, to, datatype;
     ASYNCSendId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to 
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     
.    id       - identifies operation from PInsendm.

     Note:
    "buffer" must have been allocated with MSGALLOCSEND.
M*/
/*M
     PIwrecvm - Complete an asynchronous receive

     Synopsis:
     void PIwrecvm( type, buffer, length, datatype, id)
     int type, length, datatype;
     ASYNCRecvId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer   
.    length   - length (in units of sizeof(char)) 
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     
.    id       - identifies operation from PInrecvm.

     Note:
    "buffer" must have been allocated with MSGALLOCRECV.
M*/
/*M
     PInsend - Start an asynchronous send

     Synopsis:
     void PInsend( type, buffer, length, to, datatype, id)
     int type, length, to, datatype;
     ASYNCSendId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    id       - identifies operation for Piwsend.

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCSEND
M*/
/*M
     PInrecv - Start an asynchronous receive

     Synopsis:
     void PInrecv( type, buffer, length, datatype, id)
     int type, length, datatype;
     ASYNCRecvId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    id       - identifies operation for PIwrecv.

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCRECV
M*/
/*M
     PIbsend - Send a message to another processor

     Synopsis:
     void PIbsend( type, buffer, length, to, datatype)
     int type, length, to, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to 
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCSEND
M*/
/*M
     PIbrecv - Receive a message from another processor

     Synopsis:
     void PIbrecv( type, buffer, length, datatype)
     int type, length, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCRECV
M*/
/*M
     PIwsend - Complete an asynchronous send

     Synopsis:
     void PIwsend( type, buffer, length, to, datatype, id)
     int type, length, to, datatype;
     ASYNCSendId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     
.    id       - identifies operation from PInsend.

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCSEND
M*/
/*M
     PIwrecv - Complete an asynchronous receive

     Synopsis:
     void PIwrecv( type, buffer, length, datatype, id)
     int type, length, datatype;
     ASYNCRecvId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     
.    id       - identifies operation from PInrecv.

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCRECV
M*/
/*M
     PInsendmrr - Start an asynchronous send

     Synopsis:
     void PInsendmrr( type, buffer, length, to, datatype, id)
     int type, length, to, datatype;
     ASYNCSendId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    id       - identifies operation for PIwsendmrr

     Note:
     "buffer" must have been allocated with MSGALLOCSEND.
     "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PInrecvmrr - Start an asynchronous receive

     Synopsis:
     void PInrecvmrr( type, buffer, length, datatype, id)
     int type, length, datatype;
     ASYNCRecvId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    id       - identifies operation for PIwrecvmrr

     Note:
     "buffer" must have been allocated with MSGALLOCRECV.
     "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIbsendmrr - Send a message to another processor

     Synopsis:
     void PIbsendmrr( type, buffer, length, to, datatype)
     int type, length, to, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer   
.    length   - length (in units of sizeof(char))
.    to       - processor to send to
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Note:
     "buffer" must have been allocated with MSGALLOCSEND.
     "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIbrecvmrr - Receive a message from another processor

     Synopsis:
     void PIbrecvmrr( type, buffer, length, datatype)
     int type, length, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Note:
     "buffer" must have been allocated with MSGALLOCRECV.
     "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIwsendmrr - Complete an asynchronous send

     Synopsis:
     void PIwsendmrr( type, buffer, length, to, datatype, id)
     int type, length, to, datatype;
     ASYNCSendId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to (int)
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     
.    id       - identifies operation for SENDWAIT

     Note:
     "buffer" must have been allocated with MSGALLOCSEND.
     "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIwrecvmrr - Complete an asynchronous receive

     Synopsis:
     void PIwrecvmrr( type, buffer, length, datatype, id)
     int type, length, datatype;
     ASYNCRecvId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer   
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     
.    id       - identifies operation from PInrecvmrr

     Note:
     "buffer" must have been allocated with MSGALLOCRECV.
     "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PInsendrr - Start an asynchronous send

     Synopsis:
     void PInsendrr( type, buffer, length, to, datatype, id)
     int type, length, to, datatype;
     ASYNCSendId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    id       - identifies operation for SENDWAITNOMEMFORCE

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCSEND.
     "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PInrecvrr - Start an asynchronous receive

     Synopsis:
     void PInrecvrr( type, buffer, length, datatype, id)
     int type, length, datatype;
     ASYNCRecvId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    id       - identifies operation for PIwrecvrr

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCRECV.
$    "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIbsendrr - Send a message to another processor

     Synopsis:
     void PIbsendrr( type, buffer, length, to, datatype)
     int type, length, to, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCSEND.
$    "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIbrecvrr - Receive a message from another processor

     Synopsis:
     void PIbrecvrr( type, buffer, length, datatype)
     int type, length, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  )
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCRECV.
$    "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIwsendrr - Complete an asynchronous send

     Synopsis:
     void PIwsendrr( type, buffer, length, to, datatype, id)
     int type, length, to, datatype;
     ASYNCSendId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    to       - processor to send to 
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     
.    id       - identifies operation from PInsendrr

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCSEND.
$    "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIwrecvrr - Complete an asynchronous receive

     Synopsis:
     void PIwrecvrr( type, buffer, length, datatype, id)
     int type, length, datatype;
     ASYNCRecvId_t id;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    buffer   - pointer to buffer  
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     
.    id       - identifies operation from PInrecvrr

     Note:
     This version should be used when "buffer" has NOT been allocated with
     MSGALLOCRECV.
$    "rr" means that the message will be sent and if the destination
     machine is not ready to receive it, it will be discarded.  This
     provides a way to access faster but potentially unreliable
     communications.
M*/
/*M
     PIbrecvUnsz - Receive a message of unknown length

     Synopsis:
     void PIbrecvUnsz( type, buffer, length, datatype)
     int type, length, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type       
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

     Output Parameter:
.    buffer   - pointer to buffer
.    length   - length (in units of sizeof(char)))

     Note:
     This routine allocates a buffer of the length needed to receive the 
     message.
M*/

/*M
     PIbprobe - Block until a message of a given type is available

     Synopsis:
     void PIbprobe(type)
     int type;

     Input Parameter:
.    type    - message type 

     Note:
     This routine blocks until a message of type "type" is available.
M*/
/*M
     PInprobe - Test if a message of a given type is available

     Synopsis:
     int PInprobe(type)
     int type;

     Input Parameter:
.    type    - message type 

     Note:
     This routine returns 1 if a message of type "type" is available and
     0 otherwise
M*/
/*M
     PInstatus - Test if an asynchronous message has completed

     Synopsis:
     int PInstatus(id)
     ASYNCSendId_t id;

     Input Parameter:
.    id      - message id.  This is the id set in the nonblocking routines

     Note:
     This routine returns 1 i the message with "id" has completed, and 
     0 otherwise
M*/
/*M
     PIcrecv - Cancel a previously issued PInrecv...

     Synopsis:
     void PIcrecv(id)
     ASYNCRecvId_t id;

     Input Parameter:
.    id      - message id.  This is the id set in the nonblocking routines
M*/
/*M
     PIcsend - Cancel a previously issued PInsend...

     Synopsis:
     void PIcsend(id)
     ASYNCSendId_t id;

     Input Parameter:
.    id      - message id.  This is the id set in the nonblocking routines
M*/
/*M
     PIbrecvProbed - Receive a message that has been probed

     Synopsis:
     void PIbrecvProbed( type, buffer, length, datatype)
     int type, length, datatype;
     void *buffer;

     Input Parameters:
.    type     - message type      
.    buffer   - pointer to buffer
.    length   - length (in units of sizeof(char))
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

M*/

/*M
     PIsize - Return the length of a received message

     Synopsis:
     int PIsize()

     Note: 
     Returns the number of bytes received by the last PIxrecv...
M*/
/*M
     PIfrom - Return the processor that sent a received message

     Synopsis:
     int PIfrom()

     Note: 
     Returns the number of processor that sent the message most recently
     received.
M*/

/*M
     PItype - Return the type of the most recently received message

     Synopsis:
     int PItype()

M*/

/*M
     PIgisum - Global sum reduction

     Synopsis:
     void PIgisum( val, n, work, procset)
     int *val, n, *work;
     ProcSet *procset;

     Input Parameters:
.    val     - value to sum
.    n       - number of elements in val
.    work    - work area of the same size as val
.    procset - processor set (in null, all processors are used)

     Note:
     On exit, val contains the sum from all participating nodes.
M*/
/*M
     PIgdsum - Global sum reduction

     Synopsis:
     void PIgdsum( val, n, work, procset)
     double  *val, *work;
     int     n;
     ProcSet *procset;

     Input Parameters:
.    val     - value to sum
.    n       - number of elements in val
.    work    - work area of the same size as val
.    procset - processor set (in null, all processors are used)

     Note:
     On exit, val contains the sum from all participating nodes.
M*/
/*M
     PIgfsum - Global sum reduction

     Synopsis:
     void PIgfsum( val, n, work, procset)
     float   *val, *work;
     int     n;
     ProcSet *procset;

     Input Parameters:
.    val     - value to sum
.    n       - number of elements in val
.    work    - work area of the same size as val
.    procset - processor set (in null, all processors are used)

     Note:
     On exit, val contains the sum from all participating nodes.
M*/
/*M
     PIgimax - Global maximum reduction

     Synopsis:
     void PIgimax( val, n, work, procset)
     int *val, n, *work;
     ProcSet *procset;

     Input Parameters:
.    val     - value to sum
.    n       - number of elements in val
.    work    - work area of the same size as val
.    procset - processor set (in null, all processors are used)

     Note:
     On exit, val contains the maximum from all participating nodes.
M*/
/*M
     PIgdmax - Global maximum reduction

     Synopsis:
     void PIgdmax( val, n, work, procset)
     double  *val, *work;
     int     n;
     ProcSet *procset;

     Input Parameters:
.    val     - value to sum
.    n       - number of elements in val
.    work    - work area of the same size as val
.    procset - processor set (in null, all processors are used)

     Note:
     On exit, val contains the maximum from all participating nodes.
M*/

/*M
     PIcombine - Global user-defined combination (reduction to all nodes)

     Synopsis:
     void PIcombine( val, n, work, procset, elmsize, datatype, op )
     void    *val, *work;
     int     n, elmsize, datatype;
     ProcSet *procset;
     void    (*op)( );

     Input Parameters:
.    val     - value to combine
.    n       - number of elements in val
.    work    - work area of the same size as val
.    procset - processor set (in null, all processors are used)
.    elmsize - size in bytes of a single element of val
.    datatype- datatype of val (MSG_DBL etc)
.    op      - user-defined routine to combine elements (see below)

     Notes:
     On exit, val contains the combined value from all participating nodes.

     The routine "op" is defined as
$    void op( a, b, n )
$    void *a, *b;
$    int  n;

     with arguments
.    a - value to combine with b.  Contains combined result on output
.    b - value to combine with a.
.    n - number of elements (NOT bytes) to combine.

     For example, the routine used for a global sum of doubles is
$        void PIdsum( a, b, n )
$        double *a, *b;
$        int    n;
$        {
$        int i;
$        for (i=0; i<n; i++) a[i] += b[i];
$        }
$
     Currently, there is no Fortran version.

M*/

/*M
     PIgimin - Global minimum reduction

     Synopsis:
     void PIgimin( val, n, work, procset)
     int *val, n, *work;
     ProcSet *procset;

     Input Parameters:
.    val     - value to sum
.    n       - number of elements in val
.    work    - work area of the same size as val
.    procset - processor set (in null, all processors are used)

     Note:
     On exit, val contains the minimum from all participating nodes.
M*/
/*M
     PIgdmin - Global minimum reduction

     Synopsis:
     void PIgdmin( val, n, work, procset)
     double  *val, *work;
     int     n;
     ProcSet *procset;

     Input Parameters:
.    val     - value to sum
.    n       - number of elements in val
.    work    - work area of the same size as val
.    procset - processor set (in null, all processors are used)

     Note:
     On exit, val contains the minimum from all participating nodes.
M*/
/*M
     PIgsync - Global synchronize

     Synopsis:
     void PIgsync(procset)
     ProcSet *procset;

     Input Parameter:
.    procset - processor set (in null, all processors are used)

     Note:
     All processors in the processor set wait until they have all executed
     this statement.
M*/

/*M
     PIgcol - Global collection

     Synopsis:
     void PIgcol( lbuf, lsize, gbuf, gsiz, glen, procset, datatype)
     void    *lbuf, *gbuf;
     int     lsize, gsiz, *glen, datatype;
     ProcSet *procset;

     Input Parameters:
.    lbuf    - Pointer to local contibution
.    lsize   - size of local contribution  
.    procset - processor set (if null, all processors are used)
.    gsiz    - Size of gbuf
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT) 

     Output Parameters:
.    gbuf    - Pointer to collected data
.    glen    - Actual number of characters collected

     Note:
     Data is collected from all nodes; when this routine exits, all nodes
     have copies of all the data.  The data is present in "gbuf" in
     an implementation-dependent order; the only thing that is guarenteed is
     that each contribution is contiguous.
M*/
/*M
     PIgcolx - Global collection

     Synopsis:
     void PIgcolx( lbuf, gsizes, gbuf, procset,datatype)
     void    *lbuf, *gbuf;
     int     *gsizes,datatype;
     ProcSet *procset;

     Input Parameters:
.    lbuf    - Pointer to local contibution
.    gsizes  - size of all the contributions (in bytes)
.    procset - processor set (if null, all processors are used)
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT) 

     Output Parameters:
.    gbuf    - Pointer to collected data

     Note:
     Data is collected from all nodes; when this routine exits, all nodes
     have copies of all the data.  The data is present in node-number order.
M*/

/*M
     PIbcast - Broadcast data to all processors

     Synopsis:
     void PIbcast( buf, siz, issrc, procset, datatype) 
     void    *buf;
     int     siz, issrc, datatype;
     ProcSet *procset;

     Input Parameters:
.    buf      - pointer to data
.    siz      - size of buf in bytes
.    issrc    - 1 if this is the sending processor, 0 otherwise.  Exactly
                one processor may set issrc to 1.
.    procset  - processor set (in null, all processors are used)
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

M*/

/*M
     PIbcastSrc - Broadcast data to all processors

     Synopsis:
     void PIbcastSrc( buf, siz, src, procset, datatype ) 
     void    *buf;
     int     siz, src, datatype;
     ProcSet *procset;

     Input Parameters:
.    buf      - pointer to data
.    siz      - size of buf in bytes
.    src      - node number of the source processor.
.    procset  - processor set (in null, all processors are used)
.    datatype - type of data (MSG_INT, MSG_OTHER, MSG_DBL, or MSG_FLT)     

M*/

/*M
     PIgtoken - Pass a "token" among processors

     Synopsis:
     int PIgtoken( procset, i)
     ProcSet *procset;
     int     i;

     Input Parameters:
.    procset - processor set (in null, all processors are used)
.    i       - Loop index (see below)

     Note:
     This routine should be used in a loop to insure that each processor
     can carry out an action while the others wait.  An example is for
     each processor to print out some information on what it is doing.
     An example is
$    for (i=0; i<=PInumtids; i++) 
$        if (PIgtoken(0,i)){ 
$            <do_something> 
$        }
$    Note that i runs from 0 through PInumtids; that is, the loop is
     executed PInumtids+1 times.  The last time serves to pass the "token"
     back to the 0'th processor.
M*/     

/*MC
     MSGALLOCSEND - Allocate storage for sending a message

     Synopsis:
     void MSGALLOCSEND( msg, max, type )
     (type *)msg;
     int     max;

     Input Parameters:
.    msg     - pointer to hold allocated message buffer
.    max     - length of message buffer
.    type    - type
M*/
/*MC
     MSGALLOCRECV - Allocate storage for receiving a message

     Synopsis:
     void MSGALLOCRECV( msg, max, type )
     (type *)msg;
     int     max;

     Input Parameters:
.    msg     - pointer to hold allocated message buffer
.    max     - length of message buffer
.    type    - type
M*/
/*M
     MSGFREESEND - Free storage for sending a message

     Synopsis:
     void MSGFREESEND(msg)
     void *msg;

     Input Parameters:
.    msg     - pointer to allocated message buffer
M*/
/*M
     MSGFREERECV - Free storage for receiving a message

     Synopsis:
     void MSGFREERECV(msg)
     void *msg;

     Input Parameters:
.    msg     - pointer to allocated message buffer
M*/
/*MC
     PInumtids - Return the number of processors

     Synopsis:
     int PInumtids

M*/
/*MC
     PImytid - Return my processor id

     Synopsis:
     int PImytid

     Note:
     PImytid is in the range [0,PInumtids-1]
M*/
/*M
     PIMsgSizes - Return the range of message sizes

     Synopsis:
     void PIMsgSizes( min, max)
     int *min, *max;

     Output parameters:
.    min,max - lengths of minimum and maximum messages, in bytes.

     Note:
     Message lengths in the range [min.max] may be used.  Regrettably, this
     information is not always available; in that case, this routine returns
     values that are believed to work (typical minimum is sizeof(int) and
     maximum is 16536).
M*/
/*M
     PITagRange - Return the range of value (user) message tags

     Synopsis:
     void PITagRange( low, high)
     int *low, *high;

     Output Parameters:
.    low,high - range of legal message types.

     Note:
     Message types in the range [low,high] may be used.  Regrettably, this
     information is not always available; in that case, this routine returns
     the minimum range that is known to work.     
M*/
/*M
     PIdistance - Return the number of hops between two processors
     
     Synopsis:
     int PIdistance( from, to)
     int from, to;

     Input Paramters:
.     from - id of sending processor
.     to   - id of receiving processor

     Note:
     The exact meaning of a "hop" depends on the implementation.       
M*/
/*MC
     PIdiameter - Return the maximum number of hops between two processors
     
     Synopsis:
     int PIdiameter

     Note:
     The exact meaning of a "hop" depends on the implementation.       
M*/
