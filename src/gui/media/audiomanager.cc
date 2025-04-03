#include <cstring>
#include <fstream>
#include <map>
#include <core/context.h>
#include <core/app.h>
#include <core/xmlpullparser.h>
#include <media/soundpool.h>
#include <media/audiomanager.h>
#include <view/soundeffectconstants.h>
#include <porting/cdlog.h>
#if ENABLE(AUDIO)
#include <rtaudio/RtAudio.h>
#endif
namespace cdroid{

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
    mSoundPool = std::make_unique<SoundPool>((int)NUM_SOUND_EFFECTS,0,0);
    SOUND_EFFECT_FILES_MAP.resize((int)NUM_SOUND_EFFECTS);
    auto parser = std::make_unique<XmlPullParser>(mContext,"@xml/audio_assets");
    if(!(*parser)) parser = std::make_unique<XmlPullParser>(mContext,"@cdroid:xml/audio_assets");
    int type;
    std::unordered_map<std::string,std::string> sounds;
    const AttributeSet& attrs =(*parser);
    while(((type= parser->next())!=XmlPullParser::END_DOCUMENT)){
        if(type!=XmlPullParser::START_TAG)continue;
        std::string tagName = parser->getName();
        if(tagName.compare("asset")==0){
            const std::string id = attrs.getString("id");
            const std::string file = attrs.getString("file");
            sounds.emplace(id,file);
            LOGD("%s:%s",id.c_str(),file.c_str());;
        }
    }
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
        EFF(FX_KEYPRESS_INVALID),
        EFF(FX_BACK),
        EFF(FX_HOME)
    };
    std::unordered_map<std::string,int>file2sid;
    for(auto m:mm){
        int sid = -1;
        const int fx = m.first;
        const auto its = sounds.find(m.second);
        const std::string sf = its!=sounds.end()?its->second:"";
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
