#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>

#include <math.h>
#include <stdlib.h>

#include <zephyr/logging/log.h>

#include <zephyr/drivers/led_strip.h>
#include <drivers/ext_power.h>

#include <zmk/activity.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>

#include "rgb_indicator.h"
#include "layers.h"
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/caps_word_state_changed.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>

#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if !DT_HAS_CHOSEN(zmk_underglow)

#error "A zmk,underglow chosen node must be declared"

#endif

#define STRIP_CHOSEN DT_CHOSEN(zmk_underglow)
#define STRIP_NUM_PIXELS DT_PROP(STRIP_CHOSEN, chain_length)

struct rgb_indicator_state {
    bool on;
    bool layer;
    bool caps_word;
    bool split_missing;
    bool usb_ready;
    bool ble_connected;
};

static const struct device *led_strip;

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static struct rgb_indicator_state state;

static void rgb_indicator_update() {
    if ((!state.usb_ready) || (!state.layer && !state.caps_word && !state.split_missing && !state.ble_connected)) {
        rgb_indicator_off();
    } else {
        rgb_indicator_on();
        led_strip_update_rgb(led_strip, pixels, STRIP_NUM_PIXELS);
    }
}

static int rgb_indicator_listener(const zmk_event_t *eh) {
    if (as_zmk_layer_state_changed(eh)) {
        uint8_t layer = zmk_keymap_highest_layer_active();
        switch (layer)
        {
        case CANARY:
            state.layer = false;
            pixels[20] = OFF;
            break;
        default: 
            state.layer = true;
            struct led_rgb color;
            switch (layer) {
            case GAME: case GAMERTY:
                color = WHITE; break;
            case FUN_NUM:
                color = RED; break;
            case SYMBOL:
                color = GREEN; break;
            case NAV_ACNT:
                color = BLUE; break;
            case MOUSE:
                color = PURPLE; break;
            case SCROLL:
                color = ORANGE; break;
            case SYSTEM:
                color = YELLOW; break;
            default:
                return 0;
            }
            pixels[20] = color;
            break;
        }
    } else if (as_zmk_caps_word_state_changed(eh)) {
        state.caps_word = as_zmk_caps_word_state_changed(eh)->state;
        if (state.caps_word) {
            pixels[22] = CYAN;
        } else {
            pixels[22] = OFF;
        }
    } else if (as_zmk_split_peripheral_status_changed(eh)) {
        state.split_missing = !as_zmk_split_peripheral_status_changed(eh)->connected;
        if (state.split_missing) {
            pixels[23] = RED;
        } else {
            pixels[23] = OFF;
        }
    } else if (as_zmk_usb_conn_state_changed(eh)) {
        switch (zmk_usb_get_status()) {
            case USB_DC_CONFIGURED:
            case USB_DC_RESUME:
            case USB_DC_SOF:
                state.usb_ready = true; break;
            default:
                state.usb_ready = false; break;
        }
    } else if (as_zmk_ble_active_profile_changed(eh) || as_zmk_endpoint_changed(eh)) {
        state.ble_connected = zmk_endpoints_selected().transport == ZMK_TRANSPORT_BLE &&
                                zmk_ble_active_profile_is_connected();
        pixels[BLE_0_LED] = OFF; pixels[BLE_1_LED] = OFF; pixels[BLE_2_LED] = OFF;
        pixels[BLE_3_LED] = OFF; pixels[BLE_4_LED] = OFF;
        if (state.ble_connected) {
            int index = zmk_ble_active_profile_index();
            pixels[BLE_LED(index)] = BLUE;
        }
    }

    rgb_indicator_update();
    return 0;
}

ZMK_LISTENER(rgb_indicator, rgb_indicator_listener);
ZMK_SUBSCRIPTION(rgb_indicator, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(rgb_indicator, zmk_caps_word_state_changed);
ZMK_SUBSCRIPTION(rgb_indicator, zmk_split_peripheral_status_changed);
ZMK_SUBSCRIPTION(rgb_indicator, zmk_usb_conn_state_changed);
ZMK_SUBSCRIPTION(rgb_indicator, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(rgb_indicator, zmk_endpoint_changed);


#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER)
static const struct device *const ext_power = DEVICE_DT_GET(DT_INST(0, zmk_ext_power_generic));
#endif

static int rgb_indicator_init(void) {
    led_strip = DEVICE_DT_GET(STRIP_CHOSEN);

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER)
    if (!device_is_ready(ext_power)) {
        LOG_ERR("External power device \"%s\" is not ready", ext_power->name);
        return -ENODEV;
    }
#endif

    state = (struct rgb_indicator_state){
        on : false,
        layer : false,
        caps_word : false,
        split_missing : true,
        usb_ready : false,
        ble_connected : false
    };

    return 0;
}

int rgb_indicator_on() {
    if (!led_strip)
        return -ENODEV;

    if (state.on)
        return 0;

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER)
    if (ext_power != NULL) {
        int rc = ext_power_enable(ext_power);
        if (rc != 0) {
            LOG_ERR("Unable to enable EXT_POWER: %d", rc);
        }
    }
#endif

    state.on = true;

    return 0;
}

int rgb_indicator_off() {
    if (!led_strip)
        return -ENODEV;

    if (!state.on)
        return 0;

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW_EXT_POWER)
    if (ext_power != NULL) {
        int rc = ext_power_disable(ext_power);
        if (rc != 0) {
            LOG_ERR("Unable to disable EXT_POWER: %d", rc);
        }
    }
#endif

    state.on = false;

    return 0;
}

SYS_INIT(rgb_indicator_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
