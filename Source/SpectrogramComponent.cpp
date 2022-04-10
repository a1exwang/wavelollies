/*
  ==============================================================================

    SpectrogramComponent.cpp
    Created: 10 Apr 2022 3:02:19pm
    Author:  alexwang

  ==============================================================================
*/

#include "SpectrogramComponent.h"

#include <fstream>

//==============================================================================
SpectrogramComponent::SpectrogramComponent(bool showPianoRoll): needPianoRoll(showPianoRoll) {
	setOpaque(true);
	openGLContext.setRenderer(this);
	openGLContext.setContinuousRepainting(false);
	openGLContext.attachTo(*this);

    startTimerHz(90);
}

SpectrogramComponent::~SpectrogramComponent() {
	openGLContext.detach();
}


void SpectrogramComponent::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void SpectrogramComponent::drawPianoRoll(juce::Graphics &g) {
    float minfreq = 20, maxfreq = 20000;
    float widthFreqText = 36;
    float heightFreqText = 20;
    float widthFreqTick = 8;
    float lineWidthFreqTick = 2;

    float height = getHeight();
    float width = getWidth();
    // ---- frequency meter bg
    if (needPianoRoll) {
		g.setColour(juce::Colours::grey);
		// float totalWidthLeft = pianoKeysStartX + widthKey;
		g.fillRect(0, 0, 85, (int)height);
    }

    // ---- frequency ticks and text
    for (int i = 0; i < sizeof(freqMeterTicks) / sizeof(freqMeterTicks[0]); i++) {
        auto f = freqMeterTicks[i];
        float index = log2f(f / minfreq) / log2f(maxfreq / minfreq) * height;
        float y = height - index - 1;
        if (needPianoRoll) {
			g.setColour(juce::Colours::blue);
			g.drawLine(0 + widthFreqText, y, 0 + widthFreqText + widthFreqTick, y, lineWidthFreqTick);
        }

        g.setColour(juce::Colours::blue);
        g.setOpacity(0.2);
        g.drawLine(widthFreqText + widthFreqTick, y, width, y, lineWidthFreqTick);
        if (needPianoRoll) {
			std::stringstream ss;
			if (f >= 1000) {
				ss << int(f / 1000.0) << "k";
			}
			else {
				ss << int(f);
			}
			g.setColour(juce::Colours::blue);
			g.setOpacity(1);
			g.setFont(16);
			g.drawText(ss.str(), 0, y - heightFreqText / 2, widthFreqText - 2, heightFreqText, juce::Justification::centredRight, false);
        }
    }

    // ---- piano keys
    if (needPianoRoll) {
		auto pianoKeysStartX = widthFreqText + widthFreqTick;
		float widthKeyName = 20;
		float widthKey = 20;
		float widthBlackKey = 20;
		float heightKey = height / log2f(maxfreq / minfreq) / 7 - 2;
		for (int k = 0; k < 2; k++) {
			for (int i = 24; i < 132; i++) {
				auto name = keyName(i);
				bool isWhite = name.size() == 2;
				auto freq = midiFreq(i);
				auto y = (1-log2f(freq / minfreq) / log2f(maxfreq / minfreq)) * height;
				g.setColour(isWhite ? juce::Colours::white : juce::Colours::black);
				g.fillRect(pianoKeysStartX, y - heightKey / 2, widthKey, heightKey);
				g.setColour(juce::Colours::black);
				g.drawRect(pianoKeysStartX, y - heightKey / 2, widthKey, heightKey, 1.0f);
				if (i % 12 == 0) {
					g.setColour(juce::Colours::blueviolet);
					g.setFont(16);
					g.drawText(name, pianoKeysStartX + 20, y - heightKey / 2, widthKeyName, heightKey, juce::Justification::centredLeft, false);
				}
			}
		}
    }
}

void SpectrogramComponent::paint(juce::Graphics& g)
{
    drawPianoRoll(g);

    // ---- vertical and horizontal line at mouse pointer
    if (mouseIn || mouseInOuter) {
        g.setColour(juce::Colour::fromFloatRGBA(1, 1, 1, 1.0f));
        g.setOpacity(0.1f);
        if (mouseIn) {
            g.fillRect(0, mouseY - mouseBarHeight / 2, getWidth(), mouseBarHeight);
            g.fillRect(mouseX - mouseBarWidth / 2, 0, mouseBarWidth, getHeight());
        }
        else {
            g.fillRect(0, outerY - mouseBarHeight / 2, getWidth(), mouseBarHeight);
        }

        g.setOpacity(1.0f);
        if (mouseIn) {
            g.drawLine(0, mouseY, getWidth(), mouseY);
            g.drawLine(mouseX, 0, mouseX, getHeight());
        }
        else {
            g.drawLine(0, outerY, getWidth(), outerY);
        }
    }
}

void SpectrogramComponent::timerCallback()
{
    if (nextFFTBlockReady)
    {
        drawNextLineOfSpectrogram();
        nextFFTBlockReady = false;
        repaint();
    }
}

void SpectrogramComponent::pushNextSampleIntoFifo(float sample) noexcept {
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
            std::copy(&fifo[fifoIndex], &fftData[windowSize], fftData);
            std::copy(&fifo[0], &fftData[fifoIndex], &fftData[windowSize - fifoIndex]);

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

void SpectrogramComponent::drawNextLineOfSpectrogram() {
    if (imageHeight < 2) {
        return;
    }

    const auto t0 = std::chrono::high_resolution_clock::now();
    //for (int i = 0; i < windowSize; i++) {
        //fftDataTest[i] = sinf(2 * M_PI * 128 * i / sr);
    //}
    //logFs << "dump_param: \n" << dsp->dump_param() << std::endl;
    dsp->e2e(bins_data, peaks_data, fftData, sr, true);
    /* {
    std::ofstream test("L:\\test.log");
    for (int i = 0; i < imageHeight; i++) {
    test << bins_data[i] << std::endl;
    }
    }*/
    const auto t1 = std::chrono::high_resolution_clock::now();
    logFs << t1.time_since_epoch().count() << " Time DSP: " << 1000 * (std::chrono::duration<float>(t1 - t0).count()) << "ms" << std::endl;

    for (auto y = 1; y < imageHeight; ++y) {
        auto a = bins_data[y];
        auto b = peaks_data[y];
        a = (a - mindb) / (maxdb - mindb);
        b = (b - mindb) / (maxdb - mindb);
        // make it more nearer to 1
        b = pow(b, 0.1);
        const tinycolormap::Color colorSpectro = tinycolormap::GetColor(a, tinycolormap::ColormapType::Magma);
        // peaks make it more yellow
        juce::Colour colorPeaks = juce::Colour::fromFloatRGBA(b, 0, 0, 1.0f);
        auto blend = screen(juce::Colour(colorSpectro.ri(), colorSpectro.gi(), colorSpectro.bi()), colorPeaks);
        edgeImage[y] = 0xff000000 | ((uint32_t)blend.getRed() << 16) | ((uint32_t)blend.getGreen() << 8) | (uint32_t)blend.getBlue();
    }
    edgeImageHasData = true;
    const auto t3 = std::chrono::high_resolution_clock::now();

    logFs << t3.time_since_epoch().count() << " Time All: " << 1000 * (std::chrono::duration<float>(t3 - t0).count()) << "ms" << std::endl;
}

void SpectrogramComponent::newOpenGLContextCreated()
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

void SpectrogramComponent::renderOpenGL()
{
    const auto t0 = std::chrono::high_resolution_clock::now();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glTranslatef(-1, -1, 0);
    glScalef(2.0 / imageWidth, 2.0 / imageHeight, 1.0);

    GLint matrixMode = 0;
    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glScalef(1.0 / imageWidth, 1.0 / imageHeight, 1.0);

    glBindTexture(GL_TEXTURE_2D, textureID);

    if (edgeImageHasData) {
        if (!isPaused) {
            offset = (offset + 1) % imageWidth;
            glTexSubImage2D(GL_TEXTURE_2D, 0, offset, 0, 1, imageHeight, oglFormat, GL_UNSIGNED_BYTE, edgeImage.data());
        }
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
    logFs << t1.time_since_epoch().count() << " Time paint(): " << 1000 * (std::chrono::duration<float>(t1 - t0).count()) << "ms" << std::endl;
}

void SpectrogramComponent::openGLContextClosing()
{
}

