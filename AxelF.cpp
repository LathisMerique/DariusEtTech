#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;


DaisySeed hardware;

Oscillator osc;

Svf filter;


void MyCallback(AudioHandle::InterleavingInputBuffer  in,
                AudioHandle::InterleavingOutputBuffer out,
                size_t                                size)

{
    for(size_t i = 0; i < size; i += 2)
    {
        float osc_out = osc.Process();
        filter.Process(osc_out);
        float filtered = filter.Low();

        out[i]     = filtered;
        out[i + 1] = filtered;
    }
}

int main(void)
{
    hardware.Configure();
    hardware.Init();
    double sr = hardware.AudioSampleRate();

    osc.Init(sr);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    osc.SetFreq(440);
    osc.SetAmp(1);

    filter.Init(sr);
    filter.SetFreq(2000.0f);
    filter.SetRes(0.6f);

    for(;;)
    {
        hardware.StartAudio(MyCallback);
        System::Delay(1000);
        hardware.StopAudio();
        System::Delay(1000);
    }
}
