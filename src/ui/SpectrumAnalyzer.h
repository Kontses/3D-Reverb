#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

class SpectrumAnalyzer : public juce::Component,
                        private juce::Timer
{
public:
    SpectrumAnalyzer() : forwardFFT(fftOrder),
                         window(fftSize, juce::dsp::WindowingFunction<float>::hann),
                         sampleRate(44100.0f)  // Default sample rate
    {
        // Initialize vectors with proper size and values
        fifo.resize(fftSize, 0.0f);
        fftData.resize(2 * fftSize, 0.0f);
        scopeData.resize(numPoints, 0.0f);
        freqPoints.resize(numPoints, 0.0f);
        previousScope.resize(numPoints, 0.0f);

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
        // Stop timer before cleanup
        stopTimer();
        
        // Acquire lock to ensure no timer callback is running
        const juce::SpinLock::ScopedLockType lock(mutex);
        
        // Clear all buffers safely
        fifo.clear();
        fifo.shrink_to_fit();
        fftData.clear();
        fftData.shrink_to_fit();
        scopeData.clear();
        scopeData.shrink_to_fit();
        freqPoints.clear();
        freqPoints.shrink_to_fit();
        previousScope.clear();
        previousScope.shrink_to_fit();
        
        // Make sure we're not processing anything
        nextFFTBlockReady = false;
        fifoIndex = 0;
    }

    void setSampleRate(float newSampleRate)
    {
        const juce::SpinLock::ScopedLockType lock(mutex);
        sampleRate = newSampleRate;
    }

    void pushBuffer(const float* data, int numSamples)
    {
        if (data == nullptr || numSamples <= 0)
            return;

        const juce::SpinLock::ScopedLockType lock(mutex);

        // Copy samples to the fifo buffer
        for (int i = 0; i < numSamples; ++i)
        {
            fifo[fifoIndex] = data[i]; // Write current sample
            fifoIndex = (fifoIndex + 1) % fftSize; // Move to next position, wrapping around

            if (fifoIndex == 0) // If we wrapped around, a full FFT block is ready
            {
                nextFFTBlockReady = true;
            }
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
        for (int db = -90; db <= 0; db += 6)
        {
            const float y = dbToY(static_cast<float>(db), height);
            g.drawHorizontalLine(static_cast<int>(y), 0.0f, width);
            
            // Draw dB labels
            g.setColour(juce::Colours::grey.withAlpha(0.5f));
            g.drawText(juce::String(db), 2, static_cast<int>(y - 8), 25, 15, juce::Justification::centred);
            g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
        }

        // Draw spectrum
        g.setColour(juce::Colours::cyan);
        
        juce::Path smoothPath;
        smoothPath.startNewSubPath(0.0f, height);

        // Create a temporary array for smoothed points
        std::vector<float> smoothedLevels(numPoints);
        
        // Calculate dB levels with proper scaling
        const float minDB = -90.0f;
        const float maxDB = 0.0f;
        
        for (int i = 0; i < numPoints; ++i)
        {
            const float magnitude = scopeData[i];
            if (magnitude > 0.0f)
            {
                smoothedLevels[i] = juce::jlimit(minDB, maxDB, juce::Decibels::gainToDecibels(magnitude) + displayOffsetDB);
            }
            else
            {
                smoothedLevels[i] = minDB;
            }
        }
        
        // Apply gaussian smoothing
        const int smoothingRange = 5;
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
                    float weight = std::exp(-0.5f * (j * j) / (smoothingRange * smoothingRange));
                    sum += tempLevels[index] * weight;
                    weightSum += weight;
                }
            }
            
            smoothedLevels[i] = sum / weightSum;
        }

        // Draw the smoothed curve
        const float x0 = freqToX(freqPoints[0], width);
        const float level0 = juce::jlimit(minDB, maxDB, smoothedLevels[0]);
        const float y0 = dbToY(level0, height);
        smoothPath.startNewSubPath(x0, y0);
        
        // Use more points for the curve
        for (int i = 1; i < numPoints; ++i)
        {
            const float x = freqToX(freqPoints[i], width);
            const float level = juce::jlimit(minDB, maxDB, smoothedLevels[i]);
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
            juce::Colours::cyan.withAlpha(0.5f), 0.0f, 0.0f,
            juce::Colours::cyan.withAlpha(0.2f), 0.0f, height,
            false);
        g.setGradientFill(gradient);
        g.fillPath(smoothPath);
        
        // Draw the line on top
        g.setColour(juce::Colours::cyan);
        g.strokePath(smoothPath, juce::PathStrokeType(2.0f));
    }

    void resized() override {}

private:
    static constexpr int fftOrder = 13;  // Reverted to 13 for better resolution
    static constexpr int fftSize = 1 << fftOrder; // Now 8192
    static constexpr int numPoints = 1024; // Increased for better resolution and mapping
    static constexpr float decayFactor = 0.7f;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    std::vector<float> fifo;
    std::vector<float> fftData;
    std::vector<float> scopeData;
    std::vector<float> freqPoints;
    std::vector<float> previousScope;
    
    float sampleRate;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    juce::SpinLock mutex;
    float displayOffsetDB = -60.0f; // Changed to -60.0f for more offset

    void timerCallback() override
    {
        // Acquire lock for thread safety
        const juce::SpinLock::ScopedLockType lock(mutex);

        // Only perform FFT if a new block is ready
        if (nextFFTBlockReady)
        {
            // Copy fifo data to fftData, then apply windowing
            juce::FloatVectorOperations::copy(fftData.data(), fifo.data(), fftSize);
            window.multiplyWithWindowingTable(fftData.data(), fftSize);

            // Perform forward FFT
            forwardFFT.performRealOnlyForwardTransform(fftData.data());

            // Reset the flag immediately to avoid re-processing the same data
            nextFFTBlockReady = false;

            // Process FFT data to get magnitudes
            std::vector<float> currentMagnitudes(fftSize / 2 + 1);

            currentMagnitudes[0] = fftData[0];
            if (fftSize > 1)
                currentMagnitudes[fftSize / 2] = fftData[1];

            for (int i = 1; i < fftSize / 2; ++i)
            {
                currentMagnitudes[i] = std::sqrt(fftData[i * 2] * fftData[i * 2] +
                                                 fftData[i * 2 + 1] * fftData[i * 2 + 1]);
            }
            
            // Map raw magnitudes to scopeData (logarithmic frequency scale)
            // and apply smoothing/decay
            const float binWidth = sampleRate / fftSize;

            for (int i = 0; i < numPoints; ++i)
            {
                const float freq = freqPoints[i];
                const int bin = static_cast<int>(freq / binWidth);

                if (bin >= 0 && bin < currentMagnitudes.size())
                {
                    scopeData[i] = juce::jmax(currentMagnitudes[bin], previousScope[i] * decayFactor);
                }
                else
                {
                    scopeData[i] = previousScope[i] * decayFactor;
                }
            }

            juce::FloatVectorOperations::copy(previousScope.data(), scopeData.data(), numPoints);
            repaint();
        }
        else // If no new block is ready, apply decay only
        {
            for (int i = 0; i < numPoints; ++i)
            {
                scopeData[i] = previousScope[i] * decayFactor;
            }
            juce::FloatVectorOperations::copy(previousScope.data(), scopeData.data(), numPoints);
            repaint();
        }
    }

    float freqToX(float freq, float width) const
    {
        // Convert frequency to x coordinate (logarithmic scale)
        return width * (std::log10(freq/20.0f) / std::log10(1000.0f));
    }

    float dbToY(float db, float height) const
    {
        // Convert dB to y coordinate with new range (-90 to 0)
        return height * (1.0f - (db + 90.0f) / 90.0f);  // Changed range to 90dB total
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
}; 