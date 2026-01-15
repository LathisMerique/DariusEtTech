#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed  hardware;
Oscillator osc;
void       MyCallback(AudioHandle::InterleavingInputBuffer  in,
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

#define do 65.4064
#define doD 69.2957
#define re 73.4162
#define reD 77.7817
#define mi 82.4069
#define fa 87.3071
#define faD 92.4986
#define sol 97.9989
#define solD 103.826
#define la 110
#define laD 116.541
#define si 123.471
float notes[] = {fa, la, fa, fa, si, fa, mi}; // nom de la note
float numNt[] = {3, 3, 3, 3, 3, 3, 3};        // numéro de la note
float duree[] = {0.25f, 0.125f, 0.125f, 0.0625f, 0.125f, 0.125f, 0.125f};
// durées notes individuelles
float freqNt(int indx)
{
    return notes[indx] * pow(2, (numNt[indx] - 1));
}

int main(void)
{
    hardware.Init();
    double sr = hardware.AudioSampleRate();
    osc.Init(sr);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    //osc.SetFreq(440);
    osc.SetAmp(1);
    hardware.StartAudio(MyCallback);
    // Loop forever
    for(;;)
    {
        for(int index = 0; index < 7; index++)
        {
            //osc.SetWaveform(Oscillator::WAVE_SAW);
            osc.SetFreq(freqNt(index));
            //hardware.StartAudio(MyCallback);
            System::Delay((float)3000.00 * (float)duree[index]);
        }
    }
}
