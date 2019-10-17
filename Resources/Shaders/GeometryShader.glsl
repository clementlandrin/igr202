#version 450

in vec3 fPosition[3]; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal[3];
in vec2 fTexCoord[3];
in vec3 fKeyLightPosition[3];
in vec3 fFillLightPosition[3];
in vec3 fBackLightPosition[3];
in float fDFocal[3];
in float fDEye[3];
in vec3 fTangent[3], fBitangent[3];

out vec3 gPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
out vec3 gNormal;
out vec2 gTexCoord;
out vec3 gKeyLightPosition;
out vec3 gFillLightPosition;
out vec3 gBackLightPosition;
out float gDFocal;
out float gDEye;
out vec3 gTangent, gBitangent;

float size = 0.075;
layout ( triangles ) in;
layout ( points , max_vertices = 1 ) out;

/*vec4 update_pos(vec4 pos, vec3 speed, float delta_t)
{
  pos += vec4(speed*delta_t,1.0);
  pos.y -= 0.5*9.81*delta_t*delta_t;
  return pos;
}*/

void main() {
	vec4 triangle_center = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position)/3.0;
	vec3 triangle_normal = normalize(fNormal[0] + fNormal[1] + fNormal[2]);
	//vec3 triangle_color = (color[0]+color[1]+color[2])/3.0*clamp(dot(normalize(fKeyLightPosition[0]-triangle_center.xyz), triangle_normal), 0.0, 1.0);

	float c;
	gPosition = fPosition[0]; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
	gNormal = fNormal[0];
	gTexCoord = fTexCoord[0];
	gKeyLightPosition = fKeyLightPosition[0];
	gFillLightPosition = fFillLightPosition[0];
	gBackLightPosition = fBackLightPosition[0];
	gDFocal = fDFocal[0];
	gDEye = fDEye[0];
	gTangent = fTangent[0];
	gBitangent = fBitangent[0];
  
	/*for (int i = 0; i<10; i++)
	{*/

		gl_Position = gl_in[0].gl_Position;//vec4(gPosition,1.0);update_pos(triangle_center, triangle_normal, size*0);
		EmitVertex();
	//}

	EndPrimitive();
}
