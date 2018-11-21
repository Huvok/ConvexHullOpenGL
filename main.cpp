/*
 *
 */

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
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
#include <bitset>
#include <sstream>
#include <set>
#include <iomanip>
#include <string.h>
#include <limits.h>
#include <iterator>
#include <complex>
#include <fstream>

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

using namespace std;

typedef long long ll;
typedef vector<ll> vi;
typedef pair<ll, ll> ii;
typedef vector<ii> vii;
using namespace std;

float angle = 0.0f;

struct vertex {
    double x, y, z;
    vertex() { x = 0.; y = 0; z = 0; }
    vertex(double _x, double _y, double _z) { x = _x; y = _y, z = _z; }
};

struct rgb {
    double r, g, b;
    rgb() { r = 0.8; g = 0.8; b = 0.8; }
    rgb(double _r, double _g, double _b) { r = _r; g = _g; b = _b; }
};

struct face {
    vector<ll> vidx;
    rgb color;
    face() {}
    face(vi v, rgb c) { vidx = v; color = c; }
};

struct obj {
    vector<vertex> v;
    vector<face> f;
};

struct Point {
    int x, y, z;
    Point() { x = 0, y = 0, z = 0; }
    Point(int _x, int _y, int _z) { x = _x, y = _y, z = _z; }
};

struct Polygon {
    vector<Point> v;
};

vector<obj> points;
obj sphere;

void processVertex(string line, obj *object) {
    line = line.substr(2);
    string aux = "";
    double coords[3];
    ll cur = 0;
    FOR(i, 0, sz(line)) {
        if (line[i] == ' ') {
            coords[cur++] = atof(aux.c_str());
            aux = "";
        }
        else
            aux += line[i];
    }
    coords[cur++] = atof(aux.c_str());
    object->v.pb(vertex(coords[0], coords[1], coords[2]));
}

void processFace(string line, rgb c, obj *object) {
    line = line.substr(2);
    string aux = "";
    face vface;
    FOR(i, 0, sz(line)) {
        if (line[i] == ' ') {
            string nxtFace = "";
            FOR(j, 0, sz(aux)) {
                if (aux[j] == '/')
                    break;
                nxtFace += aux[j];
            }
            vface.vidx.pb(atoi(nxtFace.c_str()));
            aux = "";
        }
        else
            aux += line[i];
    }

    vface.vidx.pb(atoi(aux.c_str()));
    object->f.pb(vface);
    object->f[sz(object->f) - 1].color = c;
}

rgb processColor(string line) {
    rgb ret;
    double arr[3] = { 0 };
    string aux = "";
    line = line.substr(8);
    ll cur = 0;
    FOR(i, 0, sz(line)) {
        if (line[i] == ' ') {
            arr[cur++] = atof(aux.c_str());
            aux = "";
        }
        else
            aux += line[i];
    }

    arr[cur] = atof(aux.c_str());
    ret.r = arr[0]; ret.g = arr[1]; ret.b = arr[2];
    return ret;
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

    FOR(i, 0, sz(files)) {
        string filePath = "C:\\Users\\hgarc\\GitHub\\ConvexHullOpenGL\\" + files[i];
        obj *object;
        if (i == 0)
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

void drawObj(obj * object) {
    FOR(i, 0, sz(object->f)) {
        if (sz(object->f[i].vidx) == 3) {
            glBegin(GL_TRIANGLES);
                rgb c = object->f[i].color;
                glColor3f(c.r, c.g, c.b);
                FOR(j, 0, sz(object->f[i].vidx)) {
                    vertex v = object->v[object->f[i].vidx[j] - 1];
                    glVertex3f(v.x, v.y, v.z);
                }
            glEnd();
        }
        else if (sz(object->f[i].vidx) == 4) {
            glBegin(GL_QUADS);
                rgb c = object->f[i].color;
                glColor3f(c.r, c.g, c.b);
                FOR(j, 0, sz(object->f[i].vidx)) {
                    vertex v = object->v[object->f[i].vidx[j] - 1];
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

    gluLookAt(  0.0f, 10.0f, 15.0f,
                0.0f, 0.0f,  0.0f,
                0.0f, 1.0f,  0.0f);

    glPushMatrix();
    drawObj(&sphere);
    glPopMatrix();

    angle += 1.;

    glutSwapBuffers();
}

void reshape(int w, int h)
{
   if (h == 0)
		h = 1;
	float ratio =  w * 1.0 / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard (unsigned char key, int x, int y)
{
    switch (key) {
        case 27:
            exit(0);
            break;
        default:
            break;
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D Modelling");
    glEnable(GL_DEPTH_TEST);
    loadObjFiles();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}
