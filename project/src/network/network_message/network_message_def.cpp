#include "network_message_def.h"

extern inline bool network_message_header_valid(const network_message_header header);
extern inline bool network_message_header_in_network_endian(const network_message_header header);

extern inline network_message_header network_message_header_create(
    const network_message_type_option type,
    const network_message_header_size size_h
);


