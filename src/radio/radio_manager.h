// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FMRADIO_FMRADIO_MANAGER_H_
#define FMRADIO_FMRADIO_MANAGER_H_

#include <string>
#include <list>
#include <device/callback.h>
#include "common/picojson.h"
#include "common/platform_exception.h"

#include "radio_instance.h"

#include <radio.h>

namespace extension {
namespace radio {

class FMRadioManager {
public:

    static FMRadioManager* GetInstance();

    void Start(double freq);
    void Stop();
    void SeekUp(const picojson::value& args);
    void SeekDown(const picojson::value& args);
    void ScanStart(const picojson::value& args);
    void ScanStop(const picojson::value& args);
    void SetFMRadioInterruptedListener();
    void UnsetFMRadioInterruptedListener();
    void SetAntennaChangeListener();
    void UnsetAntennaChangeListener();

    double FrequencyGetter();
    double SignalStrengthGetter();

private:

    static common::PlatformException GetException(char * name,int err);
    static void RadioSeekCB(int frequency, void *user_data);
    static void CheckErr(std::string str,int err);

    radio_h radio_instance;

    int Create();

    FMRadioManager();
    ~FMRadioManager();


};

} // namespace
} // namespace extension

#endif // RADIO_RADIO_MANAGER_H_

