# Arduino Libraries Guide

## What Are Libraries and Why Do We Need Them?

When working with microcontrollers like the ESP32 or Arduino, you'll quickly discover that many tasks require complex code to accomplish seemingly simple things. For example, connecting to WiFi, controlling an E-Paper display, or parsing data from the internet involves hundreds of lines of intricate code that would take months to write and debug from scratch.

**Libraries are pre-written code packages** that handle these complex tasks for you. Think of them as digital toolboxes - instead of building every tool yourself, you can use tools that experts have already created, tested, and optimized. This allows you to focus on your project's unique features rather than reinventing the wheel.

### Why Include Libraries in Your Project Repository?

Including libraries directly in your project repository serves several important purposes:

1. **Version Control**: Ensures everyone uses the exact same library versions, preventing compatibility issues
2. **Project Completeness**: Makes your project self-contained and easier to share
3. **Future-Proofing**: Protects against libraries being updated or removed from online repositories
4. **Offline Development**: Allows development without internet access to download libraries
5. **Consistency**: Guarantees that your project works the same way for everyone who downloads it

## Libraries in This Project

This folder contains the essential libraries needed for the Rejseplanen E-Paper Display project. Each library handles a specific aspect of the project's functionality:

### ArduinoJson-master.zip
**Purpose**: Handles JSON data parsing and creation
**Why We Need It**: The Rejseplanen API returns data in JSON format (a structured text format used for data exchange). This library makes it easy to extract specific information (like bus arrival times, route numbers, destinations) from the API response without having to manually parse complex text strings.

### ESP8266HttpClient-master.zip
**Purpose**: Enables HTTP/HTTPS communication for ESP8266-based boards
**Why We Need It**: This library provides the tools to make web requests to APIs over the internet. It handles the complex networking protocols needed to fetch data from the Rejseplanen API servers.

### ESP8266wifi-master.zip
**Purpose**: Manages WiFi connectivity for ESP8266 boards
**Why We Need It**: Provides the fundamental WiFi functions needed to connect your microcontroller to your home network. Without internet connectivity, the device cannot fetch real-time bus and train schedules.

### HttpClient-master.zip
**Purpose**: General HTTP client functionality
**Why We Need It**: A more general-purpose HTTP client that can work across different microcontroller platforms. It provides additional HTTP functionality that might be needed for robust web communication.

### WiFi-master.zip
**Purpose**: Core WiFi functionality (likely for ESP32 or general Arduino WiFi shields)
**Why We Need It**: Provides the basic WiFi connection and management functions. This is the foundation that allows your device to connect to the internet.

## How to Install These Libraries in Arduino IDE

There are two ways to install these libraries, and since they're included in this repository, we recommend the manual method:

### Method 1: Manual Installation (Recommended for this project)

1. **Download the Libraries**: All libraries are already provided in the `Libraries` folder of this repository
2. **Locate Your Arduino Libraries Folder**:
   - **Windows**: `Documents\Arduino\libraries\`
   - **Mac**: `~/Documents/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`
3. **Extract and Install**:
   - Extract each .zip file
   - Copy the extracted folders to your Arduino libraries directory
   - The folder structure should look like: `Arduino/libraries/ArduinoJson-master/`
4. **Restart Arduino IDE**: Close and reopen the Arduino IDE to recognize the new libraries

### Method 2: Arduino Library Manager (Alternative)

1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries**
3. Search for each library by name (without the "-master" suffix)
4. Click **Install** for each library

**Note**: Using Method 1 ensures you have the exact versions tested with this project.

## Verifying Installation

To check if libraries are properly installed:

1. Open Arduino IDE
2. Go to **Sketch** → **Include Library**
3. You should see all the installed libraries listed under "Contributed libraries"

## Understanding Library Dependencies

Libraries often depend on other libraries to function properly. The libraries in this collection work together:

- **WiFi libraries** provide the network connection
- **HTTP client libraries** use the WiFi connection to communicate with web servers
- **ArduinoJson** processes the data received by the HTTP clients

This is why we include multiple libraries - each handles a specific part of the overall functionality, and they work together to create the complete system.

## Troubleshooting Common Issues

### "Library not found" errors
- Ensure libraries are in the correct Arduino libraries folder
- Restart Arduino IDE after installing libraries
- Check that folder names don't have extra characters or spaces

### Compilation errors
- Make sure you're using compatible library versions
- Some libraries are specific to certain board types (ESP8266 vs ESP32)
- Update your board definitions in Arduino IDE

### Version conflicts
- If you have multiple versions of the same library, remove older versions
- Use the versions provided in this repository for guaranteed compatibility

## Next Steps

Once you have these libraries installed, you'll be ready to:
1. Configure your WiFi credentials in the main project code
2. Set up your specific bus stop location
3. Upload the code to your ESP32
4. Watch real-time transit information appear on your E-Paper display!

Remember: Libraries are your friends in microcontroller programming. They handle the complex technical details so you can focus on building amazing projects.