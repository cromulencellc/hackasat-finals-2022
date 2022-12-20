#pragma once

#include <string>
#include "TelescopePayloads.hpp"
#include <map>
#include <limits>

namespace MSGS=  cromulence::messages;
namespace cromulence
{
    
    class Telescope
    {
    public:
        Telescope();
        ~Telescope();
        void reset();
        void storeExposure( const MSGS::TelescopeRawExposure &exposure );
        bool getExposure( const MSGS::TelescopeExposureRequest &request , MSGS::TelescopeExposure &exposure );
        void storeMission( const MSGS::TelescopeMissionData &in );
        bool getMission( const MSGS::TelescopeMissionRequest &request , MSGS::TelescopeMissionData &mission  );
        void getHouseKeeping( MSGS::TelescopeHouseKeeping &out);
    protected:
        size_t exposureNumber_;
        std::string teamName_;
        static const size_t MAX_MISSIONS = 30;

        char funMessage[ MSGS::MAX_FUN_SIZE ];
        std::string awardTo_;
        std::map< uint16_t ,  MSGS::TelescopeMissionData> missionStore_;
        std::map< uint16_t ,  MSGS::TelescopeRawExposure > exposureStore_;
    };

}