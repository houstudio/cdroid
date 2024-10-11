#ifndef __NGL_SND_H__
#define __NGL_SND_H__

BEGIN_DECLS

typedef enum{
   SDT_CVBS=0,
   SDT_ANALOGUE=0,//CVBS
   SDT_SPDIF=1,
   SDT_HDMI=2
}SOUND_DEVICE_TYPE;

typedef enum{
   SDOUT_OFF=0,
   SDOUT_PCM=1,
   SDOUT_BYPASS=2
}SOUND_OUT_TYPE;

INT nglSngInt();
INT nglSndSetVolume(int idx,int vol);
INT nglSndSetMute(int idx,BOOL mute);
INT nglSndSetOutput(int ifc,int type);

END_DECLS

#endif
