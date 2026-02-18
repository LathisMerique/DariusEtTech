#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed  hardware;
Oscillator osc;
Adsr       adsr;

// REMPLACEMENT : On utilise Svf (State Variable Filter) qui est standard
Svf        flt; 

bool       gate = false;
bool       on = true;

void MyCallback(AudioHandle::InterleavingInputBuffer  in,
                AudioHandle::InterleavingOutputBuffer out,
                size_t                                size)
{
    float sig, env_out;

    for(size_t i = 0; i < size; i += 2)
    {
        // 1. Oscillateur & Enveloppe
        sig = osc.Process();
        env_out = adsr.Process(gate);

        // 2. LOGIQUE AXEL F (Adaptée pour Svf)
        // Le filtre s'ouvre avec l'enveloppe
        float cutoff = 500.0f + (6000.0f * env_out);

        flt.SetFreq(cutoff);
        flt.Process(sig); // On traite le signal

        // IMPORTANT : Avec Svf, on choisit la sortie. 
        // Low() = Passe-bas (le son lourd et étouffé typique)
        sig = flt.Low(); 

        // 3. VCA (Volume)
        if(on)
        {
            sig *= env_out;
        }
        else
        {
            sig = 0.0f;
        }

        // Sortie Stéréo (sans réverb pour l'instant pour garantir la compil)
        out[i]     = sig;
        out[i + 1] = sig;
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

    // Config Oscillateur
    osc.Init(sr);
    osc.SetWaveform(Oscillator::WAVE_SAW); // Dents de scie
    osc.SetAmp(0.7f); 

    // Config Enveloppe (Snappy !)
    adsr.Init(sr);
    adsr.SetAttackTime(0.01f);
    adsr.SetDecayTime(0.12f);
    adsr.SetSustainLevel(0.4f);
    adsr.SetReleaseTime(0.1f);

    // Config Filtre Svf
    flt.Init(sr);
    flt.SetRes(0.3f); // Résonance pour le côté "synthé"
    flt.SetDrive(0.2f); // Un peu de saturation si dispo, sinon ignoré

    hardware.StartAudio(MyCallback);

    // Configuration des potards (ADC) 
    AdcChannelConfig adcConfig[2];
    // Potentiomètre 1 sur pin 15
    adcConfig[0].InitSingle(hardware.GetPin(15));
    // Potentiomètre 2 sur pin 16 
    adcConfig[1].InitSingle(hardware.GetPin(16));

    hardware.adc.Init(adcConfig, 2); 
    hardware.adc.Start();

    for(;;)
    {
        for(int index = 0; index < 53; index++)
        {
            float valPot = hardware.adc.GetFloat(0);
            float valPotSynth = hardware.adc.GetFloat(1);
            flt.SetRes(valPotSynth); // Résonance pour le côté "synthé"

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
                osc.SetAmp(0.5f);
                adsr.Retrigger(true);
                osc.SetFreq(mtof(midiNotes[index]));
            }
            else
            {
                osc.SetAmp(0.0f);
            }

            System::Delay(250 * valPot + 75);
        }
    }
}