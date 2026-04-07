#if defined(ARDUINO_ARCH_CH32) && defined(CH32V20x)
#include <Arduino.h>
#include <Servo.h>

#include <HardwareTimer.h>
#include <ch32v20x.h>

static HardwareTimer timer1(TIM1);
static HardwareTimer timer2(TIM2);
static HardwareTimer timer3(TIM3);

enum servo_remap_group_t {
  SERVO_REMAP_NONE = 0,
  SERVO_REMAP_TIM2_NO_REMAP,
  SERVO_REMAP_TIM2_FULL_REMAP,
};

typedef struct {
  bool is_active;
  int pin;
  HardwareTimer* timer;
  uint8_t channel;
  uint16_t current_pulse_width;
  servo_remap_group_t remap_group;
} servo_entry_t;

servo_entry_t servo_list[] = {
    { false, PA0,  &timer2, 1, 0, SERVO_REMAP_TIM2_NO_REMAP },
    { false, PA1,  &timer2, 2, 0, SERVO_REMAP_TIM2_NO_REMAP },
    { false, PA2,  &timer2, 3, 0, SERVO_REMAP_TIM2_NO_REMAP },
    { false, PA3,  &timer2, 4, 0, SERVO_REMAP_TIM2_NO_REMAP },
    { false, PA10, &timer1, 3, 0, SERVO_REMAP_NONE },
    { false, PA9,  &timer1, 2, 0, SERVO_REMAP_NONE },
    { false, PA8,  &timer1, 1, 0, SERVO_REMAP_NONE },
    { false, PB3,  &timer2, 2, 0, SERVO_REMAP_TIM2_FULL_REMAP },
    { false, PB4,  &timer3, 1, 0, SERVO_REMAP_NONE },
    { false, PB5,  &timer3, 2, 0, SERVO_REMAP_NONE },
    { false, PB11, &timer2, 4, 0, SERVO_REMAP_TIM2_FULL_REMAP },
    { false, PB10, &timer2, 3, 0, SERVO_REMAP_TIM2_FULL_REMAP }
};
constexpr size_t servo_list_count = sizeof(servo_list) / sizeof(servo_list[0]);
static servo_remap_group_t timer2_active_remap = SERVO_REMAP_NONE;

static servo_remap_group_t *activeRemapForTimer(HardwareTimer *timer)
{
    if (timer == &timer2) {
        return &timer2_active_remap;
    }

    return nullptr;
}

static bool timerHasActiveServos(HardwareTimer *timer)
{
    for (size_t i = 0; i < servo_list_count; i++) {
        if (servo_list[i].is_active && servo_list[i].timer == timer) {
            return true;
        }
    }

    return false;
}

static int clampPulseWidth(int value)
{
    if (value < MIN_PULSE_WIDTH) {
        return MIN_PULSE_WIDTH;
    }
    if (value > MAX_PULSE_WIDTH) {
        return MAX_PULSE_WIDTH;
    }
    return value;
}

static int clampAngle(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > 180) {
        return 180;
    }
    return value;
}

Servo::Servo() {
    this->servoIndex = INVALID_SERVO;
    this->minPulseWidth = MIN_PULSE_WIDTH;
    this->maxPulseWidth = MAX_PULSE_WIDTH;
}

uint8_t Servo::attach(int pin) {
    return attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int pin, int min, int max) {
    if (min > max) {
        int tmp = min;
        min = max;
        max = tmp;
    }

    min = clampPulseWidth(min);
    max = clampPulseWidth(max);

    for (size_t i = 0; i < servo_list_count; i++) {
        if (pin == servo_list[i].pin) {
            if (servo_list[i].is_active && this->servoIndex != i) {
                this->servoIndex = INVALID_SERVO;
                return INVALID_SERVO;
            }

            servo_remap_group_t *active_remap = activeRemapForTimer(servo_list[i].timer);
            if (active_remap != nullptr) {
                if (*active_remap == SERVO_REMAP_NONE) {
                    *active_remap = servo_list[i].remap_group;
                } else if (*active_remap != servo_list[i].remap_group) {
                    this->servoIndex = INVALID_SERVO;
                    return INVALID_SERVO;
                }
            }

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
            servo_list[i].current_pulse_width = DEFAULT_PULSE_WIDTH;
            this->servoIndex = i;
            this->minPulseWidth = min;
            this->maxPulseWidth = max;
            return i;
        }
    }
    this->servoIndex = INVALID_SERVO;
    return INVALID_SERVO;
}

void Servo::detach() {
    if (this->servoIndex == INVALID_SERVO) {
        return;
    }

    servo_entry_t* servo = &servo_list[this->servoIndex];
    servo->timer->pause();
    servo->timer->setCaptureCompare(servo->channel, 0);
    servo->timer->setMode(servo->channel, TIMER_DISABLED);
    servo->timer->refresh();
    servo->timer->resume();
    servo->is_active = false;
    servo->current_pulse_width = 0;

    servo_remap_group_t *active_remap = activeRemapForTimer(servo->timer);
    if (active_remap != nullptr && !timerHasActiveServos(servo->timer)) {
        *active_remap = SERVO_REMAP_NONE;
    }

    this->servoIndex = INVALID_SERVO;
}


void Servo::write(int value) {
    if (this->servoIndex == INVALID_SERVO) {
        return;
    }

    if (value < 200) {
        value = map(clampAngle(value), 0, 180, this->minPulseWidth, this->maxPulseWidth);
    }

    writeMicroseconds(value);
}

void Servo::writeMicroseconds(int value) {
    if (this->servoIndex == INVALID_SERVO) {
        return;
    }

    servo_entry_t* servo = &servo_list[this->servoIndex];
    value = constrain(value, this->minPulseWidth, this->maxPulseWidth);

    servo->timer->pause();
    servo->timer->setCaptureCompare(servo->channel, value);
    servo->timer->refresh();
    servo->timer->resume();
    servo->current_pulse_width = value;
}

int Servo::read() {
    return map(readMicroseconds(), this->minPulseWidth, this->maxPulseWidth, 0, 180);
}

int Servo::readMicroseconds() {
    if (this->servoIndex == INVALID_SERVO) {
        return 0;
    }

    return servo_list[this->servoIndex].current_pulse_width;
}

bool Servo::attached() {
    if (this->servoIndex == INVALID_SERVO) {
        return false;
    }

    return servo_list[this->servoIndex].is_active;
}

#endif
