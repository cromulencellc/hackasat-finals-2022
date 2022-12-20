# Yay Science Solver

This solver comes in 4 parts

## Part 1: Calculate the quaternions to all the planets at a given time

The quternions to the planets are mostly constant over one day so you might be able to get away with calculating them once. However, this script can be run at any time to get the quaternions to the planets

## Part 2: Get the current mission list

On the next pass over the ground station change your radio configuration and listen to the missions available. Print them out in a handy list

## Part 3: Solve a specific mission

Given a list of planet quaternions and a mission number execute the mission. 
- Get the mission data from your satellite
- Wait until the window
- Point at the planet for the appropriate duration

## Part 4: Downlink on next pass

Given a list of mission numbers that have completed exposures trigger a downlink to the science station on the next pass