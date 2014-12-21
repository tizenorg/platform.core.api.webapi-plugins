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
#include <vector>
#include <runtime_info.h>
#include <radio.h>

#include "radio_instance.h"

namespace extension {
namespace radio {

class FMRadioManager {
public:

    static FMRadioManager* GetInstance(bool safe  = true);

    radio_h * GetRadioInstance();
    std::vector<double> * GetFreqs();

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

    bool MuteGetter();
    void MuteSetter(const picojson::value& args);
    double FrequencyGetter();
    double SignalStrengthGetter();
    bool AntennaGetter();
    char* StateGetter();

private:
    std::vector<double> freqs;
    radio_h radio_instance;

    static std::string TranslateCode(int err);
    static void RadioAntennaCB(runtime_info_key_e key, void *user_data);
    static void RadioInterruptedCB(radio_interrupted_code_e code, void *user_data);
    static common::PlatformException GetException(char * name,int err);
    static void RadioSeekCB(int frequency, void *user_data);
    static void CheckErr(std::string str,int err);
    static void ScanStartCB(int frequency, void *user_data);
    static void ScanStopCB(void *user_data);
    static void ScanCompleteCB(void *user_data);
    int Create();
    FMRadioManager();
    ~FMRadioManager();

};

} // namespace
} // namespace extension

#endif // RADIO_RADIO_MANAGER_H_

