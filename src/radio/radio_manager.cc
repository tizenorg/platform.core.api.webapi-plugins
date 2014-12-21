// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radio_manager.h"

#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <glib.h>
#include <runtime_info.h>
#include <vconf.h>

#include "common/logger.h"
#include "common/extension.h"
#include "common/platform_exception.h"

using namespace common;
using namespace std;

namespace extension {
namespace radio {

char* state_array[] = { "READY", "PLAYING", "SCANNING" ,""};
static const double FREQ_LOWER = 87.5;

static void createEventSuccess(picojson::object& obj,double callbackId)
{
    obj.insert(std::make_pair("callbackId", callbackId));
    obj.insert(std::make_pair("status", picojson::value("success")));

    picojson::value event = picojson::value(obj);
    RadioInstance::getInstance().PostMessage(event.serialize().c_str());
}

static void createEventSuccess(double callbackId)
{
    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();

    createEventSuccess(obj,callbackId);
}


static void createEventFail(double callbackId,const PlatformException& ex)
{

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();

    obj.insert(std::make_pair("error", ex.ToJSON()));
    obj.insert(std::make_pair("callbackId", callbackId));
    obj.insert(std::make_pair("status", picojson::value("error")));

    RadioInstance::getInstance().PostMessage(event.serialize().c_str());
 }

int FMRadioManager::Create()
{
    if (radio_instance == NULL)
    {
        int err = radio_create(&radio_instance);
        if(err){
            LoggerE("radio_create %d",err);
            radio_instance=NULL;
        }
        return err;
    }
    return 0;
}

bool FMRadioManager::MuteGetter()
{
    bool muted = false;
    int err = radio_is_muted (radio_instance, &muted);
    if (RADIO_ERROR_NONE != err) {
        LoggerE("radio_is_muted %d",err);
    }
    return muted;
}

void FMRadioManager::MuteSetter(const picojson::value& args)
{
    bool mute = args.get("mute").get<bool>();

    int err = radio_set_mute (radio_instance, mute);
    if (err)
    {
        LoggerE("radio_set_mute %d",err);
    }
}

bool FMRadioManager::AntennaGetter()
{
    bool connected = false;
    int err = runtime_info_get_value_bool (RUNTIME_INFO_KEY_AUDIO_JACK_CONNECTED , &connected);

    if (err)
    {
        return false;
    }
    return connected;
}



char* FMRadioManager::StateGetter()
{
    LoggerD("FMRadioManager::FrequencyGetter()");

    radio_state_e state;

    int err =  radio_get_state (radio_instance, &state);
    if (err)
    {
        return state_array[3]; // returns ""
    }

    return state_array[static_cast<int>(state)];
}

double FMRadioManager::FrequencyGetter()
{
    int freq;
    LoggerD("FMRadioManager::FrequencyGetter()");
    int err = radio_get_frequency(radio_instance,&freq);

    if (RADIO_ERROR_NONE != err)
    {
        LoggerE("radio_get_frequency() error %d",err);
        return FREQ_LOWER;
    }
    return static_cast<double>(freq/1000);
}

double FMRadioManager::SignalStrengthGetter()
{
    int stren;

    LoggerD("FMRadioManager::SignalStrengthGetter()");
    int err = radio_get_signal_strength(radio_instance,&stren);
    if (RADIO_ERROR_NONE != err)
    {
        LoggerE("radio_get_signal_strength()");
        return 0;
    }

    return static_cast<double>(stren);
}

FMRadioManager::FMRadioManager() : radio_instance(NULL)
{
    LoggerD("FMRadioManager::FMRadioManager()");
}

FMRadioManager::~FMRadioManager()
{

    LoggerD("FMRadioManager::~FMRadioManager()");
    int err = radio_destroy(radio_instance);
    LoggerD("radio_destroy() error %d",err);
}

FMRadioManager* FMRadioManager::GetInstance(bool safe)
{

    static FMRadioManager instance;

    int err=instance.Create();
    if(safe)
    {
        CheckErr("radio_create",err);
    }
    LoggerD("FMRadioManager::GetInstance()");

    return &instance;
}

void FMRadioManager::CheckErr(string str,int err)
{

    LoggerE("%s() error %d",str.c_str(),err);

    if(RADIO_ERROR_NONE == err)
    {
        return;
    }
    else if (RADIO_ERROR_INVALID_PARAMETER == err)
    {
        throw common::InvalidValuesException(str);
    }
    else if(RADIO_ERROR_INVALID_STATE == err)
    {
        throw common::InvalidStateException(str);
    }
    else if(RADIO_ERROR_SOUND_POLICY == err)
    {
        throw common::UnknownException(str);
    }
    else if(RADIO_ERROR_NOT_SUPPORTED == err)
    {
        throw common::ServiceNotAvailableException(str);
    }
    else
    {
        throw common::UnknownException(str);
    }

}

void FMRadioManager::Start(double freq)
{
    LoggerD("FMRadioManager::Start(%f)",freq);

    int err = radio_set_frequency (radio_instance, freq*1000);
    CheckErr("radio_set_frequency",err);

    err= radio_start(radio_instance);
    CheckErr("radio_start",err);
}

void FMRadioManager::Stop()
{
    int err= radio_stop(radio_instance);
    CheckErr("radio_stop",err);
}

PlatformException FMRadioManager::GetException(char * name,int err)
{

    string buff=string(name);
    buff+=" : ";
    buff+=to_string(err);

    if(RADIO_ERROR_INVALID_PARAMETER == err)
    {
        return InvalidValuesException(buff);
    }
    else if(RADIO_ERROR_INVALID_OPERATION == err)
    {
        return UnknownException(buff);
    }
    else if(RADIO_ERROR_INVALID_STATE == err)
    {
        return InvalidStateException(buff);
    }

    else if(RADIO_ERROR_SOUND_POLICY == err)
    {
        return UnknownException(buff);
    }
    else if(RADIO_ERROR_PERMISSION_DENIED == err)
    {
        return UnknownException(buff);
    }
    else if(RADIO_ERROR_OUT_OF_MEMORY == err)
    {
        return UnknownException(buff);
    }
    else
    {
        return UnknownException(buff);
    }
}

void FMRadioManager::RadioSeekCB(int frequency, void *user_data)
{
    double * id = static_cast<double*>(user_data);

    int err = radio_set_frequency(*(FMRadioManager::GetInstance(0)->GetRadioInstance()),frequency);

    if (RADIO_ERROR_NONE != err)
    {
        createEventFail(*id,GetException("radio_set_frequency",err));
    }
    else
    {
        createEventSuccess(*id);
    }

    delete id;
}

radio_h* FMRadioManager::GetRadioInstance()
{
    return &radio_instance;
}

void FMRadioManager::SeekUp(const picojson::value& args){

    double * callbackId = new double(args.get("callbackId").get<double>());

    int err = radio_seek_up (radio_instance,RadioSeekCB,static_cast<void *>(callbackId));

    if(RADIO_ERROR_NONE != err)
    {

        delete callbackId;
        createEventFail(*callbackId,GetException("radio_set_frequency",err));
    }
}

void FMRadioManager::SeekDown(const picojson::value& args){

    double * callbackId = new double(args.get("callbackId").get<double>());

     int err = radio_seek_down (radio_instance,RadioSeekCB,static_cast<void *>(callbackId));

     if(RADIO_ERROR_NONE != err)
     {

         delete callbackId;
         createEventFail(*callbackId,GetException("radio_set_frequency",err));
     }
}

void FMRadioManager::ScanStartCB(int frequency, void *user_data)
{
    std::vector<double> * freqs =  FMRadioManager::GetInstance()->GetFreqs();
    freqs->push_back(static_cast<double>(frequency));

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    obj.insert(std::make_pair("frequency", picojson::value(static_cast<double>(frequency))));
    obj.insert(std::make_pair("listenerId", "FMRadio_Onfrequencyfound"));
    RadioInstance::getInstance().PostMessage(event.serialize().c_str());

}

void FMRadioManager::ScanStopCB(void *user_data)
{
    createEventSuccess( *(static_cast<double*>(user_data)));
    delete static_cast<double*>(user_data);

}

std::vector<double> * FMRadioManager::GetFreqs()
{
    return &freqs;
}

void FMRadioManager::ScanCompleteCB(void *user_data)
{
    double * id= static_cast<double*>(user_data);

    std::vector<double> * freqs =  FMRadioManager::GetInstance()->GetFreqs();

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    obj.insert(std::make_pair("name", picojson::value("onfinished")));

    std::vector<picojson::value> vect = std::vector<picojson::value> ();
    for (std::vector<double>::iterator it = freqs->begin() ; it != freqs->end(); ++it)
    {
        vect.push_back(picojson::value(*it));
    }

    obj.insert(std::make_pair("frequencies", picojson::value(vect)));
    createEventSuccess(obj,*id);

    delete id;
}



void FMRadioManager::ScanStart(const picojson::value& args)
{
    LoggerD("FMRadioManager::ScanStart()");

    double *callbackId = new double(args.get("callbackId").get<double>());

    freqs = std::vector<double>();

    int err =radio_set_scan_completed_cb(radio_instance,ScanCompleteCB,callbackId);
    if(err)
    {
        createEventFail(*callbackId,GetException("radio_set_scan_completed_cb %d",err));
        delete callbackId;
        return;
    }

    err = radio_scan_start (radio_instance, ScanStartCB, NULL);
    if(err)
    {
        radio_unset_scan_completed_cb(radio_instance);
        createEventFail(*callbackId,GetException("radio_scan_start %d",err));
        delete callbackId;
    }

}
void FMRadioManager::ScanStop(const picojson::value& args){
    LoggerD("FMRadioManager::ScanStop()");

    double* callbackId = new double(args.get("callbackId").get<double>());

    int ret = radio_unset_scan_completed_cb(radio_instance);
    if(ret)
    {
        createEventFail(*callbackId,GetException("radio_unset_scan_completed_cb %d",ret));
        delete callbackId;
        return;
    }

    ret = radio_scan_stop (radio_instance, ScanStopCB, callbackId);
    if(ret)
    {
        createEventFail(*callbackId,GetException("radio_scan_stop %d",ret));
        delete callbackId;
    }

}

string FMRadioManager::TranslateCode(int code)
{
    if (RADIO_INTERRUPTED_BY_MEDIA == code)
    {
        return string("RADIO_INTERRUPTED_BY_MEDIA");
    }
    else if(RADIO_INTERRUPTED_BY_RESUMABLE_MEDIA  == code)
    {
        return string("RADIO_INTERRUPTED_BY_RESUMABLE_MEDIA");
    }
    else if(RADIO_INTERRUPTED_BY_CALL == code)
    {
        return string("RADIO_INTERRUPTED_BY_CALL");
    }
    else if(RADIO_INTERRUPTED_BY_EARJACK_UNPLUG == code)
    {
        return string("RADIO_INTERRUPTED_BY_EARJACK_UNPLUG");
    }
    else if(RADIO_INTERRUPTED_BY_RESOURCE_CONFLICT == code)
    {
        return string("RADIO_INTERRUPTED_BY_RESOURCE_CONFLICT");
    }
    else if(RADIO_INTERRUPTED_BY_ALARM == code)
    {
        return string("RADIO_INTERRUPTED_BY_ALARM");
    }
    else if(RADIO_INTERRUPTED_BY_EMERGENCY == code)
    {
        return string("RADIO_INTERRUPTED_BY_EMERGENCY");
    }
    else if(RADIO_INTERRUPTED_BY_MEDIA == code)
    {
        return string("RADIO_INTERRUPTED_BY_RESUMABLE_MEDIA");
    }
    else if(RADIO_INTERRUPTED_BY_NOTIFICATION == code)
    {
        return string("RADIO_INTERRUPTED_BY_NOTIFICATION");
    }
}

void FMRadioManager::RadioInterruptedCB(radio_interrupted_code_e code, void *user_data)
{

    if(code == RADIO_INTERRUPTED_COMPLETED )
    {
        picojson::value event = picojson::value(picojson::object());
        picojson::object& obj = event.get<picojson::object>();
        obj.insert(std::make_pair("action", picojson::value("oninterruptfinished")));
        obj.insert(std::make_pair("listenerId", "FMRadio_Interrupted"));
        RadioInstance::getInstance().PostMessage(event.serialize().c_str());
    }
    else
    {
        picojson::value event = picojson::value(picojson::object());
        picojson::object& obj = event.get<picojson::object>();
        obj.insert(std::make_pair("action", picojson::value("oninterrupted")));

        obj.insert(std::make_pair("reason", TranslateCode(code)));

        obj.insert(std::make_pair("listenerId", "FMRadio_Interrupted"));
        RadioInstance::getInstance().PostMessage(event.serialize().c_str());
    }

}

void FMRadioManager::SetFMRadioInterruptedListener(){
    LoggerD("FMRadioManager::SetFMRadioInterruptedListener()");
    int err = radio_set_interrupted_cb (radio_instance, RadioInterruptedCB, NULL);
    if(RADIO_ERROR_NONE != err)
    {
        CheckErr("radio_set_interrupted_cb",err);
    }
}
void FMRadioManager::UnsetFMRadioInterruptedListener(){
    LoggerD("FMRadioManager::UnsetFMRadioInterruptedListener()");
    int err = radio_unset_interrupted_cb(radio_instance);
    if(RADIO_ERROR_NONE != err)
    {
        CheckErr("radio_unset_interrupted_cb",err);
    }
}

void FMRadioManager::RadioAntennaCB(runtime_info_key_e key, void *user_data)
{
    LoggerD("FMRadioManager::RadioAntennaCB()");

    bool connected=false;
    int err = runtime_info_get_value_bool (key, &connected);
    if(RADIO_ERROR_NONE != err)
    {
        LoggerE("FMRadioManager::RadioAntennaCB() runtime_info_get_value_bool %d",err);
    }

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();

    obj.insert(std::make_pair("connected", picojson::value(connected)));

    obj.insert(std::make_pair("listenerId", "FMRadio_Antenna"));
    RadioInstance::getInstance().PostMessage(event.serialize().c_str());

}

void FMRadioManager::SetAntennaChangeListener(){
    LoggerD("FMRadioManager::SetAntennaChangeListener()");
    int err = runtime_info_set_changed_cb (RUNTIME_INFO_KEY_AUDIO_JACK_CONNECTED ,
            RadioAntennaCB, NULL);
    if(RADIO_ERROR_NONE != err)
    {
        CheckErr("runtime_info_set_changed_cb",err);
    }

}
void FMRadioManager::UnsetAntennaChangeListener(){
    LoggerD("FMRadioManager::UnsetAntennaChangeListener()");
    int err = runtime_info_unset_changed_cb (RUNTIME_INFO_KEY_AUDIO_JACK_CONNECTED);
    if(RADIO_ERROR_NONE != err)
    {
        CheckErr("runtime_info_unset_changed_cb",err);
    }

}

} // namespace
} // namespace extension

