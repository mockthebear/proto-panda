#include "drawing/modelanimation/keyframeplayer.hpp"
#include "drawing/rendering/modelhandler.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"




Keyframe Keyframe::KeyFrameMaker(int mode, uint32_t at, Vec2f val, Vec2f cntr, uint16_t color , bool igInterp, bool dynamicCenterP){
    Keyframe kf;
    kf.type = (KeyframeType)mode;
    kf.playAt = at;
    kf.value = val;
    kf.center = cntr;
    kf.color = color;
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
    kf->duration = duration;

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
        track.interpolationStartedAt = aux;
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

void KeyframePlayer::PlayAnimationId(int32_t id, bool restartAnimation, ShaderType &shdr, float &strenght){

    m_defaultShader = shdr;
    m_defaultShaderStrenght = strenght;
    
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
        if (m_defaultShader != SHADER_NONE && mod->GetShader() == SHADER_NONE){
            mod->SetShaderWithStrenght(m_defaultShader, m_defaultShaderStrenght);
        }
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
    for (int operationType=KEYFRAME_TRANSLATE;operationType<KEYFRAME_TYPE_LAST;operationType++){

        int frameId = currentFrameByType[operationType];
        if (frameId >= 0){
            uint32_t sumLocal = prevDt; //current time
            uint32_t remaining = dt;

            Keyframe &nextKf = keyframes[frameId];
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
                    if (nextKf.ignoreInterpolation){
                        applyTransformations(currentStepDt + remaining, sumLocal, nextKf, true);
                        sumLocal += currentStepDt + remaining;
                    } else {
                        applyTransformations(currentStepDt, sumLocal, nextKf, true);
                        sumLocal += currentStepDt;
                    }
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
                    uint32_t currentStepDt = nextKf.playAt-sumLocal;
                    if (!nextKf.ignoreInterpolation){
                        applyTransformations(remaining, sumLocal, nextKf, false);
                    }
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

void KeyframeTrack::applyTransformations(uint32_t dt, uint32_t frameSum, Keyframe &nextKf, bool lastIteration){
    switch (nextKf.type){
        case KEYFRAME_TRANSLATE:{
            float delta = 0.0f;
            if (nextKf.deltaToNext != 0){
                delta = (float)dt/(float)nextKf.deltaToNext;
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
                delta = (float)dt/(float)nextKf.deltaToNext;
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
                delta = (float)dt/(float)nextKf.deltaToNext;
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
        case KEYFRAME_COLOR:{
            if (lastIteration){
                obj->SetColor(nextKf.color);
                colorInterpolationStarted = false;
                break;
            }


            if (!colorInterpolationStarted) {
                startColor = obj->color[0];
                colorInterpolationStarted = true;
            }

            float delta = 0;
            if (nextKf.deltaToNext != 0){
                delta = ((float)(frameSum-nextKf.interpolationStartedAt))/(float)nextKf.deltaToNext;
            }else{
                delta = 1.0f;
            }

            uint8_t startR, startG, startB;
            uint8_t endR, endG, endB;
            Devices::Display->color565to888(startColor, startR, startG, startB);
            Devices::Display->color565to888(nextKf.color, endR, endG, endB);

            uint8_t currentR = startR + (endR - startR) * delta;
            uint8_t currentG = startG + (endG - startG) * delta;
            uint8_t currentB = startB + (endB - startB) * delta;

            uint16_t interpolatedColor = Devices::Display->color565(currentR, currentG, currentB);
            obj->SetColor(interpolatedColor);

            break;
        }
        case KEYFRAME_VISIBILITY:{
            if (lastIteration){
                obj->visible = nextKf.dynamicCenter;
            }
            break;
        }
        case KEYFRAME_SINE: {

            
            float delta = 0;
            if (nextKf.deltaToNext != 0){
                delta = ((float)(frameSum-nextKf.interpolationStartedAt))/(float)nextKf.deltaToNext;
            }else{
                delta = 1.0f;
            }

            if (delta == 0.0f || lastIteration) {
                nextKf.m_storage[0] = 0;
                nextKf.m_storage[1] = 0;
            }
            
            float targetSineY, targetSineX;

            float angleX = 2.0f * M_PI * nextKf.center.x * delta;
            float angleY = 2.0f * M_PI * nextKf.center.y * delta;

            if (nextKf.color&2 == 1){
                targetSineY = nextKf.value.y * sin(angleY);
            }else{
                targetSineY = nextKf.value.y * cos(angleY);
            }
            if (nextKf.color&1 == 1){
                targetSineX = nextKf.value.x * sin(angleX);
            }else{
                targetSineX = nextKf.value.x * cos(angleX);
            }
            // Get last sine value (stored in keyframe)
            float lastSineX = nextKf.m_storage[0];
            float lastSineY = nextKf.m_storage[1];

            // Calculate delta from last frame
            float deltaSineY = targetSineY - lastSineY;
            float deltaSineX = targetSineX - lastSineX;
            
            if (usingModel) {
                obj->Translate(Vec2f(deltaSineX, deltaSineY));
            } else {
                obj->TranslatePoints(pointGroup, Vec2f(deltaSineX, deltaSineY));
            }
            
            nextKf.m_storage[0] = targetSineX;
            nextKf.m_storage[1] = targetSineY;
            
            if (delta >= 1.0f || lastIteration) {
                nextKf.m_storage[0] = 0;
                nextKf.m_storage[1] = 0;
            }
            break;
        }
        case KEYFRAME_SHADER:{
            ShaderType shdr = (ShaderType)nextKf.color;
            if (lastIteration){
                obj->SetShaderWithStrenght(shdr, 1.0f);
                break;
            }

            float delta = 0;
            if (nextKf.deltaToNext != 0){
                delta = ((float)(frameSum-nextKf.interpolationStartedAt))/(float)nextKf.deltaToNext;
            }else{
                delta = 1.0f;
            }
            if (shdr == SHADER_NONE){
                shdr = obj->GetShader();
                delta = 1.0f - delta;
            }
            obj->SetShaderWithStrenght(shdr, delta);
        }
    
    default:
        break;
    }
}