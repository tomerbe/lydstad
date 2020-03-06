/*
 ==============================================================================
 
 SHSoundfileLBM.cpp
 Created: 16 Apr 2018 11:18:05am
 Author:  Tom Erbe
 
 ==============================================================================
 */

#include "../JuceLibraryCode/JuceHeader.h"
#include "SHSoundfileLBM.h"

//==============================================================================
SHSoundfileLBM::SHSoundfileLBM(SHListBox& flb)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    rows = 0;
    fileListBox = &flb;
}

SHSoundfileLBM::~SHSoundfileLBM()
{
}

int SHSoundfileLBM::getNumRows ()
{
    return(rows);
}

void SHSoundfileLBM::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    char filename[256];
    
    g.fillAll (Colours::olive);

    g.setColour (Colours::grey);
    g.drawRect (Rectangle<int>(0,0,width,height), 1);   // draw an outline around the component
    
    g.setFont (Font (16.0f));
    g.setColour (Colours::white);
    strcpy(filename, fileListBox->soundfiles[rowNumber]->name);
    //sprintf(filename, "%c: my little file", rowNumber+65);
    g.drawText (filename, Rectangle<int>(1,1,width-1,height-1), Justification::bottomLeft, true);
}
