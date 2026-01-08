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
                size_t                                size)

{
    for(size_t i = 0; i < size; i++)
    {
        float osc_out = osc.Process();
        out[i]        = osc_out;
        out[i + 1]    = osc_out;
    }
}

int main(void)
{
    hardware.Configure();
    hardware.Init();
    double sr = hardware.AudioSampleRate();

    osc.Init(sr);
    osc.SetFreq(440);
    osc.SetAmp(1);
    hardware.StartAudio(MyCallback);

    // Led led1;
    //Initialize led1. We'll plug it into pin 28.
    //false here indicates the value is uninverted
    //led1.Init(hardware.GetPin(28), false);

    //This is our ADC configuration
    AdcChannelConfig adcConfig;
    //Configure pin 21 as an ADC input. This is where we'll read the knob.
    adcConfig.InitSingle(hardware.GetPin(15));

    //Initialize the adc with the config we just made
    hardware.adc.Init(&adcConfig, 1);
    //Start reading values
    hardware.adc.Start();

    // Loop forever
    for(;;)

    {
        float value = hardware.adc.GetFloat(0);

        //Set the oscillator frequency based on the potentiometer value
        osc.SetFreq(220.f + (value * 880.f));

        System::Delay(1);
    }
}
