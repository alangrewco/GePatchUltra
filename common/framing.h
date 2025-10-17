#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Length-prefixed framing over any bytestream.
// Frame = [u32 N in network byte order][N bytes payload].
int send_frame(int fd, const void* payload, uint32_t len);
int recv_frame_alloc(int fd, uint8_t** out_buf, uint32_t* out_len); // mallocs *out_buf

#ifdef __cplusplus
}
#endif
