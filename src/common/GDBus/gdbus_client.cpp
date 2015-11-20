
#include "gdbuswrapper.h"
#include <iostream>
#include <string>



#define BUS_NAME    "org.tizen.system.deviced"
#define OBJECT_PATH "/Org/Tizen/System/DeviceD/Display"


int main(){

    std::cout << "-------------------------------------" << std::endl;
    std::cout << "GDBUS test client" << std::endl;
    std::cout << "-------------------------------------" << std::endl;

    GDBusWrapper *bus = new GDBusWrapper(BUS_NAME, OBJECT_PATH);

    do{

        if(bus->Connect() == false){
            std::cout << "Connection invalid";
            break;
        }

        int result;

        if(bus->CurrentBrightness(&result) == false){
            bus->error();
            std::cout << "CurrentBrightness invalid" << std::endl;
            break;
        }

        std::cout << "CurrentBrightness value: " << result << std::endl;





        if(bus->CustomBrightness(&result) == false){
            bus->error();
            std::cout << "CustomBrightness invalid" << std::endl;
            break;
        }

        std::cout << "CustomBrightness value: " << result << std::endl;

        if(bus->UnlockState("lcddim", "keeptimer", &result) == false){
            bus->error();
            std::cout << "UnlockState invalid" << std::endl;
            break;
        }

        std::cout << "UnlockState value: " << result << std::endl;






        if(bus->HoldBrightness(123, &result) == false){
            bus->error();
            std::cout << "HoldBrightness invalid" << std::endl;
            break;
        }

        std::cout << "HoldBrightness value: " << result << std::endl;





        if(bus->ReleaseBrightness(&result) == false){
            bus->error();
            std::cout << "ReleaseBrightness invalid" << std::endl;
            break;
        }

        std::cout << "ReleaseBrightness value: " << result << std::endl;




        if(bus->LockState("lcddim", "staycurstate", "NULL", 0, &result)== false){
            bus->error();
            std::cout << "LockState invalid" << std::endl;
            break;
        }

        std::cout << "LockState value: " << result << std::endl;




        std::cout << "--------TEST END -------------------------\n" << std::endl;
        return 1;

    }while(0);

    delete bus;
    return 0;

}
