/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
SoundInfoPanel::SoundInfoPanel()
    : Component("sound info")
{
}

SoundInfoPanel::~SoundInfoPanel()
{
}

void SoundInfoPanel::setAFR(File f)
{
    AudioFormatManager fm;
    fm.registerBasicFormats();
    afr.reset(fm.createReaderFor(f));
}

void SoundInfoPanel::paint(Graphics& g)
{
    String fileprop;
    Font f(String("Trebuchet MS"), String("Regular"), 18.0f);
    g.fillAll (Colours::black);
    if(afr)
    {
        // draw soundfile info for last clicked file.
        g.setColour (Colours::whitesmoke);
        g.setFont(f);
        
        fileprop = afr->getFormatName() + String(" type");
        g.drawText(fileprop, 9, 5, getWidth()-18, 24, juce::Justification::centredLeft, false);
        
        fileprop = String(afr->numChannels) + String(" channels");
        g.drawText(fileprop, 9, 35, getWidth()-18, 24, juce::Justification::centredLeft, false);
        
        int min = afr->lengthInSamples/(afr->sampleRate * 60);
        int sec = afr->lengthInSamples/afr->sampleRate - (min * 60);
        int msec = (1000 * afr->lengthInSamples/afr->sampleRate) - (((min * 60) + sec) * 1000);
        fileprop = String(min).paddedLeft('0',2) + String(":") + String(sec).paddedLeft('0',2) + String(".") + String(msec).paddedLeft('0',3) + String(" length");
        g.drawText(fileprop, 9, 65, getWidth()-18, 24, juce::Justification::centredLeft, false);
 
        fileprop = String(afr->sampleRate) + String(" sample rate");
        g.drawText(fileprop, 309, 5, getWidth()-18, 24, juce::Justification::centredLeft, false);
        
        fileprop = String(afr->lengthInSamples * afr->bitsPerSample/8) + String(" bytes");
        g.drawText(fileprop, 309, 35, getWidth()-18, 24, juce::Justification::centredLeft, false);
        
        if(afr->usesFloatingPointData)
            fileprop = String(afr->bitsPerSample) + String(" bits float");
        else
            fileprop = String(afr->bitsPerSample) + String(" bits integer");
        g.drawText(fileprop, 309, 65, getWidth()-18, 24, juce::Justification::centredLeft, false);
    }
}

//==============================================================================
MainComponent::MainComponent()
    : Thread ("process thread"), state(Stopped)

{
    // create windows
    setSize (640, 800);
    laf.reset(new SH02LAF());
    setLookAndFeel(laf.get());

    soundfileVP.setSize(getWidth() - 18, getHeight()-218);
    soundfileLB = new SHListBox();
    soundfileLB->setSize(getWidth()-18, getHeight() * 5);
    soundfileLB->setColour(ListBox::backgroundColourId, Colours::black);
    soundfileLB->setRowHeight(25);
    soundfileVP.setViewedComponent(soundfileLB, true);
    addAndMakeVisible(soundfileVP);
    soundfileLB->transportSource.addChangeListener (this); // [2]
    
    // gui components
    // top panel which is tabbed and will have processes
    siPanel.setSize(getWidth(), getHeight());
    processPanel.setBackgroundColour((Colours::black));
    // add soundinfo panel as first tab
    processPanel.addDocument(&siPanel,(Colour(51, 51, 51)), false);
    processPanel.getCurrentTabbedComponent()->setTabBarDepth(30);
    addAndMakeVisible(processPanel);

    // add files to list
    addButton.reset (new ToggleButton ("+"));
    addAndMakeVisible (addButton.get());
    addButton->addListener (this);
    addButton->setBounds (9, 142, 45, 45);
    
    // delete selected files from list
    removeButton.reset (new ToggleButton ("-"));
    addAndMakeVisible (removeButton.get());
    removeButton->addListener (this);
    removeButton->setBounds (53, 157, 45, 45);
    
    // record new file into list (TODO) - panel?
    recordButton.reset (new ToggleButton ("record"));
    addAndMakeVisible (recordButton.get());
    recordButton->addListener (this);
    recordButton->setBounds (141, 157, 45, 45);
    
    // play selected files
    playButton.reset (new ToggleButton ("play"));
    addAndMakeVisible (playButton.get());
    playButton->addListener (this);
    playButton->setBounds (185, 142, 45, 45);
     
    // pause playback
    pauseButton.reset (new ToggleButton ("pause"));
    addAndMakeVisible (pauseButton.get());
    pauseButton->addListener (this);
    pauseButton->setBounds (229, 157, 45, 45);
     
    // popup menu with processes
    binauralButton.reset (new ToggleButton ("b"));
    addAndMakeVisible (binauralButton.get());
    binauralButton->addListener (this);
    binauralButton->setBounds (366, 157, 45, 45);
     
    // convolveButton
    convolveButton.reset (new ToggleButton ("c"));
    addAndMakeVisible (convolveButton.get());
    convolveButton->addListener (this);
    convolveButton->setBounds (410, 142, 45, 45);

    // gainButton
    gainButton.reset (new ToggleButton ("g"));
    addAndMakeVisible (gainButton.get());
    gainButton->addListener (this);
    gainButton->setBounds (454, 157, 45, 45);

    // mutateButton
    mutateButton.reset (new ToggleButton ("m"));
    addAndMakeVisible (mutateButton.get());
    mutateButton->addListener (this);
    mutateButton->setBounds (498, 142, 45, 45);

    // gainButton
    pvocButton.reset (new ToggleButton ("p"));
    addAndMakeVisible (pvocButton.get());
    pvocButton->addListener (this);
    pvocButton->setBounds (542, 157, 45, 45);

    // gainButton
    extractButton.reset (new ToggleButton ("x"));
    addAndMakeVisible (extractButton.get());
    extractButton->addListener (this);
    extractButton->setBounds (586, 142, 45, 45);

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
    samplerate = 48000.0f;
    soundfileLB->playRow = -1;
    startTimer(250);
}

MainComponent::~MainComponent()
{
    long i;
    shutdownAudio();
    stopThread(100);
    for (SHOfflineProcess* op : offliners)
 //   for (i = offliners.size() - 1; i >= 0; i--)
        processEnd(op);
    setLookAndFeel (nullptr);
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colour(51, 51, 51));
    g.setColour(Colours::black);
    g.fillRect(9, 9, getWidth() - 18, 130);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    soundfileVP.setBounds(9, 209, getWidth() - 18,getHeight() - 218);
    processPanel.setBounds(9,9,getWidth() - 18, 130);
}

void MainComponent::processStart(int type)
{
    SHOfflineProcess *op;
    
    if(soundfileLB->getNumSelectedRows() == 0)
        return;
    op = new SHOfflineProcess;
    
    op->type = type;
    op->status = 1;
    op->rowsSelected = soundfileLB->getSelectedRows();
    op->blockNumber = 0;
    op->blockSize = 256;    // just guessing at a quick blocksize
    
    op->rowNumber = 0;
    switch(type)
    {
        case kBinaural:
            break;
        case kConvolution:
            break;
        case kDynamics:
            break;
        case kGain:
            op->plugin  = new ShgainAudioProcessor();
            ((ShgainAudioProcessor *)(op->plugin))->setInFile((soundfileLB->soundfiles[op->rowsSelected[op->rowNumber]]));
            break;
        case kMutation:
            break;
        case kPhaseVocoder:
            break;
        case kVarispeed:
            break;
        case kExtraction:
            break;
    }
    processPanel.addDocument(op->plugin->createEditorIfNeeded(),(Colour(51, 51, 51)), false);
    op->plugin->prepareToPlay(48000, op->blockSize);
    

/*    AudioBuffer<float> buffer;
    WavAudioFormat format;
    std::unique_ptr<AudioFormatWriter> writer;
    writer.reset (format.createWriterFor (new FileOutputStream (file),
                                      48000.0,
                                      buffer.getNumChannels(),
                                      24,
                                      {},
                                      0));
    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());*/
    
    op->status = kStopped;
    offliners.push_back(op);
    startThread();
}

void MainComponent::run()
{
    AudioBuffer<float> buf(2, 1024);
    MidiBuffer mm;
 
    while(threadShouldExit() == false)
    {
        for (SHOfflineProcess* op : offliners)
         {
             switch(op->status)
             {
                 case kStopped:
                     if(op->plugin->isSuspended() == false)
                         op->status = kStart;
                     else
                         yield();
                     break;
                 case kStart:
                     op->plugin->prepareToPlay(samplerate, 256);
                     op->status = kRun;
                     break;
                 case kRun:
                     if(op->plugin->isSuspended() == false)
                         op->plugin->processBlock(buf, mm);
                     else
                         op->status = kEnd;
                     break;
                 case kEnd:
                     op->plugin->releaseResources();
                     op->rowNumber++;
                     if(op->rowNumber < soundfileLB->getNumSelectedRows())
                     {
                         ((ShgainAudioProcessor *)(op->plugin))->setInFile((soundfileLB->soundfiles[op->rowsSelected[op->rowNumber]]));
                         op->status = kStart;
                     }
                     else
                         op->status = kStopped;
                     break;
             }
             if(op->plugin->getParameters()[0]->getValue() == 1.0f)
             {
                 const MessageManagerLock mmLock;   // lock the message manager so i can delete gui objects
                 processEnd(op);
             }
         }
        sleep(1); // 256/48
    }
}

void MainComponent::processEnd(SHOfflineProcess* op)
{
    AudioProcessorEditor*  editor;
    
    processPanel.closeDocument(op->plugin->getActiveEditor(), false);
    
    // a little shuffling to please the JUCE idea of active editor
    editor = op->plugin->getActiveEditor();
    op->plugin->editorBeingDeleted(editor);
    delete(editor);
    auto it = std::remove(offliners.begin(), offliners.end(), op);
    offliners.erase(it, offliners.end());
    delete(op->plugin);
    delete(op);
    if(offliners.size() > 0) startThread();
}

void MainComponent::play(int rowNumber)
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

void MainComponent::playNextSelected(void)
{
    soundfileLB->playRow++;
    if(soundfileLB->playRow >= soundfileLB->getNumSelectedRows())
    {
        changeState( Stopped);
        soundfileLB->playRow = -1;
    }
    else
        play(soundfileLB->getSelectedRow(soundfileLB->playRow));
}
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    String message;
    message << "Preparing to play audio..." << newLine;
    message << " samplesPerBlockExpected = " << samplesPerBlockExpected << newLine;
    message << " sampleRate = " << sampleRate;
    Logger::getCurrentLogger()->writeToLog (message);
    soundfileLB->transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}
void MainComponent::releaseResources()
{
    soundfileLB->transportSource.releaseResources();
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
             if(soundfileLB->playRow >= 0)
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
                soundfileLB->repaint();
                break;
            case Starting:                          // [4]
                soundfileLB->playSecond = 0.0f;
                lastplaySecond = 1.0f;
                pauseButton.get()->setToggleState(false, dontSendNotification);
                playButton.get()->setToggleState(true, dontSendNotification);
                playNextSelected();
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
void MainComponent::timerCallback()
{
    if(soundfileLB->playSecond != lastplaySecond && soundfileLB->playRow >= 0)
    {
        lastplaySecond = soundfileLB->playSecond;
        soundfileLB->repaintRow(soundfileLB->getSelectedRow(soundfileLB->playRow));
    }
    if(soundfileLB->getSelectedRow(0) >= 0)
    {
        siPanel.setAFR(*(soundfileLB->soundfiles[soundfileLB->getSelectedRow(0)]));
        siPanel.repaint();
    }
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (soundfileLB->readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
 
    soundfileLB->transportSource.getNextAudioBlock (bufferToFill);
    soundfileLB->playSecond = soundfileLB->transportSource.getCurrentPosition();
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
    else if (buttonThatWasClicked == binauralButton.get())
    {
            //[UserButtonCode_toggleButton] -- add your button handler code here..

            //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == convolveButton.get())
    {
            //[UserButtonCode_toggleButton] -- add your button handler code here..
            //[/UserButtonCode_toggleButton]
    }
    
    else if (buttonThatWasClicked == gainButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        if(gainButton.get()->getToggleState() == true)
        {
            processStart(kGain);
            gainButton.get()->setToggleState(false, dontSendNotification);
        }
        //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == mutateButton.get())
    {
            //[UserButtonCode_toggleButton] -- add your button handler code here..
            //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == pvocButton.get())
    {
            //[UserButtonCode_toggleButton] -- add your button handler code here..
            //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == extractButton.get())
    {
            //[UserButtonCode_toggleButton] -- add your button handler code here..
            //[/UserButtonCode_toggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}
