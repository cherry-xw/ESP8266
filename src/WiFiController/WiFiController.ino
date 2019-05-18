/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>

struct WiFiConfig = {
  const char* ssid = "TP-LINK_205A";
  const char* password = "19830930";
}

const int MAX_FAIL = 98;
int current_fail = 0;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(";");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    delay(100);
    Serial.print(".");
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    Serial.print(",");
    delay(100);
    current_fail++;
    if (current_fail > MAX_FAIL){
      WiFi.reconnect();
      while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
      }
    }
    server.begin();
    current_fail=0;
    return;
  }
  current_fail=0;

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  // Match the request
  int val;
  if (req.indexOf("/gpio/0") != -1) {
    val = 0;
  } else if (req.indexOf("/gpio/1") != -1) {
    val = 1;
  } else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  // Set GPIO2 according to the request
  digitalWrite(2, val);

  client.flush();

  // Prepare the response
  String html = CreateHTML(val);

  // Send the response to the client
  client.print(html);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

String CreateHTML(int val){
  String before = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  String head = "<!DOCTYPE HTML><html><head><meta charset=\"utf8\" /><title>Control 控制</title></head>";
  String body = "<body><p>val</p><div><a href=/gpio/1>on</a></div><div><a href=/gpio/0>off</a></div></body>";
  return before+head+body+"</html>\n";
}
