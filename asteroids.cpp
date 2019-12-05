// asteroids.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

#include "olcConsoleGameEngine.h"

class Asteroids : public olcConsoleGameEngine
{
public:
	Asteroids()
	{
		m_sAppName = L"Asteroids";
	}
private:
	struct sSpaceObject 
	{
		float x;
		float y;
		//velocity
		float dx; //spd and direction of the object
		float dy;
		int nSize;
		float angle;
	};

	vector<sSpaceObject> vecAsteroids;
	vector<sSpaceObject> vecBullets;
	
	sSpaceObject player;
	int nScore = 0;
	bool bDead = false;

	vector<pair<float, float>> vecModelShip;
	vector<pair<float, float>> vecModelAsteroid;

protected:
	//called by olcCOnsoleGame Engine
	virtual bool OnUserCreate() //threads uses this
	{
		vecAsteroids.push_back({ 20.0f, 20.0f, 8.0f, -6.0f, (int)16 });//might think not an int
		
		//initialize player position
		player.x = ScreenWidth() / 2;
		player.y = ScreenHeight() / 2;
		player.dx = 0.0f; //velocity
		player.dy = 0.0f;
		player.angle = 0.0f;
		
		//intilizer list
		vecModelShip =
		{
			{0.0f, -5.0f},
			{-2.5f, +2.5},
			{+2.5f, +2.5f}

		}; //isoceles triangle

		//ASTEROIDS
		//0.0 is assumed to be the origin/middle of asteroids
		//points lie on a circle, 20 points, 20 segments in degrees
		int verts = 20;
		for (int i = 0; i < verts; i++)
		{
			float radius = 1.0f; ///setting to one gives us a unit circle
			float a = ((float)i / (float)verts) * 6.28318f;
			vecModelAsteroid.push_back(make_pair(radius * sinf(a), radius *cosf(a)));
		}

		//static resurces and not used to define
		ResetGame();

		return true;
	}
	virtual bool OnUserUpdate(float fElapsedTime)
	{

		if (bDead)
			ResetGame();
		//clear screen and fill with blck pixels
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

		//math logic
		//V = P2-P1/T == (V*T) + P1 = P2
		// A = V2 - V1/T == (A*T) + V1 = V2

		if (m_keys[VK_LEFT].bHeld)
			player.angle -= 5.0f * fElapsedTime;
		if (m_keys[VK_RIGHT].bHeld)
			player.angle += 5.0f * fElapsedTime;

		if (m_keys[VK_UP].bHeld)
		{
			// ACCELERATION changes VELCOITY (with respect to time)
			//we get a vector and use as a velocity
			player.dx += sin(player.angle) * 20.0f * fElapsedTime;
			//cos is negative because screen upside down
			player.dy += -cos(player.angle) * 20.0f * fElapsedTime;
		}

		//BULLETS
		if (m_keys[VK_SPACE].bReleased)
		{
			vecBullets.push_back({ player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 0, 0 });
		}

		//updates the position
		//VELOCITY changes POSITION (with respect to time)
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		//cant forget ship is also can object
		WrapCoordinates(player.x, player.y, player.x, player.y);

		//Check if ship collided with asteroids
		for (auto &a : vecAsteroids)
			if (IsPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y))
				bDead = true;

		// Update and draw asteroids
		//& included because contents may change
		for (auto &a : vecAsteroids) //ranged based for loop that has a temporary range expression
		{
			//math logic
			//V = P2-P1/T == (V*T) + P1 = P2
			// A = V2 - V1/T == (A*T) + V1 = V2

			a.x += a.dx * fElapsedTime; //basically delta.time in unity
			a.y += a.dy * fElapsedTime;
			WrapCoordinates(a.x, a.y, a.x, a.y);

			//for square
			/*for (int x = 0; x < a.nSize; x++)
				for (int y = 0; y < a.nSize; y++)
					Draw(a.x + x, a.y + y, PIXEL_QUARTER, FG_RED);*/
			DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, a.nSize);
		}

		//need this because you are looping thrgh the og asteroid loop and if
		//you make changes to it will crash
		vector<sSpaceObject> newAsteroids;

		for (auto &b : vecBullets) //ranged based for loop that has a temporary range expression
		{
			//math logic
			//V = P2-P1/T == (V*T) + P1 = P2
			// A = V2 - V1/T == (A*T) + V1 = V2

			b.x += b.dx * fElapsedTime; //basically delta.time in unity  
			b.y += b.dy * fElapsedTime;
			WrapCoordinates(b.x, b.y, b.x, b.y);
			Draw(b.x, b.y);

			for(auto &a : vecAsteroids)
			{
				if (IsPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y))
				{
					//astroid is hit
					b.x = -100;
					if (a.nSize > 4)
					{
						//create 2 child
						float angle1 = ((float)rand() / (float)RAND_MAX) * 6.28318f;
						float angle2 = ((float)rand() / (float)RAND_MAX) * 6.28318f;
						// >> shifts bit over, so for 00000101 size (5 in bit) = 00000010 (2 in bit)
						newAsteroids.push_back({ a.x, a.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), (int)a.nSize >> 1, 0.0f });
						newAsteroids.push_back({ a.x, a.y, 10.0f * sinf(angle2), 10.0f * cosf(angle2), (int)a.nSize >> 1, 0.0f });
					}
					a.x = -100;
				}
			}
		}

		//add new asteroids to exisitng one
		for (auto a : newAsteroids)
			vecAsteroids.push_back(a);

		//remove off screen bullets
		if (vecBullets.size() > 0)
		{
			//lambda expression & means store by refence so potentially can alter og
			//stores information from the vec to the temp sSpaceObject and places that object
			// at the end of the array if it fails the if statments due to the remove_if
			auto i = remove_if(vecBullets.begin(), vecBullets.end(), [&](sSpaceObject o) { return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() || o.y >= ScreenHeight() - 1); });
			//points to start of all the bullets we can remove
			//make sure there is stuff to remove
			if (i != vecBullets.end())
				vecBullets.erase(i);
		}

		//remove asteroids
		//remove off screen 
		if (vecAsteroids.size() > 0)
		{
			auto i = remove_if(vecAsteroids.begin(), vecAsteroids.end(), [&](sSpaceObject o) { return (o.x < 0); });

			if (i != vecAsteroids.end())
				vecAsteroids.erase(i);
		}

		//Draw ship
		//with working with games to important to understand matrix and some trig
		//used to create an object and then morph it to desire coordi
		DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);
		
		return true;
	}

	// with working with games to important to understand matrix and some trig
	//used to create an object and then morph it to desire coordi
	void DrawWireFrameModel(const vector<pair<float, float>> &vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, short col = FG_WHITE)
	{
		// pair.first = x coordinate
		// pair.second = y coordinate

		// Create translated model vector of coordinate pairs
		//dont want to change the model coordinates
		vector<pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		// Rotate
		//rotate around the origin and multi by matrix into direction player is facing
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
		}

		// Scale
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
		}

		// Translate
		//need to offset to where player is currently is
		//remember, coordinates are just for orientation, and by 
		//adding player location, it translate to where you want the object to go
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
		}

		// Draw Closed Polygon
		//once rotate and translate, want to draw to screen, and draws lines between
		//theres one more point then the model
		for (int i = 0; i < verts + 1; i++)
		{
			//by using modules, you make sure to get all 3 lines drawn
			int j = (i + 1);
			DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
				vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
		}
	}

	void WrapCoordinates(float ix, float iy, float &ox, float &oy)
	{
		ox = ix;
		oy = iy;
		//check if beyond boundary
		if (ix < 0.0f) ox = ix + (float)ScreenWidth(); //if ix = -5, hen goes to other side of screen
		if (ix >= (float)ScreenWidth()) ox = ix - (float)ScreenWidth();
		if (iy < 0.0f) oy = iy + (float)ScreenHeight(); //if ix = -5, hen goes to other side of screen
		if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();


	}

	virtual void Draw(int x, int y, short c = 0x2588, short col = 0x000F)
	{
		float fx, fy;
		WrapCoordinates(x, y, fx, fy);
		olcConsoleGameEngine::Draw(fx, fy, c, col);
	}

	//distance formula, just calculating from origin 
	bool IsPointInsideCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) < radius;
	}

	void ResetGame()
	{
		vecAsteroids.clear();
		vecBullets.clear();

		vecAsteroids.push_back({ 20.0f, 20.0f, 8.0f, -6.0f, (int)16 });//might think not an int
		vecAsteroids.push_back({ 100.0f, 20.0f, -5.0f, 3.0f, (int)16 });//might think not an int

		//initialize player position
		player.x = ScreenWidth() / 2;
		player.y = ScreenHeight() / 2;
		player.dx = 0.0f; //velocity
		player.dy = 0.0f;
		player.angle = 0.0f;

		bDead = false;
	}

};

int main()
{
	Asteroids game;
	game.ConstructConsole(160, 100, 8, 8);
	game.Start();
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
