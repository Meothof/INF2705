//#version 410

layout(quads) in;

// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position[2];
} LightSource;

// Définition des paramètres globaux du modèle de lumière
layout (std140) uniform LightModelParameters
{
   vec4 ambient;       // couleur ambiante
   bool localViewer;   // observateur local ou à l'infini?
   bool twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel;

// Declaration implicite de ces variables
// in vec3 gl_TessCoord;
// in int gl_PatchVerticesIn;
// in int gl_PrimitiveID;
// patch in float gl_TessLevelOuter[4];
// patch in float gl_TessLevelInner[2];
// in gl_PerVertex
// {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// } gl_in[gl_MaxPatchVertices];

// out gl_PerVertex {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// };

uniform mat3 matrNormale;

uniform float facteurDeform;
uniform sampler2D textureDepl;

out Attribs {
   vec2 texCoord;
   vec3 normale;
} AttribsOut;

float interpole( float v0, float v1, float v2, float v3 )
{
   float v01 = mix( v0, v1, gl_TessCoord.x );
   float v32 = mix( v3, v2, gl_TessCoord.x );
   return mix( v01, v32, gl_TessCoord.y );
}
vec2 interpole( vec2 v0, vec2 v1, vec2 v2, vec2 v3 )
{
   vec2 v01 = mix( v0, v1, gl_TessCoord.x );
   vec2 v32 = mix( v3, v2, gl_TessCoord.x );
   return mix( v01, v32, gl_TessCoord.y );
}
vec3 interpole( vec3 v0, vec3 v1, vec3 v2, vec3 v3 )
{
   vec3 v01 = mix( v0, v1, gl_TessCoord.x );
   vec3 v32 = mix( v3, v2, gl_TessCoord.x );
   return mix( v01, v32, gl_TessCoord.y );
}
vec4 interpole( vec4 v0, vec4 v1, vec4 v2, vec4 v3 )
{
   vec4 v01 = mix( v0, v1, gl_TessCoord.x );
   vec4 v32 = mix( v3, v2, gl_TessCoord.x );
   return mix( v01, v32, gl_TessCoord.y );
}

// affichage de la fonction paramétrique
uniform vec4 bDim;
vec3 FctParam( vec2 uv )
{
   const float PI = 3.141592654;
   vec3 p = vec3(0);
#if ( INDICEFONCTION == 1 )
   // sphere
   float s = uv.x * 2.*PI;
   float t = uv.y * PI;
   p = vec3( sin(t)*cos(s),
             sin(t)*sin(s),
             cos(t) * facteurDeform );
#elif ( INDICEFONCTION == 2 )
   // tore
   float s = uv.x * 2.*PI;
   float t = uv.y * 2.*PI;
   p = vec3( ( 0.5*cos(s) + 1.0 )*cos(t),
             ( 0.5*cos(s) + 1.0 )*sin(t),
             0.5*sin(s) * facteurDeform );
#elif ( INDICEFONCTION == 3 )
   // pomme
   float s = uv.x * 2.*PI;
   float t = uv.y * 2.*PI - PI;
   p = 0.1 * vec3( cos(s) * (4 + 3.8 * cos(t)) * facteurDeform,
                   sin(s) * (4 + 3.8 * cos(t)) * facteurDeform,
                   (cos(t)+sin(t)-1)*(1+sin(t))*log(1-PI*t/10)+7.5*sin(t) );
#elif ( INDICEFONCTION == 4 )
   // coquille
   float s = -6. + ( 1.1*PI + 6. ) * uv.x;
   float t = uv.y * PI;
   p = pow(4./3.,s) * vec3( sin(t)*sin(t)*cos(s) * facteurDeform,
                            sin(t)*sin(t)*sin(s) * facteurDeform,
                            sin(t)*cos(t) );
#elif ( INDICEFONCTION == 5 )
   // serpent
   float s = uv.x;
   float t = uv.y * 2.*PI;
   p = vec3( (1.-s) * (2.+cos(t)) * cos(2*PI*s)/2.,
             (1.-s) * (2.+cos(t)) * sin(2*PI*s)/2.,
             2.*s + (1.-s) * sin(t)/2. * facteurDeform );
#elif ( INDICEFONCTION == 6 )
   // trefoil
   float s = uv.x * 2.*PI - PI;
   float t = uv.y * 2.*PI - PI;
   p = vec3( 0.75 * (sin(s) + 2*sin(2*s)) / (2+cos(t+2*PI/3)),
             sin(3*s)/(2+cos(t)),
             facteurDeform * (cos(s)-2*cos(2*s))*(2+cos(t))*(2+cos(t+2*PI/3))/8. );
#endif
   return p;
}

// déplacement du plan selon la texture
float FctText( vec2 texCoord )
{
   float z = 0.0;
   vec4 texels = texture(textureDepl, texCoord);
   z = length(texels) * facteurDeform / 10.0;
   return z; // à modifier!
}

void main( void )
{
   const float eps = 0.01;
   vec2 epsX = vec2(eps, 0.0);
   vec2 epsY = vec2(0.0, eps);

   // interpoler la position selon la tessellation (dans le repère de modélisation)
   vec4 posModel = interpole( gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position );
   vec2 uv = posModel.xy;

   // générer (en utilisant directement posModel.xy) les coordonnées de texture plutôt que les interpoler
   AttribsOut.texCoord = posModel.xy;

#if ( INDICEFONCTION != 0 )

   // affichage de la fonction paramétrique (partie 1)
   posModel.xyz = FctParam(uv);

   // calculer la normale
   vec3 N = normalize(cross(FctParam(uv + epsY) - FctParam(uv - epsY),
                  FctParam(uv + epsX) - FctParam(uv - epsX))); // à modifier



#else

   // déplacement selon la texture (partie 2)

    posModel.xy -= .5;
    posModel.xy *= 2*bDim.xy;
    AttribsOut.texCoord = posModel.xy;
    posModel.z = FctText(posModel.xy);

   float x = posModel.x;
   float y = posModel.y;
   // calculer la normale
   vec3 N = normalize(vec3((FctText(posModel.xy + epsX) - FctText(posModel.xy - epsX))/(2*eps),
                 (FctText(posModel.xy + epsY) - FctText(posModel.xy - epsY))/(2*eps),
                  -1 ));


#endif

#if ( AFFICHENORMALES == 1 )
   AttribsOut.normale = N;
#else
   AttribsOut.normale = matrNormale * N;
#endif

   gl_Position = posModel;
}
