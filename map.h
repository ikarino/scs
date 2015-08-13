#pragma once

#include "unit.h"
#include <vector>
class Unit;
class Input;
class Map {
 public:
  enum Mapping {
    EMPTY,
    WALL,
    ENEMY,
    SUMMON,
  };
  Map();
  void ShowMap() const;
  void ShowMap10x10() const;
  void ShowHP10x10(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy) const;
  void SetMP(int i, Mapping mp) { mp_[i] = mp; }
  // メインの部分
  int MainBattle(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp);
  int MainBattle(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp, Fellow *dead);
  void MainBattle_Debug(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp);
  // 各モード
  void EfficiencyMode(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp);
  void DurabilityMode(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp);
  void DebugMode(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp);
  void SafetyMode(FELLOWVECTOR *u_fellow, ENEMYVECTOR *u_enemy, Input *imp);
  void ClearMap();
  bool VacantSpace(int i);
  bool EnemyExist();
  std::vector<int> Movable(int place);
  int GetMP(int i) { return mp_[i]; }

 private:
  Mapping mp_[20*20];
};
