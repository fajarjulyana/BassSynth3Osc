#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BassSynth3OscAudioProcessor::BassSynth3OscAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      ),
      parameters (*this, nullptr, juce::Identifier("Parameters"),
                {std::make_unique<juce::AudioParameterFloat>(
                    "gain", "Gain", 0.0f, 1.0f, 0.5f)})
{
    // Initialize the synthesizer with 16 voices
    synth.clearVoices();
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new SynthVoice());

    synth.clearSounds();
    synth.addSound(new SynthSound());
}

BassSynth3OscAudioProcessor::~BassSynth3OscAudioProcessor()
{
}

//==============================================================================
const juce::String BassSynth3OscAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BassSynth3OscAudioProcessor::acceptsMidi() const
{
    return true;
}

bool BassSynth3OscAudioProcessor::producesMidi() const
{
    return false;
}

bool BassSynth3OscAudioProcessor::isMidiEffect() const
{
    return false;
}

double BassSynth3OscAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BassSynth3OscAudioProcessor::getNumPrograms()
{
    return 1;
}

int BassSynth3OscAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BassSynth3OscAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BassSynth3OscAudioProcessor::getProgramName (int index)
{
    return {};
}

void BassSynth3OscAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BassSynth3OscAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    lastSampleRate = sampleRate;
}

void BassSynth3OscAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool BassSynth3OscAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This is the function that's causing the assertion failure.
    // For synth plugins with only outputs, we need to make sure:
    // 1. We only care about output layout
    // 2. We accept mono or stereo output
    
    // We only support mono and stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && 
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
}

void BassSynth3OscAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Clear output buffer
    buffer.clear();
    
    // Update keyboard state
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    
    // Process synthesizer
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    // Apply gain
    float gainValue = *parameters.getRawParameterValue("gain");
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.applyGain(channel, 0, buffer.getNumSamples(), gainValue);
    }
}

//==============================================================================
bool BassSynth3OscAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BassSynth3OscAudioProcessor::createEditor()
{
    return new BassSynth3OscAudioProcessorEditor (*this);
}

//==============================================================================
void BassSynth3OscAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BassSynth3OscAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassSynth3OscAudioProcessor();
}

//==============================================================================
// Synth Sound implementation
BassSynth3OscAudioProcessor::SynthSound::SynthSound() {}

bool BassSynth3OscAudioProcessor::SynthSound::appliesToNote(int /*midiNoteNumber*/)
{
    return true;
}

bool BassSynth3OscAudioProcessor::SynthSound::appliesToChannel(int /*midiChannel*/)
{
    return true;
}

//==============================================================================
// Synth Voice implementation
BassSynth3OscAudioProcessor::SynthVoice::SynthVoice() : level(0.0), tailOff(0.0) {}

bool BassSynth3OscAudioProcessor::SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void BassSynth3OscAudioProcessor::SynthVoice::startNote(int midiNoteNumber, float velocity, 
                                                       juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    currentAngle = 0.0;
    level = velocity * 0.25f;
    tailOff = 0.0;
    
    auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    auto cyclesPerSample = cyclesPerSecond / getSampleRate();
    
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void BassSynth3OscAudioProcessor::SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        if (tailOff == 0.0)
            tailOff = 1.0;
    }
    else
    {
        clearCurrentNote();
        angleDelta = 0.0;
    }
}

void BassSynth3OscAudioProcessor::SynthVoice::pitchWheelMoved(int /*newValue*/)
{
    // Handle pitch wheel
}

void BassSynth3OscAudioProcessor::SynthVoice::controllerMoved(int /*controllerNumber*/, int /*newValue*/)
{
    // Handle controllers
}

void BassSynth3OscAudioProcessor::SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, 
                                                             int startSample, int numSamples)
{
    if (angleDelta != 0.0)
    {
        if (tailOff > 0.0)
        {
            // Fade out for note-off
            for (int i = 0; i < numSamples; ++i)
            {
                auto currentSample = (float) (std::sin(currentAngle) * level * tailOff);
                
                for (int j = outputBuffer.getNumChannels(); --j >= 0;)
                    outputBuffer.addSample(j, startSample + i, currentSample);
                
                currentAngle += angleDelta;
                tailOff *= 0.99;
                
                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else
        {
            // Regular playback
            for (int i = 0; i < numSamples; ++i)
            {
                auto currentSample = (float) (std::sin(currentAngle) * level);
                
                for (int j = outputBuffer.getNumChannels(); --j >= 0;)
                    outputBuffer.addSample(j, startSample + i, currentSample);
                
                currentAngle += angleDelta;
            }
        }
    }
}
