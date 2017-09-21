#version 410

uniform int coulProfondeur;
const float debAttenuation = 20.0;
const float finAttenuation = 40.0;

in Attribs {
   vec4 couleur;
} AttribsIn;

out vec4 FragColor;

void main( void )
{
   // la couleur du fragment est la couleur interpolée
   FragColor = AttribsIn.couleur;

   // atténuer selon la profondeur
   if ( coulProfondeur == 1 )
   {
      // Obtenir la distance à la caméra du sommet dans le repère de la caméra
      float dist = gl_FragCoord.z / gl_FragCoord.w;
      // Obtenir un facteur d'interpolation entre 0 et 1
      float factDist = smoothstep( 20.0f, 40.0f, dist);
      // Modifier la couleur du fragment en utilisant ce facteur
      FragColor *= 1-factDist;

      // pour déboguer et « voir » la dist, on peut utiliser:
      //FragColor = vec4( vec3(dist-floor(dist)), 1.0 );
      //FragColor = vec4( vec3(factDist), 1.0 );
   }

   // pour déboguer et « voir » le comportement de z ou w, on peut utiliser:
   //FragColor = vec4( vec3(gl_FragCoord.z), 1.0 );
   //FragColor = vec4( vec3(gl_FragCoord.w), 1.0 );
}
