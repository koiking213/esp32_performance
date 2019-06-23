/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#define n_iter (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 100000L)
#define N 100
#define SIZE 100
#define SPIFFS_ADDR 0x310000

volatile uint32_t global_variable=10;

float div_float(float a0, float a1) //divfsという関数呼び出しになる
{
  int64_t prev_time = esp_timer_get_time();
  for (int i = 0; i < n_iter; i++) {
    a0 /= a1;
  }
  printf("div_float: cycle = %g\n", ((esp_timer_get_time()-prev_time) / 100000.0));
  return a0;
}

uint32_t quou(uint32_t a0, uint32_t a1)
{
  int64_t prev_time = esp_timer_get_time();
  for (int i = 0; i < n_iter; i++) {
    a0 /= a1;
  }
  printf("quou: cycle = %g\n", ((esp_timer_get_time()-prev_time) / 100000.0));
  return a0;
}

float mul_s(float a0, float a1)
{
  int64_t prev_time = esp_timer_get_time();
  for (int i = 0; i < n_iter; i++) {
    a0 *= a1;
  }
  printf("mul.s: cycle = %g\n", ((esp_timer_get_time()-prev_time) / 100000.0));
  return a0;
}

uint32_t mul(uint32_t a0, uint32_t a1)
{
  int64_t prev_time = esp_timer_get_time();
  for (int i = 0; i < n_iter; i++) {
    a0 *= a1;
  }
  printf("mull: cycle = %g\n", ((esp_timer_get_time()-prev_time) / 100000.0));
  return a0;
}

uint32_t load(uint32_t b)
{
  int32_t a;
  int64_t prev_time = esp_timer_get_time();
  for (int i = 0; i < n_iter; i++) {
    a = global_variable;
  }
         
  printf("load: cycle = %g\n", ((esp_timer_get_time()-prev_time) / 100000.0));
  return rand()/a;
}

uint32_t store(uint32_t b)
{
  int64_t prev_time = esp_timer_get_time();
  for (int i = 0; i < n_iter; i++) {
    global_variable = 0;
  }
         
  printf("store: cycle = %g\n", ((esp_timer_get_time()-prev_time) / 100000.0));
  return rand();
}

void app_main()
{
  uint32_t i0 = rand();
  uint32_t i1 = rand();
  float f0 = rand() / RAND_MAX;
  float f1 = rand() / RAND_MAX;

  quou(i0, i1);
  div_float(f0, f1);
  mul(i0, i1);
  mul_s(f0, f1);
  load(i0);
  store(i0);

  printf("Done.\n");
  fflush(stdout);
}
