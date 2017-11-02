#version 410

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

layout(location=0) in vec4 Vertex;
layout(location=3) in vec4 Color;
in float tempsRestant;
in vec3 vitesse;

out Attribs {
   vec4 couleur;
   float tempsRestant;
   //float sens; // du vol
} AttribsOut;

void main( void )
{
   // transformation standard du sommet
   gl_Position = matrProj * matrVisu * matrModel * Vertex;

   AttribsOut.tempsRestant = tempsRestant;

   // couleur du sommet
   AttribsOut.couleur = Color;

   // À SUPPRIMER: la ligne suivante sert seulement à forcer le compilateur à conserver cet attribut
   if ( tempsRestant < 0.0 ) AttribsOut.couleur.a += 0.00001;
   if ( vitesse.r < 0.0 ) AttribsOut.couleur.b += 0.00001;
}
