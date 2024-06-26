/* ------------------------------------------------------------
name: "zitarevdsp"
Code generated with Faust 2.41.1 (https://faust.grame.fr)
Compilation options: -a faust2header.cpp -lang cpp -i -inpl -cn zitarevdsp -es 1 -mcd 16
-single -ftz 0
------------------------------------------------------------ */

#ifndef __zitarevdsp_H__
#define __zitarevdsp_H__

// NOTE: ANY INCLUDE-GUARD HERE MUST BE DERIVED FROM THE CLASS NAME
//
// faust2header.cpp - FAUST Architecture File
// This is a simple variation of matlabplot.cpp in the Faust distribution
// aimed at creating a simple C++ header file (.h) containing a Faust DSP.
// See the Makefile for how to use it.

/************************** BEGIN dsp.h ********************************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef __dsp__
#define __dsp__

#include <string>
#include <vector>

/************************************************************************
 ************************************************************************
    FAUST compiler
    Copyright (C) 2003-2018 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ************************************************************************
 ************************************************************************/

#ifndef __export__
#define __export__

#define FAUSTVERSION "2.41.1"

// Use FAUST_API for code that is part of the external API but is also compiled in faust
// and libfaust Use LIBFAUST_API for code that is compiled in faust and libfaust

#ifdef _WIN32
#pragma warning(disable : 4251)
#ifdef FAUST_EXE
#define FAUST_API
#define LIBFAUST_API
#elif FAUST_LIB
#define FAUST_API    __declspec(dllexport)
#define LIBFAUST_API __declspec(dllexport)
#else
#define FAUST_API
#define LIBFAUST_API
#endif
#else
#ifdef FAUST_EXE
#define FAUST_API
#define LIBFAUST_API
#else
#define FAUST_API    __attribute__((visibility("default")))
#define LIBFAUST_API __attribute__((visibility("default")))
#endif
#endif

#endif

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

struct FAUST_API UI;
struct FAUST_API Meta;

/**
 * DSP memory manager.
 */

struct FAUST_API dsp_memory_manager {
    virtual ~dsp_memory_manager() {}

    /**
     * Inform the Memory Manager with the number of expected memory zones.
     * @param count - the number of expected memory zones
     */
    virtual void begin(size_t /*count*/) {}

    /**
     * Give the Memory Manager information on a given memory zone.
     * @param size - the size in bytes of the memory zone
     * @param reads - the number of Read access to the zone used to compute one frame
     * @param writes - the number of Write access to the zone used to compute one frame
     */
    virtual void info(size_t /*size*/, size_t /*reads*/, size_t /*writes*/) {}

    /**
     * Inform the Memory Manager that all memory zones have been described,
     * to possibly start a 'compute the best allocation strategy' step.
     */
    virtual void end() {}

    /**
     * Allocate a memory zone.
     * @param size - the memory zone size in bytes
     */
    virtual void* allocate(size_t size) = 0;

    /**
     * Destroy a memory zone.
     * @param ptr - the memory zone pointer to be deallocated
     */
    virtual void destroy(void* ptr) = 0;
};

/**
 * Signal processor definition.
 */

class FAUST_API dsp
{
   public:
    dsp() {}
    virtual ~dsp() {}

    /* Return instance number of audio inputs */
    virtual int getNumInputs() = 0;

    /* Return instance number of audio outputs */
    virtual int getNumOutputs() = 0;

    /**
     * Trigger the ui_interface parameter with instance specific calls
     * to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the UI.
     *
     * @param ui_interface - the user interface builder
     */
    virtual void buildUserInterface(UI* ui_interface) = 0;

    /* Return the sample rate currently used by the instance */
    virtual int getSampleRate() = 0;

    /**
     * Global init, calls the following methods:
     * - static class 'classInit': static tables initialization
     * - 'instanceInit': constants and instance state initialization
     *
     * @param sample_rate - the sampling rate in Hz
     */
    virtual void init(int sample_rate) = 0;

    /**
     * Init instance state
     *
     * @param sample_rate - the sampling rate in Hz
     */
    virtual void instanceInit(int sample_rate) = 0;

    /**
     * Init instance constant state
     *
     * @param sample_rate - the sampling rate in Hz
     */
    virtual void instanceConstants(int sample_rate) = 0;

    /* Init default control parameters values */
    virtual void instanceResetUserInterface() = 0;

    /* Init instance state (like delay lines...) but keep the control parameter values */
    virtual void instanceClear() = 0;

    /**
     * Return a clone of the instance.
     *
     * @return a copy of the instance on success, otherwise a null pointer.
     */
    virtual dsp* clone() = 0;

    /**
     * Trigger the Meta* parameter with instance specific calls to 'declare' (key, value)
     * metadata.
     *
     * @param m - the Meta* meta user
     */
    virtual void metadata(Meta* m) = 0;

    /**
     * DSP instance computation, to be called with successive in/out audio buffers.
     *
     * @param count - the number of frames to compute
     * @param inputs - the input audio buffers as an array of non-interleaved FAUSTFLOAT
     * samples (eiher float, double or quad)
     * @param outputs - the output audio buffers as an array of non-interleaved FAUSTFLOAT
     * samples (eiher float, double or quad)
     *
     */
    virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;

    /**
     * DSP instance computation: alternative method to be used by subclasses.
     *
     * @param date_usec - the timestamp in microsec given by audio driver.
     * @param count - the number of frames to compute
     * @param inputs - the input audio buffers as an array of non-interleaved FAUSTFLOAT
     * samples (either float, double or quad)
     * @param outputs - the output audio buffers as an array of non-interleaved FAUSTFLOAT
     * samples (either float, double or quad)
     *
     */
    virtual void compute(double /*date_usec*/, int count, FAUSTFLOAT** inputs,
                         FAUSTFLOAT** outputs)
    {
        compute(count, inputs, outputs);
    }
};

/**
 * Generic DSP decorator.
 */

class FAUST_API decorator_dsp : public dsp
{
   protected:
    dsp* fDSP;

   public:
    decorator_dsp(dsp* dsp = nullptr) : fDSP(dsp) {}
    virtual ~decorator_dsp() { delete fDSP; }

    virtual int getNumInputs() { return fDSP->getNumInputs(); }
    virtual int getNumOutputs() { return fDSP->getNumOutputs(); }
    virtual void buildUserInterface(UI* ui_interface)
    {
        fDSP->buildUserInterface(ui_interface);
    }
    virtual int getSampleRate() { return fDSP->getSampleRate(); }
    virtual void init(int sample_rate) { fDSP->init(sample_rate); }
    virtual void instanceInit(int sample_rate) { fDSP->instanceInit(sample_rate); }
    virtual void instanceConstants(int sample_rate)
    {
        fDSP->instanceConstants(sample_rate);
    }
    virtual void instanceResetUserInterface() { fDSP->instanceResetUserInterface(); }
    virtual void instanceClear() { fDSP->instanceClear(); }
    virtual decorator_dsp* clone() { return new decorator_dsp(fDSP->clone()); }
    virtual void metadata(Meta* m) { fDSP->metadata(m); }
    // Beware: subclasses usually have to overload the two 'compute' methods
    virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
    {
        fDSP->compute(count, inputs, outputs);
    }
    virtual void compute(double date_usec, int count, FAUSTFLOAT** inputs,
                         FAUSTFLOAT** outputs)
    {
        fDSP->compute(date_usec, count, inputs, outputs);
    }
};

/**
 * DSP factory class, used with LLVM and Interpreter backends
 * to create DSP instances from a compiled DSP program.
 */

class FAUST_API dsp_factory
{
   protected:
    // So that to force sub-classes to use deleteDSPFactory(dsp_factory* factory);
    virtual ~dsp_factory() {}

   public:
    virtual std::string getName()                          = 0;
    virtual std::string getSHAKey()                        = 0;
    virtual std::string getDSPCode()                       = 0;
    virtual std::string getCompileOptions()                = 0;
    virtual std::vector<std::string> getLibraryList()      = 0;
    virtual std::vector<std::string> getIncludePathnames() = 0;

    virtual dsp* createDSPInstance() = 0;

    virtual void setMemoryManager(dsp_memory_manager* manager) = 0;
    virtual dsp_memory_manager* getMemoryManager()             = 0;
};

// Denormal handling

#if defined(__SSE__)
#include <xmmintrin.h>
#endif

class FAUST_API ScopedNoDenormals
{
   private:
    intptr_t fpsr;

    void setFpStatusRegister(intptr_t fpsr_aux) noexcept
    {
#if defined(__arm64__) || defined(__aarch64__)
        asm volatile("msr fpcr, %0" : : "ri"(fpsr_aux));
#elif defined(__SSE__)
        _mm_setcsr(static_cast<uint32_t>(fpsr_aux));
#endif
    }

    void getFpStatusRegister() noexcept
    {
#if defined(__arm64__) || defined(__aarch64__)
        asm volatile("mrs %0, fpcr" : "=r"(fpsr));
#elif defined(__SSE__)
        fpsr          = static_cast<intptr_t>(_mm_getcsr());
#endif
    }

   public:
    ScopedNoDenormals() noexcept
    {
#if defined(__arm64__) || defined(__aarch64__)
        intptr_t mask = (1 << 24 /* FZ */);
#else
#if defined(__SSE__)
#if defined(__SSE2__)
        intptr_t mask = 0x8040;
#else
        intptr_t mask = 0x8000;
#endif
#else
        intptr_t mask = 0x0000;
#endif
#endif
        getFpStatusRegister();
        setFpStatusRegister(fpsr | mask);
    }

    ~ScopedNoDenormals() noexcept { setFpStatusRegister(fpsr); }
};

#define AVOIDDENORMALS ScopedNoDenormals();

#endif

/************************** END dsp.h **************************/
/************************** BEGIN APIUI.h *****************************
FAUST Architecture File
Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
---------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

EXCEPTION : As a special exception, you may create a larger work
that contains this FAUST architecture section and distribute
that work under terms of your choice, so long as this FAUST
architecture section is not modified.
************************************************************************/

#ifndef API_UI_H
#define API_UI_H

#include <stdio.h>

#include <map>
#include <sstream>
#include <string>
#include <vector>

/************************** BEGIN meta.h *******************************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef __meta__
#define __meta__

/**
 The base class of Meta handler to be used in dsp::metadata(Meta* m) method to retrieve
 (key, value) metadata.
 */
struct FAUST_API Meta {
    virtual ~Meta() {}
    virtual void declare(const char* key, const char* value) = 0;
};

#endif
/**************************  END  meta.h **************************/
/************************** BEGIN UI.h *****************************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ********************************************************************/

#ifndef __UI_H__
#define __UI_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

/*******************************************************************************
 * UI : Faust DSP User Interface
 * User Interface as expected by the buildUserInterface() method of a DSP.
 * This abstract class contains only the method that the Faust compiler can
 * generate to describe a DSP user interface.
 ******************************************************************************/

struct Soundfile;

template<typename REAL>
struct FAUST_API UIReal {
    UIReal() {}
    virtual ~UIReal() {}

    // -- widget's layouts

    virtual void openTabBox(const char* label)        = 0;
    virtual void openHorizontalBox(const char* label) = 0;
    virtual void openVerticalBox(const char* label)   = 0;
    virtual void closeBox()                           = 0;

    // -- active widgets

    virtual void addButton(const char* label, REAL* zone)      = 0;
    virtual void addCheckButton(const char* label, REAL* zone) = 0;
    virtual void addVerticalSlider(const char* label, REAL* zone, REAL init, REAL min,
                                   REAL max, REAL step)        = 0;
    virtual void addHorizontalSlider(const char* label, REAL* zone, REAL init, REAL min,
                                     REAL max, REAL step)      = 0;
    virtual void addNumEntry(const char* label, REAL* zone, REAL init, REAL min, REAL max,
                             REAL step)                        = 0;

    // -- passive widgets

    virtual void addHorizontalBargraph(const char* label, REAL* zone, REAL min,
                                       REAL max) = 0;
    virtual void addVerticalBargraph(const char* label, REAL* zone, REAL min,
                                     REAL max)   = 0;

    // -- soundfiles

    virtual void addSoundfile(const char* /*label*/, const char* /*filename*/,
                              Soundfile** /*sf_zone*/) = 0;

    // -- metadata declarations

    virtual void declare(REAL* /*zone*/, const char* /*key*/, const char* /*val*/) {}

    // To be used by LLVM client
    virtual int sizeOfFAUSTFLOAT() { return sizeof(FAUSTFLOAT); }
};

struct FAUST_API UI : public UIReal<FAUSTFLOAT> {
    UI() {}
    virtual ~UI() {}
};

#endif
/**************************  END  UI.h **************************/
/************************** BEGIN PathBuilder.h **************************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef __PathBuilder__
#define __PathBuilder__

#include <algorithm>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

/*******************************************************************************
 * PathBuilder : Faust User Interface
 * Helper class to build complete hierarchical path for UI items.
 ******************************************************************************/

class FAUST_API PathBuilder
{
   protected:
    std::vector<std::string> fControlsLevel;
    std::vector<std::string> fFullPaths;
    std::map<std::string, std::string> fFull2Short;  // filled by computeShortNames()

    /**
     * @brief check if a character is acceptable for an ID
     *
     * @param c
     * @return true is the character is acceptable for an ID
     */
    bool isIDChar(char c) const
    {
        return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))
               || ((c >= '0') && (c <= '9'));
    }

    /**
     * @brief remove all "/0x00" parts
     *
     * @param src
     * @return modified string
     */
    std::string remove0x00(const std::string& src) const
    {
        return std::regex_replace(src, std::regex("/0x00"), "");
    }

    /**
     * @brief replace all non ID char with '_' (one '_' may replace several non ID char)
     *
     * @param src
     * @return modified string
     */
    std::string str2ID(const std::string& src) const
    {
        std::string dst;
        bool need_underscore = false;
        for (char c : src) {
            if (isIDChar(c) || (c == '/')) {
                if (need_underscore) {
                    dst.push_back('_');
                    need_underscore = false;
                }
                dst.push_back(c);
            } else {
                need_underscore = true;
            }
        }
        return dst;
    }

    /**
     * @brief Keep only the last n slash-parts
     *
     * @param src
     * @param n : 1 indicates the last slash-part
     * @return modified string
     */
    std::string cut(const std::string& src, int n) const
    {
        std::string rdst;
        for (int i = int(src.length()) - 1; i >= 0; i--) {
            char c = src[i];
            if (c != '/') {
                rdst.push_back(c);
            } else if (n == 1) {
                std::string dst;
                for (int j = int(rdst.length()) - 1; j >= 0; j--) {
                    dst.push_back(rdst[j]);
                }
                return dst;
            } else {
                n--;
                rdst.push_back(c);
            }
        }
        return src;
    }

    void addFullPath(const std::string& label) { fFullPaths.push_back(buildPath(label)); }

    /**
     * @brief Compute the mapping between full path and short names
     */
    void computeShortNames()
    {
        std::vector<std::string>
            uniquePaths;  // all full paths transformed but made unique with a prefix
        std::map<std::string, std::string>
            unique2full;  // all full paths transformed but made unique with a prefix
        char num_buffer[16];
        int pnum = 0;

        for (const auto& s : fFullPaths) {
            sprintf(num_buffer, "%d", pnum++);
            std::string u = "/P" + std::string(num_buffer) + str2ID(remove0x00(s));
            uniquePaths.push_back(u);
            unique2full[u] = s;  // remember the full path associated to a unique path
        }

        std::map<std::string, int> uniquePath2level;  // map path to level
        for (const auto& s : uniquePaths)
            uniquePath2level[s] = 1;  // we init all levels to 1
        bool have_collisions = true;

        while (have_collisions) {
            // compute collision list
            std::set<std::string> collisionSet;
            std::map<std::string, std::string> short2full;
            have_collisions = false;
            for (const auto& it : uniquePath2level) {
                std::string u         = it.first;
                int n                 = it.second;
                std::string shortName = cut(u, n);
                auto p                = short2full.find(shortName);
                if (p == short2full.end()) {
                    // no collision
                    short2full[shortName] = u;
                } else {
                    // we have a collision, add the two paths to the collision set
                    have_collisions = true;
                    collisionSet.insert(u);
                    collisionSet.insert(p->second);
                }
            }
            for (const auto& s : collisionSet)
                uniquePath2level[s]++;  // increase level of colliding path
        }

        for (const auto& it : uniquePath2level) {
            std::string u               = it.first;
            int n                       = it.second;
            std::string shortName       = replaceCharList(cut(u, n), {'/'}, '_');
            fFull2Short[unique2full[u]] = shortName;
        }
    }

    std::string replaceCharList(const std::string& str, const std::vector<char>& ch1,
                                char ch2)
    {
        auto beg        = ch1.begin();
        auto end        = ch1.end();
        std::string res = str;
        for (size_t i = 0; i < str.length(); ++i) {
            if (std::find(beg, end, str[i]) != end)
                res[i] = ch2;
        }
        return res;
    }

   public:
    PathBuilder() {}
    virtual ~PathBuilder() {}

    // Return true for the first level of groups
    bool pushLabel(const std::string& label)
    {
        fControlsLevel.push_back(label);
        return fControlsLevel.size() == 1;
    }

    // Return true for the last level of groups
    bool popLabel()
    {
        fControlsLevel.pop_back();
        return fControlsLevel.size() == 0;
    }

    std::string buildPath(const std::string& label)
    {
        std::string res = "/";
        for (size_t i = 0; i < fControlsLevel.size(); i++) {
            res = res + fControlsLevel[i] + "/";
        }
        res += label;
        return replaceCharList(
            res, {' ', '#', '*', ',', '?', '[', ']', '{', '}', '(', ')'}, '_');
    }
};

#endif  // __PathBuilder__
/**************************  END  PathBuilder.h **************************/
/************************** BEGIN ValueConverter.h ********************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ********************************************************************/

#ifndef __ValueConverter__
#define __ValueConverter__

/***************************************************************************************
 ValueConverter.h
 (GRAME, Copyright 2015-2019)

 Set of conversion objects used to map user interface values (for example a gui slider
 delivering values between 0 and 1) to faust values (for example a vslider between
 20 and 20000) using a log scale.

 -- Utilities

 Range(lo,hi) : clip a value x between lo and hi
 Interpolator(lo,hi,v1,v2) : Maps a value x between lo and hi to a value y between v1 and
v2 Interpolator3pt(lo,mi,hi,v1,vm,v2) : Map values between lo mid hi to values between v1
vm v2

 -- Value Converters

 ValueConverter::ui2faust(x)
 ValueConverter::faust2ui(x)

 -- ValueConverters used for sliders depending of the scale

 LinearValueConverter(umin, umax, fmin, fmax)
 LinearValueConverter2(lo, mi, hi, v1, vm, v2) using 2 segments
 LogValueConverter(umin, umax, fmin, fmax)
 ExpValueConverter(umin, umax, fmin, fmax)

 -- ValueConverters used for accelerometers based on 3 points

 AccUpConverter(amin, amid, amax, fmin, fmid, fmax)        -- curve 0
 AccDownConverter(amin, amid, amax, fmin, fmid, fmax)      -- curve 1
 AccUpDownConverter(amin, amid, amax, fmin, fmid, fmax)    -- curve 2
 AccDownUpConverter(amin, amid, amax, fmin, fmid, fmax)    -- curve 3

 -- lists of ZoneControl are used to implement accelerometers metadata for each axes

 ZoneControl(zone, valueConverter) : a zone with an accelerometer data converter

 -- ZoneReader are used to implement screencolor metadata

 ZoneReader(zone, valueConverter) : a zone with a data converter

****************************************************************************************/

#include <assert.h>
#include <float.h>

#include <algorithm>  // std::max
#include <cmath>
#include <vector>

//--------------------------------------------------------------------------------------
// Interpolator(lo,hi,v1,v2)
// Maps a value x between lo and hi to a value y between v1 and v2
// y = v1 + (x-lo)/(hi-lo)*(v2-v1)
// y = v1 + (x-lo) * coef           with coef = (v2-v1)/(hi-lo)
// y = v1 + x*coef - lo*coef
// y = v1 - lo*coef + x*coef
// y = offset + x*coef              with offset = v1 - lo*coef
//--------------------------------------------------------------------------------------
class FAUST_API Interpolator
{
   private:
    //--------------------------------------------------------------------------------------
    // Range(lo,hi) clip a value between lo and hi
    //--------------------------------------------------------------------------------------
    struct Range {
        double fLo;
        double fHi;

        Range(double x, double y)
            : fLo(std::min<double>(x, y)), fHi(std::max<double>(x, y))
        {
        }
        double operator()(double x) { return (x < fLo) ? fLo : (x > fHi) ? fHi : x; }
    };

    Range fRange;
    double fCoef;
    double fOffset;

   public:
    Interpolator(double lo, double hi, double v1, double v2) : fRange(lo, hi)
    {
        if (hi != lo) {
            // regular case
            fCoef   = (v2 - v1) / (hi - lo);
            fOffset = v1 - lo * fCoef;
        } else {
            // degenerate case, avoids division by zero
            fCoef   = 0;
            fOffset = (v1 + v2) / 2;
        }
    }
    double operator()(double v)
    {
        double x = fRange(v);
        return fOffset + x * fCoef;
    }

    void getLowHigh(double& amin, double& amax)
    {
        amin = fRange.fLo;
        amax = fRange.fHi;
    }
};

//--------------------------------------------------------------------------------------
// Interpolator3pt(lo,mi,hi,v1,vm,v2)
// Map values between lo mid hi to values between v1 vm v2
//--------------------------------------------------------------------------------------
class FAUST_API Interpolator3pt
{
   private:
    Interpolator fSegment1;
    Interpolator fSegment2;
    double fMid;

   public:
    Interpolator3pt(double lo, double mi, double hi, double v1, double vm, double v2)
        : fSegment1(lo, mi, v1, vm), fSegment2(mi, hi, vm, v2), fMid(mi)
    {
    }
    double operator()(double x) { return (x < fMid) ? fSegment1(x) : fSegment2(x); }

    void getMappingValues(double& amin, double& amid, double& amax)
    {
        fSegment1.getLowHigh(amin, amid);
        fSegment2.getLowHigh(amid, amax);
    }
};

//--------------------------------------------------------------------------------------
// Abstract ValueConverter class. Converts values between UI and Faust representations
//--------------------------------------------------------------------------------------
class FAUST_API ValueConverter
{
   public:
    virtual ~ValueConverter() {}
    virtual double ui2faust(double x) { return x; };
    virtual double faust2ui(double x) { return x; };
};

//--------------------------------------------------------------------------------------
// A converter than can be updated
//--------------------------------------------------------------------------------------

class FAUST_API UpdatableValueConverter : public ValueConverter
{
   protected:
    bool fActive;

   public:
    UpdatableValueConverter() : fActive(true) {}
    virtual ~UpdatableValueConverter() {}

    virtual void setMappingValues(double amin, double amid, double amax, double min,
                                  double init, double max)                  = 0;
    virtual void getMappingValues(double& amin, double& amid, double& amax) = 0;

    void setActive(bool on_off) { fActive = on_off; }
    bool getActive() { return fActive; }
};

//--------------------------------------------------------------------------------------
// Linear conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class FAUST_API LinearValueConverter : public ValueConverter
{
   private:
    Interpolator fUI2F;
    Interpolator fF2UI;

   public:
    LinearValueConverter(double umin, double umax, double fmin, double fmax)
        : fUI2F(umin, umax, fmin, fmax), fF2UI(fmin, fmax, umin, umax)
    {
    }

    LinearValueConverter() : fUI2F(0., 0., 0., 0.), fF2UI(0., 0., 0., 0.) {}
    virtual double ui2faust(double x) { return fUI2F(x); }
    virtual double faust2ui(double x) { return fF2UI(x); }
};

//--------------------------------------------------------------------------------------
// Two segments linear conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class FAUST_API LinearValueConverter2 : public UpdatableValueConverter
{
   private:
    Interpolator3pt fUI2F;
    Interpolator3pt fF2UI;

   public:
    LinearValueConverter2(double amin, double amid, double amax, double min, double init,
                          double max)
        : fUI2F(amin, amid, amax, min, init, max), fF2UI(min, init, max, amin, amid, amax)
    {
    }

    LinearValueConverter2() : fUI2F(0., 0., 0., 0., 0., 0.), fF2UI(0., 0., 0., 0., 0., 0.)
    {
    }

    virtual double ui2faust(double x) { return fUI2F(x); }
    virtual double faust2ui(double x) { return fF2UI(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double min,
                                  double init, double max)
    {
        fUI2F = Interpolator3pt(amin, amid, amax, min, init, max);
        fF2UI = Interpolator3pt(min, init, max, amin, amid, amax);
    }

    virtual void getMappingValues(double& amin, double& amid, double& amax)
    {
        fUI2F.getMappingValues(amin, amid, amax);
    }
};

//--------------------------------------------------------------------------------------
// Logarithmic conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class FAUST_API LogValueConverter : public LinearValueConverter
{
   public:
    LogValueConverter(double umin, double umax, double fmin, double fmax)
        : LinearValueConverter(umin, umax, std::log(std::max<double>(DBL_MIN, fmin)),
                               std::log(std::max<double>(DBL_MIN, fmax)))
    {
    }

    virtual double ui2faust(double x)
    {
        return std::exp(LinearValueConverter::ui2faust(x));
    }
    virtual double faust2ui(double x)
    {
        return LinearValueConverter::faust2ui(std::log(std::max<double>(x, DBL_MIN)));
    }
};

//--------------------------------------------------------------------------------------
// Exponential conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class FAUST_API ExpValueConverter : public LinearValueConverter
{
   public:
    ExpValueConverter(double umin, double umax, double fmin, double fmax)
        : LinearValueConverter(umin, umax, std::min<double>(DBL_MAX, std::exp(fmin)),
                               std::min<double>(DBL_MAX, std::exp(fmax)))
    {
    }

    virtual double ui2faust(double x)
    {
        return std::log(LinearValueConverter::ui2faust(x));
    }
    virtual double faust2ui(double x)
    {
        return LinearValueConverter::faust2ui(std::min<double>(DBL_MAX, std::exp(x)));
    }
};

//--------------------------------------------------------------------------------------
// Convert accelerometer or gyroscope values to Faust values
// Using an Up curve (curve 0)
//--------------------------------------------------------------------------------------
class FAUST_API AccUpConverter : public UpdatableValueConverter
{
   private:
    Interpolator3pt fA2F;
    Interpolator3pt fF2A;

   public:
    AccUpConverter(double amin, double amid, double amax, double fmin, double fmid,
                   double fmax)
        : fA2F(amin, amid, amax, fmin, fmid, fmax)
        , fF2A(fmin, fmid, fmax, amin, amid, amax)
    {
    }

    virtual double ui2faust(double x) { return fA2F(x); }
    virtual double faust2ui(double x) { return fF2A(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double fmin,
                                  double fmid, double fmax)
    {
        //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccUpConverter update %f %f %f
        //%f %f %f", amin,amid,amax,fmin,fmid,fmax);
        fA2F = Interpolator3pt(amin, amid, amax, fmin, fmid, fmax);
        fF2A = Interpolator3pt(fmin, fmid, fmax, amin, amid, amax);
    }

    virtual void getMappingValues(double& amin, double& amid, double& amax)
    {
        fA2F.getMappingValues(amin, amid, amax);
    }
};

//--------------------------------------------------------------------------------------
// Convert accelerometer or gyroscope values to Faust values
// Using a Down curve (curve 1)
//--------------------------------------------------------------------------------------
class FAUST_API AccDownConverter : public UpdatableValueConverter
{
   private:
    Interpolator3pt fA2F;
    Interpolator3pt fF2A;

   public:
    AccDownConverter(double amin, double amid, double amax, double fmin, double fmid,
                     double fmax)
        : fA2F(amin, amid, amax, fmax, fmid, fmin)
        , fF2A(fmin, fmid, fmax, amax, amid, amin)
    {
    }

    virtual double ui2faust(double x) { return fA2F(x); }
    virtual double faust2ui(double x) { return fF2A(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double fmin,
                                  double fmid, double fmax)
    {
        //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccDownConverter update %f %f
        //%f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
        fA2F = Interpolator3pt(amin, amid, amax, fmax, fmid, fmin);
        fF2A = Interpolator3pt(fmin, fmid, fmax, amax, amid, amin);
    }

    virtual void getMappingValues(double& amin, double& amid, double& amax)
    {
        fA2F.getMappingValues(amin, amid, amax);
    }
};

//--------------------------------------------------------------------------------------
// Convert accelerometer or gyroscope values to Faust values
// Using an Up-Down curve (curve 2)
//--------------------------------------------------------------------------------------
class FAUST_API AccUpDownConverter : public UpdatableValueConverter
{
   private:
    Interpolator3pt fA2F;
    Interpolator fF2A;

   public:
    AccUpDownConverter(double amin, double amid, double amax, double fmin,
                       double /*fmid*/, double fmax)
        : fA2F(amin, amid, amax, fmin, fmax, fmin)
        , fF2A(fmin, fmax, amin,
               amax)  // Special, pseudo inverse of a non monotonic function
    {
    }

    virtual double ui2faust(double x) { return fA2F(x); }
    virtual double faust2ui(double x) { return fF2A(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double fmin,
                                  double /*fmid*/, double fmax)
    {
        //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccUpDownConverter update %f %f
        //%f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
        fA2F = Interpolator3pt(amin, amid, amax, fmin, fmax, fmin);
        fF2A = Interpolator(fmin, fmax, amin, amax);
    }

    virtual void getMappingValues(double& amin, double& amid, double& amax)
    {
        fA2F.getMappingValues(amin, amid, amax);
    }
};

//--------------------------------------------------------------------------------------
// Convert accelerometer or gyroscope values to Faust values
// Using a Down-Up curve (curve 3)
//--------------------------------------------------------------------------------------
class FAUST_API AccDownUpConverter : public UpdatableValueConverter
{
   private:
    Interpolator3pt fA2F;
    Interpolator fF2A;

   public:
    AccDownUpConverter(double amin, double amid, double amax, double fmin,
                       double /*fmid*/, double fmax)
        : fA2F(amin, amid, amax, fmax, fmin, fmax)
        , fF2A(fmin, fmax, amin,
               amax)  // Special, pseudo inverse of a non monotonic function
    {
    }

    virtual double ui2faust(double x) { return fA2F(x); }
    virtual double faust2ui(double x) { return fF2A(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double fmin,
                                  double /*fmid*/, double fmax)
    {
        //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccDownUpConverter update %f %f
        //%f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
        fA2F = Interpolator3pt(amin, amid, amax, fmax, fmin, fmax);
        fF2A = Interpolator(fmin, fmax, amin, amax);
    }

    virtual void getMappingValues(double& amin, double& amid, double& amax)
    {
        fA2F.getMappingValues(amin, amid, amax);
    }
};

//--------------------------------------------------------------------------------------
// Base class for ZoneControl
//--------------------------------------------------------------------------------------
class FAUST_API ZoneControl
{
   protected:
    FAUSTFLOAT* fZone;

   public:
    ZoneControl(FAUSTFLOAT* zone) : fZone(zone) {}
    virtual ~ZoneControl() {}

    virtual void update(double /*v*/) const {}

    virtual void setMappingValues(int /*curve*/, double /*amin*/, double /*amid*/,
                                  double /*amax*/, double /*min*/, double /*init*/,
                                  double /*max*/)
    {
    }
    virtual void getMappingValues(double& /*amin*/, double& /*amid*/, double& /*amax*/) {}

    FAUSTFLOAT* getZone() { return fZone; }

    virtual void setActive(bool /*on_off*/) {}
    virtual bool getActive() { return false; }

    virtual int getCurve() { return -1; }
};

//--------------------------------------------------------------------------------------
//  Useful to implement accelerometers metadata as a list of ZoneControl for each axes
//--------------------------------------------------------------------------------------
class FAUST_API ConverterZoneControl : public ZoneControl
{
   protected:
    ValueConverter* fValueConverter;

   public:
    ConverterZoneControl(FAUSTFLOAT* zone, ValueConverter* converter)
        : ZoneControl(zone), fValueConverter(converter)
    {
    }
    virtual ~ConverterZoneControl()
    {
        delete fValueConverter;
    }  // Assuming fValueConverter is not kept elsewhere...

    virtual void update(double v) const
    {
        *fZone = FAUSTFLOAT(fValueConverter->ui2faust(v));
    }

    ValueConverter* getConverter() { return fValueConverter; }
};

//--------------------------------------------------------------------------------------
// Association of a zone and a four value converter, each one for each possible curve.
// Useful to implement accelerometers metadata as a list of ZoneControl for each axes
//--------------------------------------------------------------------------------------
class FAUST_API CurveZoneControl : public ZoneControl
{
   private:
    std::vector<UpdatableValueConverter*> fValueConverters;
    int fCurve;

   public:
    CurveZoneControl(FAUSTFLOAT* zone, int curve, double amin, double amid, double amax,
                     double min, double init, double max)
        : ZoneControl(zone), fCurve(0)
    {
        assert(curve >= 0 && curve <= 3);
        fValueConverters.push_back(new AccUpConverter(amin, amid, amax, min, init, max));
        fValueConverters.push_back(
            new AccDownConverter(amin, amid, amax, min, init, max));
        fValueConverters.push_back(
            new AccUpDownConverter(amin, amid, amax, min, init, max));
        fValueConverters.push_back(
            new AccDownUpConverter(amin, amid, amax, min, init, max));
        fCurve = curve;
    }
    virtual ~CurveZoneControl()
    {
        for (const auto& it : fValueConverters) {
            delete it;
        }
    }
    void update(double v) const
    {
        if (fValueConverters[fCurve]->getActive())
            *fZone = FAUSTFLOAT(fValueConverters[fCurve]->ui2faust(v));
    }

    void setMappingValues(int curve, double amin, double amid, double amax, double min,
                          double init, double max)
    {
        fValueConverters[curve]->setMappingValues(amin, amid, amax, min, init, max);
        fCurve = curve;
    }

    void getMappingValues(double& amin, double& amid, double& amax)
    {
        fValueConverters[fCurve]->getMappingValues(amin, amid, amax);
    }

    void setActive(bool on_off)
    {
        for (const auto& it : fValueConverters) {
            it->setActive(on_off);
        }
    }

    int getCurve() { return fCurve; }
};

class FAUST_API ZoneReader
{
   private:
    FAUSTFLOAT* fZone;
    Interpolator fInterpolator;

   public:
    ZoneReader(FAUSTFLOAT* zone, double lo, double hi)
        : fZone(zone), fInterpolator(lo, hi, 0, 255)
    {
    }

    virtual ~ZoneReader() {}

    int getValue() { return (fZone != nullptr) ? int(fInterpolator(*fZone)) : 127; }
};

#endif
/**************************  END  ValueConverter.h **************************/

typedef unsigned int uint;

class APIUI
    : public PathBuilder
    , public Meta
    , public UI
{
   public:
    enum ItemType {
        kButton = 0,
        kCheckButton,
        kVSlider,
        kHSlider,
        kNumEntry,
        kHBargraph,
        kVBargraph
    };
    enum Type { kAcc = 0, kGyr = 1, kNoType };

   protected:
    enum Mapping { kLin = 0, kLog = 1, kExp = 2 };

    struct Item {
        std::string fLabel;
        std::string fShortname;
        std::string fPath;
        ValueConverter* fConversion;
        FAUSTFLOAT* fZone;
        FAUSTFLOAT fInit;
        FAUSTFLOAT fMin;
        FAUSTFLOAT fMax;
        FAUSTFLOAT fStep;
        ItemType fItemType;

        Item(const std::string& label, const std::string& short_name,
             const std::string& path, ValueConverter* conversion, FAUSTFLOAT* zone,
             FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step,
             ItemType item_type)
            : fLabel(label)
            , fShortname(short_name)
            , fPath(path)
            , fConversion(conversion)
            , fZone(zone)
            , fInit(init)
            , fMin(min)
            , fMax(max)
            , fStep(step)
            , fItemType(item_type)
        {
        }
    };
    std::vector<Item> fItems;

    std::vector<std::map<std::string, std::string> > fMetaData;
    std::vector<ZoneControl*> fAcc[3];
    std::vector<ZoneControl*> fGyr[3];

    // Screen color control
    // "...[screencolor:red]..." etc.
    bool fHasScreenControl;  // true if control screen color metadata
    ZoneReader* fRedReader;
    ZoneReader* fGreenReader;
    ZoneReader* fBlueReader;

    // Current values controlled by metadata
    std::string fCurrentUnit;
    int fCurrentScale;
    std::string fCurrentAcc;
    std::string fCurrentGyr;
    std::string fCurrentColor;
    std::string fCurrentTooltip;
    std::map<std::string, std::string> fCurrentMetadata;

    // Add a generic parameter
    virtual void addParameter(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init,
                              FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step,
                              ItemType type)
    {
        std::string path = buildPath(label);
        fFullPaths.push_back(path);

        // handle scale metadata
        ValueConverter* converter = nullptr;
        switch (fCurrentScale) {
        case kLin:
            converter = new LinearValueConverter(0, 1, min, max);
            break;
        case kLog:
            converter = new LogValueConverter(0, 1, min, max);
            break;
        case kExp:
            converter = new ExpValueConverter(0, 1, min, max);
            break;
        }
        fCurrentScale = kLin;

        fItems.push_back(
            Item(label, "", path, converter, zone, init, min, max, step, type));

        if (fCurrentAcc.size() > 0 && fCurrentGyr.size() > 0) {
            fprintf(
                stderr,
                "warning : 'acc' and 'gyr' metadata used for the same %s parameter !!\n",
                label);
        }

        // handle acc metadata "...[acc : <axe> <curve> <amin> <amid> <amax>]..."
        if (fCurrentAcc.size() > 0) {
            std::istringstream iss(fCurrentAcc);
            int axe, curve;
            double amin, amid, amax;
            iss >> axe >> curve >> amin >> amid >> amax;

            if ((0 <= axe) && (axe < 3) && (0 <= curve) && (curve < 4) && (amin < amax)
                && (amin <= amid) && (amid <= amax)) {
                fAcc[axe].push_back(
                    new CurveZoneControl(zone, curve, amin, amid, amax, min, init, max));
            } else {
                fprintf(stderr, "incorrect acc metadata : %s \n", fCurrentAcc.c_str());
            }
            fCurrentAcc = "";
        }

        // handle gyr metadata "...[gyr : <axe> <curve> <amin> <amid> <amax>]..."
        if (fCurrentGyr.size() > 0) {
            std::istringstream iss(fCurrentGyr);
            int axe, curve;
            double amin, amid, amax;
            iss >> axe >> curve >> amin >> amid >> amax;

            if ((0 <= axe) && (axe < 3) && (0 <= curve) && (curve < 4) && (amin < amax)
                && (amin <= amid) && (amid <= amax)) {
                fGyr[axe].push_back(
                    new CurveZoneControl(zone, curve, amin, amid, amax, min, init, max));
            } else {
                fprintf(stderr, "incorrect gyr metadata : %s \n", fCurrentGyr.c_str());
            }
            fCurrentGyr = "";
        }

        // handle screencolor metadata "...[screencolor:red|green|blue|white]..."
        if (fCurrentColor.size() > 0) {
            if ((fCurrentColor == "red") && (fRedReader == nullptr)) {
                fRedReader        = new ZoneReader(zone, min, max);
                fHasScreenControl = true;
            } else if ((fCurrentColor == "green") && (fGreenReader == nullptr)) {
                fGreenReader      = new ZoneReader(zone, min, max);
                fHasScreenControl = true;
            } else if ((fCurrentColor == "blue") && (fBlueReader == nullptr)) {
                fBlueReader       = new ZoneReader(zone, min, max);
                fHasScreenControl = true;
            } else if ((fCurrentColor == "white") && (fRedReader == nullptr)
                       && (fGreenReader == nullptr) && (fBlueReader == nullptr)) {
                fRedReader        = new ZoneReader(zone, min, max);
                fGreenReader      = new ZoneReader(zone, min, max);
                fBlueReader       = new ZoneReader(zone, min, max);
                fHasScreenControl = true;
            } else {
                fprintf(stderr, "incorrect screencolor metadata : %s \n",
                        fCurrentColor.c_str());
            }
        }
        fCurrentColor = "";

        fMetaData.push_back(fCurrentMetadata);
        fCurrentMetadata.clear();
    }

    int getZoneIndex(std::vector<ZoneControl*>* table, int p, int val)
    {
        FAUSTFLOAT* zone = fItems[uint(p)].fZone;
        for (size_t i = 0; i < table[val].size(); i++) {
            if (zone == table[val][i]->getZone())
                return int(i);
        }
        return -1;
    }

    void setConverter(std::vector<ZoneControl*>* table, int p, int val, int curve,
                      double amin, double amid, double amax)
    {
        int id1 = getZoneIndex(table, p, 0);
        int id2 = getZoneIndex(table, p, 1);
        int id3 = getZoneIndex(table, p, 2);

        // Deactivates everywhere..
        if (id1 != -1)
            table[0][uint(id1)]->setActive(false);
        if (id2 != -1)
            table[1][uint(id2)]->setActive(false);
        if (id3 != -1)
            table[2][uint(id3)]->setActive(false);

        if (val == -1) {  // Means: no more mapping...
            // So stay all deactivated...
        } else {
            int id4 = getZoneIndex(table, p, val);
            if (id4 != -1) {
                // Reactivate the one we edit...
                table[val][uint(id4)]->setMappingValues(
                    curve, amin, amid, amax, fItems[uint(p)].fMin, fItems[uint(p)].fInit,
                    fItems[uint(p)].fMax);
                table[val][uint(id4)]->setActive(true);
            } else {
                // Allocate a new CurveZoneControl which is 'active' by default
                FAUSTFLOAT* zone = fItems[uint(p)].fZone;
                table[val].push_back(new CurveZoneControl(
                    zone, curve, amin, amid, amax, fItems[uint(p)].fMin,
                    fItems[uint(p)].fInit, fItems[uint(p)].fMax));
            }
        }
    }

    void getConverter(std::vector<ZoneControl*>* table, int p, int& val, int& curve,
                      double& amin, double& amid, double& amax)
    {
        int id1 = getZoneIndex(table, p, 0);
        int id2 = getZoneIndex(table, p, 1);
        int id3 = getZoneIndex(table, p, 2);

        if (id1 != -1) {
            val   = 0;
            curve = table[val][uint(id1)]->getCurve();
            table[val][uint(id1)]->getMappingValues(amin, amid, amax);
        } else if (id2 != -1) {
            val   = 1;
            curve = table[val][uint(id2)]->getCurve();
            table[val][uint(id2)]->getMappingValues(amin, amid, amax);
        } else if (id3 != -1) {
            val   = 2;
            curve = table[val][uint(id3)]->getCurve();
            table[val][uint(id3)]->getMappingValues(amin, amid, amax);
        } else {
            val   = -1;  // No mapping
            curve = 0;
            amin  = -100.;
            amid  = 0.;
            amax  = 100.;
        }
    }

   public:
    APIUI()
        : fHasScreenControl(false)
        , fRedReader(nullptr)
        , fGreenReader(nullptr)
        , fBlueReader(nullptr)
        , fCurrentScale(kLin)
    {
    }

    virtual ~APIUI()
    {
        for (const auto& it : fItems)
            delete it.fConversion;
        for (int i = 0; i < 3; i++) {
            for (const auto& it : fAcc[i])
                delete it;
            for (const auto& it : fGyr[i])
                delete it;
        }
        delete fRedReader;
        delete fGreenReader;
        delete fBlueReader;
    }

    // -- widget's layouts

    virtual void openTabBox(const char* label) { pushLabel(label); }
    virtual void openHorizontalBox(const char* label) { pushLabel(label); }
    virtual void openVerticalBox(const char* label) { pushLabel(label); }
    virtual void closeBox()
    {
        if (popLabel()) {
            // Shortnames can be computed when all fullnames are known
            computeShortNames();
            // Fill 'shortname' field for each item
            for (const auto& it : fFull2Short) {
                int index                = getParamIndex(it.first.c_str());
                fItems[index].fShortname = it.second;
            }
        }
    }

    // -- active widgets

    virtual void addButton(const char* label, FAUSTFLOAT* zone)
    {
        addParameter(label, zone, 0, 0, 1, 1, kButton);
    }

    virtual void addCheckButton(const char* label, FAUSTFLOAT* zone)
    {
        addParameter(label, zone, 0, 0, 1, 1, kCheckButton);
    }

    virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init,
                                   FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
    {
        addParameter(label, zone, init, min, max, step, kVSlider);
    }

    virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init,
                                     FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
    {
        addParameter(label, zone, init, min, max, step, kHSlider);
    }

    virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init,
                             FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
    {
        addParameter(label, zone, init, min, max, step, kNumEntry);
    }

    // -- passive widgets

    virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone,
                                       FAUSTFLOAT min, FAUSTFLOAT max)
    {
        addParameter(label, zone, min, min, max, (max - min) / 1000.0f, kHBargraph);
    }

    virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min,
                                     FAUSTFLOAT max)
    {
        addParameter(label, zone, min, min, max, (max - min) / 1000.0f, kVBargraph);
    }

    // -- soundfiles

    virtual void addSoundfile(const char* /*label*/, const char* /*filename*/,
                              Soundfile** /*sf_zone*/)
    {
    }

    // -- metadata declarations

    virtual void declare(FAUSTFLOAT* /*zone*/, const char* key, const char* val)
    {
        // Keep metadata
        fCurrentMetadata[key] = val;

        if (strcmp(key, "scale") == 0) {
            if (strcmp(val, "log") == 0) {
                fCurrentScale = kLog;
            } else if (strcmp(val, "exp") == 0) {
                fCurrentScale = kExp;
            } else {
                fCurrentScale = kLin;
            }
        } else if (strcmp(key, "unit") == 0) {
            fCurrentUnit = val;
        } else if (strcmp(key, "acc") == 0) {
            fCurrentAcc = val;
        } else if (strcmp(key, "gyr") == 0) {
            fCurrentGyr = val;
        } else if (strcmp(key, "screencolor") == 0) {
            fCurrentColor = val;  // val = "red", "green", "blue" or "white"
        } else if (strcmp(key, "tooltip") == 0) {
            fCurrentTooltip = val;
        }
    }

    virtual void declare(const char* /*key*/, const char* /*val*/) {}

    //-------------------------------------------------------------------------------
    // Simple API part
    //-------------------------------------------------------------------------------
    int getParamsCount() { return int(fItems.size()); }

    int getParamIndex(const char* path_aux)
    {
        std::string path = std::string(path_aux);
        auto it          = find_if(fItems.begin(), fItems.end(), [=](const Item& it) {
            return (it.fLabel == path) || (it.fShortname == path) || (it.fPath == path);
        });
        return (it != fItems.end()) ? int(it - fItems.begin()) : -1;
    }

    const char* getParamLabel(int p) { return fItems[uint(p)].fLabel.c_str(); }
    const char* getParamShortname(int p) { return fItems[uint(p)].fShortname.c_str(); }
    const char* getParamAddress(int p) { return fItems[uint(p)].fPath.c_str(); }

    std::map<const char*, const char*> getMetadata(int p)
    {
        std::map<const char*, const char*> res;
        std::map<std::string, std::string> metadata = fMetaData[uint(p)];
        for (const auto& it : metadata) {
            res[it.first.c_str()] = it.second.c_str();
        }
        return res;
    }

    const char* getMetadata(int p, const char* key)
    {
        return (fMetaData[uint(p)].find(key) != fMetaData[uint(p)].end())
                   ? fMetaData[uint(p)][key].c_str()
                   : "";
    }
    FAUSTFLOAT getParamMin(int p) { return fItems[uint(p)].fMin; }
    FAUSTFLOAT getParamMax(int p) { return fItems[uint(p)].fMax; }
    FAUSTFLOAT getParamStep(int p) { return fItems[uint(p)].fStep; }
    FAUSTFLOAT getParamInit(int p) { return fItems[uint(p)].fInit; }

    FAUSTFLOAT* getParamZone(int p) { return fItems[uint(p)].fZone; }

    FAUSTFLOAT getParamValue(int p) { return *fItems[uint(p)].fZone; }
    FAUSTFLOAT getParamValue(const char* path)
    {
        int index = getParamIndex(path);
        if (index >= 0) {
            return getParamValue(index);
        } else {
            fprintf(stderr, "getParamValue : '%s' not found\n",
                    (path == nullptr ? "NULL" : path));
            return FAUSTFLOAT(0);
        }
    }

    void setParamValue(int p, FAUSTFLOAT v) { *fItems[uint(p)].fZone = v; }
    void setParamValue(const char* path, FAUSTFLOAT v)
    {
        int index = getParamIndex(path);
        if (index >= 0) {
            setParamValue(index, v);
        } else {
            fprintf(stderr, "setParamValue : '%s' not found\n",
                    (path == nullptr ? "NULL" : path));
        }
    }

    double getParamRatio(int p)
    {
        return fItems[uint(p)].fConversion->faust2ui(*fItems[uint(p)].fZone);
    }
    void setParamRatio(int p, double r)
    {
        *fItems[uint(p)].fZone = FAUSTFLOAT(fItems[uint(p)].fConversion->ui2faust(r));
    }

    double value2ratio(int p, double r)
    {
        return fItems[uint(p)].fConversion->faust2ui(r);
    }
    double ratio2value(int p, double r)
    {
        return fItems[uint(p)].fConversion->ui2faust(r);
    }

    /**
     * Return the control type (kAcc, kGyr, or -1) for a given parameter.
     *
     * @param p - the UI parameter index
     *
     * @return the type
     */
    Type getParamType(int p)
    {
        if (p >= 0) {
            if (getZoneIndex(fAcc, p, 0) != -1 || getZoneIndex(fAcc, p, 1) != -1
                || getZoneIndex(fAcc, p, 2) != -1) {
                return kAcc;
            } else if (getZoneIndex(fGyr, p, 0) != -1 || getZoneIndex(fGyr, p, 1) != -1
                       || getZoneIndex(fGyr, p, 2) != -1) {
                return kGyr;
            }
        }
        return kNoType;
    }

    /**
     * Return the Item type (kButton = 0, kCheckButton, kVSlider, kHSlider, kNumEntry,
     * kHBargraph, kVBargraph) for a given parameter.
     *
     * @param p - the UI parameter index
     *
     * @return the Item type
     */
    ItemType getParamItemType(int p) { return fItems[uint(p)].fItemType; }

    /**
     * Set a new value coming from an accelerometer, propagate it to all relevant
     * FAUSTFLOAT* zones.
     *
     * @param acc - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer
     * @param value - the new value
     *
     */
    void propagateAcc(int acc, double value)
    {
        for (size_t i = 0; i < fAcc[acc].size(); i++) {
            fAcc[acc][i]->update(value);
        }
    }

    /**
     * Used to edit accelerometer curves and mapping. Set curve and related mapping for a
     * given UI parameter.
     *
     * @param p - the UI parameter index
     * @param acc - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer
     * (-1 means "no mapping")
     * @param curve - between 0 and 3
     * @param amin - mapping 'min' point
     * @param amid - mapping 'middle' point
     * @param amax - mapping 'max' point
     *
     */
    void setAccConverter(int p, int acc, int curve, double amin, double amid, double amax)
    {
        setConverter(fAcc, p, acc, curve, amin, amid, amax);
    }

    /**
     * Used to edit gyroscope curves and mapping. Set curve and related mapping for a
     * given UI parameter.
     *
     * @param p - the UI parameter index
     * @param gyr - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope (-1 means "no
     * mapping")
     * @param curve - between 0 and 3
     * @param amin - mapping 'min' point
     * @param amid - mapping 'middle' point
     * @param amax - mapping 'max' point
     *
     */
    void setGyrConverter(int p, int gyr, int curve, double amin, double amid, double amax)
    {
        setConverter(fGyr, p, gyr, curve, amin, amid, amax);
    }

    /**
     * Used to edit accelerometer curves and mapping. Get curve and related mapping for a
     * given UI parameter.
     *
     * @param p - the UI parameter index
     * @param acc - the acc value to be retrieved (-1 means "no mapping")
     * @param curve - the curve value to be retrieved
     * @param amin - the amin value to be retrieved
     * @param amid - the amid value to be retrieved
     * @param amax - the amax value to be retrieved
     *
     */
    void getAccConverter(int p, int& acc, int& curve, double& amin, double& amid,
                         double& amax)
    {
        getConverter(fAcc, p, acc, curve, amin, amid, amax);
    }

    /**
     * Used to edit gyroscope curves and mapping. Get curve and related mapping for a
     * given UI parameter.
     *
     * @param p - the UI parameter index
     * @param gyr - the gyr value to be retrieved (-1 means "no mapping")
     * @param curve - the curve value to be retrieved
     * @param amin - the amin value to be retrieved
     * @param amid - the amid value to be retrieved
     * @param amax - the amax value to be retrieved
     *
     */
    void getGyrConverter(int p, int& gyr, int& curve, double& amin, double& amid,
                         double& amax)
    {
        getConverter(fGyr, p, gyr, curve, amin, amid, amax);
    }

    /**
     * Set a new value coming from an gyroscope, propagate it to all relevant FAUSTFLOAT*
     * zones.
     *
     * @param gyr - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope
     * @param value - the new value
     *
     */
    void propagateGyr(int gyr, double value)
    {
        for (size_t i = 0; i < fGyr[gyr].size(); i++) {
            fGyr[gyr][i]->update(value);
        }
    }

    /**
     * Get the number of FAUSTFLOAT* zones controlled with the accelerometer.
     *
     * @param acc - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer
     * @return the number of zones
     *
     */
    int getAccCount(int acc) { return (acc >= 0 && acc < 3) ? int(fAcc[acc].size()) : 0; }

    /**
     * Get the number of FAUSTFLOAT* zones controlled with the gyroscope.
     *
     * @param gyr - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope
     * @param the number of zones
     *
     */
    int getGyrCount(int gyr) { return (gyr >= 0 && gyr < 3) ? int(fGyr[gyr].size()) : 0; }

    // getScreenColor() : -1 means no screen color control (no screencolor metadata found)
    // otherwise return 0x00RRGGBB a ready to use color
    int getScreenColor()
    {
        if (fHasScreenControl) {
            int r = (fRedReader) ? fRedReader->getValue() : 0;
            int g = (fGreenReader) ? fGreenReader->getValue() : 0;
            int b = (fBlueReader) ? fBlueReader->getValue() : 0;
            return (r << 16) | (g << 8) | b;
        } else {
            return -1;
        }
    }
};

#endif
/**************************  END  APIUI.h **************************/

// NOTE: "faust -scn name" changes the last line above to
// #include <faust/name/name.h>

//----------------------------------------------------------------------------
//  FAUST Generated Code
//----------------------------------------------------------------------------

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

#include <math.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

#ifndef FAUSTCLASS
#define FAUSTCLASS zitarevdsp
#endif

#ifdef __APPLE__
#define exp10f __exp10f
#define exp10  __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

static float zitarevdsp_faustpower2_f(float value)
{
    return value * value;
}

class zitarevdsp : public dsp
{
   private:
    int IOTA0;
    float fVec0[16384];
    float fVec1[16384];
    int fSampleRate;
    float fConst1;
    FAUSTFLOAT fVslider0;
    float fConst2;
    float fRec0[2];
    FAUSTFLOAT fVslider1;
    float fRec1[2];
    float fConst3;
    FAUSTFLOAT fVslider2;
    FAUSTFLOAT fVslider3;
    FAUSTFLOAT fVslider4;
    FAUSTFLOAT fVslider5;
    float fConst5;
    FAUSTFLOAT fVslider6;
    FAUSTFLOAT fVslider7;
    FAUSTFLOAT fVslider8;
    float fConst6;
    FAUSTFLOAT fVslider9;
    float fRec15[2];
    float fRec14[2];
    float fVec2[32768];
    int iConst8;
    float fConst9;
    FAUSTFLOAT fVslider10;
    float fVec3[2048];
    int iConst10;
    float fRec12[2];
    float fConst12;
    float fRec19[2];
    float fRec18[2];
    float fVec4[32768];
    int iConst14;
    float fVec5[4096];
    int iConst15;
    float fRec16[2];
    float fConst17;
    float fRec23[2];
    float fRec22[2];
    float fVec6[16384];
    int iConst19;
    float fVec7[4096];
    int iConst20;
    float fRec20[2];
    float fConst22;
    float fRec27[2];
    float fRec26[2];
    float fVec8[32768];
    int iConst24;
    float fVec9[4096];
    int iConst25;
    float fRec24[2];
    float fConst27;
    float fRec31[2];
    float fRec30[2];
    float fVec10[16384];
    int iConst29;
    float fVec11[2048];
    int iConst30;
    float fRec28[2];
    float fConst32;
    float fRec35[2];
    float fRec34[2];
    float fVec12[16384];
    int iConst34;
    float fVec13[4096];
    int iConst35;
    float fRec32[2];
    float fConst37;
    float fRec39[2];
    float fRec38[2];
    float fVec14[16384];
    int iConst39;
    float fVec15[4096];
    int iConst40;
    float fRec36[2];
    float fConst42;
    float fRec43[2];
    float fRec42[2];
    float fVec16[16384];
    int iConst44;
    float fVec17[2048];
    int iConst45;
    float fRec40[2];
    float fRec4[3];
    float fRec5[3];
    float fRec6[3];
    float fRec7[3];
    float fRec8[3];
    float fRec9[3];
    float fRec10[3];
    float fRec11[3];
    float fRec3[3];
    float fRec2[3];
    float fRec45[3];
    float fRec44[3];

   public:
    void metadata(Meta* m)
    {
        m->declare("basics.lib/name", "Faust Basic Element Library");
        m->declare("basics.lib/version", "0.8");
        m->declare("compile_options",
                   "-a faust2header.cpp -lang cpp -i -inpl -cn zitarevdsp -es 1 -mcd 16 "
                   "-single -ftz 0");
        m->declare("delays.lib/name", "Faust Delay Library");
        m->declare("delays.lib/version", "0.1");
        m->declare("filename", "zitarevdsp.dsp");
        m->declare("filters.lib/allpass_comb:author", "Julius O. Smith III");
        m->declare(
            "filters.lib/allpass_comb:copyright",
            "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
        m->declare("filters.lib/allpass_comb:license", "MIT-style STK-4.3 license");
        m->declare("filters.lib/fir:author", "Julius O. Smith III");
        m->declare(
            "filters.lib/fir:copyright",
            "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
        m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
        m->declare("filters.lib/iir:author", "Julius O. Smith III");
        m->declare(
            "filters.lib/iir:copyright",
            "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
        m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
        m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
        m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
        m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
        m->declare(
            "filters.lib/lowpass:copyright",
            "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
        m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
        m->declare("filters.lib/name", "Faust Filters Library");
        m->declare("filters.lib/peak_eq_rm:author", "Julius O. Smith III");
        m->declare(
            "filters.lib/peak_eq_rm:copyright",
            "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
        m->declare("filters.lib/peak_eq_rm:license", "MIT-style STK-4.3 license");
        m->declare("filters.lib/tf1:author", "Julius O. Smith III");
        m->declare(
            "filters.lib/tf1:copyright",
            "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
        m->declare("filters.lib/tf1:license", "MIT-style STK-4.3 license");
        m->declare("filters.lib/tf1s:author", "Julius O. Smith III");
        m->declare(
            "filters.lib/tf1s:copyright",
            "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
        m->declare("filters.lib/tf1s:license", "MIT-style STK-4.3 license");
        m->declare("filters.lib/tf2:author", "Julius O. Smith III");
        m->declare(
            "filters.lib/tf2:copyright",
            "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
        m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
        m->declare("filters.lib/version", "0.3");
        m->declare("maths.lib/author", "GRAME");
        m->declare("maths.lib/copyright", "GRAME");
        m->declare("maths.lib/license", "LGPL with exception");
        m->declare("maths.lib/name", "Faust Math Library");
        m->declare("maths.lib/version", "2.5");
        m->declare("name", "zitarevdsp");
        m->declare("platform.lib/name", "Generic Platform Library");
        m->declare("platform.lib/version", "0.2");
        m->declare("reverbs.lib/name", "Faust Reverb Library");
        m->declare("reverbs.lib/version", "0.2");
        m->declare("routes.lib/hadamard:author", "Remy Muller, revised by Romain Michon");
        m->declare("routes.lib/name", "Faust Signal Routing Library");
        m->declare("routes.lib/version", "0.2");
        m->declare("signals.lib/name", "Faust Signal Routing Library");
        m->declare("signals.lib/version", "0.3");
    }

    virtual int getNumInputs() { return 2; }
    virtual int getNumOutputs() { return 2; }

    static void classInit(int /*sample_rate*/) {}

    virtual void instanceConstants(int sample_rate)
    {
        fSampleRate = sample_rate;
        float fConst0 =
            std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
        fConst1       = 44.0999985f / fConst0;
        fConst2       = 1.0f - fConst1;
        fConst3       = 6.28318548f / fConst0;
        float fConst4 = std::floor(0.219990999f * fConst0 + 0.5f);
        fConst5       = (0.0f - 6.90775537f * fConst4) / fConst0;
        fConst6       = 3.14159274f / fConst0;
        float fConst7 = std::floor(0.0191229992f * fConst0 + 0.5f);
        iConst8 =
            int(std::min<float>(16384.0f, std::max<float>(0.0f, fConst4 - fConst7)));
        fConst9  = 0.00100000005f * fConst0;
        iConst10 = int(std::min<float>(1024.0f, std::max<float>(0.0f, fConst7 + -1.0f)));
        float fConst11 = std::floor(0.256891012f * fConst0 + 0.5f);
        fConst12       = (0.0f - 6.90775537f * fConst11) / fConst0;
        float fConst13 = std::floor(0.0273330007f * fConst0 + 0.5f);
        iConst14 =
            int(std::min<float>(16384.0f, std::max<float>(0.0f, fConst11 - fConst13)));
        iConst15 = int(std::min<float>(2048.0f, std::max<float>(0.0f, fConst13 + -1.0f)));
        float fConst16 = std::floor(0.192303002f * fConst0 + 0.5f);
        fConst17       = (0.0f - 6.90775537f * fConst16) / fConst0;
        float fConst18 = std::floor(0.0292910002f * fConst0 + 0.5f);
        iConst19 =
            int(std::min<float>(8192.0f, std::max<float>(0.0f, fConst16 - fConst18)));
        iConst20 = int(std::min<float>(2048.0f, std::max<float>(0.0f, fConst18 + -1.0f)));
        float fConst21 = std::floor(0.210389003f * fConst0 + 0.5f);
        fConst22       = (0.0f - 6.90775537f * fConst21) / fConst0;
        float fConst23 = std::floor(0.0244210009f * fConst0 + 0.5f);
        iConst24 =
            int(std::min<float>(16384.0f, std::max<float>(0.0f, fConst21 - fConst23)));
        iConst25 = int(std::min<float>(2048.0f, std::max<float>(0.0f, fConst23 + -1.0f)));
        float fConst26 = std::floor(0.125f * fConst0 + 0.5f);
        fConst27       = (0.0f - 6.90775537f * fConst26) / fConst0;
        float fConst28 = std::floor(0.0134579996f * fConst0 + 0.5f);
        iConst29 =
            int(std::min<float>(8192.0f, std::max<float>(0.0f, fConst26 - fConst28)));
        iConst30 = int(std::min<float>(1024.0f, std::max<float>(0.0f, fConst28 + -1.0f)));
        float fConst31 = std::floor(0.127837002f * fConst0 + 0.5f);
        fConst32       = (0.0f - 6.90775537f * fConst31) / fConst0;
        float fConst33 = std::floor(0.0316039994f * fConst0 + 0.5f);
        iConst34 =
            int(std::min<float>(8192.0f, std::max<float>(0.0f, fConst31 - fConst33)));
        iConst35 = int(std::min<float>(2048.0f, std::max<float>(0.0f, fConst33 + -1.0f)));
        float fConst36 = std::floor(0.174713001f * fConst0 + 0.5f);
        fConst37       = (0.0f - 6.90775537f * fConst36) / fConst0;
        float fConst38 = std::floor(0.0229039993f * fConst0 + 0.5f);
        iConst39 =
            int(std::min<float>(8192.0f, std::max<float>(0.0f, fConst36 - fConst38)));
        iConst40 = int(std::min<float>(2048.0f, std::max<float>(0.0f, fConst38 + -1.0f)));
        float fConst41 = std::floor(0.153128996f * fConst0 + 0.5f);
        fConst42       = (0.0f - 6.90775537f * fConst41) / fConst0;
        float fConst43 = std::floor(0.0203460008f * fConst0 + 0.5f);
        iConst44 =
            int(std::min<float>(8192.0f, std::max<float>(0.0f, fConst41 - fConst43)));
        iConst45 = int(std::min<float>(1024.0f, std::max<float>(0.0f, fConst43 + -1.0f)));
    }

    virtual void instanceResetUserInterface()
    {
        fVslider0  = FAUSTFLOAT(-3.0f);
        fVslider1  = FAUSTFLOAT(0.0f);
        fVslider2  = FAUSTFLOAT(1500.0f);
        fVslider3  = FAUSTFLOAT(0.0f);
        fVslider4  = FAUSTFLOAT(315.0f);
        fVslider5  = FAUSTFLOAT(0.0f);
        fVslider6  = FAUSTFLOAT(2.0f);
        fVslider7  = FAUSTFLOAT(6000.0f);
        fVslider8  = FAUSTFLOAT(3.0f);
        fVslider9  = FAUSTFLOAT(200.0f);
        fVslider10 = FAUSTFLOAT(60.0f);
    }

    virtual void instanceClear()
    {
        IOTA0 = 0;
        for (int l0 = 0; l0 < 16384; l0 = l0 + 1) {
            fVec0[l0] = 0.0f;
        }
        for (int l1 = 0; l1 < 16384; l1 = l1 + 1) {
            fVec1[l1] = 0.0f;
        }
        for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
            fRec0[l2] = 0.0f;
        }
        for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
            fRec1[l3] = 0.0f;
        }
        for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
            fRec15[l4] = 0.0f;
        }
        for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
            fRec14[l5] = 0.0f;
        }
        for (int l6 = 0; l6 < 32768; l6 = l6 + 1) {
            fVec2[l6] = 0.0f;
        }
        for (int l7 = 0; l7 < 2048; l7 = l7 + 1) {
            fVec3[l7] = 0.0f;
        }
        for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
            fRec12[l8] = 0.0f;
        }
        for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
            fRec19[l9] = 0.0f;
        }
        for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
            fRec18[l10] = 0.0f;
        }
        for (int l11 = 0; l11 < 32768; l11 = l11 + 1) {
            fVec4[l11] = 0.0f;
        }
        for (int l12 = 0; l12 < 4096; l12 = l12 + 1) {
            fVec5[l12] = 0.0f;
        }
        for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
            fRec16[l13] = 0.0f;
        }
        for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
            fRec23[l14] = 0.0f;
        }
        for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
            fRec22[l15] = 0.0f;
        }
        for (int l16 = 0; l16 < 16384; l16 = l16 + 1) {
            fVec6[l16] = 0.0f;
        }
        for (int l17 = 0; l17 < 4096; l17 = l17 + 1) {
            fVec7[l17] = 0.0f;
        }
        for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
            fRec20[l18] = 0.0f;
        }
        for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
            fRec27[l19] = 0.0f;
        }
        for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
            fRec26[l20] = 0.0f;
        }
        for (int l21 = 0; l21 < 32768; l21 = l21 + 1) {
            fVec8[l21] = 0.0f;
        }
        for (int l22 = 0; l22 < 4096; l22 = l22 + 1) {
            fVec9[l22] = 0.0f;
        }
        for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
            fRec24[l23] = 0.0f;
        }
        for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
            fRec31[l24] = 0.0f;
        }
        for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
            fRec30[l25] = 0.0f;
        }
        for (int l26 = 0; l26 < 16384; l26 = l26 + 1) {
            fVec10[l26] = 0.0f;
        }
        for (int l27 = 0; l27 < 2048; l27 = l27 + 1) {
            fVec11[l27] = 0.0f;
        }
        for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
            fRec28[l28] = 0.0f;
        }
        for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
            fRec35[l29] = 0.0f;
        }
        for (int l30 = 0; l30 < 2; l30 = l30 + 1) {
            fRec34[l30] = 0.0f;
        }
        for (int l31 = 0; l31 < 16384; l31 = l31 + 1) {
            fVec12[l31] = 0.0f;
        }
        for (int l32 = 0; l32 < 4096; l32 = l32 + 1) {
            fVec13[l32] = 0.0f;
        }
        for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
            fRec32[l33] = 0.0f;
        }
        for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
            fRec39[l34] = 0.0f;
        }
        for (int l35 = 0; l35 < 2; l35 = l35 + 1) {
            fRec38[l35] = 0.0f;
        }
        for (int l36 = 0; l36 < 16384; l36 = l36 + 1) {
            fVec14[l36] = 0.0f;
        }
        for (int l37 = 0; l37 < 4096; l37 = l37 + 1) {
            fVec15[l37] = 0.0f;
        }
        for (int l38 = 0; l38 < 2; l38 = l38 + 1) {
            fRec36[l38] = 0.0f;
        }
        for (int l39 = 0; l39 < 2; l39 = l39 + 1) {
            fRec43[l39] = 0.0f;
        }
        for (int l40 = 0; l40 < 2; l40 = l40 + 1) {
            fRec42[l40] = 0.0f;
        }
        for (int l41 = 0; l41 < 16384; l41 = l41 + 1) {
            fVec16[l41] = 0.0f;
        }
        for (int l42 = 0; l42 < 2048; l42 = l42 + 1) {
            fVec17[l42] = 0.0f;
        }
        for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
            fRec40[l43] = 0.0f;
        }
        for (int l44 = 0; l44 < 3; l44 = l44 + 1) {
            fRec4[l44] = 0.0f;
        }
        for (int l45 = 0; l45 < 3; l45 = l45 + 1) {
            fRec5[l45] = 0.0f;
        }
        for (int l46 = 0; l46 < 3; l46 = l46 + 1) {
            fRec6[l46] = 0.0f;
        }
        for (int l47 = 0; l47 < 3; l47 = l47 + 1) {
            fRec7[l47] = 0.0f;
        }
        for (int l48 = 0; l48 < 3; l48 = l48 + 1) {
            fRec8[l48] = 0.0f;
        }
        for (int l49 = 0; l49 < 3; l49 = l49 + 1) {
            fRec9[l49] = 0.0f;
        }
        for (int l50 = 0; l50 < 3; l50 = l50 + 1) {
            fRec10[l50] = 0.0f;
        }
        for (int l51 = 0; l51 < 3; l51 = l51 + 1) {
            fRec11[l51] = 0.0f;
        }
        for (int l52 = 0; l52 < 3; l52 = l52 + 1) {
            fRec3[l52] = 0.0f;
        }
        for (int l53 = 0; l53 < 3; l53 = l53 + 1) {
            fRec2[l53] = 0.0f;
        }
        for (int l54 = 0; l54 < 3; l54 = l54 + 1) {
            fRec45[l54] = 0.0f;
        }
        for (int l55 = 0; l55 < 3; l55 = l55 + 1) {
            fRec44[l55] = 0.0f;
        }
    }

    virtual void init(int sample_rate)
    {
        classInit(sample_rate);
        instanceInit(sample_rate);
    }
    virtual void instanceInit(int sample_rate)
    {
        instanceConstants(sample_rate);
        instanceResetUserInterface();
        instanceClear();
    }

    virtual zitarevdsp* clone() { return new zitarevdsp(); }

    virtual int getSampleRate() { return fSampleRate; }

    virtual void buildUserInterface(UI* ui_interface)
    {
        ui_interface->declare(0, "0", "");
        ui_interface->declare(0, "tooltip",
                              "~ ZITA REV1 FEEDBACK DELAY NETWORK (FDN) & SCHROEDER  "
                              "ALLPASS-COMB REVERBERATOR (8x8). See Faust's reverbs.lib "
                              "for documentation and  references");
        ui_interface->openHorizontalBox("Zita_Rev1");
        ui_interface->declare(0, "1", "");
        ui_interface->openHorizontalBox("Input");
        ui_interface->declare(&fVslider10, "1", "");
        ui_interface->declare(&fVslider10, "style", "knob");
        ui_interface->declare(&fVslider10, "tooltip",
                              "Delay in ms   before reverberation begins");
        ui_interface->declare(&fVslider10, "unit", "ms");
        ui_interface->addVerticalSlider("In Delay", &fVslider10, FAUSTFLOAT(60.0f),
                                        FAUSTFLOAT(20.0f), FAUSTFLOAT(100.0f),
                                        FAUSTFLOAT(1.0f));
        ui_interface->closeBox();
        ui_interface->declare(0, "2", "");
        ui_interface->openHorizontalBox("Decay Times in Bands (see tooltips)");
        ui_interface->declare(&fVslider9, "1", "");
        ui_interface->declare(&fVslider9, "scale", "log");
        ui_interface->declare(&fVslider9, "style", "knob");
        ui_interface->declare(
            &fVslider9, "tooltip",
            "Crossover frequency (Hz) separating low and middle frequencies");
        ui_interface->declare(&fVslider9, "unit", "Hz");
        ui_interface->addVerticalSlider("LF X", &fVslider9, FAUSTFLOAT(200.0f),
                                        FAUSTFLOAT(50.0f), FAUSTFLOAT(1000.0f),
                                        FAUSTFLOAT(1.0f));
        ui_interface->declare(&fVslider8, "2", "");
        ui_interface->declare(&fVslider8, "scale", "log");
        ui_interface->declare(&fVslider8, "style", "knob");
        ui_interface->declare(
            &fVslider8, "tooltip",
            "T60 = time (in seconds) to decay 60dB in low-frequency band");
        ui_interface->declare(&fVslider8, "unit", "s");
        ui_interface->addVerticalSlider("Low RT60", &fVslider8, FAUSTFLOAT(3.0f),
                                        FAUSTFLOAT(1.0f), FAUSTFLOAT(8.0f),
                                        FAUSTFLOAT(0.100000001f));
        ui_interface->declare(&fVslider6, "3", "");
        ui_interface->declare(&fVslider6, "scale", "log");
        ui_interface->declare(&fVslider6, "style", "knob");
        ui_interface->declare(&fVslider6, "tooltip",
                              "T60 = time (in seconds) to decay 60dB in middle band");
        ui_interface->declare(&fVslider6, "unit", "s");
        ui_interface->addVerticalSlider("Mid RT60", &fVslider6, FAUSTFLOAT(2.0f),
                                        FAUSTFLOAT(1.0f), FAUSTFLOAT(8.0f),
                                        FAUSTFLOAT(0.100000001f));
        ui_interface->declare(&fVslider7, "4", "");
        ui_interface->declare(&fVslider7, "scale", "log");
        ui_interface->declare(&fVslider7, "style", "knob");
        ui_interface->declare(&fVslider7, "tooltip",
                              "Frequency (Hz) at which the high-frequency T60 is half "
                              "the middle-band's T60");
        ui_interface->declare(&fVslider7, "unit", "Hz");
        ui_interface->addVerticalSlider("HF Damping", &fVslider7, FAUSTFLOAT(6000.0f),
                                        FAUSTFLOAT(1500.0f), FAUSTFLOAT(23520.0f),
                                        FAUSTFLOAT(1.0f));
        ui_interface->closeBox();
        ui_interface->declare(0, "3", "");
        ui_interface->openHorizontalBox("RM Peaking Equalizer 1");
        ui_interface->declare(&fVslider4, "1", "");
        ui_interface->declare(&fVslider4, "scale", "log");
        ui_interface->declare(&fVslider4, "style", "knob");
        ui_interface->declare(
            &fVslider4, "tooltip",
            "Center-frequency of second-order Regalia-Mitra peaking equalizer section 1");
        ui_interface->declare(&fVslider4, "unit", "Hz");
        ui_interface->addVerticalSlider("Eq1 Freq", &fVslider4, FAUSTFLOAT(315.0f),
                                        FAUSTFLOAT(40.0f), FAUSTFLOAT(2500.0f),
                                        FAUSTFLOAT(1.0f));
        ui_interface->declare(&fVslider5, "2", "");
        ui_interface->declare(&fVslider5, "style", "knob");
        ui_interface->declare(&fVslider5, "tooltip",
                              "Peak level   in dB of second-order Regalia-Mitra peaking "
                              "equalizer section 1");
        ui_interface->declare(&fVslider5, "unit", "dB");
        ui_interface->addVerticalSlider("Eq1 Level", &fVslider5, FAUSTFLOAT(0.0f),
                                        FAUSTFLOAT(-15.0f), FAUSTFLOAT(15.0f),
                                        FAUSTFLOAT(0.100000001f));
        ui_interface->closeBox();
        ui_interface->declare(0, "4", "");
        ui_interface->openHorizontalBox("RM Peaking Equalizer 2");
        ui_interface->declare(&fVslider2, "1", "");
        ui_interface->declare(&fVslider2, "scale", "log");
        ui_interface->declare(&fVslider2, "style", "knob");
        ui_interface->declare(
            &fVslider2, "tooltip",
            "Center-frequency of second-order Regalia-Mitra peaking equalizer section 2");
        ui_interface->declare(&fVslider2, "unit", "Hz");
        ui_interface->addVerticalSlider("Eq2 Freq", &fVslider2, FAUSTFLOAT(1500.0f),
                                        FAUSTFLOAT(160.0f), FAUSTFLOAT(10000.0f),
                                        FAUSTFLOAT(1.0f));
        ui_interface->declare(&fVslider3, "2", "");
        ui_interface->declare(&fVslider3, "style", "knob");
        ui_interface->declare(&fVslider3, "tooltip",
                              "Peak level   in dB of second-order Regalia-Mitra peaking "
                              "equalizer section 2");
        ui_interface->declare(&fVslider3, "unit", "dB");
        ui_interface->addVerticalSlider("Eq2 Level", &fVslider3, FAUSTFLOAT(0.0f),
                                        FAUSTFLOAT(-15.0f), FAUSTFLOAT(15.0f),
                                        FAUSTFLOAT(0.100000001f));
        ui_interface->closeBox();
        ui_interface->declare(0, "5", "");
        ui_interface->openHorizontalBox("Output");
        ui_interface->declare(&fVslider1, "1", "");
        ui_interface->declare(&fVslider1, "style", "knob");
        ui_interface->declare(&fVslider1, "tooltip", "Dry/Wet Mix: 0 = dry, 1 = wet");
        ui_interface->addVerticalSlider("Wet", &fVslider1, FAUSTFLOAT(0.0f),
                                        FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f),
                                        FAUSTFLOAT(0.00999999978f));
        ui_interface->declare(&fVslider0, "2", "");
        ui_interface->declare(&fVslider0, "style", "knob");
        ui_interface->declare(&fVslider0, "tooltip", "Output scale   factor");
        ui_interface->declare(&fVslider0, "unit", "dB");
        ui_interface->addVerticalSlider("Level", &fVslider0, FAUSTFLOAT(-3.0f),
                                        FAUSTFLOAT(-70.0f), FAUSTFLOAT(20.0f),
                                        FAUSTFLOAT(0.100000001f));
        ui_interface->closeBox();
        ui_interface->closeBox();
    }

    virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
    {
        FAUSTFLOAT* input0  = inputs[0];
        FAUSTFLOAT* input1  = inputs[1];
        FAUSTFLOAT* output0 = outputs[0];
        FAUSTFLOAT* output1 = outputs[1];
        float fSlow0        = fConst1 * std::pow(10.0f, 0.0500000007f * float(fVslider0));
        float fSlow1        = fConst1 * float(fVslider1);
        float fSlow2        = float(fVslider2);
        float fSlow3        = std::pow(10.0f, 0.0500000007f * float(fVslider3));
        float fSlow4        = fConst3 * fSlow2 / std::sqrt(std::max<float>(0.0f, fSlow3));
        float fSlow5        = (1.0f - fSlow4) / (fSlow4 + 1.0f);
        float fSlow6        = float(fVslider4);
        float fSlow7        = std::pow(10.0f, 0.0500000007f * float(fVslider5));
        float fSlow8        = fConst3 * fSlow6 / std::sqrt(std::max<float>(0.0f, fSlow7));
        float fSlow9        = (1.0f - fSlow8) / (fSlow8 + 1.0f);
        float fSlow10       = float(fVslider6);
        float fSlow11       = std::exp(fConst5 / fSlow10);
        float fSlow12       = zitarevdsp_faustpower2_f(fSlow11);
        float fSlow13       = std::cos(fConst3 * float(fVslider7));
        float fSlow14       = 1.0f - fSlow12 * fSlow13;
        float fSlow15       = 1.0f - fSlow12;
        float fSlow16       = std::sqrt(std::max<float>(
            0.0f, zitarevdsp_faustpower2_f(fSlow14) / zitarevdsp_faustpower2_f(fSlow15)
                      + -1.0f));
        float fSlow17       = fSlow14 / fSlow15;
        float fSlow18       = fSlow11 * (fSlow16 + 1.0f - fSlow17);
        float fSlow19       = float(fVslider8);
        float fSlow20       = std::exp(fConst5 / fSlow19) / fSlow11 + -1.0f;
        float fSlow21       = 1.0f / std::tan(fConst6 * float(fVslider9));
        float fSlow22       = 1.0f / (fSlow21 + 1.0f);
        float fSlow23       = 1.0f - fSlow21;
        float fSlow24       = fSlow17 - fSlow16;
        int iSlow25         = int(
            std::min<float>(8192.0f, std::max<float>(0.0f, fConst9 * float(fVslider10))));
        float fSlow26 = std::exp(fConst12 / fSlow10);
        float fSlow27 = zitarevdsp_faustpower2_f(fSlow26);
        float fSlow28 = 1.0f - fSlow27 * fSlow13;
        float fSlow29 = 1.0f - fSlow27;
        float fSlow30 = std::sqrt(std::max<float>(
            0.0f, zitarevdsp_faustpower2_f(fSlow28) / zitarevdsp_faustpower2_f(fSlow29)
                      + -1.0f));
        float fSlow31 = fSlow28 / fSlow29;
        float fSlow32 = fSlow26 * (fSlow30 + 1.0f - fSlow31);
        float fSlow33 = std::exp(fConst12 / fSlow19) / fSlow26 + -1.0f;
        float fSlow34 = fSlow31 - fSlow30;
        float fSlow35 = std::exp(fConst17 / fSlow10);
        float fSlow36 = zitarevdsp_faustpower2_f(fSlow35);
        float fSlow37 = 1.0f - fSlow36 * fSlow13;
        float fSlow38 = 1.0f - fSlow36;
        float fSlow39 = std::sqrt(std::max<float>(
            0.0f, zitarevdsp_faustpower2_f(fSlow37) / zitarevdsp_faustpower2_f(fSlow38)
                      + -1.0f));
        float fSlow40 = fSlow37 / fSlow38;
        float fSlow41 = fSlow35 * (fSlow39 + 1.0f - fSlow40);
        float fSlow42 = std::exp(fConst17 / fSlow19) / fSlow35 + -1.0f;
        float fSlow43 = fSlow40 - fSlow39;
        float fSlow44 = std::exp(fConst22 / fSlow10);
        float fSlow45 = zitarevdsp_faustpower2_f(fSlow44);
        float fSlow46 = 1.0f - fSlow45 * fSlow13;
        float fSlow47 = 1.0f - fSlow45;
        float fSlow48 = std::sqrt(std::max<float>(
            0.0f, zitarevdsp_faustpower2_f(fSlow46) / zitarevdsp_faustpower2_f(fSlow47)
                      + -1.0f));
        float fSlow49 = fSlow46 / fSlow47;
        float fSlow50 = fSlow44 * (fSlow48 + 1.0f - fSlow49);
        float fSlow51 = std::exp(fConst22 / fSlow19) / fSlow44 + -1.0f;
        float fSlow52 = fSlow49 - fSlow48;
        float fSlow53 = std::exp(fConst27 / fSlow10);
        float fSlow54 = zitarevdsp_faustpower2_f(fSlow53);
        float fSlow55 = 1.0f - fSlow54 * fSlow13;
        float fSlow56 = 1.0f - fSlow54;
        float fSlow57 = std::sqrt(std::max<float>(
            0.0f, zitarevdsp_faustpower2_f(fSlow55) / zitarevdsp_faustpower2_f(fSlow56)
                      + -1.0f));
        float fSlow58 = fSlow55 / fSlow56;
        float fSlow59 = fSlow53 * (fSlow57 + 1.0f - fSlow58);
        float fSlow60 = std::exp(fConst27 / fSlow19) / fSlow53 + -1.0f;
        float fSlow61 = fSlow58 - fSlow57;
        float fSlow62 = std::exp(fConst32 / fSlow10);
        float fSlow63 = zitarevdsp_faustpower2_f(fSlow62);
        float fSlow64 = 1.0f - fSlow63 * fSlow13;
        float fSlow65 = 1.0f - fSlow63;
        float fSlow66 = std::sqrt(std::max<float>(
            0.0f, zitarevdsp_faustpower2_f(fSlow64) / zitarevdsp_faustpower2_f(fSlow65)
                      + -1.0f));
        float fSlow67 = fSlow64 / fSlow65;
        float fSlow68 = fSlow62 * (fSlow66 + 1.0f - fSlow67);
        float fSlow69 = std::exp(fConst32 / fSlow19) / fSlow62 + -1.0f;
        float fSlow70 = fSlow67 - fSlow66;
        float fSlow71 = std::exp(fConst37 / fSlow10);
        float fSlow72 = zitarevdsp_faustpower2_f(fSlow71);
        float fSlow73 = 1.0f - fSlow72 * fSlow13;
        float fSlow74 = 1.0f - fSlow72;
        float fSlow75 = std::sqrt(std::max<float>(
            0.0f, zitarevdsp_faustpower2_f(fSlow73) / zitarevdsp_faustpower2_f(fSlow74)
                      + -1.0f));
        float fSlow76 = fSlow73 / fSlow74;
        float fSlow77 = fSlow71 * (fSlow75 + 1.0f - fSlow76);
        float fSlow78 = std::exp(fConst37 / fSlow19) / fSlow71 + -1.0f;
        float fSlow79 = fSlow76 - fSlow75;
        float fSlow80 = std::exp(fConst42 / fSlow10);
        float fSlow81 = zitarevdsp_faustpower2_f(fSlow80);
        float fSlow82 = 1.0f - fSlow81 * fSlow13;
        float fSlow83 = 1.0f - fSlow81;
        float fSlow84 = std::sqrt(std::max<float>(
            0.0f, zitarevdsp_faustpower2_f(fSlow82) / zitarevdsp_faustpower2_f(fSlow83)
                      + -1.0f));
        float fSlow85 = fSlow82 / fSlow83;
        float fSlow86 = fSlow80 * (fSlow84 + 1.0f - fSlow85);
        float fSlow87 = std::exp(fConst42 / fSlow19) / fSlow80 + -1.0f;
        float fSlow88 = fSlow85 - fSlow84;
        float fSlow89 = 0.0f - std::cos(fConst3 * fSlow6) * (fSlow9 + 1.0f);
        float fSlow90 = 0.0f - std::cos(fConst3 * fSlow2) * (fSlow5 + 1.0f);
        for (int i0 = 0; i0 < count; i0 = i0 + 1) {
            float fTemp0         = float(input0[i0]);
            fVec0[IOTA0 & 16383] = fTemp0;
            float fTemp1         = float(input1[i0]);
            fVec1[IOTA0 & 16383] = fTemp1;
            fRec0[0]             = fSlow0 + fConst2 * fRec0[1];
            fRec1[0]             = fSlow1 + fConst2 * fRec1[1];
            fRec15[0] = 0.0f - fSlow22 * (fSlow23 * fRec15[1] - (fRec11[1] + fRec11[2]));
            fRec14[0] = fSlow18 * (fRec11[1] + fSlow20 * fRec15[0]) + fSlow24 * fRec14[1];
            fVec2[IOTA0 & 32767] = 0.353553385f * fRec14[0] + 9.99999968e-21f;
            float fTemp2         = 0.300000012f * fVec1[(IOTA0 - iSlow25) & 16383];
            float fTemp3 =
                (0.600000024f * fRec12[1] + fVec2[(IOTA0 - iConst8) & 32767]) - fTemp2;
            fVec3[IOTA0 & 2047] = fTemp3;
            fRec12[0]           = fVec3[(IOTA0 - iConst10) & 2047];
            float fRec13        = 0.0f - 0.600000024f * fTemp3;
            fRec19[0] = 0.0f - fSlow22 * (fSlow23 * fRec19[1] - (fRec7[1] + fRec7[2]));
            fRec18[0] = fSlow32 * (fRec7[1] + fSlow33 * fRec19[0]) + fSlow34 * fRec18[1];
            fVec4[IOTA0 & 32767] = 0.353553385f * fRec18[0] + 9.99999968e-21f;
            float fTemp4 =
                (0.600000024f * fRec16[1] + fVec4[(IOTA0 - iConst14) & 32767]) - fTemp2;
            fVec5[IOTA0 & 4095] = fTemp4;
            fRec16[0]           = fVec5[(IOTA0 - iConst15) & 4095];
            float fRec17        = 0.0f - 0.600000024f * fTemp4;
            fRec23[0] = 0.0f - fSlow22 * (fSlow23 * fRec23[1] - (fRec9[1] + fRec9[2]));
            fRec22[0] = fSlow41 * (fRec9[1] + fSlow42 * fRec23[0]) + fSlow43 * fRec22[1];
            fVec6[IOTA0 & 16383] = 0.353553385f * fRec22[0] + 9.99999968e-21f;
            float fTemp5 =
                fVec6[(IOTA0 - iConst19) & 16383] + fTemp2 + 0.600000024f * fRec20[1];
            fVec7[IOTA0 & 4095] = fTemp5;
            fRec20[0]           = fVec7[(IOTA0 - iConst20) & 4095];
            float fRec21        = 0.0f - 0.600000024f * fTemp5;
            fRec27[0] = 0.0f - fSlow22 * (fSlow23 * fRec27[1] - (fRec5[1] + fRec5[2]));
            fRec26[0] = fSlow50 * (fRec5[1] + fSlow51 * fRec27[0]) + fSlow52 * fRec26[1];
            fVec8[IOTA0 & 32767] = 0.353553385f * fRec26[0] + 9.99999968e-21f;
            float fTemp6 =
                fTemp2 + 0.600000024f * fRec24[1] + fVec8[(IOTA0 - iConst24) & 32767];
            fVec9[IOTA0 & 4095] = fTemp6;
            fRec24[0]           = fVec9[(IOTA0 - iConst25) & 4095];
            float fRec25        = 0.0f - 0.600000024f * fTemp6;
            fRec31[0] = 0.0f - fSlow22 * (fSlow23 * fRec31[1] - (fRec10[1] + fRec10[2]));
            fRec30[0] = fSlow59 * (fRec10[1] + fSlow60 * fRec31[0]) + fSlow61 * fRec30[1];
            fVec10[IOTA0 & 16383] = 0.353553385f * fRec30[0] + 9.99999968e-21f;
            float fTemp7          = 0.300000012f * fVec0[(IOTA0 - iSlow25) & 16383];
            float fTemp8 =
                fVec10[(IOTA0 - iConst29) & 16383] - (fTemp7 + 0.600000024f * fRec28[1]);
            fVec11[IOTA0 & 2047] = fTemp8;
            fRec28[0]            = fVec11[(IOTA0 - iConst30) & 2047];
            float fRec29         = 0.600000024f * fTemp8;
            fRec35[0] = 0.0f - fSlow22 * (fSlow23 * fRec35[1] - (fRec6[1] + fRec6[2]));
            fRec34[0] = fSlow68 * (fRec6[1] + fSlow69 * fRec35[0]) + fSlow70 * fRec34[1];
            fVec12[IOTA0 & 16383] = 0.353553385f * fRec34[0] + 9.99999968e-21f;
            float fTemp9 =
                fVec12[(IOTA0 - iConst34) & 16383] - (fTemp7 + 0.600000024f * fRec32[1]);
            fVec13[IOTA0 & 4095] = fTemp9;
            fRec32[0]            = fVec13[(IOTA0 - iConst35) & 4095];
            float fRec33         = 0.600000024f * fTemp9;
            fRec39[0] = 0.0f - fSlow22 * (fSlow23 * fRec39[1] - (fRec8[1] + fRec8[2]));
            fRec38[0] = fSlow77 * (fRec8[1] + fSlow78 * fRec39[0]) + fSlow79 * fRec38[1];
            fVec14[IOTA0 & 16383] = 0.353553385f * fRec38[0] + 9.99999968e-21f;
            float fTemp10 =
                (fTemp7 + fVec14[(IOTA0 - iConst39) & 16383]) - 0.600000024f * fRec36[1];
            fVec15[IOTA0 & 4095] = fTemp10;
            fRec36[0]            = fVec15[(IOTA0 - iConst40) & 4095];
            float fRec37         = 0.600000024f * fTemp10;
            fRec43[0] = 0.0f - fSlow22 * (fSlow23 * fRec43[1] - (fRec4[1] + fRec4[2]));
            fRec42[0] = fSlow86 * (fRec4[1] + fSlow87 * fRec43[0]) + fSlow88 * fRec42[1];
            fVec16[IOTA0 & 16383] = 0.353553385f * fRec42[0] + 9.99999968e-21f;
            float fTemp11 =
                (fVec16[(IOTA0 - iConst44) & 16383] + fTemp7) - 0.600000024f * fRec40[1];
            fVec17[IOTA0 & 2047] = fTemp11;
            fRec40[0]            = fVec17[(IOTA0 - iConst45) & 2047];
            float fRec41         = 0.600000024f * fTemp11;
            float fTemp12        = fRec40[1] + fRec36[1];
            float fTemp13 =
                fRec29 + fRec33 + fRec37 + fRec41 + fRec28[1] + fTemp12 + fRec32[1];
            fRec4[0] = fRec12[1] + fRec16[1] + fRec20[1] + fRec24[1] + fRec13 + fRec17
                       + fRec21 + fRec25 + fTemp13;
            fRec5[0] = fTemp13
                       - (fRec12[1] + fRec16[1] + fRec20[1] + fRec24[1] + fRec13 + fRec17
                          + fRec25 + fRec21);
            float fTemp14 = fRec37 + fRec41 + fTemp12;
            float fTemp15 = fRec29 + fRec33 + fRec32[1] + fRec28[1];
            fRec6[0]      = (fRec20[1] + fRec24[1] + fRec21 + fRec25 + fTemp14)
                       - (fRec12[1] + fRec16[1] + fRec13 + fRec17 + fTemp15);
            fRec7[0] = (fRec12[1] + fRec16[1] + fRec13 + fRec17 + fTemp14)
                       - (fRec20[1] + fRec24[1] + fRec21 + fRec25 + fTemp15);
            float fTemp16 = fRec33 + fRec41 + fRec40[1] + fRec32[1];
            float fTemp17 = fRec29 + fRec37 + fRec36[1] + fRec28[1];
            fRec8[0]      = (fRec16[1] + fRec24[1] + fRec17 + fRec25 + fTemp16)
                       - (fRec12[1] + fRec20[1] + fRec13 + fRec21 + fTemp17);
            fRec9[0] = (fRec12[1] + fRec20[1] + fRec13 + fRec21 + fTemp16)
                       - (fRec16[1] + fRec24[1] + fRec17 + fRec25 + fTemp17);
            float fTemp18 = fRec29 + fRec41 + fRec40[1] + fRec28[1];
            float fTemp19 = fRec33 + fRec37 + fRec36[1] + fRec32[1];
            fRec10[0]     = (fRec12[1] + fRec24[1] + fRec13 + fRec25 + fTemp18)
                        - (fRec16[1] + fRec20[1] + fRec17 + fRec21 + fTemp19);
            fRec11[0] = (fRec16[1] + fRec20[1] + fRec17 + fRec21 + fTemp18)
                        - (fRec12[1] + fRec24[1] + fRec13 + fRec25 + fTemp19);
            float fTemp20 = 0.370000005f * (fRec5[0] + fRec6[0]);
            float fTemp21 = fSlow89 * fRec3[1];
            fRec3[0]      = fTemp20 - (fTemp21 + fSlow9 * fRec3[2]);
            float fTemp22 = fSlow9 * fRec3[0];
            float fTemp23 = 0.5f
                            * (fTemp22 + fRec3[2] + fTemp20 + fTemp21
                               + fSlow7 * ((fTemp22 + fTemp21 + fRec3[2]) - fTemp20));
            float fTemp24 = fSlow90 * fRec2[1];
            fRec2[0]      = fTemp23 - (fTemp24 + fSlow5 * fRec2[2]);
            float fTemp25 = fSlow5 * fRec2[0];
            float fTemp26 = 1.0f - fRec1[0];
            output0[i0]   = FAUSTFLOAT(
                fRec0[0]
                * (0.5f * fRec1[0]
                       * (fTemp25 + fRec2[2] + fTemp23 + fTemp24
                          + fSlow3 * ((fTemp25 + fTemp24 + fRec2[2]) - fTemp23))
                   + fTemp0 * fTemp26));
            float fTemp27 = 0.370000005f * (fRec5[0] - fRec6[0]);
            float fTemp28 = fSlow89 * fRec45[1];
            fRec45[0]     = fTemp27 - (fTemp28 + fSlow9 * fRec45[2]);
            float fTemp29 = fSlow9 * fRec45[0];
            float fTemp30 = 0.5f
                            * (fTemp29 + fRec45[2] + fTemp27 + fTemp28
                               + fSlow7 * ((fTemp29 + fTemp28 + fRec45[2]) - fTemp27));
            float fTemp31 = fSlow90 * fRec44[1];
            fRec44[0]     = fTemp30 - (fTemp31 + fSlow5 * fRec44[2]);
            float fTemp32 = fSlow5 * fRec44[0];
            output1[i0]   = FAUSTFLOAT(
                fRec0[0]
                * (0.5f * fRec1[0]
                       * (fTemp32 + fRec44[2] + fTemp30 + fTemp31
                          + fSlow3 * ((fTemp32 + fTemp31 + fRec44[2]) - fTemp30))
                   + fTemp1 * fTemp26));
            IOTA0     = IOTA0 + 1;
            fRec0[1]  = fRec0[0];
            fRec1[1]  = fRec1[0];
            fRec15[1] = fRec15[0];
            fRec14[1] = fRec14[0];
            fRec12[1] = fRec12[0];
            fRec19[1] = fRec19[0];
            fRec18[1] = fRec18[0];
            fRec16[1] = fRec16[0];
            fRec23[1] = fRec23[0];
            fRec22[1] = fRec22[0];
            fRec20[1] = fRec20[0];
            fRec27[1] = fRec27[0];
            fRec26[1] = fRec26[0];
            fRec24[1] = fRec24[0];
            fRec31[1] = fRec31[0];
            fRec30[1] = fRec30[0];
            fRec28[1] = fRec28[0];
            fRec35[1] = fRec35[0];
            fRec34[1] = fRec34[0];
            fRec32[1] = fRec32[0];
            fRec39[1] = fRec39[0];
            fRec38[1] = fRec38[0];
            fRec36[1] = fRec36[0];
            fRec43[1] = fRec43[0];
            fRec42[1] = fRec42[0];
            fRec40[1] = fRec40[0];
            fRec4[2]  = fRec4[1];
            fRec4[1]  = fRec4[0];
            fRec5[2]  = fRec5[1];
            fRec5[1]  = fRec5[0];
            fRec6[2]  = fRec6[1];
            fRec6[1]  = fRec6[0];
            fRec7[2]  = fRec7[1];
            fRec7[1]  = fRec7[0];
            fRec8[2]  = fRec8[1];
            fRec8[1]  = fRec8[0];
            fRec9[2]  = fRec9[1];
            fRec9[1]  = fRec9[0];
            fRec10[2] = fRec10[1];
            fRec10[1] = fRec10[0];
            fRec11[2] = fRec11[1];
            fRec11[1] = fRec11[0];
            fRec3[2]  = fRec3[1];
            fRec3[1]  = fRec3[0];
            fRec2[2]  = fRec2[1];
            fRec2[1]  = fRec2[0];
            fRec45[2] = fRec45[1];
            fRec45[1] = fRec45[0];
            fRec44[2] = fRec44[1];
            fRec44[1] = fRec44[0];
        }
    }
};

#endif
