#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include <portlib/digitalport.hpp>
#include <portlib/digitalpin.hpp>
#include <portlib/timer.hpp>
#include <utility/array.hpp>

#include <emulated/multipwm.hpp>
#include <peripheral/rotaryencoder.hpp>

using namespace avrsupport;
using namespace portlib;
using namespace utility;
using peripheral::RotaryEncoder;
using Timer0 = Timer<uint8_t>;

static DigitalPort
    port_a { PINA, PORTA, DDRA},
    port_b { PINB, PORTB, DDRB};

static DigitalPin
    knob_pin_a {port_b, 0, IoDirection::in, LogicLevel::high},
    knob_pin_b {port_b, 1, IoDirection::in, LogicLevel::high},
    knob_button{port_a, 6, IoDirection::in, LogicLevel::high},
     aux_button{port_a, 0, IoDirection::in, LogicLevel::high};

static emulated::MultiPwm<3, 15> pwm{ // 255 is evenly divisible by 15
    {1, 3, 5}, // Pins
    {0, 0, 0}, // Levels
    port_a
};

static Timer0 timer_0{
    TCNT0,
    OCR0A, OCR0B,
    TCCR0A, TCCR0B,
    TIMSK0,
    TIFR0
};

ISR(TIM0_OVF_vect) { pwm.step(); }

void handle_buttons(
    LogicLevel const knob_button,
    LogicLevel const aux_button
) {
    if (knob_button == LogicLevel::low) {
        pwm.pause();
        
        pwm.select_next();
        pwm.isolate_selected();
        _delay_ms(500);

        pwm.resume();
    }

    if (aux_button == LogicLevel::low) {
        // TODO: Select next preset
        pwm.pause();
        pwm.isolate_selected();
        _delay_ms(1000);
        pwm.resume();
    }
}

void handle_knob_rotate(RotaryEncoder & knob) {
    if      (knob.turned_right()) pwm.adjust_up();
    else if (knob.turned_left ()) pwm.adjust_down();
    else return;

    knob.clear();
}

int main() {
    RotaryEncoder knob;

    pwm.set_pins_out();

    timer_0
        .set_mode        (Timer0::Mode::normal)
        .set_prescale    (Timer0::Prescale::d1)
        .enable_interrupt(Timer0::Trigger::overflow);

    sei();

    while(true) {
        handle_buttons(knob_button.get(), aux_button.get());
        knob.sample(
            !static_cast<bool>(knob_pin_a.get()),
            !static_cast<bool>(knob_pin_b.get())
        );
        handle_knob_rotate(knob);

        // Debounce
        _delay_us(20);
    }
}
