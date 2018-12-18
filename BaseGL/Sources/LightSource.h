#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "Transform.h"

class LightSource : public Transform {
private:
  glm::vec3 color;
  float intensity;
  float coneAngle;
  float radialAttenuation;
  glm::vec3 distanceAttenuation;
public:
  LightSource(){};
  inline void setColor(glm::vec3 _color){color=_color;};
  inline void setIntensity(float _intensity){intensity=_intensity;};
  inline void setConeAngle(float _coneAngle){coneAngle=_coneAngle;};
  inline void setRadialAttenuation(float _radialAttenuation){radialAttenuation=_radialAttenuation;};
  inline void setDistanceAttenuation(glm::vec3 _distanceAttenuation){distanceAttenuation=_distanceAttenuation;};
  inline glm::vec3 getColor(){return color;};
  inline float getIntensity(){return intensity;};
  inline float getConeAngle(){return coneAngle;};
  inline float getRadialAttenuation(){return radialAttenuation;};
  inline glm::vec3 getDistanceAttenuation(){return distanceAttenuation;};
};

#endif // LIGHTSOURCE_H
