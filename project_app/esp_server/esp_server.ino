#include <ESP8266WiFi.h>

const char* ssid = "esp-test";
const char* password = "Hello There";

// Drinks Available
int numCoke = 100;
int numMelloYello = 1000;
int numRootBeer = 0;
int numFanta = 0;
int numSprite = 120;


// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

                                                // prepare GPIO2
                                                //pinMode(2, OUTPUT);
                                                //digitalWrite(2, 0);
                                                
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
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
    return;
  }
  client.println("Hello Client. I am free to receive an order");
  
  // Wait until the client sends drink request
  Serial.println("new client");
  while(!client.available()){
    delay(1);
    //Serial.println("Lost Connection");
    //client.stop();
    //return;
  }
  
  // Read the first line of the request
  String req = client.readString();//client.readStringUntil('\r');
  //Serial.println(String("Request: " + req));
  client.flush();
  
                                                  // Match the request
                                                //  int val;
                                                //  if (req.indexOf("/gpio/0") != -1)
                                                //    val = 0;
                                                //  else if (req.indexOf("/gpio/1") != -1)
                                                //    val = 1;
                                                //  else {
                                                //    Serial.println("invalid request");
                                                //    client.stop();
                                                //    return;
                                                //  }
                                                //
                                                //  // Set GPIO2 according to the request
                                                //  digitalWrite(2, val);
                                                //  
                                                //  client.flush();

  Serial.print("Request: ");
  Serial.println(req);

  //Serial.print("Request Length: ");
  //Serial.println(req.length());
  int startIndex = 2;
  int flag = 0;
  int temp = 0;
  
  // Match the request
  for (temp = 0; temp < req.length(); temp++) {
    //Serial.print(temp);
    //Serial.print(": ");
    //Serial.println(req[temp]);
    if (req[temp] == 'C' && flag == 0) {
      startIndex = temp;
      flag = 1;
      //Serial.print("Start Index: ");
      //Serial.println(temp);
    }
  }
  String compare = String(req.substring(startIndex, req.length()));
  //Serial.print("compare: ");
  //Serial.println(compare);
  
  /*
  for (temp = 0; temp < compare.length(); temp++) {
    Serial.print(temp);
    Serial.print(": ");
    Serial.println(compare[temp]);
  }*/
  
  String sendBack = "Not Available";
  if (compare.equals("Check Coca Cola")) {
    if (numCoke > 0) {
      sendBack = "Available";
      numCoke = numCoke - 1;
      Serial.print("Cokes left: ");
      Serial.println(numCoke);
    }
  } else if (compare.equals("Check Mello Yello")) {
    if (numMelloYello > 0) {
      sendBack = "Available";
      numMelloYello = numMelloYello - 1;
      Serial.print("Mello Yellos left: ");
      Serial.println(numMelloYello);
    }
  } else if (compare.equals("Check Root Beer")) {
    if (numRootBeer > 0) {
      sendBack = "Available"; 
      numRootBeer = numRootBeer - 1;
      Serial.print("Root Beers left: ");
      Serial.println(numRootBeer);
    }
  } else if (compare.equals("Check Fanta")) {
    if (numFanta > 0) {
      sendBack = "Available";
      numFanta = numFanta - 1;
      Serial.print("Fantas left: ");
      Serial.println(numFanta);
    }
  } else if (compare.equals("Check Sprite")) {
    if (numSprite > 0) {
      sendBack = "Available";
      numSprite = numSprite - 1;
      Serial.print("Sprites left: ");
      Serial.println(numSprite);
    }
  }

                                          // Prepare the response
                                          //String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
                                          //s += (val)?"high":"low";
                                          //s += "</html>\n";

  // Send the response to the client
  client.print(sendBack);                 //client.print(s);
  
  // Wait for GPS Coordinates if drink is available
  if (sendBack.equals("Available")) {

    while(!client.available()){
      delay(1);
    }
    String newreq = client.readString();
    client.flush();
    Serial.print("GPS: ");
    Serial.println(newreq);
    

    // Send response to client
    client.print("Delivering Order");

    Serial.println("Sending Order");
  }
  
  delay(1);
  //Serial.println("Sending order");
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}


