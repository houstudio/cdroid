#ifndef __NGL_VIDEO_H__
#define __NGL_VIDEO_H__
#include <cdtypes.h>
#include <cdgraph.h>//for NGLRect definition
BEGIN_DECLS

typedef enum{
    /**
    Value to specify and invalid audio format
    */
    DECA_INVALID,

    DECA_MPEG1,

    /**
    Value to specify the  <b> ISO/IEC 13813-3 MPEG-2 </b> audio format, which
    is used for live play and media player

    @note @b 1. When playing TS packets:
                - This audio format can be set by the function
                  #aui_deca_type_set
                - The function #aui_deca_type_get can be used to check if that
                  audio format is the current one

    @note @b 2. This audio format is also supported by AV-Injector Module
                (only in Linux OS), please refer to the header file
                @b aui_av_injecter.h
    */
    DECA_MPEG2,

    /**
    Value to specify the <b> ISO/IEC 14496-3 MPEG4-AAC </b> audio format
    (i.e. Advanced Audio Coding (AAC) with LOAS (Low Overhead Audio Stream)
    sync and LATM mux), which is used for live play and media player.

    @note When playing TS packets:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if that audio
            format is the current one

    @warning If the AAC sync format (LATM/ADTS) is not set correctly,
             audio decode will probe the sync format and change to the correct
             sync format internally, but the first few seconds of audio data
             will be discarded.
    */
    DECA_AAC_LATM,

    /**
    Value to specify the <b> AC3 </b> audio format, which is used for live play
    and media player

    @note When playing TS packets:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if that audio
            format is the current one
    */
    DECA_AC3,

    /**
    Value to specify the <b> DTS </b> audio format, which is used for live play
    and media player

    @note When playing TS packets:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if that audio
            format is the current one
    */
    DECA_DTS,

    /**
    @warning  This value is deprecated
    */
    DECA_PPCM,

    /**
    @warning  This value is deprecated
    */
    DECA_LPCM_V,

    /**
    @warning  This value is deprecated
    */
    DECA_LPCM_A,

    /**
    @warning  This value is deprecated
    */
    DECA_PCM,

    /**
    Value to specify the <b> BYE1 </b> audio format, which is used for live
    play and media player

    @note When playing TS packets:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if that audio
            format is the current one
    */
    DECA_BYE1,

    /**
    Value to specify the <b> Real audio 8 </b> format, which is only used for
    media player
    */
    DECA_RA8,

    /**
    Value to specify the <b> MPEG Audio Layer 3 </b> format which is used for
    live play and media player

    @note When playing TS packets for live play:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if this audio
            format is the current one
    */
    DECA_MP3,

    /**
    Value to specify the <b> ISO/IEC 13818-7:2003 MPEG2-AAC </b> audio format
    (i.e. Advanced Audio Coding (AAC) with ADTS (Audio Data Transport Format) sync),
    which used for live play and media player.

    @warning If the AAC sync format (LATM/ADTS) is not set correctly,
             audio decode will probe the sync format and change to the correct
             sync format internally, but the first few seconds of audio data
             will be discarded.
    */
    DECA_AAC_ADTS,

    /**
    Value to specify the <b> OGG </b> audio format, which is @a only used
    for media player
    */
    DECA_OGG,

    /**
    Value to specify the <b> EC3 </b> audio format, which is used for live play
    and media player

    @note @b 1. When playing TS packets:
                - This audio format can be set by the function
                  #aui_deca_type_set
                - The function #aui_deca_type_get can be used to check if that
                  audio format is the current one

    @note @b 2. This audio format is also supported by AV-Injector Module (only
                in Linux OS), please refer to the header file @b aui_av_injecter.h
    */
    DECA_EC3,

    /**
    The scope of this value is the same as the value #AUI_DECA_STREAM_TYPE_MP3 of
    this enum
    */
    DECA_MP3_L3,

    /**
    Value to specify pure PCM data without packet header, which contains the
    below parameters:
    - How many channels are in PCM data
    - What is the sample rate of PCM data
    - What is the sample size of PCM data
    - Whether the PCM sample is big endian or not
    - Whether the PCM sample is signed or not

    @note   @b 1. This enum value is not used for live play
    @note   @b 2. User can send pure PCM data by using the function
                  #aui_deca_set with the enum value #AUI_DECA_INJECT_ES_DATA
                  as input parameter
    @note   @b 3. User needs to set the parameters which are usually in the PCM
                  header, as previously mentioned, by using the function #aui_deca_set
                  with the enum value #AUI_DECA_PCM_PARAM_SET as input parameter before
                  sending audio data to the driver
    @note   @b 4. The PCM pure data sent by user should align with 1536 bytes length
    */
    DECA_RAW_PCM,

    /**
    Value to specify the <b> BYE1 PRO </b> audio format, which is set to DECA
    Module for decoding in media player
    */
    DECA_BYE1PRO,

    /**
    Value to specify the <b> FLAC </b> audio format, which is set to DECA Module
    for decoding in media player
    */
    DECA_FLAC,

    /**
    Value to specify the <b> APE </b> audio format, which is set to DECA Module
    for decoding in media player

    @warning  This format is not supported by the DECA Module
    */
    DECA_APE,

    /**
    The scope of this value is the same as the value #AUI_DECA_STREAM_TYPE_MP3
    of this enum
    */
    DECA_MP3_2,

    /**
    Value to specify the <b> AMR </b> audio format, which is set to DECA Module
    for decoding
    */
    DECA_AMR,

    /**
    Value to specify the <b> ADPCM </b> audio format, which is set to DECA
    Module for decoding
    */
    DECA_ADPCM,

    /**
    Value to specify the total number of audio formats available
    */
    DECA_LAST
}NGL_AUDIO_TYPE;

typedef enum{
    /**
    Value to specify an invalid video format
    */
    DECV_INVALID,
    /**
    Value to specify the <b> ISO/IEC 13818-2 MPEG-2 </b> video format

    @note @b 1. When playing <b> TS Stream </b>:
                - This video format can be set by the function
                  #aui_decv_decode_format_set
                - The function #aui_decv_decode_format_get can be used to check
                  if that video format is the current one

    @note @b 2. This video format is @a also supported by <b> AV-Injector Module
                </b> (@a only in <b> Linux OS</b>), please refer to the related
                header file
    */
    DECV_MPEG,

    /**
    Value to specify the <b> ITU-T H.264 </b> or <b> ISO/IEC 14496-10/MPEG-4 AVC
    </b> video format.

    @note @b 1. When playing <b> TS Stream </b>:
                - This video format can be set by the function
                  #aui_decv_decode_format_set
                - The function #aui_decv_decode_format_get can be used to check
                  if that video format is the current one

    @note @b 2. This video format is @a also supported by <b> AV-Injector Module
                </b> (@a only in <b> Linux OS</b>), please refer to the related
                header file
    */
    DECV_AVC,

    /**
    Value to specify the <b> AVS+ </b> video format

    @note @b 1. When playing <b> TS stream </b>:
                - This video format can be set by the function
                  #aui_decv_decode_format_set
                - The function #aui_decv_decode_format_get can be used to check
                  if that video format is the current one

    @note @b 2. This video format can be used @a only in projects based on <b>
                Linux OS </b>
    */
    DECV_AVS,

    /**
    Value to specify the  <b> XVID </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    DECV_XVID,

    /**
    Value to specify the  <b> FLV1 </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    DECV_FLV1,

    /**
    Value to specify the  <b> VP8 </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    DECV_VP8,

    /**
    Value to specify the <b> WVC1 </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    DECV_WVC1,

    /**
    Value to specify the  <b> WX3 </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    DECV_WX3,

    /**
    Value to specify the  <b> RMVB </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    DECV_RMVB,

    /**
    Value to specify the <b> MJPG </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    DECV_MJPG,

    /**
    Value to specify the <b> HEVC (H.265) </b> video format
    */
    DECV_HEVC,


    /**
    Value to specify the total number of video formats available in this enum
    */
    DECV_NUM
}NGL_VIDEO_TYPE;

typedef struct{
	unsigned short int video_pid;
	unsigned short int audio_pid;
	unsigned short int pcr_pid;
	unsigned short int subtitle_pid;
	unsigned short int ttx_pid;
	unsigned short int audio_des_pid;
	NGL_AUDIO_TYPE en_audio_stream_type;
	NGL_VIDEO_TYPE en_video_stream_type;
	
}CHANNEL_PIDInfos;

INT nglAvInit();
INT nglAvPlay(int dmxid,int vid,int vtype,int aid,int atype,int pcr);
INT nglAvStop(int dmx);
INT nglAvSetVideoWindow(int dmxid,const GFXRect*inRect,const GFXRect*outRect);
INT nglSndSetVolume(int idx,int vol);
INT nglSndGetColume(int idx);
END_DECLS
#endif


