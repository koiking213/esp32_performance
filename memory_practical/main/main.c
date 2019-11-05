#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "nvs.h"

#define STORAGE_NAMESPACE "storage"
#define CACHE_SIZE (32*1024)

int global_value;

int64_t sum_from_flash(int32_t len, nvs_handle my_handle)
{
  int64_t prev_time = esp_timer_get_time();
  size_t required_size;
  int sum=0;
  nvs_get_blob(my_handle, "data", NULL, &required_size);
  uint32_t* to = (uint32_t*)malloc(required_size);
  nvs_get_blob(my_handle, "data", to, &required_size);
  for (int i=0; i<len; i++) {
    sum += to[i];
  }
  int64_t time = esp_timer_get_time() - prev_time;
  free(to);
  global_value = sum;
  return time;
}

int64_t writeback_to_flash(int32_t len, nvs_handle my_handle)
{
  int64_t prev_time = esp_timer_get_time();
  size_t required_size;
  nvs_get_blob(my_handle, "data", NULL, &required_size);
  uint32_t* buf = (uint32_t*)malloc(required_size);
  nvs_get_blob(my_handle, "data", buf, &required_size);
  for (int i=0; i<len; i++) {
    buf[i] += 1;
  }
  nvs_set_blob(my_handle, "data", buf, len*sizeof(uint32_t));
  int64_t time = esp_timer_get_time() - prev_time;
  free(buf);
  return time;
}

int64_t sum_from_sram(int32_t len)
{
  int *from = (int*)malloc(sizeof(int)*len);
  for (int i=0; i<len; i++) {
    from[i] = 1;
  }
  int sum=0;
  int64_t prev_time = esp_timer_get_time();
  for (int i=0; i<len; i++) {
    sum += from[i];
  }
  int64_t time = esp_timer_get_time()-prev_time;
  free(from);
  global_value = sum;
  return time;
}

int64_t writeback_to_sram(int32_t len)
{
  int *buf = (int*)malloc(sizeof(int)*len);
  for (int i=0; i<len; i++) {
    buf[i] = 1;
  }
  int64_t prev_time = esp_timer_get_time();
  for (int i=0; i<len; i++) {
    buf[i] += 1;
  }
  int64_t time = esp_timer_get_time() - prev_time;
  free(buf);
  return time;
}

int64_t sum_from_spi_sram(int32_t len, int *from)
{
  int64_t prev_time = esp_timer_get_time();
  int sum=0;
  for (int i=0; i<len; i++) {
    sum += from[i];
  }
  int64_t time = esp_timer_get_time() - prev_time;
  global_value = sum;
  return time;
}

int64_t writeback_to_spi_sram(int32_t len, int *buf)
{
  int64_t prev_time = esp_timer_get_time();
  for (int i=0; i<len; i++) {
    buf[i] += 1;
  }
  return esp_timer_get_time() - prev_time;
}

void clear_cache(int *buf, size_t len) {
  volatile int sum;
  for (int i=0; i<len; i++) {
    sum += buf[i];
  }
}

void app_main()
{

  nvs_flash_init();
  nvs_flash_erase();

  size_t len = 256;

  // init flash
  nvs_handle my_handle;
  nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
  uint32_t* from = (uint32_t*)malloc(len*sizeof(uint32_t));
  for (int i=0; i<len; i++) {
    from[i] = 1;
  }
  nvs_set_blob(my_handle, "data", from, len*sizeof(uint32_t));
  free(from);

  // init spi_sram
  int *spi_sram_buf = (int*)heap_caps_malloc(sizeof(int)*len, MALLOC_CAP_SPIRAM);
  for (int i=0; i<len; i++) {
    spi_sram_buf[i] = 1;
  }

  int *buf = (int*)heap_caps_malloc(CACHE_SIZE, MALLOC_CAP_SPIRAM);
  for (int i=0; i<CACHE_SIZE/4; i++) {
    buf[i] = 0;
  }
  
  // start measurement 
  double time_sum = 0;
  int64_t time;
  puts("sum_from_sram:");
  for (int i=0; i<10; i++) {
    time = sum_from_sram(len);
    printf("%lld\n", time);
    time_sum += time;
  }
  printf("average: %f\n", time_sum/10);

  time_sum = 0;
  puts("sum_from_spi_sram:");
  for (int i=0; i<10; i++) {
    clear_cache(buf, CACHE_SIZE/4);
    time = sum_from_spi_sram(len, spi_sram_buf);
    printf("%lld\n", time);
    time_sum += time;
  }
  printf("average: %f\n", time_sum/10);

  time_sum = 0;
  puts("sum_from_cache:");
  for (int i=0; i<10; i++) {
    time = sum_from_spi_sram(len, spi_sram_buf);
    printf("%lld\n", time);
    time_sum += time;
  }
  printf("average: %f\n", time_sum/10);

  time_sum = 0;
  puts("sum_from_flash:");
  for (int i=0; i<10; i++) {
    time = sum_from_flash(len, my_handle);
    printf("%lld\n", time);
    time_sum += time;
  }
  printf("average: %f\n", time_sum/10);

  time_sum = 0;
  puts("writeback_to_sram:");
  for (int i=0; i<10; i++) {
    time = writeback_to_sram(len);
    printf("%lld\n", time);
    time_sum += time;
  }
  printf("average: %f\n", time_sum/10);

  time_sum = 0;
  puts("writeback_to_spi_sram:");
  for (int i=0; i<10; i++) {
    clear_cache(buf, CACHE_SIZE/4);
    time = writeback_to_spi_sram(len, spi_sram_buf);
    printf("%lld\n", time);
    time_sum += time;
  }
  printf("average: %f\n", time_sum/10);

  time_sum = 0;
  puts("writeback_to_cache:");
  for (int i=0; i<10; i++) {
    time = writeback_to_spi_sram(len, spi_sram_buf);
    printf("%lld\n", time);
    time_sum += time;
  }
  printf("average: %f\n", time_sum/10);

  time_sum = 0;
  puts("writeback_to_flash:");
  for (int i=0; i<10; i++) {
    time = writeback_to_flash(len, my_handle);
    printf("%lld\n", time);
    time_sum += time;
  }
  printf("average: %f\n", time_sum/10);


  printf("Done.\n");
  fflush(stdout);
}
