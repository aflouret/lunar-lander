#include "fisica.h"
#include "vector.h"
#include "config.h"
#include "naves.h"
#include "diccionario.h"

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define CARACTER_ESCALA 2
#define DT 1.0/JUEGO_FPS
#define PAUSA_MENSAJE 2000


//Asigna a las variables de estado de la nave los valores iniciales definidos en config.h.
void nave_inicializar(double * vx, double * vy, double * x, double * y, double * angulo, double * potencia);

//Asigna a las variables de estado del juego los valores iniciales correspondientes. 
void juego_inicializar(double * combustible, double * tiempo, int * puntaje);

/*
Recibe el vector de coordenadas del chorro de la nave, el angulo actual del chorro y un factor.
Agranda o achica el chorro segun sea el factor positivo o negativo, respectivamente.
*/
void chorro_transformar(float ** chorro_v, float angulo, float factor);


int main() 
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;

	SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
	
	SDL_SetWindowTitle(window, "Lunar Lander");

	int dormir = 0;

	// BEGIN código del alumno

	srand(time(NULL));

	//Inicializo los vectores dinamicos de la nave, del chorro y del terreno

	size_t nave_tam = sizeof(nave_chica) / sizeof(nave_chica[0]);
	float ** nave_v = vector_desde_matriz(nave_chica,nave_tam);
	if(nave_v == NULL)
	{
		fputs("Error de asignacion de memoria.\n", stderr);
		return 1;
	}

	const float chorro[3][2] = 
	{
		{-NAVE_CHICA_TOBERA_X, -NAVE_CHICA_TOBERA_Y},
		{0, -NAVE_CHICA_TOBERA_Y},
		{NAVE_CHICA_TOBERA_X, -NAVE_CHICA_TOBERA_Y}
	};

	size_t chorro_tam = 3;
	float **chorro_v = vector_desde_matriz(chorro, chorro_tam);
	if(chorro_v == NULL)
	{
		fputs("Error de asignacion de memoria.\n", stderr);
		vector_destruir(nave_v, nave_tam, 2);
		return 1;
	}

	size_t terreno_tam = 0;
	float ** terreno_v = crear_terreno(&terreno_tam);
	if(terreno_v == NULL)
	{
		fputs("Error de asignacion de memoria.\n", stderr);
		vector_destruir(nave_v, nave_tam, 2);
		vector_destruir(chorro_v, chorro_tam, 2);
		return 1;
	}

	
	double vx, vy, x, y, angulo, potencia, altitud; //Variables de estado de la nave
	
	double combustible, tiempo;
	int puntaje; //Variables de estado del juego

	
	//Inicializo las variables de estado

	nave_inicializar(&vx, &vy, &x, &y, &angulo, &potencia);

	vector_rotar(nave_v, nave_tam, angulo); //Roto la nave y el chorro para que coincidan con la variable de estado angulo
	vector_rotar(chorro_v, chorro_tam, angulo);

	juego_inicializar(&combustible, &tiempo, &puntaje);

	// END código del alumno

	unsigned int ticks = SDL_GetTicks();

	while(1) 
	{
		if(SDL_PollEvent(&event)) 
		{
			if (event.type == SDL_QUIT)
				break;
	    	if (event.type == SDL_KEYDOWN) 
	    	{
				// BEGIN código del alumno
				switch(event.key.keysym.sym) 
				{
					case SDLK_UP:
						if(potencia < NAVE_MAX_POTENCIA)
						{
							chorro_transformar(chorro_v, angulo, 1);

							potencia += 1;
						}
						break;

					case SDLK_DOWN:
						if(potencia > NAVE_POTENCIA_INICIAL)
						{
							chorro_transformar(chorro_v, angulo, -1);

							potencia -= 1;
						}
						break;

					case SDLK_RIGHT:
						if (angulo > -PI/2)
						{
							vector_rotar(nave_v, nave_tam, -NAVE_ROTACION_PASO);
							vector_rotar(chorro_v, chorro_tam, -NAVE_ROTACION_PASO);

							angulo -= NAVE_ROTACION_PASO;
							combustible -= NAVE_ROTACION_PASO;
						}	
						break;

					case SDLK_LEFT:
						if (angulo < PI/2)
						{	
							vector_rotar(nave_v, nave_tam, NAVE_ROTACION_PASO);
							vector_rotar(chorro_v, chorro_tam, NAVE_ROTACION_PASO);

							angulo += NAVE_ROTACION_PASO;
							combustible -= NAVE_ROTACION_PASO;
						}
						break;
				}
				// END código del alumno	
			}
			continue;
		}
        	
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
       	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);


		// BEGIN código del alumno

		//Actualizo variables de estado

		vy=computar_velocidad(vy,-G+potencia*cos(angulo),DT);
		vx=computar_velocidad(vx,-potencia*sin(angulo),DT);		
		y=computar_posicion(y,vy,DT);
		x=computar_posicion(x,vx,DT);
		if(x >= VENTANA_ANCHO)
			x = 0;		
		else if(x < 0)
			x = VENTANA_ANCHO;

		altitud = y-NAVE_CHICA_TOBERA_Y-vector_interpolar(terreno_v, terreno_tam, x);

		combustible -= potencia * JUEGO_COMBUSTIBLE_POT_X_SEG / JUEGO_FPS;
		if(combustible <= 0)
			break; //Termina el juego

		tiempo += DT;

		
		//Grafico nave, chorro, terreno y caracteres

		vector_graficar(nave_v, nave_tam, x, VENTANA_ALTO-y, 1, renderer);
		vector_graficar(chorro_v, chorro_tam, x, VENTANA_ALTO-y, 1, renderer);
		vector_graficar(terreno_v, terreno_tam, 0, VENTANA_ALTO, 1, renderer);

		cadena_graficar("SCORE", 80, VENTANA_ALTO-775, CARACTER_ESCALA, renderer);
		contador_graficar_ceros(puntaje, 4, 170, VENTANA_ALTO-775, CARACTER_ESCALA, renderer);
				
		cadena_graficar("TIME", 80, VENTANA_ALTO-750, CARACTER_ESCALA, renderer);
		contador_graficar_ceros((int)tiempo, 4, 170, VENTANA_ALTO-750, CARACTER_ESCALA, renderer);

		cadena_graficar("FUEL", 80, VENTANA_ALTO-725, CARACTER_ESCALA, renderer);
		contador_graficar_ceros((int)combustible, 4, 170, VENTANA_ALTO-725, CARACTER_ESCALA, renderer);

		cadena_graficar("ALTITUDE", 600, VENTANA_ALTO-775, CARACTER_ESCALA, renderer);
		contador_graficar_derecha((int)altitud, 4, 820, VENTANA_ALTO-775, CARACTER_ESCALA, renderer);

		cadena_graficar("HORIZONTAL SPEED", 600, VENTANA_ALTO-750, CARACTER_ESCALA, renderer);
		contador_graficar_derecha((int)vx, 4, 820, VENTANA_ALTO-750, CARACTER_ESCALA, renderer);
		cadena_graficar(vx>=0 ? " >" : " <", 870, VENTANA_ALTO-750, CARACTER_ESCALA, renderer);		
		
		cadena_graficar("VERTICAL SPEED", 600, VENTANA_ALTO-725, CARACTER_ESCALA, renderer);
		contador_graficar_derecha((int)vy, 4, 820, VENTANA_ALTO-725, CARACTER_ESCALA, renderer);
		cadena_graficar(vy>=0 ? " ^" : " v", 870, VENTANA_ALTO-725, CARACTER_ESCALA, renderer);


		//Condicion de finalizacion de partida		
		if(!vector_esta_arriba(terreno_v, terreno_tam, x, y-NAVE_CHICA_TOBERA_Y))
		{
			if(vx<1 && vx>-1 && vy<10 && vy>-10 && angulo<0.01 && angulo>-0.01)
			{
				puntaje += 50;

				cadena_graficar("YOU HAVE LANDED", 350, VENTANA_ALTO-650, 2, renderer);			
			}

			else if(vx<2 && vx>-2 && vy<20 && vy>-20 && angulo<0.01 && angulo>-0.01)
			{
				puntaje += 15;
				
				cadena_graficar("YOU LANDED HARD", 350, VENTANA_ALTO-650, 2, renderer);
			}

			else
			{
				puntaje += 5;
				combustible -= 250;
						
				cadena_graficar("DESTROYED", 450, VENTANA_ALTO-650, 2, renderer);	
			}

			//Inicializacion de nueva partida

			vector_rotar(nave_v, nave_tam, NAVE_ANGULO_INICIAL-angulo);
			vector_rotar(chorro_v, chorro_tam, NAVE_ANGULO_INICIAL-angulo);
			
			chorro_transformar(chorro_v, NAVE_ANGULO_INICIAL, -potencia);

			vector_destruir(terreno_v, terreno_tam, 2);
			terreno_v = crear_terreno(&terreno_tam);
			if(terreno_v == NULL)
			{
				fputs("Error de asignacion de memoria.\n", stderr);
				vector_destruir(nave_v, nave_tam, 2);
				vector_destruir(chorro_v, chorro_tam, 2);
				return 1;
			}

			nave_inicializar(&vx, &vy, &x, &y, &angulo, &potencia);

			dormir=PAUSA_MENSAJE;
		}

		// END código del alumno

        	SDL_RenderPresent(renderer);
		ticks = SDL_GetTicks() - ticks;
		if(dormir) {
			SDL_Delay(dormir);
			dormir = 0;
		}
		else if(ticks < 1000 / JUEGO_FPS)
			SDL_Delay(1000 / JUEGO_FPS - ticks);
		ticks = SDL_GetTicks();
	}

	// BEGIN código del alumno

	//Destruccion de vectores dinamicos
	vector_destruir(nave_v, nave_tam, 2);
	vector_destruir(chorro_v, chorro_tam, 2);
	vector_destruir(terreno_v, terreno_tam, 2);

	//Informa al usuario el puntaje final
	printf("FINAL SCORE: %d\n", puntaje);

	// END código del alumno

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);


	SDL_Quit();
	return 0;
}

void nave_inicializar(double * vx, double * vy, double * x, double * y, double * angulo, double * potencia)
{
	*vx = NAVE_VX_INICIAL;
	*vy = NAVE_VY_INICIAL;
	*x = NAVE_X_INICIAL;
	*y = NAVE_Y_INICIAL;
	*angulo = NAVE_ANGULO_INICIAL;
	*potencia = NAVE_POTENCIA_INICIAL;
}

void juego_inicializar(double * combustible, double * tiempo, int * puntaje)
{
	*combustible = JUEGO_COMBUSTIBLE_INICIAL;
	*tiempo = 0;
	*puntaje = 0;
}

void chorro_transformar(float ** chorro_v, float angulo, float factor)
{
	chorro_v[1][0] = chorro_v[1][0] + factor * sin(angulo);
	chorro_v[1][1] = chorro_v[1][1] - factor * cos(angulo);
}