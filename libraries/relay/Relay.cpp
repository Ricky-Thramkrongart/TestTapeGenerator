#include <relay.h>
    Relay::Relay(const uint8_t pin_, bool reverse_ = false, const uint8_t relay_delay_ = 100) : pin(pin_), reverse(reverse_), relay_delay(relay_delay_)
    {
        pinMode(pin, OUTPUT);
        enabled = true;
        digitalWrite(pin, reverse ? LOW : HIGH);
    }

    void Relay::Enable(void)
    {
        if (!enabled) {
            enabled = true;
            digitalWrite(pin, reverse ? LOW : HIGH);
            delay(relay_delay);
        }
    }

    void Relay::Disable(void)
    {
        if (enabled) {
            enabled = false;
            digitalWrite(pin, reverse ? HIGH : LOW);
            delay(relay_delay);
        }
    }

    bool Relay::IsEnabled(void)
    {
        return reverse ? !enabled : enabled;
    }
