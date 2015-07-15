#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>

#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>

#include <common/extension.h>

static std::string prefix_ = "libtizen";
static std::string postfix_ = ".so";
static std::string target_path_ = "/usr/lib/tizen-extensions-crosswalk/";
static std::vector<std::string> apinamespaces = {"tizen", "xwalk"};

typedef common::Extension *(*CreateExtensionFunc)(void);

struct module_description {
  std::string name;
  std::string lib;
  std::vector<std::string> entries;
};


static XW_Extension ext = 0;
static std::map<XW_Extension, module_description> descriptions;

#ifndef JSON_MINIFY
  #define PRINT_TAB() std::cout << "\t"
#else
  #define PRINT_TAB()
#endif 

void print_json() {
  std::cout << "[" << std::endl;
  for (const auto& kv : descriptions) {
    const module_description &desc = kv.second;

    std::string::size_type n = desc.name.find('.');
    std::string ns =
        n == std::string::npos ? desc.name : desc.name.substr(0, n);

    if (std::find(apinamespaces.begin(), apinamespaces.end(), ns) ==
        apinamespaces.end()) {
      continue;
    }

    PRINT_TAB();
    std::cout << "{" << std::endl;
    PRINT_TAB();
    PRINT_TAB();
    std::cout << "\"name\":\"" << desc.name << "\"," << std::endl;
    PRINT_TAB();
    PRINT_TAB();
    std::cout << "\"lib\":\"" << desc.lib << "\"," << std::endl;
    PRINT_TAB();
    PRINT_TAB();
    std::cout << "\"entry_points\": [";
    for (std::vector<std::string>::size_type i=0; i<desc.entries.size(); i++) {
      if (i != 0) {
        std::cout << ",";
      }
      std::cout << "\"" << desc.entries[i] << "\"";
    }
    std::cout << "]" << std::endl;
    PRINT_TAB();
    std::cout << "}";
    if (kv.first != ext) {
      std::cout << ",";
    }
    std::cout << std::endl;
  }
  std::cout << "]" << std::endl;
}

const void* get_interface(const char* name) {
  if (!strcmp(name, XW_CORE_INTERFACE_1)) {
    static const XW_CoreInterface coreInterface1 = {
      [](XW_Extension extension, const char* name) {
        module_description *desc = &descriptions[extension];
        desc->name = name;
      },
      [](XW_Extension extension, const char* api) {},
      [](XW_Extension extension, XW_CreatedInstanceCallback created,
         XW_DestroyedInstanceCallback destroyed) {},
      [](XW_Extension extension, XW_ShutdownCallback shutdown_callback) {},
      [](XW_Instance instance, void* data) {},
      [](XW_Instance instance) -> void* { return nullptr; }
      };
    return &coreInterface1;
  }

  if (!strcmp(name, XW_INTERNAL_ENTRY_POINTS_INTERFACE_1)) {
    static const XW_Internal_EntryPointsInterface entryPointsInterface1 = {
      [](XW_Extension extension, const char** entries) {
        module_description *desc = &descriptions[extension];
        for (int i=0; entries[i]; i++) {
          desc->entries.push_back(std::string(entries[i]));
        }
      }
    };
    return &entryPointsInterface1;
  }
  
  if (!strcmp(name, XW_MESSAGING_INTERFACE_1)) {
    static const XW_MessagingInterface_1 messagingInterface1 = {
      [](XW_Extension extension, XW_HandleMessageCallback handle_message) {
      },
      [](XW_Instance instance, const char* message) {
      }
    };
    return &messagingInterface1;
  }

  if (!strcmp(name, XW_INTERNAL_SYNC_MESSAGING_INTERFACE_1)) {
    static const XW_Internal_SyncMessagingInterface syncMessagingInterface1 = {
      [](XW_Extension extension, XW_HandleSyncMessageCallback handle_sync_msg) {
      },
      [](XW_Instance instance, const char* reply){
      }
    };
    return &syncMessagingInterface1;
  }

  if (!strcmp(name, XW_INTERNAL_RUNTIME_INTERFACE_1)) {
    static const XW_Internal_RuntimeInterface_1 runtimeInterface1 = {
      [](XW_Extension extension, const char* key, char* value, size_t vlen) {
      }
    };
    return &runtimeInterface1;
  }

  if (!strcmp(name, XW_INTERNAL_PERMISSIONS_INTERFACE_1)) {
    static const XW_Internal_PermissionsInterface_1 permissionsInterface1 = {
      [](XW_Extension extension, const char* api_name) -> int {
        return XW_ERROR;
      },
      [](XW_Extension extension, const char* perm_table) -> int {
        return XW_ERROR;
      }
    };
    return &permissionsInterface1;
  }

  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Need tizen crosswalk path" << std::endl;
    return -1;
  }
  std::string tce_path = argv[1];
  
  if (tce_path.empty()) {
    std::cerr << "Invalid tizen crosswalk path" << std::endl;
    return -1;
  }

  DIR * dir;
  struct dirent *ent;
  if ((dir = opendir(tce_path.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      std::string fname = ent->d_name;

      if (fname.size() >= prefix_.size() + postfix_.size() &&
          !fname.compare(0, prefix_.size(), prefix_) &&
          !fname.compare(fname.size() - postfix_.size(), postfix_.size(),
                        postfix_)) {
        std::string so_path = tce_path + "/" + fname;
        char* error;
        void *handle = dlopen(so_path.c_str(), RTLD_LAZY);
        if ((error = dlerror()) != NULL) {
          std::cerr << "cannot open " << so_path << std::endl;
          std::cerr << "Error >>" << error << std::endl;
          return -1;
        }

        XW_Initialize_Func initialize = reinterpret_cast<XW_Initialize_Func>(
                dlsym(handle, "XW_Initialize"));

        if (!initialize) {
          std::cerr << "Can not loading extension " << fname << std::endl;
        } else {
          ext++;
          descriptions[ext] = module_description();
          descriptions[ext].lib = target_path_ + fname;
          int ret = initialize(ext, get_interface);
          if (ret != XW_OK) {
            std::cerr << "Error loading extension " << fname << std::endl;
          }
        }

        // some Shared libraries have static finalizer.
        // __attribute__((destructor)) this gcc extension makes finalizer.
        // if close it, it can makes segfault.
        // True, It's shared object's problem. but we can't fix it.
        // so don't close it in only this tool. just finish process.
        //
        // dlclose(handle);
      }
    }
    closedir(dir);

    print_json();
  } else {
    std::cerr << "path not exist : " << tce_path << std::endl;
    return -1;
  }

  // it would be need for ignore loaded libraries destructor
  _exit(0);

  return 0;
}
