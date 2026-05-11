# Flappy Bird OpenGL

## Clonar el repositorio

```git clone --recursive https://github.com/ELiJuY/flappy-bird-opengl.git```

```cd flappy-bird-opengl```

## Compilar

```mkdir build```

```cd build``` 

```cmake .. -DOpenGL_GL_PREFERENCE=GLVND -DCMAKE_POLICY_VERSION_MINIMUM=3.5``` 

```make -j8```

## Ejecutar

```./bin/flappy```
