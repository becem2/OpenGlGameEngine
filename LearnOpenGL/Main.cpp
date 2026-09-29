#include <glad.h>
#include <glfw3.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <iostream>
#include <string>

#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// camera — constructed inside main() once the real monitor resolution is known,
// but declared as a global pointer so processInput/mouse_callback/etc. can reach it
Camera* ourCamera = nullptr;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// cursor capture state
enum class CursorMode { Captured, Free };
CursorMode cursorMode = CursorMode::Free;

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
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "LearnOpenGL", primaryMonitor, NULL);
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // start with a normal, visible cursor — it only gets captured once the user clicks
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // now that we know the real resolution, construct the camera
    ourCamera = new Camera(mode->width, mode->height);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Model loading
    // Loaded here (before stbi_set_flip_vertically_on_load below), so their
    // textures load un-flipped, matching Assimp's aiProcess_FlipUVs — flipping
    // both would cancel out and leave the models' UVs wrong again.

    // load models
    Model Terrain("Resources/mesh_final.obj");
    Model Backpack("Resources/Backpack/backpack.obj");
    Model Ball("Resources/Pallone/Ball.obj");

    // vector to store models
    vector<Model> models;
    models.push_back(Terrain);
    models.push_back(Backpack);
    models.push_back(Ball);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // tell stb_image to flip loaded textures on the y-axis so they match OpenGL's UV convention
    // (only affects textures loaded after this point, i.e. the cube's textures below)
    stbi_set_flip_vertically_on_load(true);

    // build and compile our shader program
    // ------------------------------------
    Shader objectShader("MultipleLights.vs", "MultipleLights.fs");
    Shader lightingShader("lightingShader.vs", "lightingShader.fs");
    Shader modelShader("Model.vs", "Model.fs");


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // cube positions
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,   0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f,  -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f,  -3.5f),
        glm::vec3(-1.7f,  3.0f,  -7.5f),
        glm::vec3(1.3f, -2.0f,  -2.5f),
        glm::vec3(1.5f,  2.0f,  -2.5f),
        glm::vec3(1.5f,  0.2f,  -1.5f),
        glm::vec3(-1.3f,  1.0f,  -1.5f)
    };

    // point light positions
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,   2.0f),
        glm::vec3(2.3f, -3.3f,  -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f,  -3.0f)
    };

    // Variables
    // ---------
    glm::vec3 LightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 LightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
    float AmbientStrength = 0.2f;
    float SpecularStrength = 5.0f;
    int Shininess = 32;
    bool isAmbientOn = true;
    bool isDiffuseOn = true;
    bool isSpecularOn = true;
    int LightType = 0; // 0 = Directional, 1 = Point, 2 = Flash (used in Custom scene)
    int SceneMode = 0; // 0 = Preset scene (dir + 4 points + flashlight), 1 = Custom (pick one light type)
    bool flashlightOn = true; // flashlight toggle in the Preset scene
    float LightConstant = 1.0f;
    float LightLinear = 0.09f;
    float LightQuadratic = 0.032f;
    float CutOff = glm::cos(glm::radians(12.5f));       // stored as cosine
    float OuterCutOff = glm::cos(glm::radians(17.5f));  // stored as cosine

    // per-model transform (position/scale on screen)
    glm::vec3 TerrainPosition = glm::vec3(-2.55f, -0.85f, 0.0f);
    glm::vec3 BackpackPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 BallPosition = glm::vec3(2.0f, 0.0f, 0.0f);
    float TerrainScale = 0.5f;
    float BackpackScale = 0.5f;
    float BallScale = 1.0f;

    // ----------------  VBO , VAO configuration
    // Cube VBO and VAO
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Light VAO
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load textures
    // -------------
    unsigned int diffuseMap = loadTexture("Resources/container2.png");
    unsigned int specularMap = loadTexture("Resources/container2_specular.png");

    // Dear ImGui initialization
    // -------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // ImGui frame start
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // shared view/projection transforms, used by every shader this frame
        glm::mat4 projection = glm::perspective(glm::radians(ourCamera->getFov()),
            (float)mode->width / (float)mode->height, 0.1f, 100.0f);

        // light colours; the Ambient / Diffuse / Specular checkboxes are applied here
        glm::vec3 amb = LightColor * AmbientStrength * (isAmbientOn ? 1.0f : 0.0f);
        glm::vec3 dif = LightColor * (isDiffuseOn ? 1.0f : 0.0f);
        glm::vec3 spc = LightColor * SpecularStrength * (isSpecularOn ? 1.0f : 0.0f);
        glm::vec3 off(0.0f);

        bool presetScene = (SceneMode == 0);

        // Preset scene: fixed values for every light, all on together (flashlight is toggleable).
        // Custom scene: only the selected light type is on, driven by the GUI values.
        bool dirOn = presetScene || (LightType == 0);
        bool pointOn = presetScene || (LightType == 1);
        bool spotOn = presetScene ? flashlightOn : (LightType == 2);

        // ---------------------------------------------------------
        // objectShader: the row of textured cubes
        // ---------------------------------------------------------
        objectShader.use();
        objectShader.setVec3("viewPos", ourCamera->getPosition());

        // directional light
        objectShader.setVec3("dirLight.direction", presetScene ? glm::vec3(-0.2f, -1.0f, -0.3f) : LightDirection);
        objectShader.setVec3("dirLight.ambient", presetScene ? glm::vec3(0.05f, 0.05f, 0.05f) : (dirOn ? amb : off));
        objectShader.setVec3("dirLight.diffuse", presetScene ? glm::vec3(0.4f, 0.4f, 0.4f) : (dirOn ? dif : off));
        objectShader.setVec3("dirLight.specular", presetScene ? glm::vec3(0.5f, 0.5f, 0.5f) : (dirOn ? spc : off));

        // point lights (all four)
        for (int i = 0; i < 4; i++)
        {
            std::string p = "pointLights[" + std::to_string(i) + "]";
            objectShader.setVec3(p + ".position", pointLightPositions[i]);
            objectShader.setFloat(p + ".constant", presetScene ? 1.0f : LightConstant);
            objectShader.setFloat(p + ".linear", presetScene ? 0.09f : LightLinear);
            objectShader.setFloat(p + ".quadratic", presetScene ? 0.032f : LightQuadratic);
            objectShader.setVec3(p + ".ambient", presetScene ? glm::vec3(0.05f, 0.05f, 0.05f) : (pointOn ? amb : off));
            objectShader.setVec3(p + ".diffuse", presetScene ? glm::vec3(0.8f, 0.8f, 0.8f) : (pointOn ? dif : off));
            objectShader.setVec3(p + ".specular", presetScene ? glm::vec3(1.0f, 1.0f, 1.0f) : (pointOn ? spc : off));
        }

        // spotlight = flashlight, follows the camera
        objectShader.setVec3("spotLight.position", ourCamera->getPosition());
        objectShader.setVec3("spotLight.direction", ourCamera->getFront());
        objectShader.setFloat("spotLight.cutOff", presetScene ? glm::cos(glm::radians(12.5f)) : CutOff);
        objectShader.setFloat("spotLight.outerCutOff", presetScene ? glm::cos(glm::radians(15.0f)) : OuterCutOff);
        objectShader.setFloat("spotLight.constant", presetScene ? 1.0f : LightConstant);
        objectShader.setFloat("spotLight.linear", presetScene ? 0.09f : LightLinear);
        objectShader.setFloat("spotLight.quadratic", presetScene ? 0.032f : LightQuadratic);
        objectShader.setVec3("spotLight.ambient", (!presetScene && spotOn) ? amb : off); // preset flashlight has no ambient
        objectShader.setVec3("spotLight.diffuse", spotOn ? (presetScene ? glm::vec3(1.0f) : dif) : off);
        objectShader.setVec3("spotLight.specular", spotOn ? (presetScene ? glm::vec3(1.0f) : spc) : off);

        // texture units
        objectShader.setInt("material.diffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        objectShader.setInt("material.specular", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // material properties
        objectShader.setFloat("material.shininess", (float)Shininess);

        // view/projection transformations
        ourCamera->Use(objectShader);
        objectShader.setMat4("projection", projection);

        // render the cubes
        glBindVertexArray(cubeVAO);
        for (int i = 0; i < 10; i++)
        {
            glm::mat4 cubeModel = glm::mat4(1.0f);
            float angle = 2.0f * i;

            cubeModel = glm::translate(cubeModel, cubePositions[i]);
            cubeModel = glm::rotate(cubeModel, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            objectShader.setMat4("model", cubeModel);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // draw the lamp cubes, only in Point mode (one per point light)
        if (pointOn)
        {
            lightingShader.use();
            lightingShader.setVec3("LightColor", LightColor);
            lightingShader.setMat4("projection", projection);
            ourCamera->Use(lightingShader);

            glBindVertexArray(lightCubeVAO);
            for (int i = 0; i < 4; i++)
            {
                glm::mat4 lampModel = glm::translate(glm::mat4(1.0f), pointLightPositions[i]);
                lampModel = glm::scale(lampModel, glm::vec3(0.2f)); // a smaller cube
                lightingShader.setMat4("model", lampModel);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // ---------------------------------------------------------
        // modelShader: the loaded Models
        // ---------------------------------------------------------
        modelShader.use();
        modelShader.setVec3("viewPos", ourCamera->getPosition());

        modelShader.setVec3("dirLight.direction", presetScene ? glm::vec3(-0.2f, -1.0f, -0.3f) : LightDirection);
        modelShader.setVec3("dirLight.ambient", presetScene ? glm::vec3(0.05f, 0.05f, 0.05f) : (dirOn ? amb : off));
        modelShader.setVec3("dirLight.diffuse", presetScene ? glm::vec3(0.4f, 0.4f, 0.4f) : (dirOn ? dif : off));
        modelShader.setVec3("dirLight.specular", presetScene ? glm::vec3(0.5f, 0.5f, 0.5f) : (dirOn ? spc : off));

        for (int i = 0; i < 4; i++)
        {
            std::string p = "pointLights[" + std::to_string(i) + "]";
            modelShader.setVec3(p + ".position", pointLightPositions[i]);
            modelShader.setFloat(p + ".constant", presetScene ? 1.0f : LightConstant);
            modelShader.setFloat(p + ".linear", presetScene ? 0.09f : LightLinear);
            modelShader.setFloat(p + ".quadratic", presetScene ? 0.032f : LightQuadratic);
            modelShader.setVec3(p + ".ambient", presetScene ? glm::vec3(0.05f, 0.05f, 0.05f) : (pointOn ? amb : off));
            modelShader.setVec3(p + ".diffuse", presetScene ? glm::vec3(0.8f, 0.8f, 0.8f) : (pointOn ? dif : off));
            modelShader.setVec3(p + ".specular", presetScene ? glm::vec3(1.0f, 1.0f, 1.0f) : (pointOn ? spc : off));
        }

        // spotlight = flashlight, follows the camera
        modelShader.setVec3("spotLight.position", ourCamera->getPosition());
        modelShader.setVec3("spotLight.direction", ourCamera->getFront());
        modelShader.setFloat("spotLight.cutOff", presetScene ? glm::cos(glm::radians(12.5f)) : CutOff);
        modelShader.setFloat("spotLight.outerCutOff", presetScene ? glm::cos(glm::radians(15.0f)) : OuterCutOff);
        modelShader.setFloat("spotLight.constant", presetScene ? 1.0f : LightConstant);
        modelShader.setFloat("spotLight.linear", presetScene ? 0.09f : LightLinear);
        modelShader.setFloat("spotLight.quadratic", presetScene ? 0.032f : LightQuadratic);
        modelShader.setVec3("spotLight.ambient", (!presetScene && spotOn) ? amb : off); // preset flashlight has no ambient
        modelShader.setVec3("spotLight.diffuse", spotOn ? (presetScene ? glm::vec3(1.0f) : dif) : off);
        modelShader.setVec3("spotLight.specular", spotOn ? (presetScene ? glm::vec3(1.0f) : spc) : off);

        // material property (Model.fs uses a plain "shininess" uniform, not material.shininess)
        modelShader.setFloat("shininess", (float)Shininess);

        // view/projection transformations
        ourCamera->Use(modelShader);
        modelShader.setMat4("projection", projection);

        // place, scale, and draw each model with ITS OWN transform.
        // models[0] = Terrain, models[1] = Backpack, models[2] = Ball — this order
        // must match the push_back order above.
        glm::vec3 modelPositions[3] = { TerrainPosition, BackpackPosition, BallPosition };
        float     modelScales[3] = { TerrainScale,    BackpackScale,    BallScale };

        for (unsigned int i = 0; i < models.size(); i++)
        {
            glm::mat4 modelMat = glm::mat4(1.0f);
            modelMat = glm::translate(modelMat, modelPositions[i]);
            modelMat = glm::scale(modelMat, glm::vec3(modelScales[i]));
            modelShader.setMat4("model", modelMat);

            models[i].Draw(modelShader);
        }

        // ImGui Code
        ImGui::SetNextWindowPos(ImVec2(5, 5));
        ImGui::Begin("Control Menu", nullptr, ImGuiWindowFlags_NoMove);
        ImGui::Text("Scene : ");
        ImGui::RadioButton("Preset lights", &SceneMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Custom", &SceneMode, 1);
        ImGui::Separator();

        // shininess is a material property, shared by both scenes
        ImGui::Text("Shininess : ");
        ImGui::RadioButton("2", &Shininess, 2);     ImGui::SameLine();
        ImGui::RadioButton("4", &Shininess, 4);     ImGui::SameLine();
        ImGui::RadioButton("8", &Shininess, 8);     ImGui::SameLine();
        ImGui::RadioButton("16", &Shininess, 16);
        ImGui::RadioButton("32", &Shininess, 32);   ImGui::SameLine();
        ImGui::RadioButton("64", &Shininess, 64);   ImGui::SameLine();
        ImGui::RadioButton("128", &Shininess, 128); ImGui::SameLine();
        ImGui::RadioButton("256", &Shininess, 256);
        ImGui::Separator();

        ImGui::Text("Terrain : ");
        ImGui::PushID("Terrain");
        ImGui::DragFloat3("Position", glm::value_ptr(TerrainPosition), 0.05f);
        ImGui::DragFloat("Scale", &TerrainScale, 0.01f, 0.01f, 10.0f);
        ImGui::PopID();

        ImGui::Text("Backpack : ");
        ImGui::PushID("Backpack");
        ImGui::DragFloat3("Position", glm::value_ptr(BackpackPosition), 0.05f);
        ImGui::DragFloat("Scale", &BackpackScale, 0.01f, 0.01f, 10.0f);
        ImGui::PopID();

        ImGui::Text("Ball : ");
        ImGui::PushID("Ball");
        ImGui::DragFloat3("Position", glm::value_ptr(BallPosition), 0.05f);
        ImGui::DragFloat("Scale", &BallScale, 0.01f, 0.01f, 10.0f);
        ImGui::PopID();
        ImGui::Separator();

        if (SceneMode == 0)
        {
            // Preset scene: directional + 4 point lights + flashlight
            ImGui::Text("Preset Scene");
            ImGui::Checkbox("Flashlight", &flashlightOn);
        }
        else
        {
            // Custom scene: pick one light type and tweak it
            ImGui::ColorEdit3("Light Color", glm::value_ptr(LightColor));
            ImGui::SliderFloat("Ambient Strength", &AmbientStrength, 0.1f, 1.0f);
            ImGui::SliderFloat("Specular Strength", &SpecularStrength, 0.1f, 10.0f);
            ImGui::Checkbox("Ambient", &isAmbientOn);
            ImGui::Checkbox("Diffuse", &isDiffuseOn);
            ImGui::Checkbox("Specular", &isSpecularOn);

            ImGui::Separator();
            ImGui::RadioButton("Directional", &LightType, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Point", &LightType, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Flash", &LightType, 2);
            ImGui::Separator();

            if (LightType == 0)
            {
                // Directional-only controls
                ImGui::Text("Directional Light Settings");
                ImGui::DragFloat3("Light Direction", glm::value_ptr(LightDirection), 0.05f);
            }
            else if (LightType == 1)
            {
                // Point-only controls (positions are fixed in pointLightPositions)
                ImGui::Text("Point Light Settings");
                ImGui::SliderFloat("Constant", &LightConstant, 0.0f, 1.0f);
                ImGui::SliderFloat("Linear", &LightLinear, 0.0f, 1.0f);
                ImGui::SliderFloat("Quadratic", &LightQuadratic, 0.0f, 1.0f);
            }
            else if (LightType == 2)
            {
                // Flashlight: position and direction come from the camera
                ImGui::Text("Flashlight Settings");
                ImGui::SliderFloat("Cutoff Angle", &CutOff, 0.0f, 1.0f);
                ImGui::SliderFloat("Outer Cutoff Angle", &OuterCutOff, 0.0f, 1.0f);
                ImGui::SliderFloat("Constant", &LightConstant, 0.0f, 1.0f);
                ImGui::SliderFloat("Linear", &LightLinear, 0.0f, 1.0f);
                ImGui::SliderFloat("Quadratic", &LightQuadratic, 0.0f, 1.0f);
            }
        }

        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete ourCamera;
    ourCamera = nullptr;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    // edge-triggered ESC: first press frees the cursor, second press (while free) closes the window
    static bool escWasPressed = false;
    bool escPressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    if (escPressed && !escWasPressed)
    {
        if (cursorMode == CursorMode::Captured)
        {
            cursorMode = CursorMode::Free;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetWindowShouldClose(window, true);
        }
    }
    escWasPressed = escPressed;

    // don't move the camera while the cursor is free (e.g. interacting with ImGui)
    if (cursorMode != CursorMode::Captured)
        return;

    glm::vec3 currentPos = ourCamera->getPosition();
    glm::vec3 front = ourCamera->getFront();
    glm::vec3 right = glm::normalize(glm::cross(front, ourCamera->getUp()));

    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ourCamera->setPosition(currentPos.x + front.x * cameraSpeed,
            currentPos.y + front.y * cameraSpeed,
            currentPos.z + front.z * cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ourCamera->setPosition(currentPos.x - front.x * cameraSpeed,
            currentPos.y - front.y * cameraSpeed,
            currentPos.z - front.z * cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ourCamera->setPosition(currentPos.x - right.x * cameraSpeed,
            currentPos.y - right.y * cameraSpeed,
            currentPos.z - right.z * cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ourCamera->setPosition(currentPos.x + right.x * cameraSpeed,
            currentPos.y + right.y * cameraSpeed,
            currentPos.z + right.z * cameraSpeed);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // only drive camera look while the cursor is actually captured;
    // otherwise Camera's internal last-position tracking would jump around
    // while the user is just moving the mouse over ImGui
    if (cursorMode != CursorMode::Captured)
        return;

    Camera::mouse_callback(window, xpos, ypos);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Camera::scroll_callback(window, xoffset, yoffset);
}

// glfw: click into the game to capture the cursor for camera look
// -----------------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // don't capture if ImGui wants this click (e.g. clicking on a window/widget)
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS
        && cursorMode == CursorMode::Free)
    {
        cursorMode = CursorMode::Captured;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}