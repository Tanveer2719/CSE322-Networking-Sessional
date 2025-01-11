# CSE-322: Computer Networks Sessional - README

## Overview

The **CSE-322: Computer Networks Sessional** course involves several hands-on assignments aimed at providing students with practical experience in designing, implementing, and simulating various networking protocols, file server systems, and error detection and correction mechanisms. This document serves as a guide to the four major assignments that form part of the course curriculum.

---

### Assignments Overview:

1. **File Server System Implementation**:
   - **Objective**: Develop a file server system where clients can upload, download, and request files. The system is built using Java and includes secure client-server communication.
   - **Key Features**:
     - Unique client usernames for server connection.
     - Directory creation for clients upon first login.
     - File upload (private or public) and download.
     - File request system with broadcast messages.
     - Message management for unread messages.
     - Server notification when requested files are uploaded.
   
2. **Network Simulation Using NS-3**:
   - **Objective**: Simulate wireless high-rate networks (static and mobile) to measure various performance metrics.
   - **Key Features**:
     - Simulate static and mobile wireless networks with NS-3.
     - Topologies: Dumbbell topology for static and custom topologies for mobile networks.
     - Vary network parameters like number of nodes, flows, packets per second, and node speed.
     - Performance metrics measurement in different topologies.

3. **TCP Adaptive Reno (TCP-AReno) Congestion Control Algorithm**:
   - **Objective**: Implement and evaluate the TCP-AReno congestion control algorithm, based on TCP-Westwood-BBE, to address the limitations of traditional TCP-Reno.
   - **Key Features**:
     - Adaptive congestion window adjustment based on RTT and network congestion.
     - Faster congestion window expansion to improve throughput.
     - Performance improvement in large bandwidth-delay product networks.
     - Friendly behavior towards other TCP variants such as TCP-Reno.

4. **Error Detection and Correction**:
   - **Objective**: Implement error detection using CRC checksum and error correction using Hamming distance, focusing on data link layer protocols.
   - **Key Features**:
     - CRC checksum for detecting errors in transmitted data.
     - Hamming distance to detect and correct single-bit errors.
     - Application of these techniques in data link layer protocols to ensure reliable communication.

---

## Requirements

- **Software**:
  - **Java** (for File Server System): Ensure that you have the Java Development Kit (JDK) installed to compile and run the file server system.
  - **NS-3** (for Network Simulation): Install NS-3 for network simulations.
  - **NS-3 Dependencies**: Ensure all required dependencies and modules are installed for proper simulation and evaluation.
  
- **Hardware**:
  - A computer with sufficient resources (RAM and storage) for running network simulations and handling server-client operations.
  - Linux based systems will be a plus point.
  
---

## Features and Functionalities

### File Server System
- **User Authentication**: Each client has a unique username, and duplicate login attempts are rejected.
- **Client Directory Management**: A directory is created for each client to store uploaded files.
- **File Upload and Download**: Clients can upload and download both private and public files.
- **File Requests**: Clients can broadcast file requests, which other clients can fulfill.
- **Notification System**: When a requested file is uploaded, the requesting client is notified.

### Network Simulation
- **Wireless Network Simulation**: Simulate both static and mobile wireless networks using NS-3.
- **Customizable Parameters**: Adjust the number of nodes, flows, packets per second, and other parameters.
- **Performance Metrics**: Measure and record the performance of the network under different configurations.

### TCP-AReno Algorithm
- **Congestion Window Adjustments**: Adjust the congestion window dynamically based on RTT measurements and congestion levels.
- **Improved Throughput**: Compare the throughput of TCP-AReno with traditional TCP-Reno in various network conditions.

### Error Detection & Correction
- **CRC Checksum**: Implement CRC for error detection in data transmission.
- **Hamming Distance**: Use Hamming distance to detect and correct single-bit errors in transmitted data.

---

For further questions, keep it to yourself. I have long forgotten what I had done in this course.

 
