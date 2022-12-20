# ADCS guide

## Estimators
The ADCS has two available estimation modes

### MEMS Only

| Measurement | Units | Notes |
| ----------- | ----- | ----- |
| Position    | m  | INVALID | 
| Velocity    | m/s  | INVALID | 
| Angular Velocity | rad/s | From inertial to body | 
| Quaternion   | unit-less | From `S frame` to body  | 


The `S frame` to which the quaternion referenced is a right handed orthonormal coordinate frame with:

| Axis | Direction |
| ---  | --------  |
| X    | $\vec{Sun}$ |
| Y    |  $ \vec{z} \times \vec{x}$ |  
| Z    |  $ \vec{x} \times \vec{Mag} $  |


Note that the `S frame` is not an inertial frame and but references the sun and the earth's magnetic field vector.

This mode is **not sufficient** for inertial pointing but is sufficient if you just need to point relative to the sun.

If the sun sensors are blocked in this mode due to eclipse, the algorithm will report the sun is aligned with the body X vector.

### All Sensors
GPS and star tracker data will be used in addition to MEMS sensor data.

| Measurement | Units | Notes |
| ----------- | ----- | ----- |
| Position    | m  | Earth Centered Inertial | 
| Velocity    | m/s  | Earth Centered Inertial | 
| Angular Velocity | rad/s | From inertial to body | 
| Quaternion   | unitless | From inertial to body  | 

## Controllers 
The ADCS has 4 control modes

### Constant Angular Velocity

This mode uses a PID controller to control the spacecraft so that it achieves a specific target angular velocity specified by the user.

Commanding to this mode will:
- Reset PID constants
- Set target angular velocity to zero
- Reset PID integrator

### Quaternion Command

This mode uses a two loop PID controller to control the quaternion of the spacecraft.

This mode performs best when the total momentum of the wheels is near zero.

Commanding to this mode will:
- Reset PID constants
- Set target quaternion to $ \vec{q} = [0,0,0] $ and $q_s = 1.0$
- Reset PID integrator
### Desaturate

This mode uses a PID controller to drive the satellite angular velocity with respect to the inertial frame to zero

This mode will attempt to minimize the magnitude of the total angular momentum vector of the reaction wheels by using magnetic torque rods to dump momentum into the earth's magnetic field

Commanding to this mode will:
- Reset PID controller integrators

### Flywheel / No control

This mode commands zero to all reaction wheels and all dipoles.

Commanding to this mode will not reset other parameters.

## Power Control

- Powering off the reaction wheels will zero out their base power draw and prevent you from drawing more power or sending commands
