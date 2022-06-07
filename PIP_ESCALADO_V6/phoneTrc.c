/*******************************************************************************
*
*   phoneTrc.c -    A program implementing the phone system (PragmaDev Studio
*                                                             tutorial)
*
*   Notes:                Error checking omitted...
*
*******************************************************************************/

#include    <stdio.h>
#include    <unistd.h>
#include    <string.h>
#include    <errno.h>
#include    "phtrdsMsgLyr.h"              /* pthreads message layer function
                                              prototypes, constants, structs */

/***( Manifest constants )************************************************/
#define     Full_tank_weight 40
#define     Numero_sistemas 3
#define     Numero_hilos 9 //Numero_sistemas * 3

/***( Function prototypes )***********************************************/

static void *pCreadorHilos ( void *arg_ptr );
static void *pNorgas( void *arg );
static void *pModulo_comunicacion( void *arg_ptr );
static void *pControlador ( void *arg_ptr );
static void *pHardware ( void *arg_ptr );
static void *pBaseDatos( void *arg_ptr );

/***( SDL system creation )***********************************************/

int main ( void )
{
  pthread_t   pCreadorHilos_tid;
  pthread_t   pNorgas_tid;
  pthread_t   pBaseDatos_tid;

  /* Create queues */

  initialiseQueues ();

  /* Create threads */
  pthread_create ( &pCreadorHilos_tid, NULL, pCreadorHilos, NULL );
  pthread_create ( &pNorgas_tid, NULL, pNorgas, NULL );
  pthread_create ( &pBaseDatos_tid, NULL, pBaseDatos, NULL );

  /* Wait for threads to finish */
  pthread_join ( pCreadorHilos_tid, NULL );
  pthread_join ( pNorgas_tid, NULL );
  pthread_join ( pBaseDatos_tid, NULL );

  /* Destroy queues */
  destroyQueues ();

  return ( 0 );
}

/***( SDL system processes )**********************************************/

/* Environment thread */
static void *pNorgas ( void *arg )
{
  char  line [100];
  int   activacion,
        systemId,
        userID,
        eleccion_sistema,
        PID_modulo_Comunicacion;

  msg_t OutMsg;

  for ( ; ; )
  {
    printf ( "Digite la identificacion del sistema:\n" );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &systemId );
    printf ( "Digite 1 para activarlo o 0 para desactivarlo:\n" );
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &activacion );
    OutMsg.signal = (int) sActivationFromNorgas;
    OutMsg.value = activacion ;
    OutMsg.SystemId = systemId;
    eleccion_sistema = (systemId);
    PID_modulo_Comunicacion = (eleccion_sistema*3);
    sendMessage ( &(queue [PID_modulo_Comunicacion]), OutMsg );

    printf("Digite la identificacion del usuario:\n");
    fflush ( stdout );
    fflush ( stdin );
    fgets ( line, sizeof (line), stdin );
    sscanf ( line, "%d", &userID );
    OutMsg.signal = (int) sUser;
    OutMsg.value = userID ; //userID
    OutMsg.SystemId = systemId;
    sendMessage ( &(queue [PID_modulo_Comunicacion]), OutMsg );
    }

  return ( NULL );
}

static void *pCreadorHilos ( void *arg )
{
  msg_t             InMsg, OutMsg;
  pthread_t         psystems[Numero_hilos+1];
  int               index,numSystem,Hardware_index,CM_index,Controller_index;
  int               myCM;
  Estados_creador_hilos state_next,state;

  for ( index = 2; index <=  Numero_hilos; index++ )
  {
    pthread_create ( &psystems[index], NULL , pControlador, (void *) &index );
    sleep ( 1 );
    index=index+1;
    pthread_create ( &psystems[index], NULL, pModulo_comunicacion, (void *) &index );
    sleep ( 1 );
    index=index+1;
    pthread_create ( &psystems[index], NULL, pHardware, (void *) &index );
    sleep ( 1 );
  }

  printf ( "\t\t\t\tsReady\n" );                          /* "send" message to environment */
  fflush ( stdout );
  state_next = IdleT;

  for ( ; ; )
  {

    state = state_next;
    InMsg = receiveMessage ( &(queue [creador_hilos]) );
    switch ( state )
    {
      case IdleT:
        switch ( InMsg.signal )
        {
          case sGetId:
            numSystem = InMsg.value;
            if ( ( numSystem >= 1 ) && ( numSystem <= Numero_sistemas ) )
            {
              Hardware_index = (numSystem*3)+1;
              OutMsg.signal = (int) sId_Hardware;
              OutMsg.value = Hardware_index;
              OutMsg.sender = 0;
              myCM = InMsg.sender;
              sendMessage ( &(queue [myCM]), OutMsg );

              CM_index = (numSystem*3);
              OutMsg.signal = (int) sId_CM;
              OutMsg.value = CM_index;
              OutMsg.sender = 0;
              sendMessage ( &(queue [myCM]), OutMsg );

              Controller_index = CM_index-1;
              OutMsg.signal = (int) sId_Controller;
              OutMsg.value = Controller_index;
              OutMsg.sender = 0;
              sendMessage ( &(queue [myCM]), OutMsg );
            }
            else
            {
              OutMsg.signal = (int) sError;
              OutMsg.value = 0;
              OutMsg.sender = 0;
              myCM = InMsg.sender;
              sendMessage ( &(queue [myCM]), OutMsg );   /* send message to SENDER process (pLocal instance) */
            }
            state_next = IdleT;
            break;
          default:
            break;
        }
        break;
        default:
          break;
    }
    for ( index = 1; index <= Numero_hilos; index++ )
    {
      pthread_join ( psystems[index], NULL );
      index = index + 1;
      sleep(1);
      pthread_join ( psystems[index], NULL );
      index = index + 1;
      sleep(1);
      pthread_join ( psystems[index], NULL );
      sleep(1);
    }
  }
return ( NULL );
}


/* pLocal process thread */
static void *pModulo_comunicacion( void *arg_ptr )
{

  Estados_modulo_comunicacion state,
                  state_next;
  msg_t           InMsg,
                  OutMsg;

  int             queueNo,
                  *data_ptr;

  int             Hardware_PID,
                  CM_PID,
                  Controller_PID,
                  whoami,
                  systemId;

  data_ptr = (int *) arg_ptr;
  queueNo = whoami = *data_ptr;

  printf ( "\t\t\t\tEl Modulo de Comunicacion tiene el numero %d\n", whoami );
  fflush ( stdout );

  state_next = Idle;

  int percentage = 0;
  int ON_OFF = 0;
  int Activation = 0;
  int GasOk = 0;

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [queueNo]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\t\tModulo de comunicacion %d received signal %d, value %d in state %d\n", whoami, InMsg.signal, InMsg.value, state );
    fflush ( stdout );

    switch ( state )
    {
      case Idle:
        switch ( InMsg.signal )
        {
          case sActivationFromNorgas:
            ON_OFF = InMsg.value;
            systemId = InMsg.SystemId;
            OutMsg.signal = (int) sGetId;
            OutMsg.value = systemId;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [creador_hilos]), OutMsg ); /* send message to pCentral process */
            state_next = GettingId1;
            break;
          default:
            break;
        }
        break;
      case GettingId1:
        switch ( InMsg.signal )
        {
          case sId_Hardware:
            Hardware_PID = InMsg.value;
            state_next = GettingId2;
            break;
          default:
            break;
        }
        break;
      case GettingId2:
        switch ( InMsg.signal )
        {
          case sId_CM:
            CM_PID = InMsg.value;
            state_next = GettingId3;
            break;
          default:
            break;
        }
        break;
      case GettingId3:
        switch ( InMsg.signal )
        {
          case sId_Controller:
            Controller_PID = InMsg.value;
            OutMsg.signal = (int) sPIDS_Ready;
            OutMsg.value = Hardware_PID;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [Controller_PID]), OutMsg ); /* send message to peer pLocal process */
            state_next = ActivateModule;
            break;
          default:
            break;
        }
        break;
      case ActivateModule:
        switch ( InMsg.signal )
        {
          case sStart:

            if(ON_OFF)
            {
              Activation = 1;

            }
            else
            {
              Activation = 0;
            }
            OutMsg.signal = (int) sActivation;
            OutMsg.value = Activation;
            sendMessage ( &(queue [Controller_PID]), OutMsg );
            state_next = Verify_SystemForActivation;
            break;
          default:
            break;

        }
        break;

      case Verify_SystemForActivation:
        switch ( InMsg.signal )
        {
          case sActivationOk:
            OutMsg.signal = (int) sActivationState;
            OutMsg.value = 1; // ON
            OutMsg.SystemId = systemId; //System Id
            sendMessage(&(queue [Base_datos]), OutMsg);
            state_next = GetClientData;
            break;

          case sActivationNotOK:
            OutMsg.signal = (int) sActivationState;
            OutMsg.value = 0; // OFF
            OutMsg.value = systemId; //System Id
            sendMessage(&(queue [Base_datos]), OutMsg);
            state_next = ActivateModule;
            break;

          default:
            break;
        }
        break;

      case GetClientData:
        switch ( InMsg.signal )
        {
          case sUser:
            OutMsg.signal = (int) sUserDataStorage;
            OutMsg.value = InMsg.value; //UserID
            OutMsg.SystemId = InMsg.SystemId;//SystemId
            sendMessage(&(queue [Controller_PID]), OutMsg);
            state_next = gasLevelConfirmation;
            break;
          default:
            break;
        }
        break;

      case gasLevelConfirmation:
        switch ( InMsg.signal )
        {
          case sGasLevel:
            percentage = InMsg.value;
            if(percentage < 100)
            {
              GasOk = 1;
            }
            else
            {
              GasOk = 0;
            }
            OutMsg.signal = (int) sGasLevelOk;
            OutMsg.value = GasOk;
            sendMessage(&(queue [Controller_PID]), OutMsg);
            state_next = dataSending;
            break;
        }
        break;

      case dataSending:
        switch ( InMsg.signal )
        {
          case sUser:
            OutMsg.signal = (int) sData;
            OutMsg.value = InMsg.value; //UserID
            OutMsg.SystemId = InMsg.SystemId; //systemID
            OutMsg.sender = percentage; //GasLevel
            sendMessage(&(queue [Base_datos]), OutMsg);
            OutMsg.signal = sDataOk;
            OutMsg.value = 1;
            sendMessage(&(queue [Controller_PID]), OutMsg);
            state_next = Hybernation;
            break;
        }
        break;

      case Hybernation:
        switch ( InMsg.signal )
        {
          case sSleep:

            state_next = Ready;
            break;
        }
        break;

      case Ready:
        switch ( InMsg.signal )
        {
          case sAwake:
            //InMsg.signal = WakeUp;
            OutMsg.signal = sAwake;
            sendMessage(&(queue [Controller_PID]), OutMsg);
            state_next = gasLevelConfirmation;
            break;
        }
        break;

    }
    /* show next state */
    printf ( "\t\t Modulo de comunicacion %d siguiente estados es %d\n", whoami, state_next );
    fflush ( stdout );
  }

  return ( NULL );
}
static void *pControlador ( void *arg_ptr )
{

  Estados_Controlador state,
  state_next;
  msg_t           InMsg,
  OutMsg;

  int             queueNo,
  *data_ptr;

  int             Hardware_PID,
  CM_PID,
  whoami,
  systemId,
  UserID,
  percentage,
  CylinderWeight,
  Activation,
  GasOk_C;

  data_ptr = (int *) arg_ptr;
  queueNo = whoami = *data_ptr;
  printf ( "\t\t\t\tEl controlador tiene el numero %d...\n", whoami );
  fflush ( stdout );

  state_next = StartSystem;

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [queueNo]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\t\tControlador %d received signal %d, value %d in state %d\n", whoami, InMsg.signal, InMsg.value, state );
    fflush ( stdout );

    switch ( state )
    {
      case StartSystem:
        switch ( InMsg.signal )
        {
          case sPIDS_Ready:
            Hardware_PID = InMsg.value;
            CM_PID = Hardware_PID - 1;
            OutMsg.signal = (int) sStart;
            OutMsg.value = 0;
            OutMsg.sender = whoami;
            sendMessage ( &(queue [CM_PID]), OutMsg ); /* send message to pCentral process */
            state_next = System_Activation;
            break;
          default:
            break;
        }
        break;

      case System_Activation:
        switch ( InMsg.signal )
        {
          case sActivation:
            Activation = InMsg.value;
            if (Activation)
            {
              printf("SYSTEM ON\n");
              OutMsg.signal = (int) sActivationOk;
              sendMessage ( &(queue [CM_PID]), OutMsg );
              state_next = GetClientParameters_and_getReady;
            }
            else
            {
              printf("SYSTEM OFF\n");
              OutMsg.signal = (int) sActivationNotOK;
              sendMessage ( &(queue [CM_PID]), OutMsg );
              state_next = System_Activation;
            }
            break;
          default:
            break;
        }
        break;

      case GetClientParameters_and_getReady:
          switch ( InMsg.signal )
          {
            case sUserDataStorage:
              UserID = InMsg.value; //UserID
              systemId = InMsg.SystemId; //SystemID
              OutMsg.signal = (int) sReceiveWeight;
              OutMsg.value = 0;
              sendMessage ( &(queue [Hardware_PID]), OutMsg );
              state_next = WorkingCycle;
              break;
            default:
              break;
          }
          break;


      case WorkingCycle:
        switch ( InMsg.signal )
        {
          case sWeight:
            printf("Ha iniciado el ciclo de trabajo\n");
            CylinderWeight = InMsg.value;
            percentage = CylinderWeight*100/(Full_tank_weight);
            OutMsg.signal = (int) sGasLevel;
            OutMsg.value = percentage;
            sendMessage ( &(queue [CM_PID]), OutMsg );
            printf("El porcentaje de llenado es del: %d\n",percentage);
            if (percentage < 20)
            {
              OutMsg.signal = (int)sLed2User;
              OutMsg.value = 1;
              sendMessage ( &(queue [Hardware_PID]), OutMsg );

            }
            state_next = checkGasLevelOk;
            break;
          default:
            break;
        }
        break;

      case checkGasLevelOk:
        switch ( InMsg.signal )
        {
          case sGasLevelOk:
            GasOk_C = InMsg.value;
            if (GasOk_C)
            {
              OutMsg.signal = (int) sUser;
              OutMsg.value = UserID; //UserID
              OutMsg.SystemId = systemId; //SystemID
              sendMessage ( &(queue [CM_PID]), OutMsg );
              printf("Toma de datos OK\n");
              state_next = sentDataConfirmation;
            }
            else
            {
              printf("Repitiendo toma de datos\n");
              state_next = WorkingCycle;
            }
            break;
          default:
            break;
        }
        break;

      case sentDataConfirmation:
        switch ( InMsg.signal )
        {
          case sDataOk:
            OutMsg.signal = (int) sSleep;
            OutMsg.value = 1;
            sendMessage ( &(queue [CM_PID]), OutMsg ); /* send message to peer pLocal process */
            state_next = Waiting;
            break;
          default:
            break;
        }
        break;

        case Waiting:
          switch ( InMsg.signal )
          {
            case sAwake:
              state_next = NewIteration;
              OutMsg.signal = (int) sSleep;
              OutMsg.value = 1;
              sendMessage ( &(queue [CM_PID]), OutMsg ); /* send message to peer pLocal process */
              state_next = Waiting;
              break;
            default:
              break;
          }
          break;

        // Falta Nueva Iteracion
        case NewIteration:
          switch ( InMsg.signal )
          {
            case sAwake:
              state_next = NewIteration;
              OutMsg.signal = (int) sSleep;
              OutMsg.value = 1;
              sendMessage ( &(queue [CM_PID]), OutMsg ); /* send message to peer pLocal process */
              state_next = Waiting;
              break;
            default:
              break;
          }
          break;


    }
  /* show next state */
  printf ( "\t\t Controlador %d siguiente estados es %d\n", whoami, state_next );
  fflush ( stdout );
  }
  return ( NULL );

}

static void *pHardware ( void *arg_ptr )
{

  Estados_Hardware state, state_next;

  msg_t           InMsg,
  OutMsg;

  int queueNo, whoami;
  int *data_ptr;
  int myController = 0;
  int CylinderWeight = 10;

  data_ptr = (int *) arg_ptr;
  queueNo = whoami = *data_ptr;

  printf ( "\t\t\t\tEl hardware tiene el numero %d...\n", whoami );
  fflush ( stdout );

  state_next = StartSystem;

  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [queueNo]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\t\tHardware received signal %d, value %d in state %d\n",InMsg.signal, InMsg.value, state );
    fflush ( stdout );

    switch ( state )
    {
      case IdleH:
        switch ( InMsg.signal )
        {
          case sReceiveWeight:
            myController = InMsg.sender;
            OutMsg.signal = (int) sWeight;
            OutMsg.value = CylinderWeight;
            sendMessage ( &(queue [myController]), OutMsg );
            printf("El peso actual del cilindro es: %d\n",CylinderWeight);
            state_next = END;
            break;
          default:
            break;
        }
        break;

      case END:
        printf("Hardware en END");
        break;
      default:
        break;


    }
  /* show next state */
  printf ( "\t\t Hardware siguiente estado es %d\n", state_next );
  fflush ( stdout );
  }
  return ( NULL );
}
static void *pBaseDatos( void *arg_ptr )
{

  Estados_base_datos state, state_next;
  msg_t           InMsg;
  int systemID,ActivationState;
  state_next = Activation_DataBase;
  for ( ; ; )
  {
    state = state_next;
    InMsg = receiveMessage ( &(queue [Base_datos]) );

    /* show which message (signal name and value) was received in current state */
    printf ( "\t\tBaseDatos received signal %d, value %d in state %d\n",InMsg.signal, InMsg.value, state );
    fflush ( stdout );

    switch ( state )
    {
      case Activation_DataBase:
        switch ( InMsg.signal )
        {
          case sActivationState:
            ActivationState = InMsg.value ;
            systemID = InMsg.SystemId;// variable identificador del sistema
            printf("El sistema %d tiene su estado de activacion en %d\n",ActivationState,systemID);
            state_next = SendMessage;
            break;
          default:
            break;
        }
        break;


      case SendMessage:
        switch ( InMsg.signal )
        {

          case sData:

            //InMsg.value = message;
            printf("El sistema %d del usuario %d tiene un nivel del %d porciento\n",InMsg.SystemId,InMsg.value,InMsg.sender);
            state_next = Activation_DataBase;
            break;
          default:
            break;
        }
        break;
    }
    /* show next state */
    printf ( "\t\t Base_datos siguiente estado es %d\n", state_next );
    fflush ( stdout );
  }

  return ( NULL );

}
