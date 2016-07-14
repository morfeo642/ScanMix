#!/bin/sh

: ${INSTALL_DIR:=~} 
: ${EXEC_ICON_DIR:=~}
: ${IMAGE_DIR=~}

# Instalamos dependencias
echo "instalando dependencias..."
sudo apt-get install build-essential
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

# Instalar open cv
echo "instalando opencv..."
cd opencv && cmake ../opencv-src -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local && make && make install && cd ..

# Compilamos el código fuente
echo "compilando codigo fuente..."
cd src && cmake . && make && cd ..

# Copiamos los scripts al directorio de instalación.
echo 'Copiando ficheros a ' $INSTALL_DIR '...' 
cp -r ./ScanMix $INSTALL_DIR/ScanMix
chmod -u+rwx $INSTALL_DIR/ScanMix/scripts/*.sh

# Generamos un link simbólico a la carpeta donde guardar las imagenes
echo 'Creando link simbolico al directorio de imagenes...'
ln -s $IMAGE_DIR $INSTALL_DIR/ScanMix/fotos

# Generamos un archivo .desktop en el directorio indicado.
echo 'Creando icono en ' $EXEC_ICON_DIR '...'
EXEC_ICON=$EXEC_ICON_DIR/ScanFotos.desktop
echo '[Desktop Entry]' > $EXEC_ICON
echo 'Type=Application' >> $EXEC_ICON
echo 'Version=1.0' >> $EXEC_ICON
echo 'Name=ScanMix' >> $EXEC_ICON
echo 'Comment=Aplicacion para scanear varias fotos simultaneamente' >> $EXEC_ICON
echo 'Exec=' $INSTALL_DIR/ScanMix/scripts/scanFotos.sh >> $EXEC_ICON 
echo 'Icon=' $INSTALL_DIR/ScanMix/icon.png >> $EXEC_ICON
echo 'Terminal=false' >> $EXEC_ICON

chmod -u+rwx $EXEC_ICON


