#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct COLOR {
    float r;
    float g;
    float b;
};
typedef struct COLOR COLOR;

struct Sprite {
    string name;
    COLOR color;
    float x,y;
    VAO* object;
    int status;
    float height,width;
    float x_speed,y_speed;
    float angle; //Current Angle (Actual rotated angle of the object)
    int inAir;
    float radius;
    int fixed;
    float friction; //Value from 0 to 1
    int health;
    int isRotating;
    int direction; //0 for clockwise and 1 for anticlockwise for animation
    float remAngle; //the remaining angle to finish animation
    int isMovingAnim;
    int dx;
    int dy;
    float weight;
};
typedef struct Sprite Sprite;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

map <string, Sprite> objects;
map <string, Sprite> cannonObjects; //Only store cannon components here
map <string, Sprite> coins;
map <string, Sprite> backgroundObjects;
map <string, Sprite> goalObjects;

map <string, Sprite> pig1Objects; //Pig1
map <string, Sprite> pig2Objects; //Pig2
map <string, Sprite> pig3Objects; //Pig3
map <string, Sprite> pig4Objects; //Pig4

map <string, Sprite> char1Objects; //The score displayed on top right of the screen
map <string, Sprite> char2Objects;
map <string, Sprite> char3Objects;
map <string, Sprite> char4Objects;

map <string, Sprite> charscoreObjects1; //The score that appears as a popup on top of objects
map <string, Sprite> charscoreObjects2;
map <string, Sprite> charscoreObjects3;

//We draw each character of the scorelabel string by moving the x position each time.
map <string, Sprite> scoreLabelObjects; //The score label
string scoreLabel;
float scoreLabel_x;
float scoreLabel_y;

map <string, Sprite> timerObjects; //The timer
float timer_x;
float timer_y;

map <string, Sprite> endLabelObjects; //The you win/lose label
string endLabel;
float endLabel_x;
float endLabel_y;

map <string, Sprite> *characters[10]; //For displaying text
char characterValues[10];
float characterPosX[10];
float characterPosY[10];

int scoreDrawTimer=0;
int player_score=0;
float x_change = 0; //For the camera pan
float y_change = 0; //For the camera pan
float zoom_camera = 1;
float gravity = 1;
float airResistance = 0.2/15;
int player_reset_timer=0;
double click_time=0;
float game_over=0;
float game_start_timer=0;
int game_timer=90;

pair<float,float> moveObject(string name, float dx, float dy) {
    objects[name].x+=dx;
    objects[name].y+=dy;
    return make_pair(objects[name].x,objects[name].y);
}

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, GLfloat* vertex_buffer_data, GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, GLfloat* vertex_buffer_data, GLfloat red, GLfloat green, GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

int player_status=0; //0 is ready to play, 1 is not ready yet

double launch_power=0;
double launch_angle=0;
int keyboard_pressed=0;

void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset==-1) {
        zoom_camera /= 1.1; //make it bigger than current size
    }
    else if(yoffset==1){
        zoom_camera *= 1.1; //make it bigger than current size
    }
    if (zoom_camera<=1) {
        zoom_camera = 1;
    }
    if (zoom_camera>=4) {
        zoom_camera=4;
    }
    if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
    else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
    if(y_change-300.0f/zoom_camera<-300)
        y_change=-300+300.0f/zoom_camera;
    else if(y_change+300.0f/zoom_camera>300)
        y_change=300-300.0f/zoom_camera;
    Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(-300.0f/zoom_camera+y_change), (float)(300.0f/zoom_camera+y_change), 0.1f, 500.0f);
}

//Ensure the panning does not go out of the map
void check_pan(){
    if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
    else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
    if(y_change-300.0f/zoom_camera<-300)
        y_change=-300+300.0f/zoom_camera;
    else if(y_change+300.0f/zoom_camera>300)
        y_change=300-300.0f/zoom_camera;
}

void initKeyboard(){
    if(keyboard_pressed==0){
        cout << "START KEYBOARD" << endl;
        keyboard_pressed=1;
        cannonObjects["cannonaim"].status=1;
        backgroundObjects["cannonpowerdisplay"].status=1;
        launch_power=(760*760+560*560)/10;
    }
}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_UP:
                mousescroll(window,0,+1);
                check_pan();
                break;
            case GLFW_KEY_DOWN:
                mousescroll(window,0,-1);
                check_pan();
                break;
            case GLFW_KEY_RIGHT:
                x_change+=10;
                check_pan();
                break;
            case GLFW_KEY_LEFT:
                x_change-=10;
                check_pan();
                break;
            case GLFW_KEY_N:
                y_change+=10;
                check_pan();
                break;
            case GLFW_KEY_M:
                y_change-=10;
                check_pan();
                break;
            case GLFW_KEY_S:
                initKeyboard();
                if(launch_power>(760*760+560*560)/10)
                    launch_power-=(760*760+560*560)/10;
                else
                    launch_power=0;
                break;
            case GLFW_KEY_F:
                initKeyboard();
                if(launch_power<760*760+560*560-(760*760+560*560)/10)
                    launch_power+=(760*760+560*560)/10;
                else
                    launch_power=760*760+560*560;
                break;
            case GLFW_KEY_A:
                initKeyboard();
                if(launch_angle<90-10)
                    launch_angle+=10;
                else
                    launch_angle=90;
                break;
            case GLFW_KEY_B:
                initKeyboard();
                if(launch_angle>10)
                    launch_angle-=10;
                else
                    launch_angle=0;
                break;
            case GLFW_KEY_SPACE:
                cout << "END KEYBOARD" << endl;
                click_time=glfwGetTime();
                keyboard_pressed=0;
                backgroundObjects["cannonpowerdisplay"].status=0;
                cannonObjects["cannonaim"].status=0;
                if(player_status==0){
                    player_status=1;
                    if(objects["cannonball"].inAir == 0){
                        objects["cannonball"].inAir = 1;
                        objects["cannonball"].x = -315+cos(launch_angle*(M_PI/180))*cannonObjects["cannonrectangle"].width;
                        objects["cannonball"].y = -210+sin(launch_angle*(M_PI/180))*cannonObjects["cannonrectangle"].width;
                        //Set max jump speeds here (currently 300 and 300) (Adjust these as required)
                        //Also adjust the sensitivity of the mouse drag as required
                        objects["cannonball"].y_speed = min((abs(launch_power*10/89120)*sin(launch_angle*(M_PI/180))),30.0);
                        objects["cannonball"].x_speed = min((abs(launch_power*10/89120)*cos(launch_angle*(M_PI/180))),30.0);
                        for(map<string,Sprite>::iterator it=cannonObjects.begin();it!=cannonObjects.end();it++){
                            string current = it->first; //The name of the current object
                            cannonObjects[current].dx=16;
                            cannonObjects[current].isMovingAnim=1;
                        }
                    }
                }
                break;
            case GLFW_KEY_C:
                break;
            case GLFW_KEY_P:
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            case GLFW_KEY_R:
                objects["cannonball"].y=-240;
                objects["cannonball"].x=-315;
                objects["cannonball"].inAir=0;
                for(map<string,Sprite>::iterator it=cannonObjects.begin();it!=cannonObjects.end();it++){
                    string current = it->first; //The name of the current object
                    if(cannonObjects[current].isMovingAnim==1){
                        cannonObjects[current].x+=16-cannonObjects[current].dx;
                        cannonObjects[current].isMovingAnim=0;
                        cannonObjects[current].dx=0;
                    }
                    if(cannonObjects[current].isMovingAnim==2){
                        cannonObjects[current].x+=cannonObjects[current].dx;
                        cannonObjects[current].isMovingAnim=0;
                        cannonObjects[current].dx=0;
                    }
                }
                player_status=0;
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        default:
            break;
    }
}

int mouse_clicked=0;
int right_mouse_clicked=0;
double mouse_x,mouse_y;
double mouse_x_old,mouse_y_old;

void mouse_click(){
    mouse_clicked=1;
    keyboard_pressed=0;
    cannonObjects["cannonaim"].status=1;
    backgroundObjects["cannonpowerdisplay"].status=1;
}

void mouse_release(GLFWwindow* window, int button){
    mouse_clicked=0;
    backgroundObjects["cannonpowerdisplay"].status=0;
    cannonObjects["cannonaim"].status=0;
    if(player_status==0){
        player_status=1;
        glfwGetCursorPos(window,&mouse_x,&mouse_y);
        if(objects["cannonball"].inAir == 0){
            objects["cannonball"].inAir = 1;
            float angle=cannonObjects["cannonrectangle"].angle*(M_PI/180.0);
            //Adjust the sensitivity of the mouse drag as required
            objects["cannonball"].x = -315+cos(angle)*cannonObjects["cannonrectangle"].width;
            objects["cannonball"].y = -210+sin(angle)*cannonObjects["cannonrectangle"].width;
            click_time=glfwGetTime();
            objects["cannonball"].y_speed = min((543-mouse_y)/15+3.0,30.0);
            objects["cannonball"].x_speed = min((mouse_x-77)/15+3.0,30.0);
            for(map<string,Sprite>::iterator it=cannonObjects.begin();it!=cannonObjects.end();it++){
                string current = it->first; //The name of the current object
                cannonObjects[current].dx=16;
                cannonObjects[current].isMovingAnim=1;
            }
        }
    }
}
glm::vec3 FindCurrentDirection(glm::vec3 A,glm::vec3 B)
{
    glm::vec3 C = A-B ;
    if(C == glm::vec3(0,0,0)) return glm::vec3(1,0,0) ;
    return normalize(C) ;
}
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  switch (button)
  {
    case GLFW_MOUSE_BUTTON_LEFT:
    if (action == GLFW_RELEASE)
    Laser() ;
      break;
      case GLFW_MOUSE_BUTTON_RIGHT:
      if (action == GLFW_RELEASE) {                 }
      break;
      default:
      break;
    }
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-400.0f/zoom_camera, 400.0f/zoom_camera, -300.0f/zoom_camera, 300.0f/zoom_camera, 0.1f, 500.0f);
}


// Creates the triangle object used in this sample code
void createTriangle (string name, float weight, COLOR color, float x[], float y[], string component, int fill)
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    float xc=(x[0]+x[1]+x[2])/3;
    float yc=(y[0]+y[1]+y[2])/3;
    GLfloat vertex_buffer_data [] = {
        x[0]-xc,y[0]-yc,0, // vertex 0
        x[1]-xc,y[1]-yc,0, // vertex 1
        x[2]-xc,y[2]-yc,0 // vertex 2
    };

    GLfloat color_buffer_data [] = {
        color.r,color.g,color.b, // color 1
        color.r,color.g,color.b, // color 2
        color.r,color.g,color.b // color 3
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    VAO *triangle;
    if(fill==1)
        triangle=create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
    else
        triangle=create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
    Sprite vishsprite = {};
    vishsprite.color = color;
    vishsprite.name = name;
    vishsprite.object = triangle;
    vishsprite.x=(x[0]+x[1]+x[2])/3; //Position of the sprite is the position of the centroid
    vishsprite.y=(y[0]+y[1]+y[2])/3;
    vishsprite.height=-1; //Height of the sprite is undefined
    vishsprite.width=-1; //Width of the sprite is undefined
    vishsprite.status=1;
    vishsprite.inAir=0;
    vishsprite.x_speed=0;
    vishsprite.y_speed=0;
    vishsprite.radius=-1; //The bounding circle radius is not defined.
    vishsprite.fixed=0;
    vishsprite.friction=0.4;
    vishsprite.health=100;
    vishsprite.weight=weight;
    if(component=="cannon")
        cannonObjects[name]=vishsprite;
    else if(component=="background")
        backgroundObjects[name]=vishsprite;
    else if(component=="goal")
        goalObjects[name]=vishsprite;
    else
        objects[name]=vishsprite;
}

// Creates the rectangle object used in this sample code
void createRectangle (string name, float weight, COLOR colorA, COLOR colorB, COLOR colorC, COLOR colorD, float x, float y, float height, float width, string component)
{
    // GL3 accepts only Triangles. Quads are not supported
    float w=width/2,h=height/2;
    GLfloat vertex_buffer_data [] = {
        -w,-h,0, // vertex 1
        -w,h,0, // vertex 2
        w,h,0, // vertex 3

        w,h,0, // vertex 3
        w,-h,0, // vertex 4
        -w,-h,0  // vertex 1
    };

    GLfloat color_buffer_data [] = {
        colorA.r,colorA.g,colorA.b, // color 1
        colorB.r,colorB.g,colorB.b, // color 2
        colorC.r,colorC.g,colorC.b, // color 3

        colorC.r,colorC.g,colorC.b, // color 4
        colorD.r,colorD.g,colorD.b, // color 5
        colorA.r,colorA.g,colorA.b // color 6
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    VAO *rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    Sprite vishsprite = {};
    vishsprite.color = colorA;
    vishsprite.name = name;
    vishsprite.object = rectangle;
    vishsprite.x=x;
    vishsprite.y=y;
    vishsprite.height=height;
    vishsprite.width=width;
    vishsprite.status=1;
    vishsprite.inAir=0;
    vishsprite.x_speed=0;
    vishsprite.y_speed=0;
    vishsprite.fixed=0;
    vishsprite.radius=(sqrt(height*height+width*width))/2;
    vishsprite.friction=0.4;
    vishsprite.health=100;
    vishsprite.weight=weight;
    //All the different layers
    if(component=="cannon")
        cannonObjects[name]=vishsprite;
    else if(component=="background")
        backgroundObjects[name]=vishsprite;
    else if(component=="goal")
        goalObjects[name]=vishsprite;
    else if(component=="pig1")
        pig1Objects[name]=vishsprite;
    else if(component=="pig2")
        pig2Objects[name]=vishsprite;
    else if(component=="pig3")
        pig3Objects[name]=vishsprite;
    else if(component=="pig4")
        pig4Objects[name]=vishsprite;
    else if(component=="char1")
        char1Objects[name]=vishsprite;
    else if(component=="char2")
        char2Objects[name]=vishsprite;
    else if(component=="char3")
        char3Objects[name]=vishsprite;
    else if(component=="char4")
        char4Objects[name]=vishsprite;
    else if(component=="charscore1")
        charscoreObjects1[name]=vishsprite;
    else if(component=="charscore2")
        charscoreObjects2[name]=vishsprite;
    else if(component=="charscore3")
        charscoreObjects3[name]=vishsprite;
    else if(component=="scorelabel")
        scoreLabelObjects[name]=vishsprite;
    else if(component=="endlabel")
        endLabelObjects[name]=vishsprite;
    else if(component=="timelabel")
        timerObjects[name]=vishsprite;
    else
        objects[name]=vishsprite;
}

void createCircle (string name, float weight, COLOR color, float x, float y, float r, int NoOfParts, string component, int fill)
{
    int parts = NoOfParts;
    float radius = r;
    GLfloat vertex_buffer_data[parts*9];
    GLfloat color_buffer_data[parts*9];
    int i,j;
    float angle=(2*M_PI/parts);
    float current_angle = 0;
    for(i=0;i<parts;i++){
        for(j=0;j<3;j++){
            color_buffer_data[i*9+j*3]=color.r;
            color_buffer_data[i*9+j*3+1]=color.g;
            color_buffer_data[i*9+j*3+2]=color.b;
        }
        vertex_buffer_data[i*9]=0;
        vertex_buffer_data[i*9+1]=0;
        vertex_buffer_data[i*9+2]=0;
        vertex_buffer_data[i*9+3]=radius*cos(current_angle);
        vertex_buffer_data[i*9+4]=radius*sin(current_angle);
        vertex_buffer_data[i*9+5]=0;
        vertex_buffer_data[i*9+6]=radius*cos(current_angle+angle);
        vertex_buffer_data[i*9+7]=radius*sin(current_angle+angle);
        vertex_buffer_data[i*9+8]=0;
        current_angle+=angle;
    }
    VAO* circle;
    if(fill==1)
        circle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);
    else
        circle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_LINE);
    Sprite vishsprite = {};
    vishsprite.color = color;
    vishsprite.name = name;
    vishsprite.object = circle;
    vishsprite.x=x;
    vishsprite.y=y;
    vishsprite.height=2*r; //Height of the sprite is 2*r
    vishsprite.width=2*r; //Width of the sprite is 2*r
    vishsprite.status=1;
    vishsprite.inAir=0;
    vishsprite.x_speed=0;
    vishsprite.y_speed=0;
    vishsprite.radius=r;
    vishsprite.fixed=0;
    vishsprite.friction=0.4;
    vishsprite.health=100;
    vishsprite.weight=weight;
    if(component=="cannon")
        cannonObjects[name]=vishsprite;
    else if(component=="coin")
        coins[name]=vishsprite;
    else if(component=="background")
        backgroundObjects[name]=vishsprite;
    else if(component=="goal")
        goalObjects[name]=vishsprite;
    else if(component=="pig1")
        pig1Objects[name]=vishsprite;
    else if(component=="pig2")
        pig2Objects[name]=vishsprite;
    else if(component=="pig3")
        pig3Objects[name]=vishsprite;
    else if(component=="pig4")
        pig4Objects[name]=vishsprite;
    else
        objects[name]=vishsprite;
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

int checkCollisionRight(Sprite col_object, Sprite my_object){
    if(col_object.x>my_object.x && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.x+col_object.width/2>my_object.x-my_object.width/2){
        return 1;
    }
    return 0;
}

int checkCollisionLeft(Sprite col_object, Sprite my_object){
    if(col_object.x<my_object.x && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2){
        return 1;
    }
    return 0;
}

int checkCollisionTop(Sprite col_object, Sprite my_object){
    if(col_object.y>my_object.y && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.y+col_object.height/2>my_object.y-my_object.height/2){
        return 1;
    }
    return 0;
}

int checkCollisionBottom(Sprite col_object, Sprite my_object){
    if(col_object.y<my_object.y && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2){
        return 1;
    }
    return 0;
}


//Check collisions between rectangles only
//Bounding boxes collision
//Best Method
int checkCollision(string name, float dx, float dy){
    int any_collide=0;
    if(name=="cannonball"){
        if(checkCollisionBottom(objects["springbase2"],objects["cannonball"])){
            if(objects["springbase2"].isMovingAnim==0){
                objects["springbase2"].isMovingAnim=1;
                objects["springbase2"].dy=15;
                objects["springbase3"].isMovingAnim=1;
                objects["springbase3"].dy=15;
            }
        }
        for(map<string,Sprite>::iterator it2=coins.begin();it2!=coins.end();it2++){
            Sprite col_object=coins[it2->first];
            Sprite my_object=objects["cannonball"];
            if(col_object.status==0)
                continue;
            if((dx>0 && checkCollisionRight(col_object,my_object)) || (dx<0 && checkCollisionLeft(col_object,my_object)) || (dy>0 && checkCollisionTop(col_object,my_object)) || (dy<0 && checkCollisionBottom(col_object,my_object))){
                coins[it2->first].status=0;
                cout <<" COIN " << endl;
                scoreDrawTimer=50;
                player_score+=100;
                backgroundObjects["scorebackground"].status=1;
                backgroundObjects["scorebackground"].x=it2->second.x+5;
                backgroundObjects["scorebackground"].y=it2->second.y+it2->second.height/2+15;
                characterValues[4]='1';
                characterValues[5]='0';
                characterValues[6]='0';
                characterPosX[4]=it2->second.x-20;
                characterPosY[4]=it2->second.y+it2->second.height/2+15;
                characterPosX[5]=it2->second.x;
                characterPosY[5]=it2->second.y+it2->second.height/2+15;
                characterPosX[6]=it2->second.x+20;
                characterPosY[6]=it2->second.y+it2->second.height/2+15;
            }
        }
        for(map<string,Sprite>::iterator it2=goalObjects.begin();it2!=goalObjects.end();it2++){
            Sprite col_object=goalObjects[it2->first];
            Sprite my_object=objects["cannonball"];
            if(col_object.status==0)
                continue;
            if((dx>0 && checkCollisionRight(col_object,my_object)) || (dx<0 && checkCollisionLeft(col_object,my_object)) || (dy>0 && checkCollisionTop(col_object,my_object)) || (dy<0 && checkCollisionBottom(col_object,my_object))){
                goalObjects[it2->first].status=0;
                cout <<" GOAL OBTAINED" << endl;
                scoreDrawTimer=50;
                player_score+=200;
                backgroundObjects["scorebackground"].status=1;
                backgroundObjects["scorebackground"].x=it2->second.x;
                backgroundObjects["scorebackground"].y=it2->second.y+it2->second.height/2+15;
                characterValues[4]='2';
                characterValues[5]='0';
                characterValues[6]='0';
                characterPosX[4]=it2->second.x-20;
                characterPosY[4]=it2->second.y+it2->second.height/2+15;
                characterPosX[5]=it2->second.x;
                characterPosY[5]=it2->second.y+it2->second.height/2+15;
                characterPosX[6]=it2->second.x+20;
                characterPosY[6]=it2->second.y+it2->second.height/2+15;
            }
        }
    }

    for(map<string,Sprite>::iterator it2=objects.begin();it2!=objects.end();it2++){
        int collide=0;
        string colliding = it2->first;
        Sprite col_object = it2->second;
        Sprite my_object = objects[name];
        if(col_object.status==0 || my_object.fixed==1)
            continue;
        float coef1; //'2*m1/(m1+m2)'
        float coef2; //'2*m2/(m1+m2)'
        float coef3; //'(m1-m2)/(m1+m2)'
        if(my_object.weight+col_object.weight==0){
            coef1=0;
            coef2=0;
            coef3=0;
        }
        else{
            coef1=2*my_object.weight/(my_object.weight+col_object.weight);
            coef2=2*col_object.weight/(my_object.weight+col_object.weight);
            coef3=(my_object.weight-col_object.weight)/(my_object.weight+col_object.weight);
        }
        if(colliding!=name && col_object.height!=-1){ //Check collision only with circles and rectangles
            if((dx>0 && checkCollisionRight(col_object,my_object)) || (dx<0 && checkCollisionLeft(col_object,my_object)) || (dy>0 && checkCollisionTop(col_object,my_object)) || (dy<=0 && checkCollisionBottom(col_object,my_object))){
                collide=1;
                /*float angle_from_x = atan2((col_object.y-my_object.y),(col_object.x-my_object.x));
                  float my_speed_x = cos(angle_from_x)*my_object.x_speed+sin(angle_from_x)*my_object.y_speed;
                  float my_speed_y = sin(angle_from_x)*my_object.x_speed+cos(angle_from_x)*my_object.y_speed;
                  float col_speed_x = cos(angle_from_x)*col_object.x_speed+sin(angle_from_x)*col_object.y_speed;
                  float col_speed_y = sin(angle_from_x)*col_object.x_speed+cos(angle_from_x)*col_object.y_speed;
                  cout << angle_from_x*180/M_PI << endl;*/
                if(col_object.fixed==0){
                    col_object.x_speed=(coef1*my_object.x_speed-coef3*col_object.x_speed);
                    col_object.y_speed=(coef1*my_object.y_speed-coef3*col_object.y_speed);
                    //float col_x_speed_new=(coef1*my_speed_x-coef3*col_speed_x);
                    //float col_y_speed_new=(coef1*my_speed_y-coef3*col_speed_y);
                    //col_object.x_speed=cos(angle_from_x)*col_x_speed_new+sin(angle_from_x)*col_y_speed_new;
                    //col_object.y_speed=cos(angle_from_x)*col_y_speed_new+sin(angle_from_x)*col_x_speed_new;
                    col_object.inAir=1;
                    if(col_object.isRotating==0 && name=="cannonball" && (abs(my_object.x_speed)>=15 || abs(my_object.y_speed)>=15)){
                        if(my_object.x_speed>0 || my_object.y_speed>0){
                            col_object.isRotating=1;
                            col_object.direction=0;
                            col_object.remAngle=90;
                        }
                        else{
                            col_object.isRotating=1;
                            col_object.direction=1;
                            col_object.remAngle=90;
                        }
                    }
                }
                if(col_object.fixed==1){
                    if((dx>0 && checkCollisionRight(col_object,my_object)) || (dx<0 && checkCollisionLeft(col_object,my_object))){
                        my_object.x_speed*=-1/1.2;
                    }
                    if((dy>0 && checkCollisionTop(col_object,my_object)) || (dy<0 && checkCollisionBottom(col_object,my_object))){
                        my_object.y_speed*=-1/1.2;
                    }
                }
                else{
                    if(name!="cannonball"){
                        my_object.x_speed=(coef3*my_object.x_speed+coef2*col_object.x_speed); //Use elastic collision
                        my_object.y_speed=(coef3*my_object.y_speed+coef2*col_object.y_speed); //Use elastic collision
                    }
                    //float my_x_speed_new=(coef3*my_speed_x+coef2*col_speed_x);
                    //float my_y_speed_new=(coef3*my_speed_y+coef2*col_speed_y);
                    //my_object.x_speed=cos(angle_from_x)*my_x_speed_new+sin(angle_from_x)*my_y_speed_new;
                    //my_object.y_speed=cos(angle_from_x)*my_y_speed_new+sin(angle_from_x)*my_x_speed_new;
                }
                if(dx>0 && checkCollisionRight(col_object,my_object)){
                    my_object.x=col_object.x-col_object.width/2-my_object.width/2;
                }
                else if(dx<0 && checkCollisionLeft(col_object,my_object)){
                    my_object.x=col_object.x+col_object.width/2+my_object.width/2;
                }
                if(dy>0 && checkCollisionTop(col_object,my_object)){
                    my_object.y=col_object.y-col_object.height/2-my_object.height/2;
                }
                else if(dy<=0 && checkCollisionBottom(col_object,my_object)){
                    my_object.y=col_object.y+col_object.height/2+my_object.height/2;
                }
                if(dy!=0){
                    if(abs(objects[name].y_speed)<=7.5 && abs(objects[name].x_speed)<=7.5){
                        my_object.y_speed=0;
                        my_object.x_speed=0;
                        my_object.inAir=0;
                        if(name=="cannonball" && player_reset_timer==0 && player_status==1){
                            player_reset_timer=30;
                        }
                    }
                }
                my_object.x_speed/=(1+col_object.friction);
                my_object.y_speed/=(1+col_object.friction);
                col_object.x_speed/=(1+my_object.friction);
                col_object.y_speed/=(1+my_object.friction);
                collide=1;
                if(abs(my_object.x_speed)<=2)
                    my_object.x_speed=0;
                if(abs(my_object.y_speed)<=2)
                    my_object.y_speed=0;
                if(abs(col_object.x_speed)<=2)
                    col_object.x_speed=0;
                if(abs(col_object.y_speed)<=2)
                    col_object.y_speed=0;
            }
        }
        if(collide==1 && name=="cannonball" && col_object.fixed==0 && (abs(my_object.x_speed)>=5 || abs(my_object.y_speed)>=5)){
            any_collide=1;
            col_object.health-=min(max(5.0,max(abs(my_object.x_speed),abs(my_object.y_speed))*2.5),10.0);
            if(col_object.health<60){
                if(colliding=="pig1")
                    pig1Objects["pig1eye1hurt"].status=1;
                else if(colliding=="pig2")
                    pig2Objects["pig2eye2hurt"].status=1;
                else if(colliding=="pig3")
                    pig3Objects["pig3eye1hurt"].status=1;
                else if(colliding=="pig4")
                    pig4Objects["pig4eye1hurt"].status=1;
            }
            if(col_object.health<=0){
                col_object.health=0;
                player_score+=50;
                characterValues[4]='.';
                characterValues[5]='5';
                characterValues[6]='0';
                backgroundObjects["scorebackground"].status=1;
                backgroundObjects["scorebackground"].x=col_object.x+10;
                backgroundObjects["scorebackground"].y=col_object.y+col_object.height/2+15;
                if(colliding=="pig1" || colliding=="pig2" || colliding=="pig3" || colliding=="pig4"){
                    player_score+=50;
                    backgroundObjects["scorebackground"].x=col_object.x+5;
                    characterValues[4]='1';
                    characterValues[5]='0';
                    characterValues[6]='0';
                }
                scoreDrawTimer=50;
                characterPosX[4]=col_object.x-20;
                characterPosY[4]=col_object.y+col_object.height/2+15;
                characterPosX[5]=col_object.x;
                characterPosY[5]=col_object.y+col_object.height/2+15;
                characterPosX[6]=col_object.x+20;
                characterPosY[6]=col_object.y+col_object.height/2+15;
                col_object.status=0;
            }
        }
        objects[name]=my_object;
        objects[colliding]=col_object;
    }
    return any_collide;
}

void setStrokes(char val, int charNo, map<string,Sprite> curChar){
    curChar["top"].status=0;
    curChar["bottom"].status=0;
    curChar["middle"].status=0;
    curChar["left1"].status=0;
    curChar["left2"].status=0;
    curChar["right1"].status=0;
    curChar["right2"].status=0;
    curChar["middle1"].status=0;
    curChar["middle2"].status=0;
    if(val=='0' || val=='2' || val=='3' || val=='5' || val=='6'|| val=='7' || val=='8' || val=='9' || val=='P' || val=='I' || val=='O' || val=='N' || val=='T' || val=='S' || val=='E'){
        curChar["top"].status=1;
    }
    if(val=='2' || val=='3' || val=='4' || val=='5' || val=='6' || val=='8' || val=='9' || val=='P' || val=='S' || val=='Y' || val=='E'){
        curChar["middle"].status=1;
    }
    if(val=='0' || val=='2' || val=='3' || val=='5' || val=='6' || val=='8' || val=='9' || val=='O' || val=='S' || val=='I' || val=='Y' || val=='U' || val=='L' || val=='E' || val=='W'){
        curChar["bottom"].status=1;
    }
    if(val=='0' || val=='4' || val=='5' || val=='6' || val=='8' || val=='9' || val=='P' || val=='O' || val=='N' || val=='S' || val=='Y' || val=='U' || val=='L' || val=='E' || val=='W'){
        curChar["left1"].status=1;
    }
    if(val=='0' || val=='2' || val=='6' || val=='8' || val=='P' || val=='O' || val=='N' || val=='U' || val=='L' || val=='E' || val=='W'){
        curChar["left2"].status=1;
    }
    if(val=='0' || val=='1' || val=='2' || val=='3' || val=='4' || val=='7' || val=='8' || val=='9' || val=='P' || val=='O' || val=='N' || val=='Y' || val=='U' || val=='W'){
        curChar["right1"].status=1;
    }
    if(val=='0' || val=='1' || val=='3' || val=='4' || val=='5' || val=='6' || val=='7' || val=='8' || val=='9' || val=='O' || val=='N' || val=='S' || val=='Y' || val=='U' || val=='W'){
        curChar["right2"].status=1;
    }
    if(val=='I' || val=='T'){
        curChar["middle1"].status=1;
    }
    if(val=='I' || val=='T' || val=='W'){
        curChar["middle2"].status=1;
    }
    *characters[charNo]=curChar;
}


float old_time; // Time in seconds
float cur_time; // Time in seconds
double mouse_pos_x, mouse_pos_y;
double new_mouse_pos_x, new_mouse_pos_y;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
    COLOR winbackground = {212/255.0,175/255.0,55/255.0};
    COLOR losebackground = {255/255.0,77/255.0,77/255.0};
    if(game_over==1){
        return;
    }

    game_timer=(int)(90-(glfwGetTime()-game_start_timer));

    if(player_score>=1450){
        game_over=1;
        endLabel_x=-150;
        createRectangle("endgame",10000,winbackground,winbackground,winbackground,winbackground,0,0,200,600,"background");
        endLabel="YOU WIN";
    }

    if(glfwGetTime()-game_start_timer>=90){
        game_over=1;
        createRectangle("endgame",10000,losebackground,losebackground,losebackground,losebackground,0,0,200,600,"background");
        endLabel="YOU LOSE";
    }

    if(scoreDrawTimer>0){
        scoreDrawTimer--;
        if(scoreDrawTimer<=0){
            scoreDrawTimer=-1;
            backgroundObjects["scorebackground"].status=0;
            characterValues[4]='.'; // '.' represents and empty character (It won't be drawn)
            characterValues[5]='.'; // '.' represents and empty character (It won't be drawn)
            characterValues[6]='.'; // '.' represents and empty character (It won't be drawn)
        }
    }

    characterValues[0]='.';
    characterValues[1]='.';
    characterValues[2]='.';
    characterValues[3]='.';
    int cur_score = player_score;
    int start=0;
    if(cur_score==0)
        characterValues[3]='0';
    while(cur_score){
        characterValues[3-start]=(cur_score)%10+'0';
        cur_score/=10;
        start++;
    }
    glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);
    if(right_mouse_clicked==1){
        x_change+=new_mouse_pos_x-mouse_pos_x;
        y_change-=new_mouse_pos_y-mouse_pos_y;
        check_pan();
    }
    Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(-300.0f/zoom_camera+y_change), (float)(300.0f/zoom_camera+y_change), 0.1f, 500.0f);
    glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
    if(glfwGetTime()-click_time>=2){
        objects["cannonball"].y=-240;
        objects["cannonball"].x=-315;
        objects["cannonball"].inAir=0;
        for(map<string,Sprite>::iterator it=cannonObjects.begin();it!=cannonObjects.end();it++){
            string current = it->first; //The name of the current object
            if(cannonObjects[current].isMovingAnim==1){
                cannonObjects[current].x+=16-cannonObjects[current].dx;
                cannonObjects[current].isMovingAnim=0;
                cannonObjects[current].dx=0;
            }
            if(cannonObjects[current].isMovingAnim==2){
                cannonObjects[current].x+=cannonObjects[current].dx;
                cannonObjects[current].isMovingAnim=0;
                cannonObjects[current].dx=0;
            }
        }
        player_status=0;
    }

    float time_delta = (cur_time-old_time)*60;

    if(keyboard_pressed==1){
        cannonObjects["cannonrectangle"].angle=launch_angle;
        double power = launch_power;
        double max_power=760*760+560*560;
        double width=min((power/max_power)*160,160.0);
        backgroundObjects["cannonpowerdisplay"].x=-350+width/2;
        backgroundObjects["cannonpowerdisplay"].width=width;
        COLOR my_color = backgroundObjects["cannonpowerdisplay"].color;
        createRectangle("cannonpowerdisplay",10000,my_color,my_color,my_color,my_color,backgroundObjects["cannonpowerdisplay"].x,backgroundObjects["cannonpowerdisplay"].y,25,backgroundObjects["cannonpowerdisplay"].width,"background");
        if(player_reset_timer>0){
            player_reset_timer-=1;
            if(player_reset_timer==0 && objects["cannonball"].inAir==0 && player_status==1){
                player_status=0;
                objects["cannonball"].y=-240;
                objects["cannonball"].x=-315;
            }
        }
    }
    if(mouse_clicked==1) {
        float angle=0;
        double mouse_x_cur;
        double mouse_y_cur;
        glfwGetCursorPos(window,&mouse_x_cur,&mouse_y_cur);
        if(mouse_x_cur==800)
            angle=90;
        else{
            angle=atan(abs(mouse_y_cur-600)/abs(mouse_x_cur));
            angle*=180/M_PI;
        }
        cannonObjects["cannonrectangle"].angle=angle;
        double power = mouse_x_cur*mouse_x_cur+(mouse_y_cur-600)*(mouse_y_cur-600);
        double max_power=760*760+560*560;
        double width=min((power/max_power)*160,160.0);
        backgroundObjects["cannonpowerdisplay"].x=-350+width/2;
        backgroundObjects["cannonpowerdisplay"].width=width;
        COLOR my_color = backgroundObjects["cannonpowerdisplay"].color;
        createRectangle("cannonpowerdisplay",10000,my_color,my_color,my_color,my_color,backgroundObjects["cannonpowerdisplay"].x,backgroundObjects["cannonpowerdisplay"].y,25,backgroundObjects["cannonpowerdisplay"].width,"background");
    }
    if(player_reset_timer>0){
        player_reset_timer-=1;
        if(player_reset_timer==0 && objects["cannonball"].inAir==0 && player_status==1){
            player_status=0;
            objects["cannonball"].y=-240;
            objects["cannonball"].x=-315;
        }
    }
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);

    // Compute Camera matrix (view)
    //Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(glm::vec3(0,0,1), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    //Draw the background
    for(map<string,Sprite>::iterator it=backgroundObjects.begin();it!=backgroundObjects.end();it++){
        string current = it->first; //The name of the current object
        if(backgroundObjects[current].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(backgroundObjects[current].x, backgroundObjects[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(backgroundObjects[current].object);
        //glPopMatrix ();
    }

    //Draw the coins
    for(map<string,Sprite>::iterator it=coins.begin();it!=coins.end();it++){
        string current = it->first; //The name of the current object
        if(coins[current].status==0)
            continue;
        glm::mat4 MVP;	// MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(coins[current].x, coins[current].y, 0.0f)); // glTranslatef
        glm::mat4 rotateTriangle = glm::rotate((float)((0)*M_PI/180.0f), glm::vec3(0,1,0));  // rotate about vector (1,0,0)
        coins[current].angle=(coins[current].angle+1.0*time_delta);
        if(coins[current].angle>=360.0)
            coins[current].angle=0.0;
        ObjectTransform=translateObject*rotateTriangle;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(coins[current].object);
        //glPopMatrix ();
    }

    //Draw the goals
    for(map<string,Sprite>::iterator it=goalObjects.begin();it!=goalObjects.end();it++){
        string current = it->first; //The name of the current object
        if(goalObjects[current].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(goalObjects[current].x, goalObjects[current].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(goalObjects[current].object);
        //glPopMatrix ();
    }

    for(map<string,Sprite>::iterator it=objects.begin();it!=objects.end();it++){
        string current = it->first; //The name of the current object
        if(current!="floor" && current!="floor2" && current!="roof" && current!="wall1" && current!="wall2"){
            if(objects[current].y>250){
                objects[current].y=250;
                objects[current].y_speed*=-1/2;
            }
            if(objects[current].y<-265){
                objects[current].y=-265;
                objects[current].y_speed*=-1;
            }
        }
        if(objects[current].status==0)
            continue;
        if(objects[current].fixed==0 && objects[current].y_speed==0){
            moveObject(current,0,-2);
            int col_state=0;
            for(map<string,Sprite>::iterator it3=objects.begin();it3!=objects.end();it3++){
                if(it3->second.status==0)
                    continue;
                if(checkCollisionBottom(it3->second,objects[current]))
                    col_state=1;
            }
            moveObject(current,0,2);
            if(col_state==0){
                objects[current].inAir=1;
            }
        }
        if(objects[current].inAir && objects[current].fixed==0){
            if(objects[current].y_speed>=-30)
                objects[current].y_speed-=gravity*time_delta;
            objects[current].x_speed-=airResistance*time_delta*objects[current].x_speed;
            pair<float,float> position = moveObject(current,objects[current].x_speed*time_delta,0);
            //We can also use the checkCollisionSphere here instead but since we don't have any rotated blocks currently we will stick with this
            checkCollision(current,objects[current].x_speed*time_delta,0); //Always call the checkCollision function with only 1 position change at a time!
            position = moveObject(current,0,objects[current].y_speed*time_delta);
            //We can also use the checkCollisionSphere here instead but since we don't have any rotated blocks currently we will stick with this
            checkCollision(current,0,objects[current].y_speed*time_delta);
        }
        glm::mat4 MVP;	// MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */

        glm::mat4 ObjectTransform;

        if(objects[current].isMovingAnim==1 && (current=="springbase2" || current=="springbase3")){
            if(objects[current].dy>0){
                float dy=objects[current].dy;
                float y=objects[current].y;
                if(current=="springbase3"){
                    COLOR my_color = objects["springbase3"].color;
                    createRectangle("springbase3",10000,my_color,my_color,my_color,my_color,190,objects["springbase3"].y,objects["springbase3"].height-1,20,"");
                    objects["springbase3"].fixed=1;
                    y+=1/2.0;
                    objects["springbase3"].isMovingAnim=1;
                }
                objects[current].dy=dy-1;
                objects[current].y=y-1;
                if(objects[current].dy==0){
                    goalObjects["goal1"].status=1;
                    goalObjects["goal2"].status=1;
                    goalObjects["goal3"].status=1;
                    objects[current].isMovingAnim=2;
                    objects["springbase1"].isMovingAnim=2; //To activate the goal, check for the status of the springbase1,2 or 3 if its equal to 2
                }
            }
        }

        if (objects[current].isRotating==1 && current!="cannonball"){
            objects[current].remAngle-=9;
            float xShift = -0.5;
            if(objects[current].direction==0){
                xShift*=-1;
                objects[current].angle-=9;
            }
            else
                objects[current].angle+=9;
            moveObject(current,xShift,0);
            if(checkCollision(current,xShift,0)){
                moveObject(current,-xShift,0);
            }
            if(objects[current].remAngle<=0){
                objects[current].isRotating=0;
            }
        }
        glm::mat4 translateObject = glm::translate (glm::vec3(objects[current].x, objects[current].y, 0.0f)); // glTranslatef
        glm::mat4 rotateObjectAct = glm::rotate((float)(objects[current].angle*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        ObjectTransform=translateObject*rotateObjectAct;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(objects[current].object);
        //glPopMatrix ();
    }

    //Draw the first pig (pig1)
    for(map<string,Sprite>::iterator it=pig1Objects.begin();it!=pig1Objects.end();it++){
        string current = it->first; //The name of the current object
        if(objects["pig1"].status==0 || pig1Objects[it->first].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        glm::mat4 ObjectTransform;
        float x_diff,y_diff;
        x_diff=pig1Objects[current].x;
        y_diff=pig1Objects[current].y;
        glm::mat4 translateObject = glm::translate (glm::vec3(objects["pig1"].x+pig1Objects[current].x, objects["pig1"].y+pig1Objects[current].y, 0.0f)); // glTranslatef
        glm::mat4 translateObject1 = glm::translate (glm::vec3(-x_diff, -y_diff, 0.0f));
        glm::mat4 rotateTriangle = glm::rotate((float)((objects["pig1"].angle)*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 translateObject2 = glm::translate (glm::vec3(x_diff, y_diff, 0.0f));
        ObjectTransform=translateObject*translateObject1*rotateTriangle*translateObject2;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(pig1Objects[current].object);
        //glPopMatrix ();
    }

    //Draw the second pig (pig2)
    for(map<string,Sprite>::iterator it=pig2Objects.begin();it!=pig2Objects.end();it++){
        string current = it->first; //The name of the current object
        if(objects["pig2"].status==0 || pig2Objects[it->first].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        glm::mat4 ObjectTransform;
        float x_diff,y_diff;
        x_diff=pig2Objects[current].x;
        y_diff=pig2Objects[current].y;
        glm::mat4 translateObject = glm::translate (glm::vec3(objects["pig2"].x+pig2Objects[current].x, objects["pig2"].y+pig2Objects[current].y, 0.0f)); // glTranslatef
        glm::mat4 translateObject1 = glm::translate (glm::vec3(-x_diff, -y_diff, 0.0f));
        glm::mat4 rotateTriangle = glm::rotate((float)((objects["pig2"].angle)*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 translateObject2 = glm::translate (glm::vec3(x_diff, y_diff, 0.0f));
        ObjectTransform=translateObject*translateObject1*rotateTriangle*translateObject2;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(pig2Objects[current].object);
        //glPopMatrix ();
    }

    //Draw the third pig (pig3)
    for(map<string,Sprite>::iterator it=pig3Objects.begin();it!=pig3Objects.end();it++){
        string current = it->first; //The name of the current object
        if(objects["pig3"].status==0 || pig3Objects[it->first].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        glm::mat4 ObjectTransform;
        float x_diff,y_diff;
        x_diff=pig3Objects[current].x;
        y_diff=pig3Objects[current].y;
        glm::mat4 translateObject = glm::translate (glm::vec3(objects["pig3"].x+pig3Objects[current].x, objects["pig3"].y+pig3Objects[current].y, 0.0f)); // glTranslatef
        glm::mat4 translateObject1 = glm::translate (glm::vec3(-x_diff, -y_diff, 0.0f));
        glm::mat4 rotateTriangle = glm::rotate((float)((objects["pig3"].angle)*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 translateObject2 = glm::translate (glm::vec3(x_diff, y_diff, 0.0f));
        ObjectTransform=translateObject*translateObject1*rotateTriangle*translateObject2;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(pig3Objects[current].object);
        //glPopMatrix ();
    }

    //Draw the fourth pig (pig4)
    for(map<string,Sprite>::iterator it=pig4Objects.begin();it!=pig4Objects.end();it++){
        string current = it->first; //The name of the current object
        if(objects["pig4"].status==0 || pig4Objects[it->first].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 ObjectTransform;
        float x_diff,y_diff;
        x_diff=pig4Objects[current].x;
        y_diff=pig4Objects[current].y;
        glm::mat4 translateObject = glm::translate (glm::vec3(objects["pig4"].x+pig4Objects[current].x, objects["pig4"].y+pig4Objects[current].y, 0.0f)); // glTranslatef
        glm::mat4 translateObject1 = glm::translate (glm::vec3(-x_diff, -y_diff, 0.0f));
        glm::mat4 rotateTriangle = glm::rotate((float)((objects["pig4"].angle)*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 translateObject2 = glm::translate (glm::vec3(x_diff, y_diff, 0.0f));
        ObjectTransform=translateObject*translateObject1*rotateTriangle*translateObject2;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(pig4Objects[current].object);
        //glPopMatrix ();
    }

    //Draw the cannon
    for(map<string,Sprite>::iterator it=cannonObjects.begin();it!=cannonObjects.end();it++){
        string current = it->first; //The name of the current object
        if(cannonObjects[current].isMovingAnim==1){
            cannonObjects[current].x-=4;
            cannonObjects[current].dx-=4;
            if(cannonObjects[current].dx==0){
                cannonObjects[current].isMovingAnim=2;
                cannonObjects[current].dx=16;
            }
        }
        if(cannonObjects[current].isMovingAnim==2){
            cannonObjects[current].x+=1;
            cannonObjects[current].dx-=1;
            if(cannonObjects[current].dx==0){
                cannonObjects[current].isMovingAnim=0;
            }
        }
        if(cannonObjects[current].status==0)
            continue;
        glm::mat4 MVP;  // MVP = Projection * View * Model

        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(cannonObjects[current].x, cannonObjects[current].y, 0.0f)); // glTranslatef
        float x_diff,y_diff;
        x_diff=abs(cannonObjects["cannoncircle"].x-cannonObjects[current].x);
        y_diff=abs(cannonObjects["cannoncircle"].y-cannonObjects[current].y);
        glm::mat4 translateObject1 = glm::translate (glm::vec3(-x_diff, -y_diff, 0.0f)); // glTranslatef
        glm::mat4 rotateTriangle = glm::rotate((float)((cannonObjects[current].angle)*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 translateObject2 = glm::translate (glm::vec3(x_diff, y_diff, 0.0f)); // glTranslatef
        ObjectTransform=translateObject*translateObject1*rotateTriangle*translateObject2;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        draw3DObject(cannonObjects[current].object);
        //glPopMatrix ();
    }


    if(backgroundObjects["scorebackground"].status==1){
        //Draw the scorebox background
        glm::mat4 MVP;
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(backgroundObjects["scorebackground"].x, backgroundObjects["scorebackground"].y, 0.0f));
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(backgroundObjects["scorebackground"].object);
    }

    if(game_over==1){
        //Draw the scorebox background
        glm::mat4 MVP;
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(backgroundObjects["endgame"].x, backgroundObjects["endgame"].y, 0.0f));
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(backgroundObjects["endgame"].object);
    }

    //Draw the characters
    int t;
    for(t=0;t<7;t++){
        if(game_over==1 && t>3)
            continue;
        map <string, Sprite> charCurrent = *characters[t];
        float base_x = characterPosX[t];
        float base_y = characterPosY[t];
        char charValue = characterValues[t];
        setStrokes(charValue, t, charCurrent);
        for(map<string,Sprite>::iterator it2=charCurrent.begin();it2!=charCurrent.end();it2++){
            if(it2->second.status==0)
                continue;

            string current = it2->first;

            glm::mat4 MVP;  // MVP = Projection * View * Model
            Matrices.model = glm::mat4(1.0f);

            /* Render your scene */
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(base_x+charCurrent[current].x, base_y+charCurrent[current].y, 0.0f)); // glTranslatef
            ObjectTransform=translateObject;
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M

            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

            draw3DObject(charCurrent[current].object);
            //glPopMatrix ();
        }
    }

    //Draw the "SCORE" label
    float base_x=scoreLabel_x;
    float base_y=scoreLabel_y;
    for(t=0;t<scoreLabel.length();t++){
        setStrokes(scoreLabel[t],7,scoreLabelObjects);
        for(map<string,Sprite>::iterator it2=scoreLabelObjects.begin();it2!=scoreLabelObjects.end();it2++){
            if(it2->second.status==0)
                continue;

            string current = it2->first;

            glm::mat4 MVP;  // MVP = Projection * View * Model
            Matrices.model = glm::mat4(1.0f);

            /* Render your scene */
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(base_x+it2->second.x, base_y+it2->second.y, 0.0f)); // glTranslatef
            ObjectTransform=translateObject;
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M

            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

            draw3DObject(it2->second.object);
            //glPopMatrix ();
        }
        base_x+=17; //Next character
    }

    //Draw the "THE_END" label
    base_x=endLabel_x;
    base_y=endLabel_y;
    for(t=0;t<endLabel.length();t++){
        setStrokes(endLabel[t],8,endLabelObjects);
        for(map<string,Sprite>::iterator it2=endLabelObjects.begin();it2!=endLabelObjects.end();it2++){
            if(it2->second.status==0)
                continue;

            string current = it2->first;

            glm::mat4 MVP;  // MVP = Projection * View * Model
            Matrices.model = glm::mat4(1.0f);

            /* Render your scene */
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(base_x+it2->second.x, base_y+it2->second.y, 0.0f)); // glTranslatef
            ObjectTransform=translateObject;
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M

            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

            draw3DObject(it2->second.object);
            //glPopMatrix ();
        }
        base_x+=48; //Next character
    }

    //Draw the timer
    if(game_over!=1){
    base_x=timer_x;
    base_y=timer_y;
    int cur_game_timer = game_timer;
    while(cur_game_timer){
        setStrokes('0'+(cur_game_timer)%10,9,timerObjects);
        cur_game_timer/=10;
        for(map<string,Sprite>::iterator it2=timerObjects.begin();it2!=timerObjects.end();it2++){
            if(it2->second.status==0)
                continue;

            string current = it2->first;

            glm::mat4 MVP;  // MVP = Projection * View * Model
            Matrices.model = glm::mat4(1.0f);

            /* Render your scene */
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(base_x+it2->second.x, base_y+it2->second.y, 0.0f)); // glTranslatef
            ObjectTransform=translateObject;
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M

            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

            draw3DObject(it2->second.object);
            //glPopMatrix ();
        }
        base_x-=15; //Next character
    }
    }
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, mousescroll); // mouse scroll

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models

    characters[0]=&char1Objects;
    characters[1]=&char2Objects;
    characters[2]=&char3Objects;
    characters[3]=&char4Objects;
    characters[4]=&charscoreObjects1;
    characters[5]=&charscoreObjects2;
    characters[6]=&charscoreObjects3;
    characters[7]=&scoreLabelObjects;
    characters[8]=&endLabelObjects;
    characters[9]=&timerObjects;

    characterPosX[0]=280;
    characterPosX[1]=300;
    characterPosX[2]=320;
    characterPosX[3]=340;

    characterPosY[0]=250;
    characterPosY[1]=250;
    characterPosY[2]=250;
    characterPosY[3]=250;

    COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
    COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
    COLOR coingold = {255.0/255.0,223.0/255.0,0.0/255.0};
    COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
    COLOR lightgreen = {57/255.0,230/255.0,0/255.0};
    COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
    COLOR black = {30/255.0,30/255.0,21/255.0};
    COLOR blue = {0,0,1};
    COLOR darkbrown = {46/255.0,46/255.0,31/255.0};
    COLOR lightbrown = {95/255.0,63/255.0,32/255.0};
    COLOR brown1 = {117/255.0,78/255.0,40/255.0};
    COLOR brown2 = {134/255.0,89/255.0,40/255.0};
    COLOR brown3 = {46/255.0,46/255.0,31/255.0};
    COLOR cratebrown = {153/255.0,102/255.0,0/255.0};
    COLOR cratebrown1 = {121/255.0,85/255.0,0/255.0};
    COLOR cratebrown2 = {102/255.0,68/255.0,0/255.0};
    COLOR skyblue2 = {113/255.0,185/255.0,209/255.0};
    COLOR skyblue1 = {123/255.0,201/255.0,227/255.0};
    COLOR skyblue = {132/255.0,217/255.0,245/255.0};
    COLOR cloudwhite = {229/255.0,255/255.0,255/255.0};
    COLOR cloudwhite1 = {204/255.0,255/255.0,255/255.0};
    COLOR lightpink = {255/255.0,122/255.0,173/255.0};
    COLOR darkpink = {255/255.0,51/255.0,119/255.0};
    COLOR white = {255/255.0,255/255.0,255/255.0};
    COLOR score = {117/255.0,78/255.0,40/255.0};

    createRectangle("asky1",10000,skyblue,skyblue,skyblue,skyblue,0,0,600,800,"background");
    createRectangle("asky2",10000,skyblue1,skyblue1,skyblue1,skyblue1,0,-200,600,800,"background");
    createRectangle("asky3",10000,skyblue2,skyblue2,skyblue2,skyblue2,0,-400,600,800,"background");

    createRectangle("cloud1a",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,-170,110,100,160,"background");
    createRectangle("cloud1b",10000,cloudwhite1,cloudwhite1,cloudwhite1,cloudwhite1,-180,110,40,260,"background");
    createRectangle("cloud2a",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,190,160,100,160,"background");
    createRectangle("cloud2b",10000,cloudwhite1,cloudwhite1,cloudwhite1,cloudwhite1,190,155,40,270,"background");
    createCircle("cloud1ac1",10000,cloudwhite,-250,110,50,15,"background",1);
    createCircle("cloud1ac2",10000,cloudwhite,-90,110,50,15,"background",1); //Last param is fill
    createCircle("cloud1bc1",10000,cloudwhite1,-310,110,20,15,"background",1);
    createCircle("cloud1bc2",10000,cloudwhite1,-40,110,20,15,"background",1); //Last param is fill
    createCircle("cloud2ac1",10000,cloudwhite,110,160,50,15,"background",1);
    createCircle("cloud2ac2",10000,cloudwhite,270,160,50,15,"background",1); //Last param is fill
    createCircle("cloud2bc1",10000,cloudwhite1,60,155,20,15,"background",1);
    createCircle("cloud2bc2",10000,cloudwhite1,320,155,20,15,"background",1); //Last param is fill

    createRectangle("cannonpower1",10000,cratebrown2,cratebrown2,cratebrown2,cratebrown2,-270,250,40,200,"background");
    createRectangle("cannonpower2",10000,cratebrown1,cratebrown1,cratebrown1,cratebrown1,-270,250,25,160,"background");
    createRectangle("cannonpowerdisplay",10000,red,red,red,red,-270,250,25,0,"background");

    createRectangle("skyfloor1",10000,cratebrown1,cratebrown1,cratebrown1,cratebrown1,190,30,20,100,"");
    objects["skyfloor1"].fixed=1;
    createRectangle("skyfloor2",10000,cratebrown1,cratebrown1,cratebrown1,cratebrown1,230,60,60,20,"");
    objects["skyfloor2"].fixed=1;
    createRectangle("skyfloor3",10000,cratebrown1,cratebrown1,cratebrown1,cratebrown1,270,90,20,100,"");
    objects["skyfloor3"].fixed=1;
    createRectangle("springbase1",10000,cratebrown2,cratebrown2,cratebrown2,cratebrown2,190,50,20,40,"");
    objects["springbase1"].fixed=1;
    createRectangle("springbase2",10000,cratebrown2,cratebrown2,cratebrown2,cratebrown2,190,90,20,40,"");
    objects["springbase2"].fixed=1;
    createRectangle("springbase3",10000,cratebrown,cratebrown,cratebrown,cratebrown,190,70,40,20,"");
    objects["springbase3"].fixed=1;

    createRectangle("groundfloor1",10000,cratebrown1,cratebrown1,cratebrown1,cratebrown1,-50,-260,20,20,"");
    objects["groundfloor1"].fixed=1;
    createRectangle("groundfloor2",10000,cratebrown1,cratebrown1,cratebrown1,cratebrown1,-10,-240,20,100,"");
    objects["groundfloor2"].fixed=1;
    createRectangle("groundfloor3",10000,cratebrown1,cratebrown1,cratebrown1,cratebrown1,30,-260,20,20,"");
    objects["groundfloor3"].fixed=1;

    createCircle("cannonball",2,black,-315,-270,15,10,"",1); //Generate sprites
    objects["cannonball"].friction=0.3;
    createRectangle("crate1",1,cratebrown,cratebrown2,cratebrown2,cratebrown,160,-100,60,60,"");
    createRectangle("crate2",1,cratebrown,cratebrown2,cratebrown2,cratebrown,160,-160,30,30,"");
    createRectangle("crate3",1,cratebrown,cratebrown2,cratebrown2,cratebrown,160,-190,30,30,"");
    createRectangle("crate4",1,cratebrown,cratebrown2,cratebrown2,cratebrown,160,-220,30,30,"");

    //On the skyfloor
    createRectangle("crate5",1,cratebrown,cratebrown2,cratebrown2,cratebrown,270,140,40,40,"");

    createCircle("pig1",1,lightpink,320,-155,20,15,"",1);
    createCircle("pig1ear1",1,lightpink,-17,13,7,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    createCircle("pig1ear2",1,lightpink,17,13,7,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    //createCircle("pig1ear1in",1,darkpink,-17,14,4,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    //createCircle("pig1ear2in",1,darkpink,17,14,4,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    createCircle("pig1eye1main",1,white,-15,0,5,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    createCircle("pig1eye1hurt",1,darkbrown,-14,0,8,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    createCircle("pig1eye2main",1,white,15,0,5,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    pig1Objects["pig1eye1hurt"].status=0;
    createCircle("pig1eyeball1",1,black,-13,0,2,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    createCircle("pig1eyeball2",1,black,13,0,2,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    createCircle("pig1nose",1,darkpink,0,-5,10,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    createCircle("pig1nose1",1,darkbrown,2.4,-5,2.4,15,"pig1",1); //Store x and y offsets from pig1 as x,y here
    createCircle("pig1nose2",1,darkbrown,-2.4,-5,2.4,15,"pig1",1); //Store x and y offsets from pig1 as x,y here

    createCircle("pig2",1,lightpink,0,-150,20,15,"",1);
    createCircle("pig2ear1",1,lightpink,-17,13,7,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    createCircle("pig2ear2",1,lightpink,17,13,7,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    //createCircle("pig2ear1in",1,darkpink,-17,14,4,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    //createCircle("pig2ear2in",1,darkpink,17,14,4,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    createCircle("pig2eye1main",1,white,-15,0,5,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    createCircle("pig2eye2main",1,white,15,0,5,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    createCircle("pig2eye2hurt",1,darkbrown,14,0,8,15,"pig2",1); //Store x and y offsets from pig1 as x,y here
    pig2Objects["pig2eye2hurt"].status=0;
    createCircle("pig2eyeball1",1,black,-13,0,2,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    createCircle("pig2eyeball2",1,black,13,0,2,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    createCircle("pig2nose",1,darkpink,0,-5,10,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    createCircle("pig2nose1",1,darkbrown,2.4,-5,2.4,15,"pig2",1); //Store x and y offsets from pig2 as x,y here
    createCircle("pig2nose2",1,darkbrown,-2.4,-5,2.4,15,"pig2",1); //Store x and y offsets from pig2 as x,y here

    createCircle("pig3",1,lightpink,270,255,20,15,"",1);
    createCircle("pig3ear1",1,lightpink,-17,13,7,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    createCircle("pig3ear2",1,lightpink,17,13,7,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    //createCircle("pig1ear1in",1,darkpink,-17,14,4,15,"pig1",1); //Store x and y offsets from pig3 as x,y here
    //createCircle("pig1ear2in",1,darkpink,17,14,4,15,"pig1",1); //Store x and y offsets from pig3 as x,y here
    createCircle("pig3eye1main",1,white,-15,0,5,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    createCircle("pig3eye1hurt",1,darkbrown,-14,0,8,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    createCircle("pig3eye2main",1,white,15,0,5,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    pig3Objects["pig3eye1hurt"].status=0;
    createCircle("pig3eyeball1",1,black,-13,0,2,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    createCircle("pig3eyeball2",1,black,13,0,2,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    createCircle("pig3nose",1,darkpink,0,-5,10,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    createCircle("pig3nose1",1,darkbrown,2.4,-5,2.4,15,"pig3",1); //Store x and y offsets from pig3 as x,y here
    createCircle("pig3nose2",1,darkbrown,-2.4,-5,2.4,15,"pig3",1); //Store x and y offsets from pig3 as x,y here

    createCircle("pig4",1,lightpink,160,0,20,15,"",1);
    createCircle("pig4ear1",1,lightpink,-17,13,7,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    createCircle("pig4ear2",1,lightpink,17,13,7,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    //createCircle("pig1ear1in",1,darkpink,-17,14,4,15,"pig1",1); //Store x and y offsets from pig4 as x,y here
    //createCircle("pig1ear2in",1,darkpink,17,14,4,15,"pig1",1); //Store x and y offsets from pig4 as x,y here
    createCircle("pig4eye1main",1,white,-15,0,5,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    createCircle("pig4eye1hurt",1,darkbrown,-14,0,8,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    createCircle("pig4eye2main",1,white,15,0,5,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    pig4Objects["pig4eye1hurt"].status=0;
    createCircle("pig4eyeball1",1,black,-13,0,2,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    createCircle("pig4eyeball2",1,black,13,0,2,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    createCircle("pig4nose",1,darkpink,0,-5,10,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    createCircle("pig4nose1",1,darkbrown,2.4,-5,2.4,15,"pig4",1); //Store x and y offsets from pig4 as x,y here
    createCircle("pig4nose2",1,darkbrown,-2.4,-5,2.4,15,"pig4",1); //Store x and y offsets from pig4 as x,y here


    createRectangle("floor",10000,lightgreen,lightgreen,lightgreen,lightgreen,0,-300,60,800,"");
    objects["floor"].fixed=1;
    objects["floor"].friction=0.5;
    createRectangle("floor2",10000,darkgreen,lightgreen,lightgreen,darkgreen,0,-290,20,800,"");
    objects["floor2"].fixed=1;
    objects["floor2"].friction=0.5;
    createRectangle("roof",10000,grey,grey,grey,grey,0,300,60,800,"");
    objects["roof"].fixed=1;
    objects["roof"].friction=0.5;
    createRectangle("wall1",10000,grey,grey,grey,grey,-400,0,600,60,"");
    objects["wall1"].fixed=1;
    objects["wall1"].friction=0.5;
    createRectangle("wall2",10000,grey,grey,grey,grey,400,0,600,60,"");
    objects["wall2"].fixed=1;
    objects["wall2"].friction=0.5;

    createCircle("cannonaim",100000,darkbrown,-315,-210,150,12,"cannon",0);
    cannonObjects["cannonaim"].status=0;
    createRectangle("cannonrectangle",100000,darkbrown,darkbrown,darkbrown,darkbrown,-235,-210,50,80,"cannon");
    cannonObjects["cannonrectangle"].angle=45;

    createCircle("cannoncircle",100000,darkbrown,-315,-210,50,12,"cannon",1);
    //The objects are drawn in the lexicographic ordering of their names
    createCircle("cannoncircle2",100000,brown1,-315,-210,40,12,"cannon",1);

    createCircle("cannonawheel2",100000,darkbrown,-315,-250,30,12,"cannon",1);
    createCircle("cannonawheel22",100000,lightbrown,-315,-250,25,12,"cannon",1);
    createCircle("cannonawheel222",100000,brown2,-315,-250,20,12,"cannon",1);
    createCircle("cannonwheel1",100000,darkbrown,-295,-255,30,12,"cannon",1);
    createCircle("cannonwheel11",100000,lightbrown,-295,-255,25,12,"cannon",1);
    createCircle("cannonwheel111",100000,brown2,-295,-255,20,12,"cannon",1);

    createRectangle("cannonbase1",100000,brown3,brown3,brown3,brown3,-355,-270,20,27,"cannon");
    createRectangle("cannonbase2",100000,brown3,brown3,brown3,brown3,-355,-245,30,20,"cannon");
    cannonObjects["cannonbase2"].angle=-20;

    createCircle("coin1",100000,coingold,320,-40,15,12,"coin",1);
    createCircle("coin2",100000,coingold,20,-40,15,12,"coin",1);

    createCircle("goal1",100000,darkgreen,130,-40,15,15,"goal",1);
    goalObjects["goal1"].status=0;
    createCircle("goal2",100000,darkgreen,-120,150,15,15,"goal",1);
    goalObjects["goal2"].status=0;
    createCircle("goal3",100000,darkgreen,-320,0,15,15,"goal",1);
    goalObjects["goal3"].status=0;

    createCircle("scorebackground",100000,gold,0,0,35,8,"background",1);
    backgroundObjects["scorebackground"].status=0;
    //Render the characters for the score
    int t;
    for(t=1;t<=10;t++){
        string layer;
        if(t==1)
            layer="char1";
        if(t==2)
            layer="char2";
        if(t==3)
            layer="char3";
        if(t==4)
            layer="char4";
        if(t==5)
            layer="charscore1";
        if(t==6)
            layer="charscore2";
        if(t==7)
            layer="charscore3";
        if(t==8)
            layer="scorelabel";
        if(t==9)
            layer="endlabel";
        if(t==10)
            layer="timelabel";
        float width=12;
        float height=4;
        float offset=10;
        COLOR color = score;
        if(t==5 || t==6 || t==7){ //These are the scores that appear above objects when they are destroyed
            width=10;
            height=2;
            color = white;
        }
        if(t==8){
            width=12;
            height=4;
            color = white;
        }
        if(t==9){
            width=40;
            height=12;
            offset=30;
            color = white;
        }
        if(t==10){
            color = red;
        }
        createRectangle("top",100000,color,color,color,color,0,offset,height,width,layer);
        createRectangle("bottom",100000,color,color,color,color,0,-offset,height,width,layer);
        createRectangle("middle",100000,color,color,color,color,0,0,height,width,layer);
        createRectangle("left1",100000,color,color,color,color,-offset/2,offset/2,width,height,layer);
        createRectangle("left2",100000,color,color,color,color,-offset/2,-offset/2,width,height,layer);
        createRectangle("right1",100000,color,color,color,color,offset/2,offset/2,width,height,layer);
        createRectangle("right2",100000,color,color,color,color,offset/2,-offset/2,width,height,layer);
        createRectangle("middle1",100000,color,color,color,color,0,offset/2,width,height,layer);
        createRectangle("middle2",100000,color,color,color,color,0,-offset/2,width,height,layer);
    }

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    int width = 800;
    int height = 600;

    scoreLabel="POINTS"; //Only use capitals
    scoreLabel_x=175;
    scoreLabel_y=250;

    timer_y=250;
    timer_x=20;

    endLabel="";
    endLabel_x=-160;

    GLFWwindow* window = initGLFW(width, height);

    initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);

    game_start_timer=glfwGetTime();
    old_time = glfwGetTime();
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        cur_time = glfwGetTime(); // Time in seconds
        // OpenGL Draw commands
        draw(window);
        old_time=cur_time;

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
