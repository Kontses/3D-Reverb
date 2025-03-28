#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/EditorContent.h"
#include "ui/MyColours.h"
#include "ui/NumericInputFilter.h"

class PluginEditor : public juce::AudioProcessorEditor,
                    private juce::TextEditor::Listener
{
public:
    PluginEditor (PluginProcessor&, juce::UndoManager&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    void textEditorTextChanged (juce::TextEditor&) override;

    static constexpr int defaultWidth = 600;
    static constexpr int defaultHeight = 500;

    PluginProcessor& processor;
    juce::UndoManager& undoManager;
    EditorContent editorContent;

    juce::TextEditor textBox1, textBox2, textBox3;
    juce::Label label1, label2, label3;
    juce::Label unitLabel1, unitLabel2, unitLabel3;
    NumericInputFilter numericInputFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
