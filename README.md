# OpenGL Flappy Bird

Proyecto final desarrollado en C++ con OpenGL.

## Clonar el repositorio

Este proyecto usa submódulos, por lo que debe clonarse con `--recursive`:

```bash
git clone --recursive https://github.com/ELiJuY/opengl-flappy-bird.git
cd opengl-flappy-bird
```

Si ya se ha clonado sin `--recursive`, inicializar los submódulos con:

```bash
git submodule update --init --recursive
```

## Compilar en Linux

Es necesario tener instalado `cmake`, `gcc` y `g++`.

Desde la raíz del proyecto:

```bash
mkdir -p build
cd build
cmake .. -DOpenGL_GL_PREFERENCE=GLVND -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -j8
```

## Compilar en Windows

Es necesario tener instalado Visual Studio con soporte para C++ y CMake.

Desde la raíz del proyecto, generar la solución:

```powershell
mkdir build
cd build
cmake ..
```

Después, abrir el fichero de solución `.sln` generado en la carpeta `build` con Visual Studio y compilar el proyecto desde el IDE.

## Ejecutar

El ejecutable generado se llama `flappy`.

En Linux, normalmente se podrá ejecutar desde la carpeta `build` con:

```bash
./flappy
```

En Windows, el ejecutable generado será `flappy.exe` y se encontrará dentro de la carpeta `build`, dependiendo de la configuración usada por Visual Studio.
