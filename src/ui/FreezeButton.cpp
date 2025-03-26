#include "FreezeButton.h"
#include "MyColours.h"
#include <BinaryData.h>

FreezeButton::FreezeButton (juce::RangedAudioParameter& param, juce::UndoManager* um)
    : audioParam (param)
    , paramAttachment (audioParam, [&] (float v) { updateState (static_cast<bool> (v)); }, um)
{
    setWantsKeyboardFocus (true);
    setRepaintsOnMouseActivity (true);
    setColour (onColourId, MyColours::blue);
    setColour (offColourId, MyColours::midGrey);
    setColour (focusColourId, MyColours::midGrey.brighter (0.25f));

    const auto svg = juce::Drawable::createFromImageData (BinaryData::FreezeIcon_svg, BinaryData::FreezeIcon_svgSize);
    jassert (svg != nullptr);

    if (svg != nullptr)
        iconPath = svg->getOutlineAsPath();

    paramAttachment.sendInitialUpdate();
}

void FreezeButton::resized()
{
    iconBounds = getLocalBounds().toFloat();
    iconPath.applyTransform (iconPath.getTransformToScaleToFit (iconBounds, true));
}

void FreezeButton::paint (juce::Graphics& g)
{
    g.setColour (findColour (state ? onColourId : hasKeyboardFocus (true) ? focusColourId : offColourId));
    g.fillPath (iconPath);
}

void FreezeButton::mouseDown (const juce::MouseEvent& e)
{
    juce::ignoreUnused (e);

    paramAttachment.setValueAsCompleteGesture (! state);

    const auto centre = iconBounds.getCentre();
    iconPath.applyTransform (juce::AffineTransform::scale (0.95f, 0.95f, centre.x, centre.y));
}

void FreezeButton::mouseUp (const juce::MouseEvent& e)
{
    juce::ignoreUnused (e);
    iconPath.applyTransform (iconPath.getTransformToScaleToFit (iconBounds, true));
}

void FreezeButton::focusGained (FocusChangeType cause)
{
    juce::ignoreUnused (cause);
    repaint();
}

void FreezeButton::focusLost (FocusChangeType cause)
{
    juce::ignoreUnused (cause);
    repaint();
}

bool FreezeButton::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::returnKey)
    {
        paramAttachment.setValueAsCompleteGesture (! state);
        return true;
    }

    return false;
}

void FreezeButton::updateState (bool newState)
{
    state = newState;
    repaint();
}
