#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

class SpectrumAnalyzer : public juce::Component,
                        private juce::Timer
{
public:
    SpectrumAnalyzer() : forwardFFT(fftOrder),
                         window(fftSize, juce::dsp::WindowingFunction<float>::hann),
                         fifo(fftSize, 0.0f),
                         fftData(2 * fftSize, 0.0f),
                         scopeData(numPoints, 0.0f),
                         freqPoints(numPoints, 0.0f),
                         previousScope(numPoints, 0.0f)
    {
        setOpaque(true);
        startTimerHz(30);
        
        // Initialize the frequency scale points (logarithmic scale)
        for (int i = 0; i < numPoints; ++i)
        {
            float freq = 20.0f * std::pow(1000.0f, i / static_cast<float>(numPoints - 1));
            freqPoints[i] = freq;
        }
    }

    ~SpectrumAnalyzer() override
    {
        stopTimer();
        
        // Clear all buffers safely
        const juce::SpinLock::ScopedLockType lock(mutex);
        fifo.clear();
        fftData.clear();
        scopeData.clear();
        freqPoints.clear();
        previousScope.clear();
    }

    void pushBuffer(const float* data, int numSamples)
    {
        if (data != nullptr && numSamples > 0)
        {
            const juce::SpinLock::ScopedLockType lock(mutex);
            
            // Clear any old data first
            if (fifoIndex == 0)
            {
                std::fill(fifo.begin(), fifo.end(), 0.0f);
            }
            
            const int start1 = 0;
            const int size1 = juce::jmin(numSamples, fftSize - fifoIndex);
            const int size2 = numSamples - size1;

            if (size1 > 0)
                juce::FloatVectorOperations::copy(fifo.data() + fifoIndex, data + start1, size1);

            if (size2 > 0)
                juce::FloatVectorOperations::copy(fifo.data(), data + start1 + size1, size2);

            fifoIndex = (fifoIndex + numSamples) % fftSize;
            nextFFTBlockReady = true;
        }
    }

    void paint(juce::Graphics& g) override
    {
        const juce::SpinLock::ScopedLockType lock(mutex);
        
        g.fillAll(juce::Colour(0xff1a1a1a));
        
        const auto bounds = getLocalBounds().toFloat();
        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();

        // Draw grid with more subtle appearance
        g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
        
        // Vertical lines for frequencies
        const float freqs[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
        for (auto freq : freqs)
        {
            const float x = freqToX(freq, width);
            g.drawVerticalLine(static_cast<int>(x), 0.0f, height);
            
            // Draw frequency labels
            g.setColour(juce::Colours::grey.withAlpha(0.5f));
            const juce::String label = freq >= 1000 ? juce::String(freq/1000) + "k" : juce::String(freq);
            g.drawText(label, static_cast<int>(x - 20), static_cast<int>(height - 15), 40, 15, juce::Justification::centred);
            g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
        }
        
        // Horizontal lines for dB scale
        for (int db = -60; db <= 0; db += 12)
        {
            const float y = dbToY(static_cast<float>(db), height);
            g.drawHorizontalLine(static_cast<int>(y), 0.0f, width);
            
            // Draw dB labels
            g.setColour(juce::Colours::grey.withAlpha(0.5f));
            g.drawText(juce::String(db), 2, static_cast<int>(y - 8), 25, 15, juce::Justification::centred);
            g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
        }

        // Draw spectrum
        g.setColour(juce::Colour(0xff9400ff)); // Purple color
        
        juce::Path smoothPath;
        smoothPath.startNewSubPath(0.0f, height);

        // Create a temporary array for smoothed points
        std::vector<float> smoothedLevels(numPoints);
        
        // First pass: Calculate initial levels
        for (int i = 0; i < numPoints; ++i)
        {
            smoothedLevels[i] = juce::Decibels::gainToDecibels(scopeData[i] * 0.015f);
        }
        
        // Second pass: Apply gaussian smoothing
        const int smoothingRange = 5;  // Increased smoothing range
        std::vector<float> tempLevels = smoothedLevels;
        for (int i = 0; i < numPoints; ++i)
        {
            float sum = 0.0f;
            float weightSum = 0.0f;
            
            for (int j = -smoothingRange; j <= smoothingRange; ++j)
            {
                int index = i + j;
                if (index >= 0 && index < numPoints)
                {
                    // Gaussian weight
                    float weight = std::exp(-0.5f * (j * j) / (smoothingRange * smoothingRange));
                    sum += tempLevels[index] * weight;
                    weightSum += weight;
                }
            }
            
            smoothedLevels[i] = sum / weightSum;
        }

        // Draw the smoothed curve
        const float x0 = freqToX(freqPoints[0], width);
        const float level0 = juce::jlimit(-60.0f, 0.0f, smoothedLevels[0]);
        const float y0 = dbToY(level0, height);
        smoothPath.startNewSubPath(x0, y0);
        
        // Use more points for the curve
        for (int i = 1; i < numPoints; ++i)
        {
            const float x = freqToX(freqPoints[i], width);
            const float level = juce::jlimit(-60.0f, 0.0f, smoothedLevels[i]);
            const float y = dbToY(level, height);
            
            // Use quadratic curves for smoother interpolation
            if (i > 1)
            {
                const float prevX = freqToX(freqPoints[i-1], width);
                const float controlX = (x + prevX) * 0.5f;
                smoothPath.quadraticTo(controlX, y, x, y);
            }
            else
            {
                smoothPath.lineTo(x, y);
            }
        }
        
        // Complete the path
        smoothPath.lineTo(width, height);
        smoothPath.lineTo(0.0f, height);
        smoothPath.closeSubPath();
        
        // Fill with gradient
        juce::ColourGradient gradient(
            juce::Colour(0xff9400ff).withAlpha(0.8f), 0.0f, 0.0f,
            juce::Colour(0xff9400ff).withAlpha(0.2f), 0.0f, height,
            false);
        g.setGradientFill(gradient);
        g.fillPath(smoothPath);
        
        // Draw the line on top
        g.setColour(juce::Colour(0xff9400ff));
        g.strokePath(smoothPath, juce::PathStrokeType(2.0f));
    }

    void resized() override {}

private:
    static constexpr auto fftOrder = 13;           // 8192 points (increased from 11/2048)
    static constexpr auto fftSize = 1 << fftOrder; // 8192
    static constexpr auto numPoints = 1024;        // Points in our spectrum (increased from 512)

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    std::vector<float> fifo;
    std::vector<float> fftData;
    std::vector<float> scopeData;
    std::vector<float> freqPoints;
    std::vector<float> previousScope;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    
    juce::SpinLock mutex;

    void timerCallback() override
    {
        const juce::SpinLock::ScopedLockType lock(mutex);
        
        if (nextFFTBlockReady)
        {
            // Copy fifo into fftData array
            juce::FloatVectorOperations::copy(fftData.data(), fifo.data(), fftSize);
            
            // Clear the FIFO buffer after copying
            std::fill(fifo.begin(), fifo.end(), 0.0f);
            fifoIndex = 0;
            
            // Apply window to the data
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            
            // Perform FFT
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

            // Find maximum magnitude for normalization
            float maxMagnitude = 1.0e-10f;
            for (int i = 0; i < fftSize/2; ++i)
            {
                maxMagnitude = juce::jmax(maxMagnitude, fftData[i]);
            }
            
            // Convert to spectrum data with logarithmic frequency scale
            for (int i = 0; i < numPoints; ++i)
            {
                const float freq = freqPoints[i];
                const int fftIndex = static_cast<int>(freq * static_cast<float>(fftSize) / 44100.0f);
                
                if (fftIndex < fftSize/2)
                {
                    // Use wider range for averaging in lower frequencies
                    int range = 3;
                    if (freq < 100.0f)
                        range = 7;
                    else if (freq < 500.0f)
                        range = 5;
                    
                    float sum = 0.0f;
                    int count = 0;
                    
                    // Weighted average with gaussian window
                    for (int j = -range; j <= range; ++j)
                    {
                        const int index = fftIndex + j;
                        if (index >= 0 && index < fftSize/2)
                        {
                            float weight = std::exp(-0.5f * (j * j) / (range * range));
                            sum += (fftData[index] / maxMagnitude) * weight;
                            count += weight;
                        }
                    }
                    
                    float magnitude = (count > 0.0f) ? (sum / count) * 1.2f : 0.0f;
                    
                    // Apply spectral tilt (4.5 dB/octave around 1kHz)
                    const float tiltDB = 4.5f * std::log2(freq / 1000.0f);  // Tilt relative to 1kHz
                    const float tiltGain = std::pow(10.0f, tiltDB / 20.0f);  // Convert dB to gain
                    magnitude *= tiltGain;
                    
                    scopeData[i] = magnitude;
                }
            }
            
            nextFFTBlockReady = false;
        }
        else
        {
            // Clear all buffers when no new data
            std::fill(fifo.begin(), fifo.end(), 0.0f);
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            std::fill(scopeData.begin(), scopeData.end(), 0.0f);
            fifoIndex = 0;
        }
        
        repaint();
    }

    float freqToX(float freq, float width) const
    {
        // Convert frequency to x coordinate (logarithmic scale)
        return width * (std::log10(freq/20.0f) / std::log10(1000.0f));
    }

    float dbToY(float db, float height) const
    {
        // Convert dB to y coordinate
        return height * (1.0f - (db + 60.0f) / 60.0f);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
}; 