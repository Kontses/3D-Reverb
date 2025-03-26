#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class NumericInputFilter : public juce::TextEditor::InputFilter
{
public:
    NumericInputFilter(float minValue, float maxValue, int decimalPlaces)
        : minValue(minValue), maxValue(maxValue), decimalPlaces(decimalPlaces) {}

    juce::String filterNewText(juce::TextEditor& editor, const juce::String& newInput) override
    {
        
        juce::String allowedChars = "0123456789.";
        juce::String result;

        for (auto c : newInput)
        {
            if (allowedChars.containsChar(c))
                result += c;
        }

        return result;
    }

    bool isValidInput(const juce::String& text)
    {
        if (text.isEmpty())
            return true;

        auto value = text.getFloatValue();
        if (value < minValue || value > maxValue)
            return false;

        auto dotIndex = text.indexOfChar('.');
        if (dotIndex != -1 && text.length() - dotIndex - 1 > decimalPlaces)
            return false;

        return true;
    }

private:
    float minValue;
    float maxValue;
    int decimalPlaces;
};
