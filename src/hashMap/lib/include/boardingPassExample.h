#pragma once

typedef struct passType *boardingPass;

boardingPass createBoardingPass(int, char *, char *);
char *boardingPassToString(boardingPass);
int getIdNum(boardingPass);
char *getFirstName(boardingPass);
char *getLastName(boardingPass);
void copyBoardingPass(boardingPass *, boardingPass *);
