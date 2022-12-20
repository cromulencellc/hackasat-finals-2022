# Groundstations 

Ground stations must be used to talk to satellites. All ground stations have an altitude of 0 meters.

## Locations 

| Groundstation | Latitude | Longitude | Beam | Type | 
| -----| --- | --- | --- | ---|
| spacebits_svalbard | 78.23 | 15.37 | 10.0 | specificuser  |
| organizers_svalbard | 78.23 | 15.38 | 10.0 | specificuser  |
| perfectblue_svalbard | 78.23 | 15.39 | 10.0 | specificuser  |
| samurai_svalbard | 78.23 | 15.4 | 10.0 | specificuser  |
| poland_svalbard | 78.23 | 15.41 | 10.0 | specificuser  |
| singleevent_svalbard | 78.23 | 15.42 | 10.0 | specificuser  |
| solarwine_svalbard | 78.23 | 15.43 | 10.0 | specificuser  |
| weltalles_svalbard | 78.23 | 15.44 | 10.0 | specificuser  |
| spacebits_mcmurdo | -77.83 | 166.68 | 10.0 | specificuser  |
| organizers_mcmurdo | -77.83 | 166.69 | 10.0 | specificuser  |
| perfectblue_mcmurdo | -77.83 | 166.7 | 10.0 | specificuser  |
| samurai_mcmurdo | -77.83 | 166.71 | 10.0 | specificuser  |
| poland_mcmurdo | -77.83 | 166.72 | 10.0 | specificuser  |
| singleevent_mcmurdo | -77.83 | 166.73 | 10.0 | specificuser  |
| solarwine_mcmurdo | -77.83 | 166.74 | 10.0 | specificuser  |
| weltalles_mcmurdo | -77.83 | 166.75 | 10.0 | specificuser  |
| science-station | -82.4131 | 164.601 | 85.0 | specificuser  |
| LosAngeles | 34.0522 | -118.244 | 10.0 | rolling_password  |
| Guam | 13.4443 | 144.794 | 10.0 | rolling_password  |
| Mingenew | -29.1902 | 115.442 | 10.0 | rolling_password  |
| Mauritius | -20.3484 | 57.5522 | 10.0 | rolling_password  |
| Cordoba | -31.4201 | -64.1888 | 10.0 | rolling_password  |
| Melbourne | 28.0836 | -80.6081 | 10.0 | rolling_password  |
| Hawaii | 19.0136 | -155.663 | 10.0 | rolling_password  |
| Tokyo | 35.6762 | 139.65 | 10.0 | rolling_password  |
| Kathmandu | 27.7172 | 85.324 | 10.0 | rolling_password  |
| Maspalomas | 27.7606 | -15.586 | 10.0 | rolling_password  |



## Specific User 

Specific user ground stations are assigned to a specific team. This team always has access to this ground station. No other teams are allowed access.

## Rolling Password 

Rolling password ground stations are competitive and accessible by any time. However, only one team can access the ground station at a time.

### Access

To access one of the rolling password ground stations teams must provide a plain text access token with a password. If the correct access token is provided then the team that provided it is given access.

### Revocation of access

Once a team gains access they will keep access for a fixed period of time. After that time passes the team's access to the groundstation will be revoked. When a team's access is revoked the access token for the grounstation will be changed (The password will 'rollover' when access is revoked). Repeated access requests will tell you how long you have left until revokation.

### Blackout

Once a team's access is revoked from a ground station of this type they will be forbidden from re-accessing that groundstation until one of the following occurs:
- A specific amount of time passes
- Another team accesses the ground station

Requesting access to a station you are blacked out from will tell you how much time is left in the blackout.