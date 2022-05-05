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
  int   serialNumber, Percentage, ActivationSystem, userId, systemId;
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
    sscanf ( line, "%d", &client_Parameters.userId );

    printf ( "System Id: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &client_Parameters.systemId );

    OutMsg.signal = (int) sUser;
    OutMsg.value = client_Parameters;
    sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );      /* send message to Controller */
  }

  return ( NULL );
}

/* Controller thread */
static void *pController ( void *arg )
{
  CONTROLLER_STATE_ENUM state,
                        state_next;
  msg_t                 InMsg,
                        OutMsg;
  unsigned int          NbrOfCoffeeCups;

  NbrOfCoffeeCups = 0;
  state_next = IdleC;
  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [CONTROLLER_Q]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\tController rcvd [%s(%d)] in state [%s]\n", TO_CONTROLLER_STRING[InMsg.signal], InMsg.value, CONTROLLER_STATE_STRING[state] );

    fflush ( stdout );

    switch ( state )
    {
      case IdleC:
        switch ( InMsg.signal )
        {
          case sCoin:
            if ( InMsg.value == 10 )
              state_next = PaidTen;
            else
              state_next = IdleC;
            break;
          default:
            break;
        }
        break;
      case PaidTen:
        switch ( InMsg.signal )
        {
          case sCoffee:
            OutMsg.signal = (int) sFillWater;
            OutMsg.value = 0;
            sendMessage ( &(queue [WATERD_Q]), OutMsg );  /* send message to Hardware */
            state_next = PouringWater;
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

/* Water dispenser thread */
static void *pWaterD ( void *arg )
{
  WATERD_STATE_ENUM   state,
                      state_next;
  msg_t               InMsg,
                      OutMsg;

  state_next = IdleW;
  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [WATERD_Q]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\t\tWaterD rcvd [%s(%d)] in state [%s]\n", TO_HARDWARE_STRING[InMsg.signal], InMsg.value, WATERD_STATE_STRING[state] );
    fflush ( stdout );

    switch ( state )
    {
      case IdleW:
        switch ( InMsg.signal )
        {
          case sFillWater:
            OutMsg.signal = (int) sWaterOk;
            OutMsg.value = 0;
            sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );     /* send message to Controller */
            state_next = IdleW;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    /* show next state */
    printf ( "\t\tWaterD next state is [%s]\n", WATERD_STATE_STRING[state_next] );
    fflush ( stdout );
  }

  return ( NULL );
}

/* Coffee dispenser thread */
static void *pCoffeeD ( void *arg )
{
  COFFEED_STATE_ENUM  state,
                      state_next;
  msg_t               InMsg,
                      OutMsg;

  state_next = IdleF;
  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [COFFEED_Q]) );

    printf ( "\t\t\tCoffeeD rcvd [%s(%d)] in state [%s]\n", TO_HARDWARE_STRING[InMsg.signal], InMsg.value, COFFEED_STATE_STRING[state] );
    fflush ( stdout );

    switch ( state )
    {
      case IdleF:
        switch ( InMsg.signal )
        {
          case sFillCoffee:
            OutMsg.signal = (int) sCoffeeOk;
            OutMsg.value = 0;
            sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );     /* send message to Controller */
            state_next = IdleF;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    /* show next state */
    printf ( "\t\t\tCoffeeD next state is [%s]\n", COFFEED_STATE_STRING[state_next] );
    fflush ( stdout );
  }

  return ( NULL );
}

/* Coffee dispenser thread */
static void *pHeater ( void *arg )
{
  HEATER_STATE_ENUM   state,
                      state_next;
  msg_t               InMsg,
                      OutMsg;

  state_next = IdleH;
  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [HEATER_Q]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\t\t\t\tHeater rcvd [%s(%d)] in state [%s]\n", TO_HARDWARE_STRING[InMsg.signal], InMsg.value, HEATER_STATE_STRING[state] );

    fflush ( stdout );

    switch ( state )
    {
      case IdleH:
        switch ( InMsg.signal )
        {
          case sHeatWater:
            OutMsg.signal = (int) sSetHeater;
            OutMsg.value = HEATING_TIME;
            sendMessage ( &(queue [TIMER_Q]), OutMsg );     /* send message to Timer */
            state_next = Warming;
            break;
          default:
            break;
        }
        break;
      case Warming:
        switch ( InMsg.signal )
        {
          case sHeater:
            OutMsg.signal = (int) sWarm;
            OutMsg.value = 0;
            sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );     /* send message to Controller */
            state_next = IdleH;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    /* show next state */
    printf ( "\t\t\t\tHeater next state is [%s]\n", HEATER_STATE_STRING[state_next] );
    fflush ( stdout );
  }

  return ( NULL );
}

/* Timer thread */
static void *pTimer ( void *arg )
{
  TIMER_STATE_ENUM  state,
                    state_next;
  msg_t             InMsg,
                    OutMsg;

  state_next = IdleT;
  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [TIMER_Q]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\t\t\t\t\tTimer rcvd [%s(%d)] in state [%s]\n", TO_TIMER_STRING[InMsg.signal], InMsg.value, TIMER_STATE_STRING[state] );
    fflush ( stdout );

    switch ( state )
    {
      case IdleT:
        switch ( InMsg.signal )
        {
          case sSetHeater:
            sleep ( InMsg.value );
            OutMsg.signal = (int) sTexpired;
            OutMsg.value = 0;
            sendMessage ( &(queue [TIMER_Q]), OutMsg );     /* send message to SELF */
            state_next = CheckTimeout;
            break;
          default:
            break;
        }
        break;
      case CheckTimeout:
        switch ( InMsg.signal )
        {
          case sTexpired:
            OutMsg.signal = (int) sHeater;
            OutMsg.value = 0;
            sendMessage ( &(queue [HEATER_Q]), OutMsg );     /* send message to Hardware */
            state_next = IdleT;
            break;
          case sResetHeater:
            state_next = WastingTime;
            break;
          default:
            break;
        }
        break;
      case WastingTime:
        switch ( InMsg.signal )
        {
          case sTexpired:
            state_next = IdleT;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    /* show next state */
    printf ( "\t\t\t\t\tTimer next state is [%s]\n", TIMER_STATE_STRING[state_next] );
    fflush ( stdout );
  }

  return ( NULL );
}
