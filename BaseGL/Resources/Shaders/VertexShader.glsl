#version 450 core // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition; // The 1st input attribute is the position (CPU side: glVertexAttrib 0)
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoord;
layout(location=3) in vec3 vTangent;
layout(location=4) in vec3 vBitangent;

uniform mat4 projectionMat, modelViewMat, normalMat;
uniform vec3 keyLightPosition, fillLightPosition, backLightPosition;
uniform float zMin, r, zFocus;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoord;
out vec3 fKeyLightPosition;
out vec3 fFillLightPosition;
out vec3 fBackLightPosition;
out float fDFocal;
out float fDEye;
out mat3 TBN;

void main() {
	vec4 p = modelViewMat * vec4 (vPosition, 1.0);
    gl_Position =  projectionMat * p; // mandatory to fire rasterization properly
		vec3 T = normalize(normalMat * vec4 (vTangent, 1.0)).xyz;
		//vec3 B = normalize(normalMat * vec4 (vBitangent, 1.0)).xyz;
		vec4 n = normalMat * vec4 (vNormal, 1.0);
    fPosition = p.xyz;
    fNormal = normalize (n.xyz);
		T = normalize(T-dot(T,fNormal)*fNormal);
		vec3 B = cross(fNormal,T);
		TBN = mat3 (T,B,fNormal);
    fTexCoord = vTexCoord;
		fKeyLightPosition = vec3(modelViewMat * vec4(keyLightPosition,1));
		fFillLightPosition = vec3(modelViewMat * vec4(fillLightPosition,1));
		fBackLightPosition = vec3(modelViewMat * vec4(backLightPosition,1));
		fDFocal = clamp(1 - log(p.z/zMin)/log(r),0.0,1.0);
		if(p.z<zFocus){
			fDEye = clamp(1 - log(p.z/(zFocus-zMin))/log((zFocus-r*zMin)/(zFocus-zMin)),0.0,1.0);
		} else {
			fDEye = clamp(log(p.z/(zFocus+r*zMin))/log((zFocus+zMin)/(zFocus+r*zMin)),0.0,1.0);
		}
}
