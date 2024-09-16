# ESP RainMaker Feeder

I have modified the default Fish Feeder that I have brought from online. Removed the main board and fixed esp32. Now I can controlled the feeder by Sheduled time or via Rainmaker application. 

## Working

- The motor will be controlled by the relay and the relay will be controlled by the Rainmaker Application or the Physical (Push) button attached to it.
When ever the limit switch got triggered, the relay will be turned off. 
- When ever the ESP32 powered on, the relay will be turned on automaticalled after 8 mints. By using Rainmaker application we can shedule it or can be controlled by a button (Feeder) preset in the Application.
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
