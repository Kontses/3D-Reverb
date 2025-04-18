#include "PluginProcessor.h"
#include "ParamIDs.h"
#include "PluginEditor.h"

#ifndef JucePlugin_Name
#define JucePlugin_Name "3D Reverb"
#endif

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto percentageAttributes = juce::AudioParameterFloatAttributes().withStringFromValueFunction (
        // Format the number to always display three digits like "0.01 %", "10.0 %", "100 %".
        [] (auto value, auto)
        {
            constexpr auto unit = " %";

            if (auto v { std::round (value * 100.0f) / 100.0f }; v < 10.0f)
                return juce::String { v, 2 } + unit;

            if (auto v { std::round (value * 10.0f) / 10.0f }; v < 100.0f)
                return juce::String { v, 1 } + unit;

            return juce::String { std::round (value) } + unit;
        });

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::size, 1 },
                                                             ParamIDs::size,
                                                             juce::NormalisableRange { 0.0f, 100.0f, 0.01f, 1.0f },
                                                             50.0f,
                                                             percentageAttributes));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::damp, 1 },
                                                             ParamIDs::damp,
                                                             juce::NormalisableRange { 0.0f, 100.0f, 0.01f, 1.0f },
                                                             50.0f,
                                                             percentageAttributes));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::width, 1 },
                                                             ParamIDs::width,
                                                             juce::NormalisableRange { 0.0f, 100.0f, 0.01f, 1.0f },
                                                             50.0f,
                                                             percentageAttributes));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::mix, 1 },
                                                             ParamIDs::mix,
                                                             juce::NormalisableRange { 0.0f, 100.0f, 0.01f, 1.0f },
                                                             50.0f,
                                                             percentageAttributes));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::freeze, 1 }, ParamIDs::freeze, false));

    return layout;
}

PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    , apvts (*this, &undoManager, "Parameters", createParameterLayout())
{
    auto castParameter = [this](juce::StringRef paramID, auto& destination)
    {
        destination = dynamic_cast<std::remove_reference_t<decltype(destination)>>(apvts.getParameter(paramID));
        jassert(destination != nullptr);
    };

    castParameter (ParamIDs::size, size);
    castParameter (ParamIDs::damp, damp);
    castParameter (ParamIDs::width, width);
    castParameter (ParamIDs::mix, mix);
    castParameter (ParamIDs::freeze, freeze);

    // Initialize parameter change tracking
    lastSize = size->get() * 0.01f;
    lastDamp = damp->get() * 0.01f;
    lastWidth = width->get() * 0.01f;
    lastMix = mix->get() * 0.01f;
    lastFreeze = freeze->get();
}

PluginProcessor::~PluginProcessor()
{
    // Destructor implementation
}

const juce::String PluginProcessor::getName() const { return JucePlugin_Name; }

bool PluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const { return 0.0; }

int PluginProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram() { return 0; }

void PluginProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());

    reverb.prepare (spec);
    analyzer.setSampleRate(static_cast<float>(sampleRate));
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void PluginProcessor::updateReverbParams()
{
    const float currentSize = size->get() * 0.01f;
    const float currentDamp = damp->get() * 0.01f;
    const float currentWidth = width->get() * 0.01f;
    const float currentMix = mix->get() * 0.01f;
    const bool currentFreeze = freeze->get();

    // Only update parameters if they've changed
    if (currentSize != lastSize || currentDamp != lastDamp || 
        currentWidth != lastWidth || currentMix != lastMix || 
        currentFreeze != lastFreeze)
    {
        params.roomSize = currentSize;
        params.damping = currentDamp;
        params.width = currentWidth;
        params.wetLevel = currentMix;
        params.dryLevel = 1.0f - currentMix;
        params.freezeMode = currentFreeze;

        reverb.setParameters(params);

        // Update last values
        lastSize = currentSize;
        lastDamp = currentDamp;
        lastWidth = currentWidth;
        lastMix = currentMix;
        lastFreeze = currentFreeze;
    }
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    updateReverbParams();

    // Process reverb with proper buffer handling
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing ctx (block);
    
    // Ensure buffer is properly sized
    if (block.getNumSamples() > 0)
    {
        reverb.process (ctx);
    }

    // Push audio data to analyzer only if we have valid data
    if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0)
    {
        analyzer.pushBuffer(buffer.getReadPointer(0), buffer.getNumSamples());
    }
}

bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor() { return new PluginEditor (*this, undoManager); }

void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
    apvts.state.writeToStream (mos);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (const auto tree = juce::ValueTree::readFromData (data, static_cast<size_t> (sizeInBytes)); tree.isValid())
        apvts.replaceState (tree);
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PluginProcessor(); }
