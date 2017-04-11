//
// Created by ashish on 4/4/17.
//

#ifndef GPUPROJECT_UTILS_HPP
#define GPUPROJECT_UTILS_HPP


#include <glad/glad.h>
#include <vector>
#include <iostream>
#include <string>
#include <cuda_runtime.h>
class GLFWwindow;

namespace uf {
    template<typename T>
    void check(T err, const char* const func, const char* const file, const int line) {
        if (err != cudaSuccess) {
            std::cerr << "CUDA error at: " << file << ":" << line << std::endl;
            std::cerr << cudaGetErrorString(err) << " " << func << std::endl;
            exit(1);
        }
    }

#define checkCudaErrors(val)           uf::check ( (val), #val, __FILE__, __LINE__ )

    GLFWwindow *createWindow(int width, int height, const char *title);

    int initGlad();

    GLuint createGlTex2DCuda(GLsizei w, GLsizei h, GLint clampX = GL_CLAMP_TO_EDGE, GLint clampY = GL_CLAMP_TO_EDGE,
                         GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST);

    int findCudaDevice();

    std::vector<char> fileToCharArr(const std::string & filename,bool printFile = false);
    typedef std::pair<GLenum,const char *> shDf;
    GLuint compileShader(const shDf & sd);
    GLuint compileShader(const shDf & sd);
    GLuint makeProgram(GLuint vS,GLuint fS,bool deleteDetachShaders);

}
#endif //GPUPROJECT_UTILS_HPP