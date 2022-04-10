/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpectrogramComponent.h"

class SolidColorComponent : public juce::Component {
public:
    SolidColorComponent(juce::Colour c) :color(c) { }

    void paint(juce::Graphics& g) override {
        g.fillAll(color);
    }

private:
    juce::Colour color;
};

class MainAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::MouseListener
{
public:
    double sr = 48000;
    MainAudioProcessorEditor (NewProjectAudioProcessor&);
    ~MainAudioProcessorEditor() override;
    void resized() override;

    void pushNextSampleIntoFifo(float sample) noexcept {
        spectrogramComponentL.pushNextSampleIntoFifo(sample);
        spectrogramComponentR.pushNextSampleIntoFifo(sample);
    }
	void mouseEnter(const juce::MouseEvent& event) override {
        mouseIn = true;
        spectrogramComponentL.setMouseInOuter(true);
        spectrogramComponentR.setMouseInOuter(true);
    }

	void mouseExit(const juce::MouseEvent& event) override {
        mouseIn = false;
        spectrogramComponentL.setMouseInOuter(false);
        spectrogramComponentR.setMouseInOuter(false);
    }
    void mouseMove(const juce::MouseEvent& event) override {
        mouseX = event.x;
        mouseY = event.y;
        // coordinates are relative to child component, so only Y coordinate is correct
        spectrogramComponentL.setMouseOuterPos(event.x, event.y);
        spectrogramComponentR.setMouseOuterPos(event.x, event.y);
    }
private:
    bool mouseIn;
    int mouseX = 0, mouseY = 0;
    NewProjectAudioProcessor& audioProcessor;

    SpectrogramComponent spectrogramComponentL;
    SpectrogramComponent spectrogramComponentR;

    SolidColorComponent spectrogramSeparator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAudioProcessorEditor)
};
