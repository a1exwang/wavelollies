/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

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
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer, public juce::OpenGLRenderer
{
public:
    double sr = 48000;
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    void resized() override;
    void paint (juce::Graphics& g) override
    {
    }

    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextLineOfSpectrogram();
            nextFFTBlockReady = false;
			repaint();
        }
    }


    void pushNextSampleIntoFifo (float sample) noexcept {
        // if the fifo contains enough data, set a flag to say
// that the next line should now be rendered..
if (fifoIndex % strideSize == 0)
{
    // TODO: use a queue to prevent lost
    if (!nextFFTBlockReady)
    {
        const auto t0 = std::chrono::high_resolution_clock::now();
        logFs << t0.time_since_epoch().count() << " Time begin block" << std::endl;

        juce::zeromem(fftData, sizeof(fftData));
        int fftStart = (fftSize - windowSize) / 2;
        std::copy(&fifo[fifoIndex], &fftData[windowSize], &fftData[fftStart]);
        std::copy(&fifo[0], &fftData[fifoIndex], &fftData[fftStart + windowSize - fifoIndex]);
        window.multiplyWithWindowingTable(&fftData[fftStart], windowSize);

        const auto t1 = std::chrono::high_resolution_clock::now();
        logFs << t1.time_since_epoch().count() << " Time end block" << std::endl;
        nextFFTBlockReady = true;
    }

    if (fifoIndex == windowSize) {
        fifoIndex = 0;
    }
}

fifo[fifoIndex++] = sample;
    }

#define PI 3.141592654f
    void drawNextLineOfSpectrogram() {
        if (imageHeight < 2) {
            return;
        }

        float lowfreq = 20.0f;
        float highfreq = 20000.0f;
        float k = log2(highfreq / lowfreq);

        const auto t0 = std::chrono::high_resolution_clock::now();

        dsp->forward(fftData);

        dsp->interpolate(fftDataBins, fftData, sr);

        const auto t1 = std::chrono::high_resolution_clock::now();
        float maxdb1 = -100;
        float maxdbF = 0;
        for (int y = 0; y < imageHeight; ++y) {
            auto freq = lowfreq * pow(2, k * y / imageHeight);
            auto v = fftDataBins[y];
            auto w = juce::jmap(v / 2, 0.0f, (float)windowSize, 0.0f, (float)fftSize);

            // -3dB slope
            //w *= pow(10, -3/10*log2(freq / sqrt(20*20000)));
            w = juce::jlimit(0.0f, 1.0f, w);

            float db = 10.0f * log10f(w);
            if (db > maxdb1) {
                maxdb1 = db;
                maxdbF = freq;
            }
            db = juce::jlimit(mindb, maxdb, db);
            float unify = juce::jmap(db, mindb, maxdb, 0.0f, 1.0f);
            fftDataLog2[y] = w;//unify;
        }
        logFs << "maxdb at " << maxdbF << "Hz, " << maxdb1 << "dB" << std::endl;

        // find peaks
        std::fill(&fftDataTmp[0], &fftDataTmp[imageHeight], 0.0f);
        bool found = false;
        bool rising = false;
        for (int y = 1; y < imageHeight - 1; y++) {
            const auto prev = fftDataLog2[y - 1];
            const auto next = fftDataLog2[y + 1];
            const auto curr = fftDataLog2[y];

            if (curr - prev > epsilon) {
                // rising
                stack.clear();
                rising = true;
                stack.push_back(y);
            }
			else if (abs(prev - curr) <= epsilon) {
                // leveling
                stack.push_back(y);
            }
            else {
                // falling
                if (rising) {
					while (!stack.empty()) {
						auto index = stack.back();
						fftDataTmp[index] = 1;
						stack.pop_back();
					}
                }
                else {
                    stack.clear();
                }
                rising = false;
            }
        }
		if (found) {
			const auto t3 = std::chrono::high_resolution_clock::now();
            //logFs << " at " << t3.time_since_epoch().count() << " Found" << std::endl;
        }
        for (int y = 1; y < imageHeight - 1; y++) {
            //fftDataLog2[y] = fftDataLog2[y] / 2 + fftDataTmp[y] / 2;
            //fftDataLog2[y] = (fftDataLog2[y] * fftDataTmp[y]) * 0.7 + fftDataLog2[y] * 0.3;
			fftDataLog2[y] = juce::jlimit(0.0f, 1.0f, fftDataLog2[y]);
        }

        const auto t2 = std::chrono::high_resolution_clock::now();
        //logFs << t2.time_since_epoch().count() <<  " Time Normalize: " << 1000 * (std::chrono::duration<float>(t2 - t1).count()) << "ms" << std::endl;

        for (auto y = 1; y < imageHeight; ++y) {
            auto interpolatedLevel = fftDataLog2[y];
            //const auto c = juce::Colour::fromHSV((1-0.3*interpolatedLevel), 0.0f, interpolatedLevel, 1.0f);
			const tinycolormap::Color color = tinycolormap::GetColor(interpolatedLevel, tinycolormap::ColormapType::Magma);
			edgeImage[y] = 0xff000000 | ((uint32_t)color.ri() << 16) | ((uint32_t)color.gi() << 8) | (uint32_t)color.bi();
        }
        edgeImageHasData = true;
        const auto t3 = std::chrono::high_resolution_clock::now();
        //logFs << t3.time_since_epoch().count() <<  " Time Image: " << 1000 * (std::chrono::duration<float>(t3 - t2).count()) << "ms" << std::endl;

        logFs << t3.time_since_epoch().count() <<  " Time All: " << 1000 * (std::chrono::duration<float>(t3 - t0).count()) << "ms" << std::endl;
    }

	void newOpenGLContextCreated()
	{
		glGenTextures(1, &textureID);
		if (textureID == 0) {
			auto err = glGetError();
			logFs << "GLerror " << err << std::endl;
		}
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		std::vector<GLuint> pData(imageWidth * imageHeight, 0xff000000);
        glTexImage2D(GL_TEXTURE_2D, 0, oglFormat, imageWidth, imageHeight, 0, oglFormat, GL_UNSIGNED_BYTE, pData.data());
        glBindTexture(GL_TEXTURE_2D, 0);
	}
    GLint oglFormat = GL_RGBA;

	void renderOpenGL()
	{
		const auto t0 = std::chrono::high_resolution_clock::now();
		//g.drawImage(spectrogramImage, getLocalBounds().toFloat());
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();

		glTranslatef(-1, -1, 0);
		glScalef(2.0 / imageWidth, 2.0 / imageHeight, 1.0);

        GLint matrixMode = 0;
		glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glScalef(1.0 /imageWidth, 1.0 /imageHeight, 1.0);

        glBindTexture(GL_TEXTURE_2D, textureID);

        if (edgeImageHasData) {
            offset = (offset + 1) % imageWidth;
            glTexSubImage2D(GL_TEXTURE_2D, 0, offset, 0, 1, imageHeight, oglFormat, GL_UNSIGNED_BYTE, edgeImage.data());
            edgeImageHasData = false;
        }

		glBegin(GL_QUADS);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		glTexCoord2i(0, 0);				glVertex2i(imageWidth - offset, 0);
		glTexCoord2i(0, imageHeight);		glVertex2i(imageWidth - offset, imageHeight);
		glTexCoord2i(offset, imageHeight);	glVertex2i(imageWidth, imageHeight);
		glTexCoord2i(offset, 0);		glVertex2i(imageWidth, 0);

		glTexCoord2i(offset, 0);		glVertex2i(0, 0);
		glTexCoord2i(offset, imageHeight);	glVertex2i(0, imageHeight);
		glTexCoord2i(imageWidth, imageHeight);	glVertex2i(imageWidth - offset, imageHeight);
		glTexCoord2i(imageWidth, 0);			glVertex2i(imageWidth - offset, 0);

		glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
		glPopMatrix();
		glMatrixMode(matrixMode);

		glPopMatrix();
		const auto t1 = std::chrono::high_resolution_clock::now();
		logFs << t1.time_since_epoch().count() <<  " Time paint(): " << 1000 * (std::chrono::duration<float>(t1 - t0).count()) << "ms" << std::endl;
	}

	void openGLContextClosing()
	{
	}

    enum
    {
        nsinc = 2,

        strideOrder = 6,
        strideSize = 1 << strideOrder,

        windowOrder = 11,
        windowSize = 1 << windowOrder,

        fftOrder = 16,
        fftSize  = 1 << fftOrder,

        imageWidth = 512,
        imageHeight = 2048,

    };
    const float maxdb = -10;
    const float mindb = -58;
    juce::dsp::FFT forwardFFT;
    std::vector<uint32_t> edgeImage = std::vector<uint32_t>(imageHeight, 0);
    bool edgeImageHasData = false;

    float fifo [windowSize];
    float fftData [2 * fftSize];
    float fftDataBins [imageHeight];
    float fftDataLog2[imageHeight];
    float fftDataTmp[imageHeight] = { 0 };
    std::vector<size_t> stack;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    juce::dsp::WindowingFunction<float> window = juce::dsp::WindowingFunction<float>(windowSize, juce::dsp::WindowingFunction<float>::WindowingMethod::blackmanHarris);

    juce::OpenGLContext openGLContext;
    GLuint textureID = 0;
	GLint offset = 0;

    std::unique_ptr<WaveDsp> dsp = std::make_unique<WaveDsp>(windowOrder, strideOrder, fftOrder, nsinc, imageHeight);
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
