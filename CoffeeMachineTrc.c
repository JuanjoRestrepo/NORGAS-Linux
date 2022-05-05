/*******************************************************************************
*
*  CoffeeMachine.c -      A program implementing the modified use case
*                         MakeCoffee (uses a timer) for the Coffee Machine
*                         model v3
*
*   Notes:                Error checking omitted...
*
*******************************************************************************/

#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>
#include    <unistd.h>
#include    <errno.h>
#include    "phtrdsMsgLyr.h"              /* pthreads message layer function
                                              prototypes, constants, structs */

/***( Manifest constants )************************************************/

#define     SLEEPING_TIME    24

/***( Function prototypes )***********************************************/

static void *pCustomer ( void *arg );     /* Customer code */
static void *pController ( void *arg );   /* Controller code */
static void *pCommunicationModule ( void *arg );       /* CommunicationModule code */
static void *pDataBase ( void *arg );      /* Data Base  code */
static void *pTimer ( void *arg );        /* Timer code */

typedef struct client_Parameters
{
  int   userId;
  int   systemId;
}client_Parameters;

/***( SDL system creation )***********************************************/
int main ( void )
{
  pthread_t   customr_tid;                /* Costumer tid */
  pthread_t   cntrllr_tid;                /* Controller tid */
  pthread_t   cm_tid;                     /* Communication Module tid */
  pthread_t   db_tid;                     /* Data Base tid */
  pthread_t   tmr_tid;                    /* Timer tid */

  /* Create queues */
  initialiseQueues ();

  /* Create threads */
  pthread_create ( &customr_tid, NULL, pCustomer, NULL );
  pthread_create ( &cntrllr_tid, NULL, pController, NULL );
  pthread_create ( &cm_tid, NULL, pCommunicationModule, NULL );
  pthread_create ( &db_tid, NULL, pDataBase, NULL );
  pthread_create ( &tmr_tid, NULL, pTimer, NULL );

  /* Wait for threads to finish */
  pthread_join ( customr_tid, NULL );
  pthread_join ( cntrllr_tid, NULL );
  pthread_join ( cm_tid, NULL );
  pthread_join ( db_tid, NULL );
  pthread_join ( tmr_tid, NULL );

  /* Destroy queues */
  destroyQueues ();

  return ( 0 );
}

/***( SDL system processes )**********************************************/

/* Customer thread */
static void *pCustomer ( void *arg )
{
  char  line [100];
  int   serialNumber, Percentage, ActivationSystem, user, system;
  char  cupType;
  msg_t OutMsg;

  for ( ; ; )
  {
    printf ( "Activation System: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &ActivationSystem );
    if ( ActivationSystem == 0 ){
      exit ( 0 );
    }
    OutMsg.signal = ( int ) sActivationFromNorgas;
    OutMsg.value = ActivationSystem;
    sendMessage ( &(queue [COMMUNICATION_MODULE]), OutMsg );      /* send message to Controller */

    printf ( "Type Client Parameters\n " );
    printf ( "User Id: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &user );

    printf ( "System Id: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &system );

    OutMsg.signal = (int) sUser1;
    OutMsg.value = 1;
    sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );      /* send message to Controller */
  }

  return ( NULL );
}

