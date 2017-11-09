#version 410

uniform sampler2D laTexture;
uniform int texnumero;

in Attribs {
   vec4 couleur;
   vec2 texCoord;
} AttribsIn;

out vec4 FragColor;

void main( void )
{

   if(texnumero != 0)
   {
       FragColor = texture( laTexture, AttribsIn.texCoord );

       if ( FragColor.a < 0.1 ) discard;
       else
       {
          FragColor.rgb = mix(AttribsIn.couleur.rgb, FragColor.rgb, 0.7);
          FragColor.a = AttribsIn.couleur.a;
       }
   }
   else
       FragColor = AttribsIn.couleur;
}
