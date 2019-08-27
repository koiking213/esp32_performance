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
#define SPIFFS_ADDR2 0x320000

uint32_t global_variable;

void spi_sram_to_spi_sram()
{
  float *from, *to;
  from = heap_caps_malloc(4*4*1024, MALLOC_CAP_SPIRAM);
  to = heap_caps_malloc(4*4*1024, MALLOC_CAP_SPIRAM);
  int64_t prev_time = esp_timer_get_time();
  memcpy(to, from, 4*4*1024);
  printf("1st 16KB memcpy from spi_sram to spi_sram: %lld[us]\n", esp_timer_get_time()-prev_time);
  prev_time = esp_timer_get_time();
  memcpy(to, from, 4*4*1024);
  printf("2nd 16KB memcpy from spi_sram to spi_sram: %lld[us]\n", esp_timer_get_time()-prev_time);
  heap_caps_free(from);
  heap_caps_free(to);
}

void contiguous_store_to_spi_sram()
{
  int32_t *array;
  array = heap_caps_malloc(4*4*1024, MALLOC_CAP_SPIRAM);
  int64_t prev_time = esp_timer_get_time();
  int64_t elapsed;
  memset(array, 1, 4*4*1024);
  elapsed = esp_timer_get_time() - prev_time;
  printf("1st 16KB memcpy from spi_sram to spi_sram: %lld[us], %g[cycle per loop]\n", elapsed, (float)elapsed/(1024*4)*CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);
  prev_time = esp_timer_get_time();
  memset(array, 4, 4*4*1024); // memsetはループ1周に1.25サイクルかかるはず
  elapsed = esp_timer_get_time() - prev_time;
  printf("2nd 16KB memcpy from spi_sram to spi_sram: %lld[us], %g[cycle per loop]\n", elapsed, (float)elapsed/(1024*4)*CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);
  heap_caps_free(array);
}

void same_addr_store_for_spi_sram()
{
  volatile int32_t *array;
  array = heap_caps_malloc(4, MALLOC_CAP_SPIRAM);
  int64_t prev_time = esp_timer_get_time();
  for (int32_t i=0; i<n_iter; i++) {
    array[0] = 0;
  }
  printf("store to spi_sram: %g[cycle]\n",  (esp_timer_get_time() - prev_time) / 100000.0);
  heap_caps_free(array);
}

void flash_to_spi_sram()
{
  float *to;
  to = heap_caps_malloc(4*4*1024, MALLOC_CAP_SPIRAM);
  int64_t prev_time = esp_timer_get_time();
  spi_flash_read(SPIFFS_ADDR, to, 4*4*1024);
  printf("1st 16KB memcpy from flash to spi_sram: %lld[us]\n", esp_timer_get_time()-prev_time);
  prev_time = esp_timer_get_time();
  spi_flash_read(SPIFFS_ADDR, to, 4*4*1024);
  printf("2nd 16KB memcpy from flash to spi_sram: %lld[us]\n", esp_timer_get_time()-prev_time);
  heap_caps_free(to);
}

void flash_to_sram(float* to, int size)
{
  int64_t prev_time = esp_timer_get_time();
  spi_flash_read(SPIFFS_ADDR, to, size);
  printf("1st %dB memcpy from flash to sram: %lld[us]\n", size, esp_timer_get_time()-prev_time);
  prev_time = esp_timer_get_time();
  spi_flash_read(SPIFFS_ADDR2, to, size);
  printf("2nd %dB memcpy from flash to sram: %lld[us]\n", size, esp_timer_get_time()-prev_time);
}

void sram_to_flash(float* from, int size)
{
  int64_t prev_time = esp_timer_get_time();
  spi_flash_write(SPIFFS_ADDR, from, size);
  printf("1st %dB memcpy to flash from sram: %lld[us]\n", size, esp_timer_get_time()-prev_time);
  prev_time = esp_timer_get_time();
  spi_flash_write(SPIFFS_ADDR2, from, size);
  printf("2nd %dB memcpy to flash from sram: %lld[us]\n", size, esp_timer_get_time()-prev_time);
}


void app_main()
{

  spi_sram_to_spi_sram();
  contiguous_store_to_spi_sram();
  flash_to_spi_sram();
  same_addr_store_for_spi_sram();
  
  printf("Done.\n");
  fflush(stdout);
}
