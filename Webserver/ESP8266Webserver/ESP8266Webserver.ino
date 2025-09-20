/******************************************************************************
 * Author: Jonathan Weaver
 * Adapted from code by Rui Santos, who's complete project details are at 
 * https://randomnerdtutorials.com
 * 
 * ESP Webserver
 *
 * Revised: 9/19/2025
 * 9/19/2005 - adapted code to get the output status from the ESP8266 rather 
 * than assume based on the code. This helps suppt  
******************************************************************************/

// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
//const char* ssid     = "ESP8862Wifi";
const char* ssid     = "Samantha's Wi-Fi Network";
const char* password = "2cropisfun2me";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputState = "off";

// Assign output variables to GPIO pins
const int output = 2;
const int input = 0;

// Current time
int timeCounter = 0;
unsigned long startTime = millis();
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 1000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output, LOW);

  // Initialize the input variable as an input
  pinMode(input, INPUT);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.setHostname("ESP8266");
  //WiFi.softAP(ssid);
  //WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //IPAddress IP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(IP);

  Serial.println("Starting webserver...");
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    
    currentTime = millis();
    previousTime = currentTime;
    timeCounter = (currentTime - startTime) / 1000;

    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /2/on") >= 0) {
              //Serial.println("GPIO 2 on");
              outputState = "on";
              digitalWrite(output, HIGH);
            } else if (header.indexOf("GET /2/off") >= 0) {
              //Serial.println("GPIO 2 off");
              outputState = "off";
              digitalWrite(output, LOW);
            } else if (header.indexOf("GET /inputs") >= 0) {
              client.println(timeCounter);
              if(digitalRead(input) == LOW){
                client.println(0);
              } else {
                client.println(1);
              }
              if(outputState == "off"){
                client.println(0);
              } else {
                client.println(1);
              }
              client.println();
              break;
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px; width: 375px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A; width: 375px;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Time webpage is running
            client.println("<h3><span>Timer:</span><span id=\"time\">");
            client.println(timeCounter);
            client.println("</span></h3>");
            
            // Display the current input state
            if(digitalRead(input) == LOW){
              client.println("<p><button id=\"input\" class=\"button\">Button is pressed</button></p>");
            } else {
              client.println("<p><button id=\"input\" class=\"button button2\">Button is not pressed</button></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 5  
            // If the output5State is off, it displays the ON button       
            if (outputState=="off") {
              client.println("<p><a id=\"outputHREF\" href=\"/2/on\"><button id=\"output\" class=\"button\">Blue LED is ON</button></a></p>");
            } else {
              client.println("<p><a id=\"outputHREF\" href=\"/2/off\"><button id=\"output\" class=\"button button2\">Blue LED is OFF</button></a></p>");
            } 
            client.println("</body>");
                        
            // Script for timer, input state, and output state
            client.println("<script>setInterval(function ( ) {");
            client.println("  var xhttp = new XMLHttpRequest();");
            client.println("  xhttp.onreadystatechange = function() {");
            client.println("    var lines = xhttp.responseText.split(\"\\n\");");
            client.println("    if (this.readyState == 4 && this.status == 200) {");
            client.println("      document.getElementById(\"time\").innerHTML = \"&nbsp;\" + lines[0];");
            client.println("      if(lines[2].indexOf(\"0\") >= 0) {");
            client.println("         document.getElementById(\"output\").innerHTML = \"Blue LED is ON\";");
            client.println("         document.getElementById(\"outputHREF\").href = \"/2/on\";");
            client.println("         document.getElementById(\"output\").className=\"button\";");
            client.println("      } else {");
            client.println("         document.getElementById(\"output\").innerHTML = \"Blue LED is OFF\";");
            client.println("         document.getElementById(\"outputHREF\").href = \"/2/off\";");
            client.println("         document.getElementById(\"output\").className=\"button button2\";");
            client.println("      }");
            client.println("      if(lines[1].indexOf(\"0\") >= 0) {");
            client.println("         document.getElementById(\"input\").innerHTML = \"Button is pressed\";");
            client.println("         document.getElementById(\"input\").className=\"button\";");
            client.println("      } else {");
            client.println("         document.getElementById(\"input\").innerHTML = \"Button is not pressed\";");
            client.println("         document.getElementById(\"input\").className=\"button button2\";");
            client.println("      }");            
            client.println("    }");
            client.println("  };");
            client.println("  xhttp.open(\"GET\", \"/inputs\", true);");
            client.println("  xhttp.send();"); 
            client.println("}, 100 ) ;");
            client.println("</script></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}