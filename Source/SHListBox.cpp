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
 : ListBox( String("List Box"), this )
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    somethingIsBeingDraggedOver = false;
    rows = 0;
}

SHListBox::~SHListBox()
{
}

bool     SHListBox::isInterestedInFileDrag (const StringArray &files)
{
    return(true);
}
void     SHListBox::filesDropped (const StringArray &files, int x, int y)
{
    SHSoundFile *sf;
    char filename[256];
    
    sf = new SHSoundFile();
    somethingIsBeingDraggedOver = true;
    strcpy(filename, files.begin()->toUTF8());
    if(sf->Open(filename) == 0)
    {
        soundfiles.push_back(sf);
        rows++;
    }
    else
        delete(sf);
    updateContent();
    repaint();
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

void SHListBox::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    float waveInc = 1.0f;
    Path waveBlob;
    float x, y;
    int32_t i;
    char filename[256];
    Font f(String("Century Gothic"), String("Bold"), 40.0f );
    
    g.fillAll (Colours::black);
    
    g.setColour (Colours::black);
    g.fillRoundedRectangle (Rectangle<float>(1,1,width-2,height-2), 2);   // draw an outline around the component

    for(i = 0; i < width; i++)
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
    g.strokePath(waveBlob, PathStrokeType(1.0f));

    g.setFont (f);
    g.setColour (Colours::darkred);
    strcpy(filename, soundfiles[rowNumber]->filename);
    //sprintf(filename, "%c: my little file", rowNumber+65);
    g.setColour (Colours::whitesmoke);
    g.drawText (filename, Rectangle<int>(1-1,1-1,width-2,height-2), Justification::centred, true);
    g.drawText (filename, Rectangle<int>(1-1,1+1,width-2,height-2), Justification::centred, true);
    g.drawText (filename, Rectangle<int>(1+1,1+1,width-2,height-2), Justification::centred, true);
    g.drawText (filename, Rectangle<int>(1+1,1-1,width-2,height-2), Justification::centred, true);
    g.setColour (Colours::black);
    g.drawText (filename, Rectangle<int>(1,1,width-2,height-2), Justification::centred, true);
       


}
