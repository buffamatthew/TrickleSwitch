#define CHARGER_CONNECTED ( LOW )
#define CHARGER_DISCONNECTED ( HIGH )
#define MIN_TO_MS( X ) ( X * 1000UL * 60UL )
#define SEC_TO_MS( X ) ( X * 1000UL )
#define SWITCH_PERIOD SEC_TO_MS( 5UL )
//#define DEBUG

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
  const unsigned char u8EnablePin;
  const unsigned char u8OutputPin;
}xRELAY_CONFIG;

static xRELAY_CONFIG gxLookUpRelayConfig[eRELAY_COUNT]
{
  { 10U, 6U }, //eRELAY_K1
  { 11U, 7U }, //eRELAY_K2
  { 12U, 8U }, //eRELAY_K3
  { 13U, 9U }  //eRELAY_K4
};

typedef enum eDipSwitchPins
{
  eDIP_SWITCH_PIN_B0,
  eDIP_SWITCH_PIN_B1,
  eDIP_SWITCH_PIN_B2,
  eDIP_SWITCH_PIN_B3,
  eDIP_SWITCH_PIN_COUNT
}eDIP_SWITCH_PINS;

static unsigned char gxLookUpDipSwitchConfig[eDIP_SWITCH_PIN_COUNT]
{
  2U, // eDIP_SWITCH_PIN_B0
  3U, // eDIP_SWITCH_PIN_B1
  4U, // eDIP_SWITCH_PIN_B2
  5U  // eDIP_SWITCH_PIN_B2
};

static unsigned long gulCurrentTime;
static unsigned long gulSwitchPeriod;
static unsigned long gulTimeOfLastSwitch = 0;

static unsigned int i = 0;

static unsigned char gu8CurrentSwitchState;
static unsigned char gu8LastSwitchState;
static unsigned char gu8LastRelayOn = eRELAY_COUNT;

static void vTurnOfAllRelays(void);
static unsigned char u8GetEnableSwitchState(void);
static unsigned long ulGetSwitchPeriodMs( void );
static void vTurnOffAllRelays(void);

void setup() 
{
#ifdef DEBUG
  Serial.begin( 9600 );
#endif

  // put your setup code here, to run once:
  unsigned int i;

  for( i = 0; i < eRELAY_COUNT; i++ )
  {
    //initialize all relay enable switch (inputs) and outputs
    pinMode( gxLookUpRelayConfig[i].u8EnablePin, INPUT );
    pinMode( gxLookUpRelayConfig[i].u8OutputPin, OUTPUT );
    digitalWrite( gxLookUpRelayConfig[i].u8OutputPin, CHARGER_DISCONNECTED );
  }

  for( i = 0; i < eDIP_SWITCH_PIN_COUNT; i++ )
  {
    pinMode( gxLookUpDipSwitchConfig[i], INPUT );
  }

  gu8CurrentSwitchState = 0xFF;
  
  gu8LastSwitchState = 0;
  // initialize gu8LastSwitchState to 0 so that if one the first loop through, the switch state
  // is anything but 0, the output will change immediatley
}

unsigned char u8GetEnableSwitchState( void )
{
  unsigned int i;
  unsigned char u8EnabledMask = 0;

  for( i = 0; i < eRELAY_COUNT; i++ )
  {
    if( digitalRead( gxLookUpRelayConfig[i].u8EnablePin ) == HIGH )
    {
      u8EnabledMask |= ( 1 << (int)(i) );
    }
  }
  
  return u8EnabledMask;
}

void loop() 
{
  gu8CurrentSwitchState = u8GetEnableSwitchState();

  if( gu8CurrentSwitchState != gu8LastSwitchState ) // handles sudden changes of 0 relays enabled to some enabled, or some enabled to 0 enabled.. don't want to wait for time to expire
  {
    // switch state just changed
    
    if( 0U == gu8CurrentSwitchState )
    {
      // changing from at least 1 relay enabled to 0 relays enabled
      vTurnOffAllRelays();
    }
    else if( 0U == gu8LastSwitchState )
    {
      // changing from 0 relays enabled to at least 1 relay enabled..
      for( unsigned int i = 0; i < eRELAY_COUNT; i++ ) // just find the first relay enabled and turn it on
      {
        if( digitalRead( gxLookUpRelayConfig[ i ].u8EnablePin ) == HIGH )
        {
          digitalWrite( gxLookUpRelayConfig[ i ].u8OutputPin, CHARGER_CONNECTED ); // all relays previously off, so don't need to turn any off
          gu8LastRelayOn = i;

#ifdef DEBUG
          Serial.print("Relay ON = " );
          Serial.println( gu8LastRelayOn );
#endif          
          gulTimeOfLastSwitch = millis();
          break;
        }
      }
    }
    
    gu8LastSwitchState = gu8CurrentSwitchState;
  }
  
  if( ( millis() - gulTimeOfLastSwitch ) >= ulGetSwitchPeriodMs() ) // handles periodic switching
  {
    // time to switch
    gulTimeOfLastSwitch = millis();

#ifdef DEBUG
    Serial.println( "Checking if we need to switch" );
#endif          
    
    i = 0;
    while( i < ( eRELAY_COUNT - 1) )
    {
      if( digitalRead( gxLookUpRelayConfig[ ( gu8LastRelayOn + i + 1 ) % eRELAY_COUNT ].u8EnablePin ) == HIGH ) // loops through the last relay off + 1 (next relay in sequence) to find which one to turn on next
      {
        vTurnOffAllRelays();
        
        digitalWrite( gxLookUpRelayConfig[ ( gu8LastRelayOn + i + 1 ) % eRELAY_COUNT ].u8OutputPin, CHARGER_CONNECTED );
        
        gu8LastRelayOn = ( ( gu8LastRelayOn + i + 1 ) % eRELAY_COUNT );

#ifdef DEBUG
        Serial.print("Relay ON = " );
        Serial.println( gu8LastRelayOn );
#endif          
        break;
      }
      
      i++;
    }
  }

  delay(100);
}

unsigned long ulGetSwitchPeriodMs( void )
{
  unsigned char dipSwitchInput = 0;
  unsigned int i;
  
  for( i = 0; i < eDIP_SWITCH_PIN_COUNT; i++ )
  {
    dipSwitchInput |= ( digitalRead( gxLookUpDipSwitchConfig[i] ) << i );
  }
  
  if( dipSwitchInput == 0 )
  {
    return SEC_TO_MS( 1U ); // test mode.. dip switch is set to 0, set to 1 second period
  }
  else
  {
    
#ifdef DEBUG
    return SEC_TO_MS( 1U * dipSwitchInput ); // an hour is too long to wait for debugging
#else
    return MIN_TO_MS( 60U * dipSwitchInput );
#endif
  }
}

static void vTurnOffAllRelays(void)
{
  for( unsigned int j = 0; j < eRELAY_COUNT; j++ ) // turn off all relays
  {
    digitalWrite(gxLookUpRelayConfig[ j ].u8OutputPin, CHARGER_DISCONNECTED );
  }

#ifdef DEBUG
  Serial.println( "All relays off" );
#endif
  
  delay( 100 );// give some time for the relays to settle..
}
