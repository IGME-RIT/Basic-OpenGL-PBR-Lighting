

#version 430 core
out vec4 fragColor;

in vec3 fragPosition;
in vec2 uv;
in vec3 normal;

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform vec4 lightPositions[4];
uniform vec3 camPos;

float PI = 3.14159265;

//PBR LIGHTING
//The base theory is founded on microfacet approximation and energy conversation.
//Many of the following equations are listed here:
//http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
//Implemented in UE4s shading model

//Microfacet approximation - surfaces are combined of microscopic reflective mirrors(microfacets)
//with random alignments based on the surface roughness. If more are aligned with the vector
//between the light and view vectors, (halfway vector) the specular reflection is stronger.
//http://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf

//Energy conversation - outgoing energy < incoming energy. Light gets split into reflective
//and refractive componenets

//RENDERING EQUATION
//https://en.wikipedia.org/wiki/Rendering_equation
//Currently the best model out there for simulating light in graphics
//Based on principles of radiometry and other schools of physics.
//The equation using the following physical quantites:
//Radiant flux: Transmitted energy of light in Watts. Represented as light color
//Solid angle: Area of the silhouette created by objects hit by the light, projected onto a sphere.
//Radiant intensity: radiant flux per solid angle
//Intergrals calculate area of a functuion. We solve this discreetly
//by averaging the results of multiple steps through the equation.


//BRDF - Bidirectional Reflective Distribution Function
//Appoximates how each ray of light affects the final reflection.
//Needs light direction, outgoing light direction, surface normal and a roughness value.
//Most render pipelines use the Cook-Torrance BDRF
//Paper found here: http://inst.cs.berkeley.edu/~cs294-13/fa09/lectures/cookpaper.pdf
//Consusts of 3 core componenets:
//Normal distribution function
//Geometery fuction
//The Fresenel equation

//NORMAL DISTRIBUTION
//Approximates microfacets aligned to the halfway vector
//We use the Trowbridge-Reitz GGX
float DistributionGGX(vec3 norm, vec3 halfwayVector, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH2 = pow(max(dot(norm, halfwayVector), 0.0),2);

    return a2 / (PI * pow((NdotH2 * (a2 - 1.0) + 1.0),2));
}

//GEOMETRY FUNCTION
//Self-obstructing property of microfacets
//Higher roughness means more reflected light is obstructed by other microfacets
//We use the Schlick GGX here (combines GGX and Schlick-Beckmann approximation)
float GeometrySchlickGGX(float NdotV, float k)
{	
    return NdotV / (NdotV * (1.0 - k) + k);
}

//We use Smith's method of taking both view obstruction and gemotry shadowing
//into account by using the geometry function twice, once with the view vector(reflected light)
//and once with the incoming light vector (incoming light)
//https://learnopengl.com/PBR/Theory implementation found here under Geomtry Function
float GeometrySmith(vec3 norm, vec3 view, vec3 light, float k)
{
    float NdotV = max(dot(norm, view), 0.0);
    float NdotL = max(dot(norm, light), 0.0);
	
    return GeometrySchlickGGX(NdotV, k) * GeometrySchlickGGX(NdotL, k);
}

//FRESNEL EQUATION
//Calculates ratio of reflected light to refracted light depending on view angle.
//We use the Fresnel-Schlick approximation here as the base equation is longer and more complicated
//F0 is the base reflectivity of the material
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//variation of the above with roughness as an additional input
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  


void main()
{  

    vec3  lightColor  = vec3(300.0f, 300.0f, 300.0f);

    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - fragPosition); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);
    //BRDF for each light
    for(int i = 0; i < 4; ++i) 
    {
        
        vec3 L = normalize(lightPositions[i].xyz - fragPosition);
        vec3 H = normalize(V + L);
        float distance    = length(lightPositions[i].xyz - fragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColor * attenuation;        
        
       
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);  
            

        float NdotL = max(dot(N, L), 0.0);  

        //Render equation              
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
  
    vec3 ambient = vec3(0.3) * albedo * ao;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    fragColor = vec4(color, 1.0);

}  

