/*
  ==============================================================================

    SHListBox.cpp
    Created: 19 Apr 2018 9:43:40am
    Author:  Tom Erbe

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SHListBox.h"

//==============================================================================
SHListBox::SHListBox()
 : ListBox( String("List Box"), this ), state(Stopped)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    formatManager.registerBasicFormats();
    transportSource.addChangeListener (this); // [2]
    somethingIsBeingDraggedOver = false;
    rows = 0;
}

SHListBox::~SHListBox()
{
    for (File* sf : soundfiles)
        delete sf;
    soundfiles.clear();
}

bool     SHListBox::isInterestedInFileDrag (const StringArray &files)
{
    return(true);
}
void     SHListBox::filesDropped (const StringArray &files, int x, int y)
{
    int i;
    File *sf;
    AudioFormatReader *afr;
    
    somethingIsBeingDraggedOver = true;
    for(i = 0; i < files.size(); i++)
    {
        sf = new File(files[i]);
        afr = formatManager.createReaderFor(*sf);
        if(afr)
        {
            soundfiles.push_back(sf);
            rows++;
            delete(afr);
        }
        else
            delete(sf);
    }
    updateContent();
    repaint();
}
void    SHListBox::listBoxItemClicked (int row, const MouseEvent& m)
{
    float frac;
    frac = (float)(m.x) / getWidth();
 //   soundfiles[row]->SetPosition(frac);
    auto* reader = formatManager.createReaderFor (*soundfiles[row]);              // [10]
    if (reader != nullptr)
    {
        std::unique_ptr<AudioFormatReaderSource> newSource (new AudioFormatReaderSource (reader, true)); // [11]
        transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);                     // [12]
        readerSource.reset (newSource.release());
        changeState (Starting);
// [14]
    }
   repaintRow(row);
}

void    SHListBox::fileDragEnter (const StringArray& files, int x, int y)
{
    somethingIsBeingDraggedOver = true;
}
void    SHListBox::fileDragMove (const StringArray& files, int x, int y)
{
    somethingIsBeingDraggedOver = true;
}
void    SHListBox::fileDragExit (const StringArray& files)
{
    somethingIsBeingDraggedOver = true;
}
int SHListBox::getNumRows ()
{
    return(rows);
}

void SHListBox::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState (Playing);
        else
            changeState (Stopped);
    }
}

void SHListBox::changeState (TransportState newState)
{
    if (state != newState)
    {
        state = newState;
 
        switch (state)
        {
            case Stopped:                           // [3]
                transportSource.setPosition (0.0);
                break;
 
            case Starting:                          // [4]
                transportSource.start();
                break;
 
            case Playing:                           // [5]
                break;
 
            case Stopping:                          // [6]
                transportSource.stop();
                break;
        }
    }
}
void SHListBox::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    float waveInc = 1.0f;
    Path waveBlob;
    float x, y;
    int32_t i;
    String filename;
    Font f(String("Century Gothic"), String("Bold"), (float)height);
    
    g.fillAll (Colours::black);
    
    g.setColour (Colours::black);
    g.fillRoundedRectangle (Rectangle<float>(1,1,width-2,height-2), 2);   // draw an outline around the component

 /*   for(i = 0; i < width; i++)
    {
        if(soundfiles[rowNumber]->display[i] < 0.0f)
            soundfiles[rowNumber]->display[i] *= -1.0f;
    }

    waveBlob.clear();
    waveBlob.startNewSubPath(1.0f, height);
    y = height - (soundfiles[rowNumber]->display[0] * height);
    x = 0.0f;
    waveBlob.lineTo(x, y);
    for(i = 1; i < width; i++)
    {
        x += waveInc;
        y = height - (soundfiles[rowNumber]->display[i] * height);
        if(y > 0.0f)
                waveBlob.lineTo(x, y);
    }
    waveBlob.lineTo(width, height);
    waveBlob.lineTo(0.0f, height);
    waveBlob.closeSubPath();
     
    g.setColour(Colours::darkgrey);
    g.fillPath(waveBlob);
    g.strokePath(waveBlob, PathStrokeType(1.0f));*/

    g.setFont (f);
    //strcpy(filename, soundfiles[rowNumber]->filename);
    filename = String(rowNumber);
    filename += soundfiles[rowNumber]->getFileName();
    //sprintf(filename, "%c: my little file", rowNumber+65);
    g.setColour (Colours::whitesmoke);
    if(rowIsSelected)
        g.setColour (Colours::darkred);
    g.drawText (filename, Rectangle<int>(1-1,1-1,width-2,height-2), Justification::left, true);
    g.drawText (filename, Rectangle<int>(1-1,1+1,width-2,height-2), Justification::left, true);
    g.drawText (filename, Rectangle<int>(1+1,1+1,width-2,height-2), Justification::left, true);
    g.drawText (filename, Rectangle<int>(1+1,1-1,width-2,height-2), Justification::left, true);
    g.setColour (Colours::black);
    g.drawText (filename, Rectangle<int>(1,1,width-2,height-2), Justification::left, true);
       


}
