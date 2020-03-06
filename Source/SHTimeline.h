/*
  ==============================================================================

    SHTimeline.h
    Created: 16 Apr 2018 11:18:05am
    Author:  Tom Erbe

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class SHTimeline    : public Component, public FileDragAndDropTarget
{
public:
    SHTimeline();
    ~SHTimeline();

    void paint (Graphics&) override;
    void resized() override;
    bool     isInterestedInFileDrag (const StringArray &files) override;
    void     filesDropped (const StringArray &files, int x, int y) override;
    void    fileDragEnter (const StringArray& files, int x, int y) override;
    void    fileDragMove (const StringArray& files, int x, int y) override;
    void    fileDragExit (const StringArray& files) override;
    
    bool somethingIsBeingDraggedOver;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SHTimeline)
};
