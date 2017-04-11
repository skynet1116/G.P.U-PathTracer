//
// Created by ashish on 4/4/17.
//

#include "BasicScene.hpp"
#include <GLFW/glfw3.h>
#include "utilfun.hpp"
#include <cassert>
#include <cuda.h>
#include <cuda_gl_interop.h>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

//quad positions in NDC Space
GLfloat quadVertices[20] = {
        // Positions  // Texture Coords
        -1.0f,  1.0f, 0.99f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.99f, 0.0f, 0.0f,
        1.0f,  1.0f,  0.99f, 1.0f, 1.0f,
        1.0f, -1.0f,  0.99f, 1.0f, 0.0f,
};
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
BasicScene::BasicScene(int width, int height, const std::string &title):width(width),height(height){

    using std::cout;
    using std::endl;


    //init glfw
    {
        glfwInit();

    }
    mainWindow = uf::createWindow(width,height,title.c_str());
    if(mainWindow == nullptr){
        cout <<"failed to create glfw window,exiting" << endl;
        exit(-1);
    }
    glfwMakeContextCurrent(mainWindow);
    //additonal glfw setup
    {
        glfwSetWindowUserPointer(mainWindow, this);
//        glfwSetCursorPosCallback(mainWindow, mousePosCallback);
        glfwSetKeyCallback(mainWindow, keyCallback);
//        glfwSetScrollCallback(mainWindow, scrollCallback);
        glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if(!uf::initGlad()){
        cout <<"failed to init glad,exiting" << endl;
        exit(-1);
    }

    //cuda init
    int dId;
    if((dId = uf::findCudaDevice()) < 0){
        exit(-1);
    }

    //setup texture
    {
        //TODO something unregister maybe
        renderQuad.tex = uf::createGlTex2DCuda(width, height);
        checkCudaErrors(cudaGraphicsGLRegisterImage(&cudaTexResource, renderQuad.tex, GL_TEXTURE_2D,
                                                    cudaGraphicsMapFlagsWriteDiscard));
    }
    //buffer setup
    {
        //TODO delete buffers
        glGenVertexArrays(1,&renderQuad.vao);
        glGenBuffers(1,&renderQuad.vbo);

        glBindBuffer(GL_ARRAY_BUFFER, renderQuad.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        glBindVertexArray(renderQuad.vao);
        glBindBuffer(GL_ARRAY_BUFFER, renderQuad.vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
        glBindVertexArray(0);
    }
    //shader setup
    {
        //TODO delete shader program here
        using namespace uf;
        auto vsS(fileToCharArr("./quad.vert"));
        auto fsS(fileToCharArr("./quad.frag"));
        renderQuad.program = makeProgram(compileShader(shDf(GL_VERTEX_SHADER,&vsS[0])),compileShader(shDf(GL_FRAGMENT_SHADER,&fsS[0])),true);
        renderQuad.texUniform = glGetUniformLocation(renderQuad.program,"tex");
    }
    //cuda buffer setup
    {
        //TODO delete cuda malloc here
        size_t num_texels =  (size_t)width*height;
        size_t num_values = num_texels * 4;
        size_t size_tex_data = sizeof(GLubyte) * num_values;

        //TODO pinned memory
        checkCudaErrors(cudaMalloc((void **)&cudaDestResource, size_tex_data));
    }
    //kernel default parameters
    {
        info.dev_drawRes = cudaDestResource;
        info.width = width;
        info.height = height;
        info.blockSize = dim3(16,16,1);
    }
    //TODO remove this hardcoding
    //load up mesh
    {

    }
    //TODO deletions here too
    {
//        float *buffer;
//        cudaMalloc(&buffer, N*sizeof(float));
//
//        // create texture object
//        cudaResourceDesc resDesc;
//        memset(&resDesc, 0, sizeof(resDesc));
//        resDesc.resType = cudaResourceTypeLinear;
//        resDesc.res.linear.devPtr = buffer;
//        resDesc.res.linear.desc.f = cudaChannelFormatKindFloat;
//        resDesc.res.linear.desc.x = 32; // bits per channel
//        resDesc.res.linear.sizeInBytes = N*sizeof(float);
//
//        cudaTextureDesc texDesc;
//        memset(&texDesc, 0, sizeof(texDesc));
//        texDesc.readMode = cudaReadModeElementType;
//
//        // create texture object: we only have to do this once!
//        cudaTextureObject_t tex=0;
//        cudaCreateTextureObject(&tex, &resDesc, &texDesc, NULL);
//
//        call_kernel(tex); // pass texture as argument

    }

    //setup camera

    {

        info.cam.dist = height/2;
        info.cam.pitch = 0;
        info.cam.yaw = 0;
        info.cam.aspect = width*1.0f/height;
        info.cam.fov = glm::tan(glm::radians(45.0f));

        //rotate along global y ,local x axis
        info.cam.front.x = cosf(glm::radians(info.cam.yaw-90))*cosf(glm::radians(info.cam.pitch));
        info.cam.front.y = sinf(glm::radians(info.cam.pitch));
        info.cam.front.z = sinf(glm::radians(info.cam.yaw-90))*cosf(glm::radians(info.cam.pitch));
        //right vector in x-z plane always(no roll camera)
        info.cam.right = glm::vec3(-info.cam.front.z,0,info.cam.front.x);
        info.cam.up    = glm::normalize(glm::cross(info.cam.right,info.cam.front));

        info.cam.pos = glm::vec3(0,0,0);

    }


    {
        cout << glm::to_string(info.cam.front) <<endl;
        cout << glm::to_string(info.cam.up) <<endl;
        cout << glm::to_string(info.cam.right) <<endl;

    }






}

BasicScene::~BasicScene() {

    glfwTerminate();
}

void BasicScene::run() {
    double delta = 0;
    double last = 0;
    glfwSetTime(last);
    while (!glfwWindowShouldClose(mainWindow)) {

        double curr = glfwGetTime();
        delta = curr-last;
        last = curr;

        glfwPollEvents();
        update(delta);

        //TODO see if lower level sync here
        //wait till prev frame done
        checkCudaErrors(cudaStreamSynchronize(0));


        float aaa[16];
        for(int i = 0;i<16;++i){
            aaa[i] = i%4?0:1;
        }


        launchKernel(info);

        cudaArray *texturePtr = nullptr;
        checkCudaErrors(cudaGraphicsMapResources(1, &cudaTexResource, 0));
        checkCudaErrors(cudaGraphicsSubResourceGetMappedArray(&texturePtr, cudaTexResource, 0, 0));

        size_t num_texels =  (size_t)width*height;
        size_t num_values = num_texels * 4;
        size_t size_tex_data = sizeof(GLubyte) * num_values;
        checkCudaErrors(cudaMemcpyToArray(texturePtr, 0, 0, cudaDestResource, size_tex_data, cudaMemcpyDeviceToDevice));
        checkCudaErrors(cudaGraphicsUnmapResources(1, &cudaTexResource, 0));


        draw();

        glfwSwapBuffers(mainWindow);

    }


}



void mousePosCallback(GLFWwindow *window, double posX, double posY) {

}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto sn  = (BasicScene * )glfwGetWindowUserPointer(window);
    assert(sn!= nullptr && sn->mainWindow == window);

    if(action == GLFW_PRESS) {
        if (key == GLFW_KEY_R) {

        }
        else if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {

}

void BasicScene::update(double delta) {

//    info.cam.camVecs[0];
//    float camSpeed = 3.0f*(float)delta;
//    if(glfwGetKey(mainWindow,GLFW_KEY_W)){
//
//        cam.position+=cam.front*camSpeed;
//    }
//    if(glfwGetKey(mainWindow,GLFW_KEY_A)){
//        cam.position-=cam.right*camSpeed;
//    }
//    if(glfwGetKey(mainWindow,GLFW_KEY_S)){
//        cam.position-=cam.front*camSpeed;
//    }
//    if(glfwGetKey(mainWindow,GLFW_KEY_D)){
//        cam.position+=cam.right*camSpeed;
//    }





}

void BasicScene::draw() {


    glClearColor(0.5,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);


    glUseProgram(renderQuad.program);

    glBindTexture(GL_TEXTURE_2D, renderQuad.tex);
    glActiveTexture(GL_TEXTURE0);

    glUniform1i(renderQuad.texUniform, 0);

    glBindVertexArray(renderQuad.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

}
