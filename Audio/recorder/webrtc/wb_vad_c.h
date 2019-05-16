#ifndef wb_vad_c_h
#define wb_vad_c_h

/* */
#define FRAME_LEN 256           /* Length (samples) of the input frame */
#define COMPLEN 12              /* Number of sub-bands used by VAD */

#define UNIRSHFT 7              /* = log2(MAX_16/UNITY), UNITY = 256 */
#define SCALE 128               /* (UNITY*UNITY)/512 */

#define TONE_THR 0.65f          /* Threshold for tone detection */

/* constants for speech level estimation */
#define SP_EST_COUNT 80
#define SP_ACTIVITY_COUNT 25
#define LOG2_SP_ACTIVITY_COUNT 5
#define ALPHA_SP_UP (1.0f - 0.85f)
#define ALPHA_SP_DOWN (1.0f - 0.85f)

#define NOM_LEVEL 2050.0f       /* about -26 dBov */
#define SPEECH_LEVEL_INIT NOM_LEVEL
#define MIN_SPEECH_LEVEL1  (NOM_LEVEL * 0.063f) /* NOM_LEVEL -24 dB */
#define MIN_SPEECH_LEVEL2  (NOM_LEVEL * 0.2f)   /* NOM_LEVEL -14 dB */
#define MIN_SPEECH_SNR 0.125f   /* 0 dB, lowest SNR estimation */

/* Constants for background spectrum update */
#define ALPHA_UP1   (1.0f - 0.95f)      /* Normal update, upwards: */
#define ALPHA_DOWN1 (1.0f - 0.936f)     /* Normal update, downwards */
#define ALPHA_UP2   (1.0f - 0.985f)     /* Forced update, upwards */
#define ALPHA_DOWN2 (1.0f - 0.943f)     /* Forced update, downwards */
#define ALPHA3      (1.0f - 0.95f)      /* Update downwards */
#define ALPHA4      (1.0f - 0.9f)       /* For stationary estimation */
#define ALPHA5      (1.0f - 0.5f)       /* For stationary estimation */

/* Constants for VAD threshold */
#define THR_MIN  (1.6f*SCALE)   /* Minimum threshold */

#define THR_HIGH (6.0f*SCALE)   /* Highest threshold */
#define THR_LOW (1.7f*SCALE)    /* Lowest threshold */

#define NO_P1 31744.0f          /* ilog2(1), Noise level for highest threshold */
#define NO_P2 19786.0f          /* ilog2(0.1, Noise level for lowest threshold */

#define NO_SLOPE ((float)(THR_LOW - THR_HIGH)/(float)(NO_P2 - NO_P1))

#define SP_CH_MIN (-0.75f*SCALE)
#define SP_CH_MAX (0.75f*SCALE)

#define SP_P1 22527.0f          /* ilog2(NOM_LEVEL/4) */
#define SP_P2 17832.0f          /* ilog2(NOM_LEVEL*4) */

#define SP_SLOPE ((float)(SP_CH_MAX - SP_CH_MIN)/(float)(SP_P2 - SP_P1))

/* Constants for hangover length */
#define HANG_HIGH  12           /* longest hangover */
#define HANG_LOW  2             /* shortest hangover */
#define HANG_P1 THR_LOW         /* threshold for longest hangover */
#define HANG_P2 (4*SCALE)       /* threshold for shortest hangover */
#define HANG_SLOPE ((float)(HANG_LOW-HANG_HIGH)/(float)(HANG_P2-HANG_P1))

/* Constants for burst length */
#define BURST_HIGH 8            /* longest burst length */
#define BURST_LOW 3             /* shortest burst length */
#define BURST_P1 THR_HIGH       /* threshold for longest burst */
#define BURST_P2 THR_LOW        /* threshold for shortest burst */
#define BURST_SLOPE ((float)(BURST_LOW-BURST_HIGH)/(float)(BURST_P2-BURST_P1))

/* Parameters for background spectrum recovery function */
#define STAT_COUNT 20           /* threshold of stationary detection counter */

#define STAT_THR_LEVEL 184      /* Threshold level for stationarity detection */
#define STAT_THR 1000           /* Threshold for stationarity detection */

/* Limits for background noise estimate */

#define NOISE_MIN 40            /* minimum */
#define NOISE_MAX 20000         /* maximum */
#define NOISE_INIT 150          /* initial */

/* Thresholds for signal power (now calculated on 2 frames) */
#define VAD_POW_LOW 30000.0f    /* If input power is lower than this, VAD is set to 0 */
#define POW_PITCH_TONE_THR 686080.0f    /* If input power is lower, pitch */
/* detection is ignored */

/* Constants for the filter bank */
#define COEFF3   0.407806f      /* coefficient for the 3rd order filter */
#define COEFF5_1 0.670013f      /* 1st coefficient the for 5th order filter */
#define COEFF5_2 0.195007f      /* 2nd coefficient the for 5th order filter */
#define F_5TH_CNT 5             /* number of 5th order filters */
#define F_3TH_CNT 6             /* number of 3th order filters */

#endif

