// Project: Automated Smart Light
// Author: DominicM
// License: refer to LICENSE.txt @ https://github.com/DominicMe/AutoSmartLight

#include <EEPROM.h>

// LED boot modes:
// 0 - Off - Remains off by default.
// 1 - Default - Is activated based on user defined default values.
// 2 - Last - Returns to the last state before the last power down.

// LED control modes:
// 0 - Manual - Activates by user request only.
// 1 - Motion - Activates when motion has been detected within user defined time ignoring light levels.
// 2 - Light - Activates to maintain a user defined level of brightness regardress of motion.
// 3 - Auto - Motion and Light modes combined.

// LED display modes:
// 0 - Solid - Remains at constant user defined color.
// 1 - Cycle - Cycles through all colors at user defined speed.
// 2 - Fade - Fades out the current color and fades in a new random color.
// 3 - Strobe - Alternated between on and off at a specified speed and color.
// 4 - External - Varies color based on external input such as XBMC.
// 5 - Mimics a tv to deter burglars.

// pin designations
const int redLedPin = 9;
const int greenLedPin = 10;
const int blueLedPin = 11;
const int temperatureSensorPin = 8;
const int luminositySensorPin = A3;
const int motionDetectorPin = A2;
const int soundAlarmPin = 2;
const int coolingFanPwmPin = 5;
const int coolingFanTachometerPin = A0;
const int resetDetectPin = 3;
const int resetTriggerPin = 4;

// current values
unsigned int currentControlMode;
unsigned int currentDisplayMode;
unsigned int currentFanSpeed;
unsigned int currentTemperatureMinTrigger;
unsigned int currentTemperatureMaxTrigger;
//boolean currentSoundAlarmState; *** currently not implemented ***
//float currentLedCaseTemperature; *** currently not implemented ***
// unsigned int currentCoolingFanSpeed; *** currently not implemented ***
unsigned int currentLuminosity;
unsigned int currentLuminosityThreshold;
unsigned int currentMotionTimeout;
float currentBrightnessLevel;
float currentRedValue;
float currentGreenValue;
float currentBlueValue;

// default values
unsigned int defaultBootMode;
unsigned int defaultControlMode;
unsigned int defaultDisplayMode;
unsigned int defaultTemperatureMinTrigger;
unsigned int defaultTemperatureMaxTrigger;
unsigned int defaultMotionTimeout;
unsigned int defaultLuminosityThreshold;
unsigned int defaultBrightnessLevel;
int defaultRedValue;
int defaultGreenValue;
int defaultBlueValue;

// last values
unsigned int lastControlMode;
unsigned int lastDisplayMode;
unsigned int lastTemperatureMinTrigger;
unsigned int lastTemperatureMaxTrigger;
unsigned int lastMotionTimeout;
unsigned int lastMotionTrigger;
unsigned int lastLuminosityThreshold;
unsigned int lastBrightnessLevel;
int lastRedValue;
int lastGreenValue;
int lastBlueValue;

// global values
boolean programming_mode = true;
//unsigned int averagePowerUsage = 0; // *** currently not implemented ***

// temporary software workaround for hardware issue.
// current led drivers produce significant capacitor whine.
// changing pwm frequency takes the audible noise beyond human hearing.
// to be removed with new led drivers.
void setPwmFrequency(int pin, int divisor){
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

void setup(){
  digitalWrite(resetTriggerPin, HIGH); // prevent unwanted reset due to default LOW setting during boot
  Serial.begin(57600);
  setPwmFrequency(9, 1); // Timer 1 (pin 9 & 10)
  
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(temperatureSensorPin, INPUT);
  pinMode(motionDetectorPin, INPUT);
  pinMode(soundAlarmPin, OUTPUT);
  pinMode(coolingFanPwmPin, OUTPUT);
  pinMode(coolingFanTachometerPin, INPUT);
  pinMode(luminositySensorPin, INPUT);
  pinMode(resetDetectPin, INPUT);
  pinMode(resetTriggerPin, OUTPUT);
  
  /*
  *** currently not implemented ***
  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, 9);
  sensors.setWaitForConversion(false);
  */
  
  //digitalWrite(coolingFanTachometerPin, HIGH); *** currently not implemented ***
  //digitalWrite(redLedPin, LOW);
  //digitalWrite(greenLedPin, LOW);
  //digitalWrite(blueLedPin, LOW);
  //digitalWrite(soundAlarmPin, LOW);
  //digitalWrite(coolingFanPwmPin, LOW);
  
  //restoreDefaults();
  getDefaults();
  setDefaults();
}

int currentBluetoothState = 1;
int lastBluetoothState = 1;

void loop(){
  if(programming_mode == true){
    currentBluetoothState = digitalRead(resetDetectPin);
    if(currentBluetoothState){
      // current bluetooth state is HIGH
      if(currentBluetoothState != lastBluetoothState){
        // bluetooth state has changed from LOW to HIGH
        digitalWrite(resetTriggerPin, LOW); // reset
      }else{
        digitalWrite(resetTriggerPin, HIGH); // do not reset
      }
    }
    lastBluetoothState = currentBluetoothState;
  }
  
  /*
  *** currently not implemented ***
  if(sensors.isConnected(tempDeviceAddress)){
    sensors.requestTemperatures();
    currentLedCaseTemperature = sensors.getTempC(tempDeviceAddress);
  }else{
    currentLedCaseTemperature = currentTemperatureMaxTrigger;
    Serial.println("ERROR: temperature device is disconnected or faulty.");
  }
  */
  
  currentLuminosity = map(analogRead(luminositySensorPin), 0, 1024, 0, 255);
  
  if(digitalRead(motionDetectorPin) == HIGH){
    //tone(soundAlarmPin, 50, 500); 
    lastMotionTrigger = millis() / 1000;
  }else{
    //noTone(soundAlarmPin);
  }
  
  setLedValues();
  
  /*
  *** currently not implemented ***
  getFanSpeed();
  setFanSpeed();
  */
  
  // DEBUG OUTPUT
  // this can significantly slow down code execution
  /*
  Serial.print("PROGRAMMING_MODE: ");
  Serial.println(programming_mode);
  
  // current values
  Serial.print("CURRENT_CONTROL_MODE: ");
  Serial.println(currentControlMode);
  Serial.print("CURRENT_DISPLAY_MODE: ");
  Serial.println(currentDisplayMode);
  //Serial.print("CURRENT_FAN_SPEED: ");
  //Serial.println(currentFanSpeed);
  Serial.print("CURRENT_TEMP_MIN_TRIGGER: ");
  Serial.println(currentTemperatureMinTrigger);
  Serial.print("CURRENT_TEMP_MAX_TRIGGER: ");
  Serial.println(currentTemperatureMaxTrigger);
  Serial.print("CURRENT_ALARM_STATE: ");
  Serial.println("not in use.");
  Serial.print("CURRENT_TEMP: ");
  Serial.println(currentLedCaseTemperature);
  Serial.print("CURRENT_LUMINOSITY: ");
  Serial.println(currentLuminosity);
  Serial.print("CURRENT_LUMINOSITY_THRESHOLD: ");
  Serial.println(currentLuminosityThreshold);
  Serial.print("CURRENT_MOTION_TIMEOUT: ");
  Serial.println(currentMotionTimeout);
  Serial.print("CURRENT_BRIGHTNESS_LEVEL: ");
  Serial.println(currentBrightnessLevel);
  Serial.print("CURRENT_RED_COLOR: ");
  Serial.println(currentRedValue);
  Serial.print("CURRENT_GREEN_COLOR: ");
  Serial.println(currentGreenValue);
  Serial.print("CURRENT_BLUE_COLOR: ");
  Serial.println(currentBlueValue);
  
  // default values
  Serial.print("DEFAULT_BOOT_MODE: ");
  Serial.println(defaultBootMode);
  Serial.print("DEFAULT_CONTROL_MODE: ");
  Serial.println(defaultControlMode);
  Serial.print("DEFAULT_DISPLAY_MODE: ");
  Serial.println(defaultDisplayMode);
  Serial.print("DEFAULT_TEMP_MIN_TRIGGER: ");
  Serial.println(defaultTemperatureMinTrigger);
  Serial.print("DEFAULT_TEMP_MAX_TRIGGER: ");
  Serial.println(defaultTemperatureMaxTrigger);
  Serial.print("DEFAULT_MOTION_TIMEOUT: ");
  Serial.println(defaultMotionTimeout);
  Serial.print("DEFAULT_LUMINOSITY_THRESHOLD: ");
  Serial.println(defaultLuminosityThreshold);
  Serial.print("DEFAULT_BRIGHTNESS_LEVEL: ");
  Serial.println(defaultBrightnessLevel);
  Serial.print("DEFAULT_RED_COLOR: ");
  Serial.println(defaultRedValue);
  Serial.print("DEFAULT_GREEN_COLOR: ");
  Serial.println(defaultGreenValue);
  Serial.print("DEFAULT_BLUE_COLOR: ");
  Serial.println(defaultBlueValue);
  
  // last values
  Serial.print("LAST_CONTROL_MODE: ");
  Serial.println(lastControlMode);
  Serial.print("LAST_DISPLAY_MODE: ");
  Serial.println(lastDisplayMode);
  Serial.print("LAST_TEMP_MIN_TRIGGER: ");
  Serial.println(lastTemperatureMinTrigger);
  Serial.print("LAST_TEMP_MIN_TRIGGER: ");
  Serial.println(lastTemperatureMaxTrigger);
  Serial.print("LAST_MOTION_TIMEOUT: ");
  Serial.println(lastMotionTimeout);
  Serial.print("LAST_MOTION_TRIGGER: ");
  Serial.println(lastMotionTrigger);
  Serial.print("LAST_LUMINOSITY_THRESHOLD: ");
  Serial.println(lastLuminosityThreshold);
  Serial.print("LAST_BRIGHTNESS_LEVEL: ");
  Serial.println(lastBrightnessLevel);
  Serial.print("LAST_RED_COLOR: ");
  Serial.println(lastRedValue);
  Serial.print("LAST_GREEN_COLOR: ");
  Serial.println(lastGreenValue);
  Serial.print("LAST_BLUE_COLOR: ");
  Serial.println(lastBlueValue);
  Serial.println();
  */
  
  String serialString;
  while(Serial.available() > 0){
    delay(3);
    if(Serial.available() > 0){
      char serialCharacter = Serial.read();
      serialString += serialCharacter;
      Serial.println("AVAIL");
    }
  }
  
  String command = "";
  String value = "";
  if(serialString.length() > 0){
    int delimeterPosition = serialString.indexOf(':');
    command = serialString.substring(0, delimeterPosition);
    value = serialString.substring(delimeterPosition+1, serialString.length());
  }
  
  if(command != "" && value != ""){
    Serial.println("Command received: " + command);
    Serial.println("Value received: " + value);
    
    if(command == "RGB"){
      currentRedValue = value.substring(0, 3).toInt();
      currentGreenValue = value.substring(3, 6).toInt();
      currentBlueValue = value.substring(6, 9).toInt();
      
      lastRedValue = currentRedValue;
      lastGreenValue = currentGreenValue;
      lastBlueValue = currentBlueValue;
      
      EEPROM.write(21, lastRedValue);
      EEPROM.write(22, lastGreenValue);
      EEPROM.write(23, lastBlueValue);
    }else if(command == "RESTORE_DEFAULTS"){
      boolean restore_defaults = constrain(value.toInt(), 0, 1);
      EEPROM.write(25, restore_defaults);
    }else if(command == "PROGRAMMING_MODE"){
      programming_mode = value.toInt();
      EEPROM.write(24, programming_mode);
    }else if(command == "DEFAULT_COLOR"){
      defaultRedValue = value.substring(0, 3).toInt();
      defaultGreenValue = value.substring(3, 6).toInt();
      defaultBlueValue = value.substring(6, 9).toInt();
      EEPROM.write(10, defaultRedValue);
      EEPROM.write(11, defaultGreenValue);
      EEPROM.write(12, defaultBlueValue);
    }else if(command == "DEFAULT_BOOT_MODE"){
      if(value.toInt() < 3){
        defaultBootMode = value.toInt();
        EEPROM.write(1, defaultBootMode);
      }
    }else if(command == "CURRENT_CONTROL_MODE"){
      currentControlMode = value.toInt();
      lastControlMode = currentControlMode;
      EEPROM.write(13, lastControlMode);
    }else if(command == "DEFAULT_CONTROL_MODE"){
      if(value.toInt() < 4){
        defaultControlMode = value.toInt();
        EEPROM.write(2, defaultControlMode);
      }
    }else if(command == "CURRENT_DISPLAY_MODE"){
      currentDisplayMode = value.toInt();
      lastDisplayMode = currentDisplayMode;
      EEPROM.write(14, lastDisplayMode);
    }else if(command == "DEFAULT_DISPLAY_MODE"){
      if(value.toInt() < 8){
        defaultDisplayMode = value.toInt();
        EEPROM.write(3, defaultDisplayMode);
      }
    }else if(command == "CURRENT_LUMINOSITY_THRESHOLD"){
      currentLuminosityThreshold = value.toInt();
      lastLuminosityThreshold = currentLuminosityThreshold;
      EEPROM.write(19, lastLuminosityThreshold);
    }else if(command == "CURRENT_BRIGHTNESS_LEVEL"){
      currentBrightnessLevel = value.toInt();
      lastBrightnessLevel = currentBrightnessLevel;
      EEPROM.write(20, lastBrightnessLevel);
    }else if(command == "DEFAULT_BRIGHTNESS_LEVEL"){
      defaultBrightnessLevel = value.toInt();
      EEPROM.write(9, defaultBrightnessLevel);
    }else if(command == "DEFAULT_LUMINOSITY_THRESHOLD"){
      defaultLuminosityThreshold = constrain(value.toInt(), 0, 255);
      EEPROM.write(8, defaultLuminosityThreshold);
    }else if(command == "CURRENT_TEMP_MIN_TRIGGER"){
      currentTemperatureMinTrigger = constrain(value.toInt(), 0, 50);
      lastTemperatureMinTrigger = currentTemperatureMinTrigger;
      EEPROM.write(15, lastTemperatureMinTrigger);
    }else if(command == "CURRENT_TEMP_MAX_TRIGGER"){
      currentTemperatureMaxTrigger = constrain(value.toInt(), 50, 80);
      lastTemperatureMaxTrigger = currentTemperatureMaxTrigger;
      EEPROM.write(16, lastTemperatureMaxTrigger);
    }else if(command == "CURRENT_MOTION_TIMEOUT"){
      currentMotionTimeout = constrain(value.toInt(), 1, 3600);
      if(currentMotionTimeout > 255){
        EEPROM.write(17, 255);
        EEPROM.write(18, round(currentMotionTimeout / 255));
      }else{
        EEPROM.write(17, currentMotionTimeout);
        EEPROM.write(18, 1);
      }
    }else if(command == "DEFAULT_MOTION_TIMEOUT"){
      defaultMotionTimeout = constrain(value.toInt(), 1, 3600);
      if(defaultMotionTimeout > 255){
        EEPROM.write(6, 255);
        EEPROM.write(7, round(defaultMotionTimeout / 255));
      }else{
        EEPROM.write(6, defaultMotionTimeout);
        EEPROM.write(7, 1);
      }
    }else if(command == "DEFAULT_TEMP_MIN_TRIGGER"){
      defaultTemperatureMinTrigger = constrain(value.toInt(), 0, 60);
      EEPROM.write(4, defaultTemperatureMinTrigger);
    }else if(command == "DEFAULT_TEMP_MAX_TRIGGER"){
      defaultTemperatureMaxTrigger = constrain(value.toInt(), 0, 90);
      EEPROM.write(5, defaultTemperatureMaxTrigger);
    }
    
    command = "";
    value = "";
  }
}

/*
*** currently not implemented ***
void setFanSpeed(){
  float averageColorOutput = (((currentRedValue + currentGreenValue + currentBlueValue) / 3) * currentBrightnessLevel) / 255;
  averagePowerUsage = (averagePowerUsage + round(averageColorOutput)) / 2;
  
  int minimumPowerTrigger = map(25, 0, 90, 4, 255);
  int maximumPowerTrigger = map(90, 0, 90, 4, 255);
  
  if(averagePowerUsage < minimumPowerTrigger){
    currentFanSpeed = 4;
  }else if(averagePowerUsage > minimumPowerTrigger && averagePowerUsage < maximumPowerTrigger){
    currentFanSpeed = map(averagePowerUsage, 0, 255, minimumPowerTrigger, maximumPowerTrigger) - minimumPowerTrigger;
  }else if(averagePowerUsage > maximumPowerTrigger){
    currentFanSpeed = 255;
  }
  
  // Override based on temperature reading
  int temperatureDerivedCoolingValue = map(constrain(currentLedCaseTemperature, currentTemperatureMinTrigger, currentTemperatureMaxTrigger), currentTemperatureMinTrigger, currentTemperatureMaxTrigger, 4, 255);
  
  if(currentFanSpeed < temperatureDerivedCoolingValue){
    //else if(currentLedCaseTemperature > currentTemperatureMinTrigger && currentLedCaseTemperature < currentTemperatureMaxTrigger){
    currentFanSpeed = temperatureDerivedCoolingValue;
    //}else if(currentLedCaseTemperature > currentTemperatureMaxTrigger){
    //currentFanSpeed = 255;
    //}
  }
  
  analogWrite(coolingFanPwmPin, currentFanSpeed);
}
*/

/*
*** currently not implemented ***
void getFanSpeed(){
  //unsigned long fanTachometerPulseDuration;
  //fanTachometerPulseDuration = pulseIn(coolingFanTachometerPin, LOW);
  //unsigned int frequency = 1000000/fanTachometerPulseDuration;
  //currentFanSpeedTachometergfdggdgd = (frequency / 2) * 60;
}
*/

// cycle mode properties
unsigned int currentCycleColor = 0;
unsigned int currentCycleHold = 0; // how many loop cycles to hold on primary colors
unsigned int cycleSpeed = 1; // how many units to increment/decrement by each cycle
unsigned int currentCycleTimer = 0;

// fade mode properties
unsigned int fadeSpeed = 1; // how many units to increment/decrement by each cycle
boolean fadeDirection = true; // fade in or fade out

void setLedValues(){
  if(currentControlMode == 1){
    // Motion only
    if( ((millis() / 1000)) < (lastMotionTrigger + currentMotionTimeout) ){
      if(round(currentBrightnessLevel) != 255){currentBrightnessLevel = constrain(currentBrightnessLevel + 5, 0, 255); };
    }else{
      if(round(currentBrightnessLevel) != 0){currentBrightnessLevel = constrain(currentBrightnessLevel - 5, 0, 255); };
    }
  }else if(currentControlMode == 2){
    // Light only
    if(currentLuminosity < (currentLuminosityThreshold + 10) && currentLuminosity > (currentLuminosityThreshold - 10)){
      // It's about right, leave it as is.
    }else if(currentLuminosity < currentLuminosityThreshold){
      // It's too dark increase light output.
      if(round(currentBrightnessLevel) != 255){currentBrightnessLevel = constrain(currentBrightnessLevel + 5, 0, 255); };
    }else if(currentLuminosity > currentLuminosityThreshold){
      // It's too bright, decrease light output.
      if(currentBrightnessLevel != 0){currentBrightnessLevel = constrain(currentBrightnessLevel - 5, 0, 255); };
    }
  }else if(currentControlMode == 3){
    // Motion & Light
    if( ((millis() / 1000)) < (lastMotionTrigger + currentMotionTimeout) ){
      if(currentLuminosity < (currentLuminosityThreshold + 10) && currentLuminosity > (currentLuminosityThreshold - 10)){
        // It's about right, leave it as is.
      }else if(currentLuminosity < currentLuminosityThreshold){
        // It's too dark increase light output.
        if(round(currentBrightnessLevel) != 255){currentBrightnessLevel = constrain(currentBrightnessLevel + 5, 0, 255); };
      }else if(currentLuminosity > currentLuminosityThreshold){
        // It's too bright, decrease light output.
        if(round(currentBrightnessLevel) != 0){currentBrightnessLevel = constrain(currentBrightnessLevel - 5, 0, 255); };
      }
    }else{
      if(round(currentBrightnessLevel) != 0){currentBrightnessLevel = constrain(currentBrightnessLevel - 5, 0, 255); };
    }
  }
  
  if(currentDisplayMode == 1){
    // CYCLE
    // TODO: cycle properties
    // cycle(random, sequential), colors({255, 0, 0}, {255, 255, 0}, ...) hold_on(seconds), speed(1-10)
    if(currentCycleColor == 0){
      currentRedValue = constrain(currentRedValue + cycleSpeed, 0, 255);
      currentGreenValue = constrain(currentGreenValue - cycleSpeed, 0, 255);
      currentBlueValue = constrain(currentBlueValue - cycleSpeed, 0, 255);
      if(currentCycleHold < currentCycleTimer){
        currentCycleColor = currentRedValue != 255 ? 0 : 1;
        currentCycleTimer = 0;
      }else{
        currentCycleTimer++;
      }
    }else if(currentCycleColor == 1){
      currentGreenValue = constrain(currentGreenValue + cycleSpeed, 0, 255);
      currentBlueValue = constrain(currentBlueValue - cycleSpeed, 0, 255);
      currentRedValue = constrain(currentRedValue - cycleSpeed, 0, 255);
      if(currentCycleHold < currentCycleTimer){
        currentCycleColor = currentGreenValue != 255 ? 1 : 2;
        currentCycleTimer = 0;
      }else{
        currentCycleTimer++;
      }
    }else if(currentCycleColor == 2){
      currentBlueValue = constrain(currentBlueValue + cycleSpeed, 0, 255);
      currentRedValue = constrain(currentRedValue - cycleSpeed, 0, 255);
      currentGreenValue = constrain(currentGreenValue - cycleSpeed, 0, 255);
      if(currentCycleHold < currentCycleTimer){
        currentCycleColor = currentBlueValue != 255 ? 2 : 0;
        currentCycleTimer = 0;
      }else{
        currentCycleTimer++;
      }
    } 
  }else if(currentDisplayMode == 2){
    // FADE
    // TODO: fade properties
    // fade(random, sequential), colors({255, 0, 0}, {255, 255, 0}, ...) hold_on(seconds), hold_off(seconds), speed(1-10)
    // red - yellow - green - cyan - blue - magenta
    // unsigned int colors[9][3] = { {255, 0, 0}, {255, 255, 0}, {0, 255, 0}, {0, 255, 255}, {0, 0, 255}, {255, 0, 255} };
    // fade out by default on fade mode activation - float fadeColor[3] = {0, 0, 0};
    // adjust brightness to maintain uniformity from color changes
    if(fadeDirection){
      if(currentBrightnessLevel == 255){ fadeDirection = false; }
      currentBrightnessLevel = constrain( (currentBrightnessLevel + fadeSpeed), 0, 255 );
    }else{
      if(currentBrightnessLevel == 0){
        fadeDirection = true;
        currentRedValue = random(255);
        currentGreenValue = random(255);
        currentBlueValue = random(255);
      }
      currentBrightnessLevel = constrain( (currentBrightnessLevel - fadeSpeed), 0, 255 );
    }
  }
  
  // map function adjusts for led chip color start up inconsistencies and reverses values to account for inverse logic drivers
  analogWrite(redLedPin, map( round((currentRedValue / float(255)) * currentBrightnessLevel), 0, 255, 255, 5));
  analogWrite(greenLedPin, map( round((currentGreenValue / float(255)) * currentBrightnessLevel), 0, 255, 255, 5));
  analogWrite(blueLedPin, map( round((currentBlueValue / float(255)) * currentBrightnessLevel), 0, 255, 255, 1));
}

/*
*** currently not implemented ***
void setAlarmState(boolean state = 0, unsigned long duration = 250, unsigned int frequency = 50){
  //currentSoundAlarmState = state;
  if(currentSoundAlarmState == true){
    tone(soundAlarmPin, frequency, duration);
  }else{
    noTone(soundAlarmPin);
  }
}
*/

void restoreDefaults(){
  int restore_defaults = EEPROM.read(25);
  if(restore_defaults > 1){
    restore_defaults = 0;
  }
  if(restore_defaults){
    EEPROM.write(1, 0); // defaultBootMode
    EEPROM.write(2, 3); // defaultControlMode
    EEPROM.write(3, 0); // defaultDisplayMode
    EEPROM.write(4, 40); // defaultTemperatureMinTrigger
    EEPROM.write(5, 80); // defaultTemperatureMaxTrigger
    EEPROM.write(6, 255); // defaultMotionTimeout
    EEPROM.write(7, 1); // defaultMotionTimeout multiplier
    EEPROM.write(8, 100); // defaultLuminosityThreshold
    EEPROM.write(9, 0); // defaultBrightnessLevel
    EEPROM.write(10, 255); // defaultRedValue
    EEPROM.write(11, 255); // defaultGreenValue
    EEPROM.write(12, 255); // defaultBlueValue
    EEPROM.write(13, 3); // lastControlMode
    EEPROM.write(14, 0); // lastDisplayMode
    EEPROM.write(15, 40); // lastTemperatureMinTrigger
    EEPROM.write(16, 60); // lastTemperatureMaxTrigger
    EEPROM.write(17, 255); // lastMotionTimeout
    EEPROM.write(18, 1); // lastMotionTimeout multiplier
    EEPROM.write(19, 100); // lastLuminosityThreshold
    EEPROM.write(20, 0); // lastBrightnessLevel
    EEPROM.write(21, 255); // lastRedValue
    EEPROM.write(22, 255); // lastGreenValue
    EEPROM.write(23, 255); // lastBlueValue
    EEPROM.write(24, 0); // programming_mode
    EEPROM.write(25, 0); // restoreDefaults
  }
}

void getDefaults(){
  // read default values from EEPROM
  defaultBootMode = readByte(1, 0, 2);
  defaultControlMode = readByte(2, 0, 3);
  defaultDisplayMode = readByte(3, 0, 4);
  defaultTemperatureMinTrigger = readByte(4, 0, 50);
  defaultTemperatureMaxTrigger = readByte(5, defaultTemperatureMinTrigger, 80);
  defaultMotionTimeout = constrain(int(readByte(6, 1, 255) * readByte(7, 1, 255)), 1, 3600);
  defaultLuminosityThreshold = readByte(8, 0, 255);
  defaultBrightnessLevel = readByte(9, 0, 255);
  defaultRedValue = readByte(10, 0, 255);
  defaultGreenValue = readByte(11, 0, 255);
  defaultBlueValue = readByte(12, 0, 255);
  lastControlMode = readByte(13, 0, 3);
  lastDisplayMode = readByte(14, 0, 4);
  lastTemperatureMinTrigger = readByte(15, 0, 50);
  lastTemperatureMaxTrigger = readByte(16, lastTemperatureMinTrigger, 80);
  lastMotionTimeout = constrain(int(readByte(17, 1, 255) * readByte(18, 1, 255)), 1, 3600);
  lastLuminosityThreshold = readByte(19, 0, 255);
  lastBrightnessLevel = readByte(20, 0, 255);
  lastRedValue = readByte(21, 0, 255);
  lastGreenValue = readByte(22, 0, 255);
  lastBlueValue = readByte(23, 0, 255);
  programming_mode = readByte(24, 0, 1);
}

int readByte(int byteLocation, int minValue, int maxValue){
  int byteValue = constrain(EEPROM.read(byteLocation), minValue, maxValue);
  return byteValue;
}

void setDefaults(){
  lastMotionTrigger = 0;
  if(defaultBootMode == 0){
    // manual
    currentControlMode = 0;
    currentDisplayMode = 0;
    currentTemperatureMinTrigger = 40; // baseTemperatureMin
    currentTemperatureMaxTrigger = 60; // baseTemperatureMax
    currentMotionTimeout = 30; // baseMotionTimeout
    currentLuminosityThreshold = 0;
    currentBrightnessLevel = 0;
    currentRedValue = 255;
    currentGreenValue = 255;
    currentBlueValue = 255;
  }else if(defaultBootMode == 1){
    // default
    currentControlMode = defaultControlMode;
    currentDisplayMode = defaultDisplayMode;
    currentTemperatureMinTrigger = defaultTemperatureMinTrigger;
    currentTemperatureMaxTrigger = defaultTemperatureMaxTrigger;
    currentMotionTimeout = defaultMotionTimeout;
    currentLuminosityThreshold = defaultLuminosityThreshold;
    currentBrightnessLevel = defaultBrightnessLevel;
    currentRedValue = defaultRedValue;
    currentGreenValue = defaultGreenValue;
    currentBlueValue = defaultBlueValue;
  }else if(defaultBootMode == 2){
    // last
    currentControlMode = lastControlMode;
    currentDisplayMode = lastDisplayMode;
    currentTemperatureMinTrigger = lastTemperatureMinTrigger;
    currentTemperatureMaxTrigger = lastTemperatureMaxTrigger;
    currentMotionTimeout = lastMotionTimeout;
    currentLuminosityThreshold = lastLuminosityThreshold;
    currentBrightnessLevel = lastBrightnessLevel;
    currentRedValue = lastRedValue;
    currentGreenValue = lastGreenValue;
    currentBlueValue = lastBlueValue;
  }
}
