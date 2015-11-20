#ifndef GDBUSWRAPPER_H
#define GDBUSWRAPPER_H


#include <iostream>
#include <string>
#include <memory>
#include "auto_gen_interface.h"


class GDBusWrapperError{
public:
    GDBusWrapperError();
    void reportError(GError *error_);
    void reportError(std::string msg);
    void error(void);

private:
    std::string header_;
    std::string msg_;
    int code_;
    void _initError(void);
};


class GDBusWrapper
{
public:    
    GDBusWrapper();

    GDBusWrapper(const std::string bus_name,
               const std::string object_path,
               GBusType bus_type = G_BUS_TYPE_SYSTEM,
               GDBusProxyFlags bus_proxy_flags= G_DBUS_PROXY_FLAGS_NONE);

    ~GDBusWrapper();

    static const std::string DEFAULT_BUS_NAME;
    static const std::string DEFAULT_OBJECT_PATH;

    bool Connect();
    bool CurrentBrightness(int *result);
    bool CustomBrightness(int *result);
    bool HoldBrightness(const int brightness, int *result);
    bool LockState(const std::string state,
                   const std::string option1,
                   const std::string option2,
                   const int timeout,
                   int *result);
    bool ReleaseBrightness(int *result);
    bool UnlockState(const std::string state,
                     const std::string option,
                     int *result);
    void error(void);

private:

    GBusType bus_type_;
    GDBusProxyFlags bus_proxy_flags_;

    std::string bus_name_;
    std::string object_path_;    
    _auto_genOrgTizenSystemDevicedDisplay *proxy_;    
    std::unique_ptr<GDBusWrapperError> err_;

    bool is_connected();
    bool isValidHoldBrightness(const int brightness);
    bool isValidTimeout(const int timeout);

};

#endif // GDBUSWRAPPER_H
