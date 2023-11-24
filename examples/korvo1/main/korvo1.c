/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <sdkconfig.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include <dirent.h>

/* Globals */
static const char *TAG = "example";

static QueueHandle_t q;

static int intensity = 0;

static led_indicator_handle_t leds[1];


static void button_led_task(void *pvParameters)
{
    while (1) {
        int btn_index = 0;
        if (xQueueReceive(q, &btn_index, portMAX_DELAY) == pdTRUE) {
            switch (btn_index) {
            case BSP_BUTTON_VOLUP:
                intensity = intensity + 10;
                led_indicator_stop(leds[0], BSP_LED_ON);
                for (int i = 0; i < BSP_LED_NUM; i++) {
                    led_indicator_set_rgb(leds[0], SET_IRGB(i, intensity, intensity, intensity));
                }
                led_indicator_start(leds[0], BSP_LED_ON);
                break;
            case BSP_BUTTON_VOLDOWN:
                intensity = intensity - 10;
                led_indicator_stop(leds[0], BSP_LED_ON);
                for (int i = 0; i < BSP_LED_NUM; i++) {
                    led_indicator_set_rgb(leds[0], SET_IRGB(i, intensity, intensity, intensity));
                }
                led_indicator_start(leds[0], BSP_LED_ON);
                break;
            default:
                ESP_LOGI(TAG, "other button pressed");
            }
        }
    }
}

static void btn_handler(void *button_handle, void *usr_data)
{
    ESP_LOGI(TAG, "BUTTON PRESSED %d", (int) usr_data);
    int button_pressed = (int) usr_data;
    xQueueSend(q, &button_pressed, 0);
}

void app_main(void)
{
    /* Init leds */
    ESP_ERROR_CHECK(bsp_led_indicator_create(leds, NULL, 1));

    ESP_LOGI(TAG, "BSP_LED_NUM: %d", BSP_LED_NUM);

    /* Create FreeRTOS tasks and queues */
    q = xQueueCreate(10, sizeof(int));
    assert (q != NULL);

    BaseType_t ret = xTaskCreate(button_led_task, "button_led_task", 4096, NULL, 6, NULL);
    assert(ret == pdPASS);

    /* Init buttons */
    button_handle_t btns[BSP_BUTTON_NUM];
    ESP_ERROR_CHECK(bsp_iot_button_create(btns, NULL, BSP_BUTTON_NUM));
    for (int i = 0; i < BSP_BUTTON_NUM; i++) {
        ESP_ERROR_CHECK(iot_button_register_cb(btns[i], BUTTON_PRESS_DOWN, btn_handler, (void *) i));
    }
}
