target_sources(syntropic INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_actuator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_bldc_6step.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_dc_motor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_foc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_foc_encoder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_foc_observer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_gcode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_interpolator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_kinematics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_motor_ctrl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_servo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_stepper.c
)
