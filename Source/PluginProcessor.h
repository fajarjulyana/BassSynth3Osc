#pragma once

#include <JuceHeader.h>

//==============================================================================
class BassSynth3OscAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    BassSynth3OscAudioProcessor();
    ~BassSynth3OscAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Public members for editor
    juce::MidiKeyboardState keyboardState;
    juce::AudioProcessorValueTreeState parameters;

    // Custom synth sound class
    class SynthSound : public juce::SynthesiserSound
    {
    public:
        SynthSound();
        bool appliesToNote(int midiNoteNumber) override;
        bool appliesToChannel(int midiChannel) override;
    };

    // Custom synth voice class
    class SynthVoice : public juce::SynthesiserVoice
    {
    public:
        SynthVoice();
        bool canPlaySound(juce::SynthesiserSound* sound) override;
        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
        void stopNote(float velocity, bool allowTailOff) override;
        void pitchWheelMoved(int newValue) override;
        void controllerMoved(int controllerNumber, int newValue) override;
        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    
    private:
        double currentAngle = 0.0;
        double angleDelta = 0.0;
        float level = 0.0f;
        double tailOff = 0.0;
    };

private:
    //==============================================================================
    juce::Synthesiser synth;
    double lastSampleRate = 0.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassSynth3OscAudioProcessor)
};
