/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "systeminfo/systeminfo_sim_details_manager.h"

#include <vconf.h>

#include "common/logger.h"

namespace extension {
namespace systeminfo {

namespace {
//Sim
const char* kSimStatusAbsent = "ABSENT";
const char* kSimStatusInitializing = "INITIALIZING";
const char* kSimStatusReady = "READY";
const char* kSimStatusPinRequired = "PIN_REQUIRED";
const char* kSimStatusPukRequired = "PUK_REQUIRED";
const char* kSimStatusSimLocked = "SIM_LOCKED";
const char* kSimStatusNetworkLocked = "NETWORK_LOCKED";
const char* kSimStatusUnknown = "UNKNOWN";

void SimCphsValueCallback(TapiHandle */*handle*/, int result, void *data, void *user_data) {
  LoggerD("Entered");
  SimDetailsManager* sim_mgr = static_cast<SimDetailsManager*>(user_data);
  TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
  TelSimCphsNetName_t *cphs_info = static_cast<TelSimCphsNetName_t*>(data);

  std::string result_operator;
  if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
    std::stringstream s;
    s << cphs_info->full_name;
    if (s.str().empty()) {
      s << cphs_info->short_name;
    }
    result_operator = s.str();
  } else {
    LoggerW("Failed to retrieve cphs_info: %d", access_rt);
  }
  sim_mgr->set_operator_name(result_operator);
  sim_mgr->TryReturn();
}

void SimMsisdnValueCallback(TapiHandle */*handle*/, int result, void *data, void *user_data) {
  LoggerD("Entered");
  SimDetailsManager* sim_mgr = static_cast<SimDetailsManager*>(user_data);
  TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
  TelSimMsisdnList_t *msisdn_info = static_cast<TelSimMsisdnList_t*>(data);

  std::string result_msisdn;
  if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
    if (msisdn_info->count > 0) {
      if (strlen(msisdn_info->list[0].num) > 0) {
        result_msisdn = msisdn_info->list[0].num;
      } else {
        LoggerW("MSISDN number empty");
      }
    } else {
      LoggerW("msisdn_info list empty");
    }
  } else {
    LoggerW("Failed to retrieve msisdn_: %d", access_rt);
  }

  sim_mgr->set_msisdn(result_msisdn);
  sim_mgr->TryReturn();
}

void SimSpnValueCallback(TapiHandle */*handle*/, int result, void *data, void *user_data) {
  LoggerD("Entered");
  SimDetailsManager* sim_mgr = static_cast<SimDetailsManager*>(user_data);
  TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
  TelSimSpn_t *spn_info = static_cast<TelSimSpn_t*>(data);

  std::string result_spn;
  if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
    result_spn = (char *)spn_info->spn;
  } else {
    LoggerW("Failed to retrieve spn_: %d", access_rt);
  }

  sim_mgr->set_spn(result_spn);
  sim_mgr->TryReturn();
}

void SimIccidValueCallback(TapiHandle */*handle*/, int result, void *data, void *user_data) {
  LoggerD("Entered");
  SimDetailsManager* sim_mgr = static_cast<SimDetailsManager*>(user_data);
  TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
  TelSimIccIdInfo_t *iccid_info = static_cast<TelSimIccIdInfo_t*>(data);

  std::string result_iccid;
  if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
    result_iccid = (char *)iccid_info->icc_num;
  } else {
    LoggerW("Failed to retrieve iccid_: %d", access_rt);
  }

  sim_mgr->set_iccid(result_iccid);
  sim_mgr->TryReturn();
}

const unsigned short kMccDivider = 100;

} //namespace

using common::PlatformResult;
using common::ErrorCode;

SimDetailsManager::SimDetailsManager():
            mcc_(0),
            mnc_(0),
            operator_name_(""),
            msin_(""),
            state_(""),
            msisdn_(""),
            iccid_(""),
            spn_(""),
            sim_result_obj_(nullptr),
            to_process_(0)
{
}

PlatformResult SimDetailsManager::GatherSimInformation(TapiHandle* handle, picojson::object* out) {
  LoggerD("Entered");
  std::lock_guard<std::mutex> first_lock_sim(sim_info_mutex_);
  ResetSimHolder(out);

  FetchSimState(handle);
  if (kSimStatusReady == state_) {
    PlatformResult ret = FetchSimSyncProps(handle);
    if (ret.IsError()) {
      return ret;
    }
    {
      //All props should be fetched synchronously, but sync function does not work
      std::lock_guard<std::mutex> lock_to_process(sim_to_process_mutex_);
      //would be deleted on } ending bracket
      int result = tel_get_sim_cphs_netname(handle, SimCphsValueCallback, this);
      if (TAPI_API_SUCCESS == result) {
        ++to_process_;
      } else {
        LoggerE("Failed getting cphs netname: %d", result);
      }

      result = tel_get_sim_msisdn(handle, SimMsisdnValueCallback, this);
      if (TAPI_API_SUCCESS == result) {
        ++to_process_;
      } else {
        LoggerE("Failed getting msisdn: %d", result);
      }

      result = tel_get_sim_spn(handle, SimSpnValueCallback, this);
      if (TAPI_API_SUCCESS == result) {
        ++to_process_;
      } else {
        LoggerE("Failed getting spn: %d", result);
      }

      result = tel_get_sim_iccid(handle, SimIccidValueCallback, this);
      if (TAPI_API_SUCCESS == result) {
        ++to_process_;
      } else {
        LoggerE("Failed getting iccid: %d", result);
      }
    }
    //prevent returning not filled result
    std::lock_guard<std::mutex> lock_sim(sim_info_mutex_);
    //result will come from callbacks
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  //if sim state is not READY return default values and don't wait for callbacks
  TryReturn();
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SimDetailsManager::FetchBasicSimProperties(TapiHandle* tapi_handle,
                                                          unsigned short* result_mcc,
                                                          unsigned short* result_mnc,
                                                          unsigned short* result_cell_id,
                                                          unsigned short* result_lac,
                                                          bool* result_is_roaming,
                                                          bool* result_is_flight_mode,
                                                          std::string* result_imei) {
  ScopeLogger();

  int result_value = 0;
  int tapi_res = TAPI_API_SUCCESS;
  tapi_res = tel_get_property_int(tapi_handle, TAPI_PROP_NETWORK_PLMN, &result_value);
  if (TAPI_API_SUCCESS != tapi_res) {
    std::string error_msg = "Cannot get mcc value, error: " + std::to_string(tapi_res);
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }
  *result_mcc = static_cast<unsigned short>(result_value) / kMccDivider;
  *result_mnc = static_cast<unsigned short>(result_value) % kMccDivider;

  tapi_res = tel_get_property_int(tapi_handle, TAPI_PROP_NETWORK_CELLID, &result_value);
  if (TAPI_API_SUCCESS != tapi_res) {
    std::string error_msg = "Cannot get cell_id value, error: " + std::to_string(tapi_res);
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }
  *result_cell_id = static_cast<unsigned short>(result_value);

  tapi_res = tel_get_property_int(tapi_handle, TAPI_PROP_NETWORK_LAC, &result_value);
  if (TAPI_API_SUCCESS != tapi_res) {
    std::string error_msg = "Cannot get lac value, error: " + std::to_string(tapi_res);
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }
  *result_lac = static_cast<unsigned short>(result_value);

  tapi_res = tel_get_property_int(tapi_handle, TAPI_PROP_NETWORK_ROAMING_STATUS, &result_value);
  if (TAPI_API_SUCCESS != tapi_res) {
    std::string error_msg = "Cannot get is_roaming value, error: " + std::to_string(tapi_res);
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }
  *result_is_roaming = (0 != result_value) ? true : false;

  if (0 != vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &result_value)) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Cannot get is_flight_mode value");
  }
  *result_is_flight_mode = (0 != result_value) ? true : false;

  char* imei = nullptr;
  imei = tel_get_misc_me_imei_sync(tapi_handle);
  if (nullptr != imei) {
    *result_imei = imei;
    free(imei);
  } else {
    LoggerE("Failed to get imei, nullptr pointer. Setting empty value.");
    *result_imei = "";
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void SimDetailsManager::FetchSimState(TapiHandle *tapi_handle) {
  LoggerD("Entered");
  if (nullptr == tapi_handle) {
    LoggerE("Tapi handle is null");
    state_ = kSimStatusUnknown;
  } else {
    int card_changed = 0;
    TelSimCardStatus_t sim_card_state;
    int error = tel_get_sim_init_info(tapi_handle, &sim_card_state, &card_changed);
    if (TAPI_API_SUCCESS == error) {
      switch (sim_card_state) {
        case TAPI_SIM_STATUS_CARD_NOT_PRESENT:
        case TAPI_SIM_STATUS_CARD_REMOVED:
          state_ = kSimStatusAbsent;
          break;
        case TAPI_SIM_STATUS_SIM_INITIALIZING:
          state_ = kSimStatusInitializing;
          break;
        case TAPI_SIM_STATUS_SIM_INIT_COMPLETED:
          state_ = kSimStatusReady;
          break;
        case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:
          state_ = kSimStatusPinRequired;
          break;
        case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:
          state_ = kSimStatusPukRequired;
          break;
        case TAPI_SIM_STATUS_SIM_LOCK_REQUIRED:
        case TAPI_SIM_STATUS_CARD_BLOCKED:
          state_ = kSimStatusSimLocked;
          break;
        case TAPI_SIM_STATUS_SIM_NCK_REQUIRED:
        case TAPI_SIM_STATUS_SIM_NSCK_REQUIRED:
          state_ = kSimStatusNetworkLocked;
          break;
        default:
          state_ = kSimStatusUnknown;
          break;
      }
    }
  }
}

PlatformResult SimDetailsManager::FetchSimSyncProps(TapiHandle *tapi_handle) {
  LoggerD("Entered");
  TelSimImsiInfo_t imsi;
  int error = tel_get_sim_imsi(tapi_handle, &imsi);
  if (TAPI_API_SUCCESS == error) {
    LoggerD("mcc: %s, mnc: %s, msin: %s", imsi.szMcc, imsi.szMnc, imsi.szMsin);
    mcc_ = std::stoul(imsi.szMcc);
    mnc_ = std::stoul(imsi.szMnc);
    msin_ = imsi.szMsin;
  }
  else {
    return LogAndCreateResult(
              ErrorCode::UNKNOWN_ERR, "Failed to get sim imsi",
              ("Failed to get sim imsi: %d (%s)", error, get_error_message(error)));
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void SimDetailsManager::ResetSimHolder(picojson::object* out) {
  LoggerD("Entered");
  sim_result_obj_ = out;
  to_process_ = 0;
  mcc_ = 0;
  mnc_ = 0;
  operator_name_ = "";
  msin_ = "";
  state_ = "";
  msisdn_ = "";
  iccid_ = "";
  spn_ = "";
}

void SimDetailsManager::ReturnSimToJS() {
  LoggerD("Entered");
  if (nullptr != sim_result_obj_) {
    sim_result_obj_->insert(std::make_pair("state", picojson::value(state_)));
    sim_result_obj_->insert(std::make_pair("operatorName", picojson::value(operator_name_)));
    sim_result_obj_->insert(std::make_pair("msisdn", picojson::value(msisdn_)));
    sim_result_obj_->insert(std::make_pair("iccid", picojson::value(iccid_)));
    sim_result_obj_->insert(std::make_pair("mcc", picojson::value(std::to_string(mcc_))));
    sim_result_obj_->insert(std::make_pair("mnc", picojson::value(std::to_string(mnc_))));
    sim_result_obj_->insert(std::make_pair("msin", picojson::value(msin_)));
    sim_result_obj_->insert(std::make_pair("spn", picojson::value(spn_)));
    //everything returned, clear pointer
    sim_result_obj_ = nullptr;
  } else {
    LoggerE("No sim returned JSON object pointer is null");
  }
}

void SimDetailsManager::TryReturn() {
  LoggerD("Entered");
  if (0 == to_process_){
    LoggerD("Returning property to JS");
    ReturnSimToJS();
    sim_info_mutex_.unlock();
  } else {
    LoggerD("Not ready yet - waiting");
  }
}

void SimDetailsManager::set_operator_name(const std::string& name) {
  LoggerD("Entered");
  std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
  operator_name_ = name;
  --to_process_;
  LoggerD("Operator name: %s", operator_name_.c_str());
};

void SimDetailsManager::set_msisdn(const std::string& msisdn) {
  LoggerD("Entered");
  std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
  this->msisdn_ = msisdn;
  --to_process_;
  LoggerD("MSISDN number: %s", this->msisdn_.c_str());
};

void SimDetailsManager::set_spn(const std::string& spn) {
  LoggerD("Entered");
  std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
  this->spn_ = spn;
  --to_process_;
  LoggerD("SPN value: %s", this->spn_.c_str());
};

void SimDetailsManager::set_iccid(const std::string& iccid) {
  LoggerD("Entered");
  std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
  this->iccid_ = iccid;
  --to_process_;
  LoggerD("ICCID value: %s", this->iccid_.c_str());
};

} // namespace systeminfo
} // namespace webapi
