/**
 * 
 * @file
 * 	\brief Esta cabecera establece ciertas configuraciones del programa para
 * analizar las imágenes
 */
#ifndef __CONFIG_H
#	define __CONFIG_H

// Calidad de las fotos (formato JPG)
#	define CONF_JPEG_QUALITY 100 

/// Procesamiento de la imagen.

// Filtro gaussiano

// Tamaño del kernel del filtro 
#	define CONF_GAUSSIAN_KERNEL_SIZE Size(3,3)
# 	define CONF_USE_GAUSSIAN  // Activar filtro gaussiano

// Algoritmos de detección de bordes

// Algoritmo de Canny
#	define CONF_CANNY_LOWTHRESHOLD 40
#	define CONF_CANNY_RATIO 3
# 	define CONF_CANNY_KERNEL_SIZE 3
//# 	define CONF_USE_CANNY // Activar algoritmo canny

// Algoritmo de Laplace
#	define CONF_LAPLACE_KERNEL_SIZE 3
#	ifndef CONF_USE_CANNY
#		define CONF_USE_LAPLACE
#	endif


// Umbralización de la imagen con los bordes obtenidos
//#	define CONF_USE_THRESHOLDING // Activar thresholding
#	define CONF_THRESHOLD 20 // Threshold [0, 256)

// Aplicación de la matriz de convolución (solo si se umbralizan los bordes)
//#	define CONF_USE_FILTER // Activar convolución
// Tamaño del kernel 
#	define CONF_FILTER_KERNEL_SIZE 3
// Kernel del filtro
#	define CONF_FILTER_KERNEL Mat::ones(CONF_FILTER_KERNEL_SIZE, CONF_FILTER_KERNEL_SIZE, CV_32F)


// Eliminación de áreas pequeñas
// Dimensiones mínimas para las areas de las fotos
#	define CONF_AREA_MIN_WIDTH 32
#	define CONF_AREA_MIN_HEIGHT 32



#endif
