#pragma once

#include "zmk-helpers/helper.h"

#define MACRO_CHOOSER6(_1, _2, _3, _4, _5, _6, FUNC, ...) FUNC
#undef ZMK_COMBO
#define ZMK_COMBO(...) MACRO_CHOOSER6(__VA_ARGS__, ZMK_COMBO_6_ARGS, ZMK_COMBO_5_ARGS, ZMK_COMBO_4_ARGS)(__VA_ARGS__)
#define ZMK_COMBO_6_ARGS(name, combo_bindings, keypos, combo_layers, combo_timeout, prior_idle) \
    / { \
        combos { \
            compatible = "zmk,combos"; \
            combo_ ## name { \
                timeout-ms = <combo_timeout>; \
                bindings = <combo_bindings>; \
                key-positions = <keypos>; \
                layers = <combo_layers>; \
                require-prior-idle-ms = <prior_idle>; \
                COMBO_HOOK \
            }; \
        }; \
    };

#define ZMK_SHIFT_MORPH(NAME, BINDING, SHIFT_BINDING) \
/ { \
  behaviors { \
    NAME: NAME { \
      compatible = "zmk,behavior-mod-morph"; \
      #binding-cells = <0>; \
      bindings = <BINDING>, <SHIFT_BINDING>; \
      mods = <(MOD_LSFT|MOD_RSFT)>; \
    }; \
  }; \
};

#define ZMK_MACRO_MORPH(NAME, ...) \
/ { \
  macros { \
    NAME: NAME { \
      compatible = "zmk,behavior-macro"; \
      #binding-cells = <0>; \
      __VA_ARGS__ \
    }; \
  }; \
};

#define ZMK_SHIFT_MACRO(NAME, BINDING, SHIFT_BINDING) \
ZMK_MACRO_MORPH(macro_ ## NAME, wait-ms = <0>; bindings = <SHIFT_BINDING>;) \
ZMK_SHIFT_MORPH(NAME, BINDING, &macro_ ## NAME)
