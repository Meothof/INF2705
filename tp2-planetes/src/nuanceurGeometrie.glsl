#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 8) out;

uniform vec4 planCoupe; // équation du plan de coupe

in Attribs {
   vec4 couleur;
   vec4 pos;
   // float clipDistance;
} AttribsIn[];

out Attribs {
   vec4 couleur;
} AttribsOut;

void main()
{
   // calculer le centre
   vec4 centre = vec4(0.0);
   for ( int i = 0 ; i < gl_in.length() ; ++i )
      centre += gl_in[i].gl_Position;
   centre /= gl_in.length();

   // Ã©mettre les sommets
   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
        gl_ViewportIndex = 0;
        gl_Position = gl_in[i].gl_Position;
        gl_Position.y *= 2;
        // Initialiser gl_ClipDistance

        gl_ClipDistance[0] = -dot(planCoupe, AttribsIn[i].pos);
        AttribsOut.couleur = AttribsIn[i].couleur;
        EmitVertex();
   }

   EndPrimitive();

   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {

        gl_ViewportIndex = 1;
        gl_Position = gl_in[i].gl_Position;
        gl_Position.y *= 2;

        gl_ClipDistance[0] = dot(planCoupe, AttribsIn[i].pos);
        AttribsOut.couleur = AttribsIn[i].couleur;
        EmitVertex();
   }
   EndPrimitive();
}