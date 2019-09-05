#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "freertos/semphr.h"

SemaphoreHandle_t barrierSemaphore;
struct param_t {
	float *a;
	float *b;
	float *c;
    int p;
    int q;
    int r;
};

inline void dot_product_func(float *a, float *b, float *c, const int p, const int q, const int r) {
  for (int i=0; i<p; i++) {
    for (int j=0; j<r; j++) {
      c[i*r+j] = 0;
      for (int k=0; k<q; k++) {
        c[i*r+j] += a[i*q+k]*b[k*r+j];
      }
    }
  }
}

void dot_product(float *a, float *b, float *c, const int p, const int q, const int r) {
  for (int i=0; i<p; i++) {
    for (int j=0; j<r; j++) {
      int k=0;
      float *pa = &(a[i*q]);
      float *pb = &(b[j]);
      float a_data=0, b_data=0;
      c[i*r+j] = 0;
      __asm__ volatile("mov %0, %7\n\t"
                       "loop %0, .end_loop\n\t"
                       "lsi %1, %4, 0\n\t"
                       "lsi %2, %5, 0\n\t"
                       "madd.s %3, %1, %2\n\t"
                       "addi %4, %4, 4\n\t"
                       "add %5, %5, %6\n\t"
                       ".end_loop:"
                       :"+r"(k), "+f"(a_data), "+f"(b_data),
                        "+f"(c[i*r+j]), "+r"(pa), "+r"(pb)
                       :"r"(r*4), "r"(q)
                      );
    }
  }
}

void task(void *parameters)
{
  struct param_t *param = (struct param_t *)parameters;
  int64_t prev_time = esp_timer_get_time();
  dot_product(param->a, param->b, param->c, param->p, param->q, param->r);
  printf("time %d: %lld\n",(int)parameters, (esp_timer_get_time()-prev_time));
  xSemaphoreGive(barrierSemaphore);
  vTaskDelete(NULL);
}

void app_main(void) {
  const int p=80;
  const int q=80;
  const int r=80;
  float *a = (float*)malloc(p*q*sizeof(float));
  float *b = (float*)malloc(q*r*sizeof(float));
  float *c = (float*)malloc(p*r*sizeof(float));

  struct param_t param1 = {a, b, c, p/2, q, r};
  struct param_t param2 = {a+(p/2)*q, b, c+(p/2)*r, p/2, q, r};

  for (int i=0; i<p*q; i++) {
    a[i] =  i / p*q;
  }
  for (int i=0; i<q*r; i++) {
    b[i] = i / q*r;
  }
  int64_t prev_time = esp_timer_get_time();
  barrierSemaphore = xSemaphoreCreateCounting( 2, 0 );
  xTaskCreatePinnedToCore(
    task,           /* pvTaskCode: 実行する関数のポインタ */
    "task0",        /* pcName: タスクの名前(デバッグ用) */
    5000,           /* usStackDepth: スタックサイズ(バイト数) */
    (void*)&param1, /* pvParameters: タスクに渡す引数 */
    1,              /* uxPriority: タスクの優先度 */
    NULL,           /* pvCreatedTask: 作成されたタスクのハンドルが設定される */
    0);             /* xCoreID: タスクを割り当てるCPUコア */

  xTaskCreatePinnedToCore(
    task,
    "task1",
    5000,
    (void*)&param2,
    1,
    NULL,
    1);
  xSemaphoreTake(barrierSemaphore, portMAX_DELAY);
  xSemaphoreTake(barrierSemaphore, portMAX_DELAY);
  //dot_product_func(a, b, c, p, q, r);
  printf("whole time: %lld\n", (esp_timer_get_time()-prev_time));
#if DEBUG
  for (int i=0; i<p; i++) {
    for (int j=0; j<r; j++) {
      printf("%f\n", c1[i*r+j]);
    }
  }
#endif
  free(a);
  free(b);
  free(c);
}
