#version 450 core // Minimal GL version support expected from the GPU

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

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal;
in vec2 fTexCoord;
in vec3 fKeyLightPosition;
in vec3 fFillLightPosition;
in vec3 fBackLightPosition;
in float fDFocal;
in float fDEye;
in vec3 fTangent, fBitangent;

out vec4 colorResponse; // Shader output: the color response attached to this fragment

float computeLiFromLight(LightSource lightSource, vec3 fLightPosition, vec3 n){
	vec3 wi = normalize(fLightPosition - fPosition);
	vec3 wo = normalize(-fPosition);
	vec3 wh = normalize(wi+wo);

	float fd = material.kd/3.14159;

	float metallic;
	float roughness;

	if(textureUsing ==1){
		metallic = texture(material.metallicTex,fTexCoord).r;
		roughness = texture(material.roughnessTex,fTexCoord).r;
	} else {
		metallic = material.metallic;
		roughness = material.roughness;
	}

	float ambient = texture(material.ambientTex,fTexCoord).r;
	float F = metallic + (1-metallic)*pow(1-max(0,dot(wi,wh)),5);
	float D = pow(roughness,2)/(3.14159*pow((1+(pow(roughness,2)-1)*pow(dot(n,wh),2)),2));
	float Gi = 2*dot(n,wi)/(dot(n,wi)+sqrt(pow(roughness,2)+(1-pow(roughness,2))*pow(dot(n,wi),2)));
	float Go =  2*dot(n,wo)/(dot(n,wo)+sqrt(pow(roughness,2)+(1-pow(roughness,2))*pow(dot(n,wo),2)));
	float G = Gi * Go;
	float fs = D*F*G/(4*dot(n,wi)*dot(n,wo));

	vec3 Li = lightSource.color * lightSource.intensity * (fs+fd) * max(dot(n, wi),0);
	float d = distance(fLightPosition, fPosition);
	float att = 1/(lightSource.distanceAttenuation[0]+lightSource.distanceAttenuation[1]*d+lightSource.distanceAttenuation[2]*pow(d,2));
	if(textureUsing==1){
		return Li*att*ambient;
	} else {
		return Li*att;
	}
}

bool criteriaSpecular(vec3 wi, vec3 wo,vec3 n, float limit){
	if(dot(2*n*dot(wi,n)-wi,wo)>limit){
		return true;
	} else {
		return false;
	}
}

vec3 computeNPR(vec3 n,vec3 fr){
	vec3 wiKey = normalize(fKeyLightPosition - fPosition);
	vec3 wiFill = normalize(fFillLightPosition - fPosition);
	vec3 wiBack = normalize(fBackLightPosition - fPosition);
	vec3 wo = normalize(-fPosition);
	bool criteria;
	float limit = 0.8;
	if(numberLightUsed==1){
		criteria = criteriaSpecular(wiKey,wo,n,limit);
	} else if (numberLightUsed==2){
		criteria = criteriaSpecular(wiFill,wo,n,limit)||criteriaSpecular(wiKey,wo,n,limit);
	} else {
		criteria = criteriaSpecular(wiKey,wo,n,limit)||criteriaSpecular(wiFill,wo,n,limit)||criteriaSpecular(wiBack,wo,n,limit);
	}
	if(abs(n[2])<0.4){
		return vec3(0,0,0);
	} else if(criteria){
		return vec3(1,1,1);
	} else {
		return fr;
	}
}

float computeOrientationCriteria(vec3 n, vec3 wi, vec3 wo){
	return max(dot(normalize(2*n*dot(wi,n)-wi),wo)/3,0.01);
}

vec3 computeOrientationTone(vec3 n, vec3 wi, vec3 wo){
	float D = computeOrientationCriteria(n, wi, wo);
	return texture(material.toneTex,vec2(max(dot(n,wi),0)/3,D)).rgb;
}

void main() {
	vec3 fr ;
	if (shaderMode== 0){
		if(textureUsing == 1){
			fr = texture(material.albedoTex,fTexCoord).rgb;
		} else {
			fr = material.albedo;
		}
		 //*TODO*//
	} else if (shaderMode == 1) {
		fr = vec3(0.1,0.6,0.3);
	}
	vec3 normalMap = normalize(texture(material.normalTex, fTexCoord).rgb);
	//normalMap = normalize(normalMap*2.0 - 1.0);
	vec3 tangent = normalize(fTangent);
	vec3 bitangent = normalize(fBitangent);
	vec3 n = normalize(fNormal);
	normalMap = normalize(mat3(tangent,bitangent,n)*normalMap);
	if(normalMapUsed==1){
		n = normalize (normalMap);
	}
	if(shaderMode == 0){
		float LiKey = computeLiFromLight(keyLight, fKeyLightPosition, n);
		vec3 radiance =  fr;
		if(numberLightUsed==1){
			radiance = radiance * LiKey;
		} else if (numberLightUsed == 2){
			float LiFill = computeLiFromLight(fillLight, fFillLightPosition,n);
			radiance = radiance * (LiKey + LiFill);
		} else if (numberLightUsed == 3){
			float LiFill = computeLiFromLight(fillLight, fFillLightPosition,n);
			float LiBack = computeLiFromLight(backLight, fBackLightPosition,n);
			radiance = radiance * (LiKey+LiFill+LiBack) ;
		}
	    colorResponse = vec4 (radiance, 1.0); // Building an RGBA value from an RGB one.
	} else if (shaderMode == 1){
			colorResponse = vec4(computeNPR(n,fr),1.0);
	} else if (shaderMode == 2){
		vec3 wiKey = normalize(fKeyLightPosition - fPosition);
		vec3 wiFill = normalize(fFillLightPosition - fPosition);
		vec3 wiBack = normalize(fBackLightPosition - fPosition);
			colorResponse = vec4(texture(material.toneTex,vec2(max(dot(n,wiKey)/3, 0.0),clamp(fDFocal,0.1,0.9))).rgb,1.0);
	} else if (shaderMode == 3){
		vec3 wiKey = normalize(fKeyLightPosition - fPosition);
		vec3 wiFill = normalize(fFillLightPosition - fPosition);
		vec3 wiBack = normalize(fBackLightPosition - fPosition);
			colorResponse = vec4(texture(material.toneTex,vec2(max(dot(n,wiKey)/3,0.0),clamp(fDEye,0.1,0.9))).rgb,1.0);
	} else if (shaderMode == 4){
		vec3 wiKey = normalize(fKeyLightPosition - fPosition);
		vec3 wiFill = normalize(fFillLightPosition - fPosition);
		vec3 wiBack = normalize(fBackLightPosition - fPosition);
		vec3 wo = normalize(-fPosition);
		if(numberLightUsed == 1){
			vec3 tone = computeOrientationTone(n,wiKey,wo);
			colorResponse = vec4(tone,1.0);
		} else if (numberLightUsed == 2){
			float intensityTot = keyLight.intensity+fillLight.intensity;
			vec3 tone = (keyLight.intensity*computeOrientationTone(n,wiKey,wo) + fillLight.intensity*computeOrientationTone(n,wiFill,wo))/intensityTot;
			colorResponse = vec4(tone,1.0);
		} else if (numberLightUsed == 3){
			float intensityTot = keyLight.intensity+fillLight.intensity+backLight.intensity;
			vec3 tone = (keyLight.intensity*computeOrientationTone(n,wiKey,wo) + fillLight.intensity*computeOrientationTone(n,wiFill,wo)+ backLight.intensity*computeOrientationTone(n,wiBack,wo))/intensityTot;
			colorResponse = vec4(tone,1.0);
		}
	}

}
