#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//#include <GL/gl.h>
//#include <GL/glu.h>
#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <stack>
#include <queue>
#include <map>
#include <functional>
#include <queue>
#include <sstream>
#include <set>
#include <iomanip>
#include <string.h>
#include <limits.h>
#include <iterator>
#include <fstream>
#include <time.h>
using namespace std;


enum Rotation {CCW, CW, COLLINEAR};
enum uiParameters {REGENERATE};


typedef long long ll;
typedef vector<ll> vi;
typedef pair<ll, ll> ii;
typedef vector<ii> vii;
using namespace std;

float angle = 0.0f;
float speed = 0.0f;
float mouseX = 0.0f;
float mouseY = 0.0f;
float camRotZ = 0.0f;


struct Vertex {
    double x, y, z;
    Vertex() { x = 0.; y = 0; z = 0; }
    Vertex(double _x, double _y, double _z) { x = _x; y = _y, z = _z; }
};

struct rgb {
    double r, g, b;
    rgb() { r = 0.8; g = 0.8; b = 0.8; }
    rgb(double _r, double _g, double _b) { r = _r; g = _g; b = _b; }
};

struct Face {
    vector<size_t> vidx;
    rgb color;
    Face() {}
    Face(vector<size_t> v, rgb c) { vidx = v; color = c; }
};

struct Object {
    vector<Vertex> vertices;
    vector<Face> faces;
};

struct Point {
    int x, y, z;
    Point() { x = 0, y = 0, z = 0; }
    Point(int _x, int _y, int _z) { x = _x, y = _y, z = _z; }

    // Choose the smallest point along the y axis,
    // and use x as a tie-breaker.
    bool operator < (Point other) {
        if (this->y != other.y) {
            return this->y < other.y;
        }
        return this->x < other.x;
    }
};

struct Polygon {
    vector<Point> v;
};

vector<Point> points;
Object sphere;

vector<Point> hull;
Point point(5,0,5);
Point point2(2,0,2);
Point point3(3,0,4);
int aux = 0;
int currentPoint = 0;


void drawLines(){

    int longitudDePuntos = hull.size();
    glLineWidth(2.5);
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);

    // cout << longitudDePuntos << endl;

    for (int i = 0; i < longitudDePuntos-3; i++) {
        glVertex3f((float)hull[i].x, 0.0f, (float)hull[i].z);
        glVertex3f((float)hull[i+1].x, 0.0f, (float)hull[i+1].z);
    }

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f((float)(hull.end() - 3)->x, 0.0f, (float)(hull.end() - 3)->z);
    glVertex3f((float)(hull.end() - 2)->x, 0.0f, (float)(hull.end() - 2)->z);
    glVertex3f((float)(hull.end() - 2)->x, 0.0f, (float)(hull.end() - 2)->z);
    glVertex3f((float)(hull.end() - 1)->x, 0.0f, (float)(hull.end() - 1)->z);
    // doGrahamScanStep(int currentPoint);

    glEnd();
    glFlush();
}

void processVertex(string line, Object *object) {
    line = line.substr(2);
    string aux = "";
    double coords[3];
    int cur = 0;
    for(const auto& vert : line) {
        if (vert == ' ') {
            coords[cur++] = std::stod(aux);
            aux = "";
        }
        else
            aux += vert;
    }
    coords[cur++] = std::stod(aux);
    object->vertices.push_back(Vertex(coords[0], coords[1], coords[2]));
}

void processFace(string line, rgb c, Object *object) {
    line = line.substr(2);
    string aux = "";
    Face vface;
    for (const auto& character : line) {
        if (character == ' ') {
            string nxtFace = "";
            for (const auto& value: aux) {
                if (value == '/') {
                    break;
                }
                nxtFace += value;
            }
            vface.vidx.push_back(atoi(nxtFace.c_str()));
            aux = "";
        }
        else {
            aux += character;
        }
    }

    vface.vidx.push_back(atoi(aux.c_str()));
    object->faces.push_back(vface);
    object->faces.back().color = c;
}

rgb processColor(string line) {
    vector <double> arr;
    string aux = "";
    line = line.substr(8);
    for(const auto& digit : line) {
        if (digit == ' ') {
            arr.push_back(std::stod(aux));
            aux = "";
        }
        else
            aux += digit;
    }

    arr.push_back(std::stod(aux));
    return rgb(arr[0], arr[1], arr[2]);
}


string trim(string str) {
    int l = 0, r = str.size() - 1;
    while (l < r &&
           (str[l] == ' ' || str[l] == '\t'))
        l++;
    while (l < r &&
           (str[r] == ' ' || str[r] == '\t'))
        r--;

    return str.substr(l, r - l + 1);
}

void loadObjFiles() {
    string str;
    vector<string> files;
    files.push_back("sphere.obj");
    ifstream inFile;

    for (const auto& file: files) {
        string filePath = "/home/irvel/ConvexHullOpenGL/" + file;
        Object *object;
        object = &sphere;
        inFile.open(filePath.c_str());
        if (!inFile) {
            cout << "Unable to open file" << endl;
            return;
        }

        rgb c;
        while (getline(inFile, str)) {
            if (str.size() == 0)
                continue;
            str = trim(str);
            if (str.substr(0, 7) == "# Color") {
                c = processColor(str);
            }
            else if (str[0] == 'v' && str[1] == ' ') {
                processVertex(str, object);
            }
            else if (str[0] == 'f') {
                processFace(str, c, object);
            }
        }

        inFile.close();
    }
}

void generatePoints(int n, int limit) {
    srand(time(NULL));
    while (points.size() < n) {
        int x = rand() % (limit * 2 + 1);
        int z = rand() % (limit * 2 + 1);
        x -= limit;
        z -= limit;
        for (const auto& point: points) {
            if (x == point.x && z == point.z)
                continue;
        }
        points.push_back(Point(x, 0, z));
    }
}

// Generates obj file for convex hull
void generateOutputFile() {

    ofstream output;
    output.open("output.obj");

    for(int i = 1; i <= hull.size(); i++){
        output << "v " << hull[i].x << " 0.0 " << hull[i].z << endl;
        output << "v " << hull[i+1].x << " 0.0 " << hull[i+1].z << endl;
        output << "f " << i << " " << (i+1) << endl;
    }

    output.close();
}

Rotation determineRotation(Point a, Point b, Point c) {
    int area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (area > 0) {
        return CCW;
    }
    else if (area < 0) {
        return CW;
    }
    return COLLINEAR;
}

// We use the squared Euclidean to keep using integers
// and avoid computing expensive square roots.
int squaredEuclidean(Point a, Point b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return dx * dx + dy * dy;
}

void prepareGrahamScan() {
    auto minPointIter = std::min_element(points.begin(), points.end());
    // Swap point with the first one.
    std::iter_swap(points.begin(), minPointIter);
    Point pivot = points.front();
    // points.pop_back();

    // Custom function to compare by polar angle. It's optimized so that
    // we don't actually have to compute the angle itself.
    auto polar_comparison = [=](Point a, Point b) {
        auto rotation = determineRotation(pivot, a, b);
        if (rotation == COLLINEAR) {
            return squaredEuclidean(pivot, a) < squaredEuclidean(pivot, b);
        }
        return (rotation == CCW);
    };

    // Sort points w.r.t our selected pivot.
    std::sort(points.begin(), points.end(), polar_comparison);
    hull.push_back(points[0]);
    hull.push_back(points[1]);
    hull.push_back(points[2]);
    currentPoint = 3;
}

void doGrahamScanStep(int currentPoint) {
    Point top = hull.back();
    hull.pop_back();
    while (determineRotation(hull.back(), top, points[currentPoint]) != CCW) {
        top = hull.back();
        hull.pop_back();
    }
    hull.push_back(top);
    hull.push_back(points[currentPoint]);
}

void drawObj(Object * object) {
    for (const auto& face: object->faces) {
        if (face.vidx.size() == 3) {
            glBegin(GL_TRIANGLES);
                rgb c = face.color;
                glColor3f(c.r, c.g, c.b);
                for (const auto& vidx: face.vidx) {
                    Vertex v = object->vertices[vidx - 1];
                    glVertex3f(v.x, v.y, v.z);
                }
            glEnd();
        }
        else if (face.vidx.size() == 4) {
            glBegin(GL_QUADS);
                rgb c = face.color;
                glColor3f(c.r, c.g, c.b);
                for (const auto& vidx: face.vidx) {
                    Vertex v = object->vertices[vidx - 1];
                    glVertex3f(v.x, v.y, v.z);
                }

            glEnd();
        }
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    // Induce a parallax effect correlated with mouse movement.
    gluLookAt(  0.0f, 100.0f + mouseX, 0.1f,
                0.0f + camRotZ, 0.0f ,  0.0f + mouseY,
                0.0f, 1.0f,  0.0f);

    for (const auto& point: points) {
        glPushMatrix();
        //cout << points[i].x << " " << points[i].y << endl;
        glTranslatef(point.x, point.y, point.z);
        drawObj(&sphere);
        glPopMatrix();
    }

    angle += 1.0;

    if(aux > 0){
        drawLines();
    }

    glutSwapBuffers();

}

void reshape(int w, int h) {
   if (h == 0)
		h = 1;
	float ratio =  w * 1.0 / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f, ratio, 0.1f, 150.0f);
	glMatrixMode(GL_MODELVIEW);
}

void populate() {
    hull = {};
    aux += 3;
    if(aux > points.size()){
        aux = points.size();
    }
    for(int i=0; i < aux; i++) {
        hull.push_back( points[i]);
    }
    display();
}

void keyboard (unsigned char key, int x, int y) {
    switch (key) {
        case 'q':
            exit(0);
        // Press f key to go faster.
        case 'f':
            speed += 2.0f;
            break;
            // Press s key to go slower.
        case 's':
            speed -= 2.0f;
            break;
        case 'p':
            if(hull.size() < points.size()){
                doGrahamScanStep(currentPoint);
                currentPoint += 1;
                aux += 3;
                display();
                // populate();
            }
            break;
        default:
            break;
    }
}

void processMouseMove(int x, int y) {
    // We limit the magnitude of the reported motion to prevent loosing the scene in view.
    mouseX = x * .008;
    mouseY = y * .005;
    glutPostRedisplay();
}

void processMainMenu(int option) {
    switch (option) {
        case CW:
            camRotZ += 1.0f;
            break;
        case CCW:
            camRotZ -= 1.0f;
            break;
    }
}



// hacer restart del convex hull
void processParamsMenu(int option) {
    switch (option) {
        case REGENERATE:
            points.clear();
            generatePoints(50, 30);

            break;
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Convex Hull Visualization");
    glEnable(GL_DEPTH_TEST);
    loadObjFiles();
    generatePoints(50, 30);
    prepareGrahamScan();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(processMouseMove);
    glutIdleFunc(display);

    // Create UI Menus.
    int paramsMenu = glutCreateMenu(processParamsMenu);
    glutAddMenuEntry("Regenerate Points", REGENERATE);

    int mainMenu = glutCreateMenu(processMainMenu);
    // Add options for controlling camera rotation.
    glutAddMenuEntry("Rotate View Clockwise", CW);
    glutAddMenuEntry("Rotate View Counter-Clockwise", CCW);
    glutAddSubMenu("Adjust Parameters", paramsMenu);

    // Display UI menus with mouse right click.
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutMainLoop();
    return 0;
}
