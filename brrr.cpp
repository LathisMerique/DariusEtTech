#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hardware;

// Modules Audio pour le son Jupiter-8
Oscillator osc1; // Oscillateur principal
Oscillator osc2; // Oscillateur secondaire (pour le "Detune")
Svf        flt;  // Filtre (State variable filter)
Adsr       adsr; // Enveloppe

bool gate = false;
bool on   = true;

// Variables globales pour stocker la valeur des potentiomètres
float p_cutoff  = 0.0f;
float p_res     = 0.0f;
float p_env_amt = 0.0f;
float p_detune  = 0.0f;

void MyCallback(AudioHandle::InterleavingInputBuffer  in,
                AudioHandle::InterleavingOutputBuffer out,
                size_t                                size)
{
    float sig, sig1, sig2, env_out;

    for(size_t i = 0; i < size; i += 2)
    {
        // 1. Calcul de l'enveloppe
        env_out = adsr.Process(gate);

        // 2. Génération des deux oscillateurs (Dual VCO)
        sig1 = osc1.Process();
        sig2 = osc2.Process();

        // Mixage des deux oscillateurs (0.5 chacun pour éviter la saturation)
        // C'est ici que se crée l'effet "Gros son analogique"
        sig = (sig1 + sig2) * 0.5f;

        // 3. Gestion du Filtre (VCF) façon Jupiter
        // La fréquence de coupure est définie par le potentiomètre + l'enveloppe
        float cut = p_cutoff * 10000.0f;           // Base (0Hz à 10kHz)
        float mod = env_out * p_env_amt * 8000.0f; // Modulation par l'enveloppe

        // On additionne et on limite pour rester dans des fréquences audibles
        float final_freq = cut + mod;
        if(final_freq > 12000.0f)
            final_freq = 12000.0f;
        if(final_freq < 20.0f)
            final_freq = 20.0f;

        flt.SetFreq(final_freq);
        flt.SetRes(p_res); // Résonance contrôlée par le potard

        flt.Process(sig); // Traitement du signal par le filtre

        // On récupère la sortie "Low Pass" (Passe-bas) pour le son chaud
        sig = flt.Low();

        // 4. VCA (Volume final)
        if(on)
        {
            sig *= env_out;
        }
        else
        {
            sig = 0.0f;
        }

        // Sortie Stéréo
        out[i]     = sig;
        out[i + 1] = sig;
    }
}

float midiNotes[] = {74, 0,  62, 0, 77, 0, 48, 74, 0, 74, 79, 0, 74, 0, 72, 0,
                     74, 0,  50, 0, 81, 0, 62, 74, 0, 74, 82, 0, 81, 0, 77, 0,
                     62, 0,  81, 0, 86, 0, 74, 72, 0, 72, 69, 0, 76, 0, 74, 74,
                     74, 74, 74, 0, 0,  0, 0,  0,  0, 60, 57, 0, 55, 0, 53, 0};

int main(void)
{
    hardware.Init();
    double sr = hardware.AudioSampleRate();

    // Init Osc 1
    osc1.Init(sr);
    osc1.SetWaveform(
        Oscillator::WAVE_SAW); // Dents de scie (Classique Jupiter W)
    osc1.SetAmp(0.6f);

    // Init Osc 2 (Shadow oscillator)
    osc2.Init(sr);
    osc2.SetWaveform(Oscillator::WAVE_SAW);
    osc2.SetAmp(0.6f);

    // Init Filtre
    flt.Init(sr);
    flt.SetDrive(0.3f); // Ajoute un peu de saturation

    // Init Enveloppe
    adsr.Init(sr);
    adsr.SetAttackTime(0.01f);
    adsr.SetDecayTime(0.2f);
    adsr.SetSustainLevel(0.4f);
    adsr.SetReleaseTime(0.2f);

    hardware.StartAudio(MyCallback);

    // Configuration ADC (Potentiomètres)
    // Nous avons besoin de 5 entrées (1 Tempo + 4 Son)
    AdcChannelConfig adcConfig[5];
    adcConfig[0].InitSingle(hardware.GetPin(15)); // Tempo
    adcConfig[1].InitSingle(hardware.GetPin(16)); // Cutoff
    adcConfig[2].InitSingle(hardware.GetPin(17)); // Resonance
    adcConfig[3].InitSingle(hardware.GetPin(18)); // Env Amount
    adcConfig[4].InitSingle(hardware.GetPin(19)); // Detune

    hardware.adc.Init(adcConfig, 5);
    hardware.adc.Start();
    int Nb_Midi = sizeof(midiNotes) / sizeof(midiNotes[0]);

    for(;;)
    {
        for(int index = 0; index < Nb_Midi; index++)
        {
            // Lecture des Potentiomètres
            float valTempo = hardware.adc.GetFloat(0);
            p_cutoff       = hardware.adc.GetFloat(1);
            p_res          = hardware.adc.GetFloat(2);
            p_env_amt      = hardware.adc.GetFloat(3);
            p_detune       = hardware.adc.GetFloat(4);

            /* logique de lecture
            if(index < Nb_Midi - 1 && midiNotes[index] == midiNotes[index + 1])
            {
                on = false;
            }
            else
            {
                on = true;
            } */

            int j = midiNotes[index];
            if(j != 0)
            {
                float baseFreq = mtof(midiNotes[index]);

                adsr.Retrigger(true);

                // Réglage des oscillateurs
                osc1.SetFreq(baseFreq);

                // L'Oscillateur 2 est légèrement décalé selon le potard "Detune"
                // Cela crée l'effet de battement typique des années 80
                float detuneFactor
                    = 1.0f + (p_detune * 0.02f); // Max 2% de décalage
                osc2.SetFreq(baseFreq * detuneFactor);
            }

            // Délai contrôlé par le potard de tempo
            System::Delay(250 * valTempo + 75);
        }
    }
}