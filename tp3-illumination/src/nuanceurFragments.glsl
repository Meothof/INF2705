#version 410

// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position;
   vec3 spotDirection;
   float spotExponent;
   float spotCutoff;            // ([0.0,90.0] ou 180.0)
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;
} LightSource[1];

// Définition des paramètres des matériaux
layout (std140) uniform MaterialParameters
{
   vec4 emission;
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   float shininess;
} FrontMaterial;

// Définition des paramètres globaux du modèle de lumière
layout (std140) uniform LightModelParameters
{
   vec4 ambient;       // couleur ambiante
   bool localViewer;   // observateur local ou à l'infini?
   bool twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel;

layout (std140) uniform varsUnif
{
   // partie 1: illumination
   int typeIllumination;     // 0:Lambert, 1:Gouraud, 2:Phong
   bool utiliseBlinn;        // indique si on veut utiliser modèle spéculaire de Blinn ou Phong
   bool utiliseDirect;       // indique si on utilise un spot style Direct3D ou OpenGL
   bool afficheNormales;     // indique si on utilise les normales comme couleurs (utile pour le débogage)
   // partie 3: texture
   int texnumero;            // numéro de la texture appliquée
   bool utiliseCouleur;      // doit-on utiliser la couleur de base de l'objet en plus de celle de la texture?
   int afficheTexelNoir;     // un texel noir doit-il être affiché 0:noir, 1:mi-coloré, 2:transparent?
};

uniform sampler2D laTexture;

/////////////////////////////////////////////////////////////////

in Attribs {
   vec4 couleur;
	vec3 lumiDir;
	vec3 normale, obsVec;
	vec2 texCoord;
} AttribsIn;

out vec4 FragColor;

float calculerSpot( in vec3 spotDir, in vec3 L )
{
    float cosGamma = dot(spotDir, L);
    float cosDelta = cos(radians(LightSource[0].spotCutoff));

    float fact = 0;

    if(utiliseDirect)
    {
        fact = smoothstep(pow(cosDelta, 1.01+LightSource[0].spotExponent), cosDelta, cosGamma);
    }
    else if(acos(cosGamma) < acos(cosDelta))
    {
       fact = pow(cosGamma, LightSource[0].spotExponent);
    }

    return fact;
}

vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
	// ajout de l’émission et du terme ambiant du modèle d’illumination
	vec4 coul;
  if( utiliseCouleur )
    coul = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;
  else
    coul = texture(laTexture, AttribsIn.texCoord);

	// calcul de la composante ambiante de la 1e source de lumière
	coul += FrontMaterial.ambient * LightSource[0].ambient;

	// produit scalaire pour le calcul de la réflexion diffuse
	float NdotL = dot( N, L );

	if(NdotL > 0.0)
	{
	    coul += FrontMaterial.diffuse * LightSource[0].diffuse * NdotL;

        // calcul de la composante spéculaire (selon Phong ou Blinn)
        float NdotHV;
        if(utiliseBlinn){
            NdotHV = max( 0.0, dot( normalize( L + O ), N ) ); // avec H et N
        }else{
            NdotHV = max( 0.0, dot( reflect( -L, N ), O ) ); // avec R et O
        }
        // calcul de la composante spéculaire de la 1e source de lumière
        coul += FrontMaterial.specular * LightSource[0].specular *pow( NdotHV, FrontMaterial.shininess );
    }
	// couleur finale du fragment
	return( clamp( coul, 0.0, 1.0 ) );
}

void main( void )
{
    // assigner la couleur finale
    FragColor = AttribsIn.couleur;

    vec3 L = normalize(AttribsIn.lumiDir);
    vec3 N = normalize(AttribsIn.normale);
    //vec3 O = normalize(AttribsIn.obsVec);

    if(typeIllumination != 1)
        FragColor = calculerReflexion(L, N, AttribsIn.obsVec);

    float factSpot = calculerSpot(-normalize( LightSource[0].spotDirection), L);
    FragColor *= factSpot;

    float seuilTex = 0.1;

    if( texnumero != 0 )
    {
        if( afficheTexelNoir == 0 )
            FragColor *= texture(laTexture, AttribsIn.texCoord);
        else if( afficheTexelNoir == 1)
        {
            if( texture(laTexture, AttribsIn.texCoord).r <= seuilTex &&
                texture(laTexture, AttribsIn.texCoord).g <= seuilTex &&
                texture(laTexture, AttribsIn.texCoord).b <= seuilTex)
                FragColor = 0.5 * (FragColor + texture(laTexture, AttribsIn.texCoord));
            else
                FragColor *= texture(laTexture, AttribsIn.texCoord);
        }
        else
        {
            if( texture(laTexture, AttribsIn.texCoord).r <= seuilTex &&
                texture(laTexture, AttribsIn.texCoord).g <= seuilTex &&
                texture(laTexture, AttribsIn.texCoord).b <= seuilTex)
                discard;
            else
                FragColor *= texture(laTexture, AttribsIn.texCoord);
        }
    }

    if ( afficheNormales ) FragColor = vec4(AttribsIn.normale,1.0);
}
