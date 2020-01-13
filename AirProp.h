#include <wiringPi.h>

#define MIN_PROPELLER_SPEED 0.0 //minimum propeller speed (i.e. stopped)
#define MAX_PROPELLER_SPEED 10.0 //arbitrary units
#define STOP_PULSE_TIME 1000 //pulse time required for sending a "stop" signal to the propeller
#define MAX_PULSE_TIME 1500 //maximum pulse time in microseconds, corresponds to maximum propeller speed
#define PWM_CLK_DIV 64 //divisor to use for 19.2 MHz PWM clock
#define PWM_CLK_FREQ 19200000 //19.2 MHz PWM clock frequency
#define TIME_INCREMENT_US 3.33 //timing for data portion of clock pulses has this resolution in microseconds
//#define DEAD_BAND 25 // +/- value (in uS) around STOP_PULSE_TIME for which the propeller will not be moved
#define PROP_PWM_PIN 41  //gpio pin to use for PWM for propeller (requires Alt0 alternative pin function)

class AirProp {
public:
	AirProp();//AirProp constructor
	~AirProp();
	bool isInSafetyMode();//returns true if the propeller is currently turned off and in safety mode, otherwise returns false
	void SetObstacleSafetyMode(bool bSafetyMode);//function used for disabling propeller, for example in the event that an obstacle is nearby.
	void Initialize();//initializes propeller by sending a "stopped signal".  Calling program should then pause for a few seconds to make sure that initialization has properly occurred.
	void SetSpeed(float fSpeed);//set speed of propeller (fSpeed is in arbitrary units, must be between MIN_SPEED and MAXSPEED).
	void Stop();//send a "stopped signal" to the propeller
	float GetSpeed();//get the current speed of the propeller
	static void OneTimeSetup();//setup procedure to be performed once per power-cycle of the propeller
	void DoMaxSpeedCal();//apply maximum speed signal to propeller
		 
private:
	bool m_bCalMode;//set to true whenever a calibration is being performed
	bool m_bSafetyMode;//if this flag is true, then propeller should not be turned on
	float m_fSpeed;//the speed of the propeller (arbitrary units)
};