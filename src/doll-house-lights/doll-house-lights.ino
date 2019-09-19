
#include "SmartIR.hpp"
#include "Keypad.hpp"
#include "LedStrip.hpp"
#include "LightManager.hpp"
#include "Rainbow.hpp"

using ledstrip::LedStrip;
using ledstrip::UpDown;

// ##### Config #####

// Where IR receiver data pin in connected
const uint8_t ir_pin = 4;

// Clock pin is shared by all led strips
const uint8_t clock_pin = 13;

LedStrip led_strips[] = {
  // # of pixels, data pin, clock pin
  LedStrip(4, 5, clock_pin),
  LedStrip(4, 6, clock_pin),
  LedStrip(4, 7, clock_pin),
  LedStrip(4, 8, clock_pin),
  LedStrip(4, 9, clock_pin),
  LedStrip(4, 10, clock_pin),
  LedStrip(40, 11, clock_pin),
  // LedStrip(4, 12, clock_pin), // spare channel
};


// ##### Globals #####
SmartIR ir(ir_pin);
lightmanager::LightManager lightman(led_strips, sizeof(led_strips)/sizeof(LedStrip));


uint8_t room_index(keypad::KeyCode room_key_code);
void handle_hsv_event(uint8_t room_index, keypad::KeyCode hsv_keycode);
void handle_power_event(bool lights_on);
void handle_color_event(uint8_t room_index, keypad::KeyCode color_keycode);


void setup() {
  Serial.begin(115200);
  ir.initialize();
  lightman.initialize();
}


void loop() {
  static struct {
    uint8_t current_room_index = 0;
    bool lights_on = false;
  } state;
  
  keypad::KeyCode keycode = 0;
  
  if (keycode=ir.recv()) {
  
    Serial.print(String("Received key ") + String(keycode, HEX) + ". ");

    if (keypad::isBrightness(keycode)) {
      Serial.println("It's a brightness key");
    
    } else if (keypad::isColor(keycode)) {
      Serial.println("It's a color key");
      handle_color_event(state.current_room_index, keycode);

    } else if (keypad::isHSV(keycode)) {
      Serial.println("It's a HSV key");
      handle_hsv_event(state.current_room_index, keycode);
    
    } else if (keypad::isRoom(keycode)) {
      Serial.println("It's a Room selection key");
      state.current_room_index = room_index(keycode);
    
    } else if (keypad::isCopyPaste(keycode)) {
      Serial.println("It's a copy/paste key");
    
    } else {
      switch (keycode) {
        case keypad::Power:
          Serial.println("It's a power key");
          state.lights_on = !state.lights_on;
          handle_power_event(state.lights_on);
          break;
        default:
          Serial.println("Non relevant key");
          break;
      }
    }
  }
}

uint8_t room_index(keypad::KeyCode room_key_code) {
  switch (room_key_code) {
    case keypad::RoomBL:
      return 0;
      break;
    case keypad::RoomBR:
      return 1;
      break;
    case keypad::RoomML:
      return 2;
      break;
    case keypad::RoomMR:
      return 3;
      break;
    case keypad::RoomTL:
      return 4;
      break;
    case keypad::RoomTR:
      return 5;
      break;
    case keypad::RoomEave:
      return 6;
      break;
    default:
      return 0;  // FIXME: Suboptimal - needs proper error handling.
      break;
  }
}

void handle_hsv_event(uint8_t room_index, keypad::KeyCode hsv_keycode) {
  UpDown h_dir = UpDown::None, s_dir = UpDown::None, v_dir = UpDown::None;
  
  switch (hsv_keycode) {
    case keypad::HSVHueUp:
      h_dir = UpDown::Up;
      break;
    case keypad::HSVHueDown:
      h_dir = UpDown::Down;
      break;
    case keypad::HSVSatUp:
      s_dir = UpDown::Up;
      break;
    case keypad::HSVSatDown:
      s_dir = UpDown::Down;
      break;
    case keypad::HSVValUp:
      v_dir = UpDown::Up;
      break;
    case keypad::HSVValDown:
      v_dir = UpDown::Down;
      break;
    default:
      Serial.print(String("Unknown HSV key: ") + String(hsv_keycode, HEX));
      return;
  }

  Serial.println(String("Adjusting HSV for room ") + room_index +
                 " h_dir=" + static_cast<int8_t>(h_dir) +
                 " s_dir=" + static_cast<int8_t>(s_dir) +
                 " v_dir=" + static_cast<int8_t>(v_dir));

  lightman.adjust_strip_hsv(room_index, h_dir, s_dir, v_dir);

}

void handle_power_event(bool lights_on) {
  Serial.println(String("Lights ") + (lights_on ? "on!" : "off!"));
  lights_on ? lightman.on() : lightman.off();
}

void handle_color_event(uint8_t room_index, keypad::KeyCode color_keycode) {
  rainbow::ColorHSV *color;
  const auto max_hue = 65535;
  const auto max_sat = 255;

  switch (color_keycode) {
    case keypad::Color51:  // Red
      color = new rainbow::ColorHSV(0, max_sat, 255);
      break;
    case keypad::Color41:  // Orange-ish red
      color = new rainbow::ColorHSV(1000, max_sat, 220);
      break;
    case keypad::Color31:  // Dark orange
      color = new rainbow::ColorHSV(2000, max_sat, 190);
      break;
    case keypad::Color21:  // Light orange
      color = new rainbow::ColorHSV(5000, max_sat, 160);
      break;
    case keypad::Color11:  // Yellow
      color = new rainbow::ColorHSV(max_hue/6, max_sat, 110);
      break;

    case keypad::Color52:  // Green
      color = new rainbow::ColorHSV(max_hue/3, max_sat, 150);
      break;
    case keypad::Color42:  // Light Green
      color = new rainbow::ColorHSV(max_hue/6+9000, max_sat, 150);
      break;
    case keypad::Color32:  // Bright green
      color = new rainbow::ColorHSV(max_hue/6+7000, max_sat, 150);
      break;
    case keypad::Color22:  // Yellow green
      color = new rainbow::ColorHSV(max_hue/6+5000, max_sat, 150);
      break;
    case keypad::Color12:  // Cold yellow
      color = new rainbow::ColorHSV(max_hue/6+2500, max_sat, 150);
      break;

    case keypad::Color53:  // Blue
      color = new rainbow::ColorHSV(max_hue/3*2, max_sat, 255);
      break;
    case keypad::Color43:  // Light blue
      color = new rainbow::ColorHSV(max_hue/3*2-2000, max_sat, 200);
      break;
    case keypad::Color33:  // Sky blue
      color = new rainbow::ColorHSV(max_hue/3*2-5000, max_sat, 140);
      break;
    case keypad::Color23:  // Cyan
      color = new rainbow::ColorHSV(max_hue/3*2-8000, max_sat, 120);
      break;
    case keypad::Color13:  // Cold cyan
      color = new rainbow::ColorHSV(max_hue/3*2-13000, max_sat, 100);
      break;

    case keypad::Color54:  // White
      color = new rainbow::ColorHSV(0, 0, 255);
      break;
    case keypad::Color44:  // Pink
      color = new rainbow::ColorHSV(max_hue/6*5+9000, max_sat, 250);
      break;
    case keypad::Color34:  // Cold ping
      color = new rainbow::ColorHSV(max_hue/6*5+5000, max_sat, 215);
      break;
    case keypad::Color24:  // Lilac
      color = new rainbow::ColorHSV(max_hue/6*5, max_sat, 150);
      break;
    case keypad::Color14:  // Light violet
      color = new rainbow::ColorHSV(max_hue/6*5-5000, max_sat, 170);
      break;

    default:
      Serial.print(String("Unsupported color key: ") + String(color_keycode, HEX));
      return;
  }

  lightman.set_strip_color(room_index, color);
  delete color;
}
