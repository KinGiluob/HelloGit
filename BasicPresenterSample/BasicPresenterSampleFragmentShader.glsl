#version 400
in vec3 inNormal;
in vec2 textureCoordinate;
in vec4 eyePosition;
out vec4 finalColor;

struct Light
{
  vec3 position;
  vec3 diffuse;
  vec3 specular;
};

uniform Light lights[4];
uniform int numLights;
uniform int renderStylus;

uniform vec3 specularColor;
uniform vec3 ambientColor;

uniform sampler2D diffuseTexture;

void main(void)
{
  vec3 normal = normalize(inNormal);
  vec3 lightDir;
  vec3 halfVector;
  vec3 color;
  float NdotL;
  float NdotHV;
  vec3 eye;
  
  if (renderStylus == 0)
  {
	vec4 textureSample = texture2D(diffuseTexture, textureCoordinate);
	vec3 diffuseTextureColor = vec3(textureSample);
  
	eye.x = eyePosition.x; eye.y = eyePosition.y; eye.z = eyePosition.z; 
	color = diffuseTextureColor*ambientColor;

	for (int i=0; i<numLights; i++)
	{
	lightDir = normalize(lights[i].position - eye);
	halfVector = normalize(lightDir + vec3(0, 0, 1));
	NdotL = max(dot(normal,lightDir),0.0);
	if (NdotL > 0.0) 
		{
		  color += diffuseTextureColor * NdotL * lights[i].diffuse;
		  NdotHV = max(dot(normal,halfVector),0.0);
		  color += specularColor * lights[i].specular * pow(NdotHV, 32);
		}
	}
  }
  else
  {
	  color.r = 1.0;
	  color.g = 1.0;
	  color.b = 1.0;
  }
  
  finalColor = vec4(color.r, color.g, color.b, 1);
}