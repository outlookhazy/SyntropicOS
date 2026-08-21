

# File syn\_protobuf.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_protobuf.h**](syn__protobuf_8h.md)

[Go to the documentation of this file](syn__protobuf_8h.md)


```C++

#ifndef SYN_PROTOBUF_H
#define SYN_PROTOBUF_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_PB_WIRE_VARINT 0U           
#define SYN_PB_WIRE_FIXED64 1U          
#define SYN_PB_WIRE_LENGTH_DELIMITED 2U 
#define SYN_PB_WIRE_FIXED32 5U          
typedef struct {
    uint8_t *buf;    
    size_t capacity; 
    size_t offset;   
} SYN_PB_Encoder;

typedef struct {
    const uint8_t *buf; 
    size_t size;        
    size_t offset;      
} SYN_PB_Decoder;

typedef struct {
    uint32_t field_number; 
    uint8_t wire_type;     
} SYN_PB_Tag;

SYN_Status syn_pb_encoder_init(SYN_PB_Encoder *enc, uint8_t *buf, size_t capacity);

SYN_Status syn_pb_encode_tag(SYN_PB_Encoder *enc, uint32_t field_number, uint8_t wire_type);

SYN_Status syn_pb_encode_varint(SYN_PB_Encoder *enc, uint64_t value);

SYN_Status syn_pb_encode_int32(SYN_PB_Encoder *enc, uint32_t field_number, int32_t value);

SYN_Status syn_pb_encode_uint32(SYN_PB_Encoder *enc, uint32_t field_number, uint32_t value);

SYN_Status syn_pb_encode_bool(SYN_PB_Encoder *enc, uint32_t field_number, bool value);

SYN_Status syn_pb_encode_fixed32(SYN_PB_Encoder *enc, uint32_t field_number, uint32_t value);

SYN_Status syn_pb_encode_string(SYN_PB_Encoder *enc, uint32_t field_number, const char *str);

SYN_Status syn_pb_encode_bytes(SYN_PB_Encoder *enc, uint32_t field_number, const uint8_t *bytes,
                               size_t len);

SYN_Status syn_pb_decoder_init(SYN_PB_Decoder *dec, const uint8_t *buf, size_t size);

SYN_Status syn_pb_decode_tag(SYN_PB_Decoder *dec, SYN_PB_Tag *tag);

SYN_Status syn_pb_decode_varint(SYN_PB_Decoder *dec, uint64_t *value);

SYN_Status syn_pb_decode_fixed32(SYN_PB_Decoder *dec, uint32_t *value);

SYN_Status syn_pb_decode_bytes(SYN_PB_Decoder *dec, const uint8_t **bytes, size_t *len);

SYN_Status syn_pb_skip_field(SYN_PB_Decoder *dec, uint8_t wire_type);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PROTOBUF_H */
```


