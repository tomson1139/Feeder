#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include "AppInsights.h"

#define DEFAULT_POWER_MODE false
const char *service_name = "PROV_1234";  // Provisioning service name
const char *pop = "abcd1234";            // Proof of possession

// GPIO definitions
static int gpio_button = 15;  // Push Button
static int gpio_limit_switch = 12;
static int gpio_switch = 13;  // Relay pin

// Variables for relay control and timing
bool relayState = DEFAULT_POWER_MODE;
bool buttonActivated = false;
bool appControlled = false;
unsigned long lastInputTime = 0;
unsigned long relayOnTime = 0;
const unsigned long delayDuration = 5000;  // 5 seconds delay between presses
const unsigned long limitSwitchActivationDelay = 5000;  // 5 seconds delay before deactivating by limit switch

unsigned long lastWiFiCheckTime = 0;
const unsigned long wifiCheckInterval = 10000;  // Check Wi-Fi every 10 seconds

// Debouncing variables
unsigned long lastDebounceTimeLimit = 0;
const unsigned long debounceDelay = 50;  // 50ms debounce delay

bool lastButtonState = HIGH;  // Assume the button is not pressed initially
bool lastLimitSwitchState = HIGH;  // Assume the limit switch is not triggered initially

// Variables for button delays
unsigned long lastButtonPressTime = 0;  // Track the last button press time
bool buttonCooldown = false;  // Flag to indicate if the button is in cooldown
const unsigned long buttonDelay = 12000;  // 12 seconds delay between button presses

// Variables for initial 2-minute delay after powering on
unsigned long powerOnTime = 0;
bool relayActivatedAfterPowerOn = false;
const unsigned long powerOnDelay = 8 * 60 * 1000;  // 5 minutes in milliseconds

// The framework provides some standard device types like switch, lightbulb, etc.
static Switch *my_switch = NULL;

// Provisioning event handler
void sysProvEvent(WiFiEvent_t event) {
    switch (event) {
        case IP_EVENT_STA_GOT_IP:
            Serial.println("Wi-Fi connected, IP address assigned.");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            Serial.println("Wi-Fi got disconnected, attempting to reconnect...");
            WiFi.reconnect();
            break;
        default:
            break;
    }
}

// Callback function for RainMaker app writes
void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx) {
    const char *param_name = param->getParamName();
    Serial.printf("Callback triggered for param %s with value %s\n", param_name, val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
        relayState = val.val.b;
        appControlled = true;  // The relay is now controlled by the app
        buttonActivated = false;  // Reset button-activated flag
        relayOnTime = millis();  // Track the time the relay was activated
        Serial.printf("Setting relayState to %s by app\n", relayState ? "ON" : "OFF");

        // Set relay state
        digitalWrite(gpio_switch, relayState ? LOW : HIGH);

        // Report the state back to the RainMaker app
        param_val_t reported_val;
        reported_val.val.b = relayState;
        param->updateAndReport(reported_val);
        Serial.printf("Relay state updated: %s\n", relayState ? "ON" : "OFF");

        // Set cooldown after the virtual button press
        lastButtonPressTime = millis();
        buttonCooldown = true;
    }
}

// Non-blocking Wi-Fi connection management
void manageWiFiConnection() {
    unsigned long current_time = millis();
    // Check Wi-Fi status every `wifiCheckInterval` milliseconds
    if (current_time - lastWiFiCheckTime >= wifiCheckInterval) {
        lastWiFiCheckTime = current_time;
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi got disconnected. Attempting to reconnect...");
            WiFi.disconnect(true); // Clear previous Wi-Fi credentials
            delay(1000); // Short delay to ensure disconnection
            WiFi.begin(); // Reconnect to Wi-Fi
            unsigned long wifiRetryStartTime = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - wifiRetryStartTime) < 12000) { // Retry for 12 seconds
                delay(500); // Wait a bit before retrying
                Serial.print(".");
            }
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("Reconnected to Wi-Fi.");
                RMaker.start(); // Restart RainMaker services after reconnection
                delay(2000); // Allow time for services to stabilize
            } else {
                Serial.println("Failed to reconnect to Wi-Fi.");
            }
        }
    }
}

// Start provisioning
void startProvisioning() {
    Serial.println("Starting provisioning...");
    // Provisioning choice (BLE or SoftAP)
#if CONFIG_IDF_TARGET_ESP32S2
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
    Serial.println("Using SoftAP for provisioning.");
#else
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
    Serial.println("Using BLE for provisioning.");
#endif

    Serial.printf("Provisioning started. SSID: %s\n", service_name);
}

void setup() {
    Serial.begin(115200);
    pinMode(gpio_button, INPUT_PULLUP); // Assuming the push button is connected to ground
    pinMode(gpio_limit_switch, INPUT_PULLUP); // Assuming the limit switch is connected to ground
    pinMode(gpio_switch, OUTPUT);
    digitalWrite(gpio_switch, HIGH); // Ensure the relay is off at the start

    powerOnTime = millis();  // Capture the time when the device powered on

    // Initial Wi-Fi connection attempt
    manageWiFiConnection();

    Node my_node = RMaker.initNode("ESP RainMaker Node");

    // Initialize switch device
    my_switch = new Switch("Feeder", &gpio_switch); // Updated name to 'Feeder'
    if (!my_switch) {
        Serial.println("Failed to create switch device");
        return;
    }
    my_switch->addCb(write_callback);
    my_node.addDevice(*my_switch);

    // Enable OTA, Timezone, Scheduling, Scenes, and System Services
    RMaker.enableOTA(OTA_USING_TOPICS);
    RMaker.enableTZService();
    RMaker.enableSchedule();
    RMaker.enableScenes();
    initAppInsights();
    RMaker.enableSystemService(SYSTEM_SERV_FLAGS_ALL, 2, 2, 2);

    // Start RainMaker
    RMaker.start();

    // Set the default state as OFF in the RainMaker app
    relayState = DEFAULT_POWER_MODE;
    my_switch->updateAndReportParam("Power", relayState);

    // Wi-Fi event handler
    WiFi.onEvent(sysProvEvent);

    // Start provisioning
    startProvisioning();
}

void loop() {
    manageWiFiConnection();  // Check and manage Wi-Fi connection periodically

    unsigned long current_time = millis();

    // Debugging statements to check timing
    Serial.printf("Current Time: %lu, Power On Time: %lu, Relay Activated: %d\n",
                  current_time, powerOnTime, relayActivatedAfterPowerOn);

    // Check if the initial 2-minute delay has passed
    if (!relayActivatedAfterPowerOn && (current_time - powerOnTime >= powerOnDelay)) {
        relayState = true;  // Activate the relay
        relayActivatedAfterPowerOn = true;
        Serial.println("Relay activated after 2 minutes.");
        if (my_switch) {
            my_switch->updateAndReportParam("Power", relayState);
        }
    }

    // Check if the button cooldown has expired
    if (buttonCooldown && (current_time - lastButtonPressTime >= buttonDelay)) {
        buttonCooldown = false;  // Reset the cooldown flag after 12 seconds
        Serial.println("Button cooldown expired, both buttons can be used again.");
    }

    // Physical Button Logic
    if (!buttonCooldown && current_time - lastInputTime > delayDuration) {
        bool currentButtonState = digitalRead(gpio_button);

        // Handle physical button press
        if (currentButtonState == LOW && lastButtonState == HIGH) {
            relayState = true;  // Activate the relay
            buttonActivated = true;  // Set flag that button activated the relay
            relayOnTime = millis();  // Set the relay on time
            lastInputTime = millis();  // Reset the timer
            appControlled = false;  // Indicate that the relay is not controlled by the app
            Serial.println("Relay activated by physical button");

            // Update app with new relay state
            if (my_switch) {
                my_switch->updateAndReportParam("Power", relayState);
            }

            // Set cooldown for both buttons
            lastButtonPressTime = millis();
            buttonCooldown = true;
        }
        lastButtonState = currentButtonState;
    }

    // Limit Switch Logic
    bool currentLimitSwitchState = digitalRead(gpio_limit_switch);
    if (currentLimitSwitchState == LOW && lastLimitSwitchState == HIGH) {
        unsigned long limitSwitchActivatedTime = millis();
        if (millis() - relayOnTime >= limitSwitchActivationDelay) {
            relayState = false;  // Deactivate the relay
            Serial.println("Relay deactivated by limit switch");
            if (my_switch) {
                my_switch->updateAndReportParam("Power", relayState);
            }
            lastButtonPressTime = millis();
            buttonCooldown = true;
        }
    }
    lastLimitSwitchState = currentLimitSwitchState;

    // Update relay state
    digitalWrite(gpio_switch, relayState ? LOW : HIGH);
    delay(100);  // Short delay to avoid rapid polling

    // Yield to allow other tasks to run
    yield();
}
