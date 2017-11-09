#ifndef PORTLIB_H
#define PORTLIB_H

#include <stdint.h>

namespace PortLib {
    template<typename RegSize>
    using Register = volatile RegSize &;

    using Register8 = Register<uint8_t>;
    using Register16 = Register<uint16_t>;

    typedef uint8_t PinIndex;

    struct DigitalPort {
    protected:
        // Members
        Register8 pinx, portx,  ddrx;

    public:
        // Constructors
        constexpr DigitalPort(
            Register8 pinx,
            Register8 portx,
            Register8 ddrx
        ) :
            pinx(pinx),
            portx(portx),
            ddrx(ddrx)
        {}

        // Accessors
        inline uint8_t get() const { return pinx; }
        inline bool get(PinIndex const index) { return pinx & (1<<index); }

        inline void set(uint8_t rhs) { portx = rhs; }
        inline void set_out()        { ddrx = 0xFF; }
        inline void set_in()         { ddrx = 0x00; }

        inline void set_out (PinIndex const index) { ddrx  |=  (1<<index); }
        inline void set_in  (PinIndex const index) { ddrx  &= ~(1<<index); }
        inline void set_high(PinIndex const index) { portx |=  (1<<index); }
        inline void set_low (PinIndex const index) { portx &= ~(1<<index); }
        inline void toggle  (PinIndex const index) { portx ^=  (1<<index); }

        inline void set(PinIndex const index, bool rhs) {
            if (rhs) set_high(index);
            else     set_low (index);
        }

        // Relationships
        friend class DigitalPin;
    };

    struct DigitalPin {
    protected:
        // Members
        DigitalPort &port;
        uint8_t bitmask;

    public:
        // Constructors
        constexpr DigitalPin(
            DigitalPort &port,
            PinIndex const index
        ) :
            port(port),
            bitmask(1<<index)
        {}

        // Accessors
        inline bool get() const { return (port.pinx & bitmask) != 0; }

        inline void set_out()  { port.ddrx  |=  bitmask; }
        inline void set_in()   { port.ddrx  &= ~bitmask; }
        inline void set_high() { port.portx |=  bitmask; }
        inline void set_low()  { port.portx &= ~bitmask; }
        inline void toggle()   { port.portx ^=  bitmask; }

        inline void set(bool rhs) {
            if (rhs) set_high();
            else     set_low ();
        }
    };

    template<typename TimerSize>
    struct Timer {
        using RegisterT = Register<TimerSize>;

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
            clear_on_compare,
            fast_pwm
        };

        enum struct Trigger {
            overflow  = 0b0000'0001,
            compare_a = 0b0000'0010,
            compare_b = 0b0000'0100
        };
            
    private:
        static uint8_t const PRESCALE_BITMASK{0b1111'1000};
        static uint16_t const MODE_BITMASK{0b0000'0011};

        RegisterT
            timer_counter,  // TCNTn
            compare_a, compare_b; // OCRnA, OCRnB

        Register8
            config_a, config_b,   // TCCRnA, TCCRnB
            interrupt_mask,       // TIMSKn
            interrupt_flag;       // TIFRn
        
    public:
        constexpr Timer(
            RegisterT timer_counter,
            RegisterT compare_a,
            RegisterT compare_b,
            Register8 config_a,
            Register8 config_b,
            Register8 interrupt_mask,
            Register8 interrupt_flag
        ):
            timer_counter(timer_counter),
            compare_a(compare_a), compare_b(compare_b),
            config_a(config_a), config_b(config_b),
            interrupt_mask(interrupt_mask),
            interrupt_flag(interrupt_flag)
        {}

        void set_timer_length(TimerSize duration) {
            compare_a = duration;
        }

        void set_prescale(Prescale scale) {
            config_b &= PRESCALE_BITMASK;
            config_b |= static_cast<uint8_t>(scale);
        }

        void set_mode(Mode mode) {
            config_a &= ~MODE_BITMASK;
            config_a |= static_cast<uint8_t>(mode);
        }
        
        void enable_interrupt(Trigger trigger) {
            interrupt_mask = static_cast<uint8_t>(trigger);
        }
    };

    using Timer0 = Timer<uint8_t>;
    using Timer1 = Timer<uint16_t>;

    struct RotaryEncoder {
        enum struct Active : uint8_t { A, B, BOTH, NONE };
        using Reader = bool (*)();

    private:
        struct State {
            bool a, b;
            
            constexpr State() : a(false), b(false) {}
            constexpr State(bool a, bool b) : a(a), b(b) {}

            inline bool unstable() { return a != b; }
            inline bool stable_on() { return a && b; }
            inline bool stable_off() { return !a && !b; }
        };

        State previous, current;
        Reader read_a;
        Reader read_b;

    public:
        constexpr RotaryEncoder(
            Reader a,
            Reader b
        ) :
            read_a(a),
            read_b(b)
        {}

        inline void update() {
            previous = current;
            current = { read_a(), read_b() };
        }

        inline bool rising_edge() {
            return previous.stable_off() && current.unstable();
        }

        inline bool falling_edge() {
            return previous.stable_on() && current.unstable();
        }

        inline Active get_active() {
            if (current.a) {
                if (current.b) return Active::BOTH;
                else           return Active::A;
            } else {
                if (current.b) return Active::B;
                else           return Active::NONE;
            }
        }
    };
}
#endif
