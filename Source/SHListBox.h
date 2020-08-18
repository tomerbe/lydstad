/*
  ==============================================================================

    SHListBox.h
    Created: 19 Apr 2018 9:43:40am
    Author:  Tom Erbe

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "./s_soundfile/SHSoundfile.h"

//==============================================================================
/*
*/


class SHListBox    : public ListBox,
    public ListBoxModel,
    public ChangeListener,
    public FileDragAndDropTarget
{
public:
    SHListBox();
    ~SHListBox();
    
    void    listBoxItemClicked (int row, const MouseEvent&) override;
    bool    isInterestedInFileDrag (const StringArray &files) override;
    void    filesDropped (const StringArray &files, int x, int y) override;
    void    fileDragEnter (const StringArray& files, int x, int y) override;
    void    fileDragMove (const StringArray& files, int x, int y) override;
    void    fileDragExit (const StringArray& files) override;
    int     getNumRows () override;
    void    paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;
    void    changeListenerCallback (ChangeBroadcaster* source) override;
    void    fileOpenPanel(void);
    void    addSoundFile(File sf);
    void    removeSoundFile(void);

    int     rows;
    
    bool somethingIsBeingDraggedOver;
    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;                            // [2]
 
    std::vector<File *>soundfiles;
    juce_UseDebuggingNewOperator
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SHListBox)
};
