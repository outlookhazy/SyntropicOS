

# File syn\_usb\_midi.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_midi.h**](syn__usb__midi_8h.md)

[Go to the documentation of this file](syn__usb__midi_8h.md)


```C++

#ifndef SYN_USB_MIDI_H
#define SYN_USB_MIDI_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_USB_MIDI_MAX_PACKET_SIZE 64U 
#define SYN_USB_MIDI_CIN_MISC 0x0U             
#define SYN_USB_MIDI_CIN_CABLE_EVENT 0x1U      
#define SYN_USB_MIDI_CIN_SYSTEM_COMMON_2 0x2U  
#define SYN_USB_MIDI_CIN_SYSTEM_COMMON_3 0x3U  
#define SYN_USB_MIDI_CIN_SYSEX_START 0x4U      
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_1 0x5U     
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_2 0x6U     
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_3 0x7U     
#define SYN_USB_MIDI_CIN_NOTE_OFF 0x8U         
#define SYN_USB_MIDI_CIN_NOTE_ON 0x9U          
#define SYN_USB_MIDI_CIN_POLY_KEYPRESS 0xAU    
#define SYN_USB_MIDI_CIN_CONTROL_CHANGE 0xBU   
#define SYN_USB_MIDI_CIN_PROGRAM_CHANGE 0xCU   
#define SYN_USB_MIDI_CIN_CHANNEL_PRESSURE 0xDU 
#define SYN_USB_MIDI_CIN_PITCH_BEND 0xEU       
#define SYN_USB_MIDI_CIN_SINGLE_BYTE 0xFU      
typedef struct {
    uint8_t header; 
    uint8_t midi0;  
    uint8_t midi1;  
    uint8_t midi2;  
} SYN_USB_MIDI_Packet;

typedef struct {
    uint8_t ep_in;                                
    uint8_t ep_out;                               
    uint8_t iface_num;                            
    uint8_t cable_num;                            
    uint8_t tx_buf[SYN_USB_MIDI_MAX_PACKET_SIZE]; 
    uint16_t tx_len;                              
    uint8_t rx_buf[SYN_USB_MIDI_MAX_PACKET_SIZE]; 
    uint16_t rx_len;                              
} SYN_USB_MIDI;

SYN_Status syn_usb_midi_init(SYN_USB_MIDI *midi);

SYN_Status syn_usb_midi_register(SYN_USB_Device *dev, SYN_USB_MIDI *midi);

SYN_Status syn_usb_midi_send_note_on(SYN_USB_MIDI *midi, uint8_t channel, uint8_t note,
                                     uint8_t velocity);

SYN_Status syn_usb_midi_send_note_off(SYN_USB_MIDI *midi, uint8_t channel, uint8_t note,
                                      uint8_t velocity);

SYN_Status syn_usb_midi_send_cc(SYN_USB_MIDI *midi, uint8_t channel, uint8_t controller,
                                uint8_t value);

SYN_Status syn_usb_midi_send_pitch_bend(SYN_USB_MIDI *midi, uint8_t channel, int16_t value);

SYN_Status syn_usb_midi_parse_packet(const uint8_t raw_bytes[4], SYN_USB_MIDI_Packet *packet);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_MIDI_H */
```


