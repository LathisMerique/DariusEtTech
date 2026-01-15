#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
DaisySeed hardware;

Oscillator osc;

void MyCallback(AudioHandle::InterleavingInputBuffer  in,
                AudioHandle::InterleavingOutputBuffer out,
                size_t                                size)

{
    for(size_t i = 0; i < size; i += 2)
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
}
