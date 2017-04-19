//
// Created by ashish on 4/4/17.
//

#ifndef GPUPROJECT_BASICSCENE_HPP
#define GPUPROJECT_BASICSCENE_HPP

#include <glad/glad.h>
#include <string>
#include <vector_types.h>
#include <glm/glm.hpp>
#include <cstring>

class GLFWwindow;
struct tex1DInfo{
    void                   *h_data;
    cudaResourceDesc desc;
    cudaTextureDesc texDesc;
    cudaTextureObject_t     textureObject;

    tex1DInfo()
    {
        memset(this,0,sizeof(tex1DInfo));
    }

};
struct CamInfo{
    glm::vec3 front,right,up,pos;
    float dist;
    float pitch ,yaw;
    float aspect;
    float fov;

};
void setPitchAndRoll(CamInfo & cam,float xoffset, float yoffset);
struct kernelInfo{
    dim3 blockSize;
    unsigned int * dev_drawRes;
    int width,height;
    CamInfo cam;
    glm::vec4 *  triangleTex = nullptr;
    size_t numTris = 0;

};
class BasicScene{
public:
    BasicScene(int width, int height, const std::string &title);
    ~BasicScene();
    void run();
    kernelInfo info;

private:
    friend void mousePosCallback(GLFWwindow * window,double posX,double posY);
    friend void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    friend void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    void draw();
    void launchKernel(const kernelInfo & info);


private:
    //opengl stuff
    GLFWwindow * mainWindow;
    struct Quad{
       GLuint tex,vao,vbo,program;GLint texUniform;
    }renderQuad;
    struct cudaGraphicsResource *cudaTexResource;


    //the cuda image to use for drawing
    unsigned int * cudaDestResource;
    int width,height;

    //cuda triangles texture
    tex1DInfo trianglesTex;
    glm::vec4 * gpuTris = nullptr;
private:

    void update(double delta);
    //update is a functor
    struct Updater{
        //TODO make parent Scene a template class :)
        Updater(BasicScene & prtScn):prtScn(prtScn){}
        void operator () (double delta);
        bool firstMouse = true;
        double lastX,lastY;
        BasicScene & prtScn;
    }updater;
//    void update(double delta);

};


#endif //GPUPROJECT_BASICSCENE_HPP
