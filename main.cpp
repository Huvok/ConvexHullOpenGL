#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <GL/gl.h>
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

#define FOR(i, a, b) for(ll i=ll(a); i<ll(b); i++)
#define ROF(i, a, b) for(ll i=ll(a); i>=ll(b); i--)
#define pb push_back
#define mp make_pair
#define lld I64d
#define all(a) (a).begin(), (a).end()
#define sync ios_base::sync_with_stdio(false); cin.tie(NULL); cout.tie(NULL)
#define PI 3.1415926535897932384626433832795
#define mem(x, val) memset((x), (val), sizeof(x))
#define sz(x) (ll)(x).size()
#define endl '\n'

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
const int windowHeight = 800;
const int windowWidth = 800;

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
    vi vidx;
    rgb color;
    Face() {}
    Face(vi v, rgb c) { vidx = v; color = c; }
};

struct Object {
    vector<Vertex> vertices;
    vector<Face> faces;
};

struct Point {
    int x, y, z;
    Point() { x = 0, y = 0, z = 0; }
    Point(int _x, int _y, int _z) { x = _x, y = _y, z = _z; }

    // Choose the smallest point along the z axis,
    // and use x as a tie-breaker.
    bool operator < (Point other) {
        if (this->z != other.z) {
            return this->z < other.z;
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
int currentPoint = 0;

Object ch;

void drawLines(){
    glLineWidth(2.5);
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int i = 1; i < hull.size() - 1; i++) {
        // Draw the current progress of the hull.
        glVertex3f((float)hull[i].x, 0.0f, (float)hull[i].z);
        glVertex3f((float)hull[i-1].x, 0.0f, (float)hull[i-1].z);
    }
    if (currentPoint < points.size()) {
        // Display the tentative path in red. This might be removed by the algorithm
        // if it's a concave.
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f((float)(hull.end() - 3)->x, 0.0f, (float)(hull.end() - 3)->z);
        glVertex3f((float)(hull.end() - 2)->x, 0.0f, (float)(hull.end() - 2)->z);
        glVertex3f((float)(hull.end() - 2)->x, 0.0f, (float)(hull.end() - 2)->z);
        glVertex3f((float)(hull.end() - 1)->x, 0.0f, (float)(hull.end() - 1)->z);
    }
    else {
        // We skipped drawing the last edge because during the normal execution
        // of the algorithm the edge might be wrong. Once we finish through all
        // steps of the scan, we're now able to draw it with confidence.
        glVertex3f((float)(hull.end() - 2)->x, 0.0f, (float)(hull.end() - 2)->z);
        glVertex3f((float)(hull.end() - 1)->x, 0.0f, (float)(hull.end() - 1)->z);

        // Close gap of the drawn hull from the beginning of the hull path to
        // last drawn point.
        glVertex3f((float)hull.front().x, 0.0f, (float)hull.front().z);
        glVertex3f((float)hull.back().x, 0.0f, (float)hull.back().z);
    }

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
            coords[cur++] = atof(aux.c_str());
            aux = "";
        }
        else
            aux += vert;
    }
    coords[cur++] = atof(aux.c_str());
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
            arr.push_back(atof(aux.c_str()));
            aux = "";
        }
        else
            aux += digit;
    }

    arr.push_back(atof(aux.c_str()));
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
        string filePath = "C:\\Users\\hgarc\\GitHub\\ConvexHullOpenGL\\" + file;
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
        FOR(i, 0, sz(points)) {
            if (x == points[i].x && z == points[i].z)
                continue;
        }
        points.push_back(Point(x, 0, z));
    }
}

Rotation determineRotation(Point a, Point b, Point c) {
    int area = (b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x);
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
    int dz = a.z - b.z;
    return dx * dx + dz * dz;
}

void debugPoints(string msg="") {
    if (msg != "") {
        cout << msg << ":  ";
    }
    cout << "[";
    FOR(i, 0, sz(points)) {
        cout << "(" << points[i].x << ", " << points[i].z << ") ";
    }
    cout << "]\n";
}

void debugHull(string msg="") {
    if (msg != "") {
        cout << msg << ":  ";
    }
    cout << "[";
    FOR(i, 0, sz(hull)) {
        cout << "(" << points[i].x << ", " << points[i].z << ") ";
    }
    cout << "]\n";
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
    std::sort(points.begin() + 1, points.end(), polar_comparison);

    hull.push_back(points[0]);
    hull.push_back(points[1]);
    hull.push_back(points[2]);
    currentPoint = 3;
}

void pushFace(Object &object, int n, rgb c) {
    vi v;
    FOR(i, 0, n)
        v.pb(sz(object.vertices) - n + i + 1);
    object.faces.pb(Face(v, c));
}

void doGrahamScanStep() {
    Point top = hull.back();
    hull.pop_back();
    while (determineRotation(hull.back(), top, points[currentPoint]) != CCW) {
        top = hull.back();
        hull.pop_back();
    }
    hull.push_back(top);
    hull.push_back(points[currentPoint]);
    currentPoint++;

    if (currentPoint >= sz(points)) {
        ll n = hull.size();
        for(int i = 0; i < n - 1; i++){
            ch.vertices.pb(Vertex(hull[i].x, 0.0, hull[i].z));
            ch.vertices.pb(Vertex(hull[i + 1].x, 0.0, hull[i + 1].z));
            pushFace(ch, 2, rgb(0.8, 0.8, 0.8));
        }

        ch.vertices.pb(Vertex(hull[n - 1].x, 0.0, hull[n - 1].z));
        ch.vertices.pb(Vertex(hull[0].x, 0.0, hull[0].z));
        pushFace(ch, 2, rgb(0.8, 0.8, 0.8));

        FOR(i, 0, n) {
            FOR(j, 0, sz(sphere.faces)) {
                FOR(k, 0, sz(sphere.faces[j].vidx)) {
                    double x = sphere.vertices[sphere.faces[j].vidx[k]].x;
                    double y = sphere.vertices[sphere.faces[j].vidx[k]].y;
                    double z = sphere.vertices[sphere.faces[j].vidx[k]].z;
                    ch.vertices.pb(Vertex(x + hull[i].x, y + hull[i].y, z + hull[i].z));
                }

                pushFace(ch, sz(sphere.faces[j].vidx), rgb(0.8, 0.8, 0.8));
            }
        }
    }
}

void drawObj(Object * object) {
    GLfloat shininess[] = { 81.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_emission[] = {0.3, 0.2, 0.2, 0.0};

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
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

void setOrthographicProjection() {
    // Switch to projection mode.
    glMatrixMode(GL_PROJECTION);

    // Save previous matrix which contains settings
    // for the perspective projection.
    glPushMatrix();

    // Reset matrix.
    glLoadIdentity();

    // Set a 2D orthographic projection.
    gluOrtho2D(0, windowWidth, windowHeight, 0);

    // Switch back to modelview mode.
    glMatrixMode(GL_MODELVIEW);
}

void restorePerspectiveProjection() {
    glMatrixMode(GL_PROJECTION);
    // Restore previous projection matrix.
    glPopMatrix();
    // Get back to modelview mode.
    glMatrixMode(GL_MODELVIEW);
}



void renderBitmapString(float x, float y, float z,
                        void *font, char *charArray) {
    char *c;
    glRasterPos3f(x, y, z);
    for (c = charArray; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}


void drawUiText() {
    setOrthographicProjection();
    glColor3f(1.0f, 1.0f, 0.8f);
    char s[100];
    if (currentPoint < points.size()) {
        sprintf(s,"Press 's' to perform a step in the algorithm.\n Scanned %d points out of %zu.", currentPoint, points.size());
    }
    else {
        sprintf(s,"Finished! Scanned %d points out of %zu. Right-click to restart. p to save.", currentPoint, points.size());

    }

    glPushMatrix();
    glLoadIdentity();
    renderBitmapString(5, 30, 0, GLUT_BITMAP_HELVETICA_18, s);
    glPopMatrix();
    restorePerspectiveProjection();
}

void display(void) {
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

    drawLines();
    drawUiText();
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

void saveObj(Object * object, string fileName) {
    ofstream outFile;

    string filePath = "C:\\Users\\hgarc\\Github\\ConvexHullOpenGL\\" + fileName;
    outFile.open(filePath.c_str());
    outFile << fixed << setprecision(6);
    FOR(i, 0, sz(object->vertices))
        outFile << "v " << object->vertices[i].x << " " << object->vertices[i].y << " " << object->vertices[i].z << endl;

    outFile << endl;
    FOR(i, 0, sz(object->faces)) {
        outFile << "# Color " << object->faces[i].color.r << " " << object->faces[i].color.g << " " << object->faces[i].color.b << endl;
        outFile << "f ";
        FOR(j, 0, sz(object->faces[i].vidx))
            outFile << object->faces[i].vidx[j] << " ";
        outFile << endl;
    }

    outFile.close();
    cout << "Saved object: " << fileName << endl;
}

void keyboard (unsigned char key, int x, int y) {
    switch (key) {
        case 'q':
            exit(0);
        // Press f key to go faster.
        case 'f':
            speed += 2.0f;
            break;
        case 's':
            if (currentPoint < points.size()) {
                doGrahamScanStep();
            }
            break;
        case 'r':
            doGrahamScanStep();
            break;
        case 'p':
            saveObj(&ch, "CH.obj");
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

// Restart the program's state.
void processParamsMenu(int option) {
    switch (option) {
        case REGENERATE:
            points.clear();
            hull.clear();
            generatePoints(50, 30);
            prepareGrahamScan();
            break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(300, 300);
    glutCreateWindow("Convex Hull Visualization");
    glShadeModel (GL_SMOOTH);
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 90.0 };
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 1.0 };
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);
    GLfloat light_ambient[] = { 0.0, 0.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    GLfloat lmodel_ambient[] = { 2.8, 2.8, 2.8, 2.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    // Enable lighting modes.
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    loadObjFiles();
    generatePoints(50, 30);
    prepareGrahamScan();

    // Register Glut callbacks.
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
