#include "daisy_seed.h"
#include "daisysp.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

Oscillator osc;

void MyCallback(AudioHandle::InterleavingInputBuffer  in,
                AudioHandle::InterleavingOutputBuffer out,
                size_t                    size)

{
    float osc_out = osc.Process();
    for(size_t i = 0; i < size; i++)
    {
        out[i] = osc_out;
        out[i + 1] = osc_out;
    }
}

int main(void)
{
    // Declare a variable to store the state we want to set for the LED.
    bool led_state;
    led_state = true;

    // Configure and Initialize the Daisy Seed
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    hardware.Configure();
    hardware.Init();
    double sr = hardware.AudioSampleRate();

    osc.Init(sr);
    osc.SetFreq(440);
    osc.SetAmp(1);
    hardware.StartAudio(MyCallback);

    // Loop forever
    for(;;)
    {
        // Set the onboard LED
        hardware.SetLed(led_state);

        // Toggle the LED state for the next time around.
        led_state = !led_state;

        // Wait 500ms
        System::Delay(500);
    }
}
