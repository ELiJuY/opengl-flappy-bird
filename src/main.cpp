#include <cstdlib>
#include <ctime>
#include "core/config.h"
#include "core/timing.h"
#include "game/gameState.h"
#include "game/player.h"
#include "graphics/postprocess.h"
#include "graphics/renderer.h"
#include "graphics/sky.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    init_GLFW();            // Inicializa lib GLFW
    window = Init_Window(prac);  // Crea ventana usando GLFW, asociada a un contexto OpenGL X.Y
    load_Opengl();         // Carga funciones de OpenGL, comprueba versión.
    initScene();          // Prepara escena

    srand((unsigned int)time(NULL));
    reiniciarJuego();
    lastTime = (float)glfwGetTime();

    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window))
    {
        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
        show_info();

#ifdef ENABLE_GRAPHICS_DEBUG
            printf(
            "TIME: %.2f h\n",
            simulatedHour
        );
#endif
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);


}


//////////  FUNCION PARA MOSTRAR INFO OPCIONAL EN EL TITULO DE VENTANA  //////////
void show_info()
{
    static int fps = 0;
    static double last_tt = 0;

    double elapsed, tt;
    char nombre_ventana[256];   // buffer para modificar titulo de la ventana

    fps++;
    tt = glfwGetTime();  // Contador de tiempo en segundos 

    elapsed = (tt - last_tt);

    if (elapsed >= 0.5)  // Refrescar cada 0.5 segundo
    {
        sprintf_s(
            nombre_ventana,
            256,
            "%s | Score: %d | FPS: %4.0f | %d x %d",
            prac,
            score,
            fps / elapsed,
            ANCHO,
            ALTO
        );

        glfwSetWindowTitle(window, nombre_ventana);

        last_tt = tt;
        fps = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  ASIGNACION FUNCIONES CALLBACK
///////////////////////////////////////////////////////////////////////////////////////////////////////////


// Callback de cambio tamaño de ventana
void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	ALTO = height;	ANCHO = width;
    resizeSceneFramebuffer(ANCHO, ALTO);
}

// Callback de pulsacion de tecla
// Callback de pulsacion de tecla
static void KeyCallback(GLFWwindow* window, int key, int code, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Salto del jugador
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (isGamePlaying())
        {
            playerVelY = jumpImpulse;
        }
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        reiniciarJuego();
        lastTime = (float)glfwGetTime();
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        togglePause();
    }

    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS) keyA = true;
        if (action == GLFW_RELEASE) keyA = false;
    }

    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS) keyD = true;
        if (action == GLFW_RELEASE) keyD = false;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS && isGamePlaying())
    {
        toggleHeadlights();
    }

}


void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
}



