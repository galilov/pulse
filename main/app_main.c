#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "driver/adc.h"

#include "data_handler.h"


//i2s number
#define NUM           (0)
//I2S read buffer length 0.1s
#define READ_LEN      (2 * SAMPLE_RATE/10)
//I2S built-in ADC unit
#define ADC_UNIT              ADC_UNIT_1
//I2S built-in ADC channel
#define ADC_CHANNEL           ADC1_CHANNEL_0


/**
 * @brief I2S ADC/DAC mode init.
 */
void my_i2s_init(void) {
    i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN, //| I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN ,
            .sample_rate =  SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .communication_format = I2S_COMM_FORMAT_I2S_MSB,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .intr_alloc_flags = 0,
            .dma_buf_count = 8,
            .dma_buf_len = 64,
            .use_apll = true,
    };
    //install and start i2s driver
    i2s_driver_install(NUM, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT, ADC_CHANNEL);
}


/**
 * @brief debug buffer data
 */
void disp_buf(const ProcessResult* processResult) {
#if BUF_DEBUG
    printf("======\n");
    for (size_t i = 0; i < processResult->dataSize; i++) {
        printf("%f\n", (processResult->data[i]) * 3.3f / 4095.0f);
    }
    printf("======\n");
#endif
}


void task_i2s_adc_dac(void *arg) {
    const int i2s_read_len = READ_LEN;
    size_t bytes_read = 0;

    void *i2s_read_buff = calloc(i2s_read_len, sizeof(char));
    i2s_adc_enable(NUM);
    i2s_read(NUM, (void *) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    i2s_adc_disable(NUM);

    if (bytes_read == i2s_read_len) {
        ProcessResult processResult = process_data(i2s_read_buff, bytes_read, true);
        disp_buf(&processResult);
        printf("isError = %s\n", processResult.isError ? "ERROR" : "OK");
        printf("min = %d\n", processResult.min);
        printf("max = %d\n", processResult.max);
        printf("coeffPulsation = %f\n", processResult.coeffPulsation);
        if (processResult.isError) {
            i2s_driver_uninstall(NUM);
            my_i2s_init();
        }
    }

    free(i2s_read_buff);
    i2s_read_buff = NULL;


    vTaskDelete(NULL);
}


esp_err_t app_main(void) {
    my_i2s_init();
    esp_log_level_set("I2S", ESP_LOG_INFO);

    TickType_t delay = 2000 / portTICK_PERIOD_MS;
    for(;;) {
        xTaskCreate(task_i2s_adc_dac, "task_i2s_adc_dac", 1024 * 2, NULL, 5, NULL);
        vTaskDelay(delay);
    }

    // xTaskCreate(adc_read_task, "ADC read task", 2048, NULL, 5, NULL);
    return ESP_OK;
}


