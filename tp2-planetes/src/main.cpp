// Prénoms, noms et matricule des membres de l'équipe:
// - Prénom1 NOM1 (matricule1)
// - Prénom2 NOM2 (matricule2)
#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <stdlib.h>
#include <iostream>
#include "inf2705.h"
#include <glm/gtx/io.hpp>

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint locColor = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locplanCoupe = -1;
GLint loccoulProfondeur = -1;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

// matrices de du pipeline graphique
MatricePipeline matrModel;
MatricePipeline matrVisu;
MatricePipeline matrProj;

// les formes
FormeCube *cube = NULL;
FormeSphere *sphere = NULL;
FormeTheiere *theiere = NULL;
FormeTore *toreTerre = NULL;
FormeTore *toreMars = NULL;
FormeTore *toreJupiter = NULL;
GLuint vao = 0;
GLuint vbo[2] = {0,0};

// variables pour définir le point de vue
double thetaCam = 0.0;        // angle de rotation de la caméra (coord. sphériques)
double phiCam = 0.0;          // angle de rotation de la caméra (coord. sphériques)
double distCam = 0.0;         // distance (coord. sphériques)

// variables d'état
bool modeSelection = false;   // on est en mode sélection?
bool enPerspective = false;   // indique si on est en mode Perspective (true) ou Ortho (false)
bool enmouvement = false;     // le modèle est en mouvement/rotation automatique ou non
bool afficheAxes = true;      // indique si on affiche les axes
bool coulProfondeur = false;  // indique si on veut colorer selon la profondeur
GLenum modePolygone = GL_FILL; // comment afficher les polygones
int dernierX = 0, dernierY = 0;

int modele = 1; // le modèle à afficher (1-sphère, 2-cube, 3-théière).

// partie 1: utiliser un plan de coupe
glm::vec4 planCoupe( 0, 0, 1, 0 ); // équation du plan de coupe
GLfloat angleCoupe = 0.0; // angle (degres) autour de x

// facteur de réchauffement
//float facteurRechauffement = 0.2; // un facteur qui sert à calculer la couleur des pôles (0.0=froid, 1.0=chaud)

class CorpsCeleste
{
	public:
	static int corpsCount;
	float rayon;          // le rayon
	float distance;       // la distance au soleil
	float rotation;       // l'angle actuel de rotation en degrés
	float revolution;     // l'angle actuel de révolution en degrés
	float incrRotation;   // l'incrément à ajouter à chaque appel de la fonction calculerPhysique en degrés
	float incrRevolution; // l'incrément à ajouter à chaque appel de la fonction calculerPhysique en degrés
	glm::vec4 couleur;    // la couleur
	glm::vec4 selCouleur;
	bool estSelectionne;
	CorpsCeleste( float r, float dist, float rot, float rev, float incrRot, float incrRev,
					 glm::vec4 coul=glm::vec4(1.,1.,1.,1.) ) :
	rayon(r), distance(dist),
	rotation(rot), revolution(rev),
	incrRotation(incrRot), incrRevolution(incrRev),
	couleur(coul),
	estSelectionne(false)
	{
	    selCouleur.r = corpsCount++ * 10.0 / 255.0;
    }

	void afficher( )
	{
		// la couleur
		glVertexAttrib4fv( locColor, glm::value_ptr(couleur) );
		if(modeSelection)
		    glVertexAttrib4fv( locColor, glm::value_ptr(selCouleur));

		// le corps céleste
		matrModel.PushMatrix(); {
			matrModel.Rotate( rotation, 0, 0, 1 );
			matrModel.Scale( rayon, rayon, rayon );
			glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
			switch ( modele )
			{
				default:
				case 1:
					sphere->afficher();
					break;
				case 2:
					cube->afficher();
					break;
				case 3:
					matrModel.Translate( 0.0, 0.0, -1.0 );
					glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
					theiere->afficher( );
					break;
			}
		} matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
	}

	void avancerPhysique()
	{
	    if(!estSelectionne){
	        rotation += incrRotation;
            revolution += incrRevolution;
	    }

	}
};

int CorpsCeleste::corpsCount = 0;

CorpsCeleste Soleil(   2.00,  0.0,  5.0,  0.0, 0.05, 0.0,  glm::vec4(1.0, 1.0, 0.0, 0.5) );

CorpsCeleste Terre(    0.35,  8.0, 20.0, 45.0, 2.5,  0.03, glm::vec4(0.5, 0.5, 1.0, 1.0) );
CorpsCeleste Lune(     0.10,  1.0, 20.0, 30.0, 2.5, -0.35, glm::vec4(0.6, 0.6, 0.6, 1.0) );

CorpsCeleste Mars(     0.25, 12.0, 20.0, 30.0, 2.5,  0.03, glm::vec4(0.5, 1.0, 0.5, 1.0) );
CorpsCeleste Phobos(   0.10,  0.7,  5.0, 15.0, 3.5,  1.7,  glm::vec4(0.4, 0.4, 0.8, 1.0) );
CorpsCeleste Deimos(   0.10,  1.4, 10.0,  2.0, 4.0,  0.5,  glm::vec4(0.5, 0.5, 0.1, 1.0) );

CorpsCeleste Jupiter(  0.60, 16.0, 10.0, 20.0, 0.2,  0.02, glm::vec4(1.0, 0.5, 0.5, 1.0) );
CorpsCeleste Europa(   0.10,  1.5,  5.0, 19.0, 3.5,  3.4,  glm::vec4(0.4, 0.4, 0.8, 1.0) );
CorpsCeleste Callisto( 0.15,  2.0, 10.0,  2.0, 4.0,  0.45, glm::vec4(0.5, 0.5, 0.1, 1.0) );

void verifierAngles()
{
	const GLdouble MINPHI = 0.01, MAXPHI = 180.0 - 0.01;
	phiCam = glm::clamp( phiCam, MINPHI, MAXPHI );
}

void calculerPhysique( )
{
	if ( enmouvement )
	{
		// incrémenter rotation[] et revolution[] pour faire tourner les planètes
		Soleil.avancerPhysique();
		Terre.avancerPhysique();
		Lune.avancerPhysique();
		Mars.avancerPhysique();
		Phobos.avancerPhysique();
		Deimos.avancerPhysique();
		Jupiter.avancerPhysique();
		Europa.avancerPhysique();
		Callisto.avancerPhysique();
	}
}

void chargerNuanceurs()
{
	// charger le nuanceur de base
	{
		// créer le programme
		progBase = glCreateProgram();

		// attacher le nuanceur de sommets
		{
			GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
			glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesSommetsMinimal, NULL );
			glCompileShader( nuanceurObj );
			glAttachShader( progBase, nuanceurObj );
			ProgNuanceur::afficherLogCompile( nuanceurObj );
		}
		// attacher le nuanceur de fragments
		{
			GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
			glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesFragmentsMinimal, NULL );
			glCompileShader( nuanceurObj );
			glAttachShader( progBase, nuanceurObj );
			ProgNuanceur::afficherLogCompile( nuanceurObj );
		}

		// faire l'édition des liens du programme
		glLinkProgram( progBase );

		ProgNuanceur::afficherLogLink( progBase );
		// demander la "Location" des variables
		if ( ( locVertexBase = glGetAttribLocation( progBase, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
		if ( ( locColorBase = glGetAttribLocation( progBase, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
		if ( ( locmatrModelBase = glGetUniformLocation( progBase, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
		if ( ( locmatrVisuBase = glGetUniformLocation( progBase, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
		if ( ( locmatrProjBase = glGetUniformLocation( progBase, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
	}

	{
		// charger le nuanceur de ce TP

		// créer le programme
		prog = glCreateProgram();

		// attacher le nuanceur de sommets
		const GLchar *chainesSommets = ProgNuanceur::lireNuanceur( "nuanceurSommets.glsl" );
		if ( chainesSommets != NULL )
		{
			GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
			glShaderSource( nuanceurObj, 1, &chainesSommets, NULL );
			glCompileShader( nuanceurObj );
			glAttachShader( prog, nuanceurObj );
			ProgNuanceur::afficherLogCompile( nuanceurObj );
			delete [] chainesSommets;
		}

		// attacher le nuanceur de geometrie
        const GLchar *chainesGeometrie = ProgNuanceur::lireNuanceur( "nuanceurGeometrie.glsl" );
        if ( chainesGeometrie != NULL )
        {
            GLuint nuanceurObj = glCreateShader( GL_GEOMETRY_SHADER );
            glShaderSource( nuanceurObj, 1, &chainesGeometrie, NULL );
            glCompileShader( nuanceurObj );
            glAttachShader( prog, nuanceurObj );
            ProgNuanceur::afficherLogCompile( nuanceurObj );
            delete [] chainesGeometrie;
        }

		// attacher le nuanceur de fragments
		const GLchar *chainesFragments = ProgNuanceur::lireNuanceur( "nuanceurFragments.glsl" );
		if ( chainesFragments != NULL )
		{
			GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
			glShaderSource( nuanceurObj, 1, &chainesFragments, NULL );
			glCompileShader( nuanceurObj );
			glAttachShader( prog, nuanceurObj );
			ProgNuanceur::afficherLogCompile( nuanceurObj );
			delete [] chainesFragments;
		}

		// faire l'édition des liens du programme
		glLinkProgram( prog );

		ProgNuanceur::afficherLogLink( prog );
		// demander la "Location" des variables
		if ( ( locVertex = glGetAttribLocation( prog, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
		if ( ( locColor = glGetAttribLocation( prog, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
		if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
		if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
		if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
		if ( ( locplanCoupe = glGetUniformLocation( prog, "planCoupe" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de planCoupe" << std::endl;
		if ( ( loccoulProfondeur = glGetUniformLocation( prog, "coulProfondeur" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de coulProfondeur" << std::endl;
	}
}

void initialiser()
{
	enmouvement = true;

	thetaCam = 60.0;
	phiCam = 75.0;
	distCam = 26.0;

	// donner la couleur de fond
	glClearColor( 0.1, 0.1, 0.1, 1.0 );

	// activer les etats openGL
	glEnable( GL_DEPTH_TEST );

	// activer le mélange de couleur pour la transparence
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// charger les nuanceurs
	chargerNuanceurs();
	glUseProgram( prog );

	// les valeurs à utiliser pour tracer le quad
	const GLfloat taille = Jupiter.distance + Callisto.distance + Callisto.rayon;
	GLfloat coo[] = { -taille,  taille, 0,
						  taille,  taille, 0,
						  taille, -taille, 0,
						  -taille, -taille, 0 };
	const GLuint connec[] = { 0, 1, 2, 2, 3, 0 };

	// initialiser le VAO (quad)
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);

	glBindVertexArray(vao);
	// créer les deux VBO pour les sommets et la connectivité
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coo), coo, GL_STATIC_DRAW);
	glVertexAttribPointer(locVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(locVertex);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(connec), connec, GL_STATIC_DRAW);
	glBindVertexArray(0);

	// Défaire tous les liens
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// créer quelques autres formes
	cube = new FormeCube( 3.0 );
	sphere = new FormeSphere( 2.0, 16, 16 );
	theiere = new FormeTheiere( );
	toreTerre = new FormeTore( 0.1, Terre.distance, 4, 64 );
	toreMars = new FormeTore( 0.1, Mars.distance, 4, 64 );
	toreJupiter = new FormeTore( 0.1, Jupiter.distance, 4, 64 );
}

void conclure()
{
	delete cube;
	delete sphere;
	delete theiere;
	delete toreTerre;
	delete toreMars;
	delete toreJupiter;
	glDeleteBuffers( 2, vbo );
	glDeleteVertexArrays( 1, &vao );
}

void definirCamera()
{
	// La souris influence le point de vue
	matrVisu.LookAt( distCam*cos(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
						 distCam*sin(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
						 distCam*cos(glm::radians(phiCam)),
						 0, 0, 0,
						 0, 0, 1 );

	// (pour apprentissage supplémentaire): modifs ici ...
	// (pour apprentissage supplémentaire): La caméra est sur la Terre et voir passer les autres objets célestes en utilisant l'inverse de la matrice mm
}

void afficherQuad( GLfloat alpha ) // le plan qui ferme les solides
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glVertexAttrib4f( locColor, 1.0, 1.0, 1.0, alpha );
    // afficher le plan tourné selon l'angle courant et à la position courante
    // partie 1: modifs ici ...

    matrModel.PushMatrix();{
        matrModel.Rotate(angleCoupe, 0.0, 1.0, 0.0);
        matrModel.Translate(0.0, 0.0, -planCoupe.w);
        glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
        glBindVertexArray( vao );
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
        glBindVertexArray(0);
    }matrModel.PopMatrix();
}

void afficherModele()
{
	glVertexAttrib4f( locColor, 1.0, 1.0, 1.0, 1.0 );

	// afficher le système solaire
	matrModel.PushMatrix( ); {

		// Terre
		matrModel.PushMatrix( ); {
			matrModel.Rotate( Terre.revolution, 0, 0, 1 ); // révolution terre autour soleil
			matrModel.Translate( Terre.distance, 0, 0 );
			Terre.afficher( );
			// Lune
			matrModel.PushMatrix( ); {
				matrModel.Rotate( Lune.revolution, 0, 0, 1 ); // révolution lune autour terre
				matrModel.Translate( Lune.distance, 0, 0 );
				Lune.afficher( );
			} matrModel.PopMatrix();
		} matrModel.PopMatrix();

		// Mars
		matrModel.PushMatrix( ); {
			matrModel.Rotate( Mars.revolution, 0, 0, 1 ); // révolution mars autour soleil
			matrModel.Translate( Mars.distance, 0, 0 );
			Mars.afficher( );
			// Phobos
			matrModel.PushMatrix( ); {
				matrModel.Rotate( Phobos.revolution, 0, 0, 1 ); // révolution phobos autour mars
				matrModel.Translate( Phobos.distance, 0, 0 );
				Phobos.afficher( );
			} matrModel.PopMatrix();
			// Deimos
			matrModel.PushMatrix( ); {
				matrModel.Rotate( Deimos.revolution, 0, 0, 1 ); // révolution deimos autour mars
				matrModel.Translate( Deimos.distance, 0, 0 );
				Deimos.afficher( );
			} matrModel.PopMatrix();
		} matrModel.PopMatrix();

		// Jupiter
		matrModel.PushMatrix( ); {
			matrModel.Rotate( Jupiter.revolution, 0, 0, 1 ); // révolution jupiter autour soleil
			matrModel.Translate( Jupiter.distance, 0, 0 );
			Jupiter.afficher( );
			// Europa
			matrModel.PushMatrix( ); {
				matrModel.Rotate( Europa.revolution, 0, 0, 1 ); // révolution europa autour jupiter
				matrModel.Translate( Europa.distance, 0, 0 );
				Europa.afficher( );
			} matrModel.PopMatrix();
			// Callisto
			matrModel.PushMatrix( ); {
				matrModel.Rotate( Callisto.revolution, 0, 0, 1 ); // révolution callisto autour jupiter
				matrModel.Translate( Callisto.distance, 0, 0 );
				Callisto.afficher( );
			} matrModel.PopMatrix();
		} matrModel.PopMatrix();

	} matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );

	// afficher les deux tores pour identifier les orbites des planetes
	glVertexAttrib3f( locColor, 0.0, 0.0, 1.0 );
	toreTerre->afficher();
	glVertexAttrib3f( locColor, 0.0, 1.0, 0.0 );
	toreMars->afficher();
	glVertexAttrib3f( locColor, 1.0, 0.0, 0.0 );
	toreJupiter->afficher();

	// afficher le soleil, un peu transparent
	// Soleil
	glEnable( GL_BLEND );
	glDepthMask( GL_FALSE );
	Soleil.afficher( );
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

bool compareColor(GLubyte *couleur, glm::vec4 couleurObj){
    //if((int)couleur[0] == (int) (couleurObj[0]*255) && (int)couleur[1] == (int) (couleurObj[1]*255) && (int)couleur[2] == (int) (couleurObj[2]*255)){
    if((int)couleur[0] == (int) (couleurObj[0]*255)){
        return true;
    }
    else{
        return false;
    }
}

void FenetreTP::afficherScene( )
{
	// effacer l'ecran et le tampon de profondeur et le stencil
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glUseProgram( progBase );

	// définir le pipeline graphique
	matrProj.Perspective( 70.0, (GLdouble) largeur_ / (GLdouble) hauteur_, 0.1, 100.0 );
	glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

	definirCamera( );
	glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

	matrModel.LoadIdentity();
	glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

	// afficher les axes
	if ( afficheAxes ) FenetreTP::afficherAxes();

	// dessiner la scène
	glUseProgram( prog );
	glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
	glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
	glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
	glUniform4fv( locplanCoupe, 1, glm::value_ptr(planCoupe) );
	glUniform1i( loccoulProfondeur, coulProfondeur );

	// afficher le modèle et tenir compte du stencil et du plan de coupe
	// partie 1: modifs ici ...

	glEnable(GL_CLIP_PLANE0);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);

	glStencilOp(GL_ZERO, GL_INCR, GL_INCR );
	afficherModele();

	//glDisable( GL_STENCIL_TEST );
	glDisable(GL_CLIP_PLANE0);

	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
//    if(!modeSelection)
//    {
        afficherQuad( 1 );
//    }
    glDisable( GL_STENCIL_TEST );

    // en plus, dessiner le plan en transparence pour bien voir son étendue

//    if(!modeSelection)
//    {
        afficherQuad( 0.25 );
//	}

	if ( !modeSelection ) { glDisable( GL_BLEND ); glDepthMask( GL_TRUE ); }

	if ( modeSelection )
	{
		// s'assurer que toutes les opérations sont terminées
		glFinish();

		// obtenir la clôture et calculer la position demandée
		GLint cloture[4]; glGetIntegerv( GL_VIEWPORT, cloture );
		GLint posX = dernierX, posY = 2*cloture[3]-dernierY;

		// dire de lire le tampon arrière où l'on vient tout juste de dessiner
		glReadBuffer( GL_BACK );

		// obtenir la couleur
		GLubyte couleur[3];
		glReadPixels( posX, posY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, couleur );
		std::cout << "couleur = " << (int) couleur[0] << " " << (int) couleur[1] << " " << (int) couleur[2] << std::endl;

		// obtenir la profondeur (accessoirement)
		GLfloat profondeur;
		glReadPixels( posX, posY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &profondeur );
		std::cout << "profondeur = " << profondeur << std::endl;


        std::cout << (int) (Terre.couleur[0]*255) << std::endl;
		//la couleur lu indique l'objet sélectionné
		if(compareColor(couleur, Terre.selCouleur)){
		    Terre.estSelectionne = !Terre.estSelectionne;
			std::cout << "Terre sélectionné" << std::endl;
		}
		else if(compareColor(couleur, Lune.selCouleur)){
            Lune.estSelectionne = !Lune.estSelectionne;
            std::cout << "Lune sélectionné"<< std::endl;
        }
        else if(compareColor(couleur, Mars.selCouleur)){
            Mars.estSelectionne = !Mars.estSelectionne;
            std::cout << "Mars sélectionné"<< std::endl;
        }
        else if(compareColor(couleur, Phobos.selCouleur)){
            Phobos.estSelectionne = !Phobos.estSelectionne;
            std::cout << "Phobos sélectionné"<< std::endl;
        }
        else if(compareColor(couleur, Deimos.selCouleur)){
            Deimos.estSelectionne = !Deimos.estSelectionne;
            std::cout << "Deimos sélectionné"<< std::endl;
        }
        else if(compareColor(couleur, Jupiter.selCouleur)){
            Jupiter.estSelectionne = !Jupiter.estSelectionne;
            std::cout << "Jupiter sélectionné"<< std::endl;
        }
        else if(compareColor(couleur, Europa.selCouleur)){
            Europa.estSelectionne = !Europa.estSelectionne;
            std::cout << "Europa sélectionné"<< std::endl;
        }
        else if(compareColor(couleur, Callisto.selCouleur)){
            Callisto.estSelectionne = !Callisto.estSelectionne;
            std::cout << "Callisto sélectionné"<< std::endl;
        }
		else{
			std::cout << "Aucun objet sélectionné"<< std::endl;
		}
	}
}

void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
    GLfloat W = w, H2 = 0.5*h;
    GLfloat v[]  = {
        0, 0,  W, H2,
        0, H2, W, H2,
    };
    glViewportArrayv( 0, 2, v );
//	glViewport( 0, 0, w, h );
}

void FenetreTP::clavier( TP_touche touche )
{
	switch ( touche )
	{
		case TP_ECHAP:
		case TP_q: // Quitter l'application
			quit();
			break;

		case TP_x: // Activer/désactiver l'affichage des axes
			afficheAxes = !afficheAxes;
			std::cout << "// Affichage des axes ? " << ( afficheAxes ? "OUI" : "NON" ) << std::endl;
			break;

		case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
			chargerNuanceurs();
			std::cout << "// Recharger nuanceurs" << std::endl;
			break;

		case TP_ESPACE: // Mettre en pause ou reprendre l'animation
			enmouvement = !enmouvement;
			break;

		case TP_p: // Atténuer ou non la couleur selon la profondeur
			coulProfondeur = !coulProfondeur;
			std::cout << " coulProfondeur=" << coulProfondeur << std::endl;
			break;

		case TP_g: // Permuter l'affichage en fil de fer ou plein
			modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
			glPolygonMode( GL_FRONT_AND_BACK, modePolygone );
			break;

		case TP_PLUS: // Incrémenter la distance de la caméra
		case TP_EGAL:
			distCam--;
			std::cout << " distCam=" << distCam << std::endl;
			break;

		case TP_SOULIGNE:
		case TP_MOINS: // Décrémenter la distance de la caméra
			distCam++;
			std::cout << " distCam=" << distCam << std::endl;
			break;

		case TP_CROCHETDROIT: // Augmenter l'angle du plan de coupe
		case TP_SUPERIEUR:
			angleCoupe += 0.5;
			planCoupe.x = sin(glm::radians(angleCoupe));
			planCoupe.z = cos(glm::radians(angleCoupe));
			std::cout << " angleCoupe=" << angleCoupe << std::endl;
			break;
		case TP_CROCHETGAUCHE: // Diminuer l'angle du plan de coupe
		case TP_INFERIEUR:
			angleCoupe -= 0.5;
			planCoupe.x = sin(glm::radians(angleCoupe));
			planCoupe.z = cos(glm::radians(angleCoupe));
			std::cout << " angleCoupe=" << angleCoupe << std::endl;
			break;

		case TP_m: // Choisir le modèle: 1-sphère, 2-cube, 3-théière (déjà implanté)
			if ( ++modele > 3 ) modele = 1;
			std::cout << " modele=" << modele << std::endl;
			break;

			// case TP_c: // Augmenter le facteur de réchauffement
			//    facteurRechauffement += 0.05; if ( facteurRechauffement > 1.0 ) facteurRechauffement = 1.0;
			//    std::cout << " facteurRechauffement=" << facteurRechauffement << " " << std::endl;
			//    break;
			// case TP_f: // Diminuer le facteur de réchauffement
			//    facteurRechauffement -= 0.05; if ( facteurRechauffement < 0.0 ) facteurRechauffement = 0.0;
			//    std::cout << " facteurRechauffement=" << facteurRechauffement << " " << std::endl;
			//    break;

		default:
			std::cout << " touche inconnue : " << (char) touche << std::endl;
			imprimerTouches();
			break;
	}
}

static bool pressed = false;
void FenetreTP::sourisClic( int button, int state, int x, int y )
{
	pressed = ( state == TP_PRESSE );
	if ( pressed )
	{
		switch ( button )
		{
			default:
			case TP_BOUTON_GAUCHE: // Modifier le point de vue
				modeSelection = false;
				break;
			case TP_BOUTON_DROIT: // Sélectionner des objets
			    GLint cloture[4]; glGetIntegerv( GL_VIEWPORT, cloture );
			    if(y < cloture[3])
				    modeSelection = true;
				break;
		}
		dernierX = x;
		dernierY = y;
	}
	else
	{
		modeSelection = false;
	}
}

void FenetreTP::sourisWheel( int x, int y ) // Déplacer le plan de coupe
{
	const int sens = +1;
	planCoupe.w += 0.02 * sens * y;
	std::cout << " planCoupe.w=" << planCoupe.w << std::endl;
}

void FenetreTP::sourisMouvement( int x, int y )
{
	if ( pressed )
	{
		if ( !modeSelection )
		{
			int dx = x - dernierX;
			int dy = y - dernierY;
			thetaCam -= dx / 3.0;
			phiCam   -= dy / 3.0;
		}

		dernierX = x;
		dernierY = y;

		verifierAngles();
	}
}

int main( int argc, char *argv[] )
{
	// créer une fenêtre
	FenetreTP fenetre( "INF2705 TP" );

	// allouer des ressources et définir le contexte OpenGL
	initialiser();

	bool boucler = true;
	while ( boucler )
	{
		// mettre à jour la physique
		calculerPhysique( );
        // affichage
        fenetre.afficherScene();
        if(modeSelection){
            modeSelection = pressed = false;
        }else{
            fenetre.swap();
        }
//        fenetre.afficherScene();
//        fenetre.swap();


		// récupérer les événements et appeler la fonction de rappel
		boucler = fenetre.gererEvenement();
	}

	// détruire les ressources OpenGL allouées
	conclure();

	return 0;
}
