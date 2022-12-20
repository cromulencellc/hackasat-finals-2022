#include "Telescope.hpp"
#include <cstring>
#include "secrets.hpp"
namespace cromulence
{



Telescope::Telescope() : 
exposureNumber_(0)
{
    awardTo_ = cromulence::secrets::readSecretToStr( "/opt/fsw/cpu1/cf/SCIENCE_AWARD.txt");
    std::strcpy( funMessage , "Yay science!");
}

Telescope::~Telescope()
{

}


void Telescope::reset()
{
    missionStore_.clear();
    exposureStore_.clear();
}

void Telescope::storeExposure(  const MSGS::TelescopeRawExposure &exposure)
{
    uint16_t exposureNumber = exposure.mission_id;
    exposureStore_.insert_or_assign( exposureNumber , exposure);
}

bool Telescope::getExposure( const MSGS::TelescopeExposureRequest &request , MSGS::TelescopeExposure &exposure )
{
    //
    try
    {
        
        MSGS::TelescopeRawExposure raw;
        raw = exposureStore_.at( request.exposure_number );
        exposure.exposure_number = raw.mission_id;
        exposure.data = raw;
        
        std::memcpy( exposure.submittedBy,  awardTo_.c_str(), MSGS::MAX_NAME_SIZE );
        std::memcpy( exposure.fun_message,  funMessage, MSGS::MAX_FUN_SIZE );
        return true;
    }
    catch(const std::exception& e)
    {
        return false;
    }
}
void Telescope::storeMission( const MSGS::TelescopeMissionData &in )
{
    missionStore_.insert_or_assign( in.mission_id , in);
}

bool Telescope::getMission( const MSGS::TelescopeMissionRequest &request , MSGS::TelescopeMissionData &mission  )
{
    try
    {
        mission = missionStore_.at( request.mission_number );        
    }
    catch(const std::exception& e)
    {
        return false;
    }
    
    
    return true;
    
}
void Telescope::getHouseKeeping( MSGS::TelescopeHouseKeeping &out  )
{
    memset( &out.mission_ids ,std::numeric_limits< uint16_t >::max() ,MSGS::TELESCOPE_HK_ITEMS* sizeof(uint16_t));
    memset( &out.exposure_ids ,std::numeric_limits< uint16_t >::max(), MSGS::TELESCOPE_HK_ITEMS* sizeof(uint16_t));
    size_t nMission = 0;
    size_t nExposure = 0;
    for (auto const& [key, val] : missionStore_)
    {
        out.mission_ids[nMission] = key;
        nMission++;
    }
    for (auto const& [key, val] : exposureStore_)
    {
        out.exposure_ids[nExposure] = key;
        nExposure++;
    }
}



}