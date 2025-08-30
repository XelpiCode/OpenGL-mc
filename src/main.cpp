#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <openglErrorReporting.h>
#include <iostream>
#include <fstream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>

float triangleData[] = {
    //positions
    //x  y  z      colors
    //            r  g  b
    0.5, 0.5, 0,    1, 0, 0,    // vertex 1
    -0.5, 0.5, 0,   0, 1, 0,    // vertex 2
    -0.5, -0.5, 0,  0, 0, 1,    // vertex 3
    0.5, -0.5, 0,   0, 0, 1,    // vertex 4
};

unsigned short indices[] = {
    0,1,2, //first triangle
    0,2,3 //second triangle
};

struct Shader {

    GLuint id = 0;

    bool loadShaderProgramFromFile(const char *vertexShaderPath, const char *fragmentShaderPath);

    bool loadShaderProgramFromData(const char *vertexShaderData, const char *fragmentShaderData);

    void bind() const;

    void clear() const;

    GLint getUniformLocation(const char *name) const;
};

//this allocates memory
char* readEntireFile(const char *source) {
    std::ifstream file;
    file.open(source);

    if (!file.is_open()) {
        std::cout << "Error openning file: " << source << "\n";
        return nullptr;
    }

    GLint size = 0;
    file.seekg(0, file.end);
    size = file. tellg() ;
    file.seekg(0, file.beg);

    char *fileContent = new char[size + 1] {} ;

    file.read(fileContent, size);

    file.close();

    return fileContent;
}

bool Shader::loadShaderProgramFromFile(const char *vertexShaderPath,const char *fragmentShaderPath) {

    char *vertexData = readEntireFile(vertexShaderPath);
    char *fragmentData = readEntireFile(fragmentShaderPath);

    if (vertexData == nullptr || fragmentData == nullptr) {
        delete[] vertexData;
        delete[] fragmentData;

        return false;
    }

    bool res = loadShaderProgramFromData(vertexData, fragmentData);

    delete[] vertexData;
    delete[] fragmentData;

    return res;

}

GLint createShaderFromData(const char *data, GLenum shaderType) {

    GLuint shaderId = glCreateShader(shaderType);
    glShaderSource(shaderId, 1, &data, nullptr);
    glCompileShader(shaderId);

    GLint result = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);

    if (!result) {
        char *message = 0;
        int l = 0;

        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &l);

        if (l) {
            message = new char[l];

            glGetShaderInfoLog(shaderId, l, &l, message);

            message[l - 1] = 0;

            std:: cout << data << ":\n" << message << "\n";

            delete[] message;
        } else {
            std:: cout << data << ":\n" << "unknown error" << "\n";
        }

        glDeleteShader(shaderId);

        shaderId = 0;
        return shaderId;
    }

    return shaderId;

}

bool Shader::loadShaderProgramFromData(const char *vertexShaderData, const char *fragmentShaderData)
{
    auto vertexId = createShaderFromData(vertexShaderData, GL_VERTEX_SHADER);

    if (vertexId == 0) { return 0; }

    auto fragmentId = createShaderFromData(fragmentShaderData, GL_FRAGMENT_SHADER);

    if (fragmentId == 0)
    {
        glDeleteShader(vertexId);
        return 0;
    }

    id = glCreateProgram();

    glAttachShader(id, vertexId);
    glAttachShader(id, fragmentId);

    glLinkProgram(id);

    glDeleteShader(vertexId);
    glDeleteShader(fragmentId);

    GLint info = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &info);

    if (info != GL_TRUE)
    {
        char *message = 0;
        int l = 0;

        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

        message = new char[l];

        glGetProgramInfoLog(id, l, &l, message);

        message[l - 1] = 0;

        std::cout << "Link error: " << message << "\n";

        delete[] message;

        glDeleteProgram(id);
        id = 0;
        return 0;
    }

    glValidateProgram(id);

    return true;
}

GLint Shader::getUniformLocation(const char *name) const
{
    GLint res = glGetUniformLocation(id, name);

    if (res == -1)
    {
        std::cout << "Uniform error: " << name << "\n";
    }

    return res;
}

void Shader::bind() const {
    glUseProgram(id);
}

void Shader::clear() const {
    glDeleteProgram(id);
}

int main() {

    if (!glfwInit()) {
        std:: cout << "GLFW Init Error!\n";
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window) {
        std:: cout << "Window Error!\n";
        return 1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return 1;
    }

    enableReportGlErrors();

#pragma region imgui

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

#pragma endregion

#pragma region vao

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

#pragma endregion

#pragma region buffer

    //create buffer / VBO
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);

    //send the data to the buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleData), triangleData, GL_STATIC_DRAW);

    //the attribute representing position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr);

    //the attribute representing color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *)(sizeof(float) * 3));

#pragma endregion

#pragma region index buffer

    //create the buffer / EBO
    GLuint indexBuffer = 0;
    glGenBuffers(1, &indexBuffer);

    //send the data to the buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

#pragma endregion

    //its good practice to unbind your vao after you are done
    glBindVertexArray(0);

#pragma region shader loading

    Shader shader;
    shader.loadShaderProgramFromFile(RESOURCES_PATH "myshader.vert", RESOURCES_PATH "myshader.frag");
    shader.bind();

    GLint u_time = shader.getUniformLocation("u_time");
    GLint u_color = shader.getUniformLocation("u_color");

#pragma endregion

    while (!glfwWindowShouldClose(window)) {

        int w = 0 , h = 0;
        glfwGetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

    #pragma region imgui

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

    #pragma endregion

        ImGui::Begin("Window");
        ImGui::Text("Color test");
        static float color[3] = {0.5,0.5,0.5};
        ImGui::ColorPicker3("Color: ", color);
        ImGui::End();

        ImGui::ShowDemoWindow();

        shader.bind();

        glUniform1f(u_time, (float)(clock()) / 100.f);
        glUniform3fv(u_color, 1, color);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    #pragma region imgui

        ImGui::Render();
        int display_w = 0, display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    #pragma endregion

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &buffer);
    glfwDestroyWindow(window);
    glfwTerminate();
}