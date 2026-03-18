#include <Arduino.h>
#include <Arduino_RouterBridge.h>

void printSlow(const char *s) {
  for (int i = 0; s[i] != '\0'; i++) {
    Monitor.print(s[i]);
    delay(20);  // volontairement long
  }
  Monitor.println();
}

void callbackTest(int x) {
  Monitor.println(">>> CALLBACK EXECUTED <<<");
}

void setup() {
  Bridge.begin();
  Monitor.begin();

  Bridge.provide_safe("test", callbackTest);
  //Bridge.provide("test", callbackTest);
}

void loop() {
  Monitor.println("[LOOP START]");

  printSlow("Forum");

  delay(100);

  Monitor.println("[LOOP END]");
}
