//
//  SHLAF.cpp
//  soundhack1 - App
//
//  Created by Tom Erbe on 8/6/20.
//

#include "SHLAF.hpp"

SH02LAF::SH02LAF()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

SH02LAF::~SH02LAF()
{
}

Font SH02LAF::getTextButtonFont (TextButton&, int buttonHeight)
{
    return Font (String("Trebuchet MS"), String("Regular"), 18.0f);
}
void SH02LAF::drawButtonText (Graphics& g, TextButton& t,
                     bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    Font font (getTextButtonFont (t, t.getHeight()));
    g.fillAll(Colours::black);
    g.setFont (font);
    g.setColour (Colours::whitesmoke);
    g.drawRoundedRectangle(0, 0, t.getWidth(), t.getHeight(), 0, 1);
    if(t.isMouseOver())
        g.setColour (Colour(177, 0, 28));
    g.drawFittedText (t.getButtonText(), 0, 0, t.getWidth(), t.getHeight(), Justification::centred, 2);

}

void SH02LAF::drawToggleButton (Graphics& g, ToggleButton& t,
                       bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    int colnum;
 
    juce::Array<juce::Colour> butCols {
        juce::Colour::fromRGBA (0, 0, 0, 255),   // black
        juce::Colour::fromRGBA (255, 255, 255, 255),   // white
        
        juce::Colour::fromRGBA (177, 0, 28, 127),   // red
        juce::Colour::fromRGBA (177, 0, 28, 255),
        
        juce::Colour::fromRGBA (35, 94, 0, 127),   // green
        juce::Colour::fromRGBA (35, 94, 0, 255),
        
        juce::Colour::fromRGBA (109, 80, 0, 127),   // yellow
        juce::Colour::fromRGBA (109, 80, 0, 255),
        
        juce::Colour::fromRGBA (49, 0, 103, 127),   // purple
        juce::Colour::fromRGBA (49, 0, 103, 255)
    };

    if(t.getName().equalsIgnoreCase("record"))
        colnum = 2;
    else if(t.getName().equalsIgnoreCase("play"))
        colnum = 4;
    else if(t.getName().equalsIgnoreCase("pause"))
        colnum = 6;
    else if(t.getName().equalsIgnoreCase("hack"))
        colnum = 8;
    else
        colnum = 0;
    
    if(t.getToggleState())
        g.setColour(butCols[colnum+1]);
    else
        g.setColour(butCols[colnum]);

    g.fillEllipse(0, 0, t.getWidth(), t.getHeight());
    
    g.setColour(Colour(51, 51, 51));
    g.fillEllipse(5, 5, t.getWidth()-10, t.getHeight()-10);
    
    if(t.getToggleState())
        g.setColour(butCols[colnum+1]);
    else
        g.setColour(butCols[colnum]);
    if(colnum > 8)
    {
        g.fillEllipse(10, 10, t.getWidth()-20, t.getHeight()-20);
    }
    else if (colnum == 2)
    {
        g.fillEllipse(15, 15, t.getWidth()-30, t.getHeight()-30);
        
    }
    else if (colnum == 4)
    {
        Path playArrow;
        playArrow.addTriangle((int)(0.3f * t.getWidth()), (int)(0.25f * t.getHeight()),
                              (int)(0.3f * t.getWidth()), (int)(0.75f * t.getHeight()),
                              (int)(0.8f * t.getWidth()), (int)(0.5f * t.getHeight()));
        g.fillPath(playArrow);
     }
    else if (colnum == 6)
    {
        g.fillRect(17, 17, t.getWidth()-34, t.getHeight()-34);
        g.setColour(Colour(51, 51, 51));
        g.fillRect(t.getWidth()/2 - 2, 17, 4, t.getHeight()-34);
    }
    else if (colnum == 8)
    {
//        g.fillEllipse(10, 10, t.getWidth()-20, t.getHeight()-20);
//        g.setColour(Colour(51, 51, 51));

        g.setFont(Font ("Apple Symbols", "Regular", 36.00f));
        g.drawText(String(CharPointer_UTF8("\xE2\x9A\x97")), 0, 0, t.getWidth(), t.getHeight(), juce::Justification::centred);
    }
    else
    {
        g.setFont(Font (String("Trebuchet MS"), String("Bold"), 36.00f));
        g.drawText(t.getName(), 5, 5, t.getWidth()-10, t.getHeight()-10, juce::Justification::centred);
    }
}
