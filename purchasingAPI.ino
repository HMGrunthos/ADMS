#include "Arduino.h"
#include "sha256.h"
#include "Base64.h"
#include "secret.h"

char orderId[256] ="";

void getProductId(const char *event, const char *data) {
  std::string temp = (std::string)xmlTakeParam(data, "ParentASIN");
  if(strcmp(temp.c_str(), "") != 0) {
    Serial.printlnf("Purchase API>\tProductId Found=%s", temp.c_str());
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

  Serial.printlnf("Purchase API>\tProductID=%s Title=%s Description=%s", productId, title, description);
}
int orderState = 2;
//
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
    Serial.printlnf("Purchase API>\tOrder is being processed...");
    orderState = 2;
  // Error
  } else if (strcmp(merchantId,"") != 0) {
    Serial.printlnf("Purchase API>\tOrder Success! Amazon Order ID=%s", merchantId);
    orderState = 0; // Success
  } else {
    Serial.printlnf("Purchase API>\tOrder Error! Type=%s Code=%s Message=%s", type, code, message);
    orderState = 1; // Fail
  }

}



int getOrderState()
{
  if(strcmp(orderId,"")!=0) {
    Particle.publish("zinc-checkorder", orderId, PRIVATE);
    return orderState;
  } else {
    return 2;
  }
}

void purchaseSelectedGoods(const char* productId) {
  // Trigger the purchase
  Serial.println("Purchase API>\tPurchasing...");
  orderState = 2;
  orderId[0] = '\0';
  Particle.publish("zinc-purchase", productId, PRIVATE);
}

void requestAmazonSearch(std::string keywords, std::string searchIndex) {
  delay(10000);
  //Serial.printlnf("Purchase API>\tRequesting an Amazon search for %s in %s!", keywords, searchIndex);
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

void initPurchasingAPI() {
  Particle.subscribe("hook-response/zinc-purchase", getOrderId, MY_DEVICES);
  Particle.subscribe("hook-response/zinc-checkorder", getOrderStatus, MY_DEVICES);
  Particle.subscribe("hook-response/zinc-getdetails", getProductDetails, MY_DEVICES);
  Particle.subscribe("hook-response/amazon-search", getProductId, MY_DEVICES);
}
