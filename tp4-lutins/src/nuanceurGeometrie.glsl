#if defined(NUANCEUR_GEOMETRIE_POINTS)
uniform mat4 matrProj;
layout(points) in;
layout(points, max_vertices = 1) out;

in Attribs {
   vec4 couleur;
   float tempsRestant;
   float sens; // du vol
} AttribsIn[];

out Attribs {
   vec4 couleur;
   vec2 texCoord;
} AttribsOut;

void main()
{
   gl_PointSize = 20.0; // en pixels
   gl_PointSize /= (-gl_in[0].gl_Position.z);
   gl_Position = (matrProj * gl_in[0].gl_Position);
   AttribsOut.couleur = AttribsIn[0].couleur;
   if(AttribsIn[0].tempsRestant < 1) AttribsOut.couleur.a += 0.001;
   AttribsOut.texCoord = vec2( 0.5, 0.5 );
   EmitVertex();
}

#elif defined(NUANCEUR_GEOMETRIE)
uniform mat4 matrProj;
uniform int texnumero;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in Attribs {
   vec4 couleur;
   float tempsRestant;
   float sens; // du vol
} AttribsIn[];

out Attribs {
   vec4 couleur;
   vec2 texCoord;
} AttribsOut;

void main()
{
    vec2 coins[4];
    coins[0] = vec2( -0.5, 0.5 );
    coins[1] = vec2( -0.5, -0.5 );
    coins[2] = vec2( 0.5, 0.5 );
    coins[3] = vec2( 0.5, -0.5 );

    vec2 coins_reverse[4];
    coins_reverse[0] = vec2( 0.5, 0.5 );
    coins_reverse[1] = vec2( 0.5, -0.5 );
    coins_reverse[2] = vec2( -0.5, 0.5 );
    coins_reverse[3] = vec2( -0.5, -0.5 );


    if(texnumero != 0)
    {
        for( int i=0; i < 4; i++ )
        {
            float fact = 0.1;
            vec2 decalage = vec2(0.0, 0.0);

            if(texnumero != 1){
                decalage = fact * coins[i];

                if(AttribsIn[0].sens == 1){
                    AttribsOut.texCoord = coins[i] + vec2( 0.5, 0.5 );
                }
                // Si le sens de déplacement est vers les x > 0 on retourne la texture.
                else{
                    AttribsOut.texCoord = coins_reverse[i] + vec2( 0.5, 0.5 );
                }
                AttribsOut.texCoord.x *= 1.0 / 16.0;
                AttribsOut.texCoord.x += int(mod(18.*AttribsIn[0].tempsRestant, 16)) / 16.0;
            }
            else{
                mat3 rot = mat3( cos(6.0*AttribsIn[0].tempsRestant), sin(6.0*AttribsIn[0].tempsRestant), 0.0,
                                 -sin(6.0*AttribsIn[0].tempsRestant), cos(6.0*AttribsIn[0].tempsRestant), 0.0,
                                 0.0, 0.0, 1.0);

                // Rotation des coordonnées de sommets
                decalage = fact * (rot * vec3(coins[i],1.0)).xy;
                AttribsOut.texCoord = coins[i] + vec2( 0.5, 0.5 );

                // Rotation de la texture
//                decalage = fact * coins[i];
//                AttribsOut.texCoord = coins[i];
//                AttribsOut.texCoord = (rot * vec3(AttribsOut.texCoord, 1.0)).xy;
//                AttribsOut.texCoord += vec2(0.5, 0.5);
            }

            vec4 pos = vec4(gl_in[0].gl_Position.xy + decalage, gl_in[0].gl_Position.zw);
            gl_Position = matrProj * pos;
            AttribsOut.couleur = AttribsIn[0].couleur;

            EmitVertex();
        }
    }
}

#endif