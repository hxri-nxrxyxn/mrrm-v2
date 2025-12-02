***

# Project Proposal: Intelligent River Level Monitoring & Flood Prediction System

### 1. Aim
To develop a low-cost, robust, continuous river level monitoring system using **Computer Vision (IoT)** to replace unreliable ultrasonic sensors. The system aims to provide real-time data and predict flood risks **6 hours in advance**, integrating hydraulic modeling for impact assessment.

### 2. The Problem with Current Solutions
* **Ultrasonic Limitations:** Existing sensors fail during high water (dead zone blinding) and low water (signal dissipation/wind interference).
* **Cost:** Industrial radar level sensors are prohibitively expensive.

### 3. Proposed Solution
We propose a **Visual Staff Gauge System**. An ESP32-CAM node captures images of a specially marked pole (Staff Gauge) in the river. The server processes these images to calculate water height, immune to wind, debris, or sensor blinding.

### 4. Team Responsibilities

#### A. Civil Engineering Team (Infrastructure & Hydraulics)
* **The Physical Interface:** Design and construct the "Fiducial Staff Gauge"—a pole with high-contrast neon bands (Day) and retro-reflective markers (Night) for 24/7 visibility.
* **Hydraulic Modeling:** Develop a **HEC-RAS model** using the sensor data to generate Flood Inundation Maps (determining *which* areas will flood at specific water levels).
* **Surveying:** Perform leveling surveys to calibrate the "Gauge Zero" to Mean Sea Level (MSL), ensuring data matches government standards (CWC).
* **Stabilization:** Design a "Stilling Well" or rigid mounting structure to dampen wave turbulence and ensure image stability.

#### B. Robotics/Electronics Team (Hardware & Deployment)
* **Node Development:** Configure the **ESP32-CAM** (USB variant) with optimized capture logic (Wake -> Flash/IR -> Capture -> Sleep).
* **Power & Enclosure:** Engineer an IP66 waterproof housing with heat dissipation logic. Manage power supply via Grid (5V USB) or Solar (with TP4056 & Li-ion batteries).
* **Data Transport:** Implement a robust HTTP POST protocol to transmit images and telemetry (voltage, timestamps) to the central server, ensuring auto-recovery from WiFi drops.

#### C. Computer Science Team (Vision & Intelligence)
* **Image Processing:** Develop an **OpenCV** pipeline to detect color bands (Day) and reflective glint (Night) to calculate the water level in centimeters.
* **Predictive Analysis:** Implement linear regression and acceleration algorithms to calculate the "Rate of Rise" and forecast water levels 6 hours ahead.
* **Dashboard & Alerting:** Build a real-time web dashboard for visualization and an automated alert system (Telegram/SMS) for critical thresholds.

### 5. Methodology

**Step 1: The "Zebra" Pole (Civil)**
A PVC pipe painted with alternating 10cm **Neon Orange** and **Matte Black** bands. A strip of **Retro-Reflective tape** is placed in the center of orange bands. This allows the camera to see "Color" in the day and "Light" at night (via camera flash).

**Step 2: The Capture (Robotics)**
The ESP32-CAM wakes up every 10–30 minutes, triggers a flash/illuminator, captures a VGA (640x480) image, and uploads it to the server.

**Step 3: The Analysis (CS)**
The server isolates the pole in the image, counts the visible bands/reflectors, subtracts this from the total pole height, and logs the current water level.

### 6. Budget Estimate (Per Node)

| Component | Cost (INR) |
| :--- | :--- |
| ESP32-CAM (MB USB Version) | ₹600 |
| IP66 Housing (Dummy Cam / Junction Box) | ₹350 |
| Power Adapter / Cable | ₹300 |
| Reflective Tape & Paint | ₹150 |
| Misc (Mounts, Glue) | ₹200 |
| **Total** | **~ ₹1,600** |

### 7. Expected Outcomes
1.  **Zero Blind Spots:** System works from 0cm to Infinity (as long as line-of-sight exists).
2.  **Night Capability:** Retro-reflector logic ensures clear data even in pitch darkness.
3.  **Actionable Intelligence:** Moving beyond "Current Level" to "Predicted Impact" (e.g., "Road X will submerge in 4 hours").

***
