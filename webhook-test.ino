#include "Arduino.h"
#include "sha256.h"
#include "Base64.h"
#include "secret.h"

char orderId[256] = "";
int state = 0;

void getProductId(const char *event, const char *data) {
  std::string temp = (std::string)xmlTakeParam(data, "ParentASIN");
  if(strcmp(temp.c_str(), "") != 0) {
    Serial.printlnf("Application>\tProductId Found=%s", temp.c_str());
    Particle.publish("zinc-getdetails", temp.c_str(), PRIVATE);
  }
}

String xmlTakeParam(String inStr,String needParam) {
  if(inStr.indexOf("<"+needParam+">")>0){
     int CountChar=needParam.length();
     int indexStart=inStr.indexOf("<"+needParam+">");
     int indexStop= inStr.indexOf("</"+needParam+">");
     return inStr.substring(indexStart+CountChar+2, indexStop);
  }
  return "";
}

void getOrderId(const char *event, const char *data) {
  // Debug
  //Serial.printlnf("Order ID Received=%s", data);

  strcpy(orderId, data);
  state = 2;
}

void getProductDetails(const char *event, const char *data) {
  // Process Data
  char *title;
  char *description;
  char *productId;
  char *temp;
  temp =         strdupa (data);
  title =        strsep(&temp, "|");
  description =  strsep(&temp, "|");
  productId =   strsep(&temp, "|");

  Serial.printlnf("Application>\tProductID=%s Title=%s Description=%s", productId, title, description);
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

void requestAmazonSearch(std::string keywords, std::string searchIndex) {
  delay(10000);
  Serial.printlnf("Application>\tRequesting an Amazon search for %s in %s!", keywords, searchIndex);
  // Header, Timestamps and query
  std::string header = "GET\nwebservices.amazon.co.uk\n/onca/xml\n";
  std::string timestamp = (std::string)Time.format("%Y-%m-%dT%H%%3A%M%%3A%SZ"); // Needs to be as %3a for signing
  std::string timestampURL = (std::string)Time.format("%Y-%m-%dT%H:%M:%SZ");    // Send with : to Webhook
  // Changes to the query string below MUST be replicated in the Webhook
  // Query MUST be in Alphabetical Order
  std::string query =
    AWSSKEY + "&Keywords=" + keywords +
    "&MaximumPrice=600&Operation=ItemSearch&ResponseGroup=ItemAttributes&SearchIndex=" +
    searchIndex + "&Service=AWSECommerceService&Sort=price&Timestamp=" +
    timestamp;

  // String to Sign
  std::string tosign = header + query;
  /*Serial.print("tosign result: ");
  Serial.println(tosign.c_str());*/

  // SHA256 with HMAC
  Sha256.initHmac((uint8_t*)AWSSECRET.c_str(), AWSSECRET.length());
  Sha256.print(tosign.c_str());

  // Get SHA256 result
  char* sign = (char*) Sha256.resultHmac();

  // BASE64 Encode
  int signLen = 32;
  int encodedSignLen = base64_enc_len(signLen);
  char encodedSign[encodedSignLen];
  base64_encode(encodedSign, sign, signLen);
  /*Serial.print("sign   result: ");
  Serial.println(encodedSign);*/

  // Data to pulish
  char data[256];
  snprintf(data, sizeof(data),
    "{\"timestamp\":\"%s\","
    " \"signature\":\"%s\","
    " \"keywords\":\"%s\","
    " \"searchIndex\":\"%s\"}",
    timestampURL.c_str(),
    encodedSign,
    keywords.c_str(),
    searchIndex.c_str());
  Particle.publish("amazon-search", data, PRIVATE);
}

void setup() {
  Serial.begin(9600);
  // Subscribe to the integration response event
  Particle.subscribe("hook-response/zinc-purchase", getOrderId, MY_DEVICES);
  Particle.subscribe("hook-response/zinc-checkorder", getOrderStatus, MY_DEVICES);
  Particle.subscribe("hook-response/zinc-getdetails", getProductDetails, MY_DEVICES);
  Particle.subscribe("hook-response/amazon-search", getProductId, MY_DEVICES);

  requestAmazonSearch("iPhone", "Electronics");

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
    // Get Amazon Products
    case 50 :
      break;
  }
  delay(5000);
}
