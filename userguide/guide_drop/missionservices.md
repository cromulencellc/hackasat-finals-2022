# Mission Control Services

Mission control offers a set of services available to competitors to help recover a malfunctioning or unrecoverable satellite. These services can be requested by Teams from the mission control organizers. The first service mission control offers is the Spacetug. The Spacetug is a state-of-the-art satellite with a super advanced propulsion system that can dock with a Teams satellite and provides a method of recovering a satellite. At a teams request to the mission control organizers, the organizers will deploy the spacetug to a teams satellite. The second service that mission control offers is the ability to remotely reset and restore the flight software running on a teams satellite.

## Spacetug
Spacetugs even as advanced as they are, unfortunately are not instantaneous. These tugs take some time to transit and dock with a teams satellite. They can be used to perform a number of actions that also take time to complete. In certain scenarios, a Spacetug may be required by a team to recover their satellite. Some example scenarios where a Spacetug request may be useful to a team are below:

 - Battery reaches zero
 - Satellite is unrecoverable or unresponsive
 - Satellite is in an unrecoverable spin


Once a spacetug docks with a satellite the following will happen to bring the satellite back into a recoverable state.
 - Despinning of reaction wheels
 - Despinning of the spacecraft
 - Charge the battery to 40%
 - At the conclusion of these events the satellite and flight software will be reset

In cases that the satellite is unrecoverable due to an *unintentional* bug, a spacetug will be provided by the organizers at no penalty. In all other cases, the Spacetug incurs a point penalty that is spelled out in the Scoring document.

There will be a delay of up to 6 minutes before the Spacetug docking procedure begins after a request is made.

## Software Reset
In certain cases the flight software may enter an unrecoverable or unresponsive state and the onboard Watchdog Timer (WDT) does not automatically reset the satellite. In this case Teams can request mission control to command a physical reset and restore of the flight software on the satellite.

This service performs the following operations to the satellite:
 - Powering off the flight software running on the satellite
 - Resetting the flight image to original
 - Starting the flight software

There will be a delay of 6 minutes for mission control to send the software reset command to the satellite. Also there will be a further delay while the reset occurs with the teams satellite offline while the software reset and restore completes. As this process initiates a complete restart of the flight software running on the satellite all settings will return to their initial settings.
