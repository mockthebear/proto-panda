#include "drawing/modelanimation/keyframeplayer.hpp"
#include "drawing/rendering/modelhandler.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"




Keyframe Keyframe::KeyFrameMaker(int mode, uint32_t at, Vec2f val, Vec2f cntr, bool igInterp, bool dynamicCenterP){
    Keyframe kf;
    kf.type = (KeyframeType)mode;
    kf.playAt = at;
    kf.value = val;
    kf.center = cntr;
    kf.ignoreInterpolation = igInterp;
    kf.dynamicCenter = dynamicCenterP;
    return kf;
}

bool KeyframeTrack::SetResource(std::string resourceName){
    size_t dotPos = resourceName.find('.');
     if (dotPos != std::string::npos) {
        std::string name = resourceName.substr(0, dotPos);
        std::string numStr = resourceName.substr(dotPos + 1);
        int number = std::stoi(numStr);
        
        Model *md = g_models.GetModelByName(name);
        if (md == nullptr){
            Logger::Info("Failed to find resource model called %s", name.c_str());
            return false;
        }
        number = number -1;
        if (number < 0){
            Logger::Info("Point group %d is invalid!", number+1);
            return false;
        }
        if (!md->hasPointGroup(number)){
            Logger::Info("Model %s does not have point group %d", number+1);
            return false;
        }
        obj = md;
        pointGroup = number;
        usingModel = false;
        return obj;
    } else {
        Model *md = g_models.GetModelByName(resourceName);
        if (md == nullptr){
            Logger::Info("Failed to find resource model called %s", resourceName.c_str());
            return false;
        }
        obj = md;
        usingModel = true;
        return true;
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


int KeyframeTrack::AddKeyFrame(Keyframe kf){
    keyframes.emplace_back(kf);
    return keyframes.size();
}

void KeyframeTrack::Reserve(){
    for (int i=0;i<KEYFRAME_TYPE_LAST;i++){
        currentFrameByType.emplace_back(-1);
    }
}

KeyframeAnimation* KeyframePlayer::NewKeyframeAnimation(uint32_t duration){
    KeyframeAnimation* kf = new KeyframeAnimation();
    kf->duration = 3000;

    int idx = AddKeyframeAnimation(kf);
    kf->SetId(idx);
    return kf;
}

void KeyframeAnimation::AddModelReference(Model *m){
    auto it = std::find(m_models.begin(), m_models.end(), m);
    if (it == m_models.end()) {
        m_models.emplace_back(m);
    }
}

void KeyframeAnimation::AddTrack(KeyframeTrack t){
    if (t.obj == nullptr){
        return;
    }

    AddModelReference(t.obj);

    std::map<KeyframeType,int> lastDtByType;
    for (auto &track : t.keyframes){
        uint32_t aux = lastDtByType[track.type];
        lastDtByType[track.type] = track.playAt;
        track.deltaToNext =  track.playAt - aux;
    }
    m_tracks.emplace_back(t); 
}


void KeyframeAnimation::Reset(){
    m_prevDt = 0;
    for (auto &track : m_tracks){
        track.Reset();
    }
}

void KeyframePlayer::ResetSpecificAnimation(int id){
    if (id >= m_loadedAnimations.size()){
        Logger::Info("Cannot reset animation id %d, no such id exists", id);
        return;
    }
    auto &obj = m_loadedAnimations[id];
    if (obj == nullptr){
        Logger::Info("Cannot reset animation id %d, no such id exists", id);
        return;
    }
    obj->Reset();
}

void KeyframePlayer::PlayAnimationId(int32_t id, bool restartAnimation){
    if (id == m_currentlyPlaying){
        //If we're already playing it, there is no point in restarting it.
        return;
    }
    if (id >= m_loadedAnimations.size()){
        Logger::Info("Cannot play animation id %d, no such id exists", id);
        return;
    }
    
    m_currentlyPlaying = id;
    if (restartAnimation){
        ResetCurrentAnimation();
    }
}

void KeyframePlayer::ResetCurrentAnimation(){
    if (m_currentlyPlaying < 0){
        return;
    }
    auto &obj = m_loadedAnimations[m_currentlyPlaying];
    if (obj == nullptr){
        Logger::Info("Cannot play animation id %d, no such id exists", m_currentlyPlaying);
        return;
    }
    obj->Reset();
}

void KeyframePlayer::runModelAnim(uint32_t dt){
    auto &obj = m_loadedAnimations[m_currentlyPlaying];
    for (auto &track : obj->m_tracks){
        track.UpdateTrack(dt, obj->m_prevDt);
    }

    m_finishedAnimationInThisFrame = false;

    if (m_extraDtForNextFrame > 0){
        dt += m_extraDtForNextFrame;
        m_extraDtForNextFrame = 0;
    }

    int res = 0;
    obj->m_prevDt +=  dt;
    if (obj->m_prevDt >= obj->duration){  
        res = obj->m_prevDt-obj->duration;
        obj->m_prevDt = 0;

        for (auto &track : obj->m_tracks){
            track.Reset();
        }
        
        m_finishedAnimationInThisFrame = true;

        if (res > 0){
            m_extraDtForNextFrame = res;
        }
        return;
    }

   
    for (auto &mod : obj->m_models){
        mod->Recalculate();
        mod->CopyToRaster();
    }    
    
    g_modelHandler.RenderScene(obj->m_models);    
}

bool KeyframePlayer::Update(uint32_t dt){
    if (m_currentlyPlaying < 0){
        return false;
    }
    runModelAnim(dt);
    return m_finishedAnimationInThisFrame;
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

void KeyframeTrack::applyTransformations(uint32_t remaining, Keyframe &nextKf, bool lastIteration){
    switch (nextKf.type){
        case KEYFRAME_TRANSLATE:{
            float delta = 0.0f;
            if (nextKf.deltaToNext != 0){
                delta = (float)remaining/(float)nextKf.deltaToNext;
            }else{
                delta = 1.0f;
            }
           
            float px = delta * nextKf.value.x;
            float py = delta * nextKf.value.y;
            if (usingModel){  
                obj->Translate(Vec2f(px, py));
            }else{
                obj->TranslatePoints(pointGroup, Vec2f(px, py));
            }
            
            break;
        }
        case KEYFRAME_ROTATE:{
            if (nextKf.value.x == 0){
                return;
            }
            float delta = 0;
            if (nextKf.deltaToNext != 0){
                delta = (float)remaining/(float)nextKf.deltaToNext;
            }else{
                delta = 1.0f;
            }

            float ang = delta * nextKf.value.x;
            Vec2f center = nextKf.center;
            if (nextKf.dynamicCenter){ 
                if (usingModel){   
                    obj->Recalculate();
                    center = obj->GetCenter();
                }else{
                    center = obj->GetPointGroupCenter(pointGroup);
                }
            }
            if (usingModel){   
                obj->Rotate(center, ang * (3.14159265358979323846f / 180.0) );
            }else{
                obj->RotatePoints(pointGroup, center, ang * (3.14159265358979323846f / 180.0));
            }
            break;
        }
        case KEYFRAME_SCALE:{

            float delta = 0;
            if (nextKf.deltaToNext != 0){
                delta = (float)remaining/(float)nextKf.deltaToNext;
            }else{
                delta = 1.0f;
            }
            Vec2f scaleF(1.0f + (nextKf.value.x - 1.0f) * delta, 1.0f + (nextKf.value.y - 1.0f) * delta);

            Vec2f center = nextKf.center;
            if (nextKf.dynamicCenter){ 
                if (usingModel){   
                    obj->Recalculate();
                    center = obj->GetCenter();
                }else{
                    center = obj->GetPointGroupCenter(pointGroup);
                }
            }

            if (usingModel){   
                obj->Scale(center, scaleF);
            }else{
                obj->ScalePoints(pointGroup, center, scaleF);
            }
            break;
        }
        case KEYFRAME_RESET:{
            if (lastIteration){
                obj->Reset();
            }
            break;
        }
    
    default:
        break;
    }
}