#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "config.h"

// Static IP configuration
IPAddress local_IP(192, 168, 0, 186);   // Set static IP
IPAddress gateway(192, 168, 0, 1);      // Set gateway (usually your router IP)
IPAddress subnet(255, 255, 255, 0);     // Set subnet mask

// Create a web server on port 80
ESP8266WebServer server(80);

// HTML content to be served
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Home</title>
    <!-- Bootstrap CSS -->
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <!-- Font Awesome -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">

    <style>
        body {
            background-color: #f8f9fa;
        }

        .card {
            width: 18rem;
            margin-right: 1rem;
            margin-bottom: 1rem;
        }

        .card-title {
            font-size: 1.5rem;
        }

        .btn-toggle {
            width: 100%;
        }

        .styled-input {
            width: 80%;
            margin: 0 auto;
            font-size: 18px;
        }

        .btn {
            margin:0px 20px;
        }

        .live-temp {
            font-size: 24px;
        }
    </style>
</head>

<body>
    <div class="container">
    <div class="container mt-4">
        <div class="row">
            <div class="col-md-12 text-center">
                <h1 class="display-4">Zico Corp.</h1>
                <h3 class="display-6">Headquarters Heating</h3>
            </div>
        </div>
    </div>

    <div class="container mt-4">
        <div class="row">
            <div class="col-md-4">
                <div class="card text-center">
                    <div class="card-body">
                        <h5 class="card-title"><i class="fas fa-home"></i> Living Room</h5>
                        <div class="d-flex justify-content-center mb-3">
                            <p id="living-temp" class="text-4xl font-weight-bold push-down live-temp">...</p>
                        </div>
                        <div class="d-flex justify-content-center">
                            <button onclick="adjustTemperature('living', -0.5)" class="btn btn-danger ms-2">-</button>
                            <input type="number" step="0.5" id="living-input"
                                class="form-control text-4xl text-center font-weight-bold push-down styled-input" value="20.0">
                            <button onclick="adjustTemperature('living', 0.5)" class="btn btn-success me-2">+</button>
                        </div>
                        <div class="d-flex justify-content-center mt-3">
                            <button onclick="setWantedTemperature('living')" class="btn btn-primary">Set</button>
                            <!-- <div id="set-temp-response-living"></div> -->
                        </div>
                        <div class="d-flex justify-content-center mt-3">
                            <button id="auto-toggle-living" class="btn btn-warning btn-toggle"
                                onclick="toggleAutoMode('living')">Auto Mode: Off</button>
                        </div>
                    </div>
                </div>
            </div>

            <div class="col-md-4">
                <div class="card text-center">
                    <div class="card-body">
                        <h5 class="card-title"><i class="fas fa-bed"></i> Bedroom</h5>
                        <div class="d-flex justify-content-center mb-3">
                            <p id="bedroom-temp" class="text-4xl font-weight-bold push-down live-temp">...</p>
                        </div>
                        <div class="d-flex justify-content-center">
                            <button onclick="adjustTemperature('bedroom', -0.5)" class="btn btn-danger ms-2">-</button>                            
                            <input type="number" step="0.5" id="bedroom-input"
                                class="form-control text-4xl text-center font-weight-bold push-down styled-input" value="20.0">

                            <button onclick="adjustTemperature('bedroom', 0.5)" class="btn btn-success me-2">+</button>
                        </div>
                        <div class="d-flex justify-content-center mt-3">
                            <button onclick="setWantedTemperature('bedroom')" class="btn btn-primary">Set</button>
                            <!-- <div id="set-temp-response-bedroom"></div> -->
                        </div>
                        <div class="d-flex justify-content-center mt-3">
                            <button id="auto-toggle-bedroom" class="btn btn-warning btn-toggle"
                                onclick="toggleAutoMode('bedroom')">Auto Mode: Off</button>
                        </div>
                    </div>
                </div>
            </div>

            <div class="col-md-4">
                <div class="card text-center">
                    <div class="card-body">
                        <h5 class="card-title"><i class="fas fa-utensils"></i> Kitchen</h5>
                        <div class="d-flex justify-content-center mb-3">
                            <p id="kitchen-temp" class="text-4xl font-weight-bold push-down live-temp">...</p>
                        </div>
                        <div class="d-flex justify-content-center">
                            <button onclick="adjustTemperature('kitchen', -0.5)"
                                class="btn btn-danger ms-2">-</button>                            
                            <input type="number" step="0.5" id="kitchen-input"
                                class="form-control text-4xl text-center font-weight-bold push-down styled-input" value="20.0">
                            <button onclick="adjustTemperature('kitchen', 0.5)" class="btn btn-success me-2">+</button>
                        </div>
                        <div class="d-flex justify-content-center mt-3">
                            <button onclick="setWantedTemperature('kitchen')" class="btn btn-primary">Set</button>
                            <!-- <div id="set-temp-response-kitchen"></div> -->
                        </div>
                        <div class="d-flex justify-content-center mt-3">
                            <button id="auto-toggle-kitchen" class="btn btn-warning btn-toggle"
                                onclick="toggleAutoMode('kitchen')">Auto Mode: Off</button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</div>

    <script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.10.2/dist/umd/popper.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.min.js"></script>
    <script src="https://code.jquery.com/jquery-3.6.4.min.js"></script>

    <script>
        // Function to fetch and print the current temperature
        function fetchAndPrintCurrentTemperature(room) {
            var ip = getRoomIP(room);
            var endpoint = ip + "/currentTemp";
            fetch(endpoint)
                .then(response => response.text())
                .then(data => {
                    // Print the value as a number
                    var currentTempElement = document.getElementById(room + '-temp');
                    currentTempElement.innerText = parseFloat(data) + '°C';
                })
                .catch(error => {
                    console.error('Error fetching current temperature:', error);
                });
        }

        // Function to fetch and set Auto Mode on page load
        function fetchAndSetAutoMode(room) {
            var ip = getRoomIP(room);
            var endpoint = ip + "/currentMode";
            fetch(endpoint)
                .then(response => response.text())
                .then(data => {
                    var button = document.getElementById('auto-toggle-' + room);
                    if (parseInt(data) === 1) {
                        button.innerText = 'Auto Mode: On';
                    } else {
                        button.innerText = 'Auto Mode: Off';
                    }
                })
                .catch(error => {
                    console.error('Error fetching Auto Mode status:', error);
                });
        }

        // Function to get the IP address based on the room name
        function getRoomIP(room) {
            switch (room) {
                case 'living':
                    return "http://192.168.0.187";
                case 'bedroom':
                    return "http://192.168.0.188";
                case 'kitchen':
                    return "http://192.168.0.189";
                default:
                    // Return a default value or handle the case as needed
                    return "";
            }
        }

        // Function to adjust temperature
        function adjustTemperature(room, change) {
            var inputElement = document.getElementById(room + '-input');
            var currentValue = parseFloat(inputElement.value);
            var newValue = currentValue + change;
            inputElement.value = newValue.toFixed(1);
        }

        // Function to set the wanted temperature
        function setWantedTemperature(room) {
            var ip = getRoomIP(room);
            var endpoint = ip + "/setWantedTemp";
            var wantedTemp = document.getElementById(room + '-input').value;

            // Call the provided sendPostRequest method
            sendPostRequest(endpoint, 'wanted_temp=' + wantedTemp, 'set-temp-response-' + room);
        }

        // Function to send a POST request
        function sendPostRequest(url, body, responseElementId) {
            fetch(url, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: body,
            })
            .then(response => response.text())
            .then(data => {
                // Display the response in the specified element
                document.getElementById(responseElementId).innerText = data;
            })
            .catch(error => {
                console.error('Error:', error);
            });
        }

        // Function to toggle auto mode
        function toggleAutoMode(room) {
            var ip = getRoomIP(room);
            var endpoint = ip + "/setAuto";
            var button = document.getElementById('auto-toggle-' + room);

            // Toggle the button text and value
            if (button.innerText.includes('On')) {
                button.innerText = 'Auto Mode: Off';
                sendPostRequest(endpoint, 'auto_set=0', 'auto-response');
            } else {
                button.innerText = 'Auto Mode: On';
                sendPostRequest(endpoint, 'auto_set=1', 'auto-response');
            }
        }

        // Initial fetch and print of current temperature
        fetchAndPrintCurrentTemperature('living');
        fetchAndPrintCurrentTemperature('bedroom');
        fetchAndPrintCurrentTemperature('kitchen');

        // Fetch and print the current temperature every 60 seconds
        setInterval(function () {
            fetchAndPrintCurrentTemperature('living');
            fetchAndPrintCurrentTemperature('bedroom');
            fetchAndPrintCurrentTemperature('kitchen');
        }, 1000);

        // Fetch and set Auto Mode on page load
        fetchAndSetAutoMode('living');
        fetchAndSetAutoMode('bedroom');
        fetchAndSetAutoMode('kitchen');
    </script>
</body>

</html>

)rawliteral";

// Handle root URL "/"
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  // Set static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure IP");
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define the route for "/"
  server.on("/", handleRoot);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
}
