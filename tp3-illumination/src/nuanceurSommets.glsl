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

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;
uniform mat3 matrNormale;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec4 Vertex;
layout(location=2) in vec3 Normal;
layout(location=3) in vec4 Color;
layout(location=8) in vec4 TexCoord;

out Attribs {
	vec3 lumiDir;
	vec3 normale, obsVec;
	vec4 couleur;
} AttribsOut;

vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
	// ajout de l’émission et du terme ambiant du modèle d’illumination
	vec4 coul = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;

	// calcul de la composante ambiante de la 1e source de lumière
	coul += FrontMaterial.ambient * LightSource[0].ambient;

	// produit scalaire pour le calcul de la réflexion diffuse
	float NdotL = max( 0.0, dot( N, L ) );

	// calcul de la composante spéculaire (selon Phong ou Blinn)
	if(utiliseBlinn){
		float NdotHV = max( 0.0, dot( normalize( L + O ), N ) ); // avec H et N
	}else{
		float NdotHV = max( 0.0, dot( reflect( -L, N ), O ) ); // avec R et O
	}
	// calcul de la composante spéculaire de la 1e source de lumière
	coul += FrontMaterial.specular * LightSource[0].specular *pow( NdotHV, FrontMaterial.shininess );

	// couleur finale du fragment
	return( clamp( coul, 0.0, 1.0 ) );
}

void main( void )
{
	// transformation standard du sommet
	gl_Position = matrProj * matrVisu * matrModel * Vertex;



	//calculer la normale qui sera interpollé pour la nuanceur de fragment
	vec3 N = matrNormale * Normal;

	// calculer la position du sommet (dans le repère de la camèra)
	vec3 pos = vec3( matrVisu * matrModel * Vertex );

	// vecteur de la direction de la lumière (dans le repère de la caméra)
	vec3 L = vec3( ( matrVisu * LightSource[0].position ).xyz - pos );

	// vecteur de la direction vers l'observateur
	vec3 O = ( LightModel.localViewer ?
				 normalize(-pos) :        // =(0-pos) un vecteur qui pointe vers le (0,0,0), c'est-Ã -dire vers la caméra
				 vec3( 0.0, 0.0, 1.0 ) ); // on considère que l'observateur (la caméra) est Ã  l'infini dans la direction (0,0,1)

	AttribsOut.lumiDir = L;
	AttribsOut.normale = N;
	AttribsOut.obsVec = O;
	// couleur du sommet
	if(typeIllumination = 1){
		//L'illumination de type Gouraud s'effectue directement dans le nuanceur de sommet
		AttribsOut.couleur = calculerReflexion(L,N,O);
	}else{
		AttribsOut.couleur = Color;
	}
	
}

