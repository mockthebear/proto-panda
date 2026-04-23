#include "drawing/modelanimation/keyframeplayer.hpp"
#include "drawing/rendering/modelhandler.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"



Model* KeyframeTrack::SetResource(std::string resourceName){
    size_t dotPos = resourceName.find('.');
     if (dotPos != std::string::npos) {
        std::string name = resourceName.substr(0, dotPos);
        std::string numStr = resourceName.substr(dotPos + 1);
        int number = std::stoi(numStr);
        
        Model *md = g_models.GetModelByName(name);
        if (md == nullptr){
            Logger::Info("Failed to find resource model called %s", name.c_str());
            return nullptr;
        }
        number = number -1;
        if (number < 0){
            Logger::Info("Point group %d is invalid!", number+1);
            return nullptr;
        }
        if (!md->hasPointGroup(number)){
            Logger::Info("Model %s does not have point group %d", number+1);
            return nullptr;
        }
        obj = md;
        pointGroup = number;
        usingModel = false;
        return obj;
    } else {
        Model *md = g_models.GetModelByName(resourceName);
        if (md == nullptr){
            Logger::Info("Failed to find resource model called %s", resourceName.c_str());
            return nullptr;
        }
        obj = md;
        usingModel = true;
        return obj;
    }
}

void KeyframeTrack::Reset(){

    for (int i=0;i<KEYFRAME_TYPE_LAST;i++){
        currentFrameByType[i] = -1;
    }
    int n = 0;
    for (auto &kf : keyframes){
        if (currentFrameByType[kf.type] == -1){
            currentFrameByType[kf.type] = n;
        }   
        n++;
    }


    if (obj != nullptr){
        obj->Reset();
    }
  
}


void KeyframeTrack::AddKeyFrame(Keyframe kf){
    keyframes.emplace_back(kf);
}

void KeyframeTrack::Reserve(){
    for (int i=0;i<KEYFRAME_TYPE_LAST;i++){
        currentFrameByType.emplace_back(-1);
    }
}

void KeyframePlayer::begin_tmp(){

    KeyframeAnimation* kf = new KeyframeAnimation();

    kf->duration = 3000;



    KeyframeTrack t1;
    t1.Reserve();
    Logger::Info("get moidel");
    Model *rs = t1.SetResource("mouth");
    if (!rs){
        return;
    }
    Logger::Info("setted");
    kf->m_models.emplace_back(rs);

    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  300, Vec2f(0.0f,  -1.5f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  600, Vec2f(0.0f,  3.0f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  900, Vec2f(0.0f, -3.0f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE, 1200, Vec2f(0.0f,  3.0f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE, 1500, Vec2f(0.0f, -3.0f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE, 1800, Vec2f(0.0f,  3.0f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE, 2100, Vec2f(0.0f, -3.0f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE, 2400, Vec2f(0.0f,  3.0f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE, 2700, Vec2f(0.0f, -3.0f)));
    t1.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE, 3000, Vec2f(0.0f,  1.5f)));
    Logger::Info("ADD TRACK");
    kf->AddTrack(t1);
    Logger::Info("added");


    



    KeyframeTrack t2;
    t2.Reserve();
    t2.SetResource("mouth.1");

    t2.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  600, Vec2f(-2.0f, 0.0f)));
    t2.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  1000, Vec2f(4.0f, 0.0f)));
    t2.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  1400, Vec2f(-4.0f, 0.0f)));
    t2.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  2000, Vec2f(4.0f, 0.0f)));
    t2.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  2400, Vec2f(-4.0f, 0.0f)));
    t2.AddKeyFrame(Keyframe(KEYFRAME_TRANSLATE,  2800, Vec2f(2.0f, 0.0f)));


    kf->AddTrack(t2);

    int idx = AddKeyframeAnimation(kf);
    Logger::Info("rdy: %d", idx);
}


void KeyframeAnimation::AddTrack(KeyframeTrack &t){
    std::map<KeyframeType,int> lastDtByType;
    for (auto &track : t.keyframes){
        uint32_t aux = lastDtByType[track.type];
        lastDtByType[track.type] = track.playAt;
        track.deltaToNext =  track.playAt - aux;
    }
    m_tracks.emplace_back(t); 
}


void KeyframeAnimation::Reset(){
    m_timeNow = 0;
    m_prevDt = 0;

    for (auto &track : m_tracks){

        track.Reset();
    }

}


void KeyframePlayer::PlayAnimationId(int32_t id){
    if (id >= m_loadedAnimations.size()){
        Logger::Info("Cannot play animation id %d, no such id exists", id);
        return;
    }

    auto &obj = m_loadedAnimations[id];
    if (obj == nullptr){
        Logger::Info("Cannot play animation id %d, no such id exists", id);
        return;
    }

    obj->Reset();



    m_currentlyPlaying = id;
}

void KeyframePlayer::runModelAnim(uint32_t dt){
    auto &obj = m_loadedAnimations[m_currentlyPlaying];
    for (auto &track : obj->m_tracks){
        track.UpdateTrack(dt, obj->m_prevDt);
    }

    int res = 0;
    obj->m_prevDt +=  dt;
    if (obj->m_prevDt >= obj->duration){  
        res = obj->m_prevDt-obj->duration;
        obj->m_prevDt = 0;

        for (auto &track : obj->m_tracks){
            track.Reset();
        }
        if (res > 0){
            runModelAnim(res);
        }
    }

   
    for (auto &mod : obj->m_models){
        mod->Recalculate();
        mod->CopyToRaster();
    }    
    
    g_modelHandler.RenderScene(obj->m_models);
    Devices::Display->startWrite();
    Devices::Display->drawPixel(0 , 32, Devices::Display->color565(255,255,0));
    Devices::Display->drawPixel(31 , 32, Devices::Display->color565(255,255,0));
    Devices::Display->drawPixel(31 * (obj->m_prevDt/(float)obj->duration), 32, Devices::Display->color565(255,0,0));
    Devices::Display->endWrite();

    Devices::Display->drawLine(0,0, 63 * (obj->m_prevDt/(float)obj->duration), 0, 255);
    Devices::Display->flipDma();    
    
}
void KeyframePlayer::Update(uint32_t dt){

    if (m_currentlyPlaying < 0){
        return;
    }

    runModelAnim(dt);

   
}



void KeyframeTrack::UpdateTrack(uint32_t dt, uint32_t prevDt){
    //Yeah, start at 1. Zero is KEYFRAME_NONE
  
    for (int operationType=KEYFRAME_TRANSLATE;operationType<KEYFRAME_TYPE_LAST;operationType++){

        int frameId = currentFrameByType[operationType];
        if (frameId >= 0){
            uint32_t sumLocal = prevDt;
            uint32_t remaining = dt;

            Keyframe nextKf = keyframes[frameId];
            int maxIter = 5;
            while (remaining > 0){
                if (sumLocal+remaining > nextKf.playAt){
                    uint32_t currentStepDt = nextKf.playAt-sumLocal;
                    remaining -= currentStepDt;
                    if (remaining > nextKf.deltaToNext){

                        int diff = remaining - nextKf.deltaToNext;
                        remaining = nextKf.deltaToNext;
                        currentStepDt += diff;
                    }
                    applyTransformations(currentStepDt, nextKf, true);
                    //interpolations[operationName](track.r, currentStepDt, nextKf, true)
                    sumLocal += currentStepDt;
                    bool found = false;
                    for (int i=frameId+1;i < keyframes.size();i++){  
                        nextKf = keyframes[i];
                        if (nextKf.type == operationType){  
                            frameId = i;
                            currentFrameByType[operationType] = i;
                            found = true;
                            break;
                        }
                    }
                    if (!found){
                        currentFrameByType[operationType] = -1;
                        break;
                    }
                }else{
                    //Logger::Info("[%d / %d] MINI STEP %d with step %d   AT %d  (sum %d)", maxIter, dt, frameId, remaining, nextKf.playAt, sumLocal);
                    applyTransformations(remaining, nextKf, false);
                    //interpolations[operationName](track.r, remaining, nextKf, false)
                    sumLocal += remaining;
                    remaining = 0;
                }
            }
            maxIter--;
            if (maxIter < 0){
                break;
            }
        }
    }
}

void KeyframeTrack::applyTransformations(uint32_t remaining, Keyframe &nextKf, bool instant){
    switch (nextKf.type){
    case KEYFRAME_TRANSLATE:{
        if (nextKf.deltaToNext == 0){
            if (usingModel){  
                obj->Translate(nextKf.value);
            }else{
                obj->TranslatePoints(pointGroup, nextKf.value);
            }
        }else{
            
            double delta = (double)remaining/(double)nextKf.deltaToNext;
            double px = delta * nextKf.value.x;
            double py = delta * nextKf.value.y;
            if (usingModel){  
                obj->Translate(Vec2f(px, py));
            }else{
                obj->TranslatePoints(pointGroup, Vec2f(px, py));
            }
        }
        break;
    }
    
    default:
        break;
    }
}