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
    lastRowDrawn = -1;
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
}

void SHListBox::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    String filename, fileprop;
    Font f(String("Trebuchet MS"), String("Regular"), 18.0);
    AudioFormatReader *afr;
    float startSec;
    int min, sec, msec;
    

    g.fillAll (Colours::black);
    g.setColour (Colours::black);
    g.fillRoundedRectangle (Rectangle<float>(1,1,width-2,height-2), 2);   // draw an outline around the component
    thumbnail.setSource (new FileInputSource (*(soundfiles[rowNumber])));
    afr = formatManager.createReaderFor(*(soundfiles[rowNumber]));

    if(getSelectedRow(playRow) == rowNumber)
    {
        startSec = playSecond;
    }
    else
        startSec = 0.0f;
    g.setColour(Colour(Colours::darkgrey).withAlpha(0.5f));
    thumbnail.drawChannel (g,Rectangle<int>(0,0,width,height*2), startSec, startSec+1.5f, 0, 1.0f);
    thumbnail.drawChannel (g,Rectangle<int>(0,0,width,height*2), startSec, startSec+1.5f, 1, 1.0f);

    g.setFont (f);
    g.setColour (Colours::whitesmoke);
    if(rowIsSelected)
        g.setColour (Colour(177, 0, 28));
    g.drawText (String(rowNumber), Rectangle<int>(0,0,40,height-2), Justification::left, true);
    
    fileprop = String(afr->numChannels) + String(" ch");
    g.drawText (fileprop, Rectangle<int>(width-60,0,60,height-2), Justification::left, true);
    
    fileprop = String(afr->sampleRate) + String(" sr");
    g.drawText (fileprop, Rectangle<int>(width-140,0,80,height-2), Justification::left, true);
    
    min = afr->lengthInSamples/(afr->sampleRate * 60);
    sec = afr->lengthInSamples/afr->sampleRate - (min * 60);
    msec = (1000 * afr->lengthInSamples/afr->sampleRate) - (((min * 60) + sec) * 1000);
    fileprop = String(min).paddedLeft('0',2) + String(":") + String(sec).paddedLeft('0',2) + String(".") + String(msec).paddedLeft('0',3);
 //   fileprop = Time(afr->lengthInSamples/afr->sampleRate).toString(false, true, true, true);
    g.drawText (fileprop, Rectangle<int>(width-260,0,120,height-2), Justification::left, true);

    filename = soundfiles[rowNumber]->getFileName();
    g.drawText (filename, Rectangle<int>(30,0,width-270,height-2), Justification::left, true);
    delete(afr);
}



