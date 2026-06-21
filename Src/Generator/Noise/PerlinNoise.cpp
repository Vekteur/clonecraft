#include "PerlinNoise.h"

#include "Util/Logger.h"

// p has coordinates in [0, 1[
double PerlinNoise::getNoise(dvec2 p) {
	// Unit square that contains the coordinates p, modulo 256
	ivec2 pi{ int(floor(p.x)) & 255 , int(floor(p.y)) & 255 };

	// Give random values to each corner of the square (only the last 2 bits matter)
	int g1 = perm[perm[pi.x] + pi.y];
	int g2 = perm[perm[pi.x + 1] + pi.y];
	int g3 = perm[perm[pi.x] + pi.y + 1];
	int g4 = perm[perm[pi.x + 1] + pi.y + 1];

	// Coordinates of p in the unit square
	dvec2 pf{ p.x - floor(p.x), p.y - floor(p.y) };

	// Calculate the dot product of pf and a random vector in the corner of the square
	double d1 = grad(g1, pf.x, pf.y);
	double d2 = grad(g2, pf.x - 1, pf.y);
	double d3 = grad(g3, pf.x, pf.y - 1);
	double d4 = grad(g4, pf.x - 1, pf.y - 1);

	pf = { fade(pf.x), fade(pf.y) };

	// Do bilinear interpolation
	double x1Inter = lerp(pf.x, d1, d2);
	double x2Inter = lerp(pf.x, d3, d4);
	double yInter = lerp(pf.y, x1Inter, x2Inter);

	return yInter;
}

double PerlinNoise::getNoise(dvec2 pos, double frequency) {
	return getNoise(pos * frequency);
}

// p has coordinates in [0, 1[
double PerlinNoise::getNoise(dvec3 p) {
	// Cube that contains p, modulo 256
	ivec3 pi{ int(floor(p.x)) & 255, int(floor(p.y)) & 255, int(floor(p.z)) & 255 };

	// Hash the 8 corners of the cube
	int a = perm[pi.x] + pi.y;
	int aa = perm[a] + pi.z, ab = perm[a + 1] + pi.z;
	int b = perm[pi.x + 1] + pi.y;
	int ba = perm[b] + pi.z, bb = perm[b + 1] + pi.z;

	// Coordinates of p inside the cube
	dvec3 pf{ p.x - floor(p.x), p.y - floor(p.y), p.z - floor(p.z) };

	// Dot product of pf with a random gradient at each corner
	double d000 = grad(perm[aa], pf.x, pf.y, pf.z);
	double d100 = grad(perm[ba], pf.x - 1, pf.y, pf.z);
	double d010 = grad(perm[ab], pf.x, pf.y - 1, pf.z);
	double d110 = grad(perm[bb], pf.x - 1, pf.y - 1, pf.z);
	double d001 = grad(perm[aa + 1], pf.x, pf.y, pf.z - 1);
	double d101 = grad(perm[ba + 1], pf.x - 1, pf.y, pf.z - 1);
	double d011 = grad(perm[ab + 1], pf.x, pf.y - 1, pf.z - 1);
	double d111 = grad(perm[bb + 1], pf.x - 1, pf.y - 1, pf.z - 1);

	dvec3 f{ fade(pf.x), fade(pf.y), fade(pf.z) };

	// Trilinear interpolation
	double x00 = lerp(f.x, d000, d100);
	double x10 = lerp(f.x, d010, d110);
	double x01 = lerp(f.x, d001, d101);
	double x11 = lerp(f.x, d011, d111);
	double y0 = lerp(f.y, x00, x10);
	double y1 = lerp(f.y, x01, x11);
	return lerp(f.z, y0, y1);
}

double PerlinNoise::getNoise(dvec3 pos, double frequency) {
	return getNoise(pos * frequency);
}

// Linear interpolation
double PerlinNoise::lerp(double amount, double left, double right) {
	return left + amount * (right - left);
}

// Fade function
double PerlinNoise::fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

// Get dot product of
double PerlinNoise::grad(int hash, double x, double y) {
	switch (hash & 3) {
	case 0: return x + y;
	case 1: return -x + y;
	case 2: return x - y;
	case 3: return -x - y;
	default: return 0;
	}
}

// Dot product with one of the 12 gradient directions of a cube's edges
double PerlinNoise::grad(int hash, double x, double y, double z) {
	switch (hash & 15) {
	case 0: case 12: return x + y;
	case 1: case 14: return -x + y;
	case 2: return x - y;
	case 3: return -x - y;
	case 4: return x + z;
	case 5: return -x + z;
	case 6: return x - z;
	case 7: return -x - z;
	case 8: return y + z;
	case 9: case 13: return -y + z;
	case 10: return y - z;
	case 11: case 15: return -y - z;
	default: return 0;
	}
}

std::array<int, 512> PerlinNoise::perm = []() {
	std::array<int, 512> temp{ 151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	for (int i = 0; i < 256; i++)
		temp[256 + i] = temp[i];
	return temp;
}();