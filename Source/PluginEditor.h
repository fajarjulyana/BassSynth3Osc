#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class BassSynth3OscAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BassSynth3OscAudioProcessorEditor (BassSynth3OscAudioProcessor&);
    ~BassSynth3OscAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BassSynth3OscAudioProcessor& audioProcessor;

    juce::MidiKeyboardComponent keyboardComponent;
    
    juce::Slider gainSlider;
    juce::Label gainLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassSynth3OscAudioProcessorEditor)
};
