#include "types.h"

#define MIN_TO_MS(X) (X * 1000UL * 60UL)
#define SEC_TO_MS(X) (X * 1000UL)
#define SWITCH_PERIOD SEC_TO_MS(5UL)

#define NUM_OF_RELAYS 4U
#define RELAY_K1_PIN  2U
#define RELAY_K2_PIN  3U
#define RELAY_K3_PIN  4U
#define RELAY_K4_PIN  5U

typedef enum eRelay
{
  eRELAY_K1 = 0,
  eRELAY_K2,
  eRELAY_K3,
  eRELAY_K4,
  // new relays above this line
  eRELAY_COUNT
}eRELAY;

typedef struct xRelayConfig
{
  uint8 u8EnablePin;
  uint8 u8TriggerPin;
}xRELAY_CONFIG;

static xRELAY_CONFIG gxLookUpRelayConfig[eRELAY_COUNT]
{
  { 10U, 2U }, //eRELAY_K1
  { 11U, 3U }, //eRELAY_K2
  { 12U, 4U }, //eRELAY_K3
  { 13U, 5U }  //eRELAY_K4
};

#define PERIOD_POT_PIN 0U

#define RELAY_K1_PIN_EN_PIN 10U
#define RELAY_K2_PIN_EN_PIN 11U
#define RELAY_K3_PIN_EN_PIN 12U
#define RELAY_K4_PIN_EN_PIN 13U

#define DIPSWITCH_PIN_B0 6U
#define DIPSWITCH_PIN_B1 7U
#define DIPSWITCH_PIN_B2 8U
#define DIPSWITCH_PIN_B3 9U

#define CHARGER_CONNECTED LOW
#define CHARGER_DISCONNECTED HIGH

#define RELAY_K1_ON (1 << 0)
#define RELAY_K2_ON (1 << 1)
#define RELAY_K3_ON (1 << 2) 
#define RELAY_K4_ON (1 << 3)

unsigned long getTimeDifference(unsigned long t0, unsigned long t1);
unsigned char getEnabledMask(void);
void updateOutputs (unsigned char outputStateMask);

typedef enum relays
{
  RELAY_K1,
  RELAY_K2,
  RELAY_K3,
  RELAY_K4
};

unsigned char outputStateMask;
unsigned long startTimeStamps[NUM_OF_RELAYS];
unsigned long timeSincePowerUp = 1;
unsigned long timeDifference;
unsigned long currentTime;
unsigned long switchPeriod;

const unsigned char RelayPins [NUM_OF_RELAYS][2] = 
{
  {RELAY_K1_PIN,RELAY_K1_PIN_EN_PIN},
  {RELAY_K2_PIN,RELAY_K2_PIN_EN_PIN},
  {RELAY_K3_PIN,RELAY_K3_PIN_EN_PIN},
  {RELAY_K4_PIN,RELAY_K4_PIN_EN_PIN}
};

void setup() 
{
  pinMode(RELAY_K1_PIN, OUTPUT);
  pinMode(RELAY_K2_PIN, OUTPUT);
  pinMode(RELAY_K3_PIN, OUTPUT);
  pinMode(RELAY_K4_PIN, OUTPUT);

  pinMode(RELAY_K1_PIN_EN_PIN,INPUT);
  pinMode(RELAY_K2_PIN_EN_PIN,INPUT);
  pinMode(RELAY_K3_PIN_EN_PIN,INPUT);
  pinMode(RELAY_K4_PIN_EN_PIN,INPUT);

  pinMode(PERIOD_POT_PIN,INPUT);

  /*Time selector pins*/
  pinMode(DIPSWITCH_PIN_B0,INPUT);
  pinMode(DIPSWITCH_PIN_B1,INPUT);
  pinMode(DIPSWITCH_PIN_B2,INPUT);
  pinMode(DIPSWITCH_PIN_B3,INPUT);
  
  digitalWrite(RELAY_K4_PIN,CHARGER_DISCONNECTED);
  digitalWrite(RELAY_K3_PIN,CHARGER_DISCONNECTED);
  digitalWrite(RELAY_K2_PIN,CHARGER_DISCONNECTED);//default
  digitalWrite(RELAY_K1_PIN,CHARGER_CONNECTED);
  
  Serial.println("K1 ON");
  startTimeStamps[RELAY_K1] = millis();
  outputStateMask = RELAY_K1_ON;
  
  Serial.begin(9600);
}

void loop() 
{

  currentTime = millis();
  switchPeriod = getSwitchPeriod();

  //Serial.println(switchPeriod);
  
  switch (outputStateMask)
    {
      case (RELAY_K1_ON):
        timeDifference = getTimeDifference(startTimeStamps[RELAY_K1], currentTime);
        
        if (timeDifference >= switchPeriod)
        {
          digitalWrite(RELAY_K1_PIN,CHARGER_DISCONNECTED);
          startTimeStamps[RELAY_K1] = 0;
          updateOutputs();
        }
      break;
      
      case (RELAY_K2_ON):
        timeDifference = getTimeDifference(startTimeStamps[RELAY_K2], currentTime);

        if (timeDifference >= switchPeriod)
        {
          digitalWrite(RELAY_K2_PIN,CHARGER_DISCONNECTED);
          startTimeStamps[RELAY_K2] = 0;
          updateOutputs();
        }
      break;
  
      case (RELAY_K3_ON):
        timeDifference = getTimeDifference(startTimeStamps[RELAY_K3], currentTime);
        
        if (timeDifference >= switchPeriod)
        {
          digitalWrite(RELAY_K3_PIN,CHARGER_DISCONNECTED);
          startTimeStamps[RELAY_K3] = 0;
          updateOutputs();
        }
      break;
  
      case (RELAY_K4_ON):
        timeDifference = getTimeDifference(startTimeStamps[RELAY_K4], currentTime);
        
        if (timeDifference >= switchPeriod)
        {
          digitalWrite(RELAY_K4_PIN,CHARGER_DISCONNECTED);
          startTimeStamps[RELAY_K4] = 0;
          updateOutputs();
        }
        
      break;
    }
}
  
void updateOutputs ()
{
  unsigned char enabledMask;
  enabledMask = getEnabledMask();
  
  while (1)
  {  
    if ((outputStateMask << 1) <=  RELAY_K4_ON)
    {
      outputStateMask = (outputStateMask << 1);
    }
    else
    {
      outputStateMask = 1;
    }
        
    if ((outputStateMask & enabledMask) != 0)
    {
      switch (outputStateMask)
      {
        case (RELAY_K1_ON):
          Serial.println("K1 ON ");
          digitalWrite(RELAY_K1_PIN,CHARGER_CONNECTED);
          startTimeStamps[RELAY_K1] = millis();
        break;

        case (RELAY_K2_ON):
          Serial.println("K2 ON ");
          digitalWrite(RELAY_K2_PIN,CHARGER_CONNECTED);
          startTimeStamps[RELAY_K2] = millis();
        break;

        case (RELAY_K3_ON):
          Serial.println("K3 ON ");
          digitalWrite(RELAY_K3_PIN,CHARGER_CONNECTED);
          startTimeStamps[RELAY_K3] = millis();
        break;

        case (RELAY_K4_ON):
          Serial.println("K4 ON ");
          digitalWrite(RELAY_K4_PIN,CHARGER_CONNECTED);
          startTimeStamps[RELAY_K4] = millis();
        break;
      }
      
    }
    return;
  }
}

unsigned char getEnabledMask(void)
{
  int i;
  unsigned char enabledMask = 0;

  eRELAY eWhichRelay;

  for( eWhichRelay = (eRELAY) 0; eWhichRelay < eRELAY_COUNT; eWhichRelay + 1U )
  {
    if( digitalRead( gxLookUpRelayConfig[eWhichRelay].u8EnablePin == HIGH ) )
    {
      enabledMask |= ( 1 << i );
      Serial.println( eWhichRelay );
    }
  }

  return enabledMask;
}

unsigned long getTimeDifference(unsigned long t0, unsigned long t1)
{
  if (t1 >= t0)
  {
    return (t1 - t0);
  }
  else //just rolled over
  {
    return ((4294967295 - t0) + t1);
  }
}

unsigned long getSwitchPeriod (void)
{

  unsigned char dipSwitchInput = 0;
  
  dipSwitchInput |= (digitalRead(DIPSWITCH_PIN_B3) << 3);
  dipSwitchInput |= (digitalRead(DIPSWITCH_PIN_B2) << 2);
  dipSwitchInput |= (digitalRead(DIPSWITCH_PIN_B1) << 1);
  dipSwitchInput |= digitalRead(DIPSWITCH_PIN_B0);
  
  if( dipSwitchInput == 0 )
  {
    return SEC_TO_MS(1U);
  }
  else
  {
    return MIN_TO_MS(60U * dipSwitchInput);
  }
}
