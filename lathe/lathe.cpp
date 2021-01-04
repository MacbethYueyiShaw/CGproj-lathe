#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <math.h>
#include <vector>
#include "include/shader.h"
#include "include/model.h"
#include "include/camera.h"
#include "include/skybox.h"

/*
Proj:A Lathe Simulator by openGL
Author: Macbeth Yueyi Shaw
Time: Dec/2020
*/
//对的这个代码注释就是中西合璧

//////////////////////////////////////////////FUNCTION//////////////////////////////////////////////
//tool func: global setting and control
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
//shader func: draw meshes
void skybox_draw(Shader skyboxShader, unsigned int skyboxVAO, unsigned int cubemapTexture);
void model_draw(Shader shader, Model mymodel, glm::vec3 position = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec3 rotate_axe = glm::vec3(0.0f, 1.0f, 0.0f), float radians = 0.0f);
//model caculate func
void cylinder_radius_vector_init();
void cylinder_data_update();
void cylinder_buffer_update(unsigned int cylinderVAO, unsigned int cylinderVBO);
///////////////////////////////////////////GLOBAL VALUE/////////////////////////////////////////////
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, -2.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// lighting
glm::vec3 lightPos(0.0f, 5.0f, -10.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// cylinder data config
//点阵精细度设置
const int Y_SEGMENTS = 300;
const int X_SEGMENTS = 20;
const int R_SEGMENTS = 100;
//空间参数设置
const GLfloat PI = 3.14159265358979323846f;
//const glm::vec3 cylinder_pos=glm::vec3(-2.0f, 5.0f, -0.5f);//空间位置
const glm::vec3 cylinder_pos = glm::vec3(0.0f, 0.0f, 0.0f);//空间位置
const float rotate_speed = 5.0f;//圆柱转速倍率
const float radius_k = 0.5f;//半径系数，用于调节整个圆柱的半径
const float length_k = 2.0f;//半径系数，用于调节整个圆柱的长度
float radius[Y_SEGMENTS + 1] = { 0.0f };//半径数组，记录对应segment位置圆柱的半径，用于记录切削效果
std::vector<float> cylinderVertices;//圆柱点集
//std::vector<int> cylinderIndices;//圆柱点绘制index集 <- 改良之后莫得必要
std::vector<float> cylinderAllData;//圆柱绘制数据集

//切削刀具设置(刀用一个倒四棱锥表示)
glm::vec3 knife_pos = glm::vec3(-2.0f, 0.55f, 0.0f);//空间位置
float knife_distance = 1.0f;

////////////////////////////////////////////////MAIN/////////////////////////////////////////////////
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);

    // configure global opengl state
    // -----------------------------
    // draw in wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);


    ////////////////////////////////////////////LOAD_DATA///////////////////////////////////////////////////
    // build and compile shaders
    // -------------------------
    Shader ourShader("./shaders/vs.shader", "./shaders/fs.shader");
    Shader lightCubeShader("./shaders/light_cube.vs", "./shaders/light_cube.fs");
    Shader skyboxShader("./shaders/6.1.skybox.vs", "./shaders/6.1.skybox.fs");
    Shader cylinderShader("./shaders/cylinder.vs", "./shaders/cylinder.fs");
    Shader knifeShader("./shaders/knife.vs", "./shaders/knife.fs");
    // load models
    // -----------
    //Model ourModel("./resources/objects/nanosuit/nanosuit.obj");
    //Model ourModel("./resources/objects/Miku/Gemstone Miku 1.0.1.obj"); //*if you use this mesh, please modify the uv config in the "fs.shader"*
    Model ourModel("./resources/objects/lathe/lathe.obj");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float light_vertices[] = {
       -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
       -0.5f,  0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,

       -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,
       -0.5f, -0.5f,  0.5f,

       -0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,

       -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
       -0.5f, -0.5f,  0.5f,
       -0.5f, -0.5f, -0.5f,

       -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f, -0.5f,
    };
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    float knife_vertices[] = {
        1.0f,1.0f,1.0f,0.0f,1.0f,0.0f,
        1.0f,1.0f,-1.0f,0.0f,1.0f,0.0f,
        -1.0f,1.0f,-1.0f,0.0f,1.0f,0.0f,
        -1.0f,1.0f,-1.0f,0.0f,1.0f,0.0f,
        -1.0f,1.0f,1.0f,0.0f,1.0f,0.0f,
        1.0f,1.0f,1.0f,0.0f,1.0f,0.0f,

        1.0f,1.0f,1.0f,1.0f,-1.0f,0.0f,
        1.0f,1.0f,-1.0f,1.0f,-1.0f,0.0f,
        0.0f,-1.0f,0.0f,1.0f,-1.0f,0.0f,

        1.0f,1.0f,1.0f,0.0f,-1.0f,1.0f,
        -1.0f,1.0f,1.0f,0.0f,-1.0f,1.0f,
        0.0f,-1.0f,0.0f,0.0f,-1.0f,1.0f,

        -1.0f,1.0f,1.0f,-1.0f,-1.0f,0.0f,
        -1.0f,1.0f,-1.0f,-1.0f,-1.0f,0.0f,
        0.0f,-1.0f,0.0f,-1.0f,-1.0f,0.0f,

        -1.0f,1.0f,-1.0f,0.0f,-1.0f,-1.0f,
        1.0f,1.0f,-1.0f,0.0f,-1.0f,-1.0f,
        0.0f,-1.0f,0.0f,0.0f,-1.0f,-1.0f,
    };
    cylinder_radius_vector_init();//初始化半径集合
    cylinder_data_update();//依据radius集合生成cylinder点阵数据集，必须在init之后
    
    ////////////////////////////////////////////BIND VAO/VBO/EBO//////////////////////////////////////////////
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------
    vector<std::string> faces
    {
        "resources/textures/skybox/right.jpg",
        "resources/textures/skybox/left.jpg",
        "resources/textures/skybox/top.jpg",
        "resources/textures/skybox/bottom.jpg",
        "resources/textures/skybox/front.jpg",
        "resources/textures/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    //knife VAO VBO
    unsigned int knifeVBO, knifeVAO;
    glGenVertexArrays(1, &knifeVAO);
    glGenBuffers(1, &knifeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, knifeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(knife_vertices), knife_vertices, GL_STATIC_DRAW);

    glBindVertexArray(knifeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // first, configure the VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertices), light_vertices, GL_STATIC_DRAW);

    // second, configure the light's VAO
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /*cylinder数据处理*/
    unsigned int cylinderVBO, cylinderVAO;
    /*culinder数据处理*/
    glGenVertexArrays(1, &cylinderVAO);
    glGenBuffers(1, &cylinderVBO);
    //GLuint element_buffer_object;//EBO
    //glGenBuffers(1, &element_buffer_object);
    //cylinder_buffer_update(cylinderVAO, cylinderVBO);

    ///////////////////////////////////////////////SHADING/////////////////////////////////////////////////
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);
        cylinder_buffer_update(cylinderVAO, cylinderVBO);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
  
        //draw lathe
        //model_draw(ourShader,ourModel, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);

        //draw cylinder
        cylinderShader.use();
        cylinderShader.setMat4("projection", projection);
        cylinderShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, cylinder_pos);
        model = glm::rotate(model, rotate_speed*(float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));//x轴控制轴心自转
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));//让圆柱水平放置
        model = glm::scale(model, glm::vec3(1.0f)); // a smaller cube
        cylinderShader.setMat4("model", model);
        //绘制球
        //开启面剔除(只需要展示一个面，否则会有重合)
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        glBindVertexArray(cylinderVAO);
        glDrawArrays(GL_TRIANGLES, 0, X_SEGMENTS * Y_SEGMENTS * 6);
        //glDrawElements(GL_TRIANGLES, X_SEGMENTS* Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

        //draw knife
        // light properties
        knifeShader.use();
        glm::vec3 lightColor = glm::vec3(1.0f);
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
        knifeShader.setVec3("light.ambient", ambientColor);
        knifeShader.setVec3("light.diffuse", diffuseColor);
        knifeShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        knifeShader.setVec3("light.position", lightPos);

        // material properties
        knifeShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        knifeShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        knifeShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
        knifeShader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        knifeShader.setMat4("projection", projection);
        knifeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, knife_pos);
        model = glm::scale(model, glm::vec3(0.05f)); // a smaller cube
        //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));//rotate
        knifeShader.setMat4("model", model);
        // render the cube
        glBindVertexArray(knifeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 22);

        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeShader.setMat4("model", model);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //draw skybox
        skybox_draw(skyboxShader,skyboxVAO, cubemapTexture);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /////////////////////////////////////////////END/////////////////////////////////////////////////////////
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &knifeVAO);
    glDeleteBuffers(1, &knifeVBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &cylinderVAO);
    glDeleteBuffers(1, &cylinderVBO);
   //lDeleteBuffers(1, &element_buffer_object);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        knife_pos.y = knife_pos.y + 0.5f / R_SEGMENTS;
        knife_distance += 1.0f / R_SEGMENTS;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        //std::cout << "distance: " << knife_distance << "    knife_pos.y: " << knife_pos.y << std::endl;
        if (knife_pos.y > 0.05f)
        {
            knife_pos.y = knife_pos.y - 0.5f / R_SEGMENTS;
            knife_distance -= 1.0f / R_SEGMENTS;
            if (knife_distance<0)
            {
                knife_distance = 0.0f;
            }
        }
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (knife_pos.x<2.0f)
        {
            knife_pos.x = knife_pos.x + 4.0f / Y_SEGMENTS;
            int x_seg = (int)((knife_pos.x + 2.0f) * Y_SEGMENTS / 4.0f);
            //std::cout << "distance: " << knife_distance << "    x_seg: " << x_seg << "    radius[x_seg]: " << radius[x_seg] << std::endl;
            if (radius[x_seg]>knife_distance)
            {
                radius[x_seg] = knife_distance;
                cylinder_data_update();
            }
        }   
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (knife_pos.x > -2.0f)
        {
            knife_pos.x = knife_pos.x - 4.0f / Y_SEGMENTS;
            int x_seg = (int)((knife_pos.x + 2.0f) * Y_SEGMENTS / 4.0f);
            //std::cout << "distance: " << knife_distance << "    x_seg: " << x_seg << "    radius[x_seg]: " << radius[x_seg] << std::endl;
            if (radius[x_seg] > knife_distance)
            {
                radius[x_seg] = knife_distance;
                cylinder_data_update();
            }
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
//skybox
void skybox_draw(Shader skyboxShader, unsigned int skyboxVAO, unsigned int cubemapTexture)
{
    // draw skybox as last
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    skyboxShader.use();
    view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
}
//draw shader
void model_draw(Shader shader,Model mymodel, glm::vec3 position, glm::vec3 scale , glm::vec3 rotate_axe, float radians)
{
    // don't forget to enable shader before setting uniforms
    shader.use();

    //import global values
    shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("viewPos", camera.Position);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    model = glm::rotate(model, glm::radians(radians), rotate_axe);

    shader.setMat4("model", model);
   
    // draw model with the shader
    mymodel.Draw(shader);
}
//init radius vector
void cylinder_radius_vector_init()
{
    for (int i = 0; i < Y_SEGMENTS; i++)
    {
        radius[i] = 1.0f;
    }
}
//caculate the vertex and anormal vec of target cylinder
void cylinder_data_update()
{
    //cylinderIndices.clear();
    cylinderVertices.clear();
    cylinderAllData.clear();
    //draw a sphere
    /*
    for (int y = 0; y <= Y_SEGMENTS; y++)
    {
        for (int x = 0; x <= X_SEGMENTS; x++)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            cylinderVertices.push_back(xPos);
            cylinderVertices.push_back(yPos);
            cylinderVertices.push_back(zPos);
            std::cout <<y* X_SEGMENTS + x<<": "<< xPos << " " << yPos << " "<< zPos<< std::endl;
        }
    }
    for (int i = 0; i < Y_SEGMENTS; i++)
    {
        for (int j = 0; j < X_SEGMENTS; j++)
        {
            cylinderIndices.push_back(i * (X_SEGMENTS + 1) + j);
            cylinderIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j);
            cylinderIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            cylinderIndices.push_back(i * (X_SEGMENTS + 1) + j);
            cylinderIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            cylinderIndices.push_back(i * (X_SEGMENTS + 1) + j + 1);
        }
    }*/
    //draw cylinder
    for (int y = 0; y <= Y_SEGMENTS; y++)
    {
        //std::cout <<y << ": " << radius[y] << std::endl;
        for (int x = 0; x <= X_SEGMENTS; x++)
        {
            //aPos
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = radius_k * radius[y] * std::cos(xSegment * 2.0f * PI);
            float yPos = length_k * ( 2.0f* ySegment -1.0f );
            float zPos = radius_k * radius[y] * std::sin(xSegment * 2.0f * PI);
            if (y == 0 || y == Y_SEGMENTS) {
                xPos = 0;
                zPos = 0;
            }
            cylinderVertices.push_back(xPos);
            cylinderVertices.push_back(yPos);
            cylinderVertices.push_back(zPos);
            //std::cout << y * (X_SEGMENTS+1) + x << ": " << xPos << " " << yPos << " " << zPos << std::endl;
            //aNormal

        }
    }
    /* 输入EBO <- 在启用多组属性时，禁用
    for (int i = 0; i < Y_SEGMENTS; i++)
    {
        for (int j = 0; j < X_SEGMENTS; j++)
        {
            std::cout << "index order:" 
                << i * (X_SEGMENTS + 1) + j << "   " 
                << (i + 1) * (X_SEGMENTS + 1) + j << "   "
                << (i + 1) * (X_SEGMENTS + 1) + j + 1 << "   "
                <<  i * (X_SEGMENTS + 1) + j << "   "
                << (i + 1) * (X_SEGMENTS + 1) + j + 1 << "   "
                <<  i * (X_SEGMENTS + 1) + j + 1 << "   "
                << std::endl;
            cylinderIndices.push_back(i * (X_SEGMENTS + 1) + j);
            cylinderIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j);
            cylinderIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            cylinderIndices.push_back(i * (X_SEGMENTS + 1) + j);
            cylinderIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            cylinderIndices.push_back(i * (X_SEGMENTS + 1) + j + 1);
        }
    }*/

    for (int i = 0; i < Y_SEGMENTS; i++)
    {
        for (int j = 0; j < X_SEGMENTS; j++)
        {
            /*
            std::cout << "index order:"
                << i * (X_SEGMENTS + 1) + j << "   "
                << (i + 1) * (X_SEGMENTS + 1) + j << "   "
                << (i + 1) * (X_SEGMENTS + 1) + j + 1 << "   "
                << i * (X_SEGMENTS + 1) + j << "   "
                << (i + 1) * (X_SEGMENTS + 1) + j + 1 << "   "
                << i * (X_SEGMENTS + 1) + j + 1 << "   "
                << std::endl;
            std::cout << cylinderVertices[3*(i * (X_SEGMENTS + 1) + j)]<< cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j)+1]<< cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j)+2] << "   "
                << cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j)] << cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j) + 1] << cylinderVertices[3 * ((i+1) * (X_SEGMENTS + 1) + j) + 2] << "   "
                << cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j+1)] << cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j+1) + 1] << cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j+1) + 2] << "   "
                << std::endl;
            */
            //input first 3 vertics
            float x1 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j)];
            float y1 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j)+1];
            float z1 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j)+2];
            float x2 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j)];
            float y2 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j) + 1];
            float z2 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j) + 2];
            float x3 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j + 1)];
            float y3 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j + 1) + 1];
            float z3 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j + 1) + 2];

            float x4 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j)];
            float y4 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j) + 1];
            float z4 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j) + 2];
            float x5 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j + 1)];
            float y5 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j + 1) + 1];
            float z5 = cylinderVertices[3 * ((i + 1) * (X_SEGMENTS + 1) + j + 1) + 2];
            float x6 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j + 1)];
            float y6 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j + 1) + 1];
            float z6 = cylinderVertices[3 * (i * (X_SEGMENTS + 1) + j + 1) + 2];
            //caculate normal
            glm::vec3 normal(0.0f);
            if (i == 0) {
                normal = glm::vec3(0.0f,-1.0f,0.0f);
            }
            else if (i == (Y_SEGMENTS - 1)) {
                normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            else {
                glm::vec3 AB(x2 - x1, y2 - y1, z2 - z1);
                glm::vec3 AC(x3 - x1, y3 - y1, z3 - z1);
                normal = glm::normalize(glm::cross(AB, AC));
            }
            //std::cout << "the normal:" << normal.x << normal.y << normal.z <<endl;
            cylinderAllData.push_back(x1);
            cylinderAllData.push_back(y1);
            cylinderAllData.push_back(z1);

            cylinderAllData.push_back(x2);
            cylinderAllData.push_back(y2);
            cylinderAllData.push_back(z2);

            cylinderAllData.push_back(x3);
            cylinderAllData.push_back(y3);
            cylinderAllData.push_back(z3);

            cylinderAllData.push_back(x4);
            cylinderAllData.push_back(y4);
            cylinderAllData.push_back(z4);

            cylinderAllData.push_back(x5);
            cylinderAllData.push_back(y5);
            cylinderAllData.push_back(z5);

            cylinderAllData.push_back(x6);
            cylinderAllData.push_back(y6);
            cylinderAllData.push_back(z6);
        }
    }
}
//update cylinder's VAO,VBO,EBO
void cylinder_buffer_update(unsigned int cylinderVAO, unsigned int cylinderVBO)
{
    
    //生成并绑定球体的VAO和VBO
    glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
    glBindVertexArray(cylinderVAO);
    //将顶点数据绑定至当前默认的缓冲中
    glBufferData(GL_ARRAY_BUFFER, cylinderAllData.size() * sizeof(float), &cylinderAllData[0], GL_STATIC_DRAW);

    //EBO  <- 改良之后并不需要这一步
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinderIndices.size() * sizeof(int), &cylinderIndices[0], GL_STATIC_DRAW);

    //设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //解绑VAO和VBO <- 没必要
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);
    //std::cout << "Did here" << endl;
}
