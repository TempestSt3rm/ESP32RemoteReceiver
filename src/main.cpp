#include <esp_now.h>
#include <WiFi.h>

// Define LED Pins
int LED0_PIN = 18; 
int LED1_PIN = 19; 

int LED0_confirm_Pin = 32; 
int LED1_confirm_Pin = 25; 

bool LED0_state = LOW; 
bool LED1_state = LOW; 

bool* LEDs_State[2] = {
  &LED0_state,
  &LED1_state
};

int LEDBuffer0[2] = {0, 0};
int LEDBuffer1[2] = {0, 0};

int* LEDsBuffer[2] = {LEDBuffer0, LEDBuffer1};
int risingEdge[2] = {1, 0};

// Struct for transmitted and received messages
typedef struct struct_transmission_message {
  int transmitted_state0;
  int transmitted_state1;
} struct_transmission_message;

// Create a struct_message to hold the received data
struct_transmission_message myData;

// Create a struct_message to hold the constant transmitted data
struct_transmission_message confirmMessage;

// Flag to indicate that new data has been received
volatile bool dataReceived = false;

// MAC address of the other ESP32 (the transmitter)
// mac address of the transmitter 30:83:98:ee:4e:38
uint8_t broadcastAddress[] = {0x30, 0x83, 0x98, 0xee, 0x4e, 0x38};

// Callback function to handle received data
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // Copy the received data into the myData structure
  memcpy(&myData, incomingData, sizeof(struct_transmission_message));
  // Set flag to true to indicate data was received (this will be handled in loop)
  dataReceived = true;
}

// Callback function to handle transmission status
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("Starting Receiver with Transmitter");

  // Set LED pins as outputs
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED0_confirm_Pin, INPUT);
  pinMode(LED1_confirm_Pin, INPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the callback function to handle received ESP-NOW data
  esp_now_register_recv_cb(OnDataRecv);

  // Register the callback function to handle transmission status
  esp_now_register_send_cb(OnDataSent);

  // Add the peer (sender device)
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo)); 
  memcpy(peerInfo.peer_addr, broadcastAddress, sizeof(broadcastAddress));
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(broadcastAddress)) {
    esp_now_del_peer(broadcastAddress); // Ensure no duplicate peer exists
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }
  }
}

void loop() {
  // Check if new data has been received
  if (dataReceived) {
    // Reset the flag
    dataReceived = false;

    // Print received data
    Serial.print("Data received button state:");
    Serial.print(myData.transmitted_state0);
    Serial.print(myData.transmitted_state1);

    // Update LED states
    digitalWrite(LED0_PIN, myData.transmitted_state0);
    digitalWrite(LED1_PIN, myData.transmitted_state1);
  }
  confirmMessage.transmitted_state0 = digitalRead(LED0_confirm_Pin);
  confirmMessage.transmitted_state1 = digitalRead(LED1_confirm_Pin);

  Serial.println("confirmed states ");
  Serial.print(confirmMessage.transmitted_state0);
  Serial.println(confirmMessage.transmitted_state1);

  // Send a constant message back to the transmitter
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &confirmMessage, sizeof(confirmMessage));
  if (result == ESP_OK) {
    Serial.println("Confirm message sent successfully");
  } else {
    Serial.println("Error sending constant message");
  }

  delay(200); 
}
