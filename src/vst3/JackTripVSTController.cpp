//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024-2025 JackTrip Labs, Inc.

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

// Based on the Hello World VST 3 example from Steinberg
// https://github.com/steinbergmedia/vst3_example_plugin_hello_world

#include "JackTripVSTController.h"

#include "JackTripVST.h"
#include "JackTripVSTDataBlock.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;

// the number of parameters used by the plugin
constexpr int32 JackTripVSTNumParameters = 5;

//------------------------------------------------------------------------
// JackTripVSTController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::initialize(FUnknown* context)
{
    // Here the Plug-in will be instantiated

    //---do not forget to call parent ------
    tresult result = EditControllerEx1::initialize(context);
    if (result != kResultOk) {
        return result;
    }

    // Here you could register some parameters
    if (result == kResultTrue) {
        //---Create Parameters------------
        parameters.addParameter(STR16("Send Gain"), STR16("dB"), 199, 1.,
                                Vst::ParameterInfo::kCanAutomate,
                                JackTripVSTParams::kParamGainSendId, 0, STR16("Send"));

        parameters.addParameter(STR16("Output Mix"), STR16("dB"), 199, 0,
                                Vst::ParameterInfo::kCanAutomate,
                                JackTripVSTParams::kParamMixOutputId, 0, STR16("Mix"));

        parameters.addParameter(STR16("Output Gain"), STR16("dB"), 199, 1.,
                                Vst::ParameterInfo::kCanAutomate,
                                JackTripVSTParams::kParamGainOutputId, 0, STR16("Gain"));

        parameters.addParameter(
            STR16("Connected"), STR16("On/Off"), 1, 0, Vst::ParameterInfo::kIsReadOnly,
            JackTripVSTParams::kParamConnectedId, 0, STR16("Connected"));

        parameters.addParameter(
            STR16("Bypass"), nullptr, 1, 0,
            Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
            JackTripVSTParams::kBypassId);
    }

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::terminate()
{
    // Here the Plug-in will be de-instantiated, last possibility to remove some memory!

    //---do not forget to call parent ------
    return EditControllerEx1::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::setComponentState(IBStream* state)
{
    // Here you get the state of the component (Processor part)
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    float sendGain = 1.f;
    if (streamer.readFloat(sendGain) == false)
        return kResultFalse;
    setParamNormalized(JackTripVSTParams::kParamGainSendId, sendGain);

    float outputMix = 1.f;
    if (streamer.readFloat(outputMix) == false)
        return kResultFalse;
    setParamNormalized(JackTripVSTParams::kParamMixOutputId, outputMix);

    float outputGain = 1.f;
    if (streamer.readFloat(outputGain) == false)
        return kResultFalse;
    setParamNormalized(JackTripVSTParams::kParamGainOutputId, outputGain);

    int8 connectedState = 0;
    if (streamer.readInt8(connectedState) == false)
        return kResultFalse;
    setParamNormalized(JackTripVSTParams::kParamConnectedId, connectedState);

    int32 bypassState;
    if (streamer.readInt32(bypassState) == false)
        return kResultFalse;
    setParamNormalized(kBypassId, bypassState ? 1 : 0);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::setState([[maybe_unused]] IBStream* state)
{
    // Here you get the state of the controller

    return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::getState([[maybe_unused]] IBStream* state)
{
    // Here you are asked to deliver the state of the controller (if needed)
    // Note: the real state of your plug-in is saved in the processor

    return kResultTrue;
}

//------------------------------------------------------------------------
int32 PLUGIN_API JackTripVSTController::getParameterCount()
{
    return JackTripVSTNumParameters;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API JackTripVSTController::createView(FIDString name)
{
    // Here the Host wants to open your editor (if you have one)
    if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
        // create your editor here and return a IPlugView ptr of it
        auto* view = new VSTGUI::VST3Editor(this, "view", "JackTripEditor.uidesc");
        return view;
    }
    return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::setParamNormalized(Vst::ParamID tag,
                                                             Vst::ParamValue value)
{
    // called by host to update your parameters
    tresult result = EditControllerEx1::setParamNormalized(tag, value);
    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::getParamStringByValue(
    Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
    // called by host to get a string for given normalized value of a specific parameter
    // (without having to set the value!)
    return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::getParamValueByString(
    Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
    // called by host to get a normalized value from a string representation of a specific
    // parameter (without having to set the value!)
    return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::notify(Vst::IMessage* message)
{
    if (mDataExchangeHandler.onMessage(message))
        return kResultTrue;
    return EditControllerEx1::notify(message);
}

//------------------------------------------------------------------------
void PLUGIN_API JackTripVSTController::queueOpened(
    [[maybe_unused]] Vst::DataExchangeUserContextID userContextID,
    [[maybe_unused]] uint32 blockSize, [[maybe_unused]] TBool& dispatchOnBackgroundThread)
{
    // qDebug() << "Data Exchange Queue opened.\n";
}

//------------------------------------------------------------------------
void PLUGIN_API JackTripVSTController::queueClosed(
    [[maybe_unused]] Vst::DataExchangeUserContextID userContextID)
{
    // qDebug() << "Data Exchange Queue closed.\n";
}

//------------------------------------------------------------------------
void PLUGIN_API JackTripVSTController::onDataExchangeBlocksReceived(
    [[maybe_unused]] Vst::DataExchangeUserContextID userContextID, uint32 numBlocks,
    Vst::DataExchangeBlock* blocks, [[maybe_unused]] TBool onBackgroundThread)
{
    for (auto index = 0u; index < numBlocks; ++index) {
        auto dataBlock = toDataBlock(blocks[index]);
        beginEdit(JackTripVSTParams::kParamConnectedId);
        Vst::ParamValue connectedState = dataBlock->connectedState ? 1 : 0;
        if (setParamNormalized(JackTripVSTParams::kParamConnectedId, connectedState)
            == kResultOk) {
            performEdit(JackTripVSTParams::kParamConnectedId,
                        getParamNormalized(JackTripVSTParams::kParamConnectedId));
        }
        endEdit(JackTripVSTParams::kParamConnectedId);
    }
}
