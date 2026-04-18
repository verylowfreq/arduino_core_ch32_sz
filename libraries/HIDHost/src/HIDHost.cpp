#include "HIDHost.h"

#include <string.h>

#include <Adafruit_TinyUSB.h>

#if CFG_TUH_ENABLED && CFG_TUH_HID

namespace {

enum { HID_REPORT_INFO_MAX = 8 };
enum { KB_ASCII_BUF_SIZE = 16 };

struct HIDItfState {
  bool used;
  bool auto_receive;
  bool has_report_id;
  bool is_keyboard;
  bool is_mouse;
  bool is_gamepad;

  uint8_t dev_addr;
  uint8_t instance;
  uint8_t protocol;
  uint16_t vid;
  uint16_t pid;
  uint16_t usage_page;
  uint16_t usage;

  uint8_t prev_modifiers;
  uint8_t prev_keys[6];
  bool prev_keys_valid;
  uint8_t prev_mouse_buttons;
  bool prev_mouse_buttons_valid;
};

struct PendingFeatureReport {
  bool pending;
  uint8_t dev_addr;
  uint8_t instance;
  uint8_t report_id;
  uint8_t report_type;
  uint8_t* data;
  uint16_t datalen;
};

struct HIDHostState {
  bool enabled;
  HIDItfState itf[CFG_TUH_HID];

  uint16_t gamepad_usage_filter_page;
  uint16_t gamepad_usage_filter_usage;

  bool generic_bound;
  uint8_t generic_dev_addr;
  uint8_t generic_instance;
  PendingFeatureReport generic_feature_req;

  KeyboardHostAPI::MountCallback kb_mount_cb;
  KeyboardHostAPI::UmountCallback kb_umount_cb;
  KeyboardHostAPI::KeyCallback kb_key_down_cb;
  KeyboardHostAPI::KeyCallback kb_key_up_cb;

  MouseHostAPI::MountCallback mouse_mount_cb;
  MouseHostAPI::UmountCallback mouse_umount_cb;
  MouseHostAPI::MoveCallback mouse_move_cb;
  MouseHostAPI::WheelCallback mouse_wheel_cb;
  MouseHostAPI::WheelCallback mouse_hwheel_cb;

  GamepadHostAPI::MountCallback gamepad_mount_cb;
  GamepadHostAPI::UmountCallback gamepad_umount_cb;
  GamepadHostAPI::ReportCallback gamepad_report_cb;

  HIDGenericHostAPI::MountCallback generic_mount_cb;
  HIDGenericHostAPI::UmountCallback generic_umount_cb;
  HIDGenericHostAPI::ReportCallback generic_report_cb;
  HIDGenericHostAPI::FeatureCallback generic_feature_cb;

  uint8_t kb_ascii_buf[KB_ASCII_BUF_SIZE];
  uint8_t kb_ascii_head;
  uint8_t kb_ascii_tail;
  uint8_t kb_ascii_count;
  KeyboardLayout kb_layout;
  bool kb_caps_lock;
  bool kb_clear_state_on_umount;
};

HIDHostState& st() {
  static HIDHostState s;
  return s;
}

void kb_ascii_push(uint8_t ch) {
  HIDHostState& s = st();
  if (s.kb_ascii_count >= KB_ASCII_BUF_SIZE) {
    return;
  }

  s.kb_ascii_buf[s.kb_ascii_head] = ch;
  s.kb_ascii_head = (uint8_t) ((s.kb_ascii_head + 1u) % KB_ASCII_BUF_SIZE);
  s.kb_ascii_count++;
}

int kb_ascii_peek() {
  HIDHostState& s = st();
  if (s.kb_ascii_count == 0) {
    return -1;
  }
  return (int) s.kb_ascii_buf[s.kb_ascii_tail];
}

int kb_ascii_pop() {
  HIDHostState& s = st();
  if (s.kb_ascii_count == 0) {
    return -1;
  }

  uint8_t ch = s.kb_ascii_buf[s.kb_ascii_tail];
  s.kb_ascii_tail = (uint8_t) ((s.kb_ascii_tail + 1u) % KB_ASCII_BUF_SIZE);
  s.kb_ascii_count--;
  return (int) ch;
}

void kb_ascii_flush() {
  HIDHostState& s = st();
  s.kb_ascii_head = 0;
  s.kb_ascii_tail = 0;
  s.kb_ascii_count = 0;
}

bool kb_shift_active(uint8_t modifiers) {
  return (modifiers & (KEYBOARD_MODIFIER_LEFTSHIFT |
                       KEYBOARD_MODIFIER_RIGHTSHIFT)) != 0;
}

char kb_map_printable_us(uint8_t keycode, bool shifted, bool caps_lock) {
  if (keycode >= 0x04 && keycode <= 0x1D) {
    char c = (char) ('a' + (keycode - 0x04));
    if (shifted ^ caps_lock) {
      c = (char) (c - 'a' + 'A');
    }
    return c;
  }

  switch (keycode) {
    case 0x1E: return shifted ? '!' : '1';
    case 0x1F: return shifted ? '@' : '2';
    case 0x20: return shifted ? '#' : '3';
    case 0x21: return shifted ? '$' : '4';
    case 0x22: return shifted ? '%' : '5';
    case 0x23: return shifted ? '^' : '6';
    case 0x24: return shifted ? '&' : '7';
    case 0x25: return shifted ? '*' : '8';
    case 0x26: return shifted ? '(' : '9';
    case 0x27: return shifted ? ')' : '0';
    case 0x2D: return shifted ? '_' : '-';
    case 0x2E: return shifted ? '+' : '=';
    case 0x2F: return shifted ? '{' : '[';
    case 0x30: return shifted ? '}' : ']';
    case 0x31: return shifted ? '|' : '\\';
    case 0x33: return shifted ? ':' : ';';
    case 0x34: return shifted ? '"' : '\'';
    case 0x35: return shifted ? '~' : '`';
    case 0x36: return shifted ? '<' : ',';
    case 0x37: return shifted ? '>' : '.';
    case 0x38: return shifted ? '?' : '/';
    case 0x2C: return ' ';
    default: return 0;
  }
}

char kb_map_printable_jis(uint8_t keycode, bool shifted, bool caps_lock) {
  if (keycode >= 0x04 && keycode <= 0x1D) {
    char c = (char) ('a' + (keycode - 0x04));
    if (shifted ^ caps_lock) {
      c = (char) (c - 'a' + 'A');
    }
    return c;
  }

  switch (keycode) {
    case 0x1E: return shifted ? '!' : '1';
    case 0x1F: return shifted ? '"' : '2';
    case 0x20: return shifted ? '#' : '3';
    case 0x21: return shifted ? '$' : '4';
    case 0x22: return shifted ? '%' : '5';
    case 0x23: return shifted ? '&' : '6';
    case 0x24: return shifted ? '\'' : '7';
    case 0x25: return shifted ? '(' : '8';
    case 0x26: return shifted ? ')' : '9';
    case 0x27: return shifted ? 0 : '0';
    case 0x2D: return shifted ? '=' : '-';
    case 0x2E: return shifted ? '~' : '^';
    case 0x2F: return shifted ? '`' : '@';
    case 0x30: return shifted ? '{' : '[';
    case 0x31: return shifted ? '}' : ']';
    case 0x32: return shifted ? '_' : '\\';
    case 0x33: return shifted ? '+' : ';';
    case 0x34: return shifted ? '*' : ':';
    case 0x36: return shifted ? '<' : ',';
    case 0x37: return shifted ? '>' : '.';
    case 0x38: return shifted ? '?' : '/';
    case 0x2C: return ' ';
    default: return 0;
  }
}

char kb_to_ascii(uint8_t keycode, uint8_t modifiers) {
  HIDHostState& s = st();

  if (keycode == 0x28) { // Enter
    return '\n';
  }
  if (keycode == 0x2A) { // Backspace
    return '\b';
  }
  if (keycode == 0x2B) { // Tab
    return '\t';
  }

  bool shifted = kb_shift_active(modifiers);
  if (s.kb_layout == KeyboardLayout::JP_JIS) {
    return kb_map_printable_jis(keycode, shifted, s.kb_caps_lock);
  }
  return kb_map_printable_us(keycode, shifted, s.kb_caps_lock);
}

void kb_process_keydown_ascii(uint8_t keycode, uint8_t modifiers) {
  HIDHostState& s = st();
  if (keycode == 0x39) { // Caps Lock
    s.kb_caps_lock = !s.kb_caps_lock;
    return;
  }

  char c = kb_to_ascii(keycode, modifiers);
  if (c != 0) {
    kb_ascii_push((uint8_t) c);
  }
}

bool report_contains_key(uint8_t const* keycodes, uint8_t key) {
  if (key == 0) {
    return false;
  }
  for (uint8_t i = 0; i < 6; i++) {
    if (keycodes[i] == key) {
      return true;
    }
  }
  return false;
}

bool is_gamepad_usage(uint16_t usage_page, uint16_t usage) {
  if (usage_page != HID_USAGE_PAGE_DESKTOP) {
    return false;
  }
  return usage == HID_USAGE_DESKTOP_JOYSTICK ||
         usage == HID_USAGE_DESKTOP_GAMEPAD ||
         usage == HID_USAGE_DESKTOP_MULTI_AXIS_CONTROLLER;
}

bool gamepad_usage_filter_match(uint16_t usage_page, uint16_t usage) {
  HIDHostState& s = st();
  if (s.gamepad_usage_filter_page == 0 && s.gamepad_usage_filter_usage == 0) {
    return true;
  }
  return usage_page == s.gamepad_usage_filter_page &&
         usage == s.gamepad_usage_filter_usage;
}

HIDItfState* find_itf(uint8_t dev_addr, uint8_t instance) {
  HIDHostState& s = st();
  for (size_t i = 0; i < CFG_TUH_HID; i++) {
    HIDItfState& cur = s.itf[i];
    if (cur.used && cur.dev_addr == dev_addr && cur.instance == instance) {
      return &cur;
    }
  }
  return NULL;
}

HIDItfState* alloc_itf(uint8_t dev_addr, uint8_t instance) {
  HIDHostState& s = st();
  HIDItfState* existing = find_itf(dev_addr, instance);
  if (existing != NULL) {
    return existing;
  }

  for (size_t i = 0; i < CFG_TUH_HID; i++) {
    HIDItfState& cur = s.itf[i];
    if (!cur.used) {
      memset(&cur, 0, sizeof(cur));
      cur.used = true;
      cur.auto_receive = true;
      cur.dev_addr = dev_addr;
      cur.instance = instance;
      return &cur;
    }
  }

  return NULL;
}

bool class_mounted(bool HIDItfState::*member_flag) {
  HIDHostState& s = st();
  for (size_t i = 0; i < CFG_TUH_HID; i++) {
    HIDItfState const& cur = s.itf[i];
    if (cur.used && cur.*member_flag) {
      return true;
    }
  }
  return false;
}

bool class_mounted_at(uint8_t dev_addr, uint8_t instance,
                      bool HIDItfState::*member_flag) {
  HIDItfState* itf = find_itf(dev_addr, instance);
  return (itf != NULL) && (itf->*member_flag);
}

HIDItfState* first_keyboard_itf() {
  HIDHostState& s = st();
  for (size_t i = 0; i < CFG_TUH_HID; i++) {
    HIDItfState& cur = s.itf[i];
    if (cur.used && cur.is_keyboard) {
      return &cur;
    }
  }
  return NULL;
}

void dispatch_keyboard_report(HIDItfState& itf, uint8_t const* report,
                              uint16_t len) {
  if (len < 8) {
    return;
  }

  HIDHostState& s = st();
  uint8_t const modifiers = report[0];
  uint8_t const* keys = report + 2;
  uint8_t const prev_modifiers = itf.prev_modifiers;

  if (itf.prev_keys_valid) {
    uint8_t const mod_changed = (uint8_t) (prev_modifiers ^ modifiers);
    if (mod_changed != 0) {
      for (uint8_t bit = 0; bit < 8; bit++) {
        uint8_t const mask = (uint8_t) (1u << bit);
        if ((mod_changed & mask) == 0) {
          continue;
        }

        bool const now_down = (modifiers & mask) != 0;
        if (now_down) {
          if (s.kb_key_down_cb != NULL) {
            s.kb_key_down_cb(0, modifiers);
          }
        } else {
          if (s.kb_key_up_cb != NULL) {
            s.kb_key_up_cb(0, modifiers);
          }
        }
      }
    }

    for (uint8_t i = 0; i < 6; i++) {
      uint8_t key = keys[i];
      if (key != 0 && !report_contains_key(itf.prev_keys, key)) {
        if (s.kb_key_down_cb != NULL) {
          s.kb_key_down_cb(key, modifiers);
        }
        kb_process_keydown_ascii(key, modifiers);
      }
    }
    if (s.kb_key_up_cb != NULL) {
      for (uint8_t i = 0; i < 6; i++) {
        uint8_t key = itf.prev_keys[i];
        if (key != 0 && !report_contains_key(keys, key)) {
          s.kb_key_up_cb(key, modifiers);
        }
      }
    }
  } else {
    for (uint8_t bit = 0; bit < 8; bit++) {
      uint8_t const mask = (uint8_t) (1u << bit);
      if ((modifiers & mask) != 0 && s.kb_key_down_cb != NULL) {
        s.kb_key_down_cb(0, modifiers);
      }
    }

    for (uint8_t i = 0; i < 6; i++) {
      if (keys[i] != 0) {
        if (s.kb_key_down_cb != NULL) {
          s.kb_key_down_cb(keys[i], modifiers);
        }
        kb_process_keydown_ascii(keys[i], modifiers);
      }
    }
  }

  itf.prev_modifiers = modifiers;
  memcpy(itf.prev_keys, keys, 6);
  itf.prev_keys_valid = true;
}

void dispatch_mouse_report(HIDItfState& itf, uint8_t const* report,
                           uint16_t len) {
  if (len == 0) {
    return;
  }

  HIDHostState& s = st();
  uint8_t const buttons = report[0];
  int16_t dx = 0;
  int16_t dy = 0;
  if (len > 1u) {
    dx = (int8_t) report[1];
  }
  if (len > 2u) {
    dy = (int8_t) report[2];
  }

  bool const button_changed = (!itf.prev_mouse_buttons_valid) ||
                              (buttons != itf.prev_mouse_buttons);
  bool const has_motion = (dx != 0) || (dy != 0);

  // Fire move callback also for button-only transitions.
  if (s.mouse_move_cb != NULL && (has_motion || button_changed)) {
    s.mouse_move_cb(dx, dy, buttons);
  }

  if (len > 3u && report[3] != 0 && s.mouse_wheel_cb != NULL) {
    s.mouse_wheel_cb((int8_t) report[3]);
  }

  // Non-standard boot mice may include horizontal wheel as 5th byte.
  if (len > 4u && report[4] != 0 && s.mouse_hwheel_cb != NULL) {
    s.mouse_hwheel_cb((int8_t) report[4]);
  }

  itf.prev_mouse_buttons = buttons;
  itf.prev_mouse_buttons_valid = true;
}

void dispatch_gamepad_report(HIDItfState const& itf, uint8_t const* report,
                             uint16_t len) {
  HIDHostState& s = st();
  if (s.gamepad_report_cb == NULL) {
    return;
  }

  if (!gamepad_usage_filter_match(itf.usage_page, itf.usage)) {
    return;
  }

  HIDRawReportInfo info;
  info.vid = itf.vid;
  info.pid = itf.pid;
  info.dev_addr = itf.dev_addr;
  info.instance = itf.instance;
  info.usage_page = itf.usage_page;
  info.usage = itf.usage;
  info.report_id = (itf.has_report_id && len > 0) ? report[0] : 0;

  s.gamepad_report_cb(report, len, info);
}

void clear_generic_bind_if_match(uint8_t dev_addr, uint8_t instance) {
  HIDHostState& s = st();
  if (s.generic_bound && s.generic_dev_addr == dev_addr &&
      s.generic_instance == instance) {
    s.generic_bound = false;
  }

  if (s.generic_feature_req.pending &&
      s.generic_feature_req.dev_addr == dev_addr &&
      s.generic_feature_req.instance == instance) {
    s.generic_feature_req.pending = false;
  }
}

} // namespace

KeyboardHostAPI::KeyboardHostAPI() {}

bool KeyboardHostAPI::begin() { return HIDHost.begin(); }

void KeyboardHostAPI::end() {}

bool KeyboardHostAPI::mounted() const { return class_mounted(&HIDItfState::is_keyboard); }

bool KeyboardHostAPI::mounted(uint8_t dev_addr, uint8_t instance) const {
  return class_mounted_at(dev_addr, instance, &HIDItfState::is_keyboard);
}

void KeyboardHostAPI::onMount(MountCallback cb) { st().kb_mount_cb = cb; }

void KeyboardHostAPI::onUmount(UmountCallback cb) { st().kb_umount_cb = cb; }

void KeyboardHostAPI::onKeyDown(KeyCallback cb) { st().kb_key_down_cb = cb; }

void KeyboardHostAPI::onKeyUp(KeyCallback cb) { st().kb_key_up_cb = cb; }

bool KeyboardHostAPI::setLeds(uint8_t leds) {
  HIDItfState* itf = first_keyboard_itf();
  if (itf == NULL) {
    return false;
  }
  return setLeds(itf->dev_addr, itf->instance, leds);
}

bool KeyboardHostAPI::setLeds(uint8_t dev_addr, uint8_t instance, uint8_t leds) {
  if (!mounted(dev_addr, instance)) {
    return false;
  }
  return tuh_hid_set_report(dev_addr, instance, 0, HID_REPORT_TYPE_OUTPUT, &leds,
                            sizeof(leds));
}

int KeyboardHostAPI::available() { return (int) st().kb_ascii_count; }

int KeyboardHostAPI::peak() { return kb_ascii_peek(); }

int KeyboardHostAPI::read() { return kb_ascii_pop(); }

size_t KeyboardHostAPI::read(uint8_t* buf, size_t len) {
  if (buf == NULL || len == 0) {
    return 0;
  }

  size_t n = 0;
  while (n < len) {
    int c = kb_ascii_pop();
    if (c < 0) {
      break;
    }
    buf[n++] = (uint8_t) c;
  }
  return n;
}

void KeyboardHostAPI::flush() { kb_ascii_flush(); }

void KeyboardHostAPI::setLayout(KeyboardLayout layout) { st().kb_layout = layout; }

void KeyboardHostAPI::clearStateOnUmount(bool enabled) {
  st().kb_clear_state_on_umount = enabled;
}

MouseHostAPI::MouseHostAPI() {}

bool MouseHostAPI::begin() { return HIDHost.begin(); }

void MouseHostAPI::end() {}

bool MouseHostAPI::mounted() const { return class_mounted(&HIDItfState::is_mouse); }

bool MouseHostAPI::mounted(uint8_t dev_addr, uint8_t instance) const {
  return class_mounted_at(dev_addr, instance, &HIDItfState::is_mouse);
}

void MouseHostAPI::onMount(MountCallback cb) { st().mouse_mount_cb = cb; }

void MouseHostAPI::onUmount(UmountCallback cb) { st().mouse_umount_cb = cb; }

void MouseHostAPI::onMove(MoveCallback cb) { st().mouse_move_cb = cb; }

void MouseHostAPI::onWheel(WheelCallback cb) { st().mouse_wheel_cb = cb; }

void MouseHostAPI::onHWheel(WheelCallback cb) { st().mouse_hwheel_cb = cb; }

GamepadHostAPI::GamepadHostAPI() {}

bool GamepadHostAPI::begin() { return HIDHost.begin(); }

void GamepadHostAPI::end() {}

bool GamepadHostAPI::mounted() const { return class_mounted(&HIDItfState::is_gamepad); }

bool GamepadHostAPI::mounted(uint8_t dev_addr, uint8_t instance) const {
  return class_mounted_at(dev_addr, instance, &HIDItfState::is_gamepad);
}

void GamepadHostAPI::onMount(MountCallback cb) { st().gamepad_mount_cb = cb; }

void GamepadHostAPI::onUmount(UmountCallback cb) { st().gamepad_umount_cb = cb; }

void GamepadHostAPI::onReport(ReportCallback cb) { st().gamepad_report_cb = cb; }

void GamepadHostAPI::setUsageFilter(uint16_t usage_page, uint16_t usage) {
  st().gamepad_usage_filter_page = usage_page;
  st().gamepad_usage_filter_usage = usage;
}

HIDGenericHostAPI::HIDGenericHostAPI() {}

bool HIDGenericHostAPI::begin() { return HIDHost.begin(); }

void HIDGenericHostAPI::end() {}

bool HIDGenericHostAPI::bind(uint8_t dev_addr, uint8_t instance) {
  if (find_itf(dev_addr, instance) == NULL) {
    return false;
  }
  st().generic_bound = true;
  st().generic_dev_addr = dev_addr;
  st().generic_instance = instance;
  return true;
}

void HIDGenericHostAPI::unbind() { st().generic_bound = false; }

bool HIDGenericHostAPI::bound() const { return st().generic_bound; }

bool HIDGenericHostAPI::requestInputReport() {
  HIDHostState& s = st();
  if (!s.generic_bound) {
    return false;
  }
  return tuh_hid_receive_report(s.generic_dev_addr, s.generic_instance);
}

bool HIDGenericHostAPI::sendReport(uint8_t const* data, uint16_t datalen,
                                uint8_t report_id) {
  HIDHostState& s = st();
  if (!s.generic_bound || data == NULL || datalen == 0) {
    return false;
  }
  return tuh_hid_send_report(s.generic_dev_addr, s.generic_instance, report_id,
                             data, datalen);
}

bool HIDGenericHostAPI::getFeatureReport(uint8_t report_id, uint8_t* data,
                                      uint16_t datalen) {
  HIDHostState& s = st();
  if (!s.generic_bound || data == NULL || datalen == 0 ||
      s.generic_feature_req.pending) {
    return false;
  }

  s.generic_feature_req.pending = true;
  s.generic_feature_req.dev_addr = s.generic_dev_addr;
  s.generic_feature_req.instance = s.generic_instance;
  s.generic_feature_req.report_id = report_id;
  s.generic_feature_req.report_type = HID_REPORT_TYPE_FEATURE;
  s.generic_feature_req.data = data;
  s.generic_feature_req.datalen = datalen;

  if (!tuh_hid_get_report(s.generic_dev_addr, s.generic_instance, report_id,
                          HID_REPORT_TYPE_FEATURE, data, datalen)) {
    s.generic_feature_req.pending = false;
    return false;
  }

  return true;
}

bool HIDGenericHostAPI::setFeatureReport(uint8_t report_id, uint8_t const* data,
                                      uint16_t datalen) {
  HIDHostState& s = st();
  if (!s.generic_bound || data == NULL || datalen == 0) {
    return false;
  }
  return tuh_hid_set_report(s.generic_dev_addr, s.generic_instance, report_id,
                            HID_REPORT_TYPE_FEATURE, (void*) data, datalen);
}

void HIDGenericHostAPI::onMount(MountCallback cb) { st().generic_mount_cb = cb; }

void HIDGenericHostAPI::onUmount(UmountCallback cb) { st().generic_umount_cb = cb; }

void HIDGenericHostAPI::onReport(ReportCallback cb) { st().generic_report_cb = cb; }

void HIDGenericHostAPI::onFeatureReport(FeatureCallback cb) {
  st().generic_feature_cb = cb;
}

bool HIDHostAPI::begin() {
  HIDHostState& s = st();
  memset(s.itf, 0, sizeof(s.itf));
  s.generic_bound = false;
  s.generic_feature_req.pending = false;
  kb_ascii_flush();
  s.kb_layout = KeyboardLayout::US_ASCII;
  s.kb_caps_lock = false;
  s.kb_clear_state_on_umount = true;
  s.enabled = true;
  return true;
}

void HIDHostAPI::end() { st().enabled = false; }

void HIDHostAPI::task() {}

KeyboardHostAPI& HIDHostAPI::keyboard() {
  static KeyboardHostAPI obj;
  return obj;
}

MouseHostAPI& HIDHostAPI::mouse() {
  static MouseHostAPI obj;
  return obj;
}

GamepadHostAPI& HIDHostAPI::gamepad() {
  static GamepadHostAPI obj;
  return obj;
}

HIDGenericHostAPI& HIDHostAPI::generic() {
  static HIDGenericHostAPI obj;
  return obj;
}

bool HIDHostAPI::startAutoReceive(uint8_t dev_addr, uint8_t instance) {
  HIDItfState* itf = find_itf(dev_addr, instance);
  if (itf == NULL) {
    return false;
  }
  itf->auto_receive = true;
  return tuh_hid_receive_report(dev_addr, instance);
}

bool HIDHostAPI::stopAutoReceive(uint8_t dev_addr, uint8_t instance) {
  HIDItfState* itf = find_itf(dev_addr, instance);
  if (itf == NULL) {
    return false;
  }
  itf->auto_receive = false;
  return tuh_hid_receive_abort(dev_addr, instance);
}

bool HIDHostAPI::isAutoReceiveEnabled(uint8_t dev_addr, uint8_t instance) const {
  HIDItfState* itf = find_itf(dev_addr, instance);
  return (itf != NULL) ? itf->auto_receive : false;
}

void HIDHostAPI::handleMount(uint8_t dev_addr, uint8_t instance,
                          uint8_t const* report_desc, uint16_t desc_len) {
  HIDHostState& s = st();
  if (!s.enabled) {
    return;
  }

  HIDItfState* itf = alloc_itf(dev_addr, instance);
  if (itf == NULL) {
    return;
  }

  itf->protocol = tuh_hid_interface_protocol(dev_addr, instance);
  tuh_vid_pid_get(dev_addr, &itf->vid, &itf->pid);

  itf->is_keyboard = (itf->protocol == HID_ITF_PROTOCOL_KEYBOARD);
  itf->is_mouse = (itf->protocol == HID_ITF_PROTOCOL_MOUSE);
  itf->is_gamepad = false;
  itf->usage_page = 0;
  itf->usage = 0;
  itf->has_report_id = false;
  itf->prev_keys_valid = false;
  itf->prev_mouse_buttons_valid = false;

  if (report_desc != NULL && desc_len > 0) {
    tuh_hid_report_info_t report_info[HID_REPORT_INFO_MAX];
    uint8_t const count = tuh_hid_parse_report_descriptor(
        report_info, HID_REPORT_INFO_MAX, report_desc, desc_len);

    for (uint8_t i = 0; i < count; i++) {
      if (report_info[i].report_id != 0) {
        itf->has_report_id = true;
      }
      if (itf->usage_page == 0 && itf->usage == 0) {
        itf->usage_page = report_info[i].usage_page;
        itf->usage = report_info[i].usage;
      }
      if (is_gamepad_usage(report_info[i].usage_page, report_info[i].usage)) {
        itf->usage_page = report_info[i].usage_page;
        itf->usage = report_info[i].usage;
        itf->is_gamepad = true;
        break;
      }
    }
  }

  if (!itf->is_keyboard && !itf->is_mouse && !itf->is_gamepad &&
      is_gamepad_usage(itf->usage_page, itf->usage)) {
    itf->is_gamepad = true;
  }

  if (itf->is_keyboard && s.kb_mount_cb != NULL) {
    s.kb_mount_cb(dev_addr, instance, itf->vid, itf->pid);
  }
  if (itf->is_mouse && s.mouse_mount_cb != NULL) {
    s.mouse_mount_cb(dev_addr, instance, itf->vid, itf->pid);
  }
  if (itf->is_gamepad && s.gamepad_mount_cb != NULL) {
    s.gamepad_mount_cb(dev_addr, instance, itf->vid, itf->pid);
  }
  if (s.generic_mount_cb != NULL) {
    s.generic_mount_cb(dev_addr, instance, itf->vid, itf->pid);
  }

  if (itf->auto_receive) {
    (void) tuh_hid_receive_report(dev_addr, instance);
  }
}

void HIDHostAPI::handleUmount(uint8_t dev_addr, uint8_t instance) {
  HIDHostState& s = st();
  HIDItfState* itf = find_itf(dev_addr, instance);

  if (itf != NULL) {
    if (itf->is_keyboard && s.kb_umount_cb != NULL) {
      s.kb_umount_cb(dev_addr, instance);
    }
    if (itf->is_mouse && s.mouse_umount_cb != NULL) {
      s.mouse_umount_cb(dev_addr, instance);
    }
    if (itf->is_gamepad && s.gamepad_umount_cb != NULL) {
      s.gamepad_umount_cb(dev_addr, instance);
    }

    if (itf->is_keyboard && s.kb_clear_state_on_umount) {
      kb_ascii_flush();
      s.kb_caps_lock = false;
    }
  }

  if (s.generic_umount_cb != NULL) {
    s.generic_umount_cb(dev_addr, instance);
  }

  clear_generic_bind_if_match(dev_addr, instance);

  if (itf != NULL) {
    memset(itf, 0, sizeof(*itf));
  }
}

void HIDHostAPI::handleReport(uint8_t dev_addr, uint8_t instance,
                           uint8_t const* report, uint16_t len) {
  HIDItfState* itf = find_itf(dev_addr, instance);
  if (itf == NULL) {
    return;
  }

  HIDHostState& s = st();

  if (itf->is_keyboard) {
    dispatch_keyboard_report(*itf, report, len);
  } else if (itf->is_mouse) {
    dispatch_mouse_report(*itf, report, len);
  } else if (itf->is_gamepad) {
    dispatch_gamepad_report(*itf, report, len);
  }

  if (s.generic_bound && s.generic_dev_addr == dev_addr &&
      s.generic_instance == instance && s.generic_report_cb != NULL) {
    s.generic_report_cb(report, len);
  }

  if (itf->auto_receive) {
    (void) tuh_hid_receive_report(dev_addr, instance);
  }
}

void HIDHostAPI::handleGetReportComplete(uint8_t dev_addr, uint8_t instance,
                                      uint8_t report_id, uint8_t report_type,
                                      uint16_t len) {
  HIDHostState& s = st();
  PendingFeatureReport& p = s.generic_feature_req;
  if (!p.pending) {
    return;
  }

  if (p.dev_addr != dev_addr || p.instance != instance ||
      p.report_id != report_id || p.report_type != report_type) {
    return;
  }

  p.pending = false;

  if (s.generic_feature_cb != NULL) {
    if (len > p.datalen) {
      len = p.datalen;
    }
    s.generic_feature_cb(p.data, len);
  }
}

extern "C" {

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance,
                      uint8_t const* report_desc, uint16_t desc_len) {
  HIDHost.handleMount(dev_addr, instance, report_desc, desc_len);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  HIDHost.handleUmount(dev_addr, instance);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const* report, uint16_t len) {
  HIDHost.handleReport(dev_addr, instance, report, len);
}

void tuh_hid_get_report_complete_cb(uint8_t dev_addr, uint8_t instance,
                                    uint8_t report_id, uint8_t report_type,
                                    uint16_t len) {
  HIDHost.handleGetReportComplete(dev_addr, instance, report_id,
                                              report_type, len);
}

} // extern "C"

#else

KeyboardHostAPI::KeyboardHostAPI() {}
bool KeyboardHostAPI::begin() { return false; }
void KeyboardHostAPI::end() {}
bool KeyboardHostAPI::mounted() const { return false; }
bool KeyboardHostAPI::mounted(uint8_t, uint8_t) const { return false; }
void KeyboardHostAPI::onMount(MountCallback) {}
void KeyboardHostAPI::onUmount(UmountCallback) {}
void KeyboardHostAPI::onKeyDown(KeyCallback) {}
void KeyboardHostAPI::onKeyUp(KeyCallback) {}
bool KeyboardHostAPI::setLeds(uint8_t) { return false; }
bool KeyboardHostAPI::setLeds(uint8_t, uint8_t, uint8_t) { return false; }
int KeyboardHostAPI::available() { return 0; }
int KeyboardHostAPI::peak() { return -1; }
int KeyboardHostAPI::read() { return -1; }
size_t KeyboardHostAPI::read(uint8_t*, size_t) { return 0; }
void KeyboardHostAPI::flush() {}
void KeyboardHostAPI::setLayout(KeyboardLayout) {}
void KeyboardHostAPI::clearStateOnUmount(bool) {}

MouseHostAPI::MouseHostAPI() {}
bool MouseHostAPI::begin() { return false; }
void MouseHostAPI::end() {}
bool MouseHostAPI::mounted() const { return false; }
bool MouseHostAPI::mounted(uint8_t, uint8_t) const { return false; }
void MouseHostAPI::onMount(MountCallback) {}
void MouseHostAPI::onUmount(UmountCallback) {}
void MouseHostAPI::onMove(MoveCallback) {}
void MouseHostAPI::onWheel(WheelCallback) {}
void MouseHostAPI::onHWheel(WheelCallback) {}

GamepadHostAPI::GamepadHostAPI() {}
bool GamepadHostAPI::begin() { return false; }
void GamepadHostAPI::end() {}
bool GamepadHostAPI::mounted() const { return false; }
bool GamepadHostAPI::mounted(uint8_t, uint8_t) const { return false; }
void GamepadHostAPI::onMount(MountCallback) {}
void GamepadHostAPI::onUmount(UmountCallback) {}
void GamepadHostAPI::onReport(ReportCallback) {}
void GamepadHostAPI::setUsageFilter(uint16_t, uint16_t) {}

HIDGenericHostAPI::HIDGenericHostAPI() {}
bool HIDGenericHostAPI::begin() { return false; }
void HIDGenericHostAPI::end() {}
bool HIDGenericHostAPI::bind(uint8_t, uint8_t) { return false; }
void HIDGenericHostAPI::unbind() {}
bool HIDGenericHostAPI::bound() const { return false; }
bool HIDGenericHostAPI::requestInputReport() { return false; }
bool HIDGenericHostAPI::sendReport(uint8_t const*, uint16_t, uint8_t) {
  return false;
}
bool HIDGenericHostAPI::getFeatureReport(uint8_t, uint8_t*, uint16_t) {
  return false;
}
bool HIDGenericHostAPI::setFeatureReport(uint8_t, uint8_t const*, uint16_t) {
  return false;
}
void HIDGenericHostAPI::onMount(MountCallback) {}
void HIDGenericHostAPI::onUmount(UmountCallback) {}
void HIDGenericHostAPI::onReport(ReportCallback) {}
void HIDGenericHostAPI::onFeatureReport(FeatureCallback) {}

bool HIDHostAPI::begin() { return false; }
void HIDHostAPI::end() {}
void HIDHostAPI::task() {}
KeyboardHostAPI& HIDHostAPI::keyboard() {
  static KeyboardHostAPI obj;
  return obj;
}
MouseHostAPI& HIDHostAPI::mouse() {
  static MouseHostAPI obj;
  return obj;
}
GamepadHostAPI& HIDHostAPI::gamepad() {
  static GamepadHostAPI obj;
  return obj;
}
HIDGenericHostAPI& HIDHostAPI::generic() {
  static HIDGenericHostAPI obj;
  return obj;
}
bool HIDHostAPI::startAutoReceive(uint8_t, uint8_t) { return false; }
bool HIDHostAPI::stopAutoReceive(uint8_t, uint8_t) { return false; }
bool HIDHostAPI::isAutoReceiveEnabled(uint8_t, uint8_t) const { return false; }
void HIDHostAPI::handleMount(uint8_t, uint8_t, uint8_t const*, uint16_t) {}
void HIDHostAPI::handleUmount(uint8_t, uint8_t) {}
void HIDHostAPI::handleReport(uint8_t, uint8_t, uint8_t const*, uint16_t) {}
void HIDHostAPI::handleGetReportComplete(uint8_t, uint8_t, uint8_t, uint8_t,
                                      uint16_t) {}

#endif

HIDHostAPI HIDHost;
KeyboardHostAPI KeyboardHost;
MouseHostAPI MouseHost;
GamepadHostAPI GamepadHost;
HIDGenericHostAPI HIDGenericHost;



