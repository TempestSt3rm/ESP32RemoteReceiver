#include <esp_now.h>
#include <WiFi.h>

//5 Outputs represented as 5 leds B,G,R,B,G
int LED0_PIN = 21; 
int LED1_PIN = 22; 
int LED2_PIN = 19; 
int LED3_PIN = 23; 
int LED4_PIN = 18; 

bool LED0_state = LOW; 
bool LED1_state = LOW; 
bool LED2_state = LOW; 
bool LED3_state = LOW; 
bool LED4_state = LOW;

bool* LEDs_State[5] = {
  &LED0_state,
  &LED1_state,
  &LED2_state,
  &LED3_state,
  &LED4_state
};

int LEDBuffer0[2] = {0,0};
int LEDBuffer1[2] = {0,0};
int LEDBuffer2[2] = {0,0};
int LEDBuffer3[2] = {0,0};
int LEDBuffer4[2] = {0,0};

int* LEDsBuffer[5] = {LEDBuffer0,LEDBuffer1,LEDBuffer2,LEDBuffer3,LEDBuffer4};
int risingEdge[2] = {1,0};

// Structure to receive data
typedef struct struct_message {
  int button_state0;
  int button_state1;
  int button_state2;
  int button_state3;
  int button_state4;
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


bool compareArrays(int a[], int b[], int size) {
  for (int i = 0; i < size; i++) {
    if (a[i] != b[i]) {
      return false;  // Return false if any elements differ
    }
  }
  return true;  // Return true if all elements are equal
}

void shiftArrays(int* arrays[], int numArrays, int arraySize, int inputs[]) {
  // Iterate through each array
  for (int i = 0; i < numArrays; i++) {
    // Shift the elements in the current array to the right, leaving the 0th index open for the new one
    for (int j = arraySize - 1; j > 0; j--) {
      arrays[i][j] = arrays[i][j - 1];
    }
    // Set the new input value at index 0
    arrays[i][0] = inputs[i];
  }
}



void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("Starting Receiver");

  // Set LED pin as output
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);

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
 
    int inputArray[5] = {
      myData.button_state0,
      myData.button_state1,
      myData.button_state2,
      myData.button_state3,
      myData.button_state4,
    };

    shiftArrays(LEDsBuffer,5,2,inputArray);

    // Set the LED state based on the received button state
    for (int i = 0; i<5; i++){
      if(compareArrays(LEDsBuffer[i],risingEdge,2)) {
        *LEDs_State[i] = ! *LEDs_State[i]; 
      }
    };

    digitalWrite(LED0_PIN,LED0_state);
    digitalWrite(LED1_PIN,LED1_state);
    digitalWrite(LED2_PIN,LED2_state);
    digitalWrite(LED3_PIN,LED3_state);
    digitalWrite(LED4_PIN,LED4_state);
  }
}
