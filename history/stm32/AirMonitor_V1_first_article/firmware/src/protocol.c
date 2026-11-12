#include "protocol.h"
static uint16_t be(const uint8_t *p) { return (uint16_t)((p[0]<<8)|p[1]); }
/* Manufacturer describes one's-complement magnitude, not two's complement. */
static int16_t signed_sensor(uint16_t v) { return v&0x8000 ? -(int16_t)(0xffff-v):(int16_t)v; }
bool voc_parse(const uint8_t *p,unsigned n,VocReading *o) {
    if(n!=12 || p[0]!=0x2c) return false;
    uint8_t sum=0; for(unsigned i=0;i<12;i++) sum=(uint8_t)(sum+p[i]);
    if(sum) return false;
    VocReading v={be(p+1),be(p+3),be(p+5),signed_sensor(be(p+7)),signed_sensor(be(p+9))};
    if(v.tvoc>5000 || v.temperature < -200 || v.temperature >1250 || v.rh<0 || v.rh>1000) return false;
    *o=v; return true;
}
bool pm_parse(const uint8_t *p,unsigned n,PmReading *o) {
    if(n!=32 || p[0]!=0x42 || p[1]!=0x4d || be(p+2)!=28) return false;
    uint16_t sum=0; for(unsigned i=0;i<30;i++) sum=(uint16_t)(sum+p[i]);
    if(sum!=be(p+30) || p[29]) return false;
    /* Atmospheric fields, not CF=1 calibration fields. Exact M revision pending. */
    PmReading v={be(p+10),be(p+12),be(p+14),p[28],p[29]}; *o=v; return true;
}
static void shift(PacketStream *s) { for(unsigned i=1;i<s->used;i++)s->data[i-1]=s->data[i]; s->used--; }
bool voc_feed(PacketStream *s,uint8_t b,VocReading *o) {
    s->data[s->used++]=b;
    while(s->used && s->data[0]!=0x2c)shift(s);
    if(s->used<12)return false;
    if(voc_parse(s->data,12,o)){s->used=0;return true;}
    s->rejected++;shift(s);return false;
}
bool pm_feed(PacketStream *s,uint8_t b,PmReading *o) {
    s->data[s->used++]=b;
    while(s->used && (s->data[0]!=0x42 || (s->used>=2 && s->data[1]!=0x4d)))shift(s);
    if(s->used<32)return false;
    if(pm_parse(s->data,32,o)){s->used=0;return true;}
    s->rejected++;shift(s);return false;
}
