#pragma once

#define OFF     (struct led_rgb) { .r =  0, .g =  0, .b =  0 }
#define RED     (struct led_rgb) { .r = 10, .g =  0, .b =  0 }
#define GREEN   (struct led_rgb) { .r =  0, .g = 10, .b =  0 }
#define BLUE    (struct led_rgb) { .r =  0, .g =  0, .b = 10 }
#define YELLOW  (struct led_rgb) { .r = 10, .g = 10, .b =  0 }
#define PURPLE  (struct led_rgb) { .r = 15, .g =  0, .b =  5 }
#define ORANGE  (struct led_rgb) { .r = 15, .g =  5, .b =  0 }
#define CYAN    (struct led_rgb) { .r =  0, .g = 10, .b = 10 }

#define BLE_1_LED 19
#define BLE_2_LED 12
#define BLE_3_LED 11
#define BLE_LED(n) (((n)==0) ? BLE_1_LED : ((n)==1) ? BLE_2_LED : BLE_3_LED)

int rgb_indicator_on();
int rgb_indicator_off();
