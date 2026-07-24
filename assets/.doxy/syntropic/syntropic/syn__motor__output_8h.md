

# File syn\_motor\_output.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_motor\_output.h**](syn__motor__output_8h.md)

[Go to the source code of this file](syn__motor__output_8h_source.md)

_Abstract motor output interface._ [More...](#detailed-description)

* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_MotorOutput**](structSYN__MotorOutput.md) <br>_Abstract motor output interface (vtable)._  |


















































## Detailed Description


Decouples the motor controller from specific motor drivers. Any motor (DC, stepper, brushless, etc.) implements this three-function interface and can be driven by syn\_motor\_ctrl.




**
**


```C++
// DC motor — use the factory:
SYN_MotorOutput out = syn_dc_motor_output(&my_dc);

// Custom motor — wire directly:
SYN_MotorOutput out = {
    .set_output = my_motor_set_output,
    .coast      = my_motor_coast,
    .brake      = my_motor_brake,
    .ctx        = &my_motor,
};

// Pass to motor controller:
cfg.motor = out;
```
 





    

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_output.h`

