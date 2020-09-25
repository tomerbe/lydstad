/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "SHGPluginProcessor.h"
#include "SHGPluginEditor.h"

//==============================================================================
ShgainAudioProcessorEditor::ShgainAudioProcessorEditor (ShgainAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (640-18, 130-30);
    
    setName(String("gain"));
    // these define the parameters of our slider object
    gainSlider.setSliderStyle (juce::Slider::LinearBar);
    gainSlider.setRange (-120.0, 120.0, 1.0);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 90, 20);
    gainSlider.setPopupDisplayEnabled (true, false, this);
    gainSlider.setColour(gainSlider.backgroundColourId, Colours::black);
    gainSlider.setColour(gainSlider.thumbColourId, Colour(Colours::darkgrey).withAlpha(0.5f));
    gainSlider.setColour(gainSlider.trackColourId, Colour(Colours::darkgrey).withAlpha(0.5f));
    gainSlider.setColour(gainSlider.textBoxTextColourId, Colours::whitesmoke);
    gainSlider.setTextValueSuffix (" db");
    gainSlider.setValue(0.0);
 
    // this function adds the slider to the editor
    addAndMakeVisible (&gainSlider);
    // add the listener to the slider
    gainSlider.addListener (this);
    gainAttachment.reset (new SliderAttachment (valueTreeState, "gain", gainSlider));
 
    analyzeButton.reset (new TextButton ("analyze"));
    addAndMakeVisible (analyzeButton.get());
    analyzeButton->addListener (this);
    analyzeButton->setBounds (getWidth() - (getWidth()/5) , 0, getWidth()/5, getHeight()/4);

    cancelButton.reset (new TextButton ("cancel"));
    cancelButton->setClickingTogglesState(true);
    addAndMakeVisible (cancelButton.get());
    cancelButton->addListener (this);
    cancelButton->setBounds (getWidth() - (getWidth()/5) , getHeight()/2, getWidth()/5, getHeight()/4);
    cancelAttachment.reset (new ButtonAttachment (valueTreeState, "cancel", *cancelButton));

    gainButton.reset (new TextButton ("process"));
    addAndMakeVisible (gainButton.get());
    gainButton->addListener (this);
    gainButton->setBounds (getWidth() - (getWidth()/5) , getHeight()/2 + getHeight()/4, getWidth()/5, getHeight()/4);
    
    startTimer(200);
}

ShgainAudioProcessorEditor::~ShgainAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void ShgainAudioProcessorEditor::paint (Graphics& g)
{
    String text;
    Font f(String("Trebuchet MS"), String("Regular"), 18.0f);
// (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

     // fill the whole window white
    g.fillAll (Colours::black);
    // set the current drawing colour to black
    g.setColour (Colours::whitesmoke);
    g.drawRect(0, 0, getWidth(), getHeight());
    // set the font size and draw text to the screen
    g.setFont (f);
 
    text = String(log10(processor.peak) * 20.0f, 3, false) + String(" dB Peak Level");
    g.drawFittedText (text, 10, 0, (getWidth()-40)/3, 25, juce::Justification::left, 1);
    text = String(log10(processor.rms) * 20.0f, 3, false) + String(" dB RMS Level");
    g.drawFittedText (text, (getWidth()-20)/3 + 5, 0, (getWidth()-40)/3, 25, juce::Justification::left, 1);
    g.drawFittedText (String("Gain"), (getWidth()-20)/3 + 5, 25, (getWidth()-40)/3, 25, juce::Justification::left, 1);
    
    text = String(processor.progress, 3, false);
    g.drawFittedText (text, 0, 75, (getWidth()-40)/3, 25, juce::Justification::left, 1);

}
void ShgainAudioProcessorEditor::timerCallback()
{
//    if(processor.isSuspended() == false)
        repaint();
}

void ShgainAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    gainSlider.setBounds (10, 25, (getWidth()-40)/3, 25);
}

void ShgainAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    processor.gain = gainSlider.getValue();
}

void ShgainAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == cancelButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        if(cancelButton.get()->getToggleState() == true)
        {
        }
        //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == analyzeButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        processor.setProcess(kAnalyzeProcess);
        processor.suspendProcessing(false);
        //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == gainButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
       //[/UserButtonCode_toggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

