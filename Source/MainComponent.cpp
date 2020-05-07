/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // create windows
    setSize (800, 800);
    laf = new LookAndFeel_V3();
    setLookAndFeel(laf);

    soundfileVP.setSize(getWidth(), getHeight());
    auto soundfileLB = new SHListBox();
    soundfileLB->setSize((getWidth()) - 6, getHeight() * 2);
    soundfileLB->setColour(ListBox::backgroundColourId, Colours::darkgrey);
    soundfileLB->setRowHeight(25);
    soundfileVP.setViewedComponent(soundfileLB, true);
    addAndMakeVisible(soundfileVP);
    // initialize audio services
    // Some platforms require permissions to open input channels so request that here
     if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
         && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
     {
         RuntimePermissions::request (RuntimePermissions::recordAudio,
                [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
     }
     else
     {
         // Specify the number of input and output channels that we want to open
         setAudioChannels (2, 2);
     }

}

MainComponent::~MainComponent()
{
    shutdownAudio();
 //   delete(soundfileLB);
    setLookAndFeel (nullptr);
 //   delete(laf);
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
    soundfileVP.setBounds(3, 3, getWidth() - 6,getHeight() - 6);
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    String message;
    message << "Preparing to play audio..." << newLine;
    message << " samplesPerBlockExpected = " << samplesPerBlockExpected << newLine;
    message << " sampleRate = " << sampleRate;
    Logger::getCurrentLogger()->writeToLog (message);
}
void MainComponent::releaseResources()
{;
}
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        // Get a pointer to the start sample in the buffer for this audio output channel
        auto* buffer = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);
 
        // Fill the required number of samples with noise between -0.125 and +0.125
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            buffer[sample] = random.nextFloat() * 0.25f - 0.125f;
    }
}

