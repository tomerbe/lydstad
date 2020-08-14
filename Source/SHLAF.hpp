//
//  SHLAF.hpp
//  soundhack1 - App
//
//  Created by Tom Erbe on 8/6/20.
//

#ifndef SHLAF_hpp
#define SHLAF_hpp

#include <stdio.h>
#include "../JuceLibraryCode/JuceHeader.h"

class SH02LAF    : public LookAndFeel_V3
{
public:
    SH02LAF();
    ~SH02LAF();
    
    void drawToggleButton (Graphics&, ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SH02LAF)
};

#endif /* SHLAF_hpp */
