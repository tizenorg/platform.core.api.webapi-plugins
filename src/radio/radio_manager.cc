// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radio_manager.h"

#include <unistd.h>

#include <cstring>
#include <algorithm>

#include <glib.h>

#include <vconf.h>

#include "common/logger.h"
#include "common/extension.h"
#include "common/platform_exception.h"

using namespace common;
using namespace std;

namespace extension {
namespace radio {

typedef struct RadioSeekCBstruct_
{
    double cbid;
    radio_h radio_instance;
} RadioSeekCBstruct;


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
    if (radio_instance==NULL)
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

FMRadioManager* FMRadioManager::GetInstance()
{

    static FMRadioManager instance;

    instance.Create();
    return &instance;
}

void FMRadioManager::CheckErr(string str,int err)
{
    if(err)
    {
        LoggerE("%s() error %d",str.c_str(),err);
    }

    if (err==RADIO_ERROR_INVALID_PARAMETER  )
    {
        throw common::InvalidValuesException(str);
    }
    else if(err==RADIO_ERROR_INVALID_STATE)
    {
        throw common::InvalidValuesException(str);
    }
    else if(err==RADIO_ERROR_SOUND_POLICY)
    {
        throw common::UnknownException(str);
    }
    else if(err==RADIO_ERROR_NOT_SUPPORTED)
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

    int err = radio_set_frequency (radio_instance, freq);
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
    const int SIZE = 200;
    char buff[SIZE];
    snprintf(buff,SIZE-1,"%s:%d",name,err);

    //TODO Split exception types
    return UnknownException(buff);

}

void FMRadioManager::RadioSeekCB(int frequency, void *user_data)
{
    RadioSeekCBstruct * data = static_cast<RadioSeekCBstruct*>(user_data);
    double callbackId = data->cbid;
    int err = radio_set_frequency(static_cast<radio_h>(data->radio_instance),frequency);

    if (RADIO_ERROR_NONE != err)
    {
        createEventFail(callbackId,GetException("radio_set_frequency",err));
    }
    else
    {
        createEventSuccess(callbackId);
    }

    delete data;
}

void FMRadioManager::SeekUp(const picojson::value& args){

    double callbackId = args.get("callbackId").get<double>();
    RadioSeekCBstruct *data= new RadioSeekCBstruct;
    data->cbid = callbackId;
    data->radio_instance = radio_instance;
    int err = radio_seek_up (radio_instance,RadioSeekCB,static_cast<void *>(data));
    if(RADIO_ERROR_NONE != err)
    {
        delete data;
        createEventFail(callbackId,GetException("radio_set_frequency",err));
    }

}

void FMRadioManager::SeekDown(const picojson::value& args){

    double callbackId = args.get("callbackId").get<double>();
        RadioSeekCBstruct *data= new RadioSeekCBstruct;
        data->cbid = callbackId;
        data->radio_instance = radio_instance;
        int err = radio_seek_down (radio_instance,RadioSeekCB,static_cast<void *>(data));
        if(RADIO_ERROR_NONE != err)
        {
            delete data;
            createEventFail(callbackId,GetException("radio_set_frequency",err));
        }
}

void FMRadioManager::ScanStart(const picojson::value& args){
    LoggerD("FMRadioManager::StartScan()");
}
void FMRadioManager::ScanStop(const picojson::value& args){
    LoggerD("FMRadioManager::StopScan()");
}
void FMRadioManager::SetFMRadioInterruptedListener(){
    LoggerD("FMRadioManager::SetFMRadioInterruptedListener()");
}
void FMRadioManager::UnsetFMRadioInterruptedListener(){
    LoggerD("FMRadioManager::UnsetFMRadioInterruptedListener()");
}
void FMRadioManager::SetAntennaChangeListener(){
    LoggerD("FMRadioManager::SetAntennaChangeListener()");
}
void FMRadioManager::UnsetAntennaChangeListener(){
    LoggerD("FMRadioManager::UnetAntennaChangeListener()");
}

} // namespace
} // namespace extension

