#version 410

uniform vec3 bDim, positionPuits;
uniform float temps, dt, tempsMax, gravite;

in vec3 position;
in vec3 vitesse;
in vec4 couleur;
in float tempsRestant;

out vec3 positionMod;
out vec3 vitesseMod;
out vec4 couleurMod;
out float tempsRestantMod;

uint randhash( uint seed ) // entre  0 et UINT_MAX
{
   uint i=(seed^12345391u)*2654435769u;
   i ^= (i<<6u)^(i>>26u);
   i *= 2654435769u;
   i += (i<<5u)^(i>>12u);
   return i;
}
float myrandom( uint seed ) // entre  0 et 1
{
   const float UINT_MAX = 4294967295.0;
   return float(randhash(seed)) / UINT_MAX;
}

void main( void )
{
   if ( tempsRestant <= 0.0 )
   {
      // se préparer à produire une valeur un peu aléatoire
      uint seed = uint(temps * 1000.0) + uint(gl_VertexID);
      // faire renaitre la particule au puits
      positionMod = positionPuits;
      vitesseMod = vec3( myrandom(seed++)-0.5,       // entre -0.5 et 0.5
                         myrandom(seed++)-0.5,       // entre -0.5 et 0.5
                         myrandom(seed++)*0.5+0.5 ); // entre  0.5 et 1
      //vitesseMod = vec3( -0.8, 0., 0.6 );
      const float COULMIN = 0.2; // valeur minimale d'une composante de couleur lorsque la particule (re)naît
      const float COULMAX = 0.9; // valeur maximale d'une composante de couleur lorsque la particule (re)naît

      // nouveau temps de vie
      tempsRestantMod = myrandom(seed++)*tempsMax; // entre 0 et tempsMax secondes

      // interpolation linéaire entre COULMIN et COULMAX
      float red = myrandom(seed++)*(COULMAX-COULMIN)+COULMIN;
      float blue = myrandom(seed++)*(COULMAX-COULMIN)+COULMIN;
      float green = myrandom(seed++)*(COULMAX-COULMIN)+COULMIN;
      couleurMod = vec4(red, blue, green, 1.0f);
   }
   else
   {
      // avancer la particule
      positionMod = position + vitesse*dt;
      vitesseMod = vitesse;

      // diminuer son temps de vie
      tempsRestantMod -= dt;

      // garder la couleur courante
      couleurMod = couleur;

      // collision avec la demi-sphère ?
      vec3 posSphUnitaire = positionMod / bDim;
      vec3 vitSphUnitaire = vitesseMod * bDim;
      float  dist = length( posSphUnitaire  );
      if ( dist  >= 1.0 ) // ... la  particule  est  sortie  de la  bulle
      {
          positionMod = ( 2.0 - dist ) * positionMod;
          vec3 N = posSphUnitaire / dist; //  normaliser N
          vec3  vitReflechieSphUnitaire = reflect( vitSphUnitaire , N );
          vitesseMod = vitReflechieSphUnitaire / bDim;
      }
      // collision avec le sol ?
      if(posSphUnitaire.z <= 0.0){
          vec3  vitReflechieSphUnitaire = reflect( vitSphUnitaire , vec3(0.0,0.0,1.0) );
          vitesseMod = vitReflechieSphUnitaire / bDim;
      }

      // appliquer la gravité
      vitesseMod.z -= gravite*dt;
   }
}
