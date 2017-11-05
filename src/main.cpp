#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "lib/portlib.hpp"
#include "lib/pulsemanager.hpp"

using PortLib::DigitalPort;
using PortLib::Timer0;
using PulseManager::MultiPWM;

uint8_t const PWM_PINS[3] = { 0, 1, 2 };
uint8_t const INIT_LEVELS[3] = { 96, 255, 64 }; // GRB

DigitalPort porta{
    PINA,
    PORTA,
    DDRA
};
MultiPWM<3> pwm{
    PWM_PINS,
    INIT_LEVELS,
    porta
};
Timer0 pwm_timer{
    TCNT0,
    OCR0A,
    OCR0B,
    TCCR0A,
    TCCR0B,
    TIMSK0,
    TIFR0
};

ISR(TIM0_OVF_vect) {
    pwm.step();
}

int main() {
    porta.set_out();

    pwm_timer.set_mask(false, false, true);
    pwm_timer.set_mode(Timer0::Mode::normal);
    pwm_timer.set_prescale(Timer0::Prescale::d1);

    sei();
    while(true);
}
