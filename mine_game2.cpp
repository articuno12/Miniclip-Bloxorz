#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string.h>
#define GLM_FORCE_RADIANS
#define FN(i, n) for(int i=0;i<(int)n;++i)
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <pthread.h>
#include <ao/ao.h>
#include <fstream>

using namespace std;
#define f first
#define s second
static const int BUF_SIZE = 4096;

struct WavHeader {
    char id[4]; //should contain RIFF
    int32_t totalLength;
    char wavefmt[8];
    int32_t format; // 16 for PCM
    int16_t pcm; // 1 for PCM
    int16_t channels;
    int32_t frequency;
    int32_t bytesPerSecond;
    int16_t bytesByCapture;
    int16_t bitsPerSample;
    char data[4]; // "data"
    int32_t bytesInData;
};
struct VAO {
		GLuint VertexArrayID;
		GLuint VertexBuffer;
		GLuint ColorBuffer;

		GLenum PrimitiveMode;
		GLenum FillMode;
		int NumVertices;
};
struct COLOR {
		float r;
		float g;
		float b;
};
//defining colors
COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
COLOR coingold = {255.0/255.0,223.0/255.0,0.0/255.0};
COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
COLOR lightgreen = {57/255.0,230/255.0,0/255.0};
COLOR green = {51/255.0,102/255.0,0/255.0};
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
COLOR lightpink = {176/255.0,196/255.0,222/255.0};
COLOR darkpink = {255/255.0,51/255.0,119/255.0};
COLOR white = {255/255.0,255/255.0,255/255.0};
COLOR points = {117/255.0,78/255.0,40/255.0};
struct game_object
{
		string name;//name of object//red,black,green
		VAO* object;
		glm::vec3 center,speed,change,rotation_center,angle,gravity;
		COLOR color;
		bool is_rotate;
		float height,width,radius;
};

struct GLMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		GLuint MatrixID;
} Matrices;
//defining globals
string scoreLabel,endLabel;
GLuint programID;
int score=0;
double mouse_pos_x, mouse_pos_y;
double new_mouse_pos_x, new_mouse_pos_y;
float old_time; // Time in seconds
float game_start_timer; // Time in second
float scoreLabel_x,scoreLabel_y,endLabel_x,endLabel_y,timer_x,timer_y,game_timer,zoom_camera=1,x_change=0,y_change=0;
int e_left=-400,e_right=400,e_up=300,e_down=-300,game_e_left=e_left+71,game_e_up=e_up-80,game_e_down=e_up-80;
int laser_count=0;
float speed_x_c=(float)(e_right-e_left)/50;
float speed_y_c=(float)(e_up-e_down)/50;
float speed_laser=(speed_y_c+speed_x_c)/4;
bool CursorOnScreen=0,lose=0;
map<string,vector<game_object> > all_objects;
vector<game_object> canon_vector ;
vector<game_object> frame,mirrors,buckets;
vector<int>kill_laser;
map<int,game_object>lasers,blocks;
float canon_Radius=30,mirror_width=40,mirror_height=5;
int limit=2*e_right;
float block_width=10,block_height=10,bucket_width=35,bucket_height=70,block_speed_least=speed_y_c/40,block_speed_max=speed_y_c/5;
bool l=0,r=0,m=0,n=0,pause=0; //keys
bool Right_mouse_on;
glm::vec3 Saved_mouse;
double pan_timer=glfwGetTime();
bool enable_shhot=0;
bool canon_selected,redb_selected,greenb_selected,leftmouse,rightmouse;
int lives=3,level=1;float sb=1;bool lost=0;
glm::vec3 g=glm::vec3(0,speed_y_c/300,0);
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
void cursor_enter_callback(GLFWwindow* window, int entered)
{
		//cout<<"entered"<<" "<<entered<<endl;
		if (entered) CursorOnScreen = 1 ;
		else CursorOnScreen = 0 ;
}
// Returns the mouse coordinates translated according to our coordinate system
glm::vec3 GetMouseCoordinates(GLFWwindow* window)
{
		double CursorX,CursorY ;
		glfwGetCursorPos(window, &CursorX, &CursorY) ;
		return glm::vec3(CursorX+e_left,CursorY+e_down,0) ;
}
void check_pan(void);
void pan(GLFWwindow * window)
{
		glm::vec3 m=GetMouseCoordinates(window) - Saved_mouse;
		if(m[0]>15) x_change+=15;
		else if(m[0]<-15) x_change-=15;
		if(m[1]>15) y_change+=15;
		else if(m[1]<-15) y_change-=15;
		check_pan();
}
// find angle from A to B : assuming both are normalized vectors
float FindAngle(glm::vec3 A,glm::vec3 B)
{
		float theta = acos(dot(A,B)) ;
		if(cross(A,B)[2] <= 0 ) theta *= -1 ;
		return theta ;
}
glm::vec3 FindCurrentDirection(glm::vec3 A,glm::vec3 B)
{
		glm::vec3 C = A-B ;
		if(C == glm::vec3(0,0,0)) return glm::vec3(1,0,0) ;
		return normalize(C) ;
}

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
		Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(300.0f/zoom_camera+y_change), (float)(-300.0f/zoom_camera+y_change), 0.1f, 500.0f);
}

//Ensure the panning does not go out of the map
void check_pan(){
		//cout<<"Unwanted function called"<<endl;
		if(x_change-400.0f/zoom_camera<-400)
				x_change=-400+400.0f/zoom_camera;
		else if(x_change+400.0f/zoom_camera>400)
				x_change=400-400.0f/zoom_camera;
		if(y_change-300.0f/zoom_camera<-300)
				y_change=-300+300.0f/zoom_camera;
		else if(y_change+300.0f/zoom_camera>300)
				y_change=300-300.0f/zoom_camera;
		Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(300.0f/zoom_camera+y_change), (float)(-300.0f/zoom_camera+y_change), 0.1f, 500.0f);
}

void initKeyboard(){

}

//function to move cannonaim
void move_canon(int u,float radius)
{
		//cout<<"Move cannon called"<<endl;
		vector<game_object> &r=all_objects["canon"];
		for(auto &it: r)
		{
				glm::vec3 temp=it.center+glm::vec3(0,u * speed_y_c,0);
				if((temp[1]+radius)<=game_e_up && (temp[1]+radius)>=-1*game_e_down){
						it.center=temp;
						if(it.is_rotate) it.rotation_center += glm::vec3(0,u * speed_y_c,0) ;
				}

		}
}
void RotateCannon(GLFWwindow* window)
{
		if(!CursorOnScreen) return ;
		glm::vec3 Mouse = GetMouseCoordinates(window) ;
		vector<game_object> &Cannon=all_objects["canon"];
		for(auto &it:Cannon) if(it.is_rotate)
		{
				// cout<<"int rotate cannon cr is " ; FN(i,3) cout<<it.rotation_center[i]<<" " ; cout<<endl ;
				it.angle = normalize(Mouse - it.rotation_center) ;
		}
}
void Laser();
void movebucket(glm::vec3,COLOR,glm::vec3);
void changespeed(int n)
{
	for(auto &it:blocks)
	{
		if(n==1) if(glm::length(it.s.speed) < block_speed_max) it.s.speed=it.s.angle * (float) block_speed_max,sb=2;
		else if(n==-1) if(glm::length(it.s.speed)> block_speed_least) it.s.speed=it.s.angle * (float) block_speed_least,sb=0.5;
	}
}
void RotateCannon_keyboard(bool d)
{
	glm::mat3 r=glm::mat3(glm::vec3(cos(acos(-1)/72),sin(acos(-1)/72),0),glm::vec3(sin(acos(-1)/72)*(float)-1,cos(acos(-1)/72),0),glm::vec3(0,0,1));
	glm::mat3 r2=glm::mat3(glm::vec3(cos(acos(-1)/72),sin(acos(-1)/72)*(float)-1,0),glm::vec3(sin(acos(-1)/72),cos(acos(-1)/72),0),glm::vec3(0,0,1));
	for(auto &it:all_objects["canon"])
	{
		if(d==0) it.angle=normalize(r*it.angle); //rotating by 5 degrees
		if(d==1) it.angle=normalize(r2*it.angle); //rotating by 5 degrees
	}
}
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
		//cout<<"pressed key"<<" "<<key<<endl;
		if (action == GLFW_RELEASE) {
				switch (key) {
						case GLFW_KEY_UP:
								move_canon(-1,canon_Radius);
								break;
						case GLFW_KEY_DOWN:
								move_canon(1,canon_Radius);
								break;
						case GLFW_KEY_RIGHT:
								r=false;
								break;
						case GLFW_KEY_LEFT:
								l=0;
								break;
						case GLFW_KEY_LEFT_CONTROL:
								n=0;
								break;
						case GLFW_KEY_RIGHT_CONTROL:
								n=0;
								break;
						case GLFW_KEY_LEFT_ALT:
								m=0;
								break;
						case GLFW_KEY_RIGHT_ALT:
								m=0;
								break;
						case GLFW_KEY_N:
								changespeed(-1); //decrese speed
								break;
						case GLFW_KEY_M:
								changespeed(1); //increase speed
								break;
						case GLFW_KEY_A:
								RotateCannon_keyboard(1); //0 for upwards
								break;
						case GLFW_KEY_D:
								RotateCannon_keyboard(0); //1 for downwards
								break;
				}
		}
		else if (action == GLFW_PRESS) {
				switch (key) {
						case GLFW_KEY_ESCAPE:
								quit(window);
								break;
						case GLFW_KEY_RIGHT:
								r=true;
								break;
						case GLFW_KEY_LEFT:
								l=true;
								break;
						case GLFW_KEY_SPACE:
								Laser();
								break;
						case GLFW_KEY_LEFT_CONTROL:
								n=true;
								break;
						case GLFW_KEY_RIGHT_CONTROL:
								n=true;
								break;
						case GLFW_KEY_LEFT_ALT:
								m=true;
								break;
						case GLFW_KEY_RIGHT_ALT:
								m=true;
								break;
						case GLFW_KEY_P:
								pause=pause^1;
								break;
								case GLFW_KEY_T:
										enable_shhot=enable_shhot^1;
										break;
						default:
								break;
				}
		}
		else if(action ==GLFW_REPEAT)
		{
				switch (key) {
						case GLFW_KEY_UP:
								move_canon(-1,canon_Radius);
								break;
						case GLFW_KEY_DOWN:
								move_canon(1,canon_Radius);
								break;
						case GLFW_KEY_RIGHT:
								r=true;
								break;
						case GLFW_KEY_LEFT:
								l=true;
								break;
						case GLFW_KEY_LEFT_CONTROL:
								n=true;
								break;
						case GLFW_KEY_RIGHT_CONTROL:
								n=true;
								break;
						case GLFW_KEY_LEFT_ALT:
								m=true;
								break;
						case GLFW_KEY_RIGHT_ALT:
								m=true;
								break;
						case GLFW_KEY_A:
								RotateCannon_keyboard(1);
								break;
						case GLFW_KEY_D:
								RotateCannon_keyboard(0);
								break;

				}
		}
		if(n==1&&l==1) movebucket(glm::vec3(5,0,0),red ,glm::vec3(-1,0,0));
		else if(n==1&&r==1) movebucket(glm::vec3(5,0,0),red ,glm::vec3(1,0,0));
		else if(m==1&&l==1)movebucket(glm::vec3(5,0,0),green ,glm::vec3(-1,0,0));
		else if(m==1&&r==1)movebucket(glm::vec3(5,0,0),green ,glm::vec3(1,0,0));
}

/* Executed for character input (like in text boxes) */
void keyboardChar(GLFWwindow* window, unsigned int key)
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

}

void mouse_release(GLFWwindow* window, int button){

}
void checkselection(GLFWwindow* window)
{
	glm::vec3 m=GetMouseCoordinates(window);
	for(auto &it:all_objects["canon"])
	{
		glm::vec3 r=m-it.center;
		if(glm::length(r)<=it.radius) canon_selected=1;
	}
	for(auto it:all_objects["buckets"])
	{
		glm::vec3 p=normalize(cross(glm::vec3(0,0,1),it.center));
		if(abs(dot(m-it.center,p))<=bucket_height/2 &&abs(dot(m-it.center,p))<=bucket_width/2 )
		{
			if(it.color.r==red.r&&it.color.g==red.g&&it.color.b==red.b) redb_selected=1;
			else greenb_selected=1;
		}
	}
}
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
		switch (button)
		{
				case GLFW_MOUSE_BUTTON_LEFT:
						if (action == GLFW_RELEASE)
						{
								if(enable_shhot) Laser() ;
								leftmouse=canon_selected=redb_selected=greenb_selected=0;
						}
						if (action==GLFW_PRESS) leftmouse=1,checkselection(window);
						break;

				case GLFW_MOUSE_BUTTON_RIGHT:
						if (action == GLFW_RELEASE) { right_mouse_clicked=false; }
						if (action == GLFW_PRESS)
						{
								right_mouse_clicked=true;
								Saved_mouse=GetMouseCoordinates(window);
						}
						break;
				default:
						break;
		}
}
void reshapeWindow (GLFWwindow* window, int width, int height)
{
		int fbwidth=width, fbheight=height;
		/* With Retina display on Mac OS X, GLFW's FramebufferSize
		   is different from WindowSize */
		glfwGetFramebufferSize(window, &fbwidth, &fbheight);
		GLfloat fov = 90.0f;
		glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
		Matrices.projection = glm::ortho((float)e_left,(float) e_right, (float)e_up, (float) e_down, 0.1f, 500.0f);
}
void CreateCircle(string name,COLOR color,glm::vec3 centre,float radius,int parts,bool fill)
{
		GLfloat vertex_buffer_data[parts*9];
		GLfloat color_buffer_data[parts*9];
		int i,j;
		float angle=(2*M_PI/parts);
		float current_angle = 0;
		VAO* circle;
		FN(i,parts)
		{
				FN(j,3)
				{
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
		if(fill) circle= create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);
		else circle=create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_LINE);
		game_object GO={};
		GO.color = color;
		GO.name=name;
		//  GO.name = name;
		GO.object = circle;
		GO.center=centre;
		GO.radius=radius;
		GO.angle=glm::vec3(1,0,0);
		GO.speed=glm::vec3(0,0,0);
		if(name=="canon2")GO.is_rotate=1; else GO.is_rotate=0;
		canon_vector.push_back(GO);
}
void createcanon(string name,COLOR colorA,COLOR colorB,glm::vec3 centre,float radius1,float radius2)
{
		glm::vec3 c1=centre - glm::vec3(radius1,0,0);
		CreateCircle("canon1",colorA,c1,radius1,120,1);
		glm::vec3 c2=centre + glm::vec3(radius2,0,0);
		CreateCircle("canon2",colorB,c2,radius2,120,1);
		for(auto &it:canon_vector) if(it.name=="canon2") it.rotation_center=c1;
		all_objects["canon"] = canon_vector ;

}
VAO* createRectangle (string name, COLOR colorA, COLOR colorB, COLOR colorC, COLOR colorD, glm::vec3 centre, float height, float width, string component)
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
		return rectangle;
}
void set_frame(COLOR color,float top,float bottom,float height)
{
		game_object FM={};
		FM.object=createRectangle("frame",color,color,color,color,glm::vec3(0,e_up-top-height/2,0),height,2*e_right,"frame");
		FM.name="frame1";
		FM.is_rotate=0;
		FM.angle=normalize(glm::vec3(1,0,0));
		FM.center=glm::vec3(0,e_up-top-height/2,0);
		frame.push_back(FM);
		FM.object=createRectangle("frame",color,color,color,color,glm::vec3(0,e_down+bottom+height/2,0),height,2*e_right,"frame");
		FM.center=glm::vec3(0,e_down+bottom+height/2,0);
		frame.push_back(FM);
		all_objects["frame"]=frame;
}
void create_mirror(glm::vec3 center1,glm::vec3 center2)
{
		game_object m;
		m.height=mirror_height;
		m.width=mirror_width;
		m.is_rotate=true;
		m.rotation_center = m.center=center1;
		m.angle= normalize(glm::vec3(-1,1,0)) ;
		m.object=createRectangle("mirror",white,white,white,white,center1,m.height,m.width,"m");
		mirrors.push_back(m);
		m.height=mirror_height;
		m.width=mirror_width;
		m.angle= normalize(glm::vec3(1,1,0));
		m.rotation_center = m.center=center2;
		m.object=createRectangle("mirror",white,white,white,white,center1,m.height,m.width,"m");
		m.height=mirror_width;m.width=mirror_height;
		mirrors.push_back(m);
		m.center=m.rotation_center=center1-center2;
		m.angle= normalize(glm::vec3(1,1,0)) ;
		m.object=createRectangle("mirror",white,white,white,white,center1,m.height,m.width,"m");
		mirrors.push_back(m);
		all_objects["mirrors"]=mirrors;
}
void rotate_mirror()
{
  for(auto &it:all_objects["mirrors"])
  {
    glm::mat3 r=glm::mat3(glm::vec3(cos(acos(-1)/72),sin(acos(-1)/72),0),glm::vec3(sin(acos(-1)/72)*(float)-1,cos(acos(-1)/72),0),glm::vec3(0,0,1));
    it.angle=normalize(r*it.angle);
  }
}
void Laser()
{
		//cout<<"laser"<<endl;
		laser_count++;
		vector<game_object> c=all_objects["canon"];
		game_object tmp;
		tmp.width=30; tmp.height=3;
		tmp.is_rotate = true ;
		tmp.angle=c[1].angle;
		tmp.speed=tmp.angle * speed_laser;
		//cout<<"angle"<<c[1].angle[0]<<c[1].angle[1]<<endl;
		tmp.center = c[0].center + c[1].angle*(float)(c[0].radius+ 2*c[1].radius+ tmp.width/2) ;
		tmp.rotation_center = tmp.center ;
		tmp.object = createRectangle("laser",red,red,red,red,tmp.center,tmp.height,tmp.width,"laser") ;
		lasers[laser_count]=tmp;

}
void move_Laser()
{
		//cout<<"movelaser"<<endl;
		for(auto &it:lasers)
		{
				it.s.center = it.s.rotation_center = it.s.center + it.s.speed;
				glm::vec3 n = it.s.center + it.s.angle*it.s.width ;
				if(n[1]>=game_e_down || n[1]<=-1*game_e_up || n[0]<=e_left||n[0]>=e_right) kill_laser.push_back(it.f);
		}
		for(auto it:kill_laser)
		{
				lasers.erase(it);
		}
		kill_laser.clear() ;
}
int randomno(int limit) {return rand()%limit;}
float block_x_coordinate()
{
		float x=e_left+canon_Radius*2+20+2*block_width;
		return x+(float)randomno(limit);
}
int block_no=0;
void createBlocks()
{
		game_object b;
		block_no++;
		float y=-1*game_e_up + block_height/2;
		float x=block_x_coordinate();
		b.center=glm::vec3(x,y,0);
		b.is_rotate=0;
		b.angle=normalize(glm::vec3(0,1,0));
		b.height=block_height;
		b.width=block_width;
		b.speed=glm::vec3(0,speed_y_c/20,0);
		b.gravity=g;
		int c=randomno(3);
		if(c==0) b.color=red;
		else if(c==1) b.color=green;
		else b.color=black;
		b.object=createRectangle("block",b.color,b.color,b.color,b.color,b.center,b.height,b.width,"block");
		blocks[block_no]=b;
}
vector<int> kill_blocks;
void moveBlocks()
{
		game_object temp;
		for(auto &it:blocks)
		{
				temp.center=it.s.center+it.s.speed;
				if(temp.center[0]<=e_down) kill_blocks.push_back(it.f);
				else {
						it.s.center+=it.s.speed;
						it.s.rotation_center=it.s.center;
						it.s.speed+=it.s.gravity;
				}
		}
		for(auto it:kill_blocks) blocks.erase(it);
		kill_blocks.clear();
}
void createbuckets()
{
		game_object t;
		t.height=bucket_height;
		t.width=bucket_width;
		t.center=glm::vec3(game_e_left+bucket_width/2,e_up-bucket_height/2,0);
		t.color=red;
		t.rotation_center=t.center;
		t.angle=glm::vec3(1,0,0);
		t.object=createRectangle("bucket",red,red,red,red,t.center,t.height,t.width,"b");
		t.is_rotate=0;
		buckets.push_back(t);
		t.center=glm::vec3(e_right-bucket_width,e_up-bucket_height/2,0);
		t.color=green;
		t.rotation_center=t.center;
		t.object=createRectangle("bucket",lightgreen,lightgreen,lightgreen,lightgreen,t.center,t.height,t.width,"b");
		buckets.push_back(t);
		all_objects["buckets"]=buckets;
}
void movebucket(glm::vec3 location,COLOR color,glm::vec3 angle)
{
		//at buckets[0] red bucket and at bucket[1] green bucket
		for(auto &it:all_objects["buckets"])
		{
				if(it.color.r==color.r &&it.color.g==color.g &&it.color.b==color.b)
				{
						glm::vec3 temp=it.center+location*angle;
						if((temp[0]+bucket_width/2*angle[0])<e_right && (temp[0]+bucket_width/2*angle[0])>e_left)
								it.center=temp,it.rotation_center=temp,it.angle=angle;
				}
		}
}
void movebucket_mouse(glm::vec3 location,COLOR color)
{
	for(auto &it:all_objects["buckets"])
	{
		if(it.color.r==color.r &&it.color.g==color.g &&it.color.b==color.b)
		{
				glm::vec3 temp=glm::vec3(location[0],it.center[1],0);
				if((temp[0]+bucket_width/2)<e_right && (temp[0]-bucket_width/2)>e_left)
						it.center=temp,it.rotation_center=temp;
		}
	}
}
void movecanon_mouse(glm::vec3 location)
{
	vector<game_object> &r=all_objects["canon"];
	for(auto &it: r)
	{
			glm::vec3 temp=glm::vec3(it.center[0],location[1],0);
			if((temp[1]+canon_Radius)<=game_e_up && (temp[1])>=-1*game_e_down){
					it.center=temp;
					if(it.is_rotate) it.rotation_center =it.center ;
			}

	}
}
bool checkCollision(game_object x,game_object y)
{
		glm::vec3 p=cross(x.angle,glm::vec3(0,0,1));
		p=normalize(p);
		glm::vec3 ver[4];
		ver[2]=x.center+x.angle*(float)(x.width/2)-p*(float)(x.height/2);
		ver[0]=x.center+x.angle*(float)(x.width/2)+p*(float)(x.height/2);
		ver[1]=x.center-x.angle*(float)(x.width/2)+p*(float)(x.height/2);
		ver[3]=x.center-x.angle*(float)(x.width/2)-p*(float)(x.height/2);
		p=cross(y.angle,glm::vec3(0,0,1));
		p=normalize(p);
		FN(i,4)
		{

				float d1=abs(dot(ver[i]-y.center,p));
				float d2=abs(dot(ver[i]-y.center,y.angle));
				if(d1>(y.height/2)||d2>(y.width/2)) ; else return true;
		}
		return false;
}
void reflect(game_object &a,game_object &b,float speed)
{
		//cout<<"reflect"<<endl;
		glm::vec3 t1,t2,t3;
		t1=cross(a.center -b.center,a.angle);
		t2=cross(b.angle,a.angle);
		float t=glm::length(t1)/glm::length(t2);
		float q=dot(t1,t2);
		if(q<0) t=t*(float)-1;
		t3=b.center+b.angle*(float)t;

		t2=cross(b.angle,glm::vec3(0,0,1));t2=normalize(t2);
		if(dot(t2,a.center - b.center) < 0) t2 = t2*(float)-1 ;

		t1=a.center-t3;
		q=abs(dot(t1,t2)); t=dot(b.angle,t1);
		a.center=t2*q+b.angle*t+t3;
		t1=a.angle+t3;
		q=abs(dot(t1,t2)); t=dot(b.angle,t1);
		speed=glm::length(a.speed);
		a.angle=t2*q+b.angle*t-t3;
		a.angle = normalize(a.angle) ;
		a.speed=a.angle*speed;
}
void detectCollisions(void)
{
		for(auto it1:lasers)
		{
				for(auto it2:blocks)
				{
						bool c=checkCollision(it2.s,it1.s);
						if(c==true)
						{
								kill_blocks.push_back(it2.f),kill_laser.push_back(it1.f);
								if(it2.s.color.r==black.r && it2.s.color.b==black.b && it2.s.color.g==black.g) score+=100;
								else score-=5;
						}
				}
		}
		for(auto it:kill_blocks) blocks.erase(it);
		kill_blocks.clear();
		for(auto it:kill_laser)
		{
				lasers.erase(it);
		}
		kill_laser.clear() ;

		// Collisions with mirror
		for(auto &it1:lasers)
		{
				for(auto &it2:all_objects["mirrors"])
				{
						if(checkCollision(it1.s,it2)==true) reflect(it1.s,it2,speed_laser);
				}
		}
		for(auto &it1:blocks)
		{
				for(auto &it2:all_objects["mirrors"])
				{
						if(checkCollision(it1.s,it2)==true) reflect(it1.s,it2,speed_y_c/20);
				}
				for(auto &it2:all_objects["buckets"])
				{
						if(checkCollision(it1.s,it2)==true&&checkCollision(all_objects["buckets"][0],all_objects["buckets"][1])==false)
						{
							//	cout<<"b"<<endl;
								kill_blocks.push_back(it1.f);
								if(it1.s.color.r==it2.color.r &&it1.s.color.g==it2.color.g && it1.s.color.b==it2.color.b) score+=100;
								else if(it1.s.color.r==black.r && it1.s.color.g==black.g && it1.s.color.b==black.b){ lives-=1;if(lives<0) lost=1,lives=0;}
								else score-=5;
						}
				}
		}
		for(auto it:kill_blocks) blocks.erase(it);
		kill_blocks.clear();

}
bool level_won()
{
	if(score>=500 && level==1)
	{
		level=2;
		score=0;
		lives=3;
		sb=1;
		return true;
	}
	return false;
}
bool game_lost()
{
	if(lost==1) return true;
	return false;
}
bool game_won()
{
	if(score>1000 && level>=2) return true;
	return false;
}
double last_update_time = glfwGetTime(), current_time,lastbtime=glfwGetTime();
void draw(GLFWwindow* window)
{
		//game_timer=(int)(90-(glfwGetTime()-game_start_timer));
		// clear the color and depth in the frame buffer
		//to pause
		if(pause==1) return;
		if(level_won())
		{
			cout<<"You passed this level"<<endl;
			lasers.erase(lasers.begin(),lasers.end());
			blocks.erase(blocks.begin(),blocks.end());
			g=glm::vec3(0,speed_y_c/100,0);
		}
		cout<<"level"<<" "<<level<<" "<<endl;
		cout<<"score"<<" "<<score<<" "<<endl;
		cout<<"lives"<<" "<<lives<<endl;
		cout<<"speed"<<" "<<sb<<"X"<<endl;

		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram (programID);
		Matrices.view = glm::lookAt(glm::vec3(0,0,1), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
		//  Don't change unless you are sure!!
		glm::mat4 MVP ;
		glm::mat4 VP = Matrices.projection * Matrices.view;
		if(current_time - last_update_time >=0.01)
		{
				//cout<<"moving"<<endl ;
				last_update_time = current_time ;
				move_Laser() ;
				moveBlocks();
        if(level==2) rotate_mirror();
		}
		if(current_time- pan_timer >=0.15)
		{
				if(right_mouse_clicked) pan(window);
				pan_timer=current_time;
		}
		detectCollisions();
		if(current_time-lastbtime>=0.5) {lastbtime=current_time;createBlocks();}
	if(enable_shhot==1)RotateCannon(window);
	glm::vec3 m=GetMouseCoordinates(window);
	if(canon_selected) {
		movecanon_mouse(m);
	}
	if(redb_selected)
	{
			movebucket_mouse(m,red);
	}
	if(greenb_selected) movebucket_mouse(m,green);
		for(auto it: all_objects)
		{

				for(auto it2: it.s )
				{
						Matrices.model = glm::mat4(1.0f) * glm::translate (it2.center);
						if(it2.is_rotate)
						{
								Matrices.model = glm::translate (it2.rotation_center*(float)-1 ) * Matrices.model ;
								float theta = FindAngle(FindCurrentDirection(it2.center,it2.rotation_center),it2.angle) ;
								Matrices.model = glm::rotate(theta, glm::vec3(0,0,1)) * Matrices.model ;
								Matrices.model = glm::translate (it2.rotation_center) * Matrices.model ;
						}

						MVP = VP * Matrices.model; // MVP = p * V * M
						glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
						draw3DObject(it2.object);
				}
		}
		for(auto it2:lasers)
		{
				auto it = it2.second ;
				Matrices.model = glm::translate (it.center);
				if(it.is_rotate)
				{
						Matrices.model = glm::translate (it.rotation_center*(float)-1 ) * Matrices.model ;
						float theta = FindAngle(FindCurrentDirection(it.center,it.rotation_center),it.angle) ;
						Matrices.model = glm::rotate(theta, glm::vec3(0,0,1)) * Matrices.model ;
						Matrices.model = glm::translate (it.rotation_center) * Matrices.model ;
				}
				MVP = VP * Matrices.model; // MVP = p * V * M
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(it.object);
		}
		for(auto it2:blocks)
		{
				auto it = it2.second ;
				Matrices.model = glm::translate (it.center);
				if(it.is_rotate)
				{
						Matrices.model = glm::translate (it.rotation_center*(float)-1 ) * Matrices.model ;
						float theta = FindAngle(FindCurrentDirection(it.center,it.rotation_center),it.angle) ;
						Matrices.model = glm::rotate(theta, glm::vec3(0,0,1)) * Matrices.model ;
						Matrices.model = glm::translate (it.rotation_center) * Matrices.model ;
				}
				MVP = VP * Matrices.model; // MVP = p * V * M
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(it.object);
		}
}



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
void initGL (GLFWwindow* window, int width, int height)
{
		//creating objects
		//createRectangle("cannonpower1",cratebrown2,cratebrown2,cratebrown2,cratebrown2,glm::vec3(-3,0,0),1,2,"background");
		//createRectangle("cannonpower1",darkbrown,darkbrown,darkbrown,darkbrown,glm::vec3(-1.6,0,0),0.2,0.8,"background");
		createcanon("canon",black,brown3,glm::vec3(e_left+2*canon_Radius,0,0),canon_Radius,10);
		set_frame(black,80,40,5);
		create_mirror(glm::vec3(0,0,0),glm::vec3(80,80,0));
		createbuckets();
		// Create and compile our GLSL program from the shaders
		programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
		// Get a handle for our "MVP" uniform
		Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


		reshapeWindow (window, width, height);

		// Background color of the scene
		glClearColor (lightpink.r,lightpink.g,lightpink.b, 0.0f); // R, G, B, A
		glClearDepth (1.0f);

		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);

		cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
		cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
		cout << "VERSION: " << glGetString(GL_VERSION) << endl;
		cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
void* Writer(void * i)
{
	ao_device* device;
    ao_sample_format format;
    int defaultDriver;
    WavHeader header;

    std::ifstream file;
    file.open("explosion.wav", std::ios::binary | std::ios::in);

    file.read(header.id, sizeof(header.id));
    //assert(!std::memcmp(header.id, "RIFF", 4)); //is it a WAV file?
    file.read((char*)&header.totalLength, sizeof(header.totalLength));
    file.read(header.wavefmt, sizeof(header.wavefmt)); //is it the right format?
    //assert(!std::memcmp(header.wavefmt, "WAVEfmt ", 8));
    file.read((char*)&header.format, sizeof(header.format));
    file.read((char*)&header.pcm, sizeof(header.pcm));
    file.read((char*)&header.channels, sizeof(header.channels));
    file.read((char*)&header.frequency, sizeof(header.frequency));
    file.read((char*)&header.bytesPerSecond, sizeof(header.bytesPerSecond));
    file.read((char*)&header.bytesByCapture, sizeof(header.bytesByCapture));
    file.read((char*)&header.bitsPerSample, sizeof(header.bitsPerSample));
    file.read(header.data, sizeof(header.data));
    file.read((char*)&header.bytesInData, sizeof(header.bytesInData));

    ao_initialize();

    defaultDriver = ao_default_driver_id();

    memset(&format, 0, sizeof(format));
    format.bits = header.format;
    format.channels = header.channels;
    format.rate = header.frequency;
    format.byte_format = AO_FMT_LITTLE;

    device = ao_open_live(defaultDriver, &format, NULL);
    if (device == NULL) {
        std::cout << "Unable to open driver" << std::endl;
        //return;
    }


    char* buffer = (char*)malloc(BUF_SIZE * sizeof(char));

    // determine how many BUF_SIZE chunks are in file
    int fSize = header.bytesInData;
    int bCount = fSize / BUF_SIZE;

    for (int i = 0; i < bCount; ++i) {
        file.read(buffer, BUF_SIZE);
				if(lost==1) {ao_close(device);
		    ao_shutdown();pthread_exit((void *)NULL);}
        ao_play(device, buffer, BUF_SIZE);
    }

    int leftoverBytes = fSize % BUF_SIZE;
  //  std::cout << leftoverBytes;
    file.read(buffer, leftoverBytes);
    memset(buffer + leftoverBytes, 0, BUF_SIZE - leftoverBytes);
		if(lost==1) {ao_close(device);
    ao_shutdown();pthread_exit((void *)NULL);}
    ao_play(device, buffer, BUF_SIZE);

    ao_close(device);
    ao_shutdown();

}
pthread_t Writer_thr[3];
int main (int argc, char** argv)
{
		int width = 800;
		int height = 600;
		scoreLabel="SCORE";
		scoreLabel_x=175;
		scoreLabel_y=250;
		timer_y=250;
		timer_x=20;
		endLabel="";
		endLabel_x=-160;
		GLFWwindow* window = initGLFW(width, height);
		pthread_create(&Writer_thr[1],NULL,Writer,(void*) NULL);
		glfwSetCursorEnterCallback(window, cursor_enter_callback);
		initGL (window, width, height);

		srand(glfwGetTime());


		glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);

		game_start_timer=glfwGetTime();
		old_time = glfwGetTime();
		/* Draw in loop */
		while (!glfwWindowShouldClose(window)) {

				current_time = glfwGetTime(); // Time in seconds
				// OpenGL Draw commands
				draw(window);
				if(game_lost()==1)
				{
					cout<<"YOU LOST"<<endl;
					pthread_join(Writer_thr[1],NULL);
					glfwTerminate();
					exit(EXIT_SUCCESS);

				}
				if(game_won())
				{
					cout<<"CONGRATS!! YOU WON"<<endl;
					pthread_join(Writer_thr[1],NULL);
					glfwTerminate();
					exit(EXIT_SUCCESS);
				}
				old_time=current_time;

				// Swap Frame Buffer in double buffering
				glfwSwapBuffers(window);

				// Poll for Keyboard and mouse events
				glfwPollEvents();

				// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
				current_time = glfwGetTime(); // Time in seconds
				// if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
				// do something every 0.5 seconds ..
				// last_update_time = current_time;
				// }
		}
		pthread_join(Writer_thr[1],NULL);
		glfwTerminate();
		exit(EXIT_SUCCESS);
}
