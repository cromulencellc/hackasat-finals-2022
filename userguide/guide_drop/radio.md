# Radios


## How do two radios talk?

There are a series of checks that occur to determine if two radios talk to each other. If the following conditions are met then bytes will flow from transmitter to receiver.

Only groundstation to/from satellite communications are considered.
 - Satellite to satellite links are ignored
 - Groundstation to groundstation links are ignored

### Are the antennas in line of sight?

Can a straight line be drawn between the two antennas without intersecting the earth

### Are the antennas pointed at each other?

Is the pointing vector of the transmit antenna within the half angle beam width
 - Half angle beam beamwidths are 10 degrees for all groundstations except the ScienceStation which is 85 degrees.
 - Satellite antennas are omnidirectional

### TX/RX settings?
For uplink, does the spacecraft radio RX settings match the groundstation TX settings?
For downlink, does the groundstation RX settings match the spacecraft radio RX settings?

- Spacecraft radio TX and RX settings cannot be set independently (TX settings always match RX)
- Groundstations can have independent TX and RX settings

## Can I optimize my radios performance?

In this game all radio settings perform equivalently. There is no need to try and *optimize* radio settings to improve downlink.

It is possible to achieve transfer rates higher than the bit rate the radio reports in the game.

## What are the ground station radio settings?

| Setting | Example Value | Description | 
| -----   |  ----------   | ----------- | 
| channel_id | 20 |  Allows 0-20 |  
| fec_repeat | 4  |  Allows 1-8 | 
| constellation | BPSK | BPSK or QPSK |
| samples per symbol | 12 | Allows 4, 8, 10, or 12 | 
| access code | 0xaaaa | any unsigned uint16  |

Channel ID is loosely used to refer to center frequency between groundstations and satelltie radios.

## What are the satellite radio settings?

| Setting | Example Value | Description | 
| -----   |  ----------   | ----------- | 
| channel | NETWORK | An enumerated value (Network=0 or Science=1) |  
| fec_repeat | 4  |  Always 4| 
| constellation | BPSK | BPSK or QPSK |
| samples per symbol | 12 | Allows 4, 8, 10, or 12 | 
| access code | 0xaaaa | any unsigned uint16  |


The network channel uses the unique channel ID assigned to your satellite. This means your satelite is limited to only 2 possible channels (NETWORK and SCIENCE) while ground stations have more channel options.

The science channel uses the science stations channel ID.

## How do I change my satellite radio settings?

There is an uplink command that will change both the transmit and receive settings of your satellite's radio. 

To use this command you need to provide a password. Sending a radio reconfigure with no password or an incorrect password will have no effect.


