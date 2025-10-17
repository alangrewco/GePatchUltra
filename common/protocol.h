#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Very small v0 protocol for Milestone 0.
typedef enum : uint8_t {
  PKT_FRAME_BEGIN = 1,
  PKT_GE_CMD      = 2,
  PKT_FRAME_END   = 3,
  PKT_ACK         = 4
} GePktType;

// GE command packet (enough for tracing later; pc is for debug)
typedef struct {
  uint8_t  type;      // PKT_GE_CMD
  uint8_t  cmd;       // GE opcode (0..255)
  uint16_t pad;
  uint32_t data;
  uint32_t base;
  uint32_t offset;
  uint32_t list_id;
  uint32_t pc;
} GeCmdPkt;

typedef struct {
  uint8_t  type;      // BEGIN / END / ACK
  uint8_t  pad[7];
} GeTinyPkt;

#ifdef __cplusplus
}
#endif
