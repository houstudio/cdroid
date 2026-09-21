/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <cstring>
#include <fstream>
#include <map>
#include <core/context.h>
#include <core/app.h>
#include <core/xmlpullparser.h>
#include <media/soundpool.h>
#include <media/audiomanager.h>
#include <view/soundeffectconstants.h>
#include <widget/internal_R.h>
#include <porting/cdlog.h>

namespace cdroid{

AudioManager&AudioManager::getInstance(){
    static AudioManager mInst(&App::getInstance());
    return mInst;
}

AudioManager::AudioManager(Context*ctx):mContext(ctx){
    loadSoundEffects();
}

void AudioManager::loadSoundEffects(){
    mSoundPool = std::make_unique<SoundPool>((int)NUM_SOUND_EFFECTS,0,0);
    // Per-fx file table (indexed by effectType, like SOUND_EFFECT_FILES_MAP):
    // several FX share one file, so a push_back-per-new-file list left
    // playSoundEffect's SOUND_EFFECT_FILES[effectType] log read out of range.
    SOUND_EFFECT_FILES.resize((int)NUM_SOUND_EFFECTS);
    SOUND_EFFECT_FILES_MAP.resize((int)NUM_SOUND_EFFECTS);
    // AOSP AudioService reads android.R.xml.audio_assets from the framework
    // package; an app may ship its own override (checked first, matching the
    // old string-ref fallback order).
    Resources& res = mContext->getResources();
    int audioAssetsRes = res.getIdentifier("audio_assets", "xml", mContext->getPackageName());
    if(audioAssetsRes == 0) audioAssetsRes = cdroid::internal::R::xml::audio_assets;
    auto parser = res.getXml(audioAssetsRes);
    int type;
    std::unordered_map<std::string,std::string> sounds;
    const AttributeSet& attrs =(*parser);
    while(((type= parser->next())!=XmlPullParser::END_DOCUMENT)){
        if(type!=XmlPullParser::START_TAG)continue;
        std::string tagName = parser->getName();
        if(tagName.compare("asset")==0){
            const std::string id = attrs.getAttributeValue(std::string(), "id");
            const std::string file = attrs.getAttributeValue(std::string(), "file");
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
        const int fx = m.first;
        const auto its = sounds.find(m.second);
        const std::string sf = its!=sounds.end()?its->second:"";
        /*AOSP AudioService loads each file once (keyed by filename). Resolve
          the int-id way (post binary-AXML): the app's own res/raw override
          first, then the framework pak's raw; anything else falls through as
          a plain path — AOSP's UI sounds live in /system/media/audio/ui, not
          in framework res, and SoundPool.load(path) probes it verbatim.
          load() returns 0 on failure (AOSP semantics): unmapped effects stay
          0 and playSoundEffect() is a logged no-op, exactly like a device
          that ships no UI sound files.*/
        int sid = 0;
        auto cached = file2sid.find(sf);
        if(cached!=file2sid.end()){
            sid = cached->second;
        }else{
            const size_t dot = sf.rfind('.');
            const std::string base = (dot==std::string::npos)?sf:sf.substr(0,dot);
            Resources& res = mContext->getResources();
            int resId = res.getIdentifier(base,"raw",mContext->getPackageName());
            if(resId==0) resId = res.getIdentifier(base,"raw","cdroid");
            sid = resId ? mSoundPool->load(mContext,resId,0)
                        : mSoundPool->load(sf,0);
            file2sid.emplace(sf,sid);
        }
        SOUND_EFFECT_FILES[fx] = sf;
        SOUND_EFFECT_FILES_MAP[fx] = sid;
        LOGD("%d %s->%s soundid=%d",fx,m.second.c_str(),sf.c_str(),sid);
    }
}

void AudioManager::loadTouchSoundAssetDefaults(){
    SOUND_EFFECT_FILES.resize((int)NUM_SOUND_EFFECTS,"Effect_Tick.wav");
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
