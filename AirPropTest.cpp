#include "../RemoteControlTest/AirProp.h" //"RemoteControlTest" is folder where AMOS source code is located
#include "../RemoteControlTest/AToD.h"
#include "../RemoteControlTest/ShipLog.h"
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <memory>

//example program that lets you test out the air propeller connected to an electronic speed controller (ESC)

#define TEST_DURATION 60 //duration of thruster test in seconds
#define ATOD_ADDRESS 0x48 //I2C address of AtoD

using namespace std;

//isDurationFlagPresent: return true if a duration flag (-d) was specified in the program arguments
//argc: the number of program arguments
//argv: an array of character pointers that corresponds to the program arguments
//returns true if a duration flag is present, otherwise returns false
bool isDurationFlagPresent(int argc, char * argv[]) {
	for (int i=0;i<argc;i++) {
		if (strlen(argv[i])<2) continue;
		if (strncmp(argv[i],"-d",2)==0) {
			return true;
		}
		else if (strncmp(argv[i],"-D",2)==0) {
			return true;
		}
	}
	return false;
}


//isCalFlagPresent: return true if a calibration flag was specified in the program arguments
//argc: the number of program arguments
//argv: an array of character pointers that corresponds to the program arguments
//returns true if a calibration flag is present, otherwise returns false
bool isCalFlagPresent(int argc, char * argv[]) {
	for (int i=0;i<argc;i++) {
		if (strlen(argv[i])<2) continue;
		if (strncmp(argv[i],"-c",2)==0) {
			return true;
		}
		else if (strncmp(argv[i],"-C",2)==0) {
			return true;
		}
	}
	return false;
}

//GetDuration: Gets the duration (in seconds) specified by the user in the program arguments
//fDuration: the duration (in seconds) that the user specified, must be > 0
//argc: the number of program arguments
//argv: an array of character pointers that corresponds to the program arguments
//returns true if the user successfully specified the test duration in the program arguments
bool GetDuration(float &fDuration, int argc, char * argv[]) {
	bool bFoundDurationFlag=false;
	int i=0;
	for (i=0;i<(argc-1);i++) {
		if (strlen(argv[i])<2) continue;
		if (strncmp(argv[i],"-d",2)==0) {
			bFoundDurationFlag=true;
			break;
		}
		else if (strncmp(argv[i],"-D",2)==0) {
			bFoundDurationFlag=true;
			break;
		}
	}
	if (bFoundDurationFlag) {
		if (sscanf(argv[i+1],"%f",&fDuration)<1) {
			printf("Error, could not parse duration.\n");
			return false;//invalid formatting of duration
		}
		if (fDuration<=0) {
			printf("Invalid duration, must be greater than zero.\n");
			return false;
		}
		return true;
	}
	return false;	
}

//GetSpeed: Gets the speed specified by the user in the program arguments
//fSpeed: the speed that the user specified, must be between MIN_AIRPROP_SPEED and MAX_AIRPROP_SPEED
//argc: the number of program arguments
//argv: an array of character pointers that corresponds to the program arguments
//returns true if the user successfully specified the propeller speed in the program arguments
bool GetSpeed(float &fSpeed, int argc, char * argv[]) {
	bool bFoundSpeedFlag=false;
	float fMinSpeed = MIN_AIRPROP_SPEED;
	float fMaxSpeed = MAX_AIRPROP_SPEED;
	int i=0;
	for (i=0;i<(argc-1);i++) {
		if (strlen(argv[i])<2) continue;
		if (strncmp(argv[i],"-s",2)==0) {
			bFoundSpeedFlag=true;
			break;
		}
		else if (strncmp(argv[i],"-S",2)==0) {
			bFoundSpeedFlag=true;
			break;
		}
	}
	if (bFoundSpeedFlag) {
		if (sscanf(argv[i+1],"%f",&fSpeed)<1) {
			printf("Error, could not parse speed.\n");
			return false;//invalid formatting of speed
		}
		if (fSpeed<MIN_AIRPROP_SPEED) {
			printf("Invalid speed, must be >= %.1f.\n",fMinSpeed);
			return false;
		}
		else if (fSpeed>MAX_AIRPROP_SPEED) {
			printf("Invalid speed, must be < %.1f.\n",fMaxSpeed);
			return false;
		}
		return true;
	}
	return false;	
}

//isShowVoltageFlagPresent: return true if the flag for showing voltage was specified in the program arguments
//argc: the number of program arguments
//argv: an array of character pointers that corresponds to the program arguments
//returns true if the flag for showing voltage (-v) is present, otherwise returns false
bool isShowVoltageFlagPresent(int argc, char * argv[]) {
	for (int i=0;i<argc;i++) {
		if (strlen(argv[i])<2) continue;
		if (strncmp(argv[i],"-v",2)==0) {
			return true;
		}
		else if (strncmp(argv[i],"-V",2)==0) {
			return true;
		}
	}
	return false;
}

void ShowUsage(float fMinSpeed, float fMaxSpeed) {//explain how to use this program
	printf("AirPropTest allows you to test the air propeller.\n");
	printf("Usage: AirPropTest [-c] [-s speed] [-d duration] [-V]\n");
	printf("-c performs a two-step calibration process for the electronic speed controller (ESC).\n");
	printf("-s sets the speed to use for the test. The speed can vary from %.1f to %.1f. The units of speed are arbitrary.\n",fMinSpeed,fMaxSpeed); 
	printf("-d flag (optional) sets the duration of the test in seconds.\n");
	printf("-V flag (optional) outputs voltage information while the propeller is running.\n");
}

//DoCalibration: do calibration on the electronic speed controller (ESC)
bool DoCalibration() {
	printf("Make sure that +12V power is switched OFF. Press enter when ready.\n");
	getchar();
	//perform one-time setup
	AirProp::OneTimeSetup();
	AirProp prop;
	//apply maximum speed signal to propeller (actually 2 x max speed, since we don't want to really use 40 A of current, which is the maximum drive capability)
	prop.DoMaxSpeedCal();
	
	delay(1000);//pause for a second
	printf("Switch on the +12V power, and then wait for a startup sound, followed by a dual beep. After this, press enter.\n");
	getchar();
	//apply stopped speed signal to propeller
	prop.Stop();	
	printf("Wait for three beeps, followed by a long, single beep to indicate that the calibration is complete and saved. Then press enter.\n");
	getchar();
	printf("Calibration completed!\n");
	return true;
}

int main(int argc, char * argv[])
{
  ShipLog shipLog;
  pthread_mutex_t i2cMutex = PTHREAD_MUTEX_INITIALIZER;;//mutex for controlling access to i2c bus
  float fMinPropellerSpeed = MIN_AIRPROP_SPEED;
  float fMaxPropellerSpeed = MAX_AIRPROP_SPEED;	
  if (isCalFlagPresent(argc, argv)) {
	  if (!DoCalibration()) {
		  printf("Error performing calibration.\n");
		  return -2;
	  }
	  printf("Calibration completed successfully.\n");
	  return 0;
  }
  if (argc<3) {
	  ShowUsage(fMinPropellerSpeed,fMaxPropellerSpeed);
	  return -1;
  }
  float fSpeed = 0.0;
  if (!GetSpeed(fSpeed, argc, argv)) {
	 printf("Error, a valid speed was not specified with the -s flag.\n");
	 ShowUsage(fMinPropellerSpeed, fMaxPropellerSpeed);
	 return -3;
  }
  bool bVoltageOutput = isShowVoltageFlagPresent(argc, argv);	
  float fTestDuration = TEST_DURATION;
  if (isDurationFlagPresent(argc, argv)) {
	  if (!GetDuration(fTestDuration,argc,argv)) {
		  printf("Error, a valid test duration time (in seconds) was not specified with the -d flag.\n");
		  return -4;
	  }
  }
  AToD *pAToD = nullptr;
  double dBatteryVoltage =0.0;
  if (bVoltageOutput) {	
	char *i2c_filename = (char*)"/dev/i2c-1";
	pAToD = new AToD(i2c_filename,ATOD_ADDRESS,&shipLog,&i2cMutex);
  }
  AirProp::OneTimeSetup();//do I/O setup for propeller (once per power-cycle)
  AirProp *pAirProp = new AirProp();
  pAirProp->Initialize();
     
  const int LOOP_DELAY = 100;//delay in ms to add to each loop
  printf("Setting speed to %.1f\n", fSpeed);  
  pAirProp->SetSpeed(fSpeed);
  unsigned int uiTimeoutTime = millis() + fTestDuration*1000;

  while (millis()<uiTimeoutTime) {
	  if (bVoltageOutput) {
		  if (pAToD->GetBatteryVoltage(dBatteryVoltage,nullptr)) {
			  //printf("Voltage = %.3f V\n",dBatteryVoltage);
		  }
	  }
	  delay(LOOP_DELAY);
  }
  delete pAirProp;
  pAirProp=nullptr;
  if (pAToD) {
	  delete pAToD;
	  pAToD=nullptr;
  }
  return 0 ;
}
