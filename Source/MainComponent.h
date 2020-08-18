/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SHTimeline.h"
#include "SHListBox.h"
#include "SHLAF.hpp"

enum TransportState
{
    Stopped,
    Starting,
    Playing,
    Unpause,
    Pausing,
    Paused,
    Stopping
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public DragAndDropContainer, public AudioAppComponent,
public Button::Listener,
public ChangeListener,
public Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void paint (Graphics&) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void filePlay(int rowNumber);
    void listPlayNext(void);
    void changeState (TransportState newState);
    void    changeListenerCallback (ChangeBroadcaster* source) override;
    void timerCallback() override;

    
    std::unique_ptr<SH02LAF> laf;
    std::unique_ptr<ToggleButton> addButton;
    std::unique_ptr<ToggleButton> removeButton;
    std::unique_ptr<ToggleButton> recordButton;
    std::unique_ptr<ToggleButton> playButton;
    std::unique_ptr<ToggleButton> pauseButton;
    std::unique_ptr<ToggleButton> hackButton;
    Viewport timelineVP, soundfileVP;
    SHTimeline *timeline;
    SHListBox *soundfileLB;
    TransportState state;

private:
    //==============================================================================
    // Your private member variables go here...
    Random random;
    float lastplaySecond;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
