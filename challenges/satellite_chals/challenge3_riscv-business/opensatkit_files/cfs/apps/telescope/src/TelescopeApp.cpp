#include "TelescopeApp.hpp"
#include "TelescopeMessages.hpp"

volatile char telescope_flag[32] = {0};

namespace cromulence
{

    /// Constructor sets up a 
    TelescopeApp::TelescopeApp() : ASyncApp( 10 , "Telescope" , 5000 )
    {
        
    }

    TelescopeApp::~TelescopeApp()
    {

    }

    void TelescopeApp::initialize()
    {
        CFE_EVS_SendEvent(1, CFE_EVS_INFORMATION, "Telescope App Online ");
        // subscribe to messages
        this->subscribe(MSG::TELESCOPE_CMD_ID ) ;

        this->subscribe(MSG::TelescopeRawExposureMsg::MESSAGE_ID  ) ;
        telescope_.reset();
        CFE_EVS_SendEvent(1, CFE_EVS_INFORMATION, "Telescope Init Complete ");

    }

    void TelescopeApp::shutdown()
    {
    }

    void TelescopeApp::processMessage(CFE_SB_MsgId_t id, uint16_t fcnId,  CFE_SB_Msg_t* msg )
    {

        // Unpack the message you got and form it into a hardware packet
        switch( id )
        {
            // Note: The {} in the cases are REQUIRED to avoid a 'jump to case' error
            case MSG::TELESCOPE_CMD_ID:
            {
                processCommand( msg , fcnId );
                break;
            }
            case MSG::TelescopeRawExposureMsg::MESSAGE_ID:
            {
                MSGS::TelescopeRawExposureMsg m( msg );
                telescope_.storeExposure( m.data.payload );
                break;
            }
            default:
            {
                hkMsg_.data.payload.InvalidMsgCnt++;
                CFE_EVS_SendEvent(1, CFE_EVS_CRITICAL, "Telescope got a messsage it doesnt know how to handle");
                break;
            }

        };



    }
    void TelescopeApp::processTimeout( )
    {
        telescope_.getHouseKeeping( hkMsg_.data.payload );
        hkMsg_.send();
    }

    void TelescopeApp::handleBulkMissionRequest(const MSGS::TelescopeMultipleMissionRequest &payload)
    {
        // This is how we ensure the order on the stack.
        struct {
            bool success = 0;
            size_t i;
            size_t num_missions = 0;
            uint16_t *mr = nullptr;
            MSG::TelescopeMissionRequest req;
            uint8_t missions_requested[MSGS::BULK_REQ_LIMIT] = {0};
        } locals;


        locals.num_missions = (payload.num_missions < MSGS::BULK_REQ_LIMIT) ? payload.num_missions : MSGS::BULK_REQ_LIMIT;

        locals.mr = (uint16_t*)locals.missions_requested;

        for(locals.i = 0; locals.i < locals.num_missions; locals.i++){
            *locals.mr = payload.missions_to_get[locals.i];
            locals.req.mission_number = *locals.mr;

            locals.mr++;

            locals.success = telescope_.getMission( locals.req, missionMsg_.data.payload );
            if ( true == locals.success)
            {
                missionMsg_.send();
            }
            else
            {
                CFE_EVS_SendEvent(1, CFE_EVS_CRITICAL, "Telescope App: Mission does not exist");
            }

        }

        // Add field to housekeeping 
        // Correctly copy missions requested into housekeeping
        CFE_PSP_MemCpy(hkMsg_.data.payload.last_bulk_mission_req, locals.missions_requested, sizeof(locals.missions_requested));
    }

    void TelescopeApp::processCommand( CFE_SB_Msg_t*  msg , uint16_t fcn )
    {
        switch( fcn )
        {
            case MSG::TelescopeNoOpMsg::FUNCTION_ID:
            {
                CFE_EVS_SendEvent(1, CFE_EVS_CRITICAL, "Telescope App: NO OP");
                break;
            }
            case MSG::TelescopeResetMsg::FUNCTION_ID:
            {
                CFE_EVS_SendEvent(1, CFE_EVS_CRITICAL, "Telescope App: Resetting");
                telescope_.reset();
                break;
            }
            case MSG::TelescopeExposureRequestMsg::FUNCTION_ID:
            {
                MSGS::TelescopeExposureRequestMsg mIn(msg);
                bool success;
                success = telescope_.getExposure( mIn.data.payload  , exposureMsg_.data.payload);
                if( true == success)
                {
                    exposureMsg_.send( );
                    CFE_EVS_SendEvent(1, CFE_EVS_CRITICAL, "Telescope App: Exposure sent");
                }
                else
                {
                    CFE_EVS_SendEvent(1, CFE_EVS_CRITICAL, "Telescope App: Exposure does not exist");
                }     

                break;           
            }
            case MSG::TelescopeMissionRequestMsg::FUNCTION_ID:
            {
                MSGS::TelescopeMissionRequestMsg mIn(msg);
                bool success;

                success = telescope_.getMission( mIn.data.payload , missionMsg_.data.payload );
                if ( true == success)
                {
                    missionMsg_.send();
                }
                else
                {
                    CFE_EVS_SendEvent(1, CFE_EVS_CRITICAL, "Telescope App: Mission does not exist");
                }
                break;
            }
            case MSG::TelescopeMultipleMissionRequestMsg::FUNCTION_ID:
            {
                

                MSGS::TelescopeMultipleMissionRequestMsg m(msg);

                handleBulkMissionRequest(m.data.payload);
            }

            case MSG::TelescopeSetMissionMsg::FUNCTION_ID:
            {
                MSGS::TelescopeSetMissionMsg m( msg );
                telescope_.storeMission( m.data.payload );
                break;
            }
            default:
            {
                hkMsg_.data.payload.InvalidCmdCnt++;
                CFE_EVS_SendEvent(1, CFE_EVS_CRITICAL, "Telescope got an unexpected function code");
            };
        }
    }

}