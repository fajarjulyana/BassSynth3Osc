#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BassSynth3OscAudioProcessorEditor::BassSynth3OscAudioProcessorEditor (BassSynth3OscAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      keyboardComponent (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Set up gain slider
    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    gainSlider.setValue(*p.parameters.getRawParameterValue("gain"));
    addAndMakeVisible(gainSlider);
    
    // Set up gain label
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, false);
    addAndMakeVisible(gainLabel);
    
    // Create slider attachment
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "gain", gainSlider);
    
    // Set up keyboard
    keyboardComponent.setAvailableRange(36, 84);
    keyboardComponent.setOctaveForMiddleC(4);
    addAndMakeVisible(keyboardComponent);
    
    // Set window size
    setSize (600, 300);
}

BassSynth3OscAudioProcessorEditor::~BassSynth3OscAudioProcessorEditor()
{
}

//==============================================================================
void BassSynth3OscAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Bass Synth 3 Osc", getLocalBounds().removeFromTop(30), 
                     juce::Justification::centred, 1);
}

void BassSynth3OscAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Top area for controls
    auto topArea = area.removeFromTop(160);
    
    // Gain slider
    gainSlider.setBounds(topArea.removeFromLeft(80).withTrimmedTop(20));
    
    // Keyboard at the bottom
    keyboardComponent.setBounds(area.removeFromBottom(80));
}
