#include <Arduino_FreeRTOS.h>
#include <event_groups.h>

const int carRed = 8;
const int carYellow = 9;
const int carBlue = 10;
const int pedRed = 6;
const int pedBlue = 7;
const int btnPin = 2;

const int BUTTON_FLAG = (1 << 0);

TaskHandle_t ButtonTask_Handler;
TaskHandle_t SignalTask_Handler;
TaskHandle_t SerialTask_Handler;
EventGroupHandle_t eventGroup;

int pedBlueTime = 5;
volatile char nightMode = 'd';

void setup() {
  pinMode(carRed, OUTPUT);
  pinMode(carYellow, OUTPUT);
  pinMode(carBlue, OUTPUT);
  pinMode(pedRed, OUTPUT);
  pinMode(pedBlue, OUTPUT);
  pinMode(btnPin, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("START");

  eventGroup = xEventGroupCreate();

  xTaskCreate(ButtonTask, "Button", 128, NULL, 2, &ButtonTask_Handler);
  xTaskCreate(SignalTask, "Signal", 256, NULL, 1, &SignalTask_Handler);
  xTaskCreate(SerialTask, "SerialTask", 128, NULL, 1, &SerialTask_Handler);

  vTaskStartScheduler();
}

void ButtonTask(void *pvParameters) {
  int prevState = HIGH;
  while (1) {
    int state = digitalRead(btnPin);
    if (prevState == HIGH && state == LOW) {
      Serial.println("歩行者ボタン検出");
      xEventGroupSetBits(eventGroup, BUTTON_FLAG);
    }
    prevState = state;
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
void SignalTask(void *pvParameters) {
  while (1) {
    if (nightMode == 'd') {
      Serial.println("【昼間モード】");
      Serial.println("車道: 青");
      digitalWrite(carBlue, HIGH);
      digitalWrite(carYellow, LOW);
      digitalWrite(carRed, LOW);
      digitalWrite(pedRed, HIGH);
      digitalWrite(pedBlue, LOW);
      EventBits_t bits;
      while (1) {
        bits = xEventGroupWaitBits(eventGroup, BUTTON_FLAG, pdTRUE, pdFALSE, 100 / portTICK_PERIOD_MS);
        if (bits & BUTTON_FLAG) break;
        if (nightMode == 'n') break;
      }
      if (nightMode == 'n') continue;
      for (int i = 0; i < 50; i++) {  // 100ms × 50 = 5秒
        if (nightMode == 'n') break;
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      if (nightMode == 'n') continue;

      digitalWrite(carBlue, LOW);
      digitalWrite(carYellow, HIGH);
      Serial.println("車道: 黄");
      vTaskDelay(1000 / portTICK_PERIOD_MS);

      digitalWrite(carYellow, LOW);
      digitalWrite(carRed, HIGH);
      Serial.println("車道: 赤");

      vTaskDelay(1000 / portTICK_PERIOD_MS);
      digitalWrite(pedRed, LOW);
      digitalWrite(pedBlue, HIGH);
      Serial.println("歩道: 青");
      for (int i = 0; i < pedBlueTime * 10; i++) {
        if (nightMode == 'n') break;
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      if (nightMode == 'n') continue;

      Serial.println("歩道: 青点滅");
      for (int i = 0; i < 6; i++) {
        digitalWrite(pedBlue, LOW);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        digitalWrite(pedBlue, HIGH);
        vTaskDelay(250 / portTICK_PERIOD_MS);
      }

      digitalWrite(pedBlue, LOW);
      digitalWrite(pedRed, HIGH);
      Serial.println("歩道: 赤");
      vTaskDelay(1000 / portTICK_PERIOD_MS);

      digitalWrite(carRed, LOW);
      digitalWrite(carBlue, HIGH);
    }

    else if (nightMode == 'n') {
      Serial.println("【夜間モード】赤・黄点滅");
      digitalWrite(carBlue, LOW);
      digitalWrite(carRed, LOW);
      digitalWrite(pedBlue, LOW);

      while (nightMode == 'n') {
        EventBits_t bits = xEventGroupWaitBits(eventGroup, BUTTON_FLAG, pdTRUE, pdFALSE, 0);
        if (bits & BUTTON_FLAG) {
          digitalWrite(carYellow, HIGH);
          digitalWrite(pedRed, HIGH);
          vTaskDelay(1000 / portTICK_PERIOD_MS);

          digitalWrite(carYellow, LOW);
          digitalWrite(carRed, HIGH);
          vTaskDelay(1000 / portTICK_PERIOD_MS);

          digitalWrite(pedRed, LOW);
          digitalWrite(pedBlue, HIGH);
          Serial.println("歩道: 青");
          for (int i = 0; i < pedBlueTime * 10; i++) {
            if (nightMode == 'd') break;
            vTaskDelay(100 / portTICK_PERIOD_MS);
          }

          Serial.println("歩道: 青点滅");
          for (int i = 0; i < 6; i++) {
            digitalWrite(pedBlue, LOW);
            vTaskDelay(250 / portTICK_PERIOD_MS);
            digitalWrite(pedBlue, HIGH);
            vTaskDelay(250 / portTICK_PERIOD_MS);
          }

          digitalWrite(pedBlue, LOW);
          digitalWrite(pedRed, HIGH);
          Serial.println("歩道: 赤");
          vTaskDelay(500 / portTICK_PERIOD_MS);
          Serial.println("赤・黄点滅");

          digitalWrite(carRed, LOW);
          digitalWrite(carYellow, HIGH);
          vTaskDelay(250 / portTICK_PERIOD_MS);
          continue;
        }
        digitalWrite(carYellow, HIGH);
        digitalWrite(pedRed, HIGH);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        if (nightMode == 'd') break;
        digitalWrite(carYellow, LOW);
        digitalWrite(pedRed, LOW);
        vTaskDelay(250 / portTICK_PERIOD_MS);
      }
      digitalWrite(carYellow, LOW);
      digitalWrite(pedRed, HIGH);
      digitalWrite(pedBlue, LOW);
      Serial.println("夜間モード終了 → 昼間へ切替");
    }
  }
}
void SerialTask(void *pvParameters) {
  while (1) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (isdigit(c)) {
        int time = c - '0';
        while (Serial.available() > 0) {
          char n = Serial.read();
          if (isdigit(n)) time = time * 10 + (n - '0');
        }
        if (time > 0 && time <= 30) {
          pedBlueTime = time;
          Serial.print(pedBlueTime);
          Serial.println(" 秒に設定しました。");
        } else {
          Serial.println("無効な値(1～30秒の範囲で指定)");
        }
      } else if (c == 'd' || c == 'n') {
        nightMode = c;
        Serial.print("モード切替: ");
        if (c == 'd') Serial.println("昼間");
        else Serial.println("夜間");
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
void loop() {}