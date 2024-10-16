#include <esp_now.h>
#include <WiFi.h>

int LED_PIN = 2;  // Assuming GPIO 2 is connected to an LED
bool ledState = LOW;  // Variable to track the LED state

// Structure to receive data
typedef struct struct_message {
  char message[32];
  int button_state;
} struct_message;

// Create a struct_message to hold the received data
struct_message myData;

// Flag to indicate that new data has been received
volatile bool dataReceived = false;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // Copy the received data into the myData structure
  memcpy(&myData, incomingData, sizeof(struct_message));

  // Set flag to true to indicate data was received (this will be handled in loop)
  dataReceived = true;
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("Starting Receiver");

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the callback function to handle received ESP-NOW data
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {
  // Check if new data has been received
  if (dataReceived) {
    // Reset the flag
    dataReceived = false;

    // Print the received data
    Serial.print("Received message: ");
    Serial.println(myData.message);
    Serial.print("Button state: ");
    Serial.println(myData.button_state);

    // Set the LED state based on the received button state
    if (myData.button_state == 1) {
      ledState = HIGH;  // Button pressed, turn LED on
    } else {
      ledState = LOW;  // Button released, turn LED off
    }

    // Apply the LED state (this happens outside the interrupt context)
    digitalWrite(LED_PIN, ledState);
  }
}
