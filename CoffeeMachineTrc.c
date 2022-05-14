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
#define     Weight_FullTank  10

/***( Function prototypes )***********************************************/

static void *pCustomer ( void *arg );     /* Customer code */
static void *pController ( void *arg );   /* Controller code */
static void *pCommunicationModule ( void *arg );       /* CommunicationModule code */
static void *pDataBase ( void *arg );      /* Data Base  code */
//static void *pTimer ( void *arg );        /* Timer code */


/***( SDL system creation )***********************************************/
int main ( void )
{
  pthread_t   customr_tid;                /* Costumer tid */
  pthread_t   cntrllr_tid;                /* Controller tid */
  pthread_t   cm_tid;                     /* Communication Module tid */
  pthread_t   db_tid;                     /* Data Base tid */
  //pthread_t   tmr_tid;                    /* Timer tid */

  /* Create queues */
  initialiseQueues ();

  /* Create threads */
  pthread_create ( &customr_tid, NULL, pCustomer, NULL );
  pthread_create ( &cntrllr_tid, NULL, pController, NULL );
  pthread_create ( &cm_tid, NULL, pCommunicationModule, NULL );
  pthread_create ( &db_tid, NULL, pDataBase, NULL );
  //pthread_create ( &tmr_tid, NULL, pTimer, NULL );

  /* Wait for threads to finish */
  pthread_join ( customr_tid, NULL );
  pthread_join ( cntrllr_tid, NULL );
  pthread_join ( cm_tid, NULL );
  pthread_join ( db_tid, NULL );
  //pthread_join ( tmr_tid, NULL );

  /* Destroy queues */
  destroyQueues ();

  return ( 0 );
}

/***( SDL system processes )**********************************************/

/* Customer thread */
int sWeight = 0;
static void *pCustomer ( void *arg )
{
  char  line [100];
  int   ActivationSystem;
  msg_t OutMsg;
  client_Parameters DataUser;
  for ( ; ; )
  {

    printf ( "\nActivation System: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &ActivationSystem );
/*    if ( ActivationSystem == 0 )
    {
      exit ( 0 );
    }*/
    OutMsg.signal = ( int ) sActivationFromNorgas;
    OutMsg.value = ActivationSystem;
   // OutMsg.userData = (client_Parameters){0, 0};
    //printf("jhbkbmbjjbj %d", OutMsg.value);
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

    OutMsg.signal = (int) sUser1;
    OutMsg.value = DataUser.systemId;
    //OutMsg.userData = (client_Parameters){0, 0};
    sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );      /* send message to Controller */
    printf ( "\nWeight system: " );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf( line,"%d",&sWeight );
    OutMsg.signal = (int) sWeightFromUser;
    OutMsg.value = sWeight;
    //OutMsg.userData = (client_Parameters){0, 0};
    sendMessage ( &(queue [CONTROLLER_Q]), OutMsg );
    break;

  }

  //char arreglo[50] = {DataUser.userId, DataUser.systemId, sWeight};
  return ( NULL );

  //return arreglo;
}


static void *pController ( void *arg )
{
  //printf("controladorJFFFHGFHFHFHGHF");
  CONTROLLER_STATE_ENUM state,
  state_next;
  msg_t                 InMsg,
  OutMsg;

  state_next = System_Activation;
  int Percentage = 0, GasOk = 0;


  //int *dataArray;
  //dataArray = pCustomer(&arg );

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [CONTROLLER_Q]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\tController rcvd [%s(%d)] in state [%s]\n", TO_CONTROLLER_STRING[InMsg.signal], InMsg.value, CONTROLLER_STATE_STRING[state] );

    fflush ( stdout );

    switch ( state )
    {
      case System_Activation:
        switch ( InMsg.signal)
        {
          case sActivation:
            if ( InMsg.value == 1 )
            {

              OutMsg.signal = (int) sActivationOk;
              OutMsg.value = 1;
              sendMessage (&(queue[COMMUNICATION_MODULE]), OutMsg );
              state_next = Get_ClientParameters;
            }
            else
            {


              state_next = END;
              exit(0);
            }
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

      case WorkingCycle:
        printf("ENTRO AL WorkingCycle");
        switch ( InMsg.signal)
        {
          case sWeightFromUser:
            printf("ENTRO AL WorkingCycle");
            InMsg.value = sWeight;
            Percentage = (InMsg.value*100)/Weight_FullTank;   //Valor porcentaje del peso cilindro
            OutMsg.signal = (int) sGasLevel;
            OutMsg.value = Percentage;
            if ( OutMsg.value < 20 )
            {
              OutMsg.signal = (int) sLedsUser;
              OutMsg.value = 1;
              sendMessage (&(queue[PCUSTOMER]), OutMsg );
              state_next = CheckGasLevelOK;
            }
            else
            {
              state_next = CheckGasLevelOK;
            }
            break;
          default:
            break;
        }
        break;



      case CheckGasLevelOK:
        switch ( InMsg.signal)
        {
          case sGasLevelOk:
            InMsg.value = GasOk ;
            //OutMsg.signal = (int) sGasLevel;
            //OutMsg.value = Percentaje;
            if ( InMsg.value == 1 )
            {
              OutMsg.signal = (int) sUser1;
              OutMsg.value = 0;
              //OutMsg.userData = sData;
              //OutMsg.userData = (client_Parameters){dataArray, 0};
              sendMessage (&(queue[COMMUNICATION_MODULE]), OutMsg );
              state_next = SentDataConfirmation;
            }
            else
            {
              state_next = WorkingCycle;
            }
            break;
          default:
            break;
      }
      break;


      case SentDataConfirmation:
        switch ( InMsg.signal)
        {
          case sDataOk:
            InMsg.value = 1 ;
            OutMsg.signal = (int) sSleep;
            OutMsg.value = 1;

            OutMsg.signal = (int) sWakeUp;
            OutMsg.value = 1;
            sendMessage (&(queue[COMMUNICATION_MODULE]), OutMsg );
            state_next = WorkingCycle;
            break;
          default:
            break;
        }
        break;

      case END:
        exit(0);
        break;

        default:
          break;


    }
  }
  return (NULL);
}



//MODULO DE COMUNICACION


static void *pCommunicationModule ( void *arg )
{
  COMMUNICATION_MODULE_STATE_ENUM state,
  state_next;
  msg_t                 InMsg,
  OutMsg;

  state_next = ActivateModule;
  int Activation = 0, GasOk = 0;


  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [COMMUNICATION_MODULE]) );

    /* show which message (signal name and value) was received in current state */;
    printf ( "\tCommunication Module rcvd [%s(%d)] in state [%s]\n", TO_COMMUNICATION_MODULE[InMsg.signal], InMsg.value, COMMUNICATION_MODULE_STATE_STRING[state] );

    fflush ( stdout );

    switch ( state )
    {
      case ActivateModule:
        switch ( InMsg.signal)
        {
          case sActivationFromNorgas:
            if ( InMsg.value == 1 )
            {
              Activation = 1;
              OutMsg.signal = (int) sActivation;
              OutMsg.value = Activation ;
              sendMessage (&(queue[CONTROLLER_Q]), OutMsg );
              state_next = VerifySystemForActivation;

            }
            else
            {
              Activation = 0;
              OutMsg.signal = (int) sActivation;
              OutMsg.value = Activation ;
              sendMessage (&(queue[CONTROLLER_Q]), OutMsg );
              state_next = VerifySystemForActivation;
            }
            break;
          default:
            break;
        }
          break;

        case VerifySystemForActivation:
          switch(InMsg.signal)
          {
            case sActivationOk:
              OutMsg.signal = (int) sActivationState;
              OutMsg.value = Activation;
              sendMessage (&(queue[DATA_BASE]), OutMsg );
              state_next = GasLevelConfirmation;
              break;
          }
          break;

        case GasLevelConfirmation:
          switch ( InMsg.signal )
          {
            case sGasLevel:
              OutMsg.signal = (int) sGasLevelOk;
              OutMsg.value = GasOk ;
              sendMessage (&(queue[CONTROLLER_Q]), OutMsg );
              state_next = DataSent;
              break;
            default:
              break;
          }
          break;

        //OutMsg.userData = (client_Parameters){dataArray, 0};
        case DataSent:
          switch ( InMsg.signal)
          {
            case sUser1:
              InMsg.value = 0;
              InMsg.value = 0;
              //InMsg.userData = (client_Parameters){dataArray, 0};
              OutMsg.signal = (int) sData;
              //OutMsg.userData = (client_Parameters){dataArray, 0};
              OutMsg.signal = (int) sDataOk;
              sendMessage (&(queue[CONTROLLER_Q]), OutMsg );
              state_next = Hybernation;
              break;
            default:
              break;
          }
        break;

        case Hybernation:
          switch ( InMsg.signal)
          {
            case sSleep:
              InMsg.value = 1 ;
              state_next = Ready;
              break;
            default:
              break;
          }
          break;


        case Ready:
          switch ( InMsg.signal)
          {
            case sWakeUp:
              InMsg.value = 1 ;
              state_next = END;
              break;
            default:
              break;
          }
          break;

    }
  }
  return (NULL);
}


//BASE DE DATOS

static void *pDataBase ( void *arg )
{
  DATA_BASE_STATE_ENUM state,
  state_next;
  msg_t                 InMsg;

  state_next = Activation_DataBase;
  client_Parameters DataUser;

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [DATA_BASE]) );

    /* show which message (signal name and value) was received in current state */;
    printf ( "\tData Bse rcvd [%s(%d)] in state [%s]\n", TO_DATA_BASE_STRING [InMsg.signal], InMsg.value, DATA_BASE_STATE_STRING[state] );

    fflush ( stdout );

    switch ( state )
    {
      case Activation_DataBase:
        switch ( InMsg.signal)
        {
          case sActivationState:
            InMsg.value = sActivationState;
            state_next = sendMessage_State;
            break;
          default:
            break;
        }
        break;


      case sendMessage_State:
        switch ( InMsg.signal )
        {
          case sData:
            InMsg.userData = DataUser;
            state_next = Activation_DataBase;
            break;
          default:
            break;
        }
        break;
      /*
      case END:
        exit(0);
        break;

        default:
          break;*/

    }
  }
  return (NULL);
}


