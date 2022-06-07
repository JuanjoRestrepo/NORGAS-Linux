/*******************************************************************************
*
*   phone.c -   A program implementing the phone system (PragmaDev Studio
*                                                         tutorial)
*
*   Notes:                Error checking omitted...
*
*******************************************************************************/

/*
#include    <stdio.h>
#include    <unistd.h>
#include    <string.h>
#include    <errno.h>
#include    "phtrdsMsgLyr.h"              /* pthreads message layer function
                                              prototypes, constants, structs */

/***( Manifest constants )************************************************/
/*
#define     NUM_PHONE     5

/***( Function prototypes )***********************************************/
static void *pLocal ( void *arg_ptr );    /* pLocal process code */
static void *pCentral ( void *arg_ptr );  /* pCentral process code */
static void *pEnvironment ( void *arg );  /* environment pseudo-process code */

/***( SDL system creation )***********************************************/

int main ( void )
{
  pthread_t   env_tid;                    /* Env tid */
  pthread_t   pCentral_tid;               /* pCentral process tid */

  /* Create queues */
  initialiseQueues ();

  /* Create threads */
  pthread_create ( &pCentral_tid, NULL, pCentral, NULL );
  pthread_create ( &env_tid, NULL, pEnvironment, NULL );

  /* Wait for threads to finish */
  pthread_join ( env_tid, NULL );
  pthread_join ( pCentral_tid, NULL );

  /* Destroy queues */
  destroyQueues ();

  return ( 0 );
}

/***( SDL system processes )**********************************************/

/* Environment thread */
static void *pEnvironment ( void *arg )
{
  char  line [100];
  int   choice,
        Asub,
        Bsub;

  msg_t OutMsg;

  for ( ; ; )
  {
    printf ( "1. Make a call\n" );
    printf ( "2. Hang up\n" );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &choice );
    switch ( choice )
    {
      case 1:
        printf ( "Choose A subscriber (1..%d)\n", NUM_PHONE );  /* select A (source) subscriber */
        fflush ( stdout );
        fflush ( stdin );
        fgets ( line, sizeof (line), stdin );
        sscanf ( line, "%d", &Asub );
        printf ( "Choose B subscriber (1..%d)\n", NUM_PHONE );  /* select B (destination) subscriber */
        fflush ( stdout );
        fflush ( stdin );
        fgets ( line, sizeof (line), stdin );
        sscanf ( line, "%d", &Bsub );
        OutMsg.signal = (int) sCall;                            /* send message to A's device */
        OutMsg.value = Bsub;                                    /* indicating B subscriber */
        OutMsg.sender = 0;
        sendMessage ( &(queue [Asub]), OutMsg );
        break;
      case 2:
        printf ( "Choose subscriber (1..%d)\n", NUM_PHONE );    /* select A subscriber */
        fflush ( stdout );
        fflush ( stdin );
        fgets ( line, sizeof (line), stdin );
        sscanf ( line, "%d", &Asub );
        OutMsg.signal = (int) sHangUp;                          /* send message to A's device */
        OutMsg.value = 0;                                       /* indicating hangup */
        OutMsg.sender = 0;
        sendMessage ( &(queue [Asub]), OutMsg );
        break;
      default:
        break;
    }
  }

  return ( NULL );
}

/* pCentral process thread */
static void *pCentral ( void *arg_ptr )
{
  CENTRAL_STATES    state,
                    state_next;
  msg_t             InMsg,
                    OutMsg;
  pthread_t         pLocals[NUM_PHONE+1];

  int               index;

  int               SenderQ;

  for ( index = 1; index <= NUM_PHONE; index++ )
  {
    pthread_create ( &pLocals[index], NULL, pLocal, (void *) &index );
    sleep ( 1 );
  }

  printf ( "\t\t\t\tsReady\n" );                          /* "send" message to environment */
  fflush ( stdout );

  state_next = IdleC;

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [CENTRAL_Q]) );
    switch ( state )
    {
      case IdleC:
        switch ( InMsg.signal )
        {
          case sGetId:
            index = InMsg.value;
            if ( ( index >= 1 ) && ( index <= NUM_PHONE ) )
            {
              OutMsg.signal = (int) sId;
              OutMsg.value = index;                       /* pLocals[index] */
              OutMsg.sender = 0;
            }
            else
            {
              OutMsg.signal = (int) sError;
              OutMsg.value = 0;
              OutMsg.sender = 0;
            }
            SenderQ = InMsg.sender;
            sendMessage ( &(queue [SenderQ]), OutMsg );   /* send message to SENDER process (pLocal instance) */
            state_next = IdleC;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }

  for ( index = 1; index <= NUM_PHONE; index++ )
    pthread_join ( pLocals[index], NULL );

  return ( NULL );
}

/* pLocal process thread */
static void *pLocal ( void *arg_ptr )
{
  LOCAL_STATES    state,
                  state_next;
  msg_t           InMsg,
                  OutMsg;

  int             queueNo,
                  *data_ptr;

  int             calledNumber,
                  remotePId,
                  whoami,
                  senderQ;

  data_ptr = (int *) arg_ptr;
  queueNo = whoami = *data_ptr;

  printf ( "\t\t\t\tsubscriber %d is free...\n", whoami );
  fflush ( stdout );

  state_next = IdleL;

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [queueNo]) );

    switch ( state )
    {
      case IdleL:
        switch ( InMsg.signal )
        {
          case sCall:
            calledNumber = InMsg.value;
            OutMsg.signal = (int) sGetId;
            OutMsg.value = calledNumber;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [CENTRAL_Q]), OutMsg ); /* send message to pCentral process */
            state_next = GettingId;
            break;
          case sCnxReq:
            remotePId = InMsg.sender;
            OutMsg.signal = (int) sCnxConf;
            OutMsg.value = 0;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [remotePId]), OutMsg ); /* send message to peer pLocal process */
            state_next = Connected;
            break;
          default:
            break;
        }
        break;
      case GettingId:
        switch ( InMsg.signal )
        {
          case sId:
            remotePId = InMsg.value;
            OutMsg.signal = (int) sCnxReq;
            OutMsg.value = 0;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [remotePId]), OutMsg ); /* send message to peer pLocal process */
            state_next = Connecting;
            break;
          case sError:
            printf ( "\t\t\t\t%d sBusy\n", whoami );      /* "send" message to environment */
            fflush ( stdout );
            state_next = IdleL;
            break;
          default:
            break;
        }
        break;
      case Connecting:
        switch ( InMsg.signal )
        {
          case sCnxConf:
            printf ( "\t\t\t\tsCallConf: %d -> %d\n", whoami, InMsg.sender );
                                                          /* "send" message to environment */
            fflush ( stdout );
            state_next = Connected;
            break;
          case sBusy:
            printf ( "\t\t\t\t%d sBusy\n", InMsg.sender );
                                                          /* "send" message to environment */
            fflush ( stdout );
            state_next = IdleL;
            break;
          default:
            break;
        }
        break;
      case Connected:
        switch ( InMsg.signal )
        {
          case sCnxReq:
            senderQ = InMsg.sender;
            OutMsg.signal = (int) sBusy;
            OutMsg.value = 0;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [senderQ]), OutMsg );   /* send message to peer pLocal process */
            state_next = Connected;
            break;
          case sDisReq:
            senderQ = InMsg.sender;
            OutMsg.signal = (int) sDisConf;
            OutMsg.value = 0;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [senderQ]), OutMsg );   /* send message to peer pLocal process */
            state_next = IdleL;
            break;
          case sHangUp:
            OutMsg.signal = (int) sDisReq;
            OutMsg.value = 0;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [remotePId]), OutMsg ); /* send message to peer pLocal process */
            state_next = Disconnecting;
            break;
          default:
            break;
        }
        break;
      case Disconnecting:
        switch ( InMsg.signal )
        {
          case sDisConf:
            printf ( "\t\t\t\t%d sHangUpconf, %d hung too\n", whoami, InMsg.sender );
                                                        /* "send" message to environment */
            fflush ( stdout );
            state_next = IdleL;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }

  return ( NULL );
}
