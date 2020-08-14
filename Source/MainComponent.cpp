/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : state(Stopped)
{
    // create windows
    setSize (640, 800);
    laf.reset(new SH02LAF());
    setLookAndFeel(laf.get());

    soundfileVP.setSize(getWidth() - 18, getHeight()-218);
    soundfileLB = new SHListBox();
    soundfileLB->setSize(getWidth() * 2, getHeight() * 2);
    soundfileLB->setColour(ListBox::backgroundColourId, Colours::black);
    soundfileLB->setRowHeight(25);
    soundfileVP.setViewedComponent(soundfileLB, true);
    addAndMakeVisible(soundfileVP);
    soundfileLB->transportSource.addChangeListener (this); // [2]

    // gui components
    addButton.reset (new ToggleButton ("+"));
    addAndMakeVisible (addButton.get());
    addButton->addListener (this);
    addButton->setBounds (9, 140, 60, 60);
    
    removeButton.reset (new ToggleButton ("-"));
    addAndMakeVisible (removeButton.get());
    removeButton->addListener (this);
    removeButton->setBounds (78, 140, 60, 60);
     
    recordButton.reset (new ToggleButton ("record"));
    addAndMakeVisible (recordButton.get());
    recordButton->addListener (this);
    recordButton->setBounds (200, 140, 60, 60);
     
    playButton.reset (new ToggleButton ("play"));
    addAndMakeVisible (playButton.get());
    playButton->addListener (this);
    playButton->setBounds (272, 140, 60, 60);
     
    pauseButton.reset (new ToggleButton ("pause"));
    addAndMakeVisible (pauseButton.get());
    pauseButton->addListener (this);
    pauseButton->setBounds (344, 140, 60, 60);
     
    hackButton.reset (new ToggleButton ("hack"));
    addAndMakeVisible (hackButton.get());
    hackButton->addListener (this);
    hackButton->setBounds (getWidth() - 78, 140, 60, 60);
     

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
    playRow = -1;
}

MainComponent::~MainComponent()
{
    shutdownAudio();
    setLookAndFeel (nullptr);
 //   delete(laf);
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colour(51, 51, 51));
    g.setColour(Colours::black);
    g.fillRect(9, 9, getWidth() - 18, 120);
    
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    soundfileVP.setBounds(9, 209, getWidth() - 18,getHeight() - 218);
}

void MainComponent::filePlay(int rowNumber)
{
    auto* reader = soundfileLB->formatManager.createReaderFor (*(soundfileLB->soundfiles[rowNumber]));              // [10]
    if (reader != nullptr)
    {
        std::unique_ptr<AudioFormatReaderSource> newSource (new AudioFormatReaderSource (reader, true)); // [11]
        soundfileLB->transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);                     // [12]
        soundfileLB->readerSource.reset (newSource.release());
    }
    soundfileLB->transportSource.start();
}

void MainComponent::listPlayNext(void)
{
    playRow++;
    if(playRow >= soundfileLB->getNumSelectedRows())
    {
        changeState( Stopped);
        playRow = -1;
    }
    else
        filePlay(soundfileLB->getSelectedRow(playRow));
}

void MainComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &(soundfileLB->transportSource))
    {
        if (soundfileLB->transportSource.isPlaying())
            changeState (Playing);
         else if ((state == Stopping) || (state == Playing))
         {
             changeState (Stopped);
             if(playRow >= 0)
                 changeState (Starting);
         }
         else if (Pausing == state)
             changeState (Paused);
    }
  }


void MainComponent::changeState (TransportState newState)
{
    if (state != newState)
    {
        state = newState;
 
        switch (state)
        {
             case Stopped:                           // [3]
                soundfileLB->transportSource.setPosition (0.0);
                pauseButton.get()->setToggleState(false, dontSendNotification);
                playButton.get()->setToggleState(false, dontSendNotification);
                break;
            case Starting:                          // [4]
                pauseButton.get()->setToggleState(false, dontSendNotification);
                playButton.get()->setToggleState(true, dontSendNotification);
                listPlayNext();
                break;
            case Unpause:
                pauseButton.get()->setToggleState(false, dontSendNotification);
                playButton.get()->setToggleState(true, dontSendNotification);
                soundfileLB->transportSource.start();
                break;
            case Paused:
                break;
            case Pausing:
                soundfileLB->transportSource.stop();
                break;
            case Playing:                           // [5]
                break;
            case Stopping:                          // [6]
                soundfileLB->transportSource.stop();
                break;
        }
    }
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

void MainComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == addButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        if(addButton.get()->getToggleState() == true)
        {
            soundfileLB->fileOpenPanel();
            addButton.get()->setToggleState(false, dontSendNotification);
        }
        //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == removeButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        if(removeButton.get()->getToggleState() == true)
        {
            soundfileLB->removeSoundFile();
            removeButton.get()->setToggleState(false, dontSendNotification);
        }
        //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == playButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        if (state == Stopped)
            changeState (Starting);
        else if (state == Playing)
            changeState (Stopping);
        else if(state == Paused)
        {
            soundfileLB->transportSource.setPosition (0.0);
            changeState (Starting);
        }
       //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == pauseButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        if (state == Paused)
            changeState(Unpause);
        else if(state == Playing)
            changeState(Pausing);
        //[/UserButtonCode_toggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}
