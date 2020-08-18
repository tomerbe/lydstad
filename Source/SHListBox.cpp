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
 : ListBox( String("List Box"), this ),
 thumbnailCache (5),
 thumbnail (512, formatManager, thumbnailCache)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setMultipleSelectionEnabled(true);
    formatManager.registerBasicFormats();
    somethingIsBeingDraggedOver = false;
    thumbnail.addChangeListener (this);
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
     
    somethingIsBeingDraggedOver = true;
    for(i = 0; i < files.size(); i++)
        addSoundFile(File(files[i]));
    updateContent();
    repaint();
}

void SHListBox::fileOpenPanel(void)
{
    FileChooser myChooser ("Please select the sounds you wish to add...",
                           File::getSpecialLocation (File::userHomeDirectory),
                           "*.wav;*.WAV;*.aiff;*.aif;*.mp3;*.MP3;*.flac;*.FLAC;*.oga;*.ogg");
    if (myChooser.browseForMultipleFilesToOpen())
    {
        Array<File> sfs (myChooser.getResults());
        for(int i = 0; i < sfs.size(); i++)
            addSoundFile(sfs[i]);
    }
    updateContent();
    repaint();
}

void    SHListBox::addSoundFile(File osf)
{
    File *sf;
    AudioFormatReader *afr;
    
    sf = new File(osf);
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

void     SHListBox::removeSoundFile()
{
    int i;
    File *sf;
    for(i = getNumSelectedRows() - 1; i >= 0 ; i--)
    {
        sf = soundfiles[getSelectedRow(i)];
        delete(sf);
        soundfiles.erase(soundfiles.begin() + getSelectedRow(i));
        rows--;
    }
    deselectAllRows();
    updateContent();
    repaint();
}

void    SHListBox::listBoxItemClicked (int row, const MouseEvent& m)
{

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

void SHListBox::changeListenerCallback(ChangeBroadcaster *source)
{
    repaint();
}

void SHListBox::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    float waveInc = 1.0f;
    Path waveBlob;
    float x, y;
    int32_t i;
    String filename;
    Font f(String("Trebuchet MS"), String("Regular"), (float)height);
    AudioFormatReader *afr;
    

    g.fillAll (Colours::black);
    
    g.setColour (Colours::black);
    g.fillRoundedRectangle (Rectangle<float>(1,1,width-2,height-2), 2);   // draw an outline around the component
    thumbnail.setSource (new FileInputSource (*(soundfiles[rowNumber])));    // [7]
    afr = formatManager.createReaderFor(*(soundfiles[rowNumber]));

    g.setColour(Colours::darkgrey);
    thumbnail.drawChannel (g,Rectangle<int>(-1,0,width,height*2), 0.0f, 1.5f, 0, 1.0f);
    thumbnail.drawChannel (g,Rectangle<int>(1,0,width,height*2), 0.0f, 1.5f, 0, 1.0f);
    thumbnail.drawChannel (g,Rectangle<int>(-1,height*2,width,height*-2), 0.0f, 1.5f, 0, 1.0f);
    thumbnail.drawChannel (g,Rectangle<int>(1,height*2,width,height*-2), 0.0f, 1.5f, 0, 1.0f);
    g.setColour(Colours::lightgrey);
    thumbnail.drawChannel (g,Rectangle<int>(0,0,width,height*2), 0.0f, 1.5f, 0, 1.0f);
    thumbnail.drawChannel (g,Rectangle<int>(0,height*2,width,height*-2), 0.0f, 1.5f, 0, 1.0f);

    g.setFont (f);
    filename = soundfiles[rowNumber]->getFileName();
    double seconds = afr->lengthInSamples/afr->sampleRate;
    filename = filename + String(" | (") + RelativeTime(seconds).getDescription()
        + String(") | ") + String(afr->sampleRate) + String(" sr | ")
        + String(afr->numChannels) + String(" ch");
    
    delete(afr);
    g.setColour (Colours::whitesmoke);
    if(rowIsSelected)
        g.setColour (Colour(177, 0, 28));
    g.drawText (String(rowNumber), Rectangle<int>(1,1,40,height-2), Justification::left, true);
    g.drawText (filename, Rectangle<int>(41,1,width-42,height-2), Justification::left, true);
    /*
    g.drawText (filename, Rectangle<int>(1-1,1-1,width-2,height-2), Justification::left, true);
    g.drawText (filename, Rectangle<int>(1-1,1+1,width-2,height-2), Justification::left, true);
    g.drawText (filename, Rectangle<int>(1+1,1+1,width-2,height-2), Justification::left, true);
    g.drawText (filename, Rectangle<int>(1+1,1-1,width-2,height-2), Justification::left, true);
    g.setColour (Colours::black);
    g.drawText (filename, Rectangle<int>(1,1,width-2,height-2), Justification::left, true);*/
    
}



