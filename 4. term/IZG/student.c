/******************************************************************************
 * Projekt - Zaklady pocitacove grafiky - IZG
 * spanel@fit.vutbr.cz
 *
 * $Id:xvales03
 */

#include "student.h"
#include "transform.h"
#include "fragment.h"

#include <memory.h>
#include <math.h>


/*****************************************************************************
 * Globalni promenne a konstanty
 */

/* Typ/ID rendereru (nemenit) */
const int           STUDENT_RENDERER = 1;

float frame_counter_from_onTimer = 0.0;

/*
const S_Material    MAT_WHITE_AMBIENT = { 0.8, 0.8, 0.8, 1.0 };
const S_Material    MAT_WHITE_DIFFUSE = { 0.8, 0.8, 0.8, 1.0 };
const S_Material    MAT_WHITE_SPECULAR = { 0.8, 0.8, 0.8, 1.0 };
*/

const S_Material    MAT_WHITE_AMBIENT = { 1.0, 1.0, 1.0, 1.0 };
const S_Material    MAT_WHITE_DIFFUSE = { 1.0, 1.0, 1.0, 1.0 };
const S_Material    MAT_WHITE_SPECULAR = { 1.0, 1.0, 1.0, 1.0 };


/*****************************************************************************
 * Funkce vytvori vas renderer a nainicializuje jej
 */

S_Renderer * studrenCreate()
{
    S_StudentRenderer * renderer = (S_StudentRenderer *)malloc(sizeof(S_StudentRenderer));
    IZG_CHECK(renderer, "Cannot allocate enough memory");

    /* inicializace default rendereru */
    renderer->base.type = STUDENT_RENDERER;
    renInit(&renderer->base);

    /* nastaveni ukazatelu na upravene funkce */
    /* napr. renderer->base.releaseFunc = studrenRelease; */

	renderer->base.releaseFunc = studrenRelease;
	renderer->base.projectTriangleFunc = studrenProjectTriangle;

    /* inicializace nove pridanych casti */
    renderer->active_texture = loadBitmap(TEXTURE_FILENAME, &(renderer->w), &(renderer->h));

    return (S_Renderer *)renderer;
}

/*****************************************************************************
 * Funkce korektne zrusi renderer a uvolni pamet
 */

void studrenRelease(S_Renderer **ppRenderer)
{
    S_StudentRenderer * renderer;

    if( ppRenderer && *ppRenderer )
    {
        /* ukazatel na studentsky renderer */
        renderer = (S_StudentRenderer *)(*ppRenderer);

        /* pripadne uvolneni pameti */
		free(renderer->active_texture);
        
        /* fce default rendereru */
        renRelease(ppRenderer);
    }
}

/******************************************************************************
 * Nova fce pro rasterizaci trojuhelniku s podporou texturovani
 * Upravte tak, aby se trojuhelnik kreslil s texturami
 * (doplnte i potrebne parametry funkce - texturovaci souradnice, ...)
 * v1, v2, v3 - ukazatele na vrcholy trojuhelniku ve 3D pred projekci
 * n1, n2, n3 - ukazatele na normaly ve vrcholech ve 3D pred projekci
 * x1, y1, ... - vrcholy trojuhelniku po projekci do roviny obrazovky
 */

void studrenDrawTriangle(S_Renderer *pRenderer,
                         S_Coords *v1, S_Coords *v2, S_Coords *v3,
                         S_Coords *n1, S_Coords *n2, S_Coords *n3,
                         int x1, int y1,
                         int x2, int y2,
                         int x3, int y3,
						 S_Coords  *t,
						 double h1, double h2, double h3
						 )
{
    int         minx, miny, maxx, maxy;
    int         a1, a2, a3, b1, b2, b3, c1, c2, c3;
    int         s1, s2, s3;
    int         x, y, e1, e2, e3;
    double      alpha, beta, gamma, w1, w2, w3, z;
    S_RGBA      col1, col2, col3, color;

	double interx, intery, interz;
	S_RGBA tcolor;

    IZG_ASSERT(pRenderer && v1 && v2 && v3 && n1 && n2 && n3);

    /* vypocet barev ve vrcholech */
    col1 = pRenderer->calcReflectanceFunc(pRenderer, v1, n1);
    col2 = pRenderer->calcReflectanceFunc(pRenderer, v2, n2);
    col3 = pRenderer->calcReflectanceFunc(pRenderer, v3, n3);

    /* obalka trojuhleniku */
    minx = MIN(x1, MIN(x2, x3));
    maxx = MAX(x1, MAX(x2, x3));
    miny = MIN(y1, MIN(y2, y3));
    maxy = MAX(y1, MAX(y2, y3));

    /* oriznuti podle rozmeru okna */
    miny = MAX(miny, 0);
    maxy = MIN(maxy, pRenderer->frame_h - 1);
    minx = MAX(minx, 0);
    maxx = MIN(maxx, pRenderer->frame_w - 1);

    /* Pineduv alg. rasterizace troj.
       hranova fce je obecna rovnice primky Ax + By + C = 0
       primku prochazejici body (x1, y1) a (x2, y2) urcime jako
       (y1 - y2)x + (x2 - x1)y + x1y2 - x2y1 = 0 */

    /* normala primek - vektor kolmy k vektoru mezi dvema vrcholy, tedy (-dy, dx) */
    a1 = y1 - y2;
    a2 = y2 - y3;
    a3 = y3 - y1;
    b1 = x2 - x1;
    b2 = x3 - x2;
    b3 = x1 - x3;

    /* koeficient C */
    c1 = x1 * y2 - x2 * y1;
    c2 = x2 * y3 - x3 * y2;
    c3 = x3 * y1 - x1 * y3;

    /* vypocet hranove fce (vzdalenost od primky) pro protejsi body */
    s1 = a1 * x3 + b1 * y3 + c1;
    s2 = a2 * x1 + b2 * y1 + c2;
    s3 = a3 * x2 + b3 * y2 + c3;

    if ( !s1 || !s2 || !s3 )
    {
        return;
    }

    /* normalizace, aby vzdalenost od primky byla kladna uvnitr trojuhelniku */
    if( s1 < 0 )
    {
        a1 *= -1;
        b1 *= -1;
        c1 *= -1;
    }
    if( s2 < 0 )
    {
        a2 *= -1;
        b2 *= -1;
        c2 *= -1;
    }
    if( s3 < 0 )
    {
        a3 *= -1;
        b3 *= -1;
        c3 *= -1;
    }

    /* koeficienty pro barycentricke souradnice */
    alpha = 1.0 / ABS(s2);
    beta = 1.0 / ABS(s3);
    gamma = 1.0 / ABS(s1);

    /* vyplnovani... */
    for( y = miny; y <= maxy; ++y )
    {
        /* inicilizace hranove fce v bode (minx, y) */
        e1 = a1 * minx + b1 * y + c1;
        e2 = a2 * minx + b2 * y + c2;
        e3 = a3 * minx + b3 * y + c3;

        for( x = minx; x <= maxx; ++x )
        {
            if( e1 >= 0 && e2 >= 0 && e3 >= 0 )
            {
                /* interpolace pomoci barycentrickych souradnic
                   e1, e2, e3 je aktualni vzdalenost bodu (x, y) od primek */
                w1 = alpha * e2;
                w2 = beta * e3;
                w3 = gamma * e1;

                /* interpolace z-souradnice */
                z = w1 * v1->z + w2 * v2->z + w3 * v3->z;

				interx = (w1 * t[0].x / h1 + w2 * t[1].x / h2 + w3 * t[2].x / h3) / (w1 / h1 + w2 / h2 + w3 / h3);
				intery = (w1 * t[0].y / h1 + w2 * t[1].y / h2 + w3 * t[2].y / h3) / (w1 / h1 + w2 / h2 + w3 / h3);
				//interz = w1 * t[0].z + w2 * t[1].z + w3 * t[2].z;

				tcolor = studrenTextureValue(pRenderer, interx, intery);
               
                /* interpolace barvy */
                color.red = ROUND2BYTE(w1 * col1.red + w2 * col2.red + w3 * col3.red);
                color.green = ROUND2BYTE(w1 * col1.green + w2 * col2.green + w3 * col3.green);
                color.blue = ROUND2BYTE(w1 * col1.blue + w2 * col2.blue + w3 * col3.blue);
                color.alpha = 255;

				color.red = color.red * tcolor.red / 255;
				color.green = color.green * tcolor.green / 255;
				color.blue = color.blue * tcolor.blue / 255;

                /* vykresleni bodu */
                if( z < DEPTH(pRenderer, x, y) )
                {
                    PIXEL(pRenderer, x, y) = color;
                    DEPTH(pRenderer, x, y) = z;
                }
            }

            /* hranova fce o pixel vedle */
            e1 += a1;
            e2 += a2;
            e3 += a3;
        }
    }
}

/******************************************************************************
 * Vykresli i-ty trojuhelnik n-teho klicoveho snimku modelu
 * pomoci nove fce studrenDrawTriangle()
 * Pred vykreslenim aplikuje na vrcholy a normaly trojuhelniku
 * aktualne nastavene transformacni matice!
 * Upravte tak, aby se model vykreslil interpolovane dle parametru n
 * (cela cast n udava klicovy snimek, desetinna cast n parametr interpolace
 * mezi snimkem n a n + 1)
 * i - index trojuhelniku
 * n - index klicoveho snimku (float pro pozdejsi interpolaci mezi snimky)
 */

void studrenProjectTriangle(S_Renderer *pRenderer, S_Model *pModel, int i, float n)
{
    S_Coords    aa, bb, cc;             /* souradnice vrcholu po transformaci */
    S_Coords    naa, nbb, ncc;          /* normaly ve vrcholech po transformaci */
    S_Coords    nn;                     /* normala trojuhelniku po transformaci */
    int         u1, v1, u2, v2, u3, v3; /* souradnice vrcholu po projekci do roviny obrazovky */
    S_Triangle  * triangle;
    int         vertexOffset, normalOffset; /* offset pro vrcholy a normalove vektory trojuhelniku */
    int         i0, i1, i2, in;             /* indexy vrcholu a normaly pro i-ty trojuhelnik n-teho snimku */

	int         vertexOffset_1, normalOffset_1; /* +1 */
    int         i0_1, i1_1, i2_1, in_1;         /* +1 */

	S_Coords	i0c, i1c, i2c, inc;
	S_Coords	i0c_1, i1c_1, i2c_1, inc_1;

	double h1, h2, h3;

	float t = n - (int) n;

    IZG_ASSERT(pRenderer && pModel && i >= 0 && i < trivecSize(pModel->triangles) && n >= 0 );

    /* z modelu si vytahneme i-ty trojuhelnik */
    triangle = trivecGetPtr(pModel->triangles, i);

    /* ziskame offset pro vrcholy n-teho snimku */
    vertexOffset = (((int) n) % pModel->frames) * pModel->verticesPerFrame;

    /* ziskame offset pro normaly trojuhelniku n-teho snimku */
    normalOffset = (((int) n) % pModel->frames) * pModel->triangles->size;

	/* ziskame offset pro vrcholy 1+n-teho snimku */
	vertexOffset_1 = ((1 + (int) n) % pModel->frames) * pModel->verticesPerFrame;

	/* ziskame offset pro normaly trojuhelniku 1+n-teho snimku */
	normalOffset_1 = ((1 + (int) n) % pModel->frames) * pModel->triangles->size;

    /* indexy vrcholu pro i-ty trojuhelnik n-teho snimku - pricteni offsetu */
    i0 = triangle->v[ 0 ] + vertexOffset;
    i1 = triangle->v[ 1 ] + vertexOffset;
    i2 = triangle->v[ 2 ] + vertexOffset;

	/* indexy vrcholu pro i-ty trojuhelnik 1+n-teho snimku - pricteni offsetu */
	i0_1 = triangle->v[0] + vertexOffset_1;
	i1_1 = triangle->v[1] + vertexOffset_1;
	i2_1 = triangle->v[2] + vertexOffset_1;

    /* index normaloveho vektoru pro i-ty trojuhelnik n-teho snimku - pricteni offsetu */
    in = triangle->n + normalOffset;
	in_1 = triangle->n + normalOffset_1;


	/* interpolace vrcholu */
	i0c = *(cvecGetPtr(pModel->vertices, i0));
	i1c = *(cvecGetPtr(pModel->vertices, i1));
	i2c = *(cvecGetPtr(pModel->vertices, i2));

	i0c_1 = *(cvecGetPtr(pModel->vertices, i0_1));
	i1c_1 = *(cvecGetPtr(pModel->vertices, i1_1));
	i2c_1 = *(cvecGetPtr(pModel->vertices, i2_1));

	i0c.x = ((1 - t) * i0c.x) + (t * i0c_1.x);
	i0c.y = ((1 - t) * i0c.y) + (t * i0c_1.y);
	i0c.z = ((1 - t) * i0c.z) + (t * i0c_1.z);

	i1c.x = ((1 - t) * i1c.x) + (t * i1c_1.x);
	i1c.y = ((1 - t) * i1c.y) + (t * i1c_1.y);
	i1c.z = ((1 - t) * i1c.z) + (t * i1c_1.z);

	i2c.x = ((1 - t) * i2c.x) + (t * i2c_1.x);
	i2c.y = ((1 - t) * i2c.y) + (t * i2c_1.y);
	i2c.z = ((1 - t) * i2c.z) + (t * i2c_1.z);

    /* transformace vrcholu matici model */
    trTransformVertex(&aa, &i0c);
    trTransformVertex(&bb, &i1c);
    trTransformVertex(&cc, &i2c);

    /* promitneme vrcholy trojuhelniku na obrazovku */
    h1 = trProjectVertex(&u1, &v1, &aa);
    h2 = trProjectVertex(&u2, &v2, &bb);
    h3 = trProjectVertex(&u3, &v3, &cc);


	/* interpolace normal */
	i0c = *(cvecGetPtr(pModel->normals, i0));
	i1c = *(cvecGetPtr(pModel->normals, i1));
	i2c = *(cvecGetPtr(pModel->normals, i2));

	i0c_1 = *(cvecGetPtr(pModel->normals, i0_1));
	i1c_1 = *(cvecGetPtr(pModel->normals, i1_1));
	i2c_1 = *(cvecGetPtr(pModel->normals, i2_1));

	i0c.x = ((1 - t) * i0c.x) + (t * i0c_1.x);
	i0c.y = ((1 - t) * i0c.y) + (t * i0c_1.y);
	i0c.z = ((1 - t) * i0c.z) + (t * i0c_1.z);

	i1c.x = ((1 - t) * i1c.x) + (t * i1c_1.x);
	i1c.y = ((1 - t) * i1c.y) + (t * i1c_1.y);
	i1c.z = ((1 - t) * i1c.z) + (t * i1c_1.z);

	i2c.x = ((1 - t) * i2c.x) + (t * i2c_1.x);
	i2c.y = ((1 - t) * i2c.y) + (t * i2c_1.y);
	i2c.z = ((1 - t) * i2c.z) + (t * i2c_1.z);

    /* pro osvetlovaci model transformujeme take normaly ve vrcholech */
    trTransformVector(&naa, &i0c);
    trTransformVector(&nbb, &i1c);
    trTransformVector(&ncc, &i2c);

    /* normalizace normal */
    coordsNormalize(&naa);
    coordsNormalize(&nbb);
    coordsNormalize(&ncc);


	/* interpolace trinormaly */
	inc = *(cvecGetPtr(pModel->trinormals, in));
	inc_1 = *(cvecGetPtr(pModel->trinormals, in_1));

	inc.x = ((1 - t) * inc.x) + (t * inc_1.x);
	inc.y = ((1 - t) * inc.y) + (t * inc_1.y);
	inc.z = ((1 - t) * inc.z) + (t * inc_1.z);

    /* transformace normaly trojuhelniku matici model */
    trTransformVector(&nn, &inc);
    
    /* normalizace normaly */
    coordsNormalize(&nn);

    /* je troj. privraceny ke kamere, tudiz viditelny? */
    if( !renCalcVisibility(pRenderer, &aa, &nn) )
    {
        /* odvracene troj. vubec nekreslime */
        return;
    }

    /* rasterizace trojuhelniku */
    studrenDrawTriangle(pRenderer,
                    &aa, &bb, &cc,
                    &naa, &nbb, &ncc,
                    u1, v1, u2, v2, u3, v3,
					triangle->t,
					h1, h2, h3
                    );
}

/******************************************************************************
* Vraci hodnotu v aktualne nastavene texture na zadanych
* texturovacich souradnicich u, v
* Pro urceni hodnoty pouziva bilinearni interpolaci
* Pro otestovani vraci ve vychozim stavu barevnou sachovnici dle uv souradnic
* u, v - texturovaci souradnice v intervalu 0..1, ktery odpovida sirce/vysce textury
*/

S_RGBA studrenTextureValue( S_StudentRenderer * pRenderer, double u, double v )
{
	S_RGBA p1, p2, p3, p4;
	double posw, posh;
	int w, h;

	/* prevedeni pozice y intervalu 0..1 na 0..sirka/vyska */
	posw = u * pRenderer->w;
	posh = v * pRenderer->h;
	
	/* urceni, ktere 4 pixely se budou pouzivat */
	if (posw - (int)posw < 0.5)
		posw -= 1;
	if (posh - (int)posh < 0.5)
		posh -= 1;
	
	/* prevedeni na int */
	w = posw;
	h = posh;

	/* prepocitani desetinne casti pro interpolaci */
	posw = posw - w;
	posh = posh - h;

	if (posw > 0.5)
		posw -= 0.5;
	else
		posw += 0.5;

	if (posh > 0.5)
		posh -= 0.5;
	else
		posh += 0.5;

	/* precteni 4 sousednich textur */
	p1 = pRenderer->active_texture[w * pRenderer->h + h];
	p2 = pRenderer->active_texture[w * pRenderer->h + h + 1];
	p3 = pRenderer->active_texture[(w + 1) * pRenderer->h + h];
	p4 = pRenderer->active_texture[(w + 1) * pRenderer->h + h + 1];

	/* interpolace, nejprve na radku, pak ve sloupci */
	p1.red = (1 - posh) * p1.red + p2.red * posh;
	p1.green = (1 - posh) * p1.green + p2.green * posh;
	p1.blue = (1 - posh) * p1.blue + p2.blue * posh;

	p3.red = (1 - posh) * p3.red + p4.red * posh;
	p3.green = (1 - posh) * p3.green + p4.green * posh;
	p3.blue = (1 - posh) * p3.blue + p4.blue * posh;

	p1.red = (1 - posw) * p1.red + p3.red * posw;
	p1.green = (1 - posw) * p1.green + p3.green * posw;
	p1.blue = (1 - posw) * p1.blue + p3.blue * posw;
	return p1;

    unsigned char c = ROUND2BYTE( ( ( fmod( u * 10.0, 1.0 ) > 0.5 ) ^ ( fmod( v * 10.0, 1.0 ) < 0.5 ) ) * 255 );
    return makeColor( c, 255 - c, 0 );
}

/******************************************************************************
 ******************************************************************************
 * Funkce pro vyrenderovani sceny, tj. vykresleni modelu
 * Upravte tak, aby se model vykreslil animovane
 * (volani renderModel s aktualizovanym parametrem n)
 */

void renderStudentScene(S_Renderer *pRenderer, S_Model *pModel)
{
    /* test existence frame bufferu a modelu */
    IZG_ASSERT(pModel && pRenderer);

    /* nastavit projekcni matici */
    trProjectionPerspective(pRenderer->camera_dist, pRenderer->frame_w, pRenderer->frame_h);

    /* vycistit model matici */
    trLoadIdentity();

    /* nejprve nastavime posuv cele sceny od/ke kamere */
    trTranslate(0.0, 0.0, pRenderer->scene_move_z);

    /* nejprve nastavime posuv cele sceny v rovine XY */
    trTranslate(pRenderer->scene_move_x, pRenderer->scene_move_y, 0.0);

    /* natoceni cele sceny - jen ve dvou smerech - mys je jen 2D... :( */
    trRotateX(pRenderer->scene_rot_x);
    trRotateY(pRenderer->scene_rot_y);

    /* nastavime material */
    renMatAmbient(pRenderer, &MAT_WHITE_AMBIENT);
    renMatDiffuse(pRenderer, &MAT_WHITE_DIFFUSE);
    renMatSpecular(pRenderer, &MAT_WHITE_SPECULAR);

    /* a vykreslime nas model (ve vychozim stavu kreslime pouze snimek 0) */
    renderModel(pRenderer, pModel, frame_counter_from_onTimer);
}

/* Callback funkce volana pri tiknuti casovace
 * ticks - pocet milisekund od inicializace */
void onTimer( int ticks )
{
    /* uprava parametru pouzivaneho pro vyber klicoveho snimku
     * a pro interpolaci mezi snimky */
    frame_counter_from_onTimer = (float)ticks / 100.0;
}

/*****************************************************************************
 *****************************************************************************/
