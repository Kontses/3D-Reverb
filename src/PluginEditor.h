#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/EditorContent.h"
#include "ui/MyColours.h"
#include "ui/CustomLookAndFeel.h"

class PluginEditor : public juce::AudioProcessorEditor,
                    private juce::TextEditor::Listener
{
public:
    PluginEditor (PluginProcessor&, juce::UndoManager&);
    ~PluginEditor() override
    {
        // Remove listeners before destruction
        textBox1.removeListener(this);
        textBox2.removeListener(this);
        textBox3.removeListener(this);
        
        // Remove components from the editor
        removeChildComponent(&editorContent);
        removeChildComponent(&processor.getAnalyzer());
        
        // Clear any references
        dampDial.setLookAndFeel(nullptr);
        sizeDial.setLookAndFeel(nullptr);
        widthDial.setLookAndFeel(nullptr);
    }

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    void textEditorTextChanged (juce::TextEditor&) override;

    static constexpr int defaultWidth = 800;
    static constexpr int defaultHeight = 500;

    PluginProcessor& processor;
    juce::UndoManager& undoManager;
    EditorContent editorContent;

    juce::TextEditor textBox1, textBox2, textBox3;
    juce::Label label1, label2, label3;
    juce::Label unitLabel1, unitLabel2, unitLabel3;
    juce::NumericInputFilter numericInputFilter;

    // References to the dials
    juce::Slider& dampDial;
    juce::Slider& sizeDial;
    juce::Slider& widthDial;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
