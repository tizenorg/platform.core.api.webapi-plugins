#ifndef COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_TYPES_H_
#define COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_TYPES_H_

#include <functional>
#include <vector>
#include <memory>
#include "common/filesystem/filesystem_storage.h"

namespace common {

typedef std::function<
    void(common::Storage, common::StorageState,
         common::StorageState)> DeviceChangeStateFun;
typedef std::vector<common::Storage> Storages;
typedef std::vector<common::VirtualRoot> VirtualRoots;
typedef std::vector<std::shared_ptr<common::VirtualStorage> > VirtualStorages;

}  // namespace common

#endif  // COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_TYPES_H_