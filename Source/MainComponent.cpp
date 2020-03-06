/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (800, 800);
    laf = new LookAndFeel_V3();
    setLookAndFeel(laf);

    soundfileLB.setSize((getWidth()) - 6, (getHeight()) - 6);
    soundfileLB.setColour(ListBox::backgroundColourId, Colours::darkgrey);
    soundfileLB.setRowHeight(99);
    addAndMakeVisible(soundfileLB);
}

MainComponent::~MainComponent()
{
    setLookAndFeel (nullptr);
    delete(laf);
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::darkgrey);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    soundfileLB.setBounds(3, 3, getWidth() - 6,getHeight() - 6);
}
