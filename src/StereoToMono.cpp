//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2020 Julius Smith, Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file StereoToMono.cpp
 * \author Dominick Hing
 * \date February 2023
 */

#include "StereoToMono.h"

#include <iostream>

#include "jacktrip_types.h"
#include "stereotomonodsp.h"

//*******************************************************************************
StereoToMono::StereoToMono(bool verboseFlag)
{
    setVerbose(verboseFlag);
    stereoToMonoP = new stereotomonodsp;
}

//*******************************************************************************
StereoToMono::~StereoToMono()
{
    delete static_cast<stereotomonodsp*>(stereoToMonoP);
}

//*******************************************************************************
void StereoToMono::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);

    fs = float(mSampleRate);
    static_cast<stereotomonodsp*>(stereoToMonoP)->init(fs);

    inited = true;
}

//*******************************************************************************
void StereoToMono::compute(int nframes, float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** Stereo-to-Mono " << this
                  << ": init never called! Doing it now.\n";
        init(0, 0);
    }
    static_cast<stereotomonodsp*>(stereoToMonoP)->compute(nframes, inputs, outputs);
}