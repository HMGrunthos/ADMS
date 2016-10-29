int i = 0;

void setup() {
  Serial.begin(9600);
  // Subscribe to the integration response event
  Particle.subscribe("hook-response/zinc-purchase", myHandler, MY_DEVICES);
}

void myHandler(const char *event, const char *data) {
  Serial.print("Zinc CALLBACK");
  i++;
  Serial.print(i);
  Serial.print(event);
  Serial.print(", data: ");
  if (data){
      Serial.println(data);
  }else{
      Serial.println("NULL");
  }
}

void loop() {
  // Start Delay
  Serial.println("Application>\tWaiting 10s");
  delay(10000);

  // Trigger the purchase
  Serial.println("Application>\tPurchase Triggered");
  Particle.publish("zinc-purchase", "B012D6UYTA", PRIVATE);

  // Wait 60 seconds
  Serial.println("Application>\tWaiting 60s");
  delay(60000);
}
