#ifndef ADCM_DF_H
#define ADCM_DF_H

#include <cstdint>
#include <sys/types.h>
#include <fstream>
#include <vector>



#define STOR_ID_CMAP  0x504D    /* 'MP' */
#define STOR_ID_EVNT  0x5645    /* 'EV' */
#define STOR_ID_CNTR  0x5443    /* 'CT' */

struct stor_packet_hdr_t {
  u_int16_t id;                 // data block ID
  u_int16_t size;               // block size, bytes
  friend std::ifstream &operator >> (std::ifstream &stream, stor_packet_hdr_t &d) {
      stream.read(reinterpret_cast<char *>(&d), sizeof(stor_packet_hdr_t));
      return stream;
  }
} __attribute__ ((packed));

struct stor_ev_hdr_t {
  u_int8_t np;                  // number of pulses following
  u_int8_t reserved1;
  u_int16_t reserved2;
  u_int32_t ts;                 // timestamp, 10 ns step
  friend std::ifstream &operator >> (std::ifstream &stream, stor_ev_hdr_t &d) {
      stream.read(reinterpret_cast<char *>(&d), sizeof(stor_ev_hdr_t));
      return stream;
  }
} __attribute__ ((packed));


struct stor_puls_t {
  u_int8_t ch;
  u_int8_t flags;
  float a;
  float t;
  float w;
  friend std::ifstream &operator >> (std::ifstream &stream, stor_puls_t &d) {
      stream.read(reinterpret_cast<char *>(&d), sizeof(stor_puls_t));
      return stream;
  }
} __attribute__ ((packed));

struct adcm_cmap_t {
  u_int32_t n;
  std::vector<u_int8_t> map;
  friend std::ifstream &operator >> (std::ifstream &stream, adcm_cmap_t &d) {
      stream.read(reinterpret_cast<char *>(&d.n), sizeof(u_int32_t));
      d.map.resize(d.n, 0);
      for (size_t i{0}; i < d.n; ++i)
      {
          stream.read(reinterpret_cast<char *>(&d.map[i]), sizeof(u_int8_t));
      }
      return stream;
  }
};

struct adcm_counters_t {
  u_int32_t n;
  double time;
  std::vector<u_int32_t> rawhits;
  friend std::ifstream &operator >> (std::ifstream &stream, adcm_counters_t &d) {
      stream.read(reinterpret_cast<char *>(&d.n), sizeof(u_int32_t));
      stream.read(reinterpret_cast<char *>(&d.time), sizeof(double));
      d.rawhits.resize(d.n, 0);
      for (size_t i{0}; i < d.n; ++i)
      {
         stream.read(reinterpret_cast<char *>(&d.rawhits[i]), sizeof(u_int32_t));
      }
      return stream;
  }
};

struct dec_det_t
{
    uint8_t index;
    float amp;
    float rt;
} __attribute__ ((packed));

struct dec_ev_t
{
    uint32_t ts;
    float tdc;
    dec_det_t a;
    dec_det_t g;
} __attribute__ ((packed));

struct dec_cnt_t
{
    double time;
    u_int32_t rawhits;
};

#endif /* ADCM_DF_H */
