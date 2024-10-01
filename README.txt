# PhiGuard Project

## Overview
PhiGuard is an autonomous surveillance robot designed to enhance security systems in environments such as airports and shopping centers. The robot features omnidirectional wheels and a camera, enabling versatile movement and monitoring capabilities.

## Components
- **Raspberry Pi**: Serves as the main processing unit, creating a database for facial recognition.
- **ESP32**: Handles wireless communications and interactions.
- **Arduino Nano**: Manages sensor input and motor control.
- **IR Sensors**: Eight infrared sensors allow for autonomous navigation.
- **Power Supply**: The robot is powered by rechargeable power banks.

## Functionality
1. **Data Acquisition**:
   - Performs web scraping from public databases (e.g., the most wanted list from the Guardia Civil) to download available data and images.
   - Maintains a custom blacklist using a Telegram bot for real-time updates.

2. **Facial Recognition**:
   - Utilizes OpenCV to compare faces detected by the camera against the database in real time.

3. **Alert System**:
   - When a match is found, the system sends an alert via SMTP email to security personnel or local police.

## Custom PCBs
The project includes two custom-designed PCBs:
- **Remote Control PCB**: For manual operation.
- **Robot PCB**: For autonomous functions and sensor integration.

## Installation
1. Clone the repository.
2. Set up the Raspberry Pi environment and install necessary libraries (e.g., OpenCV).
3. Configure the ESP32 and Arduino with the provided code.
4. Connect all components and power the system.

## Usage
To operate the robot:
- Use the remote control for manual navigation.
- Enable autonomous mode for automatic surveillance and alerts.

## Conclusion
PhiGuard demonstrates the integration of robotics, computer vision, and real-time data processing to enhance security measures in public spaces.


En esta carpeta encontrarás:
- Una carpeta "AvisosDeteccion_Email", con el archivo resultado de la detección captado por el Robot
- Una carpeta "Mando", con los códigos y archivos de PCB necesarios
- Una carpeta "Robot", con los códigos y archivos de PCB necesarios
- Un archivo "Documentacion-PhiGuard-ModProyecto.pdf", con la documentación presentada para el proyecto.
- Un archivo "Presentación_PhiGuard.pptx", con el PowerPoint en que me basé para la exposición del proyecto.