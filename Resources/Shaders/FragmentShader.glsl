#version 450 core // Minimal GL version support expected from the GPU

layout(location = 0) out vec4 colorResponse;

// 0 means PBR rendering
#define GLSL_SHADER_MODE_PBR 0
// 1 means a basic toon rendering
#define GLSL_SHADER_BASIC_TOON 1
// 2 means a depth-of-field-based X-toon rendering
#define GLSL_SHADER_DEPTH_X_TOON 2
// 3 means a perspective-based X-toon rendering
#define GLSL_SHADER_PERSEPECTIVE_X_TOON 3
// 4 means a orientation-based X-toon rendering
#define GLSL_SHADER_ORIENTATION 4
// 5 means a orientation-based X-toon rendering
#define GLSL_SHADER_DEPTH_MAPPING 5
// 6 means visualize the distance traveled by light in the object
#define GLSL_SHADER_DISTANCE_TRAVELED 6

#define M_PI 3.1415926535897932384626433832795

struct LightSource {
	vec3 color;
	float intensity;
	float coneAngle;
	float radialAttenuation;
	vec3 distanceAttenuation;
};

uniform LightSource keyLight;
uniform LightSource fillLight;
uniform LightSource backLight;

struct Material {
	float kd;
	sampler2D metallicTex;
	sampler2D roughnessTex;
	sampler2D albedoTex;
	sampler2D ambientTex;
	sampler2D toneTex;
	sampler2D normalTex;
	float metallic;
	float roughness;
	vec3 albedo;
};

uniform Material material;

uniform int numberLightUsed;
uniform int shaderMode;
uniform float zMax;
uniform float zMin;
uniform int normalMapUsed;
uniform int textureUsing;
uniform float fov;
uniform float aspectRatio;
uniform vec3 meshCenter;
uniform vec4 meshCenterFromLight;
uniform mat4 modelViewMat;
uniform mat4 projectionMat;
uniform mat4 modelViewMatFromLight;
uniform sampler2D renderedTexture;
uniform int subsurfaceScattering;
uniform mat4 normalMatFromLight;
uniform int windowHeight;
uniform float windowRatio;

float sssSkinDistance = 0.01;

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal;
in vec2 fTexCoord;
in vec3 fKeyLightPosition;
in vec3 fFillLightPosition;
in vec3 fBackLightPosition;
in float fDFocal;
in float fDEye;
in vec3 fTangent, fBitangent;
in vec3 fPositionInWorld;
in vec3 fNormalInWorld;

float computeDistanceTraveledByLight()
{
	float fovInRad = fov/180.0*M_PI;
	float vFovInRad = 2.0f*atan(tan(fovInRad*0.5f)/aspectRatio);

	vec4 positionFromLight = modelViewMatFromLight * vec4(fPositionInWorld, 1.0);
	
	float alpha = atan(positionFromLight.x/length(vec3(positionFromLight.x, 0.0, positionFromLight.z)))/aspectRatio;
	float beta = atan(positionFromLight.y/length(vec3(0.0, positionFromLight.y, positionFromLight.z)));

	float distanceToCenter = abs(length((modelViewMatFromLight * vec4(meshCenter, 1.0)).xyz));
	float distanceToLight = abs(length((modelViewMatFromLight * vec4(fPositionInWorld, 1.0)).xyz));
	
	vec4 textureValue = texture(renderedTexture, vec2(alpha+0.5, beta+0.5));

	float distanceTraveled = abs((textureValue.x*distanceToCenter+distanceToCenter/2.0)-distanceToLight);

	return abs(distanceTraveled-distanceToCenter/10.0)/distanceToCenter;
}

vec3 computeEnergyFromSubsurfaceScattering(float distance)
{
	float energy = 0.5*pow(1.1, -75.0*distance);
	return vec3(energy);
}

vec3 computeLiFromLight(LightSource lightSource, vec3 fLightPosition, vec3 n){
	vec3 wi = normalize(fLightPosition - fPosition);
	vec3 wo = normalize(-fPosition);
	vec3 wh = normalize(wi+wo);

	vec3 fd = vec3(material.kd/M_PI);

	vec3 metallic;
	vec3 roughness;
	if(textureUsing ==1)
	{
		metallic = texture(material.metallicTex,fTexCoord).xyz;
		roughness = texture(material.roughnessTex,fTexCoord).xyz;
	} 
	else 
	{
		metallic = vec3(material.metallic);
		roughness = vec3(material.roughness);
	}
	
	float ambient = texture(material.ambientTex,fTexCoord).r;
	vec3 F = vec3(metallic.r + (1-metallic.r)*pow(1-max(0,dot(wi,wh)),5), metallic.g + (1-metallic.g)*pow(1-max(0,dot(wi,wh)),5), metallic.b + (1-metallic.b)*pow(1-max(0,dot(wi,wh)),5));
	vec3 D = vec3(pow(roughness.r,2)/(M_PI*pow((1+(pow(roughness.r,2)-1)*pow(dot(n,wh),2)),2)),pow(roughness.g,2)/(M_PI*pow((1+(pow(roughness.g,2)-1)*pow(dot(n,wh),2)),2)),pow(roughness.b,2)/(M_PI*pow((1+(pow(roughness.b,2)-1)*pow(dot(n,wh),2)),2)));
	vec3 Gi = vec3(2*dot(n,wi)/(dot(n,wi)+sqrt(pow(roughness.r,2)+(1-pow(roughness.r,2))*pow(dot(n,wi),2))), 2*dot(n,wi)/(dot(n,wi)+sqrt(pow(roughness.g,2)+(1-pow(roughness.g,2))*pow(dot(n,wi),2))), 2*dot(n,wi)/(dot(n,wi)+sqrt(pow(roughness.b,2)+(1-pow(roughness.b,2))*pow(dot(n,wi),2))));
	vec3 Go =  vec3(2*dot(n,wo)/(dot(n,wo)+sqrt(pow(roughness.r,2)+(1-pow(roughness.r,2))*pow(dot(n,wo),2))), 2*dot(n,wo)/(dot(n,wo)+sqrt(pow(roughness.g,2)+(1-pow(roughness.g,2))*pow(dot(n,wo),2))), 2*dot(n,wo)/(dot(n,wo)+sqrt(pow(roughness.b,2)+(1-pow(roughness.b,2))*pow(dot(n,wo),2))));
	
	vec3 G = Gi * Go;
	vec3 fs = D*F*G/(4*dot(n,wi)*dot(n,wo));

	vec3 Li = lightSource.color * lightSource.intensity * (fs+fd) * max(dot(n, wi),0);
	float d = distance(fLightPosition, fPosition);
	float att = 1/(lightSource.distanceAttenuation[0]+lightSource.distanceAttenuation[1]*d+lightSource.distanceAttenuation[2]*pow(d,2));

	float distanceTraveled = computeDistanceTraveledByLight();
	vec3 contributionFromSSS =  vec3(abs(dot(n,wi))*computeEnergyFromSubsurfaceScattering(distanceTraveled));

	/*float distanceToEnhancedRed = 0.30;
	if (dot(n,wi)<0 && distanceTraveled < distanceToEnhancedRed / 2.0)
	{
		float enhancedFactor = 1.0 + (distanceToEnhancedRed/2.0 - distanceTraveled)*2.0/distanceToEnhancedRed;
		contributionFromSSS = pow(enhancedFactor, 3.0) * vec3(contributionFromSSS.x*2.0, contributionFromSSS.y*enhancedFactor, contributionFromSSS.z*enhancedFactor);
	}
	else if (dot(n,wi)<0 && distanceTraveled < distanceToEnhancedRed)
	{
		float enhancedFactor = 1.0 + (distanceToEnhancedRed - distanceTraveled)*2.0/distanceToEnhancedRed;
		contributionFromSSS = vec3(contributionFromSSS.x*enhancedFactor, contributionFromSSS.y, contributionFromSSS.z);
	}*/

	if (subsurfaceScattering == 1)
	{
		Li = Li + contributionFromSSS;
	}
	else if (subsurfaceScattering == 2)
	{
		Li = contributionFromSSS;
	}

	if(textureUsing==1)
	{
		return Li*att*ambient;
	} 
	else 
	{
		return Li*att;
	}
}

bool criteriaSpecular(vec3 wi, vec3 wo,vec3 n, float limit)
{
	if(dot(2*n*dot(wi,n)-wi,wo)>limit)
	{
		return true;
	} else {
		return false;
	}
}

vec3 computeNPR(vec3 n,vec3 fr)
{
	vec3 wiKey = normalize(fKeyLightPosition - fPosition);
	vec3 wiFill = normalize(fFillLightPosition - fPosition);
	vec3 wiBack = normalize(fBackLightPosition - fPosition);
	vec3 wo = normalize(-fPosition);
	bool criteria;
	float limit = 0.8;

	if(numberLightUsed==1)
	{
		criteria = criteriaSpecular(wiKey,wo,n,limit);
	} else if (numberLightUsed==2)
	{
		criteria = criteriaSpecular(wiFill,wo,n,limit)||criteriaSpecular(wiKey,wo,n,limit);
	} 
	else 
	{
		criteria = criteriaSpecular(wiKey,wo,n,limit)||criteriaSpecular(wiFill,wo,n,limit)||criteriaSpecular(wiBack,wo,n,limit);
	}
	if(abs(n[2])<0.4)
	{
		return vec3(0,0,0);
	} else if(criteria){
		return vec3(1,1,1);
	} else {
		return fr;
	}
}

float computeOrientationCriteria(vec3 n, vec3 wi, vec3 wo)
{
	return max(dot(normalize(2*n*dot(wi,n)-wi),wo)/3,0.01);
}

vec3 computeOrientationTone(vec3 n, vec3 wi, vec3 wo)
{
	float D = computeOrientationCriteria(n, wi, wo);
	return texture(material.toneTex,vec2(max(dot(n,wi),0)/3,D)).rgb;
}

void main() 
{
	vec3 fr ;
	if (shaderMode == GLSL_SHADER_MODE_PBR)
	{
		if(textureUsing == 1)
		{
			fr = texture(material.albedoTex,fTexCoord).rgb;
		} 
		else 
		{
			fr = material.albedo;
		}
	} 
	else if (shaderMode == 1) 
	{
		fr = vec3(0.1,0.6,0.3);
	}

	vec3 normalMap = normalize(texture(material.normalTex, fTexCoord).rgb);
	vec3 tangent = normalize(fTangent);
	vec3 bitangent = normalize(fBitangent);
	vec3 n = normalize(fNormal);

	if(normalMapUsed==1)
	{
		n = normalize(mat3(tangent,bitangent,n)*normalMap);
	}

	if(shaderMode == GLSL_SHADER_MODE_PBR)
	{
		vec3 LiKey = computeLiFromLight(keyLight, fKeyLightPosition, n);
		vec3 radiance =  fr;
		if(numberLightUsed == 1){
			radiance = radiance * LiKey;
		} else if (numberLightUsed == 2){
			vec3 LiFill = computeLiFromLight(fillLight, fFillLightPosition,n);
			radiance = radiance * (LiKey + LiFill);
		} else if (numberLightUsed == 3){
			vec3 LiFill = computeLiFromLight(fillLight, fFillLightPosition,n);
			vec3 LiBack = computeLiFromLight(backLight, fBackLightPosition,n);
			radiance = radiance * (LiKey+LiFill+LiBack) ;
		}
	    colorResponse = vec4 (radiance, 1.0); // Building an RGBA value from an RGB one.
	} 
	else if (shaderMode == GLSL_SHADER_BASIC_TOON)
	{
			colorResponse = vec4(computeNPR(n,fr),1.0);
	} 
	else if (shaderMode == GLSL_SHADER_DEPTH_X_TOON)
	{
		vec3 wiKey = normalize(fKeyLightPosition - fPosition);
		vec3 wiFill = normalize(fFillLightPosition - fPosition);
		vec3 wiBack = normalize(fBackLightPosition - fPosition);
		colorResponse = vec4(texture(material.toneTex,vec2(max(dot(n,wiKey)/3, 0.0),clamp(fDFocal,0.1,0.9))).rgb,1.0);
	} 
	else if (shaderMode == GLSL_SHADER_PERSEPECTIVE_X_TOON)
	{
		vec3 wiKey = normalize(fKeyLightPosition - fPosition);
		vec3 wiFill = normalize(fFillLightPosition - fPosition);
		vec3 wiBack = normalize(fBackLightPosition - fPosition);
		colorResponse = vec4(texture(material.toneTex,vec2(max(dot(n,wiKey)/3,0.0),clamp(fDEye,0.1,0.9))).rgb,1.0);
	} 
	else if (shaderMode == GLSL_SHADER_ORIENTATION)
	{
		vec3 wiKey = normalize(fKeyLightPosition - fPosition);
		vec3 wiFill = normalize(fFillLightPosition - fPosition);
		vec3 wiBack = normalize(fBackLightPosition - fPosition);
		vec3 wo = normalize(-fPosition);
		if(numberLightUsed == 1)
		{
			vec3 tone = computeOrientationTone(n,wiKey,wo);
			colorResponse = vec4(tone,1.0);
		} 
		else if (numberLightUsed == 2)
		{
			float intensityTot = keyLight.intensity+fillLight.intensity;
			vec3 tone = (keyLight.intensity*computeOrientationTone(n,wiKey,wo) + fillLight.intensity*computeOrientationTone(n,wiFill,wo))/intensityTot;
			colorResponse = vec4(tone,1.0);
		} 
		else if (numberLightUsed == 3)
		{
			float intensityTot = keyLight.intensity+fillLight.intensity+backLight.intensity;
			vec3 tone = (keyLight.intensity*computeOrientationTone(n,wiKey,wo) + fillLight.intensity*computeOrientationTone(n,wiFill,wo)+ backLight.intensity*computeOrientationTone(n,wiBack,wo))/intensityTot;
			colorResponse = vec4(tone,1.0);
		}
	}
	else if (shaderMode == GLSL_SHADER_DEPTH_MAPPING)
	{
		float distanceToLight = abs(length((modelViewMatFromLight * vec4(fPositionInWorld, 1.0)).xyz));
		float distanceToCenter = abs(length((modelViewMatFromLight * vec4(meshCenter, 1.0)).xyz));

		colorResponse = vec4((distanceToLight-distanceToCenter/2.0)/distanceToCenter);
	}
	else if (shaderMode == GLSL_SHADER_DISTANCE_TRAVELED)
	{
		colorResponse = vec4(computeDistanceTraveledByLight());
	}
}
