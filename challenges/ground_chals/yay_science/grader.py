import base64
import secrets 
import queue
import mission

class Grader:
    # Grade mission success criteria 
    def __init__( self ,  satellite ):
        
        self.gradebook = {}
        self.payloadQ = queue.Queue()
        
    def add_mission( self , missionCfg ):
        m = mission.Mission( missionCfg )
        id =  m.getId().to_bytes( 2 ,"little") 
        payload = id  + secrets.token_bytes(150) 
        id = m.getId()
        self.gradebook[id] =  { 
            "mission": m ,
             "payload": payload,
             "payloadSent":False, # The payload has been sent to the satellite
             "scored": False # the payload has been returned and scored
            }
    def getPayload( self ):
        
        return self.payloadQ.get( block=False )
    def gradePayload( self , payload ):
        for key,item in self.gradebook.items():
            if (item["payload"] == payload) and ( False == item["scored"]):
                item["scored"] = True
                return True 
        return False
    def evaluateMissions( self , currentTime, pos, quaternion):

        for key,grade in self.gradebook.items():
            complete = grade["mission"].evaluate( currentTime, pos , quaternion)
            if (True == complete):
                payload = grade["payload"]
                self.payloadQ.put( payload )
                grade["payloadSent"] = True

    def getGradeState( self ):
        return self.gradebook  