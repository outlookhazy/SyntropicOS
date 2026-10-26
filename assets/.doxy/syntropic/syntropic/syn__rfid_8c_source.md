

# File syn\_rfid.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_rfid.c**](syn__rfid_8c.md)

[Go to the documentation of this file](syn__rfid_8c.md)


```C++

#include "syn_rfid.h"

#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_rfid_init(SYN_RFID *rfid, SYN_GPIO_Pin ss_pin, SYN_GPIO_Pin rst_pin,
                         SYN_RFIDType type)
{
    SYN_ASSERT(rfid != NULL);

    memset(rfid, 0, sizeof(*rfid));
    rfid->type = type;
    rfid->ss_pin = ss_pin;
    rfid->rst_pin = rst_pin;

    return SYN_OK;
}

void syn_rfid_feed_card(SYN_RFID *rfid, const uint8_t *uid, uint8_t len)
{
    if (rfid == NULL || uid == NULL || len == 0)
        return;

    if (len > sizeof(rfid->uid))
        len = sizeof(rfid->uid);

    memcpy(rfid->uid, uid, len);
    rfid->uid_len = len;
    rfid->card_present = true;
}

void syn_rfid_clear_card(SYN_RFID *rfid)
{
    if (rfid == NULL)
        return;
    rfid->card_present = false;
    rfid->uid_len = 0;
}

bool syn_rfid_is_card_present(const SYN_RFID *rfid)
{
    if (rfid == NULL)
        return false;
    return rfid->card_present;
}

const uint8_t *syn_rfid_get_uid(const SYN_RFID *rfid, uint8_t *len)
{
    if (rfid == NULL || !rfid->card_present) {
        if (len)
            *len = 0;
        return NULL;
    }
    if (len)
        *len = rfid->uid_len;
    return rfid->uid;
}
```


