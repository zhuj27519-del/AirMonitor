#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdint.h>
#include <stdbool.h>
typedef struct { uint16_t tvoc,hcho,eco2; int16_t temperature,rh; } VocReading;
typedef struct { uint16_t pm1,pm25,pm10; uint8_t version,error; } PmReading;
typedef struct { uint8_t data[32], used; uint32_t rejected; } PacketStream;
bool voc_parse(const uint8_t *p, unsigned n, VocReading *out);
bool pm_parse(const uint8_t *p, unsigned n, PmReading *out);
bool voc_feed(PacketStream *s, uint8_t b, VocReading *out);
bool pm_feed(PacketStream *s, uint8_t b, PmReading *out);
#endif
