# ESP RainMaker Switch

This example demonstrates how to build a switch device to be used with ESP RainMaker. 
I have started with this example code to control the relay, which is controlling the motor. Here I am not using an DC motor controller or any other things to control the motor. Here I have modified the existing feeding system which can be controlled by a push button or using a Rainmaker application. 
Additinaly i have added a 5V Realy.

## Working

-- The motor will be connected to the relay and the relay will be controlled by the Rainmaker Application or the Physical (Push) button attached to it.
When ever the limit switch got triggered, the relay will be turned off. 
-- When ever the ESP32 powered on, the relay will be turned on automaticalled after 8 mints. By using Rainmaker application we can shedule it or can be controlled by a button (Feeder) preset in the Application.

## What to expect in this example?

- After compiling and flashing the example, add your device using the [ESP RainMaker phone apps](https://rainmaker.espressif.com/docs/quick-links.html#phone-apps) by scanning the QR code.

## Components

- ESP32, Limit Switch, 5V Relay, Motor

### Output

```
[    63][I][RMaker.cpp:13] event_handler(): RainMaker Initialized.
[    69][I][WiFiProv.cpp:158] beginProvision(): Already Provisioned
[    69][I][WiFiProv.cpp:162] beginProvision(): Attempting connect to AP: Viking007_2GEXT

Toggle State to false.
[  8182][I][RMakerDevice.cpp:162] updateAndReportParam(): Device : Switch, Param Name : Power, Val : false
Toggle State to true.
[  9835][I][RMakerDevice.cpp:162] updateAndReportParam(): Device : Switch, Param Name : Power, Val : true
Received value = false for Switch - Power
Received value = true for Switch - Power
Toggle State to false.
[ 29937][I][RMakerDevice.cpp:162] updateAndReportParam(): Device : Switch, Param Name : Power, Val : false
```

### Resetting the device
- Press and Hold the Boot button for more than 3 seconds and then release to reset Wi-Fi configuration.
- Press and Hold the Boot button for more than 10 seconds and then release to reset to factory defaults.
