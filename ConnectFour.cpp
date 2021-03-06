﻿// ConnectFour.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// Author: Alfredo Sepulveda Van Hoorde
// https://github.com/alfredo-svh/ConnectFour

#include "pch.h"
#include <iostream>
#include <vector>
#include <thread>
#include <Windows.h>

using namespace std;


int nFieldWidth = 17;
int nFieldHeight = 15;
unsigned char *pField = nullptr;

int nScreenWidth = 80; //80
int nScreenHeight = 30; //30

bool bTurn = 0;
WORD wColor[2];
vector<vector<int>> vGrid = { {-1,-1,-1,-1,-1,-1, -1}, {-1,-1,-1,-1,-1,-1, -1}, {-1,-1,-1,-1,-1,-1, -1}, {-1,-1,-1,-1,-1,-1, -1}, {-1,-1,-1,-1,-1,-1, -1}, {-1,-1,-1,-1,-1,-1, -1}};


bool checkForWin(int color) {
	int cur = 0;

	//horizontal
	for (int py = 0; py < 6; py++) {
		cur = 0;
		for (int px = 0; px < 7; px++) {
			if (vGrid[py][px] == color) {
				cur++;
				if (cur == 4) {
					return true;
				}
			}
			else {
				cur = 0;
			}
		}
	}

	//vertical
	for (int px = 0; px < 7; px++) {
		cur = 0;
		for (int py = 0; py < 6; py++) {
			if (vGrid[py][px] == color) {
				cur++;
				if (cur == 4) {
					return true;
				}
			}
			else {
				cur = 0;
			}
		}
	}

	//diagonalBL2TR
	for (int py = 0; py < 3; py++) {
		cur = 0;
		for (int px = 0, y = py; px < 7 && y < 6; px++, y++) {
			if (vGrid[y][px] == color) {
				cur++;
				if (cur == 4) {
					return true;
				}
			}
			else {
				cur = 0;
			}
		}
	}
	for (int px =1; px < 4; px++) {
		cur = 0;
		for (int x = px, py = 0; x < 7 && py < 6 ; py++, x++) {
			if (vGrid[py][x] == color) {
				cur++;
				if (cur == 4) {
					return true;
				}
			}
			else {
				cur = 0;
			}
		}
	}

	//digaonalBR2TL
	for (int py = 0; py < 3; py++) {
		cur = 0;
		for (int y = py, px = 6; y < 6 && px >=0; px--, y++) {
			if (vGrid[y][px] == color) {
				cur++;
				if (cur == 4) {
					return true;
				}
			}
			else {
				cur = 0;
			}
		}
	}
	for (int px = 5; px >=3; px--) {
		cur = 0;
		for (int py = 0, x=px; x >=0 && py < 6; py++, x--) {
			if (vGrid[py][x] == color) {
				cur++;
				if (cur == 4) {
					return true;
				}
			}
			else {
				cur = 0;
			}
		}
	}

	return false;
}


bool insertToken(int column) {
	for (int i = 0; i < 6; i++) {
		if (vGrid[i][column] < 0) {
			vGrid[i][column] = bTurn;
			return true;
		}
	}
	return false;
}

int main() {
	/* Create playing field */

	pField = new unsigned char[nFieldWidth * nFieldHeight];

	//board
	for (int px = 0; px < nFieldWidth; px++) {
		for (int py = 0; py < nFieldHeight; py++) {
			pField[py*nFieldWidth + px] = (px % 2 == 1) ? 4 : 0;	// |
			pField[py*nFieldWidth + px] = (py % 2 == 1) ? 3 : pField[py*nFieldWidth + px]; // -
			pField[py*nFieldWidth + px] = (py % 2 == 1 && px % 2 == 1) ? 6 : pField[py*nFieldWidth + px]; //+
			pField[py*nFieldWidth + px] = (px == 0 || px == 1 || px == nFieldWidth - 1 || px == nFieldWidth - 2 || py == nFieldHeight - 1 || py == nFieldHeight - 2) ? 5 : pField[py*nFieldWidth + px]; // #
			pField[py*nFieldWidth + px] = (py == 0) ? 0 : pField[py*nFieldWidth + px]; // clear top row
		}
	}
	for (int i = 0; i < 7; i++) {
		pField[2 + 2 * i] = i + 7;
	}

	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	for (int i = 0; i < nScreenWidth*nScreenHeight; i++) {
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	/* RENDER */
	//Draw field
	for (int i = 0; i < nFieldWidth; i++) {
		for (int j = 0; j < nFieldHeight; j++) {
			screen[(j + 2)*nScreenWidth + (i + 2)] = L" BR-|#+1234567"[pField[j*nFieldWidth + i]];
		}
	}
	//Draw Turn
	if (bTurn)
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 21, L"TURN: PLAYER 2 (RED)");
	else
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 22, L"TURN: PLAYER 1 (BLUE)");

	//Display frame
	WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);


	/* game logic */

	wColor[0] = FOREGROUND_BLUE | BACKGROUND_BLUE;
	wColor[1] = FOREGROUND_RED | BACKGROUND_RED;

	bool bGameOver = false;
	int nWin= -1;
	bool bKey[7];
	short nColumn;
	int nTokenInScreen[7] = {nFieldHeight - 3,nFieldHeight - 3, nFieldHeight - 3, nFieldHeight - 3, nFieldHeight - 3, nFieldHeight - 3, nFieldHeight - 3};
	int nSpacesLeft = 42;


	/* main game loop */

	while (!bGameOver) {
		/* INPUT */
		while (1) {
			this_thread::sleep_for(1000ms);
			for (int k = 0; k < 7; k++) {
				bKey[k] = (1 & GetAsyncKeyState((unsigned char)("1234567"[k]))) != 0;
			}
			if (bKey[0]) {
				if (insertToken(0)) {
					nColumn = 0;
					break;
				}
			}
			else if (bKey[1]) {
				if (insertToken(1)) {
					nColumn = 1;
					break;
				}
			}
			else if (bKey[2]) {
				if (insertToken(2)) {
					nColumn = 2;
					break;
				}
			}
			else if (bKey[3]) {
				if (insertToken(3)) {
					nColumn = 3;
					break;
				}
			}			
			else if (bKey[4]) {
				if (insertToken(4)) {
					nColumn = 4;
					break;
				}
			}			
			else if (bKey[5]) {
				if (insertToken(5)) {
					nColumn = 5;
					break;
				}
			}
			else if (bKey[6]) {
				if (insertToken(6)) {
					nColumn = 6;
					break;
				}
			}
		}


		/* GAME LOGIC */

		WriteConsoleOutputAttribute(hConsole, (bTurn) ? &wColor[1] : &wColor[0], 1, { nColumn * 2 + 4,  (short)nTokenInScreen[nColumn] + 2 }, &dwBytesWritten);
		pField[nTokenInScreen[nColumn] * nFieldWidth + nColumn * 2 + 2] = (bTurn) ? 2 : 1;
		nTokenInScreen[nColumn] -= 2;

		//check for Game Over
		nSpacesLeft--;

		if (checkForWin(bTurn)) {
			nWin = bTurn;
			bGameOver = true;
		}
		else if (nSpacesLeft == 0) {
			bGameOver = true;
		}
		else {
			//change turn
			bTurn = !bTurn;
		}

		/* RENDER */
		//Draw field
		for (int i = 0; i < nFieldWidth; i++) {
			for (int j = 0; j < nFieldHeight; j++) {
				screen[(j + 2)*nScreenWidth + (i + 2)] = L" BR-|#+1234567"[pField[j*nFieldWidth + i]];
			}
		}
		if (bGameOver) {
			//Draw Game Over
			switch (nWin) {
				case(-1):
					swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 22, L"TIE! NO WINNER!      ");
					break;
				case(0):
					swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 22, L"PLAYER 1 (BLUE) WINS!");
					break;
				case(1):
					swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 22, L"PLAYER 2 (RED) WINS! ");
					break;
			};
			swprintf_s(&screen[4 * nScreenWidth + nFieldWidth + 6], 20, L"PRESS ENTER TO QUIT");
		}
		else {
			//Draw Turn
			if (bTurn)
				swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 21, L"TURN: PLAYER 2 (RED)");
			else
				swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 22, L"TURN: PLAYER 1 (BLUE)");
		}

		//Display frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);

		if (bGameOver) {
			bool bEnter=0;
			while (1) {
				this_thread::sleep_for(1000ms);
				bEnter = (1 & GetAsyncKeyState((unsigned char)('\x0D'))) != 0;
				if (bEnter) {
						break;
				}
			}
		}
	}

	// Game Over
	CloseHandle(hConsole);
	system("pause");

	return 0;
}
