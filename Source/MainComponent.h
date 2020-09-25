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
#include "SHGPluginProcessor.h"

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

enum ProcessType
{
    kBinaural,
    kConvolution,
    kDynamics,
    kGain,
    kMutation,
    kPhaseVocoder,
    kExtraction,
    kVarispeed
};

typedef struct SHOfflineProcess
{
    int type;
    int status;
    AudioProcessor *plugin;
    int blockSize;
    int blockNumber;
    int rowNumber;
    SparseSet<int> rowsSelected;
}   SHOfflineProcess;


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MultiProcessPanel : public MultiDocumentPanel
{
    public:
        MultiProcessPanel() {
            setLayoutMode(MaximisedWindowsWithTabs);
        }

        ~MultiProcessPanel() override
        {
            closeAllDocuments (true);
        }
    bool tryToCloseDocument (Component* component) override
    {
        ignoreUnused (component);

        return true;
    }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiProcessPanel)

};

class SoundInfoPanel : public Component
{
public:
    SoundInfoPanel();
    ~SoundInfoPanel();
    void paint(Graphics& g);
    void setAFR(File f);
    std::unique_ptr<AudioFormatReader> afr;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundInfoPanel)
};

class MainComponent   : public DragAndDropContainer, public AudioAppComponent,
public Button::Listener,
public ChangeListener,
public Timer,
public Thread
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    //==============================================================================
    void releaseResources() override;
    void paint (Graphics&) override;
    void resized() override;
    
    void processStart(int type);
    void run() override;
    void processEnd(SHOfflineProcess* op);

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void play(int rowNumber);
    void playNextSelected(void);
    
    void changeState (TransportState newState);
    void buttonClicked (Button* buttonThatWasClicked) override;
    void changeListenerCallback (ChangeBroadcaster* source) override;
    void timerCallback() override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    
    std::unique_ptr<SH02LAF> laf;
    std::unique_ptr<ToggleButton> addButton;
    std::unique_ptr<ToggleButton> removeButton;
    std::unique_ptr<ToggleButton> recordButton;
    std::unique_ptr<ToggleButton> playButton;
    std::unique_ptr<ToggleButton> pauseButton;
    std::unique_ptr<ToggleButton> binauralButton;
    std::unique_ptr<ToggleButton> convolveButton;
    std::unique_ptr<ToggleButton> gainButton;
    std::unique_ptr<ToggleButton> mutateButton;
    std::unique_ptr<ToggleButton> pvocButton;
    std::unique_ptr<ToggleButton> extractButton;
    
    MultiProcessPanel processPanel;
    SoundInfoPanel siPanel;
    
    Viewport timelineVP, soundfileVP;
    
    SHTimeline *timeline;
    SHListBox *soundfileLB;
    
    TransportState state;
    std::vector<SHOfflineProcess *> offliners;
    float samplerate;

private:
    //==============================================================================
    // Your private member variables go here...
    Random random;
    float lastplaySecond;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
