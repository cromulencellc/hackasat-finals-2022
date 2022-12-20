# Astropoint

This challenge gives players multiple missions they need to complete over the course of the game. The missions require them to point their satellite sensor at various planets for specific time periods. When they complete a given mission their sensor will generate data that they can then redeem for points.

This challenge has multiple parts.

## Science Groundstation
 A groundstation exists which players can not access. This ground station broadcasts "missions" to the players. 
 
 These missions contain the following information
 - What planet to point at
 - How long you must point at the planet
 - During what period of time you should point at the planet

This groundstation has fixed radio settings. Players can discover these settings via the groundstation transmitter settings crack challenge.

New missions are added and broadcast at a fixed rate throughout the couse of the game

## Satellite Receives Mission

If the players change their radio settings to match the groundstation they can use their satellite to receive missions.

These missions can then be downlinked to one of the players groundstations.

## Satellite executes mission

Once the players have received the current mission list from their satellite they will need to solve a quaternion to the target of interest and point their satellite at it.

Once they have pointed at the target for the specified duration the satellies sensor will generate "science data"

## Satellite transmits science data back to the science station
Once the satellite has received its science data it can change its settings back to those that match the science station.

If the science data packet is transmitted to the science station players recieve point.s

# Pre-Requisites
- Ground station pointing
- Radio config
- Radio configuration break.