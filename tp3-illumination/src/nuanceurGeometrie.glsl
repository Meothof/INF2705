#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

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

in Attribs {
	vec4 couleur;
	vec3 lumiDir;
	vec3 normale, obsVec;
} AttribsIn[];

out Attribs {
	vec4 couleur;
	vec3 lumiDir;
	vec3 normale, obsVec;
} AttribsOut;

void main()
{
    vec3 normaleLambert;
    if(typeIllumination == 0)
    {
        vec3 prodVect = vec3(0.0, 0.0 , 0.0);
        for ( int i = 0 ; i < gl_in.length() ; i++ )
    	{
            prodVect += cross(gl_in[(i+1)%3].gl_Position.xyz - gl_in[i].gl_Position.xyz,
                gl_in[(i+2)%3].gl_Position.xyz - gl_in[i].gl_Position.xyz);
        }
        prodVect.xy *= -1;

        normaleLambert = normalize(prodVect);
    }

	// émettre les sommets
	for ( int i = 0 ; i < gl_in.length() ; ++i )
	{
		gl_Position = gl_in[i].gl_Position;
		AttribsOut.couleur = AttribsIn[i].couleur;
		AttribsOut.lumiDir = AttribsIn[i].lumiDir;
		if(typeIllumination == 0)
		    AttribsOut.normale = normaleLambert;
		else
		    AttribsOut.normale = AttribsIn[i].normale;
		AttribsOut.obsVec = AttribsIn[i].obsVec;
		EmitVertex();
	}
}
