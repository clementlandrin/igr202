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
out vec3 fTangent, fBitangent;
out vec3 fPositionInWorld;
out vec3 fNormalInWorld;

void main() {
	vec4 p = modelViewMat * vec4 (vPosition, 1.0);
	vec4 n = normalMat * vec4 (vNormal, 1.0);
	fNormal = normalize (n.xyz);
    gl_Position =  projectionMat * p; // mandatory to fire rasterization properly
	fTangent = (normalMat * vec4 (vTangent, 0.0)).xyz;
	fTangent = normalize(fTangent);
	fBitangent = (normalMat * vec4 (vBitangent, 0.0)).xyz;
	fBitangent = normalize(fBitangent);
	fBitangent = (normalMat* vec4(normalize(cross(fNormal,fTangent)),0.0)).xyz;
    fPosition = p.xyz;
    fTexCoord = vec2(5.0*vTexCoord.x, 5.0*vTexCoord.y);
	fKeyLightPosition = vec3(modelViewMat * vec4(keyLightPosition,1));
	fFillLightPosition = vec3(modelViewMat * vec4(fillLightPosition,1));
	fBackLightPosition = vec3(modelViewMat * vec4(backLightPosition,1));
	fDFocal = clamp(1 - log(p.z/zMin)/log(r),0.0,1.0);
	fPositionInWorld = vPosition;
	fNormalInWorld = vNormal;

	if(p.z<zFocus)
	{
		fDEye = clamp(1 - log(distance(p.xyz,vec3(0,0,0))/(zFocus-zMin))/log((zFocus-r*zMin)/(zFocus-zMin)),0.0,1.0);
	} 
	else 
	{
		fDEye = clamp(log(distance(p.xyz,vec3(0,0,0))/(zFocus+r*zMin))/log((zFocus+zMin)/(zFocus+r*zMin)),0.0,1.0);
	}
}
