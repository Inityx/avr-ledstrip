#ifndef PORTLIB_H
#define PORTLIB_H

#include <stdint.h>

namespace PortLib {
        
    typedef volatile uint8_t & Register;
    typedef const uint8_t PinIndex;

    struct DigitalPort {
    protected:
        // Members
        Register pinx;
        Register portx;
        Register ddrx;

    public:
        // Constructors
        constexpr DigitalPort(
            Register pinx,
            Register portx,
            Register ddrx
        ) :
            pinx(pinx),
            portx(portx),
            ddrx(ddrx)
        {}

        // Accessors
        inline uint8_t get() const { return pinx; }
        inline void set(uint8_t rhs) { portx = rhs; }
        inline void set_out() { ddrx = 0xFF; }
        inline void set_in() { ddrx = 0x00; }

        inline void set_out (PinIndex index) { ddrx  |=  (1<<index); }
        inline void set_in  (PinIndex index) { ddrx  &= ~(1<<index); }
        inline void set_high(PinIndex index) { portx |=  (1<<index); }
        inline void set_low (PinIndex index) { portx &= ~(1<<index); }
        inline void toggle  (PinIndex index) { portx ^=  (1<<index); }

        inline void set(bool rhs, PinIndex index) { rhs ? set_high(index) : set_low(index); }

        // Relationships
        friend class DigitalPin;
    };

    struct DigitalPin {
    protected:
        // Members
        DigitalPort port;
        uint8_t bitmask;

    public:
        // Constructors
        constexpr DigitalPin(
            DigitalPort port,
            PinIndex index
        ) :
            port(port),
            bitmask(1<<index)
        {}

        // Accessors
        inline bool get() const { return port.pinx | bitmask; }

        inline void set_out()  { port.ddrx  |=  bitmask; }
        inline void set_in()   { port.ddrx  &= ~bitmask; }
        inline void set_high() { port.portx |=  bitmask; }
        inline void set_low()  { port.portx &= ~bitmask; }
        inline void toggle()   { port.portx ^=  bitmask; }

        inline void set(bool rhs) { rhs ? set_high() : set_low(); }
    };

    template<typename Size>
    struct Timer {
        enum struct Prescale : uint8_t {
            off = 0x00,
            d1,
            d8,
            d64,
            d256,
            d1024,
            t0_falling_edge,
            t0_rising_edge
        };
        
        enum struct Mode : uint8_t {
            normal = 0x00,
            pwm_phase_correct,
            clear_on_comare,
            fast_pwm
        };

    private:
        static uint8_t const PRESCALE_BITMASK{0x07};
        static uint16_t const MODE_BITMASK{0x03};

        volatile Size & timer_counter;  // TCNTn
        Register output_compare_a;      // OCRnA
        Register output_compare_b;      // OCRnB
        Register config_a;              // TCCRnA
        Register config_b;              // TCCRnB
        Register interrupt_mask;        // TIMSKn
        Register interrupt_flag;        // TIFRn
        
    public:
        constexpr Timer(
            volatile Size & timer_counter,
            Register output_compare_a,
            Register output_compare_b,
            Register config_a,
            Register config_b,
            Register interrupt_mask,
            Register interrupt_flag
        ):
            timer_counter(timer_counter),
            output_compare_a(output_compare_a),
            output_compare_b(output_compare_b),
            config_a(config_a),
            config_b(config_b),
            interrupt_mask(interrupt_mask),
            interrupt_flag(interrupt_flag)
        {}

        void set_timer_length(Size duration) {
            output_compare_a = duration;
        }

        void set_prescale(Prescale scale) {
            config_b &= ~PRESCALE_BITMASK;
            config_b |= static_cast<uint8_t>(scale);
        }

        void set_mode(Mode mode) {
            config_a &= ~MODE_BITMASK;
            config_a |= static_cast<uint8_t>(mode);
        }
        
        void set_mask(bool compare_a, bool compare_b, bool overflow) {
            interrupt_mask = compare_b<<2 | compare_a<<1 | overflow;
        };
    };

    typedef Timer<uint8_t> Timer0;
    typedef Timer<uint16_t> Timer1;
}
#endif
