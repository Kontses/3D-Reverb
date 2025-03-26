#include "EditorLnf.h"
#include "MyColours.h"
#include <BinaryData.h>

EditorLnf::EditorLnf()
{
    setDefaultSansSerifTypeface (
        juce::Typeface::createSystemTypefaceFor (BinaryData::UbuntuRegular_ttf, BinaryData::UbuntuRegular_ttfSize));
}

void EditorLnf::drawCornerResizer (juce::Graphics& g, int w, int h, bool isMouseOver, bool isMouseDragging)
{
    juce::ignoreUnused (isMouseDragging);

    const auto width = static_cast<float> (w);
    const auto height = static_cast<float> (h);
    const auto lineThickness = juce::jmin (width, height) * 0.07f;

    g.setColour (isMouseOver ? MyColours::blue : MyColours::blackGrey);

    for (float i = 0.0f; i < 1.0f; i += 0.3f)
    {
        g.drawLine (width * i, height + 1.0f, width + 1.0f, height * i, lineThickness);
        g.drawLine (width * i + lineThickness, height + 1.0f, width + 1.0f, height * i + lineThickness, lineThickness);
    }
}
