#include <ESP8266WiFi.h>

const char* ssid = "esp-test";
const char* password = "Hello There";

// Drinks Available
int numCoke = 100;
int numMelloYello = 1000;
int numRootBeer = 0;
int numFanta = 0;
int numSprite = 120;
String drinkOrder = "";


// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);
                                                
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

  // Check if STM is sending data
  getUARTData();
  
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

  if (compare.equals("Cancel Order")) {
    cancelDrink(client);
  } else if (compare.equals("Calc Update")){
    updateNum(client);
  } else {
    orderDrink(client, compare);
  }
 
  delay(1);
  //Serial.println("Sending order");
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

void getUARTData() {
  char charValue;
  String strRecvd;
  int gotFlag = 0;
  while (Serial.available() || gotFlag != 0) {
    charValue = Serial.read();
    strRecvd = strRecvd + charValue;
    gotFlag++;
    if (strRecvd.equals("Done")) {
      gotFlag = 0;
      drinkOrder = "";
    }
  }
}

String matchRequest(String req) {
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

  return compare;
}

void updateNum(WiFiClient client) {
  Serial.println("In UpdateNum");
  client.print("Got Initial Request");
  
  // Wait for client to send number of drinks to update
  while(!client.available()) {
      delay(1);
  }

  // Get data and parse to get number itself
  String recvd = client.readString();
  client.flush();

  int startIndex = 2;
  int flag = 0;
  int temp = 0;
  
  // Match the request
  for (temp = 0; temp < recvd.length(); temp++) {
    if (recvd[temp] == 'C' && flag == 0) {
      startIndex = temp;
      flag = 1;
    }
  }

  recvd = String(recvd.substring(startIndex, recvd.length()));
  Serial.print("Update Request: ");
  Serial.println(recvd);
  //recvd = matchRequest(recvd);
  recvd = recvd.substring(5);
  Serial.print("Substring: ");
  Serial.println(recvd);
  int update = recvd.toInt();

  // Send ack to client and get drink name to update
  client.print("Got Number to order");
  while(!client.available()) {
      delay(1);
  }
  String recvd2 = client.readString();
  client.flush();

  Serial.print("Drink Stock Before: ");
  Serial.println(recvd2);
  
  startIndex = 2;
  flag = 0;
  temp = 0;
  
  // Match the request
  for (temp = 0; temp < recvd2.length(); temp++) {
    if (recvd2[temp] == 'C' && flag == 0) {
      startIndex = temp;
      flag = 1;
    }
  }
  recvd2 = String(recvd2.substring(startIndex, recvd2.length()));
  Serial.print("Drink Stock: ");
  Serial.println(recvd2);
  
  // Send ack to client
  client.print("Got Drink Name");

  // Update total amount of drinks in stock
  if (recvd2.equals("Calc Coke")) {
    numCoke = numCoke + update;
  } else if (recvd2.equals("Calc Mello Yello")) {
    numMelloYello = numMelloYello + update;
  } else if (recvd2.equals("Calc Root Beer")) {
    numRootBeer = numRootBeer + update;
  } else if (recvd2.equals("Calc Fanta")) {
    numFanta = numFanta + update;
  } else if (recvd2.equals("Calc Sprite")) {
    numSprite = numSprite + update;
  }
}

void cancelDrink(WiFiClient client) {
  if (drinkOrder.equals("Coke")) {
    numCoke = numCoke + 1;
  } else if (drinkOrder.equals("Mello Yello")) {
    numMelloYello = numMelloYello + 1;
  } else if (drinkOrder.equals("Root Beer")) {
    numRootBeer = numRootBeer + 1;
  } else if (drinkOrder.equals("Fanta")) {
    numFanta = numFanta + 1;
  } else if (drinkOrder.equals("Sprite")) {
    numSprite = numSprite + 1;
  }

  client.print("Order Cancelled");
  Serial.println("Cancel Order to Micro");
  //Serial.write("Cancel Order");
}

void orderDrink(WiFiClient client, String compare) {
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
      drinkOrder = "Coke";
      Serial.println(numCoke);
    }
  } else if (compare.equals("Check Mello Yello")) {
    if (numMelloYello > 0) {
      sendBack = "Available";
      numMelloYello = numMelloYello - 1;
      Serial.print("Mello Yellos left: ");
      drinkOrder = "Mello Yello";
      Serial.println(numMelloYello);
    }
  } else if (compare.equals("Check Root Beer")) {
    if (numRootBeer > 0) {
      sendBack = "Available"; 
      numRootBeer = numRootBeer - 1;
      Serial.print("Root Beers left: ");
      drinkOrder = "Root Beer";
      Serial.println(numRootBeer);
    }
  } else if (compare.equals("Check Fanta")) {
    if (numFanta > 0) {
      sendBack = "Available";
      numFanta = numFanta - 1;
      Serial.print("Fantas left: ");
      drinkOrder = "Fanta";
      Serial.println(numFanta);
    }
  } else if (compare.equals("Check Sprite")) {
    if (numSprite > 0) {
      sendBack = "Available";
      numSprite = numSprite - 1;
      Serial.print("Sprites left: ");
      drinkOrder = "Sprite";
      Serial.println(numSprite);
    }
  }

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
    String sendOrder = drinkOrder + "," + newreq + "\r";
    //Serial.write(sendOrder);
    Serial.println("Sending Order");
  }

}


