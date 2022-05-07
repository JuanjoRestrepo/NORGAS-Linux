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

/*typedef struct client_Parameters
{
  int   userId;
  int   systemId;
}client_Parameters;
*/
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
  //pthread_create ( &cm_tid, NULL, pCommunicationModule, NULL );
  //pthread_create ( &db_tid, NULL, pDataBase, NULL );
  //pthread_create ( &tmr_tid, NULL, pTimer, NULL );

  /* Wait for threads to finish */
  pthread_join ( customr_tid, NULL );
  //pthread_join ( cntrllr_tid, NULL );
  //pthread_join ( cm_tid, NULL );
  //pthread_join ( db_tid, NULL );
  //pthread_join ( tmr_tid, NULL );

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
  msg_t_2 OutMsg_2;
  client_Parameters DataUser;
  int sWeight = 0;

  for ( ; ; )
  {
    printf ( "Activation System: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &ActivationSystem );
    OutMsg.signal = ( int ) sActivationFromNorgas;
    OutMsg.value = ActivationSystem;
    sendMessage ( &(queue [COMMUNICATION_MODULE]), OutMsg );      /* send message to Controller */

    printf ( "Type Client Parameters\n " );
    printf ( "User Id: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &DataUser.userId );
    printf ( "System Id: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &DataUser.systemId );

    OutMsg_2.signal_2 = (int) sUser1;
    OutMsg_2.value_2 = DataUser;
    sendMessage_2 ( &(queue [CONTROLLER_Q]), OutMsg_2 );      /* send message to Controller */
    printf ( "Weight system\n " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf( line,"%d",&sWeight );
    OutMsg.signal = (int) sWeight;
    OutMsg.value = sWeight;
    sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );
  }

  return ( NULL );
}

static void *pController ( void *arg )
{
  CONTROLLER_STATE_ENUM state,
  state_next;
  msg_t                 InMsg,
  OutMsg;
  unsigned int          NbrOfCoffeeCups;

  msg_t_2 InMsg_2, OutMsg_2;
  state_next = System_Activation;
  for ( ; ; )
  {
    state = state_next;
    InMsg_2 = receiveMessage_2 ( &(queue [CONTROLLER_Q]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\tController rcvd [%s(%d)] in state [%s]\n", TO_CONTROLLER_STRING[InMsg_2.signal_2], InMsg_2.value_2, CONTROLLER_STATE_STRING[state] );

    fflush ( stdout );

    switch ( state )
    {
      case System_Activation:
        switch ( InMsg.signal)
        {
          case sActivation:
            if ( InMsg.value == 1 ){
            OutMsg.signal = (int) sActivationOk;
            OutMsg.value = 1;
            sendMessage (&(queue[COMMUNICATION_MODULE]), OutMsg );
            state_next = Get_ClientParameters;}
            else{
              state_next = END;}
            break;
          default:
            break;
        }
        break;
          case Get_ClientParameters:
            switch ( InMsg.signal )
            {
              case sUser1:/* send message to Hardware */
                state_next = WorkingCycle;
                break;
              default:
                break;
            }
            break;
              case PouringWater:
                switch ( InMsg.signal )
                {
                  case sWaterOk:
                    OutMsg.signal = (int) sFillCoffee;
                    OutMsg.value = 0;
                    sendMessage ( &(queue [COFFEED_Q]), OutMsg );  /* send message to Hardware */
                    state_next = DispensingCoffee;
                    break;
                  default:
                    break;
                }
                break;
                  case DispensingCoffee:
                    switch ( InMsg.signal )
                    {
                      case sCoffeeOk:
                        OutMsg.signal = (int) sHeatWater;
                        OutMsg.value = 0;
                        sendMessage ( &(queue [HEATER_Q]), OutMsg ); /* send message to Hardware */
                        state_next = BrewingCoffee;
                        break;
                      default:
                        break;
                    }
                    break;
                      case BrewingCoffee:
                        switch ( InMsg.signal )
                        {
                          case sWarm:
                            NbrOfCoffeeCups++;
                            printf ( "\n%d Cup(s) of Coffee served!\n", NbrOfCoffeeCups );
                            fflush ( stdout );
                            state_next = IdleC;
                            break;
                          default:
                            break;
                        }
                        break;
                          default:
                            break;
    }
    /* show next state */
    printf ( "\tController next state is [%s]\n", CONTROLLER_STATE_STRING[state_next] );
    fflush ( stdout );
  }

  return ( NULL );
}
