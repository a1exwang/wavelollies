/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <fstream>

std::fstream logFs("L:\\wavelolly.log", std::ios::out);

//==============================================================================
MainAudioProcessorEditor::MainAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), spectrogramComponentL(true), spectrogramComponentR(false), spectrogramSeparator(juce::Colours::grey) {
	setSize(1024, 740);
    setResizable(true, true);
    setResizeLimits(256, 256, 1048576, 1048576);

    addAndMakeVisible(spectrogramComponentL);
    spectrogramComponentL.addMouseListener(this, false);
    addAndMakeVisible(spectrogramComponentR);
    spectrogramComponentR.addMouseListener(this, false);
}

MainAudioProcessorEditor::~MainAudioProcessorEditor()
{
}


void MainAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto sepWidth = 10;
    auto specWidth = (bounds.getWidth() - sepWidth) / 2;
    spectrogramComponentL.setBounds(bounds.removeFromLeft(specWidth));
    spectrogramSeparator.setBounds(bounds.removeFromLeft(sepWidth));
    spectrogramComponentR.setBounds(bounds);
}

