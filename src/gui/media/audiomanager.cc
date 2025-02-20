#include <cstring>
#include <fstream>
#include <expat.h>
#include <map>
#include <core/context.h>
#include <core/app.h>
#include <media/soundpool.h>
#include <media/audiomanager.h>
#include <view/soundeffectconstants.h>
#include <porting/cdlog.h>
#if ENABLE(AUDIO)
#include <rtaudio/RtAudio.h>
#endif
namespace cdroid{

using SoundEffect  = std::unordered_map<std::string,std::string>;
using SoundEffects = std::unordered_map<std::string,std::shared_ptr<SoundEffect>>;

typedef struct{
    SoundEffects effects;
    SoundEffects::iterator it;
}SoundEffectData;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
   SoundEffectData*data=(SoundEffectData*)userData;
   SoundEffects&ss=data->effects;
   AttributeSet atts(nullptr,"");
   atts.set(satts);
   if(strcmp(name,"group")==0){
       const std::string groupName = atts.getString("name");
       LOGD("groupname=%s",groupName.c_str());
       data->it = ss.emplace(groupName,std::make_shared<SoundEffect>()).first;
   }else if(strcmp(name,"asset")==0){
       const std::string groupName = data->it->first;
       const std::string id = atts.getString("id");
       const std::string file = atts.getString("file");
       data->it->second->emplace(id,file);
       LOGD("%s %s:%s",groupName.c_str(),id.c_str(),file.c_str());
   }
}

static std::unique_ptr<AudioManager>mInst;
AudioManager&AudioManager::getInstance(){
    if(mInst==nullptr){
        mInst = std::unique_ptr<AudioManager>(new AudioManager(&App::getInstance()));
    }
    return *mInst;
}

AudioManager::AudioManager(Context*ctx):mContext(ctx){
    loadSoundEffects();
}

void AudioManager::loadSoundEffects(){
    char buf[256];
    std::streamsize len = 0;

    std::unique_ptr<std::istream> stream = mContext->getInputStream("@xml/audio_assets");
    if((stream==nullptr)||(!*stream)){
        stream = mContext->getInputStream("@cdroid:xml/audio_assets");
    }
    if((stream==nullptr)||(!*stream)){
        LOGW("@xml/audio_assets and @cdroid:xml/audio_assets not exist!");
        return;
    }

    XML_Parser parser = XML_ParserCreate(nullptr);
    mSoundPool = std::make_unique<SoundPool>((int)NUM_SOUND_EFFECTS,0,0);
    SoundEffectData sounds;
    XML_SetUserData(parser,&sounds);
    XML_SetElementHandler(parser, startElement, nullptr);
    SOUND_EFFECT_FILES_MAP.resize((int)NUM_SOUND_EFFECTS);
    do {
        stream->read(buf,sizeof(buf));
        len = stream->gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es = XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("audio_assets.xml:%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return ;
        }
    } while(len!=0);
    XML_ParserFree(parser);

    SoundEffects* ss = &sounds.effects;
#define EFF(A){(int)A,#A}
    std::map<int,std::string>mm={
        EFF(FX_KEY_CLICK),
        EFF(FX_FOCUS_NAVIGATION_UP),
        EFF(FX_FOCUS_NAVIGATION_DOWN),
        EFF(FX_FOCUS_NAVIGATION_LEFT),
        EFF(FX_FOCUS_NAVIGATION_RIGHT),
        EFF(FX_KEYPRESS_STANDARD),
        EFF(FX_KEYPRESS_SPACEBAR),
        EFF(FX_KEYPRESS_DELETE),
        EFF(FX_KEYPRESS_RETURN),
        EFF(FX_KEYPRESS_INVALID)
    };
    std::unordered_map<std::string,int>file2sid;
    for(auto m:mm){
        int sid = -1;
        const int fx = m.first;
        const std::string sf = ss->begin()->second->find(m.second)->second;
        auto it = std::find(SOUND_EFFECT_FILES.begin(),SOUND_EFFECT_FILES.end(),sf);
        if(it==SOUND_EFFECT_FILES.end()){
            sid = mSoundPool->load(mContext,sf,0);//SOUND_EFFECT_FILES.size();
            SOUND_EFFECT_FILES.push_back(sf);
            file2sid.insert({sf,sid});
        }else {
            auto its = file2sid.find(sf);
            sid = its->second;
        }
        SOUND_EFFECT_FILES_MAP[fx] = sid;
        LOGD("%d %s->%s soundid=%d",fx,m.second.c_str(),sf.c_str(),sid);
    }
    LOGD("%s",ss->begin()->first.c_str());
}

void AudioManager::loadTouchSoundAssetDefaults(){
    SOUND_EFFECT_FILES.push_back("Effect_Tick.wav");
    for (int i = 0; i < (int)NUM_SOUND_EFFECTS; i++) {
        SOUND_EFFECT_FILES_MAP[i] = 0;
    }
}

void  AudioManager::playSoundEffect(int effectType){
    playSoundEffect(effectType,1.f);
}

void AudioManager::unloadSoundEffects(){
    mSoundPool = nullptr;
}

void  AudioManager::playSoundEffect(int effectType,int userId){
    playSoundEffect(effectType,1.f);
}

void  AudioManager::playSoundEffect(int effectType, float volume){
    if ((effectType >= (int)NUM_SOUND_EFFECTS) || (effectType < 0)) {
        LOGW("AudioManager effectType value%d outof range", effectType);
        return;
    }
    std::string sndfile = SOUND_EFFECT_FILES[effectType];
    const int sid = SOUND_EFFECT_FILES_MAP[effectType];
    const int stid= mSoundPool->play(sid,volume);
    LOGD("effectType=%d soundid=%d/%d streamid=%d file=%s",effectType,sid,SOUND_EFFECT_FILES_MAP[effectType],stid,sndfile.c_str());
}

}
