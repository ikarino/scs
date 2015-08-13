#pragma once

#include <random>
#include <ctime>
#include <string>
#include <functional>
/* 種族とlvに対して能力値を返す */
/* 種族リストはname.listを参照 */

int GetHP(int sp, int lv);
int GetAT(int sp, int lv);
int GetDF(int sp, int lv);
int GetREC(int sp);
int GetEXP(int sp, int lv);
int GetMXLV(int sp);


int hp1(int lv);
int hp2(int lv);
int hp3(int lv);
int hp4(int lv);
int hp5(int lv);
int hp6(int lv);
int hp7(int lv);
int hp8(int lv);

int at1(int lv);
int at2(int lv);
int at3(int lv);
int at4(int lv);
int at5(int lv);
int at6(int lv);
int at7(int lv);
int at8(int lv);

int df1(int lv);
int df2(int lv);
int df3(int lv);
int df4(int lv);
int df5(int lv);
int df6(int lv);
int df7(int lv);
int df8(int lv);

std::string GetName(int sp);

// メルセンヌ・ツイスターによる乱数。25199は1~10,100の最小公倍数
static auto mtrand = std::bind(std::uniform_int_distribution<int>(0, 25199),
                        std::mt19937(static_cast<unsigned int>(time(nullptr))));

