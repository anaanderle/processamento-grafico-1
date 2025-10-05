#include <iostream>
#include <string>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void   key_callback(GLFWwindow *w, int k, int sc, int action, int mods);

GLuint setupShader();
void   setupGeometry();
static void makeVAO(const GLfloat* data, size_t count, GLuint& vaoOut);

const GLuint WIDTH = 800, HEIGHT = 600;

const GLchar *vsSrc = R"(
#version 400
layout (location = 0) in vec3 position;
void main() { gl_Position = vec4(position, 1.0); }
)";

const GLchar *fsSrc = R"(
#version 400
uniform vec4 inputColor;
out vec4 color;
void main() { color = inputColor; }
)";

GLuint vaoRoofBorder, vaoRoofFill;
GLuint vaoWallLeft, vaoWallRight;
GLuint vaoWindowFill, vaoWindowBorder, vaoWinCrossV, vaoWinCrossH;
GLuint vaoDoorFrame, vaoDoorFill, vaoGround;
GLuint vaoRoofBase;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *win = glfwCreateWindow(WIDTH, HEIGHT, "Casa", nullptr, nullptr);
    if (!win) { cerr << "Falha ao criar a janela\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetKeyCallback(win, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { cerr << "Falha GLAD\n"; return -1; }

    int fbw, fbh; glfwGetFramebufferSize(win, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    GLuint prog = setupShader();
    glUseProgram(prog);
    setupGeometry();

    GLint uColor = glGetUniformLocation(prog, "inputColor");

    double prev = glfwGetTime(), cd = 0.1;
    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime(), dt = now - prev; prev = now;
        cd -= dt;
        if (cd <= 0.0 && dt > 0.0) {
            double fps = 1.0 / dt;
            char buf[96]; sprintf(buf, "Casinha - OpenGL | FPS %.2lf", fps);
            glfwSetWindowTitle(win, buf);
            cd = 0.1;
        }

        glfwPollEvents();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // fundo branco
        glClear(GL_COLOR_BUFFER_BIT);

        // Telhado
        glBindVertexArray(vaoRoofBorder);
        glUniform4f(uColor, 0.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Paredes
        glBindVertexArray(vaoWallLeft);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(vaoWallRight);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Moldura janela
        glBindVertexArray(vaoWindowBorder);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Porta
        glBindVertexArray(vaoDoorFrame);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(vaoDoorFill);
        glUniform4f(uColor, 0.36f, 0.18f, 0.12f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Janela
        glBindVertexArray(vaoWindowFill);
        glUniform4f(uColor, 1.0f, 1.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform4f(uColor, 0.0f, 0.0f, 0.0f, 1.0f);
        glBindVertexArray(vaoWinCrossV);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(vaoWinCrossH);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Telhado
        glBindVertexArray(vaoRoofFill);
        glUniform4f(uColor, 0.80f, 0.00f, 0.00f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(vaoRoofBase);
        glUniform4f(uColor, 0.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Chão
        glBindVertexArray(vaoGround);
        glUniform4f(uColor, 1.0f, 0.5f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(win);
    }

    GLuint vaos[] = {vaoRoofBorder,vaoRoofFill,vaoWallLeft,vaoWallRight,
                     vaoWindowFill,vaoWindowBorder,vaoWinCrossV,vaoWinCrossH,
                     vaoDoorFrame,vaoDoorFill,vaoGround};
    for (GLuint v : vaos) glDeleteVertexArrays(1, &v);

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *w, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(w, GL_TRUE);
}

GLuint setupShader() {
    GLint ok; GLchar log[512];
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, NULL);
    glCompileShader(vs); glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) { glGetShaderInfoLog(vs, 512, NULL, log); cerr << "VS:\n" << log << endl; }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, NULL);
    glCompileShader(fs); glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) { glGetShaderInfoLog(fs, 512, NULL, log); cerr << "FS:\n" << log << endl; }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog); glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { glGetProgramInfoLog(prog, 512, NULL, log); cerr << "LINK:\n" << log << endl; }
    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

static void makeVAO(const GLfloat* data, size_t count, GLuint& vaoOut) {
    GLuint vbo;
    glGenVertexArrays(1, &vaoOut);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vaoOut);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLfloat), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setupGeometry() {
    const float yTopHouse =  0.20f;
    const float yBotHouse = -0.55f;
    const float xLeft     = -0.35f;
    const float xRight    =  0.35f;

    // Telhado
    const GLfloat roofBorder[] = {
        xLeft - 0.03f, yTopHouse, 0.0f,
        xRight+ 0.03f, yTopHouse, 0.0f,
        0.0f,          0.85f,     0.0f
    };

    const GLfloat roofFill[] = {
        xLeft,  yTopHouse, 0.0f,
        xRight, yTopHouse, 0.0f,
        0.0f,   0.80f,     0.0f
    };

    const float baseT = 0.03f;
    const GLfloat roofBase[] = {
        xLeft,  yTopHouse + baseT*0.5f, 0,
        xRight, yTopHouse + baseT*0.5f, 0,
        xRight, yTopHouse - baseT*0.5f, 0,
        xLeft,  yTopHouse + baseT*0.5f, 0,
        xRight, yTopHouse - baseT*0.5f, 0,
        xLeft,  yTopHouse - baseT*0.5f, 0
    };

    // Paredes
    const float wallW = 0.03f;
    const GLfloat wallLeft[] = {
        xLeft-wallW,yTopHouse,0,   xLeft,yTopHouse,0,   xLeft,yBotHouse,0,
        xLeft-wallW,yTopHouse,0,   xLeft,yBotHouse,0,   xLeft-wallW,yBotHouse,0
    };
    const GLfloat wallRight[] = {
        xRight,yTopHouse,0,   xRight+wallW,yTopHouse,0,   xRight+wallW,yBotHouse,0,
        xRight,yTopHouse,0,   xRight+wallW,yBotHouse,0,   xRight,yBotHouse,0
    };

    // Janela
    const float xW0 = -0.25f, xW1 = -0.05f, yW0 = -0.05f, yW1 = 0.15f; // corpo amarelo
    const float border = 0.02f;                                         // “traço” preto
    const GLfloat windowFill[] = {
        xW0,yW0,0,  xW1,yW0,0,  xW1,yW1,0,
        xW0,yW0,0,  xW1,yW1,0,  xW0,yW1,0
    };
    const GLfloat windowBorder[] = {
        xW0-border,yW0-border,0,  xW1+border,yW0-border,0,  xW1+border,yW1+border,0,
        xW0-border,yW0-border,0,  xW1+border,yW1+border,0,  xW0-border,yW1+border,0
    };
    const float cx = 0.5f*(xW0+xW1), cy = 0.5f*(yW0+yW1);
    const float crossT = 0.01f;
    const GLfloat winCrossV[] = {
        cx-crossT, yW0, 0,  cx+crossT, yW0, 0,  cx+crossT, yW1, 0,
        cx-crossT, yW0, 0,  cx+crossT, yW1, 0,  cx-crossT, yW1, 0
    };
    const GLfloat winCrossH[] = {
        xW0, cy-crossT, 0,  xW1, cy-crossT, 0,  xW1, cy+crossT, 0,
        xW0, cy-crossT, 0,  xW1, cy+crossT, 0,  xW0, cy+crossT, 0
    };

    // Porta
    const float xDf0=-0.12f, xDf1=0.12f, yDf0=-0.55f, yDf1=-0.27f;
    const GLfloat doorFrame[] = {
        xDf0,yDf0,0,  xDf1,yDf0,0,  xDf1,yDf1,0,
        xDf0,yDf0,0,  xDf1,yDf1,0,  xDf0,yDf1,0
    };
    const float xDi0=-0.08f, xDi1=0.08f, yDi0=-0.55f, yDi1=-0.30f;
    const GLfloat doorFill[] = {
        xDi0,yDi0,0,  xDi1,yDi0,0,  xDi1,yDi1,0,
        xDi0,yDi0,0,  xDi1,yDi1,0,  xDi0,yDi1,0
    };

    // Chão
    const float yG1 = yBotHouse;
    const float yG0 = yG1 - 0.035f;
    const GLfloat ground[] = {
        -0.92f,yG0,0,   0.92f,yG0,0,   0.92f,yG1,0,
        -0.92f,yG0,0,   0.92f,yG1,0,  -0.92f,yG1,0
    };

    makeVAO(roofBorder,   sizeof(roofBorder)/sizeof(GLfloat),   vaoRoofBorder);
    makeVAO(roofFill,     sizeof(roofFill)/sizeof(GLfloat),     vaoRoofFill);
    makeVAO(roofBase,   sizeof(roofBase)  /sizeof(GLfloat), vaoRoofBase);
    makeVAO(wallLeft,     sizeof(wallLeft)/sizeof(GLfloat),     vaoWallLeft);
    makeVAO(wallRight,    sizeof(wallRight)/sizeof(GLfloat),    vaoWallRight);
    makeVAO(windowFill,   sizeof(windowFill)/sizeof(GLfloat),   vaoWindowFill);
    makeVAO(windowBorder, sizeof(windowBorder)/sizeof(GLfloat), vaoWindowBorder);
    makeVAO(winCrossV,    sizeof(winCrossV)/sizeof(GLfloat),    vaoWinCrossV);
    makeVAO(winCrossH,    sizeof(winCrossH)/sizeof(GLfloat),    vaoWinCrossH);
    makeVAO(doorFrame,    sizeof(doorFrame)/sizeof(GLfloat),    vaoDoorFrame);
    makeVAO(doorFill,     sizeof(doorFill)/sizeof(GLfloat),     vaoDoorFill);
    makeVAO(ground,       sizeof(ground)/sizeof(GLfloat),       vaoGround);
}