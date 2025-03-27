#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class CustomAudioVisualiser : public juce::AudioVisualiserComponent
{
public:
    CustomAudioVisualiser() : juce::AudioVisualiserComponent(2) // 2 channels for stereo
    {
        setBufferSize(2048);
        setSamplesPerBlock(1024);
        setColours(juce::Colour(0xff1a1a1a), juce::Colour(0xff00ff00));
        setRepaintRate(15);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        auto height = bounds.getHeight();
        auto width = bounds.getWidth();

        // Draw background grid
        g.fillAll(juce::Colour(0xff1a1a1a));

        // Draw horizontal grid lines
        g.setColour(juce::Colours::darkgrey);
        for (int i = 1; i < 10; ++i)
        {
            float y = (height * i) / 10.0f;
            g.drawHorizontalLine(y, 0, width);
        }

        // Let the base class draw the waveform
        AudioVisualiserComponent::paint(g);
    }
}; 