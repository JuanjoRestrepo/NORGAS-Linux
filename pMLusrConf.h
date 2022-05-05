/*******************************************************************************
*
*  pMLusrConf.h -   Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue
*
*   Notes:          User defined according to application
*
*******************************************************************************/

/***( Manifest constants for user-defined queuing system  )********************/

#define     BUFSIZE       8   /* number of slots in queues */
#define     NUM_QUEUES    4   /* number of queues */
#define     CONTROLLER_Q  0   /* queue 0: controller */
#define     COMMUNICATION_MODULE      1   /* queue 1: water dispenser */
#define     DATA_BASE     2   /* queue 2: coffee dispenser */
#define     TIMER_Q       3   /* queue 4: timer */

/***( Macros to manage symbolic (literal) enumerated values )******************/

#define     GENERATE_ENUM(ENUM)     ENUM,
#define     GENERATE_STRING(STRING) #STRING,

/***( User-defined message structure )*****************************************/

typedef struct
{
  int   signal;
  int   value;
} msg_t;



/***( User-defined signals )****************************************************/

/* Signals sent to pController3 */

#define FOREACH_TO_CONTROLLER(SIGNAL) \
        SIGNAL(sActivation) \
        SIGNAL(sUser1) \
        SIGNAL(sWeightFromUser) \
        SIGNAL(sGasLevelOk) \
        SIGNAL(sDataOk)

typedef enum
{
  FOREACH_TO_CONTROLLER(GENERATE_ENUM)
} TO_CONTROLLER_ENUM;

static const char *TO_CONTROLLER_STRING[] =
{
  FOREACH_TO_CONTROLLER(GENERATE_STRING)
};

/* Signals sent to pHardware2 */

#define FOREACH_TO_COMMUNICATION_MODULE(SIGNAL) \
        SIGNAL(sActivationFromNorgas)   \
        SIGNAL(sActivationOk)  \
        SIGNAL(sGasLevel) \
        SIGNAL(sUser2) \
        SIGNAL(sSleep) \
        SIGNAL (sWakeUp)

typedef enum
{
  FOREACH_TO_COMMUNICATION_MODULE(GENERATE_ENUM)
} TO_COMMUNICATION_MODULE_ENUM;

static const char *TO_COMMUNICATION_MODULE[] =
{
  FOREACH_TO_COMMUNICATION_MODULE(GENERATE_STRING)
};

//Signals Sent_To_Data_Base

#define FOREACH_TO_DATA_BASE(SIGNAL) \
        SIGNAL(sActivationState)  \
        SIGNAL(sData)

typedef enum
{
  FOREACH_TO_DATA_BASE(GENERATE_ENUM)
} TO_DATA_BASE_ENUM;

static const char *TO_DATA_BASE[] =
{
  FOREACH_TO_DATA_BASE(GENERATE_STRING)
};


/* Signals sent to pTimer */

#define FOREACH_TO_TIMER(SIGNAL) \
        SIGNAL(sSetHeater)   \
        SIGNAL(sTexpired) \
        SIGNAL(sResetHeater)

typedef enum
{
  FOREACH_TO_TIMER(GENERATE_ENUM)
} TO_TIMER_ENUM;

static const char *TO_TIMER_STRING[] =
{
  FOREACH_TO_TIMER(GENERATE_STRING)
};

/***( User-defined EFSM states )************************************************/

/* EFSM states for pController3 */

#define FOREACH_CONTROLLER_STATE(STATE) \
        STATE(System_Activation)   \
        STATE(Get_ClientParameters)  \
        STATE(WorkingCycle)  \
        STATE(CheckGasLevelOK)  \
        STATE(SentDataConfirmation) \
        STATE(END)

typedef enum
{
  FOREACH_CONTROLLER_STATE(GENERATE_ENUM)
} CONTROLLER_STATE_ENUM;

static const char *CONTROLLER_STATE_STRING[] =
{
  FOREACH_CONTROLLER_STATE(GENERATE_STRING)
};


/* EFSM states for COMMUNICATION_MODULE */

#define FOREACH_COMMUNICATION_MODULE_STATE(STATE) \
        STATE(ActivateModule) \
        STATE(VerifySystemForActivation) \
        STATE(GasLevelConfirmation) \
        STATE(DataSent) \
        STATE(Hybernation)

typedef enum
{
  FOREACH_COMMUNICATION_MODULE_STATE(GENERATE_ENUM)
} COMMUNICATION_MODULE_STATE_ENUM;

static const char *COMMUNICATION_MODULE_STATE_STRING[] =
{
  FOREACH_COMMUNICATION_MODULE_STATE(GENERATE_STRING)
};

/* EFSM states for DATA_BASE */

#define FOREACH_DATA_BASE_STATE(STATE) \
        STATE(Activation_DataBase) \
        STATE(sendMessage_State)

typedef enum
{
  FOREACH_DATA_BASE_STATE(GENERATE_ENUM)
} DATA_BASE_STATE_ENUM;

static const char *DATA_BASE_STATE_STRING[] =
{
  FOREACH_DATA_BASE_STATE(GENERATE_STRING)
};


/* EFSM states for pTimer */

#define FOREACH_TIMER_STATE(STATE) \
        STATE(IdleT) \
        STATE(CheckTimeout) \
        STATE(WastingTime)

typedef enum
{
  FOREACH_TIMER_STATE(GENERATE_ENUM)
} TIMER_STATE_ENUM;

static const char *TIMER_STATE_STRING[] =
{
  FOREACH_TIMER_STATE(GENERATE_STRING)
};
