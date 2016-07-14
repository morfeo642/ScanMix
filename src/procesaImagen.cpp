/**
 * @file
 *	\brief Este programa extrae fotos de una imagen específica y las guarda
 * en distintos ficheros.
 * El programa toma dos parámetros:
 * - La imagen de donde se extraerán las fotos 
 * - El directorio donde se almacenarán las distintas fotos obtenidas por el
 * algoritmo.
 */
 
// Librerías estándar
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sstream>
#include <list>
#include <stack>

// Librerías de UNIX
#include <sys/types.h>
#include <sys/stat.h>

// openCV para el procesamiento de la imagen
#include <opencv2/opencv.hpp> 

// Configuración
#include "config.h"

// Macros
#define PI 3.14159265

// Espacios con nombre
using namespace cv; 

// Métodos auxiliares
int dirExists(const char* path); // Comprueba la existencia de un directorio
std::vector<Mat> procesaImagen(Mat);
std::vector<Rect> procesaAreas(std::vector<Rect> areas);
bool isInside(Rect A, Rect B);
bool isInside(Point2f p, Rect R);

// Rutina principal 
int main(int argc, char** argv)
{
	/** Validamos los parámetros */
	if(argc < 3)
	{
		printf("Error: Sintaxis valida es %s <%s> <%s>\n", argv[0], "imagen", "directorio-fotos");
		return -1;
	}
	
	
	if(!dirExists(argv[2]))
	{
		printf("Error: El directorio \"%s\" no existe\n", argv[2]);
		return -1;
	}
	std::string dirFotos = argv[2];
	std::string rutaImagen = argv[1];
	
	/* Leemos la imagen */
	Mat imagen = imread(rutaImagen, 1);
	if(imagen.data == NULL)
	{
		printf("Error: No se ha podido leer la imagen \"%s\"\n", argv[1]);
		return -1;
	}
	
	
	/* Procesamos la imagen */
	std::vector<Mat> fotos = procesaImagen(imagen);
	std::vector<int> compressionParams;
	compressionParams.push_back(CV_IMWRITE_JPEG_QUALITY);
	compressionParams.push_back(CONF_JPEG_QUALITY);
	
	/* Guardamos las fotos extraídas */
	for(int i = 0; i < fotos.size(); i++)
	{	
		std::stringstream buf;
		buf << dirFotos << "/foto" << (i+1) << ".jpg";
		std::string rutaFoto = buf.str();
		
		imwrite(rutaFoto, fotos[i], compressionParams);
	}
	/* También guardamos la imágen original */
	std::stringstream buf; buf << dirFotos << "/imagen-original.jpg";
	imwrite(buf.str(), imagen, compressionParams);
}

int dirExists(const char *path)
{
	struct stat info;

	if(stat( path, &info ) != 0)
		return 0;
	else if(info.st_mode & S_IFDIR)
		return 1;
	else
		return 0;
}


std::vector<Mat> procesaImagen(Mat originalImage)
{
	std::vector<Mat> fotos;
	RNG rng(12345);
		
	
	// Preprocesamos la imagen.
	// Convertimos la imagen a escala de grises y aplicamos un filtro
	// gaussiano
	Mat BW;

	std::cout << "Convirtiendo imagen a escala de grises..." << std::endl;
	cvtColor(originalImage, BW, CV_BGR2GRAY );
	
#ifdef CONF_USE_GAUSSIAN
	std::cout << "Aplicando un filtro gaussiano a la imagen [kernel_size=" << CONF_GAUSSIAN_KERNEL_SIZE << "]..." << std::endl;
	GaussianBlur(BW, BW, CONF_GAUSSIAN_KERNEL_SIZE, 0, 0);
#endif
	imwrite("filtered.jpg", BW);
	
	Mat E;
#ifdef CONF_USE_CANNY
	// Aplicamos el algoritmo de Canny para detectar bordes
	std::cout << "Usando algoritmo Canny para deteccion de bordes.." << std::endl;
	Canny( BW, E, CONF_CANNY_LOWTHRESHOLD, CONF_CANNY_LOWTHRESHOLD*CONF_CANNY_RATIO, CONF_CANNY_KERNEL_SIZE );
	
#else
	// Aplicamos Laplace para detectar bordes.
	std::cout << "Usando algoritmo de Laplace para deteccion de bordes.." << std::endl;
	Laplacian( BW, E, CV_16S, CONF_LAPLACE_KERNEL_SIZE, 1, 0, BORDER_DEFAULT );
	convertScaleAbs( E, E );
#endif
	
	// Aplicamos convolución y umbralizamos la imagen con los bordes

#ifdef CONF_USE_THRESHOLDING
	threshold( E, E, 16, 255, THRESH_BINARY );

#	ifdef CONF_USE_FILTER
	filter2D(E, E, -1, CONF_FILTER_KERNEL);
#	endif

#endif

	imwrite("bordes.jpg", E);
	
	
	// Obtenemos los contornos de la imagen
	std::cout << "Generando contornos de la imagen..." << std::endl;
    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    findContours(E, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	
	Mat drawing = Mat::zeros( originalImage.size(), CV_8UC3 );
	for(int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( drawing, contours, i, color, 2, 8, vector<Vec4i>(), 0, Point() );
	}
	imwrite("contours.jpg", drawing);
	
	
	// Obtenemos los rectángulos de tamaño mínimo que encapsulan cada contorno.
	std::cout << "Aproximando contornos a areas rectangulares..." << std::endl;
	std::vector<Rect> rects;
	rects.reserve(contours.size());
	//for(int i = 0; i < contours.size(); i++)
	//	rects.push_back(minAreaRect(contours[i]));
		
	for(int i = 0; i < contours.size(); i++)
		rects.push_back(boundingRect(contours[i]));
		
		
	drawing = Mat::zeros(originalImage.size(), CV_8UC3);
	for(int i = 0; i < rects.size(); i++)
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		rectangle(drawing, rects[i].tl(), rects[i].br(), color, 2);
	}
	imwrite("areas.jpg", drawing);
	
		
	// Eliminamos areas o rectángulos erroneos.
	std::cout << "Eliminando areas rectangulares incorrectas..." << std::endl;
	rects = procesaAreas(rects);
	
	drawing = Mat::zeros(originalImage.size(), CV_8UC3);
	for(int i = 0; i < rects.size(); i++)
	{		
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		rectangle(drawing, rects[i].tl(), rects[i].br(), color, 3);
	}
	imwrite("processed-areas.jpg", drawing);
	
	// Por cada rectángulo, extraemos la foto que hay dentro.
	std::cout << "Extrayendo las fotos de las areas rectangulares..." << std::endl;
	fotos.reserve(rects.size());
	
	for(int i = 0; i < rects.size(); i++)
	{	
		// Coger la porción de imagen correspondiente a la foto.
		Mat M = Mat(originalImage, rects[i]);
		fotos.push_back(M);
	}
	
	return fotos;
}



/**
 * @return Devuelve true si el punto p está dentro del rectángulo R
 */
bool isInside(Point2f p, Rect R)
{
	return R.contains(p);
}

/**
 * @return Devuelve true si A está dentro de B o A es igual a B
 */
bool isInside(Rect A, Rect B)
{
	return isInside(A.tl(), B) && isInside(A.br(), B);
}

std::vector<Rect> procesaAreas(std::vector<Rect> areas)
{
	std::list<Rect> validAreas;
	std::list<Rect> uncheckedAreas(areas.begin(), areas.end());
	
	/* validamos cada área */
	while(!uncheckedAreas.empty())
	{
		Rect A = uncheckedAreas.front();
		uncheckedAreas.pop_front();
		std::list<Rect>::iterator it = uncheckedAreas.begin();

		/* comprobamos si A está dentro de alguna otra área sin comprobar */
		if(it != uncheckedAreas.end())
		{
			Rect B = *it;
			it++;
			while(!isInside(A,B) && (it != uncheckedAreas.end()))
			{
				B = *it;
				it++;
			}
			
			if(isInside(A,B))
			{
				/* A no es una área válida */
				continue;
			}
		}
		
		/* comprobamos si A está dentro de alguna area dada por válida */
		if(!validAreas.empty())
		{
			it = validAreas.begin();
			Rect B = *it;
			it++;
			while(!isInside(A,B) && (it != validAreas.end()))
			{
				B = *it;
				it++;
			}
			if(isInside(A,B))
			{
				/* A no es un área válida */
				continue;
			}
		}
		
		/* A es un área válida */
		validAreas.push_back(A);
	}
	
	/* De las áreas válidas, eliminar las que sean muy pequeñas... */
	std::list<Rect>::iterator it = validAreas.begin();	
	while(it != validAreas.end())
	{
		Rect R = *it;
		if((R.width < CONF_AREA_MIN_WIDTH) || (R.height < CONF_AREA_MIN_HEIGHT))
			it=validAreas.erase(it);
		else
			it++;
	}
	
	return std::vector<Rect>(validAreas.begin(),validAreas.end());
}
