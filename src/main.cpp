#include <Arduino.h>
#include <Bounce2.h>
#include <ESP32Servo.h>
#include "data.h"

#define led1 27
#define led2 26
#define led3 25
#define led4 33
#define button1 32
#define button2 12
#define button3 21
#define button4 23
#define button5 14
#define LDR 34
// call_func
void servo_door_open(void *param);
void pin_c(void *param);
void indoor(void *param);
void obstacle(void *param);
void servo_door_close(void *param);
void time_count(void *param);
void GET_data(void *param);
void PUT_data(void *param);
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();
Bounce debouncer4 = Bounce();
Bounce debouncer5 = Bounce();
// global var
// pin
int key[3];
// int pin[3] = {1, 2, 3};
bool is_pass;
int index_pin = 0;
int count = 0;
int demo_count = 3;
// servo
Servo myservo;
bool goto_door = false;
// is_open = false;
int door_position = 45;
// buzzer
int buzzerPin = 4;
bool is_state_change = false;
// Avoidance
int laser = 22;
int threshold = 2900;
bool is_ob = false;
unsigned long time_now = 0;
// timer
bool is_time = 0;
int count_time = 0;
int demo_timer = 5;
// data
bool is_post_data = false;
bool is_get_data = false;
// task
TaskHandle_t TaskGet;

void setup()
{
  Serial.begin(115200);
  // LED
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  // BUTTON
  debouncer1.attach(button1, INPUT_PULLUP);
  debouncer2.attach(button2, INPUT_PULLUP);
  debouncer3.attach(button3, INPUT_PULLUP);
  debouncer4.attach(button4, INPUT_PULLUP);
  debouncer5.attach(button5, INPUT_PULLUP);
  debouncer1.interval(25);
  debouncer2.interval(25);
  debouncer3.interval(25);
  debouncer4.interval(25);
  debouncer5.interval(25);
  // servo
  myservo.attach(13);
  myservo.write(45);
  // buzzer
  pinMode(buzzerPin, OUTPUT);
  // Avoidance
  pinMode(laser, OUTPUT);
  Connect_Wifi();
  GET_home();
  // mutiTask
  xTaskCreatePinnedToCore(pin_c, "buttons", 1024, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(servo_door_open, "open", 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(servo_door_close, "close", 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(time_count, "timer", 1024, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(obstacle, "LDR&laser", 2048, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(GET_data, "GET_data", 10240, NULL, 2, &TaskGet, 0);
  xTaskCreatePinnedToCore(PUT_data, "PUT_data", 10240, NULL, 1, NULL, 0);
}

void loop() {}

void GET_data(void *param)
{
  while (true)
  {
    GET_home();
    if (door_position == 135 || door_position == 45)
      is_open = temp_is_open;
    Serial.println();
    if (is_open != is_state_change)
    {
      goto_door = 1;
      is_state_change = is_open;
    }
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void PUT_data(void *param)
{
  while (true)
  {
    if (is_post_data)
    {
      Serial.println("PUT");
      vTaskSuspend(TaskGet);
      PUT_home();
      is_post_data = 0;
      time_now = millis();
      while(millis() < time_now + 1000){
      //wait approx. [period] ms
      }
      vTaskResume(TaskGet);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void time_count(void *param)
{
  while (true)
  {
    if (is_time)
    {
      if (!is_open)
      {
        is_time = 0;
        count_time = 0;
      }
      Serial.println(count_time);
      count_time++;
      if (count_time >= delay_val + 1)
      {
        is_time = 0;
        goto_door = 1;
        is_open = 0;
        count_time = 0;
        is_post_data = 1;
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void obstacle(void *param)
{
  while (true)
  {
    if (is_ob)
    {
      digitalWrite(laser, HIGH);
      delay(100);
      int light_ldr = analogRead(LDR);
      Serial.println(light_ldr);
      while (light_ldr < threshold)
      {
        Serial.println("OBSTACLE");
        Serial.println(light_ldr);
        light_ldr = analogRead(LDR);
        time_now = millis();
        while(millis() < time_now + 700){
        //wait approx. [period] ms
        }
      }
      is_ob = 0;
      digitalWrite(laser, LOW);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void servo_door_open(void *param)
{
  while (true)
  {
    if (goto_door)
    {
      if (is_open)
      {
        Serial.println("iam open");
        if (door_position < 135)
        {
          myservo.write(door_position);
          door_position++;
        }
        else if (door_position == 135)
        {
          goto_door = 0;
          is_time = 1;
        }
      }
    }
    vTaskDelay(30 / portTICK_PERIOD_MS);
  }
}

void servo_door_close(void *param)
{
  while (true)
  {
    if (goto_door)
    {
      if (!is_open)
      {
        Serial.println("iam close");
        is_ob = 1;
        if (door_position > 45)
        {
          is_ob = 1;
          myservo.write(door_position);
          door_position--;
        }
        else if (door_position == 45)
        {
          goto_door = 0;
        }
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void pin_c(void *param)
{
  while (true)
  {
    debouncer1.update();
    debouncer2.update();
    debouncer3.update();
    debouncer4.update();
    debouncer5.update();
    if (debouncer1.fell())
    {
      key[index_pin] = 1;
      index_pin++;
    }
    else if (debouncer2.fell())
    {
      key[index_pin] = 2;
      index_pin++;
    }
    else if (debouncer3.fell())
    {
      key[index_pin] = 3;
      index_pin++;
    }
    else if (debouncer4.fell())
    {
      if (index_pin > 0)
      {
        key[index_pin] = 0;
        index_pin--;
      }
      else if (index_pin == 0)
      {
        key[index_pin] = 0;
      }
    }
    else if (debouncer5.fell())
    {
      Serial.println("Hi");
      goto_door = 1;
      is_open = !is_open;
      is_post_data = 1;
    }
    // เเสดงไฟ
    if (index_pin == 0)
    {
      digitalWrite(led1, 0);
      digitalWrite(led2, 0);
      digitalWrite(led3, 0);
    }
    else if (index_pin == 1)
    {
      digitalWrite(led1, 1);
      digitalWrite(led2, 0);
      digitalWrite(led3, 0);
    }
    else if (index_pin == 2)
    {
      digitalWrite(led1, 1);
      digitalWrite(led2, 1);
      digitalWrite(led3, 0);
    }
    else if (index_pin == 3)
    {
      digitalWrite(led1, 1);
      digitalWrite(led2, 1);
      digitalWrite(led3, 1);
    }
    // ตรวจสอบความถูกต้อง
    if (index_pin == 3)
    {
      index_pin = 0;
      for (int i = 0; i < 3; i++)
      {
        if (pin[i] == key[i])
        {
          is_pass = true;
        }
        else
        {
          is_pass = false;
          break;
        }
      }
      if (is_pass)
      {
        count = 0;
        if (!is_open)
        {
          digitalWrite(led4, 1);
          digitalWrite(buzzerPin, HIGH);
          goto_door = 1;
          is_open = 1;
          is_post_data = 1;
          delay(500);
          digitalWrite(buzzerPin, LOW);
          delay(1000);
          digitalWrite(led4, 0);
        }
        else
        {
          goto_door = 0;
          Serial.println("OPEN?");
        }
      }
      else
      {
        count++;
        Serial.println(count);
        for (int i = 0; i < 3; i++)
        {
          digitalWrite(buzzerPin, HIGH);
          delay(50);
          digitalWrite(buzzerPin, LOW);
          delay(50);
        }
        if (count == 3)
        {
            goto_door = 1;
            is_open = 0;
            digitalWrite(led1, 0);
            digitalWrite(led2, 0);
            digitalWrite(led3, 0);
            for (int i = 0; i < 70; i++)
            {
              digitalWrite(buzzerPin, HIGH);
              digitalWrite(led4, 1);
              delay(50);
              digitalWrite(buzzerPin, LOW);
              digitalWrite(led4, 0);
              delay(50);
            }
            count = 0;

        }
      }
    }
  }
}
