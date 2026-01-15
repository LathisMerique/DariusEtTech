#include "daisy_seed.h"
#include "daisysp.h"


using namespace daisy;
using namespace daisysp;

DaisySeed  hardware;
Oscillator osc;
Adsr       adsr;
bool       gate = false;

bool on = true;

void MyCallback(AudioHandle::InterleavingInputBuffer  in,
                AudioHandle::InterleavingOutputBuffer out,
                size_t                                size)
{
    for(size_t i = 0; i < size; i += 2)
    {
        float osc_out  = osc.Process();
        float adsr_out = adsr.Process(gate);
        if(on)
        {
            out[i]     = osc_out * adsr_out;
            out[i + 1] = osc_out * adsr_out;
        }
        else
        {
            out[i]     = osc_out;
            out[i + 1] = osc_out;
        }
    }
}


float midiNotes[]
    = {74, 0,  0,  0,  77, 0,  0,  74, 0,  74, 79, 0,  74, 0,  72, 0, 74, 0,
       0,  0,  81, 0,  0,  74, 0,  74, 82, 0,  81, 0,  77, 0,  74, 0, 81, 0,
       86, 74, 0,  72, 72, 0,  69, 0,  76, 0,  74, 74, 74, 74, 0,  0};


int main(void)
{
    hardware.Init();
    double sr = hardware.AudioSampleRate();
    osc.Init(sr);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    osc.SetAmp(1);
    adsr.Init(sr);
    adsr.SetAttackTime(0.01f);
    adsr.SetSustainLevel(0);
    adsr.SetDecayTime(0.1f);
    adsr.SetReleaseTime(0.1f);


    hardware.StartAudio(MyCallback);


    // ADC
    AdcChannelConfig adcConfig;
    adcConfig.InitSingle(hardware.GetPin(15));
    hardware.adc.Init(&adcConfig, 1);
    hardware.adc.Start();

    for(;;)
    {
        for(int index = 0; index < 53; index++)
        {
            float valPot = hardware.adc.GetFloat(0); //maj adc

            if(midiNotes[index] == midiNotes[index + 1] && index < 52)
            {
                on = false;
            }
            else
            {
                on = true;
            }

            int j = midiNotes[index];
            if(j != 0)
            {
                osc.SetAmp(0.5);
                adsr.Retrigger(true);
                osc.SetFreq(mtof(midiNotes[index]));
            }
            else
            {
                osc.SetAmp(0.0);
            }
            System::Delay(250 * valPot + 75);
        }
    }
}
