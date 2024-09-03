## 📜 **Sensor Node**

This document provides an overview of the sensor node's code in the Project FFDS. The sensor node is responsible for collecting environmental data and capturing images, which are then sent to a mid or end gateway.

## 🛠 **Features**

- **Environmental Monitoring**: Measures temperature and CO2 levels using Adafruit sensors.
- **Image Capture**: Captures images with the Arducam Mega and transmits them in fragments to the gateway.
- **Data Transmission**: Utilizes the SX1281 radio module for wireless communication with the gateway.
- **Custom Protocol**: Implements a binary communication protocol using structs to efficiently handle data.

## 🧩 **Components**

The sensor node is built using the following components:

- **Adafruit SCD30**:
   - CO2 and temperature sensor.

- **Adafruit AMG88xx**:
   - Thermal sensor.

- **Arducam Mega**:
   - Camera module for image capture.

- **SX1281**:
   - Radio module for long-range communication.

- **GPS GY-NEO6MV2**:
    - GPS module for collecting geolocation data.

## 💻 **Installation**

To get started with the sensor node, clone the repository and ensure that you have the necessary libraries installed:
```bash
git clone https://github.com/Project-FFDS/Project-FFDS
```

## 📚 **Libraries**

- **TinyGPS++**
- **SoftwareSerial**
- **SPI**
- **Wire**
- **RadioLib**
- **Adafruit_SCD30**
- **Adafruit_AMG88xx**
- **Arducam_Mega**

## 🔌 **Setup**

1. Connect the hardware components.

2. Open the project in your preferred IDE (e.g., Arduino IDE).

3. Install the required libraries via the Library Manager.

4. Upload the code to the microcontroller.

## 📓 **Notes**

### **Buffer size**

The node uses a fixed payload size of 255 bytes for consistency across transmissions. Adjustments to buffer size or struct definitions can be made in the code to accommodate different hardware configurations.

### **Image Transmission**

Images are transmitted in fragments using the ImageFragmentDataPacket struct. Ensure that you are using the compatible version of the gateway to receive and reassemble these fragments correctly.

## **Usage**

Once the code is uploaded, the sensor node will automatically start collecting data and sending it to the gateway. The node's status and any errors will be reported via the serial monitor. Feel free to add more logs if you want 🔥🎉.

## 🐛 **Debugging**

For troubleshooting, use the serial monitor to view debug messages. If you encounter issues with communication or sensor readings, double-check connections and ensure that all components are functioning correctly. Remember that we are a community, if you need help feel free to contact me 💙💛💚💜🖤.

## 📄 **License**
This project is licensed under the MIT License.

**Developed with ❤️ by Erick.**