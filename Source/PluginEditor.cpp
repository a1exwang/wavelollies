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
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    forwardFFT (fftOrder)
{
	using namespace juce::gl;
	setSize(512, 740);

	setOpaque(true);
	openGLContext.setRenderer(this);
	openGLContext.setContinuousRepainting(false);
	openGLContext.attachTo(*this);

	startTimerHz(60);

	stack.reserve(imageHeight);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
	openGLContext.detach();
}


void NewProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

