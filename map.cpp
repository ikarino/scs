// Copyright 2014 ikarino
#include <vector>
#include <cstdio>
#include <iostream>
#include "map.h"
#include "unit.h"
#include "input.h"
#include "status.h"

using std::vector;
Map::Map() {
  for (int i = 0; i < 400; ++i) {
    mp_[i] = EMPTY;
  }
}

bool Map::EnemyExist() {
  for (int i = 0; i < 400; i++) {
    if (mp_[i] == 2) {
      return true;
    }
  }
  return false;
}

vector<int> Map::Movable(int place) {
  vector<int> ok;
  bool movable[8];
  for (int i = 0; i < 8; i++) {
    if (mp_[benri(i, place)] == 0) {
      movable[i] = true;
    } else {
      movable[i] = false;
    }
  }
  if (mp_[place-1] == 1) {
    movable[0] = false;
    movable[3] = false;
    movable[5] = false;
  }
  if (mp_[place+1] == 1) {
    movable[2] = false;
    movable[4] = false;
    movable[7] = false;
  }
  if (mp_[place-20] == 1) {
    movable[0] = false;
    movable[1] = false;
    movable[2] = false;
  }
  if (mp_[place+20] == 1) {
    movable[5] = false;
    movable[6] = false;
    movable[7] = false;
  }

  for (int i = 0; i < 8; i++) {
    if (movable[i]) {
      ok.push_back(benri(i, place));
    }
  }
  return ok;
}

bool Map::VacantSpace(int i) {
  if (mp_[i-21] * mp_[i-20] * mp_[i-19]
      * mp_[i-1] * mp_[i+1]
      * mp_[i+19] * mp_[i+20] * mp_[i+21] == 0) {
    return true;
  } else {
    return false;
  }
}


void Map::ShowMap() const {
  for (int i = 0; i < 20; i++) {
    // printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
    for (int j = 0; j < 20; j++) {
      printf("|%d", mp_[i*20+j]);
    }
    printf("|\n");
  }
  // printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
}

void Map::ShowMap10x10() const {
  for (int i = 0; i < 10; i++) {
    // printf("+-+-+-+-+-+-+-+-+-+-+\n");
    for (int j = 0; j < 10; j++) {
      printf("|%d", mp_[i*20+j]);
    }
    printf("|\n");
  }
  // printf("+-+-+-+-+-+-+-+-+-+-+\n");
}

void Map::ShowHP10x10(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy) const {
  int hps[400];
  for (int i = 0; i < 400; i++) {
    if (mp_[i] == 1) {
      hps[i] = -100;
    } else {
      hps[i] = 0;
    }
  }

  vector<Fellow>::iterator it = u_fellow->begin();
  while (it != u_fellow->end()) {
    hps[it->GetPlace()] = it->GetIncidentHP();
    ++it;
  }
  vector<Enemy>::iterator it2 = u_enemy->begin();
  while (it2 != u_enemy->end()) {
    hps[it2->GetPlace()] = - it2->GetIncidentHP();
    ++it2;
  }
  for (int i = 0; i < 10; i++) {
    printf("+----+----+----+----+----+----+----+----+----+----+\n");
    for (int j = 0; j < 10; j++) {
      if (hps[i*20+j] == 0) {
        printf("|    ");
      } else if (hps[i*20+j] == -100) {
        printf("|++++");
      } else {
        printf("|%4d", hps[i*20+j]);
      }
    }
    printf("|\n");
  }
  printf("+----+----+----+----+----+----+----+----+----+----+\n\n\n");
}


// Efficeincy, Durability
int Map::MainBattle(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp) {
  int presentNumOfEnemy;
  int i;
  for (int t = 0; t < imp->T_; t++) {
    // cout << "Turn " << t << std::endl;
    vector<Enemy>::iterator it_e = u_enemy->begin();
    presentNumOfEnemy = u_enemy->size();
    i = 0;
    while (it_e != u_enemy->end()) {
      it_e->Action(u_enemy, u_fellow, this);
      ++it_e;
      ++i;
      if (i >= presentNumOfEnemy) break;
    }
    // this->ShowHP10x10(u_fellow, u_enemy);
    for (int j = 0; j < 2; j++) {
      vector<Fellow>::iterator it_f = u_fellow->begin();
      while (it_f != u_fellow->end()) {
        if (j == 0 || it_f->GetSPEED() == 2) {
          it_f->Action(u_enemy, u_fellow, this);
        }
        // this->ShowHP10x10(u_fellow, u_enemy);
        ++it_f;
      }
    }
    // this->ShowHP10x10(u_fellow, u_enemy);
    // u_enemy->back().ShowEXP(0);
    if (!this->EnemyExist()) {
      return t;
      break;
    }
  }
  return imp->T_-1;
}


// Debug用MainBattle
// 分裂方向がelseに渡らなかったパターンを返す
void Map::MainBattle_Debug(
    FELLOWVECTOR *u_fellow,
    ENEMYVECTOR *u_enemy,
    Input *imp) {
  int presentNumOfEnemy;
  int i;
  for (int t = 0; t < imp->T_; t++) {
    std::cout << "Turn " << t << std::endl;
    vector<Enemy>::iterator it_e = u_enemy->begin();
    presentNumOfEnemy = u_enemy->size();
    i = 0;
    while (it_e != u_enemy->end()) {
      it_e->Action(u_enemy, u_fellow, this);
      ++it_e;
      ++i;
      if (i >= presentNumOfEnemy) break;
    }
    this->ShowHP10x10(u_fellow, u_enemy);
    for (int j = 0; j < 2; j++) {
      vector<Fellow>::iterator it_f = u_fellow->begin();
      while (it_f != u_fellow->end()) {
        if (j == 0 || it_f->GetSPEED() == 2) {
          it_f->Action(u_enemy, u_fellow, this);
        }
        ++it_f;
      }
    }
    // this->ShowHP10x10(u_fellow, u_enemy);
    u_enemy->back().ShowEXP(0);
    if (!this->EnemyExist()) {
      std::cout << "Smallghoul Terminated" << std::endl;
      exit(1);
    }
  }
}


// Safety
/*
 *
 * 引数*deadに死んだモンスターのインスタンスを渡す
 * 戻り値に絶滅ターンを渡す
 *
 */
int Map::MainBattle(
    FELLOWVECTOR *u_fellow,
    ENEMYVECTOR *u_enemy,
    Input *imp,
    Fellow *dead) {
  int presentNumOfEnemy;
  int i;
  for (int t = 0; t < imp->T_; t++) {
    vector<Enemy>::iterator it_e = u_enemy->begin();
    presentNumOfEnemy = u_enemy->size();
    i = 0;
    while (it_e != u_enemy->end()) {
      it_e->Action(u_enemy, u_fellow, this, dead);
      ++it_e;
      ++i;
      if (i >= presentNumOfEnemy) break;
    }
    for (int j = 0; j < 2; j++) {
      vector<Fellow>::iterator it_f = u_fellow->begin();
      while (it_f != u_fellow->end()) {
        if (j == 0 || it_f->GetSPEED() == 2) {
          it_f->Action(u_enemy, u_fellow, this);
        }
        ++it_f;
      }
    }
    if (!this->EnemyExist()) {
      return t;
      break;
    }
  }
  return imp->T_-1;
}

void Map::DurabilityMode(
    FELLOWVECTOR *u_fellow,
    ENEMYVECTOR *u_enemy,
    Input *imp) {
  int numOfFellow = imp->GetNumberOfSummon();
  int totalTURN = 0;
  int totalEXP = 0;
  int part[10];
  int durated = 0;
  for (int i = 0; i < 10; i++) { part[i] = 0; }
  int t;
  std::cout << "#####################################################\n"
            << "scs :: 持続性シミュレーションをスタートします\n\n" << std::endl;
  for (int n = 0; n < imp->N_; n++) {
    //
    this->ClearMap();
    imp->MakeUnits(u_fellow, u_enemy);
    imp->MakeMap(this);
    u_enemy->back().ClearDeadSG();

    // main
    t = this->MainBattle(u_fellow, u_enemy, imp);

    if (t == imp->T_-1) durated++;
    //
    part[10*t/imp->T_]++;

    // トータルの処理
    totalTURN += t;
    totalEXP += Enemy::GetDeadSG();
    // 持続ターン表示
    std::cout << "N=" << n+1 << "\t" << t+1 << std::endl;
    // 初期化
    u_fellow->clear();
    u_enemy->clear();
  }

  std::cout << "----------------------RESULT-------------------------\n";
  std::cout << "Average Turn:\t"
            << 1 + static_cast<double>(totalTURN)/imp->N_ << std::endl;
  std::cout << "Average EXP:\t"
            << static_cast<double>(totalEXP)/imp->N_ << std::endl;
  std::cout << "Duration Rate:\t"
            << 100.0*durated/imp->N_ << " %" << std::endl;
  std::cout << std::endl;
  int max = 0;
  for (int i = 0; i < 10; i++) {
    if (part[i]>max) max = part[i];
  }
  max = max/20;
  for (int i = 0; i < 10; i++) {
    printf("%4d - %4d %4d\t|",
           static_cast<int>(imp->T_*i/10),
           static_cast<int>(imp->T_*(i+1)/10),
           part[i]);
    for (int j = 0; j < part[i]/max; j++) {
      std::cout <<  "+";
    }
    std::cout << std::endl;
  }
  std::cout << "-----------------------------------------------------"
            << std::endl;
}


void Map::DebugMode(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp) {
  int numOfFellow = imp->GetNumberOfSummon();
  int totalTURN = 0;
  int totalEXP = 0;
  int dirAll = 0;
  int dirElse = 0;
  std::cout << "#####################################################\n"
            << "scs :: Debug Mode\n\n" << std::endl;
  for (int n = 0; n < imp->N_; n++) {
    this->ClearMap();
    imp->MakeUnits(u_fellow, u_enemy);
    imp->MakeMap(this);
    u_enemy->back().ClearDeadSG();

    // main
    this->MainBattle_Debug(u_fellow, u_enemy, imp);

    // トータルの処理
    totalEXP += Enemy::GetDeadSG();
    dirAll += Enemy::directionAll_;
    dirElse += Enemy::directionElse_;
    dirAll += u_enemy->size();
    dirAll--;
    // 初期化
    u_fellow->clear();
    u_enemy->clear();
  }

  std::cout << "----------------------RESULT-------------------------\n";
  std::cout << "Total EXP :\t" << totalEXP << std::endl;
  std::cout << "Times Divided Sum  :\t" << dirAll << std::endl;
  std::cout << "Times Divided Else :\t" << dirElse << std::endl;
  printf("    percentage      :\t%5.2f%%\n", 100.0*dirElse/dirAll);
}



void Map::EfficiencyMode(FELLOWVECTOR *u_fellow,
                         ENEMYVECTOR *u_enemy,
                         Input *imp) {
  int numOfFellow = imp->GetNumberOfSummon();
  int totalEXP = 0;
  int exp[numOfFellow];
  for (int i = 0; i < numOfFellow; i++) { exp[i] = 0;}
  std::cout << "#####################################################\n"
            << "scs :: 効率計算シミュレーションをスタートします\n\n\n";
  for (int n = 0; n < imp->N_; n++) {
    //
    this->ClearMap();
    imp->MakeUnits(u_fellow, u_enemy);
    imp->MakeMap(this);
    u_enemy->back().ClearDeadSG();

    // main
    this->MainBattle(u_fellow, u_enemy, imp);

    totalEXP += Enemy::GetDeadSG();
    vector<Fellow>::iterator it_f = u_fellow->begin();
    int i = 0;
    while (it_f != u_fellow->end()) {
      exp[i] += it_f->GetEXP_();
      ++it_f;
      ++i;
    }
    // 経験値表示
    Enemy::ShowEXP(n+1);
    // 初期化
    u_fellow->clear();
    u_enemy->clear();
  }

  std::cout << "----------------------RESULT-------------------------\n";
  std::cout << "ID\tName\t\tEXP" << std::endl;
  for (int i = 0; i < numOfFellow; i++) {
    for (int j = 0; j < numOfFellow; j++) {
      if (imp->GetOrder(j) == i) {
        std::cout << i << "\t"
             << GetName(imp->GetSP(i)) << "\t"
             << static_cast<double>(exp[j])/imp->N_ << std::endl;
      }
    }
  }
  std::cout << "total:\t" << static_cast<double>(totalEXP)/imp->N_ << std::endl;
  std::cout << "1turn:\t" << static_cast<double>(totalEXP)/imp->N_/imp->T_
            << "\n-----------------------------------------------------\n";
}

void Map::SafetyMode(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp) {
  int numOfFellow = imp->GetNumberOfSummon();
  int dead[10];
  for (int i = 0; i < numOfFellow; i++) { dead[i] = 0;}
  Fellow *tmp;
  tmp = new Fellow();
  std::cout << "#####################################################\n"
            << "scs :: 安定性シミュレーションをスタートします\n\n\n";
  for (int n = 0; n < imp->N_; n++) {
    this->ClearMap();
    imp->MakeUnits(u_fellow, u_enemy);
    imp->MakeMap(this);
    u_enemy->back().ClearDeadSG();
    tmp->SetID(-1);
    // main
    // cout << "N=" <<  (n+1) << "\r";
    this->MainBattle(u_fellow, u_enemy, imp, tmp);
    // 死んだら処理
    if (tmp->GetID() != -1) {
      dead[tmp->GetID()]++;
    }
    // 初期化
    u_fellow->clear();
    u_enemy->clear();
  }
  delete tmp;

  std::cout << "----------------------RESULT-------------------------\n";
  std::cout << "ID\tName\t\tLV\tDeadCount\n";
  for (int i = 0; i < numOfFellow; i++) {
    std::cout << i << "\t"
         << GetName(imp->GetSP(i)) << "\t"
         << imp->GetLV(i) << "\t" << dead[i] << std::endl;
  }
  std::cout << "-----------------------------------------------------\n";
}


void Map::ClearMap() {
  for (int i = 0; i < 400; i++) {
    mp_[i] = EMPTY;
  }
}
