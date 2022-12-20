#include "TelescopeApp.hpp"




extern "C" void Telescope_AppMain(void);


void Telescope_AppMain(void) {
   //
   cromulence::TelescopeApp obj;
   OS_TaskDelay( 1000 );
   // start the main loop
   obj.execute();

}