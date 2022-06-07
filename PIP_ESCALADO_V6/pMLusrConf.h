/*******************************************************************************
*
*  msgq_usr.h - Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue
*
*   Notes:          User defined according to application
*
*******************************************************************************/

/***( Manifest constants for user-defined queuing system  )********************/

#define     BUFSIZE     8     /* number of slots in queues */
#define     NUM_QUEUES  8      /* number of queues */
#define     creador_hilos   0     /* queue 0: creador de hilos process */
#define     Base_datos    1     /* queue 1: Base de datos process */
#define     Hardware_1    2     /* queue 2: Hardware 1 process */
#define     Controlador_1    3     /* queue 3: Controlador 1 process */
#define     Modulo_comunicacion_1    4     /* queue 4: Modulo de comunicacion 1 process */
#define     Hardware_2    5     /* queue 5:  Hardware 2 process */
#define     Controlador_2    6     /* queue 6: Controlador 2 process */
#define     Modulo_comunicacion_2    7     /* queue 7: Modulo de comunicacion 2 */
/***( User-defined message structure )*****************************************/

typedef struct
{
  int   signal;
  int   value;
  int   sender;
  int SystemId;
} msg_t;

/***( User-defined signals )****************************************************/

typedef enum
{
  sPIDS_Ready,
  sActivation,
  sGasLevelOk,
  sDataOk,
  sUserDataStorage,
  sWeight,
  sAwake,
  sActivationNotOK
} TO_Controlador;                             /* Señales que llegan al controlador */

typedef enum
{
  sReceiveWeight,
  sLed2User
} TO_Hardware;                               /* Señales que llegan al hardware */


typedef enum
{
  sActivationFromNorgas,
  sId_Hardware,
  sId_CM,
  sId_Controller,
  sStart,
  sActivationOk,
  sUser,
  sGasLevel,
  sUser_1,
  sSleep,
  sError
} TO_M_comunicacion;                               /* Señales que llegan al modulo de comunicacion */

typedef enum
{
  sActivationState,
  sData
} TO_Base_datos;                               /* Señales que llegan a la base de datos*/

typedef enum
{
  sGetId
} TO_creador_hilos;                               /* Señales que llegan al creador de hilos*/




/***( User-defined EFSM states )************************************************/

typedef enum
{
  StartSystem,
  System_Activation,
  GetClientParameters_and_getReady,
  WorkingCycle,
  checkGasLevelOk,
  sentDataConfirmation,
  Waiting,
  NewIteration
} Estados_Controlador;                            /* Estados del controlador */

typedef enum
{
  IdleH,
  END
} Estados_Hardware;                            /* Estados del hardware */

typedef enum
{
  Idle,
  GettingId1,
  GettingId2,
  GettingId3,
  ActivateModule,
  Verify_SystemForActivation,
  GetClientData,
  gasLevelConfirmation,
  dataSending,
  Hybernation,
  Ready
} Estados_modulo_comunicacion;                            /* Estados del modulo_comunicacion */

typedef enum
{
  Activation_DataBase,
  SendMessage
} Estados_base_datos;   /* Estados de la base de datos */


typedef enum
{
  IdleT
} Estados_creador_hilos;   /* Estados del creador de hilos*/
