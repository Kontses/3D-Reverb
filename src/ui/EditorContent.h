#pragma once

#include "../PluginProcessor.h"
#include "Dial.h"
#include "FreezeButton.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class EditorContent final : public juce::Component
{
public:
    EditorContent (PluginProcessor& p, juce::UndoManager& um);

    void resized() override;
    bool keyPressed (const juce::KeyPress& k) override;

    // Add a public getter for sizeDial
    Dial& getSizeDial() { return sizeDial; }

    // Add a public getter for dampDial
    Dial& getDampDial() { return dampDial; }

    // Add a public getter for widthDial
    Dial& getWidthDial() { return widthDial; }

private:
    juce::AudioProcessorValueTreeState& apvts;

    Dial sizeDial;
    Dial dampDial;
    Dial widthDial;
    Dial mixDial;

    FreezeButton freezeButton;

    // Add a SliderAttachment for the sizeDial
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment;

    // Add a SliderAttachment for the dampDial
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampAttachment;

    // Add a SliderAttachment for the widthDial
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorContent)
};
