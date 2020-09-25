/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum
{
    kStopped,
    kStart,
    kRun,
    kEnd
};

enum
{
    kAnalyzeProcess,
    kGainProcess
};

//==============================================================================
/**
*/
class ShgainAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    ShgainAudioProcessor();
    ~ShgainAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void setProcess(int newprocess);

    void setInFile(File *f);
    float gain, peak, rms, sum, progress;
    int plugprocess;
    int inSamplePos;
    File *inFile;
    std::unique_ptr<AudioFormatReader> inFileReader;

private:
   //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
 
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShgainAudioProcessor)
};
