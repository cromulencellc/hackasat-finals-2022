# Puzzlebox

## Solver Script
1. Send `[0x1,0x2,0x3,0x4]` in `HEX_VALUE_1` entry in Stage1 Command, all other HEX_VALUES must be empty
2. Send in Stage1 Command:
    `[0x54,0x68,0x69,0x73]` in `HEX_VALUE_1`
    `[0x5F,0x31,0x73,0x6E]` in `HEX_VALUE_2`
    `[0x74,0x5F,0x74,0x68]` in `HEX_VALUE_3`
    `[0x65,0x5F,0x61,0x4E]` in `HEX_VALUE_4`
3. Send in Stage2 Command:
    `[0x73,0x57,0x65,0x52]` in `HEX_VALUE_1`
    `[0x5f,0x63,0x48,0x33]` in `HEX_VALUE_2`
    `[0x63,0x4b,0x5f,0x37]` in `HEX_VALUE_3`
    `[0x68,0x45,0x5f,0x74]` in `HEX_VALUE_4`
4. Send in Stage3 Command, with encoding function selections:
    `[0x30,0x6b,0x33,0x6e]` in `HEX_VALUE_1`
    * Select `Encode2` (value of 1)
    `[0x5f,0x70,0x40,0x39]` in `HEX_VALUE_2`
    * Select `Encode1` (value of 0)
    `[0x65,0x5f,0x70,0x52]` in `HEX_VALUE_3`
    * Select `Encode2` (value of 1)
    `[0x6f,0x4c,0x6c,0x79]` in `HEX_VALUE_4`
    * Select `Encode3` (value of 2)
5. Send in Stage4 Command - part 1:
    * Stages 1, 2, 3 must be complete (unlocked)
    `[0x5F,0x73,0x30,0x6D]` in `HEX_VALUE_1`
    `[0x65,0x74,0x68,0x21]` in `HEX_VALUE_2`
6. Send in Stage4 Command - part 2:
    * Stages 1, 2, 3 must be locked (bad entries reset)
    * Send messages on stages 1, 2, 3 that do not have the values from above. Changing one value suffices.
    `[0x6E,0x5F,0x74,0x48]` in `HEX_VALUE_3`
    `[0x33,0x72,0x65,0x2E]` in `HEX_VALUE_4`
7. Check telemetry page for token


## Stages
### Pre-Stage 1
The process of inputting test messages is locked until users recognize that they must enter `[0x1,0x2,0x3,0x4]` into the first hex input for stage1 without any other values in the other hex inputs.

The *Status* is updated from `Awating user input` to `Setup Complete`.

### Stage 1
```
This_1snt_the_aN = 3333@@@@22225555
```
The user provided inputs, 4x4 characters, are used directly to produce an output. The encoding scheme is `1111222233334444`. The input characters are changed via hardcoded functions(add/sub) with hardcoded values. These values are provided to the encode1 function. The result of the encoding function is checked versus the expected result: `3333@@@@22225555`. If correct, then the telemetry page's Stage1 Status is updated from *Locked* to *Unlocked*.

### Stage 2
```
sWeR_cH3cK_7hE_t = 3333EEEEDDDDyyyy
```
The user provided inputs, 4x4 characters, are used directly to produce an output. The encoding scheme is `1234123412341234`. The input characters are changed via hardcoded functions(add/sub) with hardcoded values. These values are provided to the encode1 function. The output is provided to the input of the encode2 function. The result of the encoding function is checked versus the expected result: `3333EEEEDDDDyyyy`. If correct, then the telemetry page's Stage2 Status is updated from *Locked* to *Unlocked*.

### Stage 3
```
0k3n_p@9e_pRoLly = +tQs+tQs+tQs+tQs
```

The user provided inputs, 4x4 characters, are used directly to produce an output. The user also provides which encoding function to use for each of the 4x4 character inputs. TThe encoding scheme is `1234123412341234`. The input characters are changed via hardcoded functions(add/sub) with hardcoded values. These values are passed to the encoding function provided by the user. The result of the encoding function is checked versus the expected result: `+tQs+tQs+tQs+tQs`. If correct, then the telemetry page's Stage3 Status is updated from *Locked* to *Unlocked*.

### Stage 4
```
 _s0meth!n_tH3re. = R;-ejh"y>6*_\"<(
```

The user provided inputs, 4x4 characters, are used directly to produce an output. The encoding scheme is `12345671234567`. The first 8 input characters are changed via the correct inputs from the previous stages. Therefore, those stages must be completed. The values solved from this first part are used to change the last 8 input characters. However, the stages 1,2, and 3 must be in the locked condition for this second part of stage 4. Therefore the teams need to undo their results for the first 3 stages and then solve the second half. Both stages feature encoding via the 3 encoding functions called from a function lookup table. The result of the encoding is checked versus the expected result: `R;-ejh"y>6*_\"<(`. If correct, then the telemetry page's Stage4 Status is updated from *Locked* to *Unlocked*. 

### Completion

The teams must successfully unlock all 4 stages at the same time - that is after stage 4 is unlocked they must re-enter the correct values for stages 1,2, and 3. A token release function is then called and the telemetry page is updated with the token.


