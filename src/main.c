#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "SDL2/SDL_events.h"
#include "SDL2/SDL_mouse.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_stdinc.h"
#include "SDL2/SDL_timer.h"

// TODO --- fps counter

//-#- macros
// #define SCREEN_WIDTH 1280
// #define SCREEN_HEIGHT 720
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 900
#define FPS 60
const float TICKS_PER_FRAME = 1000.0f / FPS;

#define CONE_BASE_SIZE SCREEN_WIDTH

#define MAIN_DIR_PATH "./"

#define TRUE 1
#define FALSE 0

#define MOVE_SPEED 0.1f
#define CAM_ROT_SPEED 0.01f

#define DEBUG_FILE TRUE

#define MAX_VERTICES_PER_FACE 50
#define MAX_FACES_PER_MODEL 40000
#define MAX_VERTICES_PER_MODEL 40000

//-#- standard datatypes
typedef char Int8;
typedef unsigned int Uint;
typedef unsigned char Uint8;
typedef unsigned char Bool;

typedef int FResult;
#define F_BUFFER_SIZE 100
#define F_FAILURE 0b0
#define F_SUCCESS 0b1
#define F_OVERFLOW 0b10  // will also happen on for example "200text"
#define F_EOL 0b100
#define F_EOF 0b1000
#define F_WRONGTYPE 0b10000

//-#- readme naming conventions comments
/*
 * Naming convention
 *
 *
 * prefix_nameOfVariable223
 *
 * NameOfType_suffix
 *
 * ( short scope, iterators etc. )
 * _p, _x
 */

//-#- structs unions classes
typedef struct {
  float x;
  float y;
} Vec2;

typedef struct {
  float x;
  float y;
  float z;
} Vec3;

typedef struct {
  float x;
  float y;
  float z;
  float w;
} Vec4;

typedef struct {
  Vec3 v1;
  Vec3 v2;
  Vec3 v3;
} Matrix3x3;

typedef struct {
  Vec3 v1;
  Vec3 v2;
  Vec3 v3;
  Vec3 v4;
} Matrix4x4;

typedef struct {
  Bool gameIsRunning;
  SDL_Window* window;
} GeneralGameData;

typedef struct {
  Bool left;
  Bool right;
  Bool up;
  Bool down;
} MoveKeys;

typedef struct {
  Bool left;
  Bool right;
  Bool up;
  Bool down;
} MoveCamKeys;

typedef struct {
  Bool left;
  Bool right;
  Bool up;
  Bool down;
} RotateCamKeys;

typedef struct {
  RotateCamKeys rotateCamKeys;
  MoveCamKeys moveCamKeys;
} CamKeys;

typedef struct {
  MoveKeys moveKeys;
  CamKeys camKeys;
} InputKeys;

typedef struct {
  Bool mb1;
  Bool mb2;
  Bool mb3;
  Bool mb4;
  Bool mb5;
  Vec2 pos;
  Vec2 prevPos;
  Vec2 deltaPos;
} InputMouse;

typedef struct {
  Uint x;
  Uint y;
  Uint width;
  Uint height;
} Rect;

typedef struct {
  Uint8 r;
  Uint8 g;
  Uint8 b;
} Color;

typedef struct {
  Uint length;
  Vec3* array;
} Vec3Array;

typedef struct {
  float pitch;  // x
  float yaw;    // y
  float roll;   // z
} Rotation;

typedef struct {
  Vec3 pos;
  Rotation rot;
  Vec3 dirFront;
  Vec3 dirRight;
  Vec3 dirAbove;
  Matrix3x3 mYawInv;
  Matrix3x3 mRollInv;
  Matrix3x3 mPitchInv;
} Camera;

typedef Vec2 Tri2[3];

typedef struct {
  Uint length;
  Uint* array;
} Face;

typedef struct {
  Uint length;
  Face* array;
} FaceArray;

void freeFaceArray(const FaceArray faceArray) {
  for (int i = 0; i < faceArray.length; i++) {
    free(&faceArray.array[i]);
  }
  free(faceArray.array);
}

typedef struct {
  Vec3Array vertices;  // array of point3D
  FaceArray faces;     // array of arrays of 3 uint32
  Color color;         // color
} Model;

typedef struct {
  Vec3 pos;      // position is space
  Rotation rot;  // rotation in space
  Model* model;  // model to represent object
} Object;

// -#- globalVariables
// TODO remove
Model model1_global;
// vertices
Vec3Array vertices1_global;
// faces
FaceArray faces1_global;
// color
Color color1 = {255, 0, 0};

// real model
Model model2_global;

Object object2_global;

Model model3_global;

Object object3_global;

// Model model3_global;

InputKeys keys_global;

Camera camera_global;

InputMouse mouse_global = {FALSE, FALSE,        FALSE,        FALSE,
                           FALSE, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}};

Matrix3x3 initMatrix3x3(float _11, float _12, float _13, float _21, float _22,
                        float _23, float _31, float _32, float _33) {
  //
  Matrix3x3 r = {
      .v1 = {_11, _21, _31}, .v2 = {_12, _22, _32}, .v3 = {_13, _23, _33}};
  return r;
}

Uint maxUint(Uint a, Uint b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

Vec3 scaleV3(const float scalar, const Vec3 vec) {
  Vec3 result = {scalar * vec.x, scalar * vec.y, scalar * vec.z};
  return result;
}

Vec3 addV3(const Vec3 vec1, const Vec3 vec2) {
  Vec3 result = {vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z};
  return result;
}

float dotV3(const Vec3 v1, const Vec3 v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float atV3(const Vec3 vec, const Uint row) {
  switch (row) {
    case 0:
      return vec.x;
    case 1:
      return vec.y;
    case 2:
      return vec.z;
  }
  return 0.0f;
}

Vec2 scaleV2(const float scalar, const Vec2 vec) {
  Vec2 result = {scalar * vec.x, scalar * vec.y};
  return result;
}

Vec2 addV2(const Vec2 vec1, const Vec2 vec2) {
  Vec2 result = {vec1.x + vec2.x, vec1.y + vec2.y};
  return result;
}

float dotV2(const Vec2 v1, const Vec2 v2) { return v1.x * v2.x + v1.y * v2.y; }

float atV2(const Vec2 vec, const Uint row) {
  switch (row) {
    case 0:
      return vec.x;
    case 1:
      return vec.y;
  }
  return 0.0f;
}

float atM3x3(const Matrix3x3 matrix, Uint col, Uint row) {
  if (col < 3 && row < 3) {
    switch (col) {
      case 0:
        return atV3(matrix.v1, row);
      case 1:
        return atV3(matrix.v2, row);
      case 2:
        return atV3(matrix.v3, row);
    }
  }
  return 0.0f;
}

Vec3 multM3x3V3(const Matrix3x3 matrix, const Vec3 vec) {
  // A * v
  Vec3 resultVec;
  resultVec.x = matrix.v1.x * vec.x + matrix.v2.x * vec.y + matrix.v3.x * vec.z;
  resultVec.y = matrix.v1.y * vec.x + matrix.v2.y * vec.y + matrix.v3.y * vec.z;
  resultVec.z = matrix.v1.z * vec.x + matrix.v2.z * vec.y + matrix.v3.z * vec.z;
  return resultVec;
}

Matrix3x3 multM3x3M3x3(const Matrix3x3 m1, const Matrix3x3 m2) {
  Matrix3x3 r;

  // Column 0
  r.v1.x = m1.v1.x * m2.v1.x + m1.v2.x * m2.v1.y + m1.v3.x * m2.v1.z;  // x  # #
  r.v1.y = m1.v1.y * m2.v1.x + m1.v2.y * m2.v1.y + m1.v3.y * m2.v1.z;  // y  # #
  r.v1.z = m1.v1.z * m2.v1.x + m1.v2.z * m2.v1.y + m1.v3.z * m2.v1.z;  // z  # #

  // Column 1
  r.v2.x = m1.v1.x * m2.v2.x + m1.v2.x * m2.v2.y + m1.v3.x * m2.v2.z;  // #  x #
  r.v2.y = m1.v1.y * m2.v2.x + m1.v2.y * m2.v2.y + m1.v3.y * m2.v2.z;  // #  y #
  r.v2.z = m1.v1.z * m2.v2.x + m1.v2.z * m2.v2.y + m1.v3.z * m2.v2.z;  // #  z #

  // Column 2
  r.v3.x = m1.v1.x * m2.v3.x + m1.v2.x * m2.v3.y + m1.v3.x * m2.v3.z;  // #  # x
  r.v3.y = m1.v1.y * m2.v3.x + m1.v2.y * m2.v3.y + m1.v3.y * m2.v3.z;  // #  # y
  r.v3.z = m1.v1.z * m2.v3.x + m1.v2.z * m2.v3.y + m1.v3.z * m2.v3.z;  // #  # z

  return r;
}

void printVec3(const Vec3 v) {
  for (int i = 0; i < 3; i++) {
    printf("%f ", atV3(v, i));
    printf("\n");
  }
}

void printMatrix(const Matrix3x3 m) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      printf("%f ", atM3x3(m, j, i));
    }
    printf("\n");
  }
}

// radians
//
// around y-axis
// | cos x __|_ -sin x |
// | sin x __|__ cos x |
//
// pitch
// | cos x _ | _-sin x | _ _0 |
// | sin x _ | _ cos x | _ _0 |
// | _ _ _ 0 | _ _ _ 0 | _ _1 |
//
// roll
// | _ _ _ 1 | _ _ _ 0 | _ _ _ 0 |
// | _ _ _ 0 | cos x _ | _-sin x |
// | _ _ _ 0 | sin x _ | _ cos x |
// yaw
// | cos x _ | _ _ _ 0 | -sin x_ |
// | _ _ _ 0 | _ _ _ 1 | _ _ _ 0 |
// | sin x _ | _ _ _ 0 | _ cos x |
//

Matrix3x3 generateRotM3x3yaw(const float angle) {
  Matrix3x3 m = initMatrix3x3(cosf(angle), 0, -sinf(angle), 0, 1, 0,
                              sinf(angle), 0, cosf(angle));
  return m;
}

Matrix3x3 generateRotM3x3pitch(const float angle) {
  Matrix3x3 m = initMatrix3x3(1, 0, 0, 0, cosf(angle), -sinf(angle), 0,
                              sinf(angle), cosf(angle));
  return m;
}

Matrix3x3 generateRotM3x3roll(const float angle) {
  Matrix3x3 m = initMatrix3x3(cosf(angle), -sinf(angle), 0, sinf(angle),
                              cosf(angle), 0, 0, 0, 1);
  return m;
}

Matrix3x3 generateRotM3x3all(const float pitch, const float yaw,
                             const float roll) {
  return multM3x3M3x3(
      multM3x3M3x3(generateRotM3x3roll(roll), generateRotM3x3yaw(yaw)),
      generateRotM3x3pitch(pitch));
}
Matrix3x3 generateRotM3x3fromRotation(const Rotation rot) {
  return generateRotM3x3all(rot.pitch, rot.yaw, rot.roll);
}

float _triangleSign(Vec2 p1, Vec2 p2, Vec2 p3) {
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

Bool pntIsInTri2(Tri2 tri, Vec2 pt) {
  float d1, d2, d3;
  Bool has_neg, has_pos;
  d1 = _triangleSign(pt, tri[0], tri[1]);
  d2 = _triangleSign(pt, tri[1], tri[2]);
  d3 = _triangleSign(pt, tri[2], tri[0]);
  has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
  return !(has_neg && has_pos);
}

float limitRotationPitch(const float pitch) {
  static const float absLimit = 0.5f * M_PI;
  if (pitch < -absLimit) {
    return -absLimit;
  } else if (pitch > absLimit) {
    return absLimit;
  }
  return pitch;
}

float limitRotationYaw(const float yaw) {
  static const float PI2 = 2 * M_PI;
  if (abs(yaw) < PI2) {
    return yaw;
  } else if (yaw < 0) {
    return limitRotationYaw(yaw + PI2);
  }
  return limitRotationYaw(yaw - PI2);
}

void updateCameraRotMat(Camera* cam) {
  // used to rotate points in perspective.
  // Are therefore rotating the opposite direction

  // TEMP ----------------------- IGNORING ROLL
  // cam->mYawInv = generateRotM3x3yaw(-cam->rot.yaw);
  cam->mYawInv = generateRotM3x3yaw(-cam->rot.yaw);
  cam->mPitchInv = generateRotM3x3pitch(-cam->rot.pitch);
  cam->mRollInv = initMatrix3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

void updateCameraDirValue(Camera* cam) {
  Vec3 buf = {0, 0, 1};
  Matrix3x3 myaw = generateRotM3x3yaw(cam->rot.yaw);
  Matrix3x3 mpitch = generateRotM3x3pitch(cam->rot.pitch);
  // pitch and then yaw to get classical rotation
  Matrix3x3 rotMatrix = multM3x3M3x3(myaw, mpitch);
  // rotMatrix = multM3x3M3x3(rotMatrix, cam->mroll);

  cam->dirFront = multM3x3V3(rotMatrix, buf);
  buf.x = 1;
  buf.y = 0;
  buf.z = 0;
  cam->dirRight = multM3x3V3(rotMatrix, buf);
  buf.x = 0;
  buf.y = 1;
  buf.z = 0;
  cam->dirAbove = multM3x3V3(rotMatrix, buf);
}

void updateCameraVals(Camera* cam) {
  updateCameraRotMat(&camera_global);
  updateCameraDirValue(&camera_global);
}

void printModel(const Model model) {
  printf("%d\n", model.vertices.length);
  printf("%d\n", model.faces.length);
  printf("AFTER: model->vertices.array[2].x = %f\n", model.vertices.array[2].x);
  printf("AFTER: faces.x = %d\n", model.faces.array[0].array[0]);
  printf("Coordinates: ");
  for (int i = 0; i < model.vertices.length; i++) {
    printf("(%f, ", model.vertices.array[i].x);
    printf("%f, ", model.vertices.array[i].y);
    printf("%f), ", model.vertices.array[i].z);  //
  }
  printf("\n\n");
  printf("Faces(after): ");
  for (int i = 0; i < model.faces.length; i++) {
    printf("(");
    for (int j = 0; j < model.faces.array[i].length; j++) {
      printf("%d, ", model.faces.array[i].array[j]);
    }
    printf("), ");
  }
  printf("\n");
}

void destroyModel(Model* model) {
  free(model->vertices.array);
  free(model->faces.array);
  free(model);
}

// does not check for EOF
int fCheckNextChar(FILE* file) {
  int c = fgetc(file);
  ungetc(c, file);
  return c;
}

FResult fSkipCharacter(FILE* file) {
  if (fgetc(file) == EOF) return F_EOF;
  return F_SUCCESS;
}

FResult fSkipRestOfLine(FILE* file) {
  fscanf(file, "%*[^\n]");
  if (fgetc(file) == EOF) {  // if eof
    return F_EOF;
  }
  return F_SUCCESS;
  /*
  char buf[F_BUFFER_SIZE] = "";  // string of "\0\0\0\0\0..." with F_BUFFER_SIZE
  characters buf[99] = '0';       // set last byte to '0' not '\0'
  // this is done to check if fgets has written up to end of array

  while (TRUE) {
    fgets(buf, F_BUFFER_SIZE, file);
    if (buf[98] == '\n' || buf[99] != '\0') {
      // if fgets has not read up to the maximum.
      // The last byte must then not be a null-terminator character,
      // which is standard formatting for fgets
      // if second last byte is '\n' then fgets has reached
      // the end of the line
      break;
    }
    buf[99] = '0';  // set last byte to '0' not '\0'
  }
  */
}

FResult fSkipWhiteSpace(FILE* file) {
  // Leaves stream position at start of next WORD
  fscanf(file, "%*[ ]");         // skip empty space
  int c = fCheckNextChar(file);  // gets next char
  if (c == EOF) return F_EOF;
  if ((char)c == '\n') {  // next char is \n
    return F_EOL;         // return status
  }
  return F_SUCCESS;
}

FResult fSkipWord(FILE* file) {
  // skips until start of next WORD
  fscanf(file,
         "%*[^ \t\n]");  // moves position in stream forwards until next ' '
  return fSkipWhiteSpace(file);
}

FResult _fGetWord(FILE* file, char* buf) {
  FResult fRes = fSkipWhiteSpace(file);
  if (fRes == F_EOF) return F_EOF;
  if (fRes == F_EOL) return F_EOL;

  int result =
      fscanf(file, "%100s", buf);  // amount of characters assigned or EOF
  int c = fCheckNextChar(file);
  if (c == EOF) return F_EOF;
  if ((char)c != ' ' && (char)c != '\n') {  // checks next character in stream
    return F_OVERFLOW;
  }
  return F_SUCCESS;
}

FResult _fGetFloat(FILE* file, float* buf) {
  FResult fRes = fSkipWhiteSpace(file);
  if (fRes == F_EOL) return F_EOL;
  if (fRes == F_EOF) return F_EOF;

  long pos = ftell(file);                // remember current position
  int result = fscanf(file, "%f", buf);  // amount of characters assigned or EOF
  if (result == EOF) return F_EOF;

  // --- error checking
  if (result == 0) {  // no characters assigned
    fseek(file, pos, SEEK_SET);
    return F_WRONGTYPE;
  }

  return F_SUCCESS;
}

FResult _fGetInt(FILE* file, int* buf) {
  FResult fRes = fSkipWhiteSpace(file);
  if (fRes == F_EOL) return F_EOL;
  if (fRes == F_EOF) return F_EOF;

  long pos = ftell(file);                // remember current position
  int result = fscanf(file, "%d", buf);  // amount of characters assigned or EOF
  if (result == EOF) return F_EOF;

  // --- error checking
  if (result == 0) {  // no characters assigned
    fseek(file, pos, SEEK_SET);
    return F_WRONGTYPE;
  }

  return F_SUCCESS;
}

void printFResult(const FResult fRes) {
  switch (fRes) {
    case F_OVERFLOW:
      printf("Too large word\n");
      break;
    case F_EOF:
      printf("Reached end of file\n");
      break;
    case F_FAILURE:
      printf("Failed\n");
      break;
    case F_EOL:
      printf("Reached end of line\n");
      break;
    case F_WRONGTYPE:
      printf("WRONGTYPE\n");
      break;
  }
}

FResult fGetWord(FILE* file, char* sBuf) {
  FResult result = _fGetWord(file, sBuf);
  // printFResult(result);
  return result;
}

FResult fGetFloat(FILE* file, float* fBuf) {
  FResult result = _fGetFloat(file, fBuf);
  // printFResult(result);
  return result;
}

FResult fGetInt(FILE* file, int* nBuf) {
  FResult result = _fGetInt(file, nBuf);
  // PrintFResult(result);
  return result;
}

FResult fGetVertexCoord(FILE* file, Vec3* vertex) {
  fGetFloat(file, &vertex->x);
  fGetFloat(file, &vertex->y);
  fGetFloat(file, &vertex->z);
}

FResult fGetObjFaceValue(FILE* file, Face* face) {
  // finds face-indices written in obj format
  // "<vertex-index>/<texture-index>/<normal-index>"
  int nBuf = 0;
  int index = 0;
  FResult fRes = F_SUCCESS;
  Uint* array = (Uint*)malloc(sizeof(Uint) * MAX_FACES_PER_MODEL);
  // Uint array[MAX_FACES_PER_MODEL] = {};

  while (fRes == F_SUCCESS && index < MAX_VERTICES_PER_FACE) {
    fRes = fGetInt(file, &nBuf);
    array[index] =
        (Uint)nBuf - 1;  // subtract 1 because .obj files use indices > 0
    if ((char)fCheckNextChar(file) == '/') {
      fSkipWord(file);
    }
    index++;
  }
  face->array = array;
  face->length = index - 1;
}

//-#- .obj file loading
Bool loadObjFile(const char* path, Model* model, const float scale,
                 const Rotation rot) {
  printf("Loading object: %s\n", path);
  // File pointer to store the
  // value returned by fopen
  FILE* file;

  // Opening the file in read mode
  file = fopen(path, "r");

  // checking if the file
  // opened successfully
  if (file == NULL) {
    printf("The file was not opened.\n");
    return TRUE;
  }

  Matrix3x3 rotMatrix = generateRotM3x3all(rot.pitch, rot.yaw, rot.roll);

  printf("Allocating temporary memory\n");
  char sBuf[F_BUFFER_SIZE];
  // vertices
  Vec3Array vertices;
  Vec3* verticesArray = (Vec3*)malloc(sizeof(Vec3) * MAX_VERTICES_PER_MODEL);
  // Vec3 verticesArray[MAX_VERTICES_PER_MODEL] = {};  // may be scoped
  Uint vIndex = 0;

  // faces
  FaceArray faces;
  Face* facesArray = (Face*)malloc(sizeof(Face) * MAX_FACES_PER_MODEL);
  // Face facesArray[MAX_FACES_PER_MODEL] = {};
  Uint fIndex = 0;
  FResult fRes = fGetWord(file, sBuf);
  printf("Parsing file\n");
  while (fRes != F_EOF) {
    // buf will contain the first non-space characters leading
    // up to a space-character. Maximum F_BUFFER_SIZE characters.
    // If the maximum number of characters is reached
    if (fRes == F_EOL) {  // next line
      printf("eol\n");
      fSkipRestOfLine(file);
      fRes = fGetWord(file, sBuf);
      continue;
    }

    if (strcmp(sBuf, "v") == 0) {  // first word is "v"
      if (vIndex < MAX_VERTICES_PER_MODEL) {
        fGetVertexCoord(file, &verticesArray[vIndex]);
        verticesArray[vIndex] = scaleV3(scale, verticesArray[vIndex]);
        verticesArray[vIndex] = multM3x3V3(rotMatrix, verticesArray[vIndex]);
        vIndex++;
      }
    } else if (strcmp(sBuf, "f") == 0) {  // first word is "f"
      if (fIndex < MAX_FACES_PER_MODEL) {
        fGetObjFaceValue(file, &facesArray[fIndex]);
        fIndex++;
      }
    }
    fSkipRestOfLine(file);
    fRes = fGetWord(file, sBuf);
  }
  printf("Finished parsing file\n");
  vertices.length = vIndex;
  faces.length = fIndex;

  // optimize size of heap-allocated arrays
  size_t _size = vertices.length * sizeof(Vec3);
  void* pBuf = (void*)realloc((void*)verticesArray, _size);
  if (pBuf != NULL) {
    // if a new pointer is created
    // in case of inability to store
    // in same location
    verticesArray = (Vec3*)pBuf;
  }
  _size = faces.length * sizeof(Face);
  pBuf = (void*)realloc((void*)facesArray, _size);
  if (pBuf != NULL) {
    facesArray = (Face*)pBuf;
  }

  vertices.array = verticesArray;
  faces.array = facesArray;
  model->faces = faces;
  model->vertices = vertices;

  // temporary
  Color color = {255, 0, 0};
  model->color = color;
  fclose(file);
  return TRUE;
}

//-#- functions
Vec3 model_getVertex(const Model* model, Uint faceIndex, Uint vertexIndex) {
  if (faceIndex >= model->faces.length) {
    printf(" faceIndex >= model.faces.length \n");
    Vec3 nullPoint = {0, 0, 0};
    return nullPoint;
  }
  if (vertexIndex >= model->faces.length) {
    printf(" vertexIndex >= model.faces.length \n");
    Vec3 nullPoint = {0, 0, 0};
    return nullPoint;
  }
  const Uint indexVertices = model->faces.array[faceIndex].array[vertexIndex];

  if (indexVertices >= model->vertices.length) {
    printf(" (indexVertices)%d >= (model.vertices.length)%d \n", indexVertices,
           model->vertices.length);
    Vec3 nullPoint = {0, 0, 0};
    return nullPoint;
  }
  return model->vertices.array[indexVertices];
}

void default_clearScreen(const SDL_Renderer* renderer) {
  /* Clear screen to black */
  SDL_SetRenderDrawColor(renderer, 0, 30, 0, 255);
  SDL_RenderClear(renderer);
}

void handleKeyboardDownInput(const SDL_Event* event) {
  //
  switch (event->key.keysym.sym) {
    // vertical
    case SDLK_w:
      keys_global.camKeys.moveCamKeys.up = TRUE;
      break;
    case SDLK_s:
      keys_global.camKeys.moveCamKeys.down = TRUE;
      break;
    // horizontal
    case SDLK_a:
      keys_global.camKeys.moveCamKeys.left = TRUE;
      break;
    case SDLK_d:
      keys_global.camKeys.moveCamKeys.right = TRUE;
      break;
    case SDLK_UP:
      // keys_global.camKeys.up = TRUE;
      break;
    case SDLK_DOWN:
      // keys_global.camKeys.down = TRUE;
      break;
    // horizontal
    case SDLK_LEFT:
      // keys_global.camKeys.left = TRUE;
      break;
    case SDLK_RIGHT:
      // keys_global.camKeys.right = TRUE;
      break;
  }
}

void handleKeyboardUpInput(const SDL_Event* event) {
  //
  switch (event->key.keysym.sym) {
    // vertical
    case SDLK_w:
      keys_global.camKeys.moveCamKeys.up = FALSE;
      break;
    case SDLK_s:
      keys_global.camKeys.moveCamKeys.down = FALSE;
      break;
    // horizontal
    case SDLK_a:
      keys_global.camKeys.moveCamKeys.left = FALSE;
      break;
    case SDLK_d:
      keys_global.camKeys.moveCamKeys.right = FALSE;
      break;
    case SDLK_UP:
      keys_global.camKeys.rotateCamKeys.up = FALSE;
      break;
    case SDLK_DOWN:
      keys_global.camKeys.rotateCamKeys.down = FALSE;
      break;
    // horizontal
    case SDLK_LEFT:
      keys_global.camKeys.rotateCamKeys.left = FALSE;
      break;
    case SDLK_RIGHT:
      keys_global.camKeys.rotateCamKeys.right = FALSE;
      break;
  }
}

void handleMouseMotion(const SDL_Event* event) {
  if (!mouse_global.mb3) {
    static int _x1, _y1;
    SDL_GetMouseState(&_x1, &_y1);
    mouse_global.pos.x = (float)_x1;
    mouse_global.pos.y = (float)_y1;
  } else {
    mouse_global.pos = mouse_global.prevPos;
  }

  mouse_global.deltaPos.x = (float)event->motion.xrel;
  mouse_global.deltaPos.y = (float)event->motion.yrel;
}

void handleMouseInputDown(const SDL_Event* event) {
  // TODO
  if (event->button.button == SDL_BUTTON_RIGHT) {
    mouse_global.mb3 = TRUE;
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    mouse_global.prevPos = mouse_global.pos;
  }
}

void handleMouseInputUp(const SDL_Event* event, SDL_Window* window) {
  // TODO
  if (event->button.button == SDL_BUTTON_RIGHT) {
    mouse_global.mb3 = FALSE;
    SDL_ShowCursor(SDL_ENABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_WarpMouseInWindow(window, mouse_global.prevPos.x,
                          mouse_global.prevPos.y);
  }
}

GeneralGameData handleSDLEvent(GeneralGameData genData,
                               const SDL_Event* event) {
  if (event->type == SDL_QUIT) {
    genData.gameIsRunning = FALSE;
  }
  switch (event->type) {
    case SDL_QUIT:
      genData.gameIsRunning = FALSE;
      break;
    case SDL_KEYDOWN:
      handleKeyboardDownInput(event);
      break;
    case SDL_KEYUP:
      handleKeyboardUpInput(event);
      break;
    case SDL_MOUSEMOTION:
      handleMouseMotion(event);
      break;
    case SDL_MOUSEBUTTONDOWN:
      handleMouseInputDown(event);
      break;
    case SDL_MOUSEBUTTONUP:
      handleMouseInputUp(event, genData.window);
      break;
  }

  return genData;
}

Vec2 transform_spaceToScreen(const Vec3 point, const Camera cam) {
  // projection
  Vec3 bufPnt;
  bufPnt.x = point.x - cam.pos.x;
  bufPnt.y = point.y - cam.pos.y;
  bufPnt.z = point.z - cam.pos.z;

  // Matrix3x3 myaw = generateRotM3x3yaw(cam.rot.yaw);
  // Matrix3x3 mpitch = generateRotM3x3pitch(cam.rot.pitch);
  // Matrix3x3 mroll = generateRotM3x3roll(cam.rot.roll);
  // handling yaw, followed by pitch because these are
  // the inverse transformations.

  Matrix3x3 mRot = multM3x3M3x3(cam.mPitchInv, cam.mYawInv);  // ignoring roll

  bufPnt = multM3x3V3(mRot, bufPnt);
  // bufPnt = multM3x3V3(cam.myaw, bufPnt);
  // bufPnt = multM3x3V3(cam.mpitch, bufPnt);
  // bufPnt = multM3x3V3(cam.mroll, bufPnt);

  Vec2 transformedPoint;
  if (bufPnt.z > 0) {
    transformedPoint.x = bufPnt.x / bufPnt.z;
    transformedPoint.y = bufPnt.y / bufPnt.z;
  } else {
    transformedPoint.x = -9999;
    transformedPoint.y = -9999;
  }

  // ### to CLASSICAL screen coords
  // window___(SCR_WDTH)
  // (0,0)            |
  // |                |
  // |                |
  // |                |
  // |                |
  // |                |
  // (SCR_HEGT)--------
  transformedPoint.x += 1;
  transformedPoint.y += 1;
  // treating view cone as having a square base
  static const Uint scaled_CONE_BASE_SIZE = CONE_BASE_SIZE * 0.5;
  // TODO calculate from FOV value
  transformedPoint.x *= scaled_CONE_BASE_SIZE;
  transformedPoint.y *= scaled_CONE_BASE_SIZE;
  transformedPoint.y *= -1;
  transformedPoint.y += SCREEN_HEIGHT;

  return transformedPoint;
}

Bool pointIsInsideRect(const Rect viewRect, const Vec2 pnt) {
  if (pnt.x >= viewRect.x && pnt.x <= viewRect.x + viewRect.width) {
    if (pnt.y >= viewRect.y && pnt.y <= viewRect.y + viewRect.height) {
      return TRUE;
    }
  }
  return FALSE;
}

Bool drawVec2_screen(const SDL_Renderer* renderer, const Rect viewRect,
                     const Vec2 pnt, const Color color) {
  const int size = 5;
  const int offset = -(size >> 1);
  SDL_Rect rect = {0, 0, size, size};
  if (pointIsInsideRect(viewRect, pnt)) {
    rect.x = pnt.x + offset;
    rect.y = pnt.y + offset;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    // SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
    SDL_RenderFillRect(renderer, &rect);
    // printf( "x: %d\n", rect.x );
    // printf( "y: %d\n", rect.y );
    // printf( "w: %d\n", rect.w );
    // printf( "h: %d\n", rect.h );
    return TRUE;
  }
  return FALSE;
}

Bool drawLine2D_screen(const SDL_Renderer* renderer, const Rect viewRect,
                       const Vec2 pnt1, const Vec2 pnt2, const Color color) {
  Bool pnt1_isVisible = pointIsInsideRect(viewRect, pnt1);
  Bool pnt2_isVisible = pointIsInsideRect(viewRect, pnt2);
  if (pnt1_isVisible || pnt2_isVisible) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    // SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
    SDL_RenderDrawLine(renderer, pnt1.x, pnt1.y, pnt2.x, pnt2.y);
    return TRUE;
  }
  return FALSE;
}

// return FaceArray with pointer to memory on the heap
// NOTE: .array must be deallocated manually with free();
FaceArray splitFaceIntoTris(const Face face) {
  // assuming no improper polygons
  const Uint tris = face.length - 2;
  FaceArray out_faceBuf;
  out_faceBuf.length = tris;
  out_faceBuf.array = (Face*)malloc(sizeof(Face) * out_faceBuf.length);

  out_faceBuf.length = tris;
  Uint* _array;
  for (int i = 0; i < tris; i++) {
    for (int j = 0; j < 3; j++) {
      Uint index = i * 2 + j;
      if (index >= face.length) index -= face.length;
      // printf("i: %d\n", i);
      // printf("j: %d\n", j);
      // printf("index: %d\n", index);
      // printf("faceValue: %d\n", face.array[index]);
      _array = (Uint*)malloc(sizeof(Uint) * 3);
      _array[j] = face.array[index];
    }
    out_faceBuf.array[i].array = _array;
    // printf("---------\n");
  }
  return out_faceBuf;
}

Bool drawFace_screen(const SDL_Renderer* renderer, const Rect viewRect,
                     const Model* model, const unsigned faceIndex,
                     const Camera cam, const Vec3 faceAnchor,
                     const Rotation faceRot) {
  /*
  for (int vertexInFaceIndex = 0;
       vertexInFaceIndex < model.faces.array[faceIndex].length;
       vertexInFaceIndex++) {
    Vec2 projected_3DPoint = transform_spaceToScreen(
        model_getVertex(model, faceIndex, vertexInFaceIndex), camPosition,
        camRotation);
    drawVec2_screen(renderer, viewRect, projected_3DPoint, model.color);
  }
  // ---------------------
  Vec2 projected_3DPoint1 =
      transform_spaceToScreen(model_getVertex(model, faceIndex, 0), cam);
  Vec2 projected_3DPoint2 =
      transform_spaceToScreen(model_getVertex(model, faceIndex, 1), cam);
  Vec2 projected_3DPoint3 =
      transform_spaceToScreen(model_getVertex(model, faceIndex, 2), cam);
  drawLine2D_screen(renderer, viewRect, projected_3DPoint1,
  projected_3DPoint2, model.color); drawLine2D_screen(renderer, viewRect,
  projected_3DPoint2, projected_3DPoint3, model.color);
  drawLine2D_screen(renderer, viewRect, projected_3DPoint3,
  projected_3DPoint1, model.color);
  */

  // TODO CACHING OF ROTATION MATRICES

  Matrix3x3 rotMatrix = initMatrix3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
  // Matrix3x3 rotMatrix = generateRotM3x3fromRotation(faceRot);

  const int amountOfVertices = (int)model->faces.array[faceIndex].length;
  const int amountOfTriangles = amountOfVertices - 2;
  static SDL_Vertex verticesBuf[MAX_VERTICES_PER_FACE];

  for (int i = 0; i < amountOfTriangles; i++) {
    for (int j = 0; j < 3; j++) {
      Uint index = i * 2 + j;
      if (index >= amountOfVertices) index -= amountOfVertices;
      Vec3 v = model_getVertex(model, faceIndex, index);
      v = multM3x3V3(rotMatrix, v);                // rotate v
      v = addV3(faceAnchor, v);                    // translate v
      Vec2 v_p = transform_spaceToScreen(v, cam);  // projecting v
      verticesBuf[i * 3 + j].position.x = v_p.x;
      verticesBuf[i * 3 + j].position.y = v_p.y;
      // verticesBuf[i * 3 + j].color.r = 255 * i / amountOfVertices - 1;
      // verticesBuf[i * 3 + j].color.g = 255 * v_p.x / SCREEN_WIDTH;
      // verticesBuf[i * 3 + j].color.b = 255 / (i + 1);
      verticesBuf[i * 3 + j].color.r = 255;
      verticesBuf[i * 3 + j].color.g = 255 * faceIndex / model->faces.length;
      verticesBuf[i * 3 + j].color.b = 255 * faceIndex / model->faces.length;
      verticesBuf[i * 3 + j].color.a = 255;
    }
  }

  int resultRG = SDL_RenderGeometry(renderer, NULL, verticesBuf,
                                    3 * amountOfTriangles, NULL, 0);
  return TRUE;
}

void drawModel_screen(SDL_Renderer* renderer, Rect viewRect, const Model* model,
                      const Vec3 pos, const Rotation rot, const Camera cam) {
  for (int i = 0; i < model->faces.length; i++) {
    drawFace_screen(renderer, viewRect, model, i, cam, pos, rot);
  }
}

void drawObject_screen(SDL_Renderer* renderer, Rect viewRect,
                       const Object* object, const Camera cam) {
  drawModel_screen(renderer, viewRect, object->model, object->pos, object->rot,
                   cam);
}

// TEMP ----------------------------------------------------
Tri2 tri1 = {{200.0, 200.0}, {600.0, 200.0}, {200.0, 600.0}};
Vec2 p1 = {200.0, 200.0};
// TEMP ----------------------------------------------------

void draw(SDL_Renderer* renderer) {
  default_clearScreen(renderer);
  static const Rect exampleViewRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
  static const Color exampleColor = {0, 255, 0};
  static const Color exampleColor2 = {0, 0, 255};

  p1.x = mouse_global.pos.x;
  p1.y = mouse_global.pos.y;

  Bool _r0 = drawVec2_screen(renderer, exampleViewRect, p1, exampleColor);

  for (int _i = 0; _i < 3; _i++) {
    Bool _r1 =
        drawVec2_screen(renderer, exampleViewRect, tri1[_i], exampleColor2);
    Bool _r2 = drawLine2D_screen(renderer, exampleViewRect, tri1[_i], p1,
                                 exampleColor2);
  }

  if (pntIsInTri2(tri1, p1)) {
    Vec2 indicator = {300, 100};
    Bool _r4 =
        drawVec2_screen(renderer, exampleViewRect, indicator, exampleColor2);
  }

  // DRAW OBJECT
  drawObject_screen(renderer, exampleViewRect, &object2_global, camera_global);
  drawObject_screen(renderer, exampleViewRect, &object3_global, camera_global);

  // update visible frame
  SDL_RenderPresent(renderer);
}

void update() {
  static Vec3 vBuf;
  if (keys_global.camKeys.moveCamKeys.down &&
      !keys_global.camKeys.moveCamKeys.up) {
    vBuf = scaleV3(MOVE_SPEED, camera_global.dirFront);
    vBuf = scaleV3(-1.0, vBuf);
    camera_global.pos = addV3(camera_global.pos, vBuf);
  } else if (!keys_global.camKeys.moveCamKeys.down &&
             keys_global.camKeys.moveCamKeys.up) {
    vBuf = scaleV3(MOVE_SPEED, camera_global.dirFront);
    camera_global.pos = addV3(camera_global.pos, vBuf);
  }

  if (keys_global.camKeys.moveCamKeys.left &&
      !keys_global.camKeys.moveCamKeys.right) {
    vBuf = scaleV3(MOVE_SPEED, camera_global.dirRight);
    vBuf = scaleV3(-1.0, vBuf);
    camera_global.pos = addV3(camera_global.pos, vBuf);
  } else if (!keys_global.camKeys.moveCamKeys.left &&
             keys_global.camKeys.moveCamKeys.right) {
    vBuf = scaleV3(MOVE_SPEED, camera_global.dirRight);
    camera_global.pos = addV3(camera_global.pos, vBuf);
  }

  /*
  if (keys_global.camKeys.rotateCamKeys.down &&
      !keys_global.camKeys.rotateCamKeys.up) {
    camera_global.rot.pitch += CAM_ROT_SPEED * mouseScalar.y;
    camera_global.rot.pitch = limitRotationpitch(camera_global.rot.pitch);
    updateCameraVals(&camera_global);
  } else if (!keys_global.camKeys.rotateCamKeys.down &&
             keys_global.camKeys.rotateCamKeys.up) {
    camera_global.rot.pitch -= CAM_ROT_SPEED * mouseScalar.y;
    camera_global.rot.pitch = limitRotationpitch(camera_global.rot.pitch);
    updateCameraVals(&camera_global);
  }

  if (keys_global.camKeys.rotateCamKeys.left &&
      !keys_global.camKeys.rotateCamKeys.right) {
    camera_global.rot.yaw += CAM_ROT_SPEED * mouseScalar.x;
    camera_global.rot.yaw = limitRotationyaw(camera_global.rot.yaw);
    updateCameraVals(&camera_global);
  } else if (!keys_global.camKeys.rotateCamKeys.left &&
             keys_global.camKeys.rotateCamKeys.right) {
    camera_global.rot.yaw -= CAM_ROT_SPEED * mouseScalar.x;
    camera_global.rot.yaw = limitRotationyaw(camera_global.rot.yaw);
    updateCameraVals(&camera_global);
  }
  */

  if (mouse_global.mb3) {
    float _dx = mouse_global.deltaPos.x;
    float _dy = mouse_global.deltaPos.y;
    camera_global.rot.yaw += (-1 * _dx) * CAM_ROT_SPEED;
    camera_global.rot.pitch += (_dy)*CAM_ROT_SPEED;
    camera_global.rot.pitch = limitRotationPitch(camera_global.rot.pitch);
    camera_global.rot.yaw = limitRotationYaw(camera_global.rot.yaw);
    updateCameraVals(&camera_global);
  }

  // !!! RESET GLOBAL MOUSE VALUES
  mouse_global.deltaPos.x = 0.0f;
  mouse_global.deltaPos.y = 0.0f;
}

void initGlobals() {
  // global variables
  //
  //

  MoveKeys _moveKeys = {FALSE, FALSE, FALSE, FALSE};

  CamKeys _camKeys = {{FALSE, FALSE, FALSE, FALSE},
                      {FALSE, FALSE, FALSE, FALSE}};

  keys_global.moveKeys = _moveKeys;
  keys_global.camKeys = _camKeys;

  Vec3 _position = {8.5, 8.0, -20.0};

  Rotation _rotation = {0.0, 0.0, 0.0};

  camera_global.pos = _position;
  camera_global.rot = _rotation;

  updateCameraVals(&camera_global);

  vertices1_global.length = 8;
  // front
  // bottom
  Vec3 p1 = {0.2, -0.4, 1.0};
  // ----right
  Vec3 p2 = {0.6, -0.4, 1.0};
  // top
  Vec3 p3 = {0.2, 0.4, 1.0};
  // ----right
  Vec3 p4 = {0.6, 0.4, 1.0};

  // back
  // bottom
  Vec3 p5 = {0.2, -0.4, 1.4};
  // ----right
  Vec3 p6 = {0.6, -0.4, 1.4};
  // top
  Vec3 p7 = {0.2, 0.4, 1.4};
  // ----right
  Vec3 p8 = {0.6, 0.4, 1.4};
  Vec3 vArray[8] = {p1, p2, p3, p4, p5, p6, p7, p8};
  vertices1_global.array = vArray;

  faces1_global.length = 12;

  Face face1;
  Uint fArray1[3] = {0, 1, 2};
  face1.array = fArray1;
  face1.length = 3;

  Face face2;
  Uint fArray2[3] = {3, 2, 1};
  face2.array = fArray2;
  face2.length = 3;

  Face face3;
  Uint fArray3[3] = {0, 1, 4};
  face3.array = fArray3;
  face3.length = 3;

  Face face4;
  Uint fArray4[3] = {2, 4, 6};
  face4.array = fArray4;
  face4.length = 3;

  Face face5;
  Uint fArray5[3] = {1, 4, 5};
  face5.array = fArray5;
  face5.length = 3;

  Face face6;
  Uint fArray6[3] = {2, 3, 6};
  face6.array = fArray6;
  face6.length = 3;

  Face face7;
  Uint fArray7[3] = {6, 7, 3};
  face7.array = fArray7;
  face7.length = 3;

  Face face8;
  Uint fArray8[3] = {4, 5, 6};
  face8.array = fArray8;
  face8.length = 3;

  Face face9;
  Uint fArray9[3] = {6, 7, 5};
  face9.array = fArray9;
  face9.length = 3;

  Face face10;
  Uint fArray10[3] = {1, 5, 7};
  face10.array = fArray10;
  face10.length = 3;

  Face face11;
  Uint fArray11[3] = {3, 5, 7};
  face11.array = fArray11;
  face11.length = 3;

  Face face12;
  Uint fArray12[3] = {0, 2, 4};
  face12.array = fArray12;
  face12.length = 3;

  Face fsArray[12] = {face1, face2, face3, face4,  face5,  face6,
                      face7, face8, face9, face10, face11, face12};
  faces1_global.length = 12;
  faces1_global.array = fsArray;

  // updating model
  model1_global.faces = faces1_global;
  model1_global.vertices = vertices1_global;
  model1_global.color = color1;

  // OBJECT 2
  object2_global.model = &model2_global;
  object2_global.rot.pitch = 0.0f;
  object2_global.rot.yaw = 0.0f;
  object2_global.rot.roll = 0.0f;
  object2_global.pos.x = 0.0f;
  object2_global.pos.y = -8.0f;
  object2_global.pos.z = 0.0f;

  // OBJECT 3
  object3_global.model = &model3_global;
  object3_global.rot.pitch = 0.0f;
  object3_global.rot.yaw = 0.0f;
  object3_global.rot.roll = 0.0f;
  object3_global.pos.x = 0.0f;
  object3_global.pos.y = 0.0f;
  object3_global.pos.z = 0.0f;

  // real model
  // loadObjFile(MAIN_DIR_PATH "/bin/res/models/test.obj", &model2_global);
  // loadObjFile(MAIN_DIR_PATH "/bin/res/models/test2.obj", &model2_global,
  // 0.2f,
  //             exampleRotation);

  Rotation initialOrientation2 = {0.0f, M_PI, 0.0f};
  Rotation initialOrientation3 = {0.0f, M_PI, 0.0f};
  float initialScalar2 = 5.0f;
  float initialScalar3 = 0.25f;
  loadObjFile(MAIN_DIR_PATH
              "/bin/res/models/Madara Uchiha/obj/Madara_Uchiha.obj",
              &model2_global, initialScalar2, initialOrientation2);
  // loadObjFile(MAIN_DIR_PATH "/bin/res/models/test2.obj", &model2_global,
  //             initialScalar2, initialOrientation2);
  loadObjFile(MAIN_DIR_PATH "/bin/res/models/test.obj", &model3_global,
              initialScalar3, initialOrientation3);
  // printf("\nvertsLen: %d\n", model2_global.vertices.length);
  // printf("facesLen: %d\n", model2_global.faces.length);
  // printf("lFaceLen: %d\n",
  // model2_global.faces.array[model2_global.faces.length - 1].length);
  // printf("lvertLen: %f\n",
  // model2_global.vertices.array[model2_global.vertices.length - 1].z);
  // printModel(model2_global);
}

int main(int argc, char* argv[]) {
  // Face _f1;
  // _f1.length = 4;
  // Uint _array[4] = {1, 2, 3, 4};
  // _f1.array = _array;
  // FaceArray _faceBuf;
  // _faceBuf = splitFaceIntoTris(_f1);
  // printf("Result: %d\n", _faceBuf.length);
  // for (int i = 0; i < _faceBuf.length; i++) {
  //   for (int j = 0; j < 3; j++) {
  //     printf("%d, ", _faceBuf.array[i].array[j]);
  //   }
  //   printf("\n");
  // }
  // freeFaceArray(_faceBuf);

  initGlobals();

  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  GeneralGameData genData = {};
  genData.gameIsRunning = TRUE;

  Uint32 startTicks = 0;
  Uint32 endTicks = 0;

  float estimatedFPS = 0;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("SDL_Init Error: %s", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("3D rendering", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN);
  genData.window = window;

  if (!window) {
    SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // first clear
  default_clearScreen(renderer);

  // -#- main loop while running
  while (genData.gameIsRunning) {
    startTicks = SDL_GetTicks();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {  // poll until all events are handled!
      GeneralGameData result_genData = handleSDLEvent(genData, &event);
      genData = result_genData;
    }

    // update game state, draw the current frame
    update();
    draw(renderer);
    endTicks = SDL_GetTicks();
    Uint32 deltaTicks = endTicks - startTicks;

    // printf("FPS: %f\n", estimatedFPS);

    char fpsString[32];
    int result = snprintf(fpsString, sizeof(fpsString), "%f", estimatedFPS);
    SDL_SetWindowTitle(window, fpsString);
    if (deltaTicks < TICKS_PER_FRAME) {
      SDL_Delay(TICKS_PER_FRAME - deltaTicks);
      estimatedFPS = FPS;
    } else {
      estimatedFPS = 1000.0f / deltaTicks;
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

int main2(int argc, char* argv[]) { return 0; }
