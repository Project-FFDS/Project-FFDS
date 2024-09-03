#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <Adafruit_SCD30.h>
#include <Adafruit_AMG88xx.h>
#include "Arducam_Mega.h"

//To avoid compilation error
const size_t IMAGE_FRAGMENT_DATA_SIZE = 255 - (sizeof(bool) * 2) - sizeof(size_t);

// Structs
struct AuthorizationPacket {
  bool authorized;
  int deviceID;
};

struct SimpleDataPacket {
  int deviceID;
  float temperature;
  float co2Concentration;
  float latitude;
  float longitude;
};

struct ImageFragmentDataPacket {
  bool start;
  bool end;
  size_t dataSize;
  uint8_t data[IMAGE_FRAGMENT_DATA_SIZE];
};

// Pins
const int ARDUCAM_CS = 9;    // Arducam chip select pin
const int GPS_RX_PIN = 5;    // GPS RX pin
const int GPS_TX_PIN = 6;    // GPS TX pin
const int SX1281_CS = 10;    // SX1281 chip select pin
const int SX1281_RESET = 2;  // SX1281 reset pin
const int SX1281_DIO1 = 3;   // SX1281 DIO1 pin
const int SX1281_DIO2 = 8;   // SX1281 DIO2 pin

// Sensors and Components - Instances
Adafruit_AMG88xx amg;                                                          // Thermal image sensor
Adafruit_SCD30 scd30;                                                          // CO2, temperature and humidity sensor
Arducam_Mega CAM(ARDUCAM_CS);                                                  // Arducam instance
TinyGPSPlus GEO;                                                               // TinyGPS object
SX1280 radio = new Module(SX1281_CS, SX1281_RESET, SX1281_DIO1, SX1281_DIO2);  // SX1281 radio controller

// Software Serial
SoftwareSerial GEO_SERIAL(GPS_RX_PIN, GPS_TX_PIN);  // SoftwareSerial for GPS

// Network
const int IMAGE_FRAGMENT_DATA_PACKET_SIZE = 255;
const int BUFFER_SIZE = IMAGE_FRAGMENT_DATA_SIZE;                 // Image buffer size
const int DEVICE_ID = 0;                                          // Device ID
const size_t AUTH_PACKET_SIZE = sizeof(AuthorizationPacket);      // Authorization Packet buffer size
const size_t SIMPLE_DATA_PACKET_SIZE = sizeof(SimpleDataPacket);  // Simple Data Packet buffer size

// Buffers
uint8_t authPacketBuffer[AUTH_PACKET_SIZE];                              // Authorization packet buffer
uint8_t simpleDataPacketBuffer[SIMPLE_DATA_PACKET_SIZE];                 // Data packet buffer
uint8_t imageFragmentDataPacketBuffer[IMAGE_FRAGMENT_DATA_PACKET_SIZE];  // Image fragment packet buffer

// Simple data variables
float lat = 0.0, lng = 0.0;

void setup() {
  Serial.begin(9600);      // Initialize serial communication
  GEO_SERIAL.begin(9600);  // Initialize serial communication with GPS
  Wire.begin();            // Initialize I2C bus

  // Initialize AMG8833 sensor
  if (!amg.begin()) {
    Serial.println("Failed to initialize AMG8833");
    while (true)
      ;  // If initialization fails, halt the system
  } else {
    Serial.println("AMG8833 initialized successfully");
  }

  // Initialize SCD30 sensor
  if (!scd30.begin()) {
    Serial.println("Failed to initialize SCD30");
    while (true)
      ;  // If initialization fails, halt the system
  } else {
    Serial.println("SCD30 initialized successfully");
  }

  // Initialize Arducam
  CAM.begin();

  // Initialize SX1281 radio
  int state = radio.beginGFSK();

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Radio initialized successfully!");
  } else {
    Serial.print("Initialization failed, code ");
    Serial.println(state);
    while (true)
      ;  // If initialization fails, halt the system
  }

  // Configure radio
  state = radio.setFrequency(2400.0);
  state = radio.setBitRate(500);
  state = radio.setFrequencyDeviation(100.0);
  state = radio.setOutputPower(15);
  state = radio.setDataShaping(RADIOLIB_SHAPING_1_0);
  uint8_t syncWord[] = { 0x01, 0x23, 0x45, 0x67, 0x89 };
  state = radio.setSyncWord(syncWord, 5);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("Unable to set configuration, code "));
    Serial.println(state);
    while (true)
      ;  // If configuration fails, halt the system
  }
}

void loop() {

  AuthorizationPacket* authorizationPacket = reinterpret_cast<AuthorizationPacket*>(authPacketBuffer);
  SimpleDataPacket* simpleDataPacket = reinterpret_cast<SimpleDataPacket*>(simpleDataPacketBuffer);

  if (radio.available()) {

    // Wait to receive the authorization
    int state = radio.receive(authPacketBuffer, AUTH_PACKET_SIZE);

    if (state == RADIOLIB_ERR_NONE) {

      // Verify is the authorization is for this deviceID
      if (authorizationPacket->deviceID == DEVICE_ID && authorizationPacket->authorized) {

        Serial.println("Authorized. Reading data...");

        readGeolocation();
        float temperature = readTemperatureSCD30();
        float co2Concentration = readCO2ConcentrationSCD30();
        float maxTemperature = readMaxTemperatureAMG8833();

        //Creating packet
        simpleDataPacket->deviceID = DEVICE_ID;
        simpleDataPacket->temperature = maxTemperature;
        simpleDataPacket->co2Concentration = co2Concentration;
        simpleDataPacket->latitude = lat;
        simpleDataPacket->longitude = lng;

        Serial.print("Simple data packet generated: ");
        printSimpleDataPacket(simpleDataPacket);

        if (sendSimpleDataPacketBuffer()) {

          sendCameraData();
        }

      } else {
        Serial.println("Authorization failed or deviceID mismatch.");
      }
    } else {
      Serial.println("Error receiving data.");
    }
  }
  delay(5000);  // Wait 5 seconds before the next check
}

void readGeolocation() {
  while (GEO_SERIAL.available() > 0) {
    if (GEO.encode(GEO_SERIAL.read())) {
      lat = GEO.location.lat();
      lng = GEO.location.lng();
      Serial.print("GPS Location read: Latitude = ");
      Serial.print(lat);
      Serial.print(", Longitude = ");
      Serial.println(lng);
    }
  }
}

float readTemperatureSCD30() {
  if (scd30.dataReady()) {
    if (!scd30.read()) {
      Serial.println("Error reading temperature data from SCD30.");
      return NAN;
    }
    return scd30.temperature;
  }
  return 0;
}

float readCO2ConcentrationSCD30() {
  if (scd30.dataReady()) {
    if (!scd30.read()) {
      Serial.println("Error reading CO2 data from SCD30.");
      return NAN;
    }
    return scd30.CO2;
  }
  return 0;
}

float readMaxTemperatureAMG8833() {
  float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
  amg.readPixels(pixels);
  float maxTemp = pixels[0];
  for (int i = 1; i < AMG88xx_PIXEL_ARRAY_SIZE; i++) {
    if (pixels[i] > maxTemp) {
      maxTemp = pixels[i];
    }
  }
  Serial.print("Maximum temperature detected by AMG8833: ");
  Serial.println(maxTemp);
  return maxTemp;
}

void printSimpleDataPacket(const SimpleDataPacket* simpleDataPacket) {
  Serial.println("Simple data: ");

  Serial.print("\tDevice ID: ");
  Serial.println(simpleDataPacket->deviceID);
  Serial.print("\tTemperature: ");
  Serial.println(simpleDataPacket->temperature);
  Serial.print("\tCO2 Concentration: ");
  Serial.println(simpleDataPacket->co2Concentration);
  Serial.print("\tLatitude: ");
  Serial.println(simpleDataPacket->latitude);
  Serial.print("\tLongitude: ");
  Serial.println(simpleDataPacket->longitude);

  Serial.println();
}

void printImageFragmentDataPacket(const ImageFragmentDataPacket* imageFragmentDataPacket) {
  Serial.println("Image fragment data: ");

  Serial.print("\tStart: ");
  Serial.println(imageFragmentDataPacket->start);
  Serial.print("\tEnd: ");
  Serial.println(imageFragmentDataPacket->end);
  Serial.print("\tData size: ");
  Serial.println(imageFragmentDataPacket->dataSize);

  Serial.write(imageFragmentDataPacket->data, imageFragmentDataPacket->dataSize);

  Serial.println();
}

bool sendSimpleDataPacketBuffer() {
  int state = radio.transmit(simpleDataPacketBuffer, SIMPLE_DATA_PACKET_SIZE);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Simple data packet successfully sent.");
    return true;
  } else {
    Serial.print("Error sending simple data packet. Error code: ");
    Serial.println(state);
    return false;
  }
}

bool sendImageFragmentDataPacketBuffer() {
  int state = radio.transmit(imageFragmentDataPacketBuffer, IMAGE_FRAGMENT_DATA_PACKET_SIZE);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Image fragment data packet successfully sent.");
    return true;
  } else {
    Serial.print("Error sending image fragment data packet. Error code: ");
    Serial.println(state);
    return false;
  }
}

void sendCameraData() {

  ImageFragmentDataPacket* imageFragmentDataPacket = reinterpret_cast<ImageFragmentDataPacket*>(imageFragmentDataPacketBuffer);

  Serial.println("Starting image capture...");
  CAM.takePicture(CAM_IMAGE_MODE_QVGA, CAM_IMAGE_PIX_FMT_JPG);

  uint8_t imageData[BUFFER_SIZE] = { 0 };  // Buffer for image data
  uint8_t imageDataCurrent = 0;            // Current image data
  uint8_t imageDataNext = 0;               // Next image data
  uint8_t headFlag = 0;                    // Flag to identify start and end of JPEG
  int i = 0;                               // Index for image buffer
  bool start = false;                      // Auxiliar bool for start

  // Loop to process image data
  while (CAM.getReceivedLength()) {

    imageDataCurrent = imageDataNext;
    imageDataNext = CAM.readByte();

    if (imageDataCurrent == 0xff && imageDataNext == 0xd8) {

      headFlag = 1;

      imageData[i++] = imageDataCurrent;
      imageData[i++] = imageDataNext;

      start = true;
    }else{
      if (imageDataCurrent == 0xff && imageDataNext == 0xd9) {

        headFlag = 0;

        Serial.println("JPEG end of image detected. Sending data...");

        imageFragmentDataPacket->start = start;
        imageFragmentDataPacket->end = true;
        imageFragmentDataPacket->dataSize = IMAGE_FRAGMENT_DATA_SIZE;
        memcpy(imageFragmentDataPacket->data, imageData, IMAGE_FRAGMENT_DATA_SIZE);

        printImageFragmentDataPacket(imageFragmentDataPacket);
        sendImageFragmentDataPacketBuffer();
        break;
      }else{
        if (headFlag == 1) {

          imageData[i++] = imageDataNext;

          if (i >= BUFFER_SIZE) {

            Serial.println("Image buffer full. Sending data...");

            imageFragmentDataPacket->start = start;
            imageFragmentDataPacket->end = false;
            imageFragmentDataPacket->dataSize = IMAGE_FRAGMENT_DATA_SIZE;
            memcpy(imageFragmentDataPacket->data, imageData, IMAGE_FRAGMENT_DATA_SIZE);

            printImageFragmentDataPacket(imageFragmentDataPacket);
            sendImageFragmentDataPacketBuffer();

            i = 0;
            start = false;
          }
        }
      }
    }
  }
}