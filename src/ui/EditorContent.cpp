#include "EditorContent.h"
#include "../ParamIDs.h"

EditorContent::EditorContent (PluginProcessor& p, juce::UndoManager& um)
    : apvts (p.getPluginState())
    , sizeDial (*apvts.getParameter (ParamIDs::size), &um)
    , dampDial (*apvts.getParameter (ParamIDs::damp), &um)
    , widthDial (*apvts.getParameter (ParamIDs::width), &um)
    , mixDial (*apvts.getParameter (ParamIDs::mix), &um)
    , freezeButton (*apvts.getParameter (ParamIDs::freeze), &um)
{
    setWantsKeyboardFocus (true);
    setFocusContainerType (FocusContainerType::keyboardFocusContainer);

    sizeDial.setExplicitFocusOrder (1);
    dampDial.setExplicitFocusOrder (2);
    freezeButton.setExplicitFocusOrder (3);
    widthDial.setExplicitFocusOrder (4);
    mixDial.setExplicitFocusOrder (5);

    addAndMakeVisible (sizeDial);
    addAndMakeVisible (dampDial);
    addAndMakeVisible (widthDial);
    addAndMakeVisible (mixDial);
    addAndMakeVisible (freezeButton);
}

void EditorContent::resized()
{
    const juce::Rectangle baseDialBounds { 0, 73, 80, 96 };
    sizeDial.setBounds (baseDialBounds.withX (46));
    dampDial.setBounds (baseDialBounds.withX (144));
    widthDial.setBounds (baseDialBounds.withX (342));
    mixDial.setBounds (baseDialBounds.withX (440));

    freezeButton.setBounds (259, 110, 48, 32);
}

bool EditorContent::keyPressed (const juce::KeyPress& k)
{
    if (k.isKeyCode (juce::KeyPress::tabKey) && hasKeyboardFocus (false))
    {
        sizeDial.grabKeyboardFocus();
        return true;
    }

    return false;
}
