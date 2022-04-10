#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <fstream>
#include "tinycolormap/include/tinycolormap.hpp"
#include "dsp.h"
extern std::fstream logFs;
using namespace juce::gl;

//==============================================================================
/**
*/
constexpr float epsilon = 1e-9;
const float freqMeterTicks[] = { 50.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };

class SpectrogramComponent : public juce::Component, private juce::Timer, public juce::OpenGLRenderer, public juce::MouseListener
{
public:
    SpectrogramComponent(bool showPianoRoll);
    ~SpectrogramComponent() override;

	static float midiFreq(int midiValue) {
		return 440 * powf(2, (midiValue - 69) / 12.0f);
	}
	static std::string keyName(int midiValue) {
		static std::string names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		return names[midiValue % 12] + std::to_string(midiValue / 12 - 1);
	}
    // screen blend color
	static juce::Colour screen(juce::Colour color1, juce::Colour color2) {
		auto r = (1 - (1 - color1.getFloatRed()) * (1 - color2.getFloatRed()));
		auto g = (1 - (1 - color1.getFloatGreen()) * (1 - color2.getFloatGreen()));
		auto b = (1 - (1 - color1.getFloatBlue()) * (1 - color2.getFloatBlue()));
		return juce::Colour::fromFloatRGBA(r, g, b, 1.0f);
	}

    // juce UI events
    void setMouseInOuter(bool isInOuter) {
        this->mouseInOuter = isInOuter;
    }
    void setMouseOuterPos(int x, int y) {
        outerX = x;
        outerY = y;
    }
	void mouseEnter(const juce::MouseEvent& event) override {
        mouseIn = true;
    }
	void mouseExit(const juce::MouseEvent& event) override {
        mouseIn = false;
    }
	void mouseMove(const juce::MouseEvent& event) override {
        mouseX = event.x;
        mouseY = event.y;
    }
    void mouseDown(const juce::MouseEvent& event) override {
        isPaused = !isPaused;
    }

    // feed me data here
    void setSampleRate(float sr) { this->sr = sr; }
    void pushNextSampleIntoFifo(float sample) noexcept;

protected:
    void newOpenGLContextCreated();
    void renderOpenGL();
    void openGLContextClosing();
    void resized() override;
    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    void drawNextLineOfSpectrogram();
    void drawPianoRoll(juce::Graphics &g);

private:
    enum
    {
        strideOrder = 8,
        strideSize = 1 << strideOrder,

        windowOrder = 12,
        windowSize = 1 << windowOrder,

        fftOrder = 16,
        fftSize  = 1 << fftOrder,

        nsinc = 128,

        imageWidth = 256,
        imageHeight = 1200,
    };

    const float maxdb = 0;
    const float mindb = -96;

    // input fifo
    float fifo [windowSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    // fft related
    double sr = 48000; // TODO: get from processor
    float fftData [windowSize];
    float fftDataTest [windowSize];
    float bins_data [imageHeight];
    float peaks_data [imageHeight];
    std::unique_ptr<WaveDsp> dsp = 
        std::make_unique<WaveDsp>(windowOrder, strideOrder, fftOrder, nsinc, imageHeight);

    // mouse related
    int mouseBarWidth = 20;
    int mouseBarHeight = 20;
    bool mouseIn = false;
    int mouseX = 0, mouseY = 0;
    bool mouseInOuter = false;
    int outerX = 0, outerY = 0;

    // UI related
    bool needPianoRoll;

    // spectro OpenGL data
    bool isPaused = false;
    juce::OpenGLContext openGLContext;
    GLint oglFormat = GL_RGBA;
    GLuint textureID = 0;
	GLint offset = 0;
    std::vector<uint32_t> edgeImage = std::vector<uint32_t>(imageHeight, 0);
    bool edgeImageHasData = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramComponent)
};
