#include "PluginEditor.h"
#include "PluginProcessor.h"

PluginEditor::PluginEditor (PluginProcessor& p, juce::UndoManager& um)
    : AudioProcessorEditor (&p)
    , processor (p)
    , undoManager (um)
    , editorContent (p, um)
    , dampDial (editorContent.getDampDial())    // Use the getter method to access dampDial
    , sizeDial (editorContent.getSizeDial())    // Use the getter method to access sizeDial
    , widthDial (editorContent.getWidthDial())  // Use the getter method to access widthDial
{
    constexpr auto ratio = static_cast<double> (defaultWidth) / defaultHeight;
    setResizable (false, true);
    getConstrainer()->setFixedAspectRatio (ratio);
    getConstrainer()->setSizeLimits (defaultWidth, defaultHeight, defaultWidth * 2, defaultHeight * 2);
    setSize (defaultWidth, defaultHeight);
    editorContent.setSize (defaultWidth, defaultHeight);

    addAndMakeVisible (editorContent);
    addAndMakeVisible (processor.getAnalyzer());

    // Initialize the text boxes
    textBox1.setMultiLine (false);
    textBox2.setMultiLine (false);
    textBox3.setMultiLine (false);

    // Set the input filter for the text boxes
    textBox1.setInputFilter (&numericInputFilter, true);
    textBox2.setInputFilter (&numericInputFilter, true);
    textBox3.setInputFilter (&numericInputFilter, true);

    // Add the text editor listener
    textBox1.addListener (this);
    textBox2.addListener (this);
    textBox3.addListener (this);

    // Initialize the labels
    label1.setText ("Height:", juce::dontSendNotification);
    label2.setText ("Length:", juce::dontSendNotification);
    label3.setText ("Width:", juce::dontSendNotification);
    label1.setColour (juce::Label::textColourId, juce::Colour (0xfff6f9e4));
    label2.setColour (juce::Label::textColourId, juce::Colour (0xfff6f9e4));
    label3.setColour (juce::Label::textColourId, juce::Colour (0xfff6f9e4));

    // Initialize the additional labels
    unitLabel1.setText ("0 - 100 meters", juce::dontSendNotification);
    unitLabel2.setText ("0 - 100 meters", juce::dontSendNotification);
    unitLabel3.setText ("0 - 100 meters", juce::dontSendNotification);
    unitLabel1.setColour (juce::Label::textColourId, juce::Colours::grey);
    unitLabel2.setColour (juce::Label::textColourId, juce::Colours::grey);
    unitLabel3.setColour (juce::Label::textColourId, juce::Colours::grey);

    // Set a smaller font size for the unit labels
    unitLabel1.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 12.0f, juce::Font::plain));
    unitLabel2.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 12.0f, juce::Font::plain));
    unitLabel3.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 12.0f, juce::Font::plain));

    // Set the minimum horizontal scale to prevent text wrapping
    unitLabel1.setMinimumHorizontalScale (0.5f);
    unitLabel2.setMinimumHorizontalScale (0.5f);
    unitLabel3.setMinimumHorizontalScale (0.5f);

    // Add the text boxes to the editor's component tree
    addAndMakeVisible (textBox1);
    addAndMakeVisible (textBox2);
    addAndMakeVisible (textBox3);

    // Add the labels to the editor's component tree
    addAndMakeVisible (label1);
    addAndMakeVisible (label2);
    addAndMakeVisible (label3);

    // Add the additional labels to the editor's component tree
    addAndMakeVisible (unitLabel1);
    addAndMakeVisible (unitLabel2);
    addAndMakeVisible (unitLabel3);
}

void PluginEditor::paint (juce::Graphics& g) 
{ 
    g.fillAll (MyColours::black);
}

void PluginEditor::resized()
{
    const auto factor = static_cast<float> (getWidth()) / defaultWidth;
    editorContent.setTransform (juce::AffineTransform::scale (factor));

    // Calculate positions
    const int visualizerHeight = 250;
    const int spacing = 20;
    const int contentHeight = 180;
    const int textBoxesHeight = 120;  // Total height needed for text boxes
    
    // Calculate total available height
    const int totalNeededHeight = visualizerHeight + spacing + contentHeight + textBoxesHeight + spacing;
    
    // Adjust window height if needed
    if (getHeight() < totalNeededHeight)
    {
        setSize(getWidth(), totalNeededHeight);
    }
    
    // Position the visualizer with some margin from top
    processor.getAnalyzer().setBounds(0, spacing, getWidth(), visualizerHeight);

    // Position the editor content (knobs) below the visualizer with spacing
    editorContent.setBounds(0, visualizerHeight + spacing, getWidth(), contentHeight);

    // Set the bounds of the text boxes and labels
    const int labelWidth = 80;
    const int textBoxHeight = 30;
    const int textBoxSpacing = 10;
    const int unitLabelWidth = 80;
    const int textBoxY = getHeight() - textBoxesHeight - spacing;

    label1.setBounds (10, textBoxY, labelWidth, textBoxHeight);
    textBox1.setBounds (10 + labelWidth + 10, textBoxY, getWidth() - labelWidth - unitLabelWidth - 40, textBoxHeight);
    unitLabel1.setBounds (textBox1.getRight() - unitLabelWidth - 5, textBoxY, unitLabelWidth, textBoxHeight);

    label2.setBounds (10, textBoxY + textBoxHeight + textBoxSpacing, labelWidth, textBoxHeight);
    textBox2.setBounds (10 + labelWidth + 10, textBoxY + textBoxHeight + textBoxSpacing, getWidth() - labelWidth - unitLabelWidth - 40, textBoxHeight);
    unitLabel2.setBounds (textBox2.getRight() - unitLabelWidth - 5, textBoxY + textBoxHeight + textBoxSpacing, unitLabelWidth, textBoxHeight);

    label3.setBounds (10, textBoxY + 2 * (textBoxHeight + textBoxSpacing), labelWidth, textBoxHeight);
    textBox3.setBounds (10 + labelWidth + 10, textBoxY + 2 * (textBoxHeight + textBoxSpacing), getWidth() - labelWidth - unitLabelWidth - 40, textBoxHeight);
    unitLabel3.setBounds (textBox3.getRight() - unitLabelWidth - 5, textBoxY + 2 * (textBoxHeight + textBoxSpacing), unitLabelWidth, textBoxHeight);
}

void PluginEditor::textEditorTextChanged (juce::TextEditor& editor) 
{
    if (! numericInputFilter.isValidInput (editor.getText()))
    {
        editor.undo();
    }
    else if (&editor == &textBox1)
    {
        dampDial.setParameterUpdatesEnabled(false);
        processor.damp->setValueNotifyingHost (editor.getText().getFloatValue() / 100.0f);
        dampDial.setParameterUpdatesEnabled(true);
    }
    else if (&editor == &textBox2)
    {
        sizeDial.setParameterUpdatesEnabled(false);
        processor.size->setValueNotifyingHost (editor.getText().getFloatValue() / 100.0f);
        sizeDial.setParameterUpdatesEnabled(true);
    }
    else if (&editor == &textBox3)
    {
        widthDial.setParameterUpdatesEnabled(false);
        processor.width->setValueNotifyingHost (editor.getText().getFloatValue() / 100.0f);
        widthDial.setParameterUpdatesEnabled(true);
    }
}

bool PluginEditor::keyPressed (const juce::KeyPress& k)
{
    if (k.isKeyCode ('Z') && k.getModifiers().isCommandDown())
    {
        if (k.getModifiers().isShiftDown())
            undoManager.redo();
        else
            undoManager.undo();

        return true;
    }

    return false;
}

PluginEditor::~PluginEditor() {}