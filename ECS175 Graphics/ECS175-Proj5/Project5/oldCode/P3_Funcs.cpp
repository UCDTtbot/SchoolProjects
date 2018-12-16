#include "P3_Funcs.h"
using namespace std;
//      Project 3

//Phong Model Lighting
/********************************************************\
    Intesity = Iambiant + Idiffuse + Ispecular

    Ip = ka*Ia + (Il / (||f-p|| + K)) * (kd*(l(dot)n) + ks(r(dot)v)^n)
    
    ka, kd, ks - ambient, diffuse, specular reflection coefficient
    K - non-negative constant approx avg dist between scene and light
    n - phong constant
    Ia - ambient light intensity
    Il - light source intensity
    l - light vector (x-p)/||x-p|| where p is vertex pos and x is light source pos
        note, double bar ||x|| is 'norm', or the length sqrt(x1^2+...)
    n - unit outward normal vector at p
    r - normalized reflection vector
    v - veiewing vector (f-p)/||f-p|| where f being from point and p being the vertex pos

    Must be normalized

\********************************************************/

void phong(THPolygon &curPoly, float I_amb, float I_source, int nPhong, TPnt x, TPnt f, triple<float> k_amb, triple<float> k_dif, triple<float> k_spec)
{
    //Use to figure out Ip per vertex
    //Ip should be condensed to RGB
    for (int step = 0; step < curPoly.totalVerts; step++)
    {
        //Input: I's, k's, point x, point f
        //Existing: vertex p
        //Calculated: n, r, v, l, K
        triple<float> normal = curPoly.vertices[step].vertexNormal;
        TPnt p = curPoly.vertices[step];
        float x_p = sqrt(pow(p.x - x.x, 2) + pow(p.y - x.y, 2) + pow(p.z - x.z, 2));
        float f_p = sqrt(pow(p.x - f.x, 2) + pow(p.y - f.y, 2) + pow(p.z - f.z, 2));
        triple<float> l_vec((float)((x.x - p.x) / (float)x_p), (float)((x.y - p.y) / (float)x_p), (float)((x.z - p.z) / (float)x_p));
        triple<float> v_vec((float)((f.x - p.x) / (float)f_p), (float)((f.y - p.y) / (float)f_p), (float)((f.z - p.z) / (float)f_p));
        float K = x_p; 
        float nDOTl = normal.i * l_vec.i + normal.j * l_vec.j + normal.k * l_vec.k;
        triple<float> r_vec((float)(2*nDOTl * normal.i - l_vec.i), (float)(2*nDOTl * normal.j - l_vec.j), (float)(2 * nDOTl * normal.k - l_vec.k));
        float rDOTv = r_vec.i * v_vec.i + r_vec.j * v_vec.j + r_vec.k * v_vec.k;
        
        float R = k_amb.i*I_amb + ((I_source / (f_p + K))*(k_dif.i*nDOTl + k_spec.i*pow(rDOTv, nPhong)));
        float G = k_amb.j*I_amb + ((I_source / (f_p + K))*(k_dif.j*nDOTl + k_spec.j*pow(rDOTv, nPhong)));
        float B = k_amb.k*I_amb + ((I_source / (f_p + K))*(k_dif.k*nDOTl + k_spec.k*pow(rDOTv, nPhong)));


        curPoly.vertices[step].R = R;
        curPoly.vertices[step].G = G;
        curPoly.vertices[step].B = B;
//        triple<int> fir(curPoly.vertices[step].x, curPoly.vertices[step].y, curPoly.vertices[step].z);
//        triple<float> sec(R, G, B);
//        inten.insert(std::make_pair(fir, sec));
        cout << "\n";
        cout << "Polygon #: " << curPoly.ID << " Vertex: " << step << " \n";
        cout << "Phong Check: \n";
        cout << "Normal of Vertex: i = " << normal.i << ", j = " << normal.j << ", k = " << normal.k << " \n";
        cout << "Light Vector: i = " << l_vec.i << ", j = " << l_vec.j << ", k = " << l_vec.k << " \n";
        cout << "Reflection Vector: i = " << r_vec.i << ", j = " << r_vec.j << ", k = " << r_vec.k << " \n";
        cout << "Viewing Vector: i = " << v_vec.i << ", j = " << v_vec.j << ", k = " << v_vec.k << " \n";
        cout << "N dot L: " << nDOTl << " \n";
        cout << "r DOT v: " << rDOTv << " \n";
        cout << "||f - p||: " << f_p << " \n";
        cout << "Distance from light K (and x_p): " << K << " \n";

        cout << "RGB value obtained: R = " << curPoly.vertices[step].R << ", G = " << curPoly.vertices[step].G << ", B =  " << curPoly.vertices[step].B << " \n";
    }
}

//Gouraud Shading
/********************************************************\
    Colors vary in interior of the triangle. Different
    intensities are possible. 
    "Linear Interpolation of the 3 corner colors"

    General Linear Interpolation:
    f(Xbar) = (L1/L)*f1 + (L2/L)*f2

    General Linear Interpolation for triangles:
    f(P) = (a1/A)*f1 + (a2/A)*f2 + (a3/A)*f3
        Where A is total area, an are face areas, 
        and f() is an arbitrary function
    3-Steps of Linear Interpolation:
        Find intensities for three different lines,
        Between two points, and finally a horizontal
        scan line

\********************************************************/

//Painter's Algorithm
/********************************************************\



\********************************************************/


//Halftone
/********************************************************\
    How to simulate multiple intensity levels with
    a BINARY output device? (Think diff tones of blue)
     - Use a 3x3 "Mega-Pixel"


\********************************************************/


//Project 3 Util
void displayInfo()
{

}

