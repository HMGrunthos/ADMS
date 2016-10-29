char orderId[256] = "";
int state = 0;

void setup() {
  Serial.begin(9600);
  // Subscribe to the integration response event
  Particle.subscribe("hook-response/zinc-purchase", getOrderID, MY_DEVICES);
  Particle.subscribe("hook-response/zinc-checkorder", getOrderStatus, MY_DEVICES);
}

void getOrderID(const char *event, const char *data) {
  // Debug
  //Serial.printlnf("Order ID Received=%s", data);

  strcpy(orderId, data);
  state = 2;
}

void getOrderStatus(const char *event, const char *data) {

  // Process Data
  char *type;
  char *code;
  char *merchantId;
  char *message;
  char *temp;
  temp = strdupa (data);
  type =        strsep(&temp, ",");
  code =        strsep(&temp, ",");
  merchantId =  strsep(&temp, ",");
  message =     strsep(&temp, ",");

  //sscanf(data,"%19[^,],%39[^,],%99[^,],%255[^,]", type, code, merchantId, message);

  // Debug
  //Serial.printlnf("Order Status type=%s code=%s merchantId=%s message=%s", type, code, merchantId, message);

  // Keep Checking until success
  if(strcmp(code,"request_processing") == 0) {
    Serial.printlnf("Application>\tOrder is being processed...");
    state = 2;
  // Error
  } else if (strcmp(merchantId,"") != 0) {
    Serial.printlnf("Application>\tOrder Success! Amazon Order ID=%s", merchantId);
    state = 99;
  } else {
    Serial.printlnf("Application>\tOrder Error! Type=%s Code=%s Message=%s", type, code, message);
    strcpy(orderId, "");
    state = 99;
  }

}

void loop() {

  switch(state) {
    // Make Purchase
    case 0 :
      //purchaseSelectedGoods("B00J06V1Q2"); // RULER (1.99)
      purchaseSelectedGoods("B000NM4OHK"); // RULER ADDON (0.49) - Fails
      break;
    // Wait for Order ID
    case 1 :
      // Do Nothing
      break;
    // Request Order Status
    case 2 :
      Particle.publish("zinc-checkorder", orderId, PRIVATE);
      state = 3;
      break;
    // Wait for Order Status
    case 3 :
      // Do Nothing
      break;
  }
  delay(5000);
}

void purchaseSelectedGoods(const char* productId) {
  // Start Delay
  Serial.println("Application>\tWaiting 30s to purchase");
  delay(10000);
  // Start Delay
  Serial.println("Application>\tWaiting 20s to purchase");
  delay(10000);
  // Start Delay
  Serial.println("Application>\tWaiting 10s to purchase");
  delay(10000);

  // Trigger the purchase
  Serial.println("Application>\tPurchasing...");
  Particle.publish("zinc-purchase", productId, PRIVATE);
  state = 1;
}
