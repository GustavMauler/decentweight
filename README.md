# decentweight
### Control a relay with a decent espresso scale
Add a brew-by-weight to an espresso machine via a bluetooth scale.
When the new brew button is hit, use a relay to "press" the single-shot brew button on the espresso machine, and hold until the weight threshold is reached.
If we want, we can put in an offset to account for overshoot.


## Rough path:
* initialize everything and wait for the scale
* rotary to pick a stop-weight and "go" (reflect current target weight on alphanumeric)
* tare scale
* turn relay on (wired to the 1-cup brew button on an Ascaso Steel Duo)
* when scale crosses threshold (maybe with epsilon?) turn relay off and await next click of the go button.




Hardware used (lazy mode from SparkFun):
* [SparkFun RedBoard Artemis](https://www.sparkfun.com/products/15444)
* [SparkFun Qwiic Alphanumeric Display](https://www.sparkfun.com/products/18565)
* [SparkFun Qwiic Twist - RGB Rotary Encoder Breakout](https://www.sparkfun.com/products/15083)
* [SparkFun Qwiic Single Relay](https://www.sparkfun.com/products/15093)

[Decent Scale (v1.2)](https://decentespresso.com/decentscale)
[Decent's scale API](https://decentespresso.com/decentscale_api)

[Ascaso 'tech dossier' for Steel wiring diagram](https://ascaso-usa.com/pages/tech-dossier)

## Log:
> NOTE: A lot of stuff in the machine runs at line-level voltages (heaters etc.) so unplug before you open it and be careful!
Removing the top of the case is just 2 hex screws in the rear. It's attached by wires for grounding + the cup heater. There is enough length to set it aside.

### Splice on coffee switch:
I wanted this to be fully reversable. Although the crimps are.....bad (couldn't find my crimp tool), basic 1/4" flag quick disconnects fit the switches, and with some blades made for the other end of the splice, the switch works as normal.
![splice](https://github.com/GustavMauler/decentweight/assets/3944964/1d3c2da2-0ba9-4d41-a4ae-9f5fdd2effba)

### Wires from splice out rear vent
(20AWG silicone insulated fit super easily):
![wire-exit](https://github.com/GustavMauler/decentweight/assets/3944964/88afc369-c6cc-43e2-8b25-43891f3c2af1)
