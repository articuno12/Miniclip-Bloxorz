#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <time.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
using namespace std;

#define pb push_back
#define mp make_pair

struct VAO {
		GLuint VertexArrayID;
		GLuint VertexBuffer;
		GLuint ColorBuffer;
		GLuint TextureBuffer;
		GLuint TextureID ;
		GLenum PrimitiveMode;
		GLuint TexMatrixID; // For use with texture shader
		GLenum FillMode;
		int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		GLuint MatrixID; // For use with normal shader
		GLuint TexMatrixID; // For use with texture shader
} Matrices;

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
		string name;//name of object
		VAO* object;
		glm::vec3 center,speed,scale,rotation_axis,angle,gravity,up;
		COLOR color;
		bool is_rotate;
		float height,width,length,radius;
};
//int do_rot, floor_rel;;

GLuint programID, waterProgramID, fontProgramID, textureProgramID;
double last_update_time, current_time;
float camera_radius=12;
float rectangle_rotation = 0;
game_object Camera;
map<string,GLuint> Texture;
vector<game_object> normal_floor,button_floor;
vector<vector<game_object> > hidden_floor;
map<pair<int,int> , int> buttons;
int floor_width,floor_length;
float tilewidth=2;float tileheight=0.2;float tilelength=2;
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{

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
		cout << "Compiling shader : " <<  vertex_file_path << endl;
		char const * VertexSourcePointer = VertexShaderCode.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
		glCompileShader(VertexShaderID);

		// Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> VertexShaderErrorMessage( max(InfoLogLength, int(1)) );
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		cout << VertexShaderErrorMessage.data() << endl;

		// Compile Fragment Shader
		cout << "Compiling shader : " << fragment_file_path << endl;
		char const * FragmentSourcePointer = FragmentShaderCode.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
		glCompileShader(FragmentShaderID);

		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> FragmentShaderErrorMessage( max(InfoLogLength, int(1)) );
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		cout << FragmentShaderErrorMessage.data() << endl;

		// Link the program
		cout << "Linking program" << endl;
		GLuint ProgramID = glCreateProgram();
		glAttachShader(ProgramID, VertexShaderID);
		glAttachShader(ProgramID, FragmentShaderID);
		glLinkProgram(ProgramID);

		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		cout << ProgramErrorMessage.data() << endl;

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
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
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
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
		GLfloat* color_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
				color_buffer_data [3*i] = red;
				color_buffer_data [3*i + 1] = green;
				color_buffer_data [3*i + 2] = blue;
		}

		return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}
struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
		struct VAO* vao = new struct VAO;
		vao->PrimitiveMode = primitive_mode;
		vao->NumVertices = numVertices;
		vao->FillMode = fill_mode;
		vao->TextureID = textureID;

		// Create Vertex Array Object
		// Should be done after CreateWindow and before any other GL calls
		glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
		glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
		glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

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

		glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
		glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
		glVertexAttribPointer(
						2,                  // attribute 2. Textures
						2,                  // size (s,t)
						GL_FLOAT,           // type
						GL_FALSE,           // normalized?
						0,                  // stride
						(void*)0            // array buffer offset
						);

		return vao;
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
void draw3DTexturedObject (struct VAO* vao)
{
		// Change the Fill Mode for this object
		glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

		// Bind the VAO to use
		glBindVertexArray (vao->VertexArrayID);

		// Enable Vertex Attribute 0 - 3d Vertices
		glEnableVertexAttribArray(0);
		// Bind the VBO to use
		glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

		// Bind Textures using texture units
		glBindTexture(GL_TEXTURE_2D, vao->TextureID);

		// Enable Vertex Attribute 2 - Texture
		glEnableVertexAttribArray(2);
		// Bind the VBO to use
		glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

		// Draw the geometry !
		glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

		// Unbind Textures to be safe
		glBindTexture(GL_TEXTURE_2D, 0);
}
GLuint createTexture (const char* filename)
{
		GLuint TextureID;
		// Generate Texture Buffer
		glGenTextures(1, &TextureID);
		// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
		glBindTexture(GL_TEXTURE_2D, TextureID);
		// Set our texture parameters
		// Set texture wrapping to GL_REPEAT
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set texture filtering (interpolation)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Load image and create OpenGL texture
		int twidth, theight;
		unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
		SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

		return TextureID;
}
/**************************
 * Customizable functions *
 **************************/
float camera_rotation_angle = (float)30*M_PI/(float)180;
void update_Camera()
{
		Matrices.view = glm::lookAt(Camera.center,Camera.angle, Camera.up);
}
void set_Camera()
{

		Camera.center=glm::vec3(camera_radius,0,0);
		Camera.angle=glm::vec3(0, 0, 0);
		Camera.up =glm::vec3(0, 0, 1);
		update_Camera();
}
void camera_move(int d,int c)
{
		if(d==0) //implies horizontal roattion
				Camera.center=glm::rotate(Camera.center,camera_rotation_angle*c,Camera.up);
		else if(d==1) //vertical roattion
		{
				glm::vec3 normal=cross(Camera.up,Camera.angle-Camera.center);
				normal=normalize(normal);
				Camera.center=glm::rotate(Camera.center,camera_rotation_angle*c,normal);
				Camera.up=normalize(cross(Camera.angle-Camera.center,normal));
		}
		update_Camera();
}
void set_camera_radius(float d)
{
		camera_radius=glm::length(Camera.center-Camera.angle) * (float)(1+d*0.15);
		Camera.center=normalize(Camera.center-Camera.angle) * camera_radius + Camera.angle;
		update_Camera();
}
//* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
		// Function is called first on GLFW_PRESS.

		if (action == GLFW_RELEASE) {
				switch (key) {
						case GLFW_KEY_C:

								break;
						case GLFW_KEY_P:
								break;
						case GLFW_KEY_X:
								// do something ..
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
				case 'v':
						camera_move(1,1);
						break;
				case 'm':
						camera_move(0,1);
						break;
				case 'f':
						set_camera_radius(1);
						break;
				case 'n':
						set_camera_radius(-1);
						break;
				default:
						break;
		}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
		switch (button) {
				case GLFW_MOUSE_BUTTON_RIGHT:
						if (action == GLFW_RELEASE) {
								//	    rectangle_rot_dir *= -1;
						}
						break;
				default:
						break;
		}
}
void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
		// if (yoffset==-1) {
		//     camera_fov*=1.1;
		// }
		// else if(yoffset==1){
		//     camera_fov/=1.1; //make it bigger than current size
		// }
		// if(camera_fov>=2){
		// 	camera_fov=2;
		// }
		// if(camera_fov<=0.5){
		// 	camera_fov=0.5;
		// }
		// reshapeWindow(window,700,1400);
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
		int fbwidth=width, fbheight=height;
		glfwGetFramebufferSize(window, &fbwidth, &fbheight);

		GLfloat fov = M_PI/2;

		// sets the viewport of openGL renderer
		glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

		// Store the projection matrix in a variable for future use
		// Perspective projection for 3D views
		Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

		// Ortho projection for 2D views
		//Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *rectangle, *floor_vao;

// Creates the rectangle object used in this sample code
VAO* createRectangle (GLuint textureID)
{
		// GL3 accepts only Triangles. Quads are not supported
		static const GLfloat vertex_buffer_data [] = {
				-0.5, 0.5, 0.5,
				-0.5, -0.5, 0.5,
				0.5, -0.5, 0.5,
				-0.5, 0.5, 0.5,
				0.5, -0.5, 0.5,
				0.5, 0.5, 0.5,
				0.5, 0.5, 0.5,
				0.5, -0.5, 0.5,
				0.5, -0.5, -0.5,
				0.5, 0.5, 0.5,
				0.5, -0.5, -0.5,
				0.5, 0.5, -0.5,
				0.5, 0.5, -0.5,
				0.5, -0.5, -0.5,
				-0.5, -0.5, -0.5,
				0.5, 0.5, -0.5,
				-0.5, -0.5, -0.5,
				-0.5, 0.5, -0.5,
				-0.5, 0.5, -0.5,
				-0.5, -0.5, -0.5,
				-0.5, -0.5, 0.5,
				-0.5, 0.5, -0.5,
				-0.5, -0.5, 0.5,
				-0.5, 0.5, 0.5,
				-0.5, 0.5, -0.5,
				-0.5, 0.5, 0.5,
				0.5, 0.5, 0.5,
				-0.5, 0.5, -0.5,
				0.5, 0.5, 0.5,
				0.5, 0.5, -0.5,
				-0.5, -0.5, 0.5,
				-0.5, -0.5, -0.5,
				0.5, -0.5, -0.5,
				-0.5, -0.5, 0.5,
				0.5, -0.5, -0.5,
				0.5, -0.5, 0.5,

		};

		static const GLfloat color_buffer_data [] = {
				1.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f,
				1.0f, 0.0f, 1.0f,
				1.0f, 0.0f, 1.0f,
				1.0f, 0.0f, 1.0f,
				1.0f, 0.0f, 1.0f,
				1.0f, 0.0f, 1.0f,
				1.0f, 0.0f, 1.0f,
				0.0f, 1.0f, 1.0f,
				0.0f, 1.0f, 1.0f,
				0.0f, 1.0f, 1.0f,
				0.0f, 1.0f, 1.0f,
				0.0f, 1.0f, 1.0f,
				0.0f, 1.0f, 1.0f,
				1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 1.0f,

		};
		// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
		GLfloat texture_buffer_data [] = {
				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				0,1,  // TexCoord 1 - bot left
				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left

				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				0,1,  // TexCoord 1 - bot left
				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left

				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				0,1,  // TexCoord 1 - bot left
				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left

				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				0,1,  // TexCoord 1 - bot left
				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left

				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				0,1,  // TexCoord 1 - bot left
				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left

				0,1, // TexCoord 1 - bot left
				1,1, // TexCoord 2 - bot right
				1,0, // TexCoord 3 - top right

				0,1,  // TexCoord 1 - bot left
				1,0, // TexCoord 3 - top right
				0,0, // TexCoord 4 - top left

		};

		// create3DTexturedObject creates and returns a handle to a VAO that can be used later
		return create3DTexturedObject(GL_TRIANGLES, 12*3, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);

		// create3DObject creates and returns a handle to a VAO that can be used later
		//rectangle = create3DObject(GL_TRIANGLES, 12*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}
//***************************
//CUBOID
//***************************
game_object cuboid;
int RandomNo(int limit) { return rand()%limit ;}
void create_cuboid()
{
		cuboid.height=tilewidth+tilelength;
		cuboid.length=tilelength;
		cuboid.width=tilewidth;
		cuboid.scale=glm::vec3(cuboid.width,cuboid.length,cuboid.height);
		cuboid.up=glm::vec3(0,0,1);
		cuboid.rotation_axis=glm::vec3(0,0,1);
		cuboid.angle=glm::vec3(-1,0,0);
		cuboid.center=glm::vec3(0,0,0);
		cuboid.object=createRectangle(Texture["cuboid"]);
}
glm::mat4 Rotatecuboid(glm::vec3 X,glm::vec3 Y,glm::vec3 Z)
{
		glm::vec4 row1(X.x,X.y,X.z,0) ;
		glm::vec4 row2(Y.x,Y.y,Y.z,0) ;
		glm::vec4 row3(Z.x,Z.y,Z.z,0) ;
		glm::vec4 row4(0,0,0,1) ;
		return glm::mat4(row1,row2,row3,row4) ;
}
void align_block(vector<vector<int> > &Grid)
{
		vector< pair<int,int> > Available  ;
		for(int i=0;i<(int)Grid.size();++i) if(Grid[i][0] == 1 || (Grid[i][0]%2 == 0 && Grid[i][0] > 0) ) Available.pb({i,0}) ;
		// there must be atleast one spot available
		if((int)Available.size()>0)
		{
				auto it = Available[ RandomNo((int)Available.size()) ] ;
				cuboid.center.x = (it.first - floor_width/2)*tilewidth;
				cuboid.center.y = (it.second - floor_length/2)*tilelength ;
		}
}
glm::vec3 RightOfBlock()
{
	glm::vec3 right,u,r ;
	right = normalize(cross(Camera.angle - Camera.center,Camera.up)) ;
	u=abs(dot(right,glm::vec3(1,0,0)));
	r=abs(dot(right,glm::vec3(0,1,0)));
	if(u>r) right=normalize(glm::vec3(1,0,0) * dot(right,glm::vec3(1,0,0)));
	else right=normalize(glm::vec3(0,1,0) * dot(right,glm::vec3(0,1,0)));
	return right;
}
glm::vec3 FrontOfBlock(void)
 {
	  glm::vec3 temp=cross(glm::vec3(0,0,1),RightOfBlock());
		return normalize(temp);
}
float currentBlockHeight()
{
	if(abs(dot(glm::vec3(0,0,1),cuboid.up)) > 0.98) return tilewidth+tilelength;
	else return tilewidth;
}
vector<vector<int> > read_floor(int level)
{
		string filename;
		filename="Levels/" + to_string(level) + ".txt";
		ifstream in(filename.c_str()) ;
		int t=0;
		if(!in.is_open())
		{
				cout<<"Unable to open Level file"<<endl ;
				exit(0) ;
		}
		in>>floor_width>>floor_length;
		vector<vector<int> > grid(floor_width);
		for(int i=0;i<floor_width;++i)
		{
				for(int j=0;j<floor_length;++j)
				{
						in>>t;
						grid[i].pb(t);
						cout<<"im"<<endl;
				}
		}
		in.close() ;
		return grid;
}
void make_floor(int level)
{
		normal_floor.clear(),hidden_floor.clear(),button_floor.clear();
		vector<vector<int> > floor_plan; floor_plan=read_floor(level);
		float del=0.06;
		int max_n=0;
		for(auto &i:floor_plan)
		{
				for(auto &j:i) max_n=max(max_n,j);
		}
		//for hidden blocks numbering is 2n+1 so 2n+1/2 will give number.
		hidden_floor.resize(max_n/2);
		game_object block,hblock,bblock;
		block.object=createRectangle(Texture["live_block"]);
		block.width=tilewidth;block.height=tileheight;block.length=tilelength;
		block.scale=glm::vec3(block.width-del,block.length-del,block.height);
		block.rotation_axis=glm::vec3(0,0,1);
		hblock.object=createRectangle(Texture["hidden_block"]);
		bblock.object=createRectangle(Texture["button_block"]);
		bblock.scale=hblock.scale=block.scale;
		bblock.width=hblock.width=block.width;bblock.height=hblock.height=block.height;bblock.length=hblock.length=block.length;
		for(int i=0;i<floor_width;++i)
		{
				for(int j=0;j<floor_length;++j)
				{
						if(floor_plan[i][j]==1)
						{
								block.center = glm::vec3((i - floor_width/2)*tilewidth,(j - floor_length/2)*tilelength, -(tilelength + tilewidth)/2 - tileheight/2) ;
								normal_floor.pb(block);
						}
						else if(floor_plan[i][j]%2==0 && floor_plan[i][j]!=0)
						{
								bblock.center = glm::vec3((i - floor_width/2)*tilewidth,(j - floor_length/2)*tilelength, -(tilelength + tilewidth)/2 - tileheight/2) ;
								button_floor.pb(bblock);
								buttons[mp(i,j)] = floor_plan[i][j]/2-1;
						}
						else if(floor_plan[i][j]%2==1)
						{
								hblock.center = glm::vec3((i - floor_width/2)*tilewidth,(j - floor_length/2)*tilelength, -(tilelength + tilewidth)/2 - tileheight/2) ;
								hidden_floor[floor_plan[i][j]/2-1].pb(hblock);
						}
				}
		}
		align_block(floor_plan);
}

void draw (GLFWwindow* window, float x, float y, float w, float h, int doM, int doV, int doP)
{
		int fbwidth, fbheight;
		glfwGetFramebufferSize(window, &fbwidth, &fbheight);
		glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));


		// use the loaded shader program
		// Don't change unless you know what you are doing
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);

		glm::mat4 VP;

		VP = Matrices.projection * Matrices.view;
		glm::mat4 MVP;	// MVP = Projection * View * Model




		glUseProgram(textureProgramID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
		// Load identity to model matrix
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateRectangle = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
		// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle);
		MVP = VP * Matrices.model;

		// Copy MVP to texture shaders
		glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
		//cout<<"Done till here"<<endl ;
		for(auto &it:normal_floor)
		{
				Matrices.model = glm::translate(it.center) * glm::scale(it.scale);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DTexturedObject(it.object);
		}
		for(auto &it:button_floor)
		{
				Matrices.model = glm::translate(it.center) * glm::scale(it.scale);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DTexturedObject(it.object);
		}
		//drawing cuboid
		Matrices.model = Rotatecuboid(cuboid.angle,normalize(cross(cuboid.up,cuboid.angle)),cuboid.up) * glm::scale(cuboid.scale);
		Matrices.model = glm::translate(cuboid.center)* Matrices.model ;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DTexturedObject(cuboid.object);
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
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
				exit(EXIT_FAILURE);
				glfwTerminate();
		}

		glfwMakeContextCurrent(window);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		glfwSwapInterval( 1 );
		glfwSetFramebufferSizeCallback(window, reshapeWindow);
		glfwSetWindowSizeCallback(window, reshapeWindow);
		glfwSetWindowCloseCallback(window, quit);
		glfwSetKeyCallback(window, keyboard);      // general keyboard input
		glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
		glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
		glfwSetScrollCallback(window, mousescroll); // mouse scroll
		return window;
}
void set_textures()
{
		Texture["live_block"]=createTexture("Images/live_block.jpg");
		Texture["hidden_block"]=createTexture("Images/hidden_block.jpg");
		Texture["button_block"]=createTexture("Images/button_block.jpg");
		Texture["cuboid"]=createTexture("Images/middle.png");
}
/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
		set_Camera();
		glActiveTexture(GL_TEXTURE0);
		// Create and compile our GLSL program from the texture shaders
		textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
		// Get a handle for our "MVP" uniform
		Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");
		set_textures();


		GLint textureID5 = createTexture("Images/middle.jpg");
		createRectangle (textureID5);

		create_cuboid();
		make_floor(1);
		// Create and compile our GLSL program from the shaders
		programID = LoadShaders( "shader.vert", "shader.frag" );
		waterProgramID = LoadShaders ( "watershader.vert", "watershader.frag");

		// Get a handle for our "MVP" uniform
		Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


		reshapeWindow (window, width, height);

		// Background color of the scene
		glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
		glClearDepth (1.0f);

		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);
		//glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

int main (int argc, char** argv)
{
		int width = 600;
		int height = 600;

		GLFWwindow* window = initGLFW(width, height);

		initGL (window, width, height);
		last_update_time = glfwGetTime();
		/* Draw in loop */
		while (!glfwWindowShouldClose(window)) {

				// OpenGL Draw commands
				current_time = glfwGetTime();
				srand(glfwGetTime()) ;
				last_update_time = current_time;
				draw(window, 0, 0, 1, 1, 1, 1, 1);

				// Swap Frame Buffer in double buffering
				glfwSwapBuffers(window);

				// Poll for Keyboard and mouse events
				glfwPollEvents();
		}

		glfwTerminate();
		//    exit(EXIT_SUCCESS);
}
