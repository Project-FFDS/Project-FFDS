## 📜 **Project Description**

The **Forest Fire Detection System (FFDS)** is a system designed to detect forest fires in their early stages and quickly alert authorities and response teams. The system uses a combination of sensors, thermal cameras, and artificial intelligence algorithms to monitor vast forest areas, helping to mitigate the devastating impacts of wildfires.

## 🛠 **Features**

- **Early Detection**: Identifies fires in their early stages using temperature sensors and infrared cameras.
- **Continuous Monitoring**: Provides continuous surveillance of forest areas with strategically placed sensor nodes.
- **Image Analysis**: Utilizes the Gemini API for image analysis and automatic risk classification.
- **Real-Time Notifications**: Sends immediate alerts to response teams in case of fire detection.
- **Advanced Gateway**: Efficient communication between sensor nodes and the cloud using cutting-edge technology such as the SX1281 and SIM800L modules.

## 🧩 **System Architecture**

# [![Forest Fire Detection System (FFDS) Video](https://img.youtube.com/vi/-TY7FCctu0s/0.jpg)](https://www.youtube.com/watch?v=-TY7FCctu0s) 

The FFDS is composed of the following components:

1. **Sensor Node**:
   - Temperature sensors, infrared cameras (AMG8833), and gas sensors (SCD30).
   - Local data processing and information transmission to the gateway.
   - ~~Data formatted in JSON compatible with the gateway.~~

2. **Gateway**:
   - Mediates communication between sensor nodes and the cloud.
   - Implemented with Arduino and SX1280/SX1281 transceivers.
   - Sends data to the cloud via 3G/4G using the SIM800L module.

3. **Cloud**:
   - Data storage and processing in Google Cloud Functions.
   - Integration with the Gemini API for image analysis.

## 📂 **Project Structure**
```plaintext
FFDS/
├── gateway/
│   ├── src/
│   │   └── gateway.ino
│   ├── docs/
│   └── README.md
├── node/
│   ├── src/
│   │   └── node.ino
│   └── README.md
├── cloud/
│   ├── functions/
│   │   ├── analyzeImage/
│   │   ├── images/
│   │   └── readings/
│   └── README.md
└── README.md
```
## 🌟 **Contributing**
Contributions are welcome! Please follow the contribution guidelines and ensure that all changes adhere to the project's standards.
```plaintext
1. Fork the repository.
2. Create a branch for your feature (git checkout -b feature/new-feature).
3. Commit your changes (git commit -m 'Add new feature').
4. Push the branch (git push origin feature/new-feature).
5. Open a Pull Request.
6. Share the idea with your friends!
```
## 📄 **License**
This project is licensed under the MIT License.

**Developed with ❤️ by Erick.**
