# Hack-a-Sat 3 -  2022 Final #
---
Distribution Statement A: Approved for public release. Distribution is
unlimited. AFRL-2022-5934

This repository contains the open source release for the Hack-a-Sat 3 2022
final event.

Released artifacts include:

* Source code for all challenges
* Source code for all challenge solutions
* User Guide given to teams

Released artifacts *do not* include:

* Infrastructure used to host and run the game
* Source code for the score board
* Source code for custom cFS and COSMOS deployments
* Source code for for visualization
* Source code for satellite emulation

## Repository Structure ##

* [challenges](./challenges/) - Source code for the ground and satellite challenges.
  - [ground_chals](./challenges/ground_chals/) - Source code for challenges that were considered running on the groundstation
  - [satellite_chals](./challenges/satellite_chals/) - Source code for challenges considered running on the satellite
* [solvers](./solvers/) - Mission Control's solutions/solvers for the challenges.
  - [ground_chals](./solvers/ground_chals/) - Solvers for challenges that were considered running on the groundstation
    - [commanding_chals](./solvers/ground_chals/commanding_chals/) - Challenges involving commanding/steering your satellite correctly
  - [satellite_chals](./solvers/satellite_chals) - Solvers for challenges considered running on the satellite
* [tech papers](./team_writeups/) - Writeups about the game written by the top teams

![Challenges](challenges.png)

## License ##

Files in this repository are provided as-is under the MIT license unless
otherwise stated below or by a license header. See [LICENSE.md](LICENSE.md)
for more details.

cFS are provided under the NOSA v1.3 license.

COSMOS is provided under the GPLv3 license.

OSK is provided under the LGPL license.


## Contact ##

Questions, comments, or concerns can be sent to `hackasat[at]cromulence.com`.