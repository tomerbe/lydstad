/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "SHGPluginProcessor.h"
#include "SHGPluginEditor.h"

//==============================================================================
ShgainAudioProcessor::ShgainAudioProcessor()
    :
#ifndef JucePlugin_PreferredChannelConfigurations
     AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters (*this, nullptr, juce::Identifier ("shgain"),
    {
        std::make_unique<juce::AudioParameterBool> ("cancel", "Cancel", false),              // default value
        std::make_unique<juce::AudioParameterFloat> ("gain", "Gain",  -120.0f, 120.0f, 0.0f)             // default value
    })
{
    peak = rms = 0.0f;
    plugprocess = 0;
    suspendProcessing(true);
}

ShgainAudioProcessor::~ShgainAudioProcessor()
{
}

//==============================================================================
const String ShgainAudioProcessor::getName() const
{
    return String("SHGain");
}

void ShgainAudioProcessor::setInFile(File *f)
{
    inFile = f;
}

bool ShgainAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ShgainAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ShgainAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ShgainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ShgainAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ShgainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ShgainAudioProcessor::setCurrentProgram (int index)
{
}

const String ShgainAudioProcessor::getProgramName (int index)
{
    return {};
}

void ShgainAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ShgainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    AudioFormatManager fm;
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    fm.registerBasicFormats();
    inFileReader.reset(fm.createReaderFor(*inFile));
    inSamplePos = 0;
    progress = peak = rms = sum = 0.0f;
}

void ShgainAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    suspendProcessing(true);
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShgainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void ShgainAudioProcessor::setProcess(int newprocess)
{
    plugprocess = newprocess;
}

void ShgainAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    if(plugprocess == kAnalyzeProcess)
    {
        inFileReader->read(&buffer, 0, buffer.getNumSamples(), inSamplePos, true, true);
        inSamplePos +=  buffer.getNumSamples();
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getReadPointer (channel);
            for(auto sample = 0; sample < buffer.getNumSamples(); sample++)
            {
                if(channelData[sample] > peak)
                    peak = channelData[sample];
                if(-channelData[sample] > peak)
                    peak = -channelData[sample];
                sum += channelData[sample] * channelData[sample];
                    
            }

        }
        progress = (100.0f * inSamplePos)/inFileReader->lengthInSamples;
        rms = sqrt(sum/(inSamplePos * totalNumInputChannels));
        if(inSamplePos >= inFileReader->lengthInSamples)
        {
            progress = 100.0f;
            // this signals to the parent that we are done with the current file
            suspendProcessing(true);
        }
    }
}

//==============================================================================
bool ShgainAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ShgainAudioProcessor::createEditor()
{
    return new ShgainAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void ShgainAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ShgainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShgainAudioProcessor();
}
