/*
  ==============================================================================

    soundfileLBM.h
    Created: 16 Apr 2018 12:55:34pm
    Author:  Tom Erbe

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "SHListBox.h"

//==============================================================================
/*
 */
class SHSoundfileLBM    : public ListBoxModel
{
public:
    SHSoundfileLBM(SHListBox &flb);
    ~SHSoundfileLBM();
    
    SHListBox *fileListBox;
    int getNumRows () override;
    void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;
    int rows;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SHSoundfileLBM)
};
