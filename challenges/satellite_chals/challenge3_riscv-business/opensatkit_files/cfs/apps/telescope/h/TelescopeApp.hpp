#pragma once

#include "ASyncApp.hpp"
#include "Message.h"
#include "TelescopeMessages.hpp"
#include "Telescope.hpp"

namespace MSG = cromulence::messages; // aliasses are helpful!

namespace cromulence
{

/// \breif A CFS app that wakes up at a given rate and sends a messsage
class TelescopeApp : public cromulence::ASyncApp
{

public:
    /// Constructor
    TelescopeApp(); 
    /// Destructor 
    ~TelescopeApp();

    /// Set stuff up
    virtual void initialize();
    /// Tear stuff down
    virtual void shutdown();
    /// What to do when you have a message
    virtual void processMessage(CFE_SB_MsgId_t id, uint16_t fcnId,  CFE_SB_Msg_t *msg);
    virtual void processTimeout( );
    void handleBulkMissionRequest(const MSGS::TelescopeMultipleMissionRequest &payload);
protected:
    void processCommand( CFE_SB_Msg_t*  msg , uint16_t fcn );
    Telescope telescope_;
    // tlm 
    MSGS::TelescopeMissionMsg missionMsg_;
    MSGS::TelescopeExposureMsg exposureMsg_;
    MSGS::TelescopeHouseKeepingMsg hkMsg_;


};


}