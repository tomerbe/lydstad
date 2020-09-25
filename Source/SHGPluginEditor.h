/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SHGPluginProcessor.h"

//==============================================================================
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
/**
*/
class ShgainAudioProcessorEditor  : public AudioProcessorEditor,
    private juce::Slider::Listener,
    private juce::Button::Listener,
    private Timer
{
public:
    ShgainAudioProcessorEditor (ShgainAudioProcessor&, AudioProcessorValueTreeState& vts);
    ~ShgainAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void sliderValueChanged (juce::Slider* slider) override; // [3]
    void buttonClicked (Button* buttonThatWasClicked) override;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ShgainAudioProcessor& processor;
    
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    Slider gainSlider;
    
    std::unique_ptr<TextButton> cancelButton;
    std::unique_ptr<TextButton> analyzeButton;
    std::unique_ptr<TextButton> gainButton;
    
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> cancelAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShgainAudioProcessorEditor)
};
