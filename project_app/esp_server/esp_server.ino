#include <ESP8266WiFi.h>

// Drinks Available
int numCoke = 0;
int numMelloYello = 0;
int numRootBeer = 0;
int numFanta = 0;
int numSprite = 0;
String drinkOrder = "";

int pinLED = 2;


// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(1058);

void setup() {
  Serial.begin(115200);
  delay(10);

  //Set Pin Modes
  pinMode(pinLED, OUTPUT); 
                                                
  // Connect to WiFi network
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.begin("Imad", "36e03fvcy2c0n");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Start the server
  server.begin();
}

void loop() {

  // Check if STM is sending data
  getUARTData();

  // Check to see if WiFi is still connected. If it is, turn on LED
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(pinLED, HIGH);
  } else {
    digitalWrite(pinLED, LOW);
  }
  
  // Check if a client has connected
  WiFiClient client = server.available();
  client.setTimeout(5000);
  if (!client) {
    return;
  }

  // Send initial response to client
  client.println("Hello Client. I am free to receive an order");
  
  // Wait until the client sends drink request
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String compare = matchRequest(client); 

  if (compare.equals("Cancel Order")) {
    cancelDrink(client);
  } else if (compare.equals("Calc Update")){
    updateNum(client);
  } else if (compare.equals("Check Stock")) {
    sendCurrStock(client);
  } else {
    orderDrink(client, compare);
  }
 
  delay(1);

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

String matchRequest(WiFiClient client) {
  String req = client.readString();
  client.flush();
  

  int startIndex = 2;
  int flag = 0;
  int temp = 0;
  
  // Match the request
  for (temp = 0; temp < req.length(); temp++) {
    if ((req[temp] == 'C' || req[temp] == '+' || req[temp] == '-') && flag == 0) {
      startIndex = temp;
      flag = 1;
    }
  }
  
  String compare = String(req.substring(startIndex, req.length()));

  return compare;
}

void sendCurrStock(WiFiClient client) {
  client.print("Got Stock Request");

  // Wait for client to send number of drinks to update
  while(!client.available()) {
      delay(1);
  }

   // Get data and parse to get number itself
  String recvd = matchRequest(client);
  
  String stockAmount = "Stock is ";
  if (recvd.equals("Check Coca Cola")) {
    stockAmount = stockAmount + numCoke;
  } else if (recvd.equals("Check Mello Yello")) {
    stockAmount = stockAmount + numMelloYello;
  } else if (recvd.equals("Check Root Beer")) {
    stockAmount = stockAmount + numRootBeer;
  } else if (recvd.equals("Check Fanta")) {
    stockAmount = stockAmount + numFanta;
  } else if (recvd.equals("Check Sprite")) {
    stockAmount = stockAmount + numSprite;
  }

  client.print(stockAmount);
}

void updateNum(WiFiClient client) {
  client.print("Got Initial Request");
  
  // Wait for client to send number of drinks to update
  while(!client.available()) {
      delay(1);
  }

  // Get data and parse to get number itself
  String recvd = matchRequest(client);
  
  recvd = recvd.substring(5);
  int update = recvd.toInt();

  // Send ack to client and get drink name to update
  client.print("Got Number to order");
  while(!client.available()) {
      delay(1);
  }
  String recvd2 = matchRequest(client);
  

  // Update total amount of drinks in stock
  if (recvd2.equals("Calc Coca Cola")) {
    numCoke = update;
  } else if (recvd2.equals("Calc Mello Yello")) {
    numMelloYello = update;
  } else if (recvd2.equals("Calc Root Beer")) {
    numRootBeer = update;
  } else if (recvd2.equals("Calc Fanta")) {
    numFanta = update;
  } else if (recvd2.equals("Calc Sprite")) {
    numSprite = update;
  }

  // Send ack to client
  client.print("Got Drink Name");
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
  drinkOrder = "";
  client.print("Order Cancelled");
  Serial.write("C\r"); // Cancel Order
}

void orderDrink(WiFiClient client, String compare) {
  String sendBack = "Not Available";
	String a = ""; // temp variable

	// TODO: Need to fix later
	if (drinkOrder.equals("")) {
		a = "Hi";
	} else {	
		client.print("Wrong client");
		return;
	}

  if (compare.equals("Check Coca Cola")) {
    if (numCoke > 0) {
      sendBack = "Available";
      numCoke = numCoke - 1;
      drinkOrder = "Coke";
    }
  } else if (compare.equals("Check Mello Yello")) {
    if (numMelloYello > 0) {
      sendBack = "Available";
      numMelloYello = numMelloYello - 1;
      drinkOrder = "Mello Yello";
    }
  } else if (compare.equals("Check Root Beer")) {
    if (numRootBeer > 0) {
      sendBack = "Available"; 
      numRootBeer = numRootBeer - 1;
      drinkOrder = "Root Beer";
    }
  } else if (compare.equals("Check Fanta")) {
    if (numFanta > 0) {
      sendBack = "Available";
      numFanta = numFanta - 1;
      drinkOrder = "Fanta";
    }
  } else if (compare.equals("Check Sprite")) {
    if (numSprite > 0) {
      sendBack = "Available";
      numSprite = numSprite - 1;
      drinkOrder = "Sprite";
    }
  }

  // Send the response to the client
  client.print(sendBack);                 //client.print(s);
  
  // Wait for GPS Coordinates if drink is available
  if (sendBack.equals("Available")) {

    while(!client.available()){
      delay(1);
    }

		String newreq = matchRequest(client);

    // Send response to client
    client.print("Delivering Order");
    String sendOrder = newreq + "," + drinkOrder + ",\r";
    char sendData[35];
    sendOrder.toCharArray(sendData, 35);
    Serial.write(sendData);
  }

}
