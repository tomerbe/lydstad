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
    laf.reset(new LookAndFeel_V3());
    setLookAndFeel(laf.get());

    soundfileVP.setSize(getWidth(), getHeight());
    soundfileLB = new SHListBox();
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
 //   if (soundfileLB->readerSource.get() != nullptr)
        soundfileLB->transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}
void MainComponent::releaseResources()
{
    soundfileLB->transportSource.releaseResources();
}
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (soundfileLB->readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
 
    soundfileLB->transportSource.getNextAudioBlock (bufferToFill);
}

