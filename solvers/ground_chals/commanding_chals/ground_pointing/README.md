# Ground Antenna Control 

This challenge requires players to point their ground station antenna at the satellite they want to transmit to.

Players will be provided with the ground station position and initial orbital elements for all the satellites.

They can then use keplerian orbital mechanics to calculate the satellite position and translate this to azimuth and elevation commands. 


## Ground Station Pointing Solver

This solver will send commands to Cosmos5 to keep a groundstation pointed at a satellite


## Assumptions
The satellite is in a keplerian oribt - this isnt entirely true but should suffice for pointing
The ground station isn't moving
Azimuth and elevation define an intrinsic rotation rotation sequence starting at the Up-East-North refrence frame where:
- Azimuth is a negative rotation about the x axis
- Followed by elevation, a negative rotation about the y axis 


## How to use 
```sh
./setup.sh #if you need the depenedencies
python3 main.py groundstation_name satellite_name
```