#if defined(ARDUINO_ARCH_CH32) && defined(CH32V20x)
#include <Arduino.h>
#include <Servo.h>

#include <HardwareTimer.h>
#include <ch32v20x.h>

static HardwareTimer timer1(TIM1);
static HardwareTimer timer2(TIM2);
static HardwareTimer timer3(TIM3);


typedef struct {
  bool is_active;
  int pin;
  HardwareTimer* timer;
  uint8_t channel;
  uint16_t current_pulse_width;
} servo_entry_t;

servo_entry_t servo_list[] = {
    { false, 0, &timer1, 3, 0 },
    { false, 1, &timer1, 2, 0 },
    { false, 2, &timer1, 1, 0 },
    { false, 5, &timer2, 2, 0 },
    { false, 6, &timer3, 1, 0 },
    { false, 7, &timer3, 2, 0 },
    { false, 8, &timer2, 4, 0 },
    { false, 9, &timer2, 3, 0 }
};
constexpr size_t servo_list_count = sizeof(servo_list) / sizeof(servo_list[0]);


Servo::Servo() {
    this->servoIndex = INVALID_SERVO;
}

uint8_t Servo::attach(int pin) {
    for (size_t i = 0; i < servo_list_count; i++) {
        if (pin == servo_list[i].pin) {
            uint8_t timer_ch = servo_list[i].channel;
            HardwareTimer* timer = servo_list[i].timer;
            timer->pause();
            timer->setPrescaleFactor(SystemCoreClock / 1000000);
            timer->setOverflow(REFRESH_INTERVAL);
            timer->setMode(timer_ch, TIMER_OUTPUT_COMPARE_PWM1, digitalPinToPinName(pin));
            timer->setCaptureCompare(timer_ch, DEFAULT_PULSE_WIDTH);
            timer->refresh();
            timer->resume();
            servo_list[i].is_active = true;
            this->servoIndex = i;
            return i;
        }
    }
    this->servoIndex = INVALID_SERVO;
    return INVALID_SERVO;
}


void Servo::write(int value) {
    if (this->servoIndex == INVALID_SERVO) {
        return;
    }
    servo_entry_t* servo = &servo_list[this->servoIndex];

    if (value < 200) {
        // Treat as angle
        value = map(value, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
    }

    servo->timer->pause();
    servo->timer->setCaptureCompare(servo->channel, value);
    servo->timer->refresh();
    servo->timer->resume();
}

#endif
