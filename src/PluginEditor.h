#pragma once

#include "PluginProcessor.h"
#include "ui/EditorContent.h"
#include "ui/EditorLnf.h"
#include "ui/NumericInputFilter.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class PluginEditor final : public juce::AudioProcessorEditor, public juce::TextEditor::Listener
{
public:
    PluginEditor (PluginProcessor& p, juce::UndoManager& um);
    ~PluginEditor(); 

    void resized() override;
    void paint (juce::Graphics& g) override;
    void textEditorTextChanged (juce::TextEditor& editor) override;

    bool keyPressed (const juce::KeyPress& k) override;

private:
    PluginProcessor& processor;
    juce::UndoManager& undoManager;

    EditorContent editorContent;

    static constexpr auto defaultWidth { 560 };
    static constexpr auto defaultHeight { 360 };

    struct SharedLnf
    {
        SharedLnf() { juce::LookAndFeel::setDefaultLookAndFeel (&editorLnf); }
        ~SharedLnf() { juce::LookAndFeel::setDefaultLookAndFeel (nullptr); }

        EditorLnf editorLnf;
    };

    juce::SharedResourcePointer<SharedLnf> lnf;

    // Declare the text boxes
    juce::TextEditor textBox1;
    juce::TextEditor textBox2;
    juce::TextEditor textBox3;

    // Declare the labels
    juce::Label label1;
    juce::Label label2;
    juce::Label label3;

    // Declare the additional labels
    juce::Label unitLabel1;
    juce::Label unitLabel2;
    juce::Label unitLabel3;

    // Declare the input filter
    NumericInputFilter numericInputFilter { 0.0f, 100.0f, 2 };

    // Add a reference to the dampDial
    Dial& dampDial;
    Dial& sizeDial;
    Dial& widthDial;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
